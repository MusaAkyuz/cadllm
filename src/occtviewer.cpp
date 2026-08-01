#include "occtviewer.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <GeomLProp_SLProps.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Precision.hxx>
#include <TopoDS.hxx>
#include <V3d_Viewer.hxx>
#include <WNT_Window.hxx>

OcctViewer::OcctViewer(QWidget* parent)
    : QWidget(parent)
{
    // A real native window (HWND) is required: OCCT draws into it directly.
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);

    // Without this, move events only arrive while a button is held, so the
    // cursor could never highlight the face it is hovering over.
    setMouseTracking(true);
}

void OcctViewer::initViewer()
{
    if (!m_view.IsNull()) {
        return;
    }

    Handle(Aspect_DisplayConnection) display = new Aspect_DisplayConnection();
    Handle(OpenGl_GraphicDriver) driver = new OpenGl_GraphicDriver(display);

    Handle(V3d_Viewer) viewer = new V3d_Viewer(driver);
    viewer->SetDefaultLights();
    viewer->SetLightOn();

    m_context = new AIS_InteractiveContext(viewer);

    m_view = viewer->CreateView();
    Handle(WNT_Window) window = new WNT_Window((Aspect_Drawable)winId());
    m_view->SetWindow(window);
    if (!window->IsMapped()) {
        window->Map();
    }
    m_view->SetBackgroundColor(Quantity_NOC_GRAY30);
    m_view->MustBeResized();

    TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();
    m_shape = new AIS_Shape(box);

    // The selection mode must be supplied to Display itself: passing -1 there
    // displays the shape but never loads it into the selection manager, and a
    // later Activate() cannot recover from that. Face mode makes individual
    // faces the pickable unit rather than the whole solid.
    m_context->Display(m_shape, AIS_Shaded, AIS_Shape::SelectionMode(TopAbs_FACE),
                       Standard_False);

    m_view->SetProj(V3d_XposYnegZpos);
}

void OcctViewer::syncViewSize()
{
    if (m_view.IsNull() || m_view->Window().IsNull()) {
        return;
    }

    Standard_Integer w = 0;
    Standard_Integer h = 0;
    m_view->Window()->Size(w, h);
    if (w <= 0 || h <= 0) {
        return;
    }

    const QSize current(w, h);
    if (current == m_appliedSize) {
        return;
    }
    m_appliedSize = current;

    m_view->MustBeResized();

    // Frame the model once the viewport finally has its real size; fitting
    // earlier would compute the zoom against the default 100x30 window.
    if (!m_didInitialFit) {
        m_view->FitAll();
        m_didInitialFit = true;
    }
}

void OcctViewer::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    initViewer();
    syncViewSize();
}

void OcctViewer::paintEvent(QPaintEvent*)
{
    initViewer();
    syncViewSize();
    m_view->Redraw();
}

void OcctViewer::resizeEvent(QResizeEvent*)
{
    syncViewSize();
}

QPoint OcctViewer::devicePos(const QPointF& logicalPos) const
{
    const qreal ratio = devicePixelRatioF();
    return QPoint(qRound(logicalPos.x() * ratio), qRound(logicalPos.y() * ratio));
}

void OcctViewer::mousePressEvent(QMouseEvent* event)
{
    if (m_view.IsNull()) {
        return;
    }

    m_lastPos = devicePos(event->position());
    m_pressPos = m_lastPos;
    if (event->button() == Qt::LeftButton) {
        m_view->StartRotation(m_lastPos.x(), m_lastPos.y());
    }
}

void OcctViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (m_view.IsNull()) {
        return;
    }

    const QPoint pos = devicePos(event->position());
    if (event->buttons() & Qt::LeftButton) {
        m_view->Rotation(pos.x(), pos.y());
    } else if (event->buttons() & Qt::MiddleButton) {
        // OCCT's Y axis points up, Qt's points down, hence the inverted dy.
        m_view->Pan(pos.x() - m_lastPos.x(), m_lastPos.y() - pos.y());
    } else {
        // No button held: let OCCT highlight whatever sits under the cursor.
        m_context->MoveTo(pos.x(), pos.y(), m_view, Standard_True);
    }
    m_lastPos = pos;
}

void OcctViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_view.IsNull() || event->button() != Qt::LeftButton) {
        return;
    }

    // Only treat this as a pick if the press did not turn into an orbit drag.
    const QPoint pos = devicePos(event->position());
    if ((pos - m_pressPos).manhattanLength() > 3) {
        return;
    }

    m_context->MoveTo(pos.x(), pos.y(), m_view, Standard_False);
    m_context->SelectDetected();
    m_view->Redraw();
    reportSelection();
}

void OcctViewer::reportSelection()
{
    m_context->InitSelected();
    if (!m_context->MoreSelected()) {
        emit selectionCleared();
        return;
    }

    const TopoDS_Shape picked = m_context->SelectedShape();
    if (picked.IsNull() || picked.ShapeType() != TopAbs_FACE) {
        emit selectionCleared();
        return;
    }

    // TopoDS::Face is a checked downcast from the generic TopoDS_Shape to the
    // face-specific type; the ShapeType() guard above is what makes it safe.
    const TopoDS_Face face = TopoDS::Face(picked);

    GProp_GProps props;
    BRepGProp::SurfaceProperties(face, props);
    const double area = props.Mass();

    // Sample the surface normal at the middle of the face's parameter space.
    Standard_Real uMin = 0.0;
    Standard_Real uMax = 0.0;
    Standard_Real vMin = 0.0;
    Standard_Real vMax = 0.0;
    BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);

    Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
    GeomLProp_SLProps slProps(surface, 0.5 * (uMin + uMax), 0.5 * (vMin + vMax),
                              1, Precision::Confusion());

    QVector3D normal;
    if (slProps.IsNormalDefined()) {
        gp_Dir dir = slProps.Normal();
        // A reversed face carries the same surface but faces the other way.
        if (face.Orientation() == TopAbs_REVERSED) {
            dir.Reverse();
        }
        normal = QVector3D(dir.X(), dir.Y(), dir.Z());
    }

    emit faceSelected(area, normal);
}

void OcctViewer::wheelEvent(QWheelEvent* event)
{
    if (m_view.IsNull()) {
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta == 0) {
        return;
    }

    const QPoint pos = devicePos(event->position());
    m_view->StartZoomAtPoint(pos.x(), pos.y());
    m_view->ZoomAtPoint(0, 0, delta > 0 ? 100 : -100, 0);
}
