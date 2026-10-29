#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSaveFile>
#include <QFileInfo>
#include <QSettings>
#include <QTextCursor>
#include <QUndoCommand>

namespace
{
class TextChangeCommand : public QUndoCommand
{
public:
    TextChangeCommand(QPlainTextEdit *editor, QString *previousText, bool *applyingUndoCommand,
                      bool *isChanged, int position, QString removedText, QString insertedText)
        : editor(editor), previousText(previousText), applyingUndoCommand(applyingUndoCommand),
          isChanged(isChanged), position(position), removedText(std::move(removedText)),
          insertedText(std::move(insertedText))
    {
        setText(removedText.isEmpty() ? "Insert text" : insertedText.isEmpty() ? "Delete text" : "Replace text");
    }

    void undo() override
    {
        apply(insertedText.size(), removedText);
    }

    void redo() override
    {
        if (firstRedo)
        {
            firstRedo = false;
            return;
        }
        apply(removedText.size(), insertedText);
    }

private:
    void apply(int removedLength, const QString &replacement)
    {
        *applyingUndoCommand = true;
        QTextCursor cursor(editor->document());
        cursor.setPosition(position);
        cursor.setPosition(position + removedLength, QTextCursor::KeepAnchor);
        cursor.insertText(replacement);
        *previousText = editor->toPlainText();
        *isChanged = true;
        *applyingUndoCommand = false;
    }

    QPlainTextEdit *editor;
    QString *previousText;
    bool *applyingUndoCommand;
    bool *isChanged;
    int position;
    QString removedText;
    QString insertedText;
    bool firstRedo = true;
};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    updateWindowTitle();
    ui->plainTextEdit->setPlainText("t*(42&t>>10)");
    previousEditorText = ui->plainTextEdit->toPlainText();

