#ifndef AUDIOPEAKMETER_WIN_H
#define AUDIOPEAKMETER_WIN_H
#include <QObject>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class AudioPeakMeter : public QObject
{
public:
    AudioPeakMeter(const QString& audioDeviceId, QObject *parent = nullptr);
    ~AudioPeakMeter();

    float getPeakValue() const;
    bool isValid() const;
private:
    IAudioMeterInformation *pMeterInfo = nullptr;
};

#endif // AUDIOPEAKMETER_WIN_H
