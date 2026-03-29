#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlEngine>

#include "mediaitemmodel.h"

class MaClient;

class LibraryController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(MediaItemModel *artistsModel READ artistsModel CONSTANT)
    Q_PROPERTY(MediaItemModel *albumsModel READ albumsModel CONSTANT)
    Q_PROPERTY(MediaItemModel *tracksModel READ tracksModel CONSTANT)
    Q_PROPERTY(MediaItemModel *playlistsModel READ playlistsModel CONSTANT)
    Q_PROPERTY(MediaItemModel *radiosModel READ radiosModel CONSTANT)
    Q_PROPERTY(MediaItemModel *searchResultsModel READ searchResultsModel CONSTANT)
    Q_PROPERTY(QString currentMediaType READ currentMediaType WRITE setCurrentMediaType NOTIFY currentMediaTypeChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    explicit LibraryController(QObject *parent = nullptr);

    void setClient(MaClient *client);

    MediaItemModel *artistsModel() const;
    MediaItemModel *albumsModel() const;
    MediaItemModel *tracksModel() const;
    MediaItemModel *playlistsModel() const;
    MediaItemModel *radiosModel() const;
    MediaItemModel *searchResultsModel() const;

    QString currentMediaType() const;
    void setCurrentMediaType(const QString &type);
    bool loading() const;

    Q_INVOKABLE void loadLibrary(const QString &mediaType, bool favoriteOnly = false);
    Q_INVOKABLE void search(const QString &query);
    Q_INVOKABLE void loadAlbumTracks(const QString &itemId, const QString &provider);
    Q_INVOKABLE void loadArtistAlbums(const QString &itemId, const QString &provider);
    Q_INVOKABLE void addToFavorites(const QString &uri);
    Q_INVOKABLE void removeFromFavorites(const QString &mediaType, const QString &itemId);

Q_SIGNALS:
    void currentMediaTypeChanged();
    void loadingChanged();
    void albumTracksLoaded(const QJsonArray &tracks);
    void artistAlbumsLoaded(const QJsonArray &albums);

private:
    void setLoading(bool loading);

    MaClient *m_client = nullptr;
    MediaItemModel *m_artistsModel;
    MediaItemModel *m_albumsModel;
    MediaItemModel *m_tracksModel;
    MediaItemModel *m_playlistsModel;
    MediaItemModel *m_radiosModel;
    MediaItemModel *m_searchResultsModel;
    QString m_currentMediaType = QStringLiteral("artists");
    bool m_loading = false;
};
