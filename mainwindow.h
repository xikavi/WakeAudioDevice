#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QFile>
#include <QTimer>
#include <QSettings>

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
    bool started = false;
    QAudioFormat inputAudioFormat;
    QAudioInput* audioInput = nullptr;
    QIODevice* audioInputDevice = nullptr;
    QTimer* timer = nullptr; // 00:11:20
    QString outputSoundFileName;

    void timerOnTimeout();
    QAudioDeviceInfo audioDeviceInfoByDeviceName(const QString& deviceName);

    void startListening();
    void stopListening();
    void playSound();
};

#endif // MAINWINDOW_H
