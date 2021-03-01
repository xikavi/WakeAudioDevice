#include <QApplication>
#include "eventfilter.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EventFilter nativeEventFilter;
    a.installNativeEventFilter(&nativeEventFilter);
    MainWindow w;
    QObject::connect(&nativeEventFilter, &EventFilter::systemResumed, &w, &MainWindow::onSystemResumed);
    auto result = a.exec();
    return result;
}
