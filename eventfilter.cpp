#include "eventfilter.h"
#include "windows.h"

bool WinEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    PMSG msg = (PMSG)message;
    if (msg->message == WM_POWERBROADCAST) {
        if (msg->wParam == PBT_APMRESUMEAUTOMATIC) {
            emit systemResumed();
        }
    }
    return false;
}
