#pragma once

#include <QWidget>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>

// Hosts an OCCT 3D view. OCCT owns the OpenGL context and draws straight into
// this widget's native window, so Qt must be told not to paint over it.
class OcctViewer : public QWidget
{
    Q_OBJECT

public:
    explicit OcctViewer(QWidget* parent = nullptr);

    // Returning nullptr tells Qt this widget has no Qt-side paint engine,
    // which is required when OCCT renders directly onto the native window.
    QPaintEngine* paintEngine() const override { return nullptr; }

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initViewer();

    // Qt reports mouse positions in logical pixels, OCCT expects device
    // pixels. These differ whenever Windows display scaling is not 100%.
    QPoint devicePos(const QPointF& logicalPos) const;

    Handle(AIS_InteractiveContext) m_context;
    Handle(V3d_View) m_view;
    QPoint m_lastPos;
};
