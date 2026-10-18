#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->plainTextEdit->setPlainText("t");

    /* Bytebeat Device Creation */
    // Create the persistent ExprTk backend formula tracker
    formulaBackend = new BytebeatFormula();

    // Create the device feeding it the backend, and spin up the sink
    audioDevice = new BytebeatDevice(formulaBackend, this);

    /* Menu and QActions */
    // File
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // Edit Menu
    connect(ui->actionCopy, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionCut, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionPaste, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::paste);

    // Bytebeat Menu
    connect(ui->action_Play, &QAction::triggered, this, &MainWindow::playBeat);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::stopBeat);
}

MainWindow::~MainWindow()
{
    stopBeat();
    delete ui;
    delete formulaBackend;
    delete audioDevice;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    QMessageBox comfirmClose(this);

    comfirmClose.setWindowTitle("Quit?");
    comfirmClose.setText("Do you want to save your formula?");
    comfirmClose.setIcon(QMessageBox::Question);

    QPushButton *yesButton = comfirmClose.addButton(QMessageBox::Yes);
    QPushButton *noButton = comfirmClose.addButton(QMessageBox::No);
    QPushButton *cancelButton = comfirmClose.addButton(QMessageBox::Cancel);

    comfirmClose.setDefaultButton(yesButton);

    comfirmClose.exec();

    if (comfirmClose.clickedButton() == yesButton) {
        // TODO: save logic
    }
    else if (comfirmClose.clickedButton() == noButton) {
        event->accept();
    }
    else if (comfirmClose.clickedButton() == cancelButton) {
        event->ignore();
    }
}

void MainWindow::playBeat()
{
    if (audioSink != nullptr) {
        audioSink->stop();
        delete audioSink;
        audioSink = nullptr;
    }

    if (!formulaBackend->setFormula(ui->plainTextEdit->toPlainText())) {
        ui->statusbar->showMessage("Invalid bytebeat formula", 5000);
        return;
    }

    const QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    if (outputDevice.isNull()) {
        ui->statusbar->showMessage("No audio output device is available", 5000);
        return;
    }

    const QAudioFormat format = outputDevice.preferredFormat();
    audioDevice->setAudioFormat(format);
    if (!audioDevice->open(QIODevice::ReadOnly)) {
        ui->statusbar->showMessage("Could not open the bytebeat audio stream", 5000);
        return;
    }

    audioSink = new QAudioSink(outputDevice, format, this);
    connect(audioSink, &QAudioSink::stateChanged, this, [this](QAudio::State state) {
        if (state == QAudio::ActiveState) {
            ui->statusbar->showMessage("Playing", 2000);
        } else if (audioSink != nullptr && audioSink->error() != QAudio::NoError) {
            ui->statusbar->showMessage("Audio playback failed (error "
                                           + QString::number(audioSink->error()) + ")", 5000);
        }
    });
    audioSink->start(audioDevice);
    if (audioSink->error() != QAudio::NoError) {
        ui->statusbar->showMessage("Could not start audio playback", 5000);
    }

}

void MainWindow::stopBeat()
{
    if (audioDevice != nullptr) {
        audioDevice->close();
    }
    if (audioSink != nullptr) {
        audioSink->stop();
        delete audioSink;
        audioSink = nullptr;
    }
}