    /* Menu and QActions */
    // File
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveAsFile);
    connect(ui->actionExport_WAV, &QAction::triggered, this, &MainWindow::exportWav);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // Edit Menu
    connect(ui->actionCopy, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionCut, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionPaste, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::paste);
    undoStack = new QUndoStack(this);
    ui->plainTextEdit->document()->setUndoRedoEnabled(false);
    connect(ui->action_Undo, &QAction::triggered, undoStack, &QUndoStack::undo);
    connect(ui->action_Redo, &QAction::triggered, undoStack, &QUndoStack::redo);
    ui->action_Undo->setEnabled(false);
    ui->action_Redo->setEnabled(false);
    connect(undoStack, &QUndoStack::canUndoChanged, ui->action_Undo, &QAction::setEnabled);
    connect(undoStack, &QUndoStack::canRedoChanged, ui->action_Redo, &QAction::setEnabled);
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, [this]()
            {
                const QString currentText = ui->plainTextEdit->toPlainText();
                if (applyingUndoCommand)
                {
                    previousEditorText = currentText;
                    return;
                }

                int prefixLength = 0;
                const int shortestLength = qMin(previousEditorText.size(), currentText.size());
                while (prefixLength < shortestLength && previousEditorText[prefixLength] == currentText[prefixLength])
                    ++prefixLength;

                int oldSuffix = previousEditorText.size();
                int newSuffix = currentText.size();
                while (oldSuffix > prefixLength && newSuffix > prefixLength &&
                       previousEditorText[oldSuffix - 1] == currentText[newSuffix - 1])
                {
                    --oldSuffix;
                    --newSuffix;
                }

                const QString removedText = previousEditorText.mid(prefixLength, oldSuffix - prefixLength);
                const QString insertedText = currentText.mid(prefixLength, newSuffix - prefixLength);
                if (!removedText.isEmpty() || !insertedText.isEmpty())
                {
                    undoStack->push(new TextChangeCommand(ui->plainTextEdit, &previousEditorText,
                                                          &applyingUndoCommand, &isChanged,
                                                          prefixLength, removedText, insertedText));
                }
                previousEditorText = currentText;
                isChanged = true;
            });

    // View Menu
    connect(ui->actionZoom_In, &QAction::triggered, ui->plainTextEdit, [this]()
            {
                ui->plainTextEdit->zoomIn(2); });
    connect(ui->actionZoom_Out, &QAction::triggered, ui->plainTextEdit, [this]()
            { 
                ui->plainTextEdit->zoomOut(2); });

    // Themes
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "JarredApps", "JarredsBeatComposer");
    const QString theme = settings.value("theme", "tropicalParadise").toString();
    if (theme == "tropicalParadise")
    {
        ui->action_Tropical_Paradise->setChecked(true);
        ui->action_Spiced_Pumpkin->setChecked(false);
        ui->action_Royal_Sapphire->setChecked(false);
    }
    else if (theme == "spicedPumpkin")
    {
        ui->action_Tropical_Paradise->setChecked(false);
        ui->action_Spiced_Pumpkin->setChecked(true);
        ui->action_Royal_Sapphire->setChecked(false);
    }
    else if (theme == "royalSapphire")
    {
        ui->action_Tropical_Paradise->setChecked(false);
        ui->action_Spiced_Pumpkin->setChecked(false);
        ui->action_Royal_Sapphire->setChecked(true);
    }
    else
    {
        ui->action_Tropical_Paradise->setChecked(true);
        ui->action_Spiced_Pumpkin->setChecked(false);
        ui->action_Royal_Sapphire->setChecked(false);
    }
    connect(ui->action_Tropical_Paradise, &QAction::triggered, this, [this]()
            { 
                qApp->setPalette(AppThemePalettes::tropicalParadise()); 
                QSettings settings(QSettings::IniFormat, QSettings::UserScope, "JarredApps", "JarredsBeatComposer");
                settings.setValue("theme", "tropicalParadise");

                ui->statusbar->showMessage("Theme changed to Tropical Paradise", 2000);
                addLogs("Theme changed to Tropical Paradise");

                ui->action_Tropical_Paradise->setChecked(true);
                ui->action_Spiced_Pumpkin->setChecked(false);
                ui->action_Royal_Sapphire->setChecked(false); });
    connect(ui->action_Spiced_Pumpkin, &QAction::triggered, this, [this]()
            { 
                qApp->setPalette(AppThemePalettes::spicedPumpkin()); 
                QSettings settings(QSettings::IniFormat, QSettings::UserScope, "JarredApps", "JarredsBeatComposer");
                settings.setValue("theme", "spicedPumpkin");

                ui->statusbar->showMessage("Theme changed to Spiced Pumpkin", 2000);
                addLogs("Theme changed to Spiced Pumpkin");

                ui->action_Tropical_Paradise->setChecked(false);
                ui->action_Spiced_Pumpkin->setChecked(true);
                ui->action_Royal_Sapphire->setChecked(false); });
    connect(ui->action_Royal_Sapphire, &QAction::triggered, this, [this]()
            { 
                qApp->setPalette(AppThemePalettes::royalSapphire()); 
                QSettings settings(QSettings::IniFormat, QSettings::UserScope, "JarredApps", "JarredsBeatComposer");
                settings.setValue("theme", "royalSapphire");

                ui->statusbar->showMessage("Theme changed to Royal Sapphire", 2000);
                addLogs("Theme changed to Royal Sapphire");

                ui->action_Tropical_Paradise->setChecked(false);
                ui->action_Spiced_Pumpkin->setChecked(false);
                ui->action_Royal_Sapphire->setChecked(true); });

    // Bytebeat Menu
    connect(ui->action_Play, &QAction::triggered, this, &MainWindow::playBeat);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::stopBeat);
    connect(ui->actionSet_Bytebeat_Type, &QAction::triggered, this, &MainWindow::setType);
    connect(ui->actionSet_Frequency, &QAction::triggered, this, &MainWindow::setFrequency);

    // Tools Menu
    connect(ui->actionTxt_to_BB, &QAction::triggered, this, &MainWindow::convertTxtToBB);

    // Window Menu
    connect(ui->action_Library, &QAction::toggled, ui->formulasLibraryDock, &QDockWidget::setVisible);
    connect(ui->action_Output_Logs, &QAction::toggled, ui->outputLogs, &QDockWidget::setVisible);
    connect(ui->action_Undo_Logs, &QAction::toggled, ui->undoLogs, &QDockWidget::setVisible);

    connect(ui->formulasLibraryDock, &QDockWidget::visibilityChanged, ui->action_Library, &QAction::setChecked);
    connect(ui->outputLogs, &QDockWidget::visibilityChanged, ui->action_Output_Logs, &QAction::setChecked);
    connect(ui->undoLogs, &QDockWidget::visibilityChanged, ui->action_Undo_Logs, &QAction::setChecked);

    ui->undoView->setStack(undoStack);
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

    const QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    if (outputDevice.isNull())
    {
        ui->statusbar->showMessage("No audio output device is available", 5000);
        addLogs("No audio output device is available");
        return;
    }

    QAudioFormat format = outputDevice.preferredFormat();
    format.setSampleRate(sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    const bool requestedFormatSupported = outputDevice.isFormatSupported(format);
    if (!requestedFormatSupported)
    {
        format = outputDevice.preferredFormat();
    }

    audioDevice = new BytebeatDevice(this);
    QString formula = ui->plainTextEdit->toPlainText();

    if (!audioDevice->setFormula(formula))
    {
        delete audioDevice;
        audioDevice = nullptr;
        ui->statusbar->showMessage("Invalid bytebeat formula", 5000);
        addLogs("Invalid bytebeat formula");
        return;
    }

    audioDevice->setBytebeatType(bytebeatType);
    audioDevice->setAudioFormat(format);
    if (audioDevice->open(QIODevice::ReadOnly))
    {
        audioSink = new QAudioSink(outputDevice, format, this);
        audioSink->start(audioDevice);
        if (audioSink->error() != QAudio::NoError)
        {
            const QString error = QString::number(audioSink->error());
            stopBeat();
            ui->statusbar->showMessage("Could not start audio playback (error " + error + ")", 5000);
            addLogs("Could not start audio playback (error " + error + ")");
        }
        else if (!requestedFormatSupported)
        {
            ui->statusbar->showMessage("Playing with the output device's preferred format", 5000);
            addLogs("Playing with the output device's preferred format");
        }
        else
        {
            ui->statusbar->showMessage("Playing Bytebeat formula...", 2000);
            addLogs("Playing Bytebeat formula...");
        }
    }
    else
    {
        ui->statusbar->showMessage("Failed to open sound engine pipeline.", 5000);
        addLogs("Failed to open sound engine pipeline.");
        delete audioDevice;
        audioDevice = nullptr;
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

void MainWindow::exportWav()
{
    bool durationAccepted = false;
    const int durationSeconds = QInputDialog::getInt(
        this, "Export WAV", "Duration (seconds):", 30, 1, 3600, 1,
        &durationAccepted);
    if (!durationAccepted) {
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export WAV", "", "WAV audio (*.wav)");
    if (filePath.isEmpty()) {
        return;
    }
    if (!filePath.endsWith(".wav", Qt::CaseInsensitive)) {
        filePath += ".wav";
    }

    QString errorMessage;
    const QString formula = ui->plainTextEdit->toPlainText();
    const bool exported = bytebeatType == BYTEBEAT_SIGNED
        ? WavExporter::exportSigned(formula, filePath, sampleRate,
                                    durationSeconds, &errorMessage)
        : WavExporter::exportUnsigned(formula, filePath, sampleRate,
                                      durationSeconds, &errorMessage);
    if (!exported) {
        QMessageBox::warning(this, "WAV Export Failed", errorMessage);
        addLogs("WAV export failed: " + errorMessage);
        return;
    }

    ui->statusbar->showMessage("WAV exported successfully", 3000);
    addLogs("Exported WAV audio to " + filePath);
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

void MainWindow::setType()
{
    bool ok;

    QStringList typeLists;
    typeLists.append("Bytebeat");
    typeLists.append("Signed Bytebeat");

    QString strTypeDlg = QInputDialog::getItem(this,
                                               "Set Bytebeat Type",
                                               "", typeLists, bytebeatTypeIndex, false, &ok, Qt::Dialog);

    if (ok)
    {
        if (strTypeDlg == "Bytebeat")
        {
            bytebeatTypeIndex = 0;
            bytebeatType = BYTEBEAT_UNSIGNED;
            ui->statusbar->showMessage("Bytebeat type set to Bytebeat", 2000);
            addLogs("Bytebeat type set to Bytebeat");
        }
        else if (strTypeDlg == "Signed Bytebeat")
        {
            bytebeatTypeIndex = 1;
            bytebeatType = BYTEBEAT_SIGNED;
            ui->statusbar->showMessage("Bytebeat type set to Signed Bytebeat", 2000);
            addLogs("Bytebeat type set to Signed Bytebeat");
        }
        typeLists.clear(); // cleanup
    }
}

// tools
void MainWindow::convertTxtToBB()
{
    ConvertWizard wizard(this);
    wizard.exec();
}

// file menu actions
void MainWindow::newFile()
{
    if (!confirmDiscardChanges())
    {
        return;
    }

    ui->plainTextEdit->clear();
    undoStack->clear();
    currentFilePath.clear();
    updateWindowTitle();
    previousEditorText = ui->plainTextEdit->toPlainText();
    isChanged = false;
    ui->statusbar->showMessage("New file created", 2000);
    addLogs("New file created");
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
        addLogs("Could not open the selected file");
        return false;
    }

    if (fileName.endsWith(".bb", Qt::CaseInsensitive))
    {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        {
            ui->statusbar->showMessage("Could not parse the selected BB file", 5000);
            addLogs("Could not parse the selected BB file");
            return false;
        }

        QJsonObject json = doc.object();
        if (!json.contains("formula") || !json["formula"].isString())
        {
            ui->statusbar->showMessage("The selected BB file does not contain a valid formula", 5000);
            addLogs("The selected BB file does not contain a valid formula");
            return false;
        }
        ui->plainTextEdit->setPlainText(json["formula"].toString());

        if (json.contains("sampleRate") && json["sampleRate"].isDouble())
        {
            sampleRate = json["sampleRate"].toInt();
        }

        if (json.contains("bytebeatType") && json["bytebeatType"].isDouble())
        {
            bytebeatTypeIndex = json["bytebeatType"].toInt();
            bytebeatType = bytebeatTypeIndex == 0 ? BYTEBEAT_UNSIGNED : BYTEBEAT_SIGNED;
        }
    }
    else if (fileName.endsWith(".txt", Qt::CaseInsensitive))
    {
        QTextStream txtUtf(&file);
        txtUtf.setEncoding(QStringConverter::Utf16LE);

        ui->plainTextEdit->setPlainText(txtUtf.readAll());
    }
    currentFilePath = fileName;
    updateWindowTitle();
    undoStack->clear();
    previousEditorText = ui->plainTextEdit->toPlainText();
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
        addLogs("Could not save the selected file");
        return false;
    }

    QTextStream stream(&file);
    if (fileName.endsWith(".bb", Qt::CaseInsensitive))
    {
        // add all the settings
        QJsonObject json;
        json["formula"] = ui->plainTextEdit->toPlainText();
        json["sampleRate"] = sampleRate;
        json["bytebeatType"] = bytebeatTypeIndex;

        QJsonDocument doc(json);
        stream << doc.toJson();
        stream.flush();
        if (stream.status() != QTextStream::Ok || !file.commit())
        {
            ui->statusbar->showMessage("Could not save the selected file", 5000);
            addLogs("Could not save the selected file");
            return false;
        }
    }
    else if (fileName.endsWith(".txt", Qt::CaseInsensitive))
    {
        stream.setEncoding(QStringConverter::Utf16LE);
        stream << ui->plainTextEdit->toPlainText();
        stream.flush();
        if (stream.status() != QTextStream::Ok || !file.commit())
        {
            ui->statusbar->showMessage("Could not save the selected file", 5000);
            addLogs("Could not save the selected file");
            return false;
        }
    }

    currentFilePath = fileName;
    updateWindowTitle();
    isChanged = false;
    ui->statusbar->showMessage("Saved " + currentFilePath, 3000);
    addLogs("Saved " + currentFilePath);
    return true;
}

void MainWindow::updateWindowTitle()
{
    const QString fileName = currentFilePath.isEmpty()
        ? QStringLiteral("Untitled")
        : QFileInfo(currentFilePath).fileName();
    setWindowTitle(QStringLiteral("Jarred's Bytebeat Composer - %1").arg(fileName));
}

void MainWindow::addLogs(QString message)
{
    ui->listWidget->addItem(message);
}