#include "occtviewer.h"

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
    m_view->FitAll();
}

void OcctViewer::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    initViewer();
}

void OcctViewer::paintEvent(QPaintEvent*)
{
    initViewer();
    m_view->Redraw();
}

void OcctViewer::resizeEvent(QResizeEvent*)
{
    if (!m_view.IsNull()) {
        m_view->MustBeResized();
    }
}
