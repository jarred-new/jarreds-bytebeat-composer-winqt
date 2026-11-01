#include "librarymanager.h"

#include <QDebug>
#include <QSaveFile>

LibraryManager::LibraryManager(QObject *parent)
    : QObject(parent),
      m_libraryFile(QDir(QCoreApplication::applicationDirPath()).filePath("library.json"))
{
    qDebug() << "Library file path:" << m_libraryFile.fileName();
}

bool LibraryManager::loadLibrary()
{
    if (!m_libraryFile.exists())
    {
        QFile builtInLibrary(":/libraries.json");
        if (!builtInLibrary.open(QIODevice::ReadOnly))
        {
            qWarning() << "Could not open the built-in formula library:" << builtInLibrary.errorString();
            return false;
        }

        QSaveFile newLibrary(m_libraryFile.fileName());
        if (!newLibrary.open(QIODevice::WriteOnly) || newLibrary.write(builtInLibrary.readAll()) < 0 || !newLibrary.commit())
        {
            qWarning() << "Could not create the formula library:" << newLibrary.errorString();
            return false;
        }
    }

    if (!m_libraryFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not read the formula library:" << m_libraryFile.errorString();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(m_libraryFile.readAll(), &parseError);
    m_libraryFile.close();
    if (parseError.error != QJsonParseError::NoError || !document.isArray())
    {
        qWarning() << "Could not parse the formula library:" << parseError.errorString();
        return false;
    }

    m_libraryDoc = document;
    return true;
}

bool LibraryManager::saveLibrary()
{
    if (!m_libraryDoc.isArray())
    {
        qWarning() << "No formula library is loaded.";
        return false;
    }

    QSaveFile outputFile(m_libraryFile.fileName());
    if (!outputFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Could not save the formula library:" << outputFile.errorString();
        return false;
    }

    if (outputFile.write(m_libraryDoc.toJson(QJsonDocument::Indented)) < 0 || !outputFile.commit())
    {
        qWarning() << "Could not save the formula library:" << outputFile.errorString();
        return false;
    }
    return true;
}

QJsonArray LibraryManager::libraryEntries() const
{
    return m_libraryDoc.array();
}

void LibraryManager::setLibraryEntries(const QJsonArray &entries)
{
    m_libraryDoc = QJsonDocument(entries);
}

