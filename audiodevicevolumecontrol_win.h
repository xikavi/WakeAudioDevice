#ifndef AUDIODEVICEVOLUME_H
#define AUDIODEVICEVOLUME_H
#include <QObject>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class AudioDeviceVolumeControl : public QObject
{
public:
    AudioDeviceVolumeControl(const QString& audioDeviceId, QObject *parent = nullptr);
    ~AudioDeviceVolumeControl();

    std::optional<float> getMasterVolumeLevelScalar();
    bool setMasterVolumeLevelScalar(float level);
private:
    IAudioEndpointVolume *pAudioEndpointVolume = nullptr;
};

#endif // AUDIODEVICEVOLUME_H
