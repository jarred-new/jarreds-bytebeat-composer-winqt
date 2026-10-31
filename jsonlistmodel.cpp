#include "jsonlistmodel.h"

JsonListModel::JsonListModel(QObject *parent)
    : QAbstractListModel(parent) {}

void JsonListModel::setJsonData(const QJsonArray &jsonArray, const QString &displayKey) {
    // Notify the view that the underlying model data is about to change
    beginResetModel();
    m_data = jsonArray;
    m_displayKey = displayKey;
    endResetModel();
}

int JsonListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_data.size();
}

QVariant JsonListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_data.size()) {
        return QVariant();
    }

    // Handle the standard text display role
    if (role == Qt::DisplayRole) {
        QJsonObject obj = m_data.at(index.row()).toObject();
        return obj.value(m_displayKey).toVariant();
    }

    // Optional: Return a raw QVariantMap/QJsonObject if you use custom delegates
    if (role == Qt::UserRole) {
        return m_data.at(index.row()).toObject().toVariantMap();
    }

    return QVariant();
}
