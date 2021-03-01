#ifndef EVENTFILTER_H
#define EVENTFILTER_H
#include <QObject>
#include <QAbstractNativeEventFilter>

class EventFilter: public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    EventFilter() : QObject() { }
    virtual ~EventFilter() { }

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
signals:
    void systemResumed();
};

#endif // EVENTFILTER_H
