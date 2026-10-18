#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QCloseEvent>

#include <QMessageBox>
#include <QPushButton>
#include <QClipboard>

#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>

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
    void closeEvent(QCloseEvent *event) override;

protected slots:
    void playBeat();
    void stopBeat();

private:
    Ui::MainWindow *ui;
    BytebeatFormula* formulaBackend;
    BytebeatDevice* audioDevice;
    QAudioSink* audioSink;
};
#endif // MAINWINDOW_H
