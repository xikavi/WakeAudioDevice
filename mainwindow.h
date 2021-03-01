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
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <stdio.h>
#include <cmath>
#include "audiopeakmeter_win.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_start_clicked();

private:
    static constexpr const char* settingsFileName = "settings.ini";
    static constexpr int peakMeterTimerInterval = 100;

    Ui::MainWindow *ui;
    QSystemTrayIcon* trayIcon = nullptr;

    bool started = false;
    QTimer* playSoundTimer = nullptr;
    QTimer* peakMeterTimer = nullptr;
    QString outputSoundFileName;

    std::unique_ptr<AudioPeakMeter> peakMeter;

    void start();
    void stop();

    void setTrayIcon();

    QAudioDeviceInfo audioDeviceInfoByDeviceName(const QString &deviceName);
protected:
    void hideEvent(QHideEvent *event);
public slots:
    void onSystemResumed();
private slots:
    void playSound();
    void showWindow();
};

#endif // MAINWINDOW_H
