#include "queueitemmodel.h"

QueueItemModel::QueueItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int QueueItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant QueueItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const auto item = m_items.at(index.row()).toObject();

    switch (role) {
    case QueueItemIdRole:
        return item.value(QStringLiteral("queue_item_id")).toString();
    case NameRole:
    case Qt::DisplayRole:
        return item.value(QStringLiteral("name")).toString();
    case DurationRole:
        return item.value(QStringLiteral("duration")).toInt(0);
    case ImageUrlRole: {
        auto image = item.value(QStringLiteral("image")).toObject();
        return image.value(QStringLiteral("path")).toString();
    }
    case ArtistNameRole: {
        auto mediaItem = item.value(QStringLiteral("media_item")).toObject();
        auto artists = mediaItem.value(QStringLiteral("artists")).toArray();
        if (!artists.isEmpty())
            return artists.first().toObject().value(QStringLiteral("name")).toString();
        return {};
    }
    case AlbumNameRole: {
        auto mediaItem = item.value(QStringLiteral("media_item")).toObject();
        auto album = mediaItem.value(QStringLiteral("album")).toObject();
        return album.value(QStringLiteral("name")).toString();
    }
    case IndexRole:
        return item.value(QStringLiteral("index")).toInt(index.row());
    case AvailableRole:
        return item.value(QStringLiteral("available")).toBool(true);
    }

    return {};
}

QHash<int, QByteArray> QueueItemModel::roleNames() const
{
    return {
        {QueueItemIdRole, "queueItemId"},
        {NameRole, "name"},
        {DurationRole, "duration"},
        {ImageUrlRole, "imageUrl"},
        {ArtistNameRole, "artistName"},
        {AlbumNameRole, "albumName"},
        {IndexRole, "itemIndex"},
        {AvailableRole, "available"},
    };
}

void QueueItemModel::setItems(const QJsonArray &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
    Q_EMIT countChanged();
}

void QueueItemModel::clear()
{
    beginResetModel();
    m_items = QJsonArray();
    endResetModel();
    Q_EMIT countChanged();
}

QString QueueItemModel::queueItemIdAt(int index) const
{
    if (index < 0 || index >= m_items.size()) return {};
    return m_items.at(index).toObject().value(QStringLiteral("queue_item_id")).toString();
}
