#include "audiorenderer_win.h"
#include "def_win.h"

AudioRenderer::AudioRenderer(const QString &fileName, const QString &audioDeviceId, QObject *parent) : QObject(parent), timer(new QTimer(this)), audioDeviceId(audioDeviceId)
{
    HRESULT hr;

    IMFSourceReader *pReader = nullptr;
    IMFMediaType *pType = nullptr;
    IMFSample *pSample = nullptr;
    IMFMediaBuffer *pBuffer = nullptr;
    BYTE *pAudioData = nullptr;
    DWORD cbBuffer = 0;

    connect(timer, &QTimer::timeout, this, &AudioRenderer::onTimerTimeout);
    timer->setSingleShot(false);

    // Get audio client mix format

    if (!activateAudioClient())
        return;

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr);

    pwfx->wFormatTag = WAVE_FORMAT_PCM;
    pwfx->cbSize = 0;

    // Setup decoder

    hr = MFCreateSourceReaderFromURL(fileName.toStdWString().c_str(), nullptr, &pReader);
    EXIT_ON_ERROR(hr);

    hr = MFCreateMediaType(&pType);
    EXIT_ON_ERROR(hr);

    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    EXIT_ON_ERROR(hr);

    hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    EXIT_ON_ERROR(hr);

    hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, pwfx->nChannels);
    EXIT_ON_ERROR(hr);

    hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, pwfx->nSamplesPerSec);
    EXIT_ON_ERROR(hr);

    hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, pwfx->nBlockAlign);
    EXIT_ON_ERROR(hr);

    hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, pwfx->nAvgBytesPerSec);
    EXIT_ON_ERROR(hr);

    hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, pwfx->wBitsPerSample);
    EXIT_ON_ERROR(hr);

    hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    EXIT_ON_ERROR(hr);

    hr = pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pType);
    EXIT_ON_ERROR(hr);

    // Decode audio file and store it to memory

    while (true)
    {
        DWORD dwFlags = 0;

        // Read the next sample.
        hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwFlags, nullptr, &pSample);

        if (FAILED(hr)) { break; }

        if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
        {
            FileDebug() << "Type change - not supported by WAVE file format.";
            break;
        }
        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            break;
        }

        if (pSample == nullptr)
        {
            FileDebug() << "No sample";
            continue;
        }

        // Get a pointer to the audio data in the sample.

        hr = pSample->ConvertToContiguousBuffer(&pBuffer);

        if (FAILED(hr)) { break; }

        hr = pBuffer->Lock(&pAudioData, nullptr, &cbBuffer);

        if (FAILED(hr)) { break; }

        if (cbBuffer <= 0) {
            hr = pBuffer->Unlock();
            if (FAILED(hr)) { break; }
            continue;
        }

        DWORD copyPos = cbAllAudioData;
        cbAllAudioData += cbBuffer;
        pAllAudioData = static_cast<BYTE*>(pAllAudioData ? std::realloc(pAllAudioData, cbAllAudioData) : std::malloc(cbBuffer));
        if (!pAllAudioData)
            throw std::bad_alloc();
        std::memcpy(pAllAudioData + copyPos, pAudioData, cbBuffer);

        // Unlock the buffer.
        hr = pBuffer->Unlock();
        pAudioData = nullptr;

        if (FAILED(hr)) { break; }

        SAFE_RELEASE(pSample);
        SAFE_RELEASE(pBuffer);
    }

    if (pAudioData)
    {
        pBuffer->Unlock();
    }

    EXIT_ON_ERROR(hr);

Exit:
    SAFE_RELEASE(pType);
    SAFE_RELEASE(pReader);
    SAFE_RELEASE(pSample);
    SAFE_RELEASE(pBuffer);
    SAFE_RELEASE(pAudioClient);
}

bool AudioRenderer::activateAudioClient()
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDevice *pDevice = nullptr;

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->GetDevice(audioDeviceId.toStdWString().c_str(), &pDevice);
    EXIT_ON_ERROR(hr);

    SAFE_RELEASE(pAudioClient);

    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr);

Exit:
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pDevice);
    return !FAILED(hr);
}

