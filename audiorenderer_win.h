#ifndef AUDIORENDERER_WIN_H
#define AUDIORENDERER_WIN_H
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <cstring>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <AudioSessionTypes.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class AudioRenderer : public QObject
{
    Q_OBJECT
public:
    enum class State { Stopped, Playing };

    static constexpr int REFTIMES_PER_SEC = 10000000;
    inline static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    inline static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    inline static const IID IID_IAudioClient = __uuidof(IAudioClient);
    inline static const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
    inline static const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);

    AudioRenderer(const QString& fileName, const QString& deviceId, QObject *parent = nullptr);
    ~AudioRenderer();

    void play(float volume = 1);
    void stop();
private:
    State state = State::Stopped;
    QTimer* timer = nullptr;

    WAVEFORMATEX *pwfx;
    IAudioClient *pAudioClient = nullptr;
    IAudioRenderClient *pRenderClient = nullptr;
    UINT32 numBufferFrames = 0;

    BYTE *pAllAudioData = nullptr;
    DWORD cbAllAudioData = 0;
    DWORD cbPosition = 0;

    bool allDataProcessed() const { return cbPosition >= cbAllAudioData; }

    void setState(State state);
    bool updateBuffer();
private slots:
    void onTimerTimeout();
signals:
    void stateChanged(AudioRenderer::State state);
};

#endif // AUDIORENDERER_WIN_H
