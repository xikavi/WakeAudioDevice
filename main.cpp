#include <QApplication>
#include "eventfilter.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WinEventFilter nativeEventFilter;
    a.installNativeEventFilter(&nativeEventFilter);
    MainWindow w;
    QObject::connect(&nativeEventFilter, &WinEventFilter::systemResumed, &w, &MainWindow::onSystemResumed);
    return a.exec();
}
