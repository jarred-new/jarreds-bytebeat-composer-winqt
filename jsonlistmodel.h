#ifndef JSONLISTMOD_H
#define JSONLISTMOD_H

#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonObject>

class JsonListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit JsonListModel(QObject *parent = nullptr);

    // Load data into the model
    void setJsonData(const QJsonArray &jsonArray, const QString &displayKey);

    // Required overrides for QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QJsonArray m_data;
    QString m_displayKey; // The JSON key to show in the list view
};

#endif JSONLISTMOD_H