#include "audiopeakmeter_win.h"
#include "def_win.h"

AudioPeakMeter::AudioPeakMeter(const QString &audioDeviceId, QObject *parent) : QObject(parent)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDevice *pDevice = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->GetDevice(audioDeviceId.toStdWString().c_str(), &pDevice);
    EXIT_ON_ERROR(hr);

    hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, nullptr, (void**)&pMeterInfo);
    EXIT_ON_ERROR(hr);

Exit:
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pDevice);
}

float AudioPeakMeter::getPeakValue() const
{
    HRESULT hr;
    float peak = 0;
    if (isValid()) {
        hr = pMeterInfo->GetPeakValue(&peak);
        EXIT_ON_ERROR(hr);
    }
Exit:
    return peak;
}

bool AudioPeakMeter::isValid() const
{
    return pMeterInfo != nullptr;
}

AudioPeakMeter::~AudioPeakMeter()
{
    SAFE_RELEASE(pMeterInfo);
}
