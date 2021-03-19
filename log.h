#ifndef LOG_H
#define LOG_H
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <Windows.h>
#include "audiorenderer_win.h"

class FileDebug: public QDebug {
    QString logFileName;
    QString str;
public:
    FileDebug(QString logFileName = "log.txt") : QDebug(&str), logFileName(logFileName) {}
    virtual ~FileDebug();
};

QString HResultToString(HRESULT hr);
QDebug operator<<(QDebug debug, AudioRenderer::State state);

#endif // LOG_H
