#include "audiopeakmeter_win.h"
#include "def_win.h"

AudioPeakMeter::AudioPeakMeter(const QString &audioDeviceId)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->GetDevice(audioDeviceId.toStdWString().c_str(), &pDevice);
    EXIT_ON_ERROR(hr);

    hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
    EXIT_ON_ERROR(hr);

Exit:
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pDevice);
}

float AudioPeakMeter::getPeakValue() const
{
    if (!isValid())
        return 0;
    HRESULT hr;
    float peak = 0;
    hr = pMeterInfo->GetPeakValue(&peak);
    if (FAILED(hr)) {
        FileDebug() << "GetPeakValue error " << hr;
    }
    return peak;
}

bool AudioPeakMeter::isValid() const
{
    return pMeterInfo != NULL;
}

AudioPeakMeter::~AudioPeakMeter()
{
    SAFE_RELEASE(pMeterInfo);
}
