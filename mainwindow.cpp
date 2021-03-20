#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "log.h"
#include "audiodevice.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent, Qt::MSWindowsFixedSizeDialogHint), ui(new Ui::MainWindow)
{
    // UI

    ui->setupUi(this);

    for (auto&& device: AudioDevice::list()) {
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
        auto volume = audioPeakMeter->getPeakValue();
        ui->label_listenerVolume->setText(QString::number(volume, 'f'));
        if (volume > ui->doubleSpinBox_listenerVolumeThreshold->value()) {
//            FileDebug() << "peak volume " << volume;
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
    ui->doubleSpinBox_playSoundInterval->setValue(settings.value("PlaySoundInterval", ui->doubleSpinBox_playSoundInterval->value()).toDouble());
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
        playSoundTimer->start(0);
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

    QFileInfo fileInfo (outputSoundFileName);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "File not found", "File \"" + fileInfo.absoluteFilePath() + "\" not found.");
        return;
    }

    audioRenderer = new AudioRenderer(outputSoundFileName, ui->comboBox_audioDeviceOutput->currentData().toString(), this);
    connect(audioRenderer, &AudioRenderer::stateChanged, this, &MainWindow::onAudioRendererStateChanged);

    audioDeviceVolumeControl = new AudioDeviceVolumeControl(ui->comboBox_audioDeviceOutput->currentData().toString(), this);

    audioPeakMeter = new AudioPeakMeter(ui->comboBox_audioDeviceOutput->currentData().toString(), this);
    peakMeterTimer->start();

    started = true;
    FileDebug() << "Started";

    setTrayIcon();
    ui->pushButton_start->setText("Stop");
    ui->comboBox_audioDeviceOutput->setEnabled(false);

    playSound();
}

void MainWindow::stop()
{
    if (!started)
        return;

    started = false;
    FileDebug() << "Stopped";

    QOBJECT_SAFE_DELETE(audioRenderer);
    QOBJECT_SAFE_DELETE(audioDeviceVolumeControl);
    QOBJECT_SAFE_DELETE(audioPeakMeter);

    setTrayIcon();
    playSoundTimer->stop();
    peakMeterTimer->stop();
    ui->label_listenerVolume->clear();
    ui->pushButton_start->setText("Start");
    ui->comboBox_audioDeviceOutput->setEnabled(true);
}

void MainWindow::playSound()
{
    if (!audioRenderer)
        return;
    audioRenderer->play(ui->doubleSpinBox_playSoundVolume->value());
}

void MainWindow::showWindow()
{
    show();
    showNormal();
    activateWindow();
}

void MainWindow::onAudioRendererStateChanged(AudioRenderer::State state)
{
//    FileDebug() << "onAudioRendererStateChanged " << state;
    if (!audioDeviceVolumeControl)
        return;
    if (state == AudioRenderer::State::Playing) {
        prevAudioMasterVolume = audioDeviceVolumeControl->getMasterVolumeLevelScalar();
        audioDeviceVolumeControl->setMasterVolumeLevelScalar(1);
    } else if (state == AudioRenderer::State::Stopped) {
        if (prevAudioMasterVolume)
            audioDeviceVolumeControl->setMasterVolumeLevelScalar(*prevAudioMasterVolume);
        if (!playSoundTimer->isActive())
            playSoundTimer->start(0);
    }
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
    settings.setValue("PlaySoundFileName", outputSoundFileName);
    delete ui;
}
