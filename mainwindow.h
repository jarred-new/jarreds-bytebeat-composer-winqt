#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QCloseEvent>

#include <QMessageBox>
#include <QPushButton>
#include <QClipboard>

#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#include <QIODevice>

#include <QTimer>
#include <QString>

#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "bytebeatdevice.h"
#include "bytebeatformula.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // Close event
    void closeEvent(QCloseEvent *event) override;

    bool isNew;
    bool isOpen;
    bool isChanged;

protected slots:
    // file
    void newFile();
    bool openFile();
    bool saveFile();
    bool saveAsFile();

    // bytebeat
    void playBeat();
    void stopBeat();

private:
    Ui::MainWindow *ui;
    BytebeatFormula* formulaBackend;
    BytebeatDevice* audioDevice;
    QAudioSink* audioSink = nullptr;
};
#endif // MAINWINDOW_H
