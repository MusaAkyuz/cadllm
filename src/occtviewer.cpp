#include "occtviewer.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <OpenGl_GraphicDriver.hxx>
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
    Handle(AIS_Shape) aisBox = new AIS_Shape(box);
    m_context->Display(aisBox, AIS_Shaded, 0, Standard_False);

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
    }
    m_lastPos = pos;
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
