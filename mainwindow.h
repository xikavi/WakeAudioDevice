#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QAudioInput>
#include <QFile>
#include <QTimer>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QHideEvent>

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

    Ui::MainWindow *ui;
    QSystemTrayIcon* trayIcon = nullptr;

    bool started = false;
    QAudioFormat inputAudioFormat;
    QAudioInput* audioInput = nullptr;
    QIODevice* audioInputDevice = nullptr;
    QTimer* timer = nullptr;
    QString outputSoundFileName;
    bool mustPlaySound = false;

    void timerOnTimeout();
    QAudioDeviceInfo audioDeviceInfoByDeviceName(const QString& deviceName);

    void startListening();
    void stopListening();

    void playSound();

    void setTrayIcon();

protected:
    void hideEvent(QHideEvent *event);
public slots:
    void onSystemResumed();
};

#endif // MAINWINDOW_H
