#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSaveFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->plainTextEdit->setPlainText("t");

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
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, [this]()
            { isChanged = true; });

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
}

// close event
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!isChanged)
    {
        event->accept();
        return;
    }

    event->ignore();
    QMessageBox comfirmClose(this);

    comfirmClose.setWindowTitle("Quit?");
    comfirmClose.setText("Save changes to your formula before closing?");
    comfirmClose.setIcon(QMessageBox::Question);

    QPushButton *yesButton = comfirmClose.addButton(QMessageBox::Yes);
    QPushButton *noButton = comfirmClose.addButton(QMessageBox::No);
    QPushButton *cancelButton = comfirmClose.addButton(QMessageBox::Cancel);

    comfirmClose.setDefaultButton(yesButton);

    comfirmClose.exec();

    if (comfirmClose.clickedButton() == yesButton)
    {
        if (saveFile())
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else if (comfirmClose.clickedButton() == noButton)
    {
        event->accept();
    }
    else if (comfirmClose.clickedButton() == cancelButton)
    {
        event->ignore();
    }
}

// play/stop bytebeat
void MainWindow::playBeat()
{
    stopBeat();

    // const QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    // if (outputDevice.isNull())
    // {
    //     ui->statusbar->showMessage("No audio output device is available", 5000);
    //     return;
    // }

    // QAudioFormat format = outputDevice.preferredFormat();
    // format.setSampleRate(sampleRate);
    // const bool requestedFormatSupported = outputDevice.isFormatSupported(format);
    // if (!requestedFormatSupported)
    // {
    //     format = outputDevice.preferredFormat();
    // }

    // audioDevice = new BytebeatGenerator(this);
    // if (!audioDevice->setFormula(ui->plainTextEdit->toPlainText()))
    // {
    //     delete audioDevice;
    //     audioDevice = nullptr;
    //     ui->statusbar->showMessage("Invalid bytebeat formula", 5000);
    //     return;
    // }

    // audioDevice->setAudioFormat(format);
    // if (!audioDevice->open(QIODevice::ReadOnly))
    // {
    //     ui->statusbar->showMessage("Could not open the bytebeat audio stream", 5000);
    //     delete audioDevice;
    //     audioDevice = nullptr;
    //     return;
    // }

    // audioSink = new QAudioSink(outputDevice, format, this);
    // connect(audioSink, &QAudioSink::stateChanged, this, [this](QAudio::State state)
    //         {
    //     if (state == QAudio::ActiveState) {
    //         ui->statusbar->showMessage("Playing", 2000);
    //     } else if (audioSink != nullptr && audioSink->error() != QAudio::NoError) {
    //         ui->statusbar->showMessage("Audio playback failed (error "
    //                                        + QString::number(audioSink->error()) + ")", 5000);
    //     } });
    // audioSink->start(audioDevice);
    // if (audioSink->error() != QAudio::NoError)
    // {
    //     ui->statusbar->showMessage("Could not start audio playback (error " + QString::number(audioSink->error()) + ")", 5000);
    // }
    // else if (!requestedFormatSupported)
    // {
    //     ui->statusbar->showMessage("Requested format unavailable; playing at " + QString::number(format.sampleRate()) + " Hz", 5000);
    // }

    QAudioFormat format;
    format.setSampleRate(sampleRate); // Traditional bytebeat clock rate
    format.setChannelCount(1);  // Mono
    format.setSampleFormat(QAudioFormat::UInt8); // Unsigned 8-bit PCM

    audioDevice = new BytebeatDevice(&a);
    
    // Note: ExprTk treats bitwise operations out of the box
    QString formula = ui->plainTextEdit->toPlainText();
    
    if (!audioDevice->setFormula(formula)) {
        ui->statusbar->showMessage("Invalid bytebeat formula", 5000);
        return -1;
    }

    audioSink = new QAudioSink(format, &a);
    
    // Open device in read-only and fire the engine
    if (audioDevice->open(QIODevice::ReadOnly)) {
        audioSink->start(audioDevice);
        ui->statusbar->showMessage("Playing Bytebeat formula...", 2000);
    } else {
        ui->statusbar->showMessage("Failed to open sound engine pipeline.", 5000);
    }
}

