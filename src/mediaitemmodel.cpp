#include "mediaitemmodel.h"

MediaItemModel::MediaItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MediaItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant MediaItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const auto item = m_items.at(index.row()).toObject();

    switch (role) {
    case ItemIdRole:
        return item.value(QStringLiteral("item_id")).toString();
    case NameRole:
    case Qt::DisplayRole:
        return item.value(QStringLiteral("name")).toString();
    case MediaTypeRole:
        return item.value(QStringLiteral("media_type")).toString();
    case ProviderRole:
        return item.value(QStringLiteral("provider")).toString();
    case UriRole:
        return item.value(QStringLiteral("uri")).toString();
    case ImageUrlRole:
        return extractImageUrl(item);
    case ArtistNameRole:
        return extractArtistName(item);
    case AlbumNameRole:
        return extractAlbumName(item);
    case DurationRole:
        return item.value(QStringLiteral("duration")).toInt(0);
    case TrackNumberRole:
        return item.value(QStringLiteral("track_number")).toInt(0);
    case YearRole:
        return item.value(QStringLiteral("year")).toInt(0);
    case FavoriteRole:
        return item.value(QStringLiteral("favorite")).toBool(false);
    case VersionRole:
        return item.value(QStringLiteral("version")).toString();
    case RawDataRole:
        return item;
    }

    return {};
}

QHash<int, QByteArray> MediaItemModel::roleNames() const
{
    return {
        {ItemIdRole, "itemId"},
        {NameRole, "name"},
        {MediaTypeRole, "mediaType"},
        {ProviderRole, "provider"},
        {UriRole, "uri"},
        {ImageUrlRole, "imageUrl"},
        {ArtistNameRole, "artistName"},
        {AlbumNameRole, "albumName"},
        {DurationRole, "duration"},
        {TrackNumberRole, "trackNumber"},
        {YearRole, "year"},
        {FavoriteRole, "favorite"},
        {VersionRole, "version"},
        {RawDataRole, "rawData"},
    };
}

void MediaItemModel::setItems(const QJsonArray &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
    Q_EMIT countChanged();
}

void MediaItemModel::appendItems(const QJsonArray &items)
{
    if (items.isEmpty()) return;
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size() + items.size() - 1);
    for (const auto &item : items)
        m_items.append(item);
    endInsertRows();
    Q_EMIT countChanged();
}

void MediaItemModel::clear()
{
    beginResetModel();
    m_items = QJsonArray();
    endResetModel();
    Q_EMIT countChanged();
}

QJsonObject MediaItemModel::getItem(int index) const
{
    if (index < 0 || index >= m_items.size())
        return {};
    return m_items.at(index).toObject();
}

QString MediaItemModel::extractImageUrl(const QJsonObject &item)
{
    // Returns "path|provider" for the image provider, or empty string
    auto extractFromImageObj = [](const QJsonObject &imgObj) -> QString {
        auto path = imgObj.value(QStringLiteral("path")).toString();
        if (path.isEmpty()) return {};
        auto provider = imgObj.value(QStringLiteral("provider")).toString();
        return path + QLatin1Char('|') + provider;
    };

    // Check metadata.images array — prefer thumb type
    auto metadata = item.value(QStringLiteral("metadata")).toObject();
    auto images = metadata.value(QStringLiteral("images")).toArray();
    for (const auto &img : images) {
        auto imgObj = img.toObject();
        auto type = imgObj.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("thumb") || type == QStringLiteral("0")) {
            return extractFromImageObj(imgObj);
        }
    }
    // Fallback to first image
    if (!images.isEmpty()) {
        return extractFromImageObj(images.first().toObject());
    }
    // Check image field directly (for ItemMapping / QueueItem)
    auto image = item.value(QStringLiteral("image")).toObject();
    if (!image.isEmpty()) {
        return extractFromImageObj(image);
    }
    return {};
}

QString MediaItemModel::extractArtistName(const QJsonObject &item)
{
    auto artists = item.value(QStringLiteral("artists")).toArray();
    if (artists.isEmpty()) return {};

    QStringList names;
    for (const auto &artist : artists) {
        auto obj = artist.toObject();
        auto name = obj.value(QStringLiteral("name")).toString();
        if (!name.isEmpty())
            names.append(name);
    }
    return names.join(QStringLiteral(", "));
}

QString MediaItemModel::extractAlbumName(const QJsonObject &item)
{
    auto album = item.value(QStringLiteral("album")).toObject();
    return album.value(QStringLiteral("name")).toString();
}
