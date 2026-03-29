#pragma once

#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QQmlEngine>

class QueueItemModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        QueueItemIdRole = Qt::UserRole + 1,
        NameRole,
        DurationRole,
        ImageUrlRole,
        ArtistNameRole,
        AlbumNameRole,
        IndexRole,
        AvailableRole,
    };
    Q_ENUM(Roles)

    explicit QueueItemModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QJsonArray &items);
    void clear();

    Q_INVOKABLE QString queueItemIdAt(int index) const;

Q_SIGNALS:
    void countChanged();

private:
    QJsonArray m_items;
};
