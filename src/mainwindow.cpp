#include "mainwindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector3D>

#include "occtviewer.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    m_viewer = new OcctViewer(this);
    m_statusLabel = new QLabel(this);
    m_closeButton = new QPushButton("Kapat", this);

    connect(m_closeButton, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
    connect(m_viewer, &OcctViewer::faceSelected, this, &MainWindow::onFaceSelected);
    connect(m_viewer, &OcctViewer::selectionCleared, this, &MainWindow::onSelectionCleared);

    QHBoxLayout* statusRow = new QHBoxLayout;
    statusRow->addWidget(m_statusLabel, 1);
    statusRow->addWidget(m_closeButton, 0);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_viewer, 1);
    layout->addLayout(statusRow, 0);

    onSelectionCleared();
}

void MainWindow::onCloseClicked()
{
    close();
}

void MainWindow::onFaceSelected(double area, const QVector3D& normal)
{
    m_statusLabel->setText(QString("Face  area %1  normal (%2, %3, %4)")
                               .arg(area, 0, 'f', 2)
                               .arg(normal.x(), 0, 'f', 3)
                               .arg(normal.y(), 0, 'f', 3)
                               .arg(normal.z(), 0, 'f', 3));
}

void MainWindow::onSelectionCleared()
{
    m_statusLabel->setText("No face selected - click a face of the box");
}