// Starts to play audio data
bool AudioRenderer::play(float volume)
{
    HRESULT hr;
    ISimpleAudioVolume *pSimpleAudioVolume = nullptr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC; // 1 sec buffer

    stop();

    posAllAudioData = 0;

    if (!activateAudioClient())
        return false;

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE, hnsRequestedDuration, 0, pwfx, nullptr);
    EXIT_ON_ERROR(hr);

    hr = pAudioClient->GetService(IID_ISimpleAudioVolume, (void**)&pSimpleAudioVolume);
    EXIT_ON_ERROR(hr);

    hr = pSimpleAudioVolume->SetMasterVolume(volume, nullptr);
    EXIT_ON_ERROR(hr);

    // Get the size of the audio endpoint buffer.
    hr = pAudioClient->GetBufferSize(&numBufferFrames);
    EXIT_ON_ERROR(hr);

    // Setting update buffer timer interval to half of buffer duration
    timer->setInterval(500 * numBufferFrames / pwfx->nSamplesPerSec);

    hr = pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient);
    EXIT_ON_ERROR(hr);

    if (updateBuffer()) {
        hr = pAudioClient->Start();
        EXIT_ON_ERROR(hr);

        setState(State::Playing);
        timer->start();

        SAFE_RELEASE(pSimpleAudioVolume);
        return true;
    }

Exit:
    SAFE_RELEASE(pSimpleAudioVolume);
    SAFE_RELEASE(pRenderClient);
    SAFE_RELEASE(pAudioClient);
    return false;
}

// Updates OS Audio buffer with new portion of data
bool AudioRenderer::updateBuffer()
{
    HRESULT hr;
    BYTE *pData, *src;
    std::size_t cbCopy;
    UINT32 numFramesPadding;
    UINT32 numFramesAvailable;
    UINT32 numFramesRequested;
    UINT32 numFramesToProcess;

    hr = pAudioClient->GetCurrentPadding(&numFramesPadding); //AUDCLNT_E_DEVICE_INVALIDATED
    EXIT_ON_ERROR(hr);

    numFramesAvailable = numBufferFrames - numFramesPadding;
    numFramesToProcess = (cbAllAudioData - posAllAudioData) / pwfx->nBlockAlign;
    numFramesRequested = std::min(numFramesAvailable, numFramesToProcess);

    if (numFramesRequested <= 0)
        return true;

    src = pAllAudioData + posAllAudioData;
    cbCopy = numFramesRequested * pwfx->nBlockAlign;

    qDebug() << " numFramesPadding " << numFramesPadding << " numFramesAvailable " << numFramesAvailable << " numFramesRequested " << numFramesRequested;

    hr = pRenderClient->GetBuffer(numFramesRequested, (BYTE**)(&pData));
    EXIT_ON_ERROR(hr);

    std::memcpy(pData, src, cbCopy);
    qDebug() << "memcpy from " << src << " cbCopy " << cbCopy;

    hr = pRenderClient->ReleaseBuffer(numFramesRequested, 0);
    EXIT_ON_ERROR(hr);

    posAllAudioData += cbCopy;
    if (allDataProcessed()) {
        // Wait for all data in buffer to be played and then stop renderer by timer
        timer->start(1000 * (numFramesPadding + numFramesRequested) / pwfx->nSamplesPerSec);
        qDebug() << "all data played! stop in " << timer->interval();
    }

    return true;

    Exit:
    return false;
}

void AudioRenderer::onTimerTimeout()
{
    if (allDataProcessed() || !updateBuffer())
        stop();
}

// Stops to play audio data
void AudioRenderer::stop()
{
    if (state == State::Stopped)
        return;
    setState(State::Stopped);
    qDebug() << "AudioRenderer stopped";
    timer->stop();

    if (pAudioClient)
        pAudioClient->Stop();

    SAFE_RELEASE(pRenderClient);
    SAFE_RELEASE(pAudioClient);
}

void AudioRenderer::setState(AudioRenderer::State state)
{
    this->state = state;
    emit stateChanged(this->state);
}

AudioRenderer::~AudioRenderer()
{
    stop();
    std::free(pAllAudioData);
    CoTaskMemFree(pwfx);
}
