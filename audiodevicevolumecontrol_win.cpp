#include "audiodevicevolumecontrol_win.h"
#include "def_win.h"

AudioDeviceVolumeControl::AudioDeviceVolumeControl(const QString &audioDeviceId, QObject *parent) : QObject(parent)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDevice *pDevice = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->GetDevice(audioDeviceId.toStdWString().c_str(), &pDevice);
    EXIT_ON_ERROR(hr);

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&pAudioEndpointVolume);
    EXIT_ON_ERROR(hr);

Exit:
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pDevice);
}

std::optional<float> AudioDeviceVolumeControl::getMasterVolumeLevelScalar()
{
    HRESULT hr;
    float level = 0;
    if (pAudioEndpointVolume) {
        hr = pAudioEndpointVolume->GetMasterVolumeLevelScalar(&level);
        EXIT_ON_ERROR(hr);
        return level;
    }
Exit:
    return std::nullopt;
}

bool AudioDeviceVolumeControl::setMasterVolumeLevelScalar(float level)
{
    HRESULT hr;
    if (pAudioEndpointVolume) {
        hr = pAudioEndpointVolume->SetMasterVolumeLevelScalar(level, nullptr);
        EXIT_ON_ERROR(hr);
        return true;
    }
Exit:
    return false;
}

AudioDeviceVolumeControl::~AudioDeviceVolumeControl()
{
    SAFE_RELEASE(pAudioEndpointVolume);
}
