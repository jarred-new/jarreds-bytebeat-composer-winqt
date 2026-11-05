#ifndef LIBRARYMAN_H
#define LIBRARYMAN_H

#include <QCoreApplication>

#include <QFile>
#include <QObject>
#include <QString>
#include <QDir>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class LibraryManager : public QObject
{
    Q_OBJECT
public:
    explicit LibraryManager(QObject *parent = nullptr);
    bool loadLibrary();
    bool saveLibrary();
    QJsonArray libraryEntries() const;
    void setLibraryEntries(const QJsonArray &entries);

    void importLibraryFromFile(const QString &filePath);
    void exportLibraryToFile(const QString &filePath);

private:
    QFile m_libraryFile;
    QJsonDocument m_libraryDoc;
};

#endif // LIBRARYMAN_H
