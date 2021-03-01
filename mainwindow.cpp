#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "log.h"
#include "audiodevice.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    // UI

    ui->setupUi(this);

    for (const auto& device: AudioDevice::list()) {
        if (device.mode() == AudioDevice::Mode::Output)
            ui->comboBox_audioDeviceOutput->addItem(device.friendlyName(), device.id());
    }

    trayIcon = new QSystemTrayIcon(this);
    QMenu* trayMenu = new QMenu(this);
    trayMenu->addAction("Show", this, &MainWindow::showWindow);
    trayMenu->addAction("Close", this, &MainWindow::close);
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick)
            showWindow();
    });
    setTrayIcon();
    trayIcon->show();

    // Timers

    playSoundTimer = new QTimer(this);
    playSoundTimer->setSingleShot(true);
    connect(playSoundTimer, &QTimer::timeout, this, &MainWindow::playSound);

    QTimer* watcherTimer = new QTimer(this);
    watcherTimer->setSingleShot(false);
    connect(watcherTimer, &QTimer::timeout, this, [this]() {
        ui->label_timerTime->setText(QString::number(playSoundTimer->remainingTime() / 1000));
    });
    watcherTimer->start(1000);

    peakMeterTimer = new QTimer(this);
    peakMeterTimer->setSingleShot(false);
    peakMeterTimer->setInterval(peakMeterTimerInterval);
    connect(peakMeterTimer, &QTimer::timeout, this, [this]() {
        auto volume = peakMeter->getPeakValue();
        ui->label_listenerVolume->setText(QString::number(volume, 'f'));
        if (volume > ui->doubleSpinBox_listenerVolumeThreshold->value()) {
            playSoundTimer->start(ui->doubleSpinBox_playSoundInterval->value() * 1000);
        }
    });

    // Settings read

    QSettings settings(settingsFileName, QSettings::IniFormat);

    if (settings.contains("OutputAudioDeviceId")) {
        ui->comboBox_audioDeviceOutput->setCurrentIndex(-1);
        auto id = settings.value("OutputAudioDeviceId").toString();
        for (int i = 0; i < ui->comboBox_audioDeviceOutput->count(); ++i) {
            if (ui->comboBox_audioDeviceOutput->itemData(i) == id)
                ui->comboBox_audioDeviceOutput->setCurrentIndex(i);
        }
    }

    ui->doubleSpinBox_listenerVolumeThreshold->setValue(settings.value("ListenerVolumeThreshold", ui->doubleSpinBox_listenerVolumeThreshold->value()).toDouble());

    outputSoundFileName = settings.value("PlaySoundFileName", "sound.wav").toString();
    ui->doubleSpinBox_playSoundInterval->setValue(settings.value("PlaySoundInterval", ui->doubleSpinBox_playSoundInterval->value()).toDouble()); // 00:11:20
    ui->doubleSpinBox_playSoundVolume->setValue(settings.value("PlaySoundVolume", ui->doubleSpinBox_playSoundVolume->value()).toDouble());

    // Args check

    auto args = QCoreApplication::arguments();
    for (auto &arg: args)
        arg = arg.mid(1);
    if (args.indexOf("hide") == -1)
        show();
    if (args.indexOf("start") > -1)
        start();

}

void MainWindow::on_pushButton_start_clicked()
{
    started ? stop() : start();
}

void MainWindow::onSystemResumed()
{
    FileDebug() << "System resumed";
    if (started)
        playSound();
}

void MainWindow::setTrayIcon()
{
    trayIcon->setIcon(started ? QIcon(":/icon_enabled.png") : QIcon(":/icon_disabled.png"));
}

void MainWindow::start()
{
    if (started)
        return;
    auto id = ui->comboBox_audioDeviceOutput->currentData().toString();
    if (id.isEmpty())
        return;
    peakMeter = std::make_unique<AudioPeakMeter>(id);
    if (!peakMeter->isValid())
        return;
    started = true;
    setTrayIcon();
    ui->pushButton_start->setText("Stop");
    peakMeterTimer->start();
    playSound();
    FileDebug() << "Started";
}

void MainWindow::stop()
{
    if (!started)
        return;
    FileDebug() << "Stopped";
    started = false;
    setTrayIcon();
    playSoundTimer->stop();
    peakMeterTimer->stop();
    ui->pushButton_start->setText("Start");
}

void MainWindow::playSound()
{
    FileDebug() << "Playing sound" << outputSoundFileName;
    auto adi = audioDeviceInfoByDeviceName(ui->comboBox_audioDeviceOutput->currentText());
    if (!adi.isNull()) {
        QFile* file = new QFile(outputSoundFileName);
        if (file->open(QFile::ReadOnly)) {
            QAudioOutput* audio = new QAudioOutput(adi, adi.preferredFormat(), this);
            audio->setVolume(ui->doubleSpinBox_playSoundVolume->value());
            connect(audio, &QAudioOutput::stateChanged, this, [this, audio, file](QAudio::State state) {
                FileDebug() << "audio output state" << state;
                switch (state) {
                case QAudio::IdleState:
                    audio->stop();
                    file->close();
                    audio->deleteLater();
                    file->deleteLater();
                    break;
                case QAudio::StoppedState:
                    if (audio->error() != QAudio::NoError) {
                        FileDebug() << "audio output error" << audio->error();
                    }
                    if (started && !playSoundTimer->isActive())
                        playSoundTimer->start(0);
                    break;
                default: ;
                }
            });
            audio->start(file);
        } else
            file->deleteLater();
    }
}

QAudioDeviceInfo MainWindow::audioDeviceInfoByDeviceName(const QString &deviceName)
{
    auto devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    for (const auto& info: devices) {
        if (info.deviceName() == deviceName)
            return info;
    }
    return QAudioDeviceInfo();
}

void MainWindow::showWindow()
{
    show();
    showNormal();
    activateWindow();
}

void MainWindow::hideEvent(QHideEvent *event)
{
    if (event->spontaneous() && isVisible())
        hide();
}

MainWindow::~MainWindow()
{
    stop();
    QSettings settings(settingsFileName, QSettings::IniFormat);
    settings.setValue("OutputAudioDeviceId", ui->comboBox_audioDeviceOutput->currentData().toString());
    settings.setValue("ListenerVolumeThreshold", ui->doubleSpinBox_listenerVolumeThreshold->value());
    settings.setValue("PlaySoundInterval", ui->doubleSpinBox_playSoundInterval->value());
    settings.setValue("PlaySoundVolume", ui->doubleSpinBox_playSoundVolume->value());
    delete ui;
}
