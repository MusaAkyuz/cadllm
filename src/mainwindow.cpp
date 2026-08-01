#include "mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    m_closeButton = new QPushButton("Kapat", this);
    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
}

void MainWindow::onCloseClicked()
{
    close();
}
