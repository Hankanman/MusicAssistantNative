#pragma once

#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QQmlEngine>

class MediaItemModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        ItemIdRole = Qt::UserRole + 1,
        NameRole,
        MediaTypeRole,
        ProviderRole,
        UriRole,
        ImageUrlRole,
        ArtistNameRole,
        AlbumNameRole,
        DurationRole,
        TrackNumberRole,
        YearRole,
        FavoriteRole,
        VersionRole,
        RawDataRole,
    };
    Q_ENUM(Roles)

    explicit MediaItemModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QJsonArray &items);
    void appendItems(const QJsonArray &items);
    void clear();

    Q_INVOKABLE QJsonObject getItem(int index) const;

Q_SIGNALS:
    void countChanged();

private:
    static QString extractImageUrl(const QJsonObject &item);
    static QString extractArtistName(const QJsonObject &item);
    static QString extractAlbumName(const QJsonObject &item);

    QJsonArray m_items;
};
