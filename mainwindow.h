#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QHideEvent>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <stdio.h>
#include <cmath>
#include "audiopeakmeter_win.h"
#include "audiorenderer_win.h"
#include "audiodevicevolumecontrol_win.h"

#define QOBJECT_SAFE_DELETE(obj) if ((obj) != nullptr) { delete obj; (obj) = nullptr; }

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    static constexpr const char* settingsFileName = "settings.ini";
    static constexpr int peakMeterTimerInterval = 100;

    Ui::MainWindow *ui;
    QSystemTrayIcon* trayIcon = nullptr;

    bool started = false;
    QTimer* playSoundTimer = nullptr;
    QTimer* peakMeterTimer = nullptr;
    QString outputSoundFileName;

    AudioRenderer* audioRenderer = nullptr;
    AudioPeakMeter* audioPeakMeter = nullptr;
    AudioDeviceVolumeControl* audioDeviceVolumeControl = nullptr;

    std::optional<float> prevAudioMasterVolume;

    void start();
    void stop();

    void setTrayIcon();
protected:
    void hideEvent(QHideEvent *event);
public slots:
    void onSystemResumed();
private slots:
    void on_pushButton_start_clicked();
    void playSound();
    void showWindow();
    void onAudioRendererStateChanged(AudioRenderer::State state);
};

#endif // MAINWINDOW_H