void MainWindow::stopBeat()
{
    if (audioSink != nullptr)
    {
        audioSink->stop();
        delete audioSink;
        audioSink = nullptr;
    }
    if (audioDevice != nullptr)
    {
        audioDevice->close();
        delete audioDevice;
        audioDevice = nullptr;
    }
}

void MainWindow::setFrequency()
{
    bool ok;
    int inputFreq = QInputDialog::getInt(this,
                                         "Set Frequency",
                                         "example: 8000, 11025, 16000, 22050, 32000, etc.",
                                         sampleRate, 8000, 192000, 100, &ok, Qt::Dialog);

    if (ok)
    {
        this->sampleRate = inputFreq;
    }
}

// file menu actions
void MainWindow::newFile()
{
    if (!confirmDiscardChanges())
    {
        return;
    }

    ui->plainTextEdit->clear();
    currentFilePath.clear();
    isChanged = false;
    ui->statusbar->showMessage("New file created", 2000);
}

bool MainWindow::openFile()
{
    if (!confirmDiscardChanges())
    {
        return false;
    }

    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Bytebeat Formulas (*.bb *.txt);;All Files (*)");
    if (fileName.isEmpty())
    {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->statusbar->showMessage("Could not open the selected file", 5000);
        return false;
    }

    if (fileName.endsWith(".json", Qt::CaseInsensitive))
    {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        {
            ui->statusbar->showMessage("Could not parse the selected JSON file", 5000);
            return false;
        }

        QJsonObject json = doc.object();
        if (!json.contains("formula") || !json["formula"].isString())
        {
            ui->statusbar->showMessage("The selected JSON file does not contain a valid formula", 5000);
            return false;
        }

        ui->plainTextEdit->setPlainText(json["formula"].toString());
        if (json.contains("sampleRate") && json["sampleRate"].isDouble())
        {
            sampleRate = json["sampleRate"].toInt();
        }
    }
    else
    {
        ui->plainTextEdit->setPlainText(file.readAll());
    }
    currentFilePath = fileName;
    isChanged = false;
    return true;
}

bool MainWindow::saveFile()
{
    if (currentFilePath.isEmpty())
    {
        return saveAsFile();
    }

    return writeFile(currentFilePath);
}

bool MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save As...", "", "Bytebeat Formulas (*.bb *.txt);;All Files (*)");
    if (fileName.isEmpty())
    {
        return false;
    }

    return writeFile(fileName);
}

bool MainWindow::confirmDiscardChanges()
{
    if (!isChanged)
    {
        return true;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Unsaved changes",
        "Save changes before continuing?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes);

    if (answer == QMessageBox::Yes)
    {
        return saveFile();
    }
    return answer == QMessageBox::No;
}

bool MainWindow::writeFile(const QString &fileName)
{
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        ui->statusbar->showMessage("Could not save the selected file", 5000);
        return false;
    }

    QTextStream stream(&file);
    if (fileName.endsWith(".json", Qt::CaseInsensitive))
    {
        // add all the settings
        QJSONObject json;
        json["formula"] = ui->plainTextEdit->toPlainText();
        json["sampleRate"] = sampleRate;

        QJsonDocument doc(json);
        stream << doc.toJson();
        stream.flush();
        if (stream.status() != QTextStream::Ok || !file.commit())
        {
            ui->statusbar->showMessage("Could not save the selected file", 5000);
            return false;
        }
    }
    else
    {
        stream << ui->plainTextEdit->toPlainText();
        stream.flush();
        if (stream.status() != QTextStream::Ok || !file.commit())
        {
            ui->statusbar->showMessage("Could not save the selected file", 5000);
            return false;
        }
    }

    currentFilePath = fileName;
    isChanged = false;
    ui->statusbar->showMessage("Saved " + currentFilePath, 3000);
    return true;
}