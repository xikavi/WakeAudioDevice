#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H
#include <QObject>
#include <QDebug>

class AudioDevice
{
public:
    enum class Mode { Output, Input };
    AudioDevice(Mode mode, QString id, QString friendlyName) : m_mode(mode), m_id(id), m_friendlyName(friendlyName) {}

    static QList<AudioDevice> list();

    Mode mode() const { return m_mode; }
    QString id() const { return m_id; }
    QString friendlyName() const { return m_friendlyName; }
    QString toString() const;
private:
    Mode m_mode;
    QString m_id;
    QString m_friendlyName;
};

#endif // AUDIODEVICE_H
