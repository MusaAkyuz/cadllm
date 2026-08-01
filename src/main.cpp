#include <QApplication>

#include "mainwindow.h"
#include "version.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle(QString("cadllm %1").arg(appVersion()));
    window.resize(800, 600);
    window.show();

    return app.exec();
}
