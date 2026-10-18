#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
}

MainWindow::~MainWindow()
{
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
    // Setup your audio stream specifications
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::UInt8);

    audioDevice->open(QIODevice::ReadOnly);

    audioSink = new QAudioSink(format, this);
    audioSink->start(audioDevice);

    bool success = formulaBackend->setFormula(ui->plainTextEdit->toPlainText());
    if (!success) {
        // Show validation error toast/label in UI
    }

}

void MainWindow::stopBeat()
{
    if (audioDevice != nullptr) {
        audioDevice->close();
    }
    if (audioSink != nullptr) {
        audioSink->stop();
    }
}
