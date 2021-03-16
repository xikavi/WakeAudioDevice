#include <QApplication>
#include <mfapi.h>
#include "eventfilter.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MFStartup(MF_VERSION);
    EventFilter nativeEventFilter;
    a.installNativeEventFilter(&nativeEventFilter);
    MainWindow w;
    QObject::connect(&nativeEventFilter, &EventFilter::systemResumed, &w, &MainWindow::onSystemResumed);
    auto result = a.exec();
    MFShutdown();
    return result;
}
