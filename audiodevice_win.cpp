#include "audiodevice.h"
#include "def_win.h"
#include <initguid.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

QList<AudioDevice> AudioDevice::list()
{
    QList<AudioDevice> result;
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDeviceCollection *pCollection = nullptr;
    IMMDevice *pDevice = nullptr;
    IMMEndpoint *pEndpoint = nullptr;
    IPropertyStore *pProps = nullptr;
    LPWSTR pwszID = nullptr;

    // Get enumerator for audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &pCollection);
    EXIT_ON_ERROR(hr);

    UINT count;
    hr = pCollection->GetCount(&count);
    EXIT_ON_ERROR(hr);

    for (UINT i = 0; i < count; i++) {
        // Get pointer to endpoint number i.
        hr = pCollection->Item(i, &pDevice);
        EXIT_ON_ERROR(hr);

        // Get the endpoint ID string.
        hr = pDevice->GetId(&pwszID);
        EXIT_ON_ERROR(hr);

        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
        EXIT_ON_ERROR(hr);

        PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr);

        hr = pDevice->QueryInterface(IID_IMMEndpoint, (void**)&pEndpoint);
        EXIT_ON_ERROR(hr);

        // Get endpoint's data flow
        EDataFlow dataFlow;
        hr = pEndpoint->GetDataFlow(&dataFlow);
        EXIT_ON_ERROR(hr);

        result.append({dataFlow == eRender ? Mode::Output : Mode::Input, QString::fromWCharArray(pwszID), QString::fromWCharArray(varName.pwszVal)});

        CoTaskMemFree(pwszID);
        pwszID = nullptr;
        PropVariantClear(&varName);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pProps);
        SAFE_RELEASE(pEndpoint);
    }

    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pCollection);
    return result;
Exit:
    CoTaskMemFree(pwszID);
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pCollection);
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pEndpoint);
    SAFE_RELEASE(pProps);
    return {};
}

QString AudioDevice::toString() const
{
    return QString(m_mode == Mode::Output ? "Output" : "Input") + " " + m_friendlyName + " " + m_id;
}
