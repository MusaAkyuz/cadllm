#pragma once

#include <QWidget>

class OcctViewer;
class QLabel;
class QPushButton;
class QVector3D;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onCloseClicked();
    void onFaceSelected(double area, const QVector3D& normal);
    void onSelectionCleared();

private:
    OcctViewer* m_viewer;
    QLabel* m_statusLabel;
    QPushButton* m_closeButton;
};
