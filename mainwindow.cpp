#include <QDebug>
#include <cmath>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    // UI

    ui->setupUi(this);

    QFile::remove("log.txt");

    {
        auto devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
        for (const auto& info: devices) {
            ui->comboBox_audioDeviceListener->addItem(info.deviceName());
        }
    }
    {
        auto devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
        for (const auto &info: devices) {
            ui->comboBox_audioDeviceOutput->addItem(info.deviceName());
        }
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

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MainWindow::playSound);

    QTimer* watcherTimer = new QTimer(this);
    watcherTimer->setSingleShot(false);
    connect(watcherTimer, &QTimer::timeout, this, [this]() {
        ui->label_timerTime->setText(QString::number(timer->remainingTime() / 1000));
    });
    watcherTimer->start(1000);

    // Settings read

    QSettings settings(settingsFileName, QSettings::IniFormat);

    outputSoundFileName = settings.value("OutputSoundFileName", "sound.wav").toString();

    inputAudioFormat.setSampleRate(settings.value("ListenerAudioFormat/SampleRate", 48000).toInt());
    inputAudioFormat.setSampleSize(settings.value("ListenerAudioFormat/SampleSize", 16).toInt());
    inputAudioFormat.setChannelCount(settings.value("ListenerAudioFormat/ChannelCount", 2).toInt());
    inputAudioFormat.setCodec("audio/pcm");
    inputAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    inputAudioFormat.setSampleType(QAudioFormat::SignedInt);

    ui->comboBox_audioDeviceListener->setCurrentText(settings.value("ListenerAudioDevice").toString());
    ui->comboBox_audioDeviceOutput->setCurrentText(settings.value("OutputAudioDevice").toString());
    ui->doubleSpinBox_listenerVolumeThreshold->setValue(settings.value("ListenerVolumeThreshold", ui->doubleSpinBox_listenerVolumeThreshold->value()).toDouble());
    ui->doubleSpinBox_timerDurationSeconds->setValue(settings.value("TimerDuration", ui->doubleSpinBox_timerDurationSeconds->value()).toDouble()); // 00:11:20
    ui->doubleSpinBox_volume->setValue(settings.value("OutputAudioVolume", ui->doubleSpinBox_volume->value()).toDouble());

    // Args check

    auto args = QCoreApplication::arguments();
    bool hide = false, start = false;
    for (auto arg: args) {
        arg = arg.mid(1);
        if (arg == "hide")
            hide = true;
        else if (arg == "start")
            start = true;
    }
    if (!hide)
        show();
    if (start)
        startListening();
}

void MainWindow::on_pushButton_start_clicked()
{
    if (!started) {
        startListening();
    } else {
        stopListening();
    }
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

void MainWindow::startListening()
{
    if (started)
        return;
    FileDebug() << "Started";
    auto adi = audioDeviceInfoByDeviceName(ui->comboBox_audioDeviceListener->currentText());
    if (!adi.isNull()) {

        ui->pushButton_start->setText("Stop");
        started = true;
        setTrayIcon();

        QAudioFormat format = inputAudioFormat;

        if (!format.isValid())
            format = adi.preferredFormat();
        if (!adi.isFormatSupported(format))
            format = adi.nearestFormat(format);

        audioInput = new QAudioInput(adi, format, this);

        connect(audioInput, &QAudioInput::stateChanged, this, [this](QAudio::State state) {
            QString str;
            QDebug(&str) << state;
            switch (state) {
            case QAudio::StoppedState:
                if (audioInput->error() != QAudio::NoError) {
                    FileDebug() << "audio input error" << audioInput->error();
                }
                stopListening();
                break;
            default: ;
            }
            ui->label_listenerState->setText(str);
        });

        audioInput->setBufferSize(100000);
        audioInputDevice = audioInput->start();

        connect(audioInputDevice, &QIODevice::readyRead, this, [this]() {
            auto bytes = audioInputDevice->readAll();
            if (bytes.size() == 0)
                return;
            static const auto max_sample_value = std::pow(2, audioInput->format().sampleSize());
            static const auto max_sample_value_div2 = max_sample_value / 2;
            static const auto bytes_per_sample = audioInput->format().sampleSize() / 8;
            bool hasSound = false;
            for (int n = 0; n <= bytes.size() - bytes_per_sample; n += bytes_per_sample) {
                int a = 0;
                for (int m = bytes_per_sample; m > 0; m--) {
                    a = ((bytes[n + m - 1] & 0xFF) << (m * 8 - 8)) | a;
                }
                if (a > max_sample_value_div2 - 1)
                    a -= max_sample_value;
                auto volume = std::abs(a / max_sample_value_div2);
                ui->label_listenerVolume->setText(QString::number(volume, 'f'));
                if (volume > ui->doubleSpinBox_listenerVolumeThreshold->value()) {
//                    FileDebug() << "sound detected:" << a << volume;
                    hasSound = true;
                    break;
                }
            }
            if (hasSound) {
                timer->start(ui->doubleSpinBox_timerDurationSeconds->value() * 1000);
            }
        });

        playSound();
    }
}

void MainWindow::stopListening()
{
    if (!started)
        return;
    FileDebug() << "Stopped";
    started = false;
    timer->stop();
    audioInput->stop();
    audioInput->deleteLater();
    audioInput = nullptr;
    audioInputDevice = nullptr;
    ui->pushButton_start->setText("Start");
    setTrayIcon();
}

void MainWindow::playSound()
{
    FileDebug() << "Playing sound" << outputSoundFileName;
    auto adi = audioDeviceInfoByDeviceName(ui->comboBox_audioDeviceOutput->currentText());
    if (!adi.isNull()) {
        QFile* file = new QFile(outputSoundFileName);
        if (file->open(QFile::ReadOnly)) {
            QAudioOutput* audio = new QAudioOutput(adi, adi.preferredFormat(), this);
            audio->setVolume(ui->doubleSpinBox_volume->value());
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
                    if (started && !timer->isActive())
                        timer->start(0);
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
    devices.append(QAudioDeviceInfo::availableDevices(QAudio::AudioInput));
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
    stopListening();
    QSettings settings(settingsFileName, QSettings::IniFormat);
    settings.setValue("ListenerAudioDevice", ui->comboBox_audioDeviceListener->currentText());
    settings.setValue("OutputAudioDevice", ui->comboBox_audioDeviceOutput->currentText());
    settings.setValue("ListenerVolumeThreshold", ui->doubleSpinBox_listenerVolumeThreshold->value());
    settings.setValue("TimerDuration", ui->doubleSpinBox_timerDurationSeconds->value());
    settings.setValue("OutputAudioVolume", ui->doubleSpinBox_volume->value());
    delete ui;
}
