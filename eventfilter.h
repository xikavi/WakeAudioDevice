#ifndef EVENTFILTER_H
#define EVENTFILTER_H
#include <QObject>
#include <QAbstractNativeEventFilter>

class WinEventFilter: public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    WinEventFilter() : QObject() { }
    virtual ~WinEventFilter() { }

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
signals:
    void systemResumed();
};

#endif // EVENTFILTER_H
