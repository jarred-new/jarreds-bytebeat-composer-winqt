#ifndef CONVERT_WIZARD_H
#define CONVERT_WIZARD_H

#include <QWizard>
#include <QWizardPage>

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QIntValidator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSaveFile>
#include <QTextStream>
#include <QStringConverter>

// Page 1: Welcome Page
class WelcomePage : public QWizardPage
{
    Q_OBJECT
public:
    WelcomePage(QWidget *parent = nullptr) : QWizardPage(parent)
    {
        setTitle("Welcome to the Convert Wizard");
        QLabel *label = new QLabel("This wizard will help you convert your files.");
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        setLayout(layout);
    }
};

// Page 2: File Selection Page
class FileSelectionPage : public QWizardPage
{
    Q_OBJECT
public:
    FileSelectionPage(QWidget *parent = nullptr) : QWizardPage(parent)
    {
        setTitle("Select File to Convert:");
        QLabel *label = new QLabel("Choose the file you want to convert:");
        fileLineEdit = new QLineEdit;
        QPushButton *browseButton = new QPushButton("Browse...");

        connect(browseButton, &QPushButton::clicked, this, &FileSelectionPage::browseFile);
        registerField("fileSelectionPage.selectedFile*", fileLineEdit);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        layout->addWidget(fileLineEdit);
        layout->addWidget(browseButton);
        setLayout(layout);
    }

    QString selectedFile() const { return fileLineEdit->text(); }

private slots:
    void browseFile()
    {
        const QString fileName = QFileDialog::getOpenFileName(this, "Select File to Convert", QString(),
                                                              "Text Files (*.txt);;All Files (*)");
        if (!fileName.isEmpty())
            fileLineEdit->setText(fileName);
    }

private:
    QLineEdit *fileLineEdit;
};

// Page 3: Conversion Options Page (where to put Bytebeat type and sample rate)
class ConversionOptionsPage : public QWizardPage
{
    Q_OBJECT

public:
    ConversionOptionsPage(QWidget *parent = nullptr) : QWizardPage(parent)
    {
        setTitle("Conversion Options");
        QLabel *typeLabel = new QLabel("Select Bytebeat Type:");
        typeComboBox = new QComboBox;
        typeComboBox->addItem("Bytebeat");
        typeComboBox->addItem("Signed Bytebeat");

        QLabel *sampleRateLabel = new QLabel("Set Sample Rate:");
        sampleRateLineEdit = new QLineEdit;
        sampleRateLineEdit->setText("8000");
        sampleRateLineEdit->setValidator(new QIntValidator(8000, 192000, sampleRateLineEdit));
        sampleRateLineEdit->setPlaceholderText("e.g., 8000, 11025, 16000, 22050, 32000");
        registerField("conversionOptionsPage.selectedType", typeComboBox);
        registerField("conversionOptionsPage.sampleRate*", sampleRateLineEdit);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(typeLabel);
        layout->addWidget(typeComboBox);
        layout->addWidget(sampleRateLabel);
        layout->addWidget(sampleRateLineEdit);
        setLayout(layout);
    }

    QString selectedType() const { return typeComboBox->currentText(); }
    QString sampleRate() const { return sampleRateLineEdit->text(); }

private:
    QComboBox *typeComboBox;
    QLineEdit *sampleRateLineEdit;
};

// Page 4: Summary Page
class SummaryPage : public QWizardPage
{
    Q_OBJECT
public:
    SummaryPage(QWidget *parent = nullptr) : QWizardPage(parent)
    {
        setTitle("Summary");
        summaryLabel = new QLabel;
        summaryLabel->setWordWrap(true);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(summaryLabel);
        setLayout(layout);
    }

    void initializePage() override
    {
        QString file = field("fileSelectionPage.selectedFile").toString();
        QString type = field("conversionOptionsPage.selectedType").toString();
        QString sampleRate = field("conversionOptionsPage.sampleRate").toString();

        summaryLabel->setText(QString("You are about to convert the file:\n%1\n\n"
                                      "Conversion Type: %2\n"
                                      "Sample Rate: %3")
                                  .arg(file)
                                  .arg(type)
                                  .arg(sampleRate));
    }

private:
    QLabel *summaryLabel;
};

// Page 5: Output Page (where to save the output file)
class OutputPage : public QWizardPage
{
    Q_OBJECT

public:
    OutputPage(QWidget *parent = nullptr) : QWizardPage(parent)
    {
        setTitle("Set Conversion Output:");
        QLabel *label = new QLabel("Choose the output file name and location:");
        fileLineEdit = new QLineEdit;
        QPushButton *browseButton = new QPushButton("Browse...");
        connect(browseButton, &QPushButton::clicked, this, &OutputPage::browseFile);
        registerField("outputFile*", fileLineEdit);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        layout->addWidget(fileLineEdit);
        layout->addWidget(browseButton);
        setLayout(layout);
    }

private slots:
    void browseFile()
    {
        const QString fileName = QFileDialog::getSaveFileName(this, "Select Output File", QString(),
                                                              "Bytebeat Files (*.bb);;All Files (*)");
        if (!fileName.isEmpty())
            fileLineEdit->setText(fileName);
    }

private:
    QLineEdit *fileLineEdit;
};

// Main Convert Wizard
class ConvertWizard : public QWizard
{
    Q_OBJECT
public:
    QString selectedFile;

    ConvertWizard(QWidget *parent = nullptr) : QWizard(parent)
    {
        setWindowTitle("Convert txt to bb Wizard");
        setWizardStyle(QWizard::ModernStyle);
        setPalette(parent ? parent->palette() : QApplication::palette());

        addPage(new WelcomePage);
        addPage(new FileSelectionPage);
        addPage(new ConversionOptionsPage);
        addPage(new SummaryPage);
        addPage(new OutputPage);

        setStartId(0);
    }

    void accept() override
    {
        const QString inputPath = field("fileSelectionPage.selectedFile").toString();
        QString outputPath = field("outputFile").toString().trimmed();
        bool sampleRateOk = false;
        const int sampleRate = field("conversionOptionsPage.sampleRate").toInt(&sampleRateOk);
        const int bytebeatType = field("conversionOptionsPage.selectedType").toString() == "Signed Bytebeat" ? 1 : 0;

        if (inputPath.isEmpty() || outputPath.isEmpty() || !sampleRateOk || sampleRate < 8000 || sampleRate > 192000)
        {
            QMessageBox::critical(this, "Conversion failed", "Select valid input and output files and a sample rate from 8000 to 192000 Hz.");
            return;
        }

        if (!outputPath.endsWith(".bb", Qt::CaseInsensitive))
            outputPath += ".bb";

        QFile source(inputPath);
        if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, "Conversion failed", "The selected input file could not be opened.");
            return;
        }

        QTextStream input(&source);
        input.setEncoding(QStringConverter::Utf16LE);
        const QString formula = input.readAll();
        if (input.status() != QTextStream::Ok || formula.trimmed().isEmpty())
        {
            QMessageBox::critical(this, "Conversion failed", "The input file is empty or could not be read.");
            return;
        }

        QJsonObject data;
        data["formula"] = formula;
        data["sampleRate"] = sampleRate;
        data["bytebeatType"] = bytebeatType;

        QSaveFile output(outputPath);
        const QByteArray json = QJsonDocument(data).toJson();
        if (!output.open(QIODevice::WriteOnly) || output.write(json) != json.size() || !output.commit())
        {
            QMessageBox::critical(this, "Conversion failed", "The converted file could not be saved.");
            return;
        }

        QWizard::accept();
    }
};

#endif // CONVERT_WIZARD_H