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
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveAsFile);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // Edit Menu
    connect(ui->actionCopy, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionCut, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionPaste, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::paste);

    // Bytebeat Menu
    connect(ui->action_Play, &QAction::triggered, this, &MainWindow::playBeat);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::stopBeat);
    connect(ui->actionSet_Frequency, &QAction::triggered, this, &MainWindow::setFrequency);
}

// cleanup event
MainWindow::~MainWindow()
{
    stopBeat();
    delete ui;
    delete formulaBackend;
    delete audioDevice;
}

// close event
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
        if (saveFile()) {
            event->accept();
        }
        else {
            event->ignore();
        }
    }
    else if (comfirmClose.clickedButton() == noButton) {
        event->accept();
    }
    else if (comfirmClose.clickedButton() == cancelButton) {
        event->ignore();
    }
}

// play/stop bytebeat
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

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::UInt8);

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

void MainWindow::setFrequency()
{
    bool ok;
    int inputFreq = QInputDialog::getInt(this,
                         "Set Frequency",
                         "example: 8000, 11025, 16000, 22050, 32000, etc.",
                         sampleRate, 0, 999999, 10, &ok, Qt::Dialog);

    if (ok) {
        this->sampleRate = inputFreq;
    }
}

// file menu actions
void MainWindow::newFile()
{
    isNew = true;
    isOpen = false;
    isChanged = false;
    ui->plainTextEdit->clear();
    ui->statusbar->showMessage("New file created", 2000);
}

bool MainWindow::openFile()
{
    isNew = false;
    isOpen = true;
    isChanged = false;

    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "JSON Files (*.json);;Plain Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->plainTextEdit->setPlainText(file.readAll());
            file.close();
        }
    }
    return true;
}

bool MainWindow::saveFile()
{
    if (isNew) {
        return saveAsFile();
    } else {
        QString fileName = ui->statusbar->currentMessage();
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->plainTextEdit->toPlainText();
            file.close();
        }
    }

    return true;
}

bool MainWindow::saveAsFile()
{
    isNew = false;
    isOpen = true;
    isChanged = false;

    QString fileName = QFileDialog::getSaveFileName(this, "Save As...", "", "JSON Files (*.json);;Plain Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->plainTextEdit->toPlainText();
            file.close();
        }
    }
    else {
        return false; // User canceled the save dialog
    }
    return true;
}