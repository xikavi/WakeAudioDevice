#ifndef AUDIOPEAKMETER_WIN_H
#define AUDIOPEAKMETER_WIN_H
#include <QObject>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class AudioPeakMeter
{
public:
    AudioPeakMeter(const QString& audioDeviceId);
    ~AudioPeakMeter();

    float getPeakValue() const;
    bool isValid() const;
private:
    IAudioMeterInformation *pMeterInfo = NULL;
};

#endif // AUDIOPEAKMETER_WIN_H
