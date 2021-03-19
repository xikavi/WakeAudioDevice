#include "log.h"

QString HResultToString(HRESULT hr) {
    LPTSTR errorText = nullptr;
    QString str;

    FormatMessage(
                // use system message tables to retrieve error text
                FORMAT_MESSAGE_FROM_SYSTEM
                // allocate buffer on local heap for error text
                |FORMAT_MESSAGE_ALLOCATE_BUFFER
                // Important! will fail otherwise, since we're not
                // (and CANNOT) pass insertion parameters
                |FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
                hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&errorText,  // output
                0, // minimum size for output buffer
                nullptr);   // arguments - see note

    if ( nullptr != errorText )
    {
        str = QString::fromWCharArray(errorText).trimmed();
        // release memory allocated by FormatMessage()
        LocalFree(errorText);
        errorText = nullptr;
    }
    return str;
}

QDebug operator<<(QDebug debug, AudioRenderer::State state)
{
    QDebugStateSaver saver(debug);
    switch (state) {
    case AudioRenderer::State::Stopped:
        debug.nospace() << "Stopped";
        break;
    case AudioRenderer::State::Playing:
        debug.nospace() << "Playing";
        break;
    }
    return debug;
}

FileDebug::~FileDebug() {
    str = str.trimmed();
    qt_message_output(QtDebugMsg, QMessageLogContext(), str);
    if (QFile file(logFileName); file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream (&file) << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss.zzz") << " " << str << Qt::endl;
        file.close();
    }
}
