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

    virtual ~FileDebug() {
        qt_message_output(QtDebugMsg, QMessageLogContext(), str);
        if (QFile file(logFileName); file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream (&file) << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss.zzz") << " " << str << Qt::endl;
            file.close();
        }
    }
};

QString HResultToString(HRESULT hr);
QDebug operator<<(QDebug debug, AudioRenderer::State state);

#endif // LOG_H
