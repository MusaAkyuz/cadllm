#pragma once

#include <QWidget>

class OcctViewer;
class QPushButton;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onCloseClicked();

private:
    OcctViewer* m_viewer;
    QPushButton* m_closeButton;
};
