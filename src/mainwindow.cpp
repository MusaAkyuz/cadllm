#include "mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>

#include "occtviewer.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    m_viewer = new OcctViewer(this);
    m_closeButton = new QPushButton("Kapat", this);
    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::onCloseClicked);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_viewer, 1);
    layout->addWidget(m_closeButton, 0);
}

void MainWindow::onCloseClicked()
{
    close();
}
