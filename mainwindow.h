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

#include <QInputDialog>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include "bytebeatgenerator.h"
#include "bytebeattype.h"

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

    bool isChanged = false;

    int sampleRate = 8000;

    BytebeatType bytebeatType = BYTEBEAT_UNSIGNED;

protected slots:
    // file
    void newFile();
    bool openFile();
    bool saveFile();
    bool saveAsFile();

    // bytebeat
    void playBeat();
    void stopBeat();
    void setFrequency();
    void setType();

private:
    bool confirmDiscardChanges();
    bool writeFile(const QString& fileName);

    Ui::MainWindow *ui;
    QString currentFilePath;
    BytebeatDevice* audioDevice = nullptr;
    QAudioSink* audioSink = nullptr;

};
#endif // MAINWINDOW_H
