#include "librarycontroller.h"
#include "maclient.h"
#include "mediaitemmodel.h"

LibraryController::LibraryController(QObject *parent)
    : QObject(parent)
    , m_artistsModel(new MediaItemModel(this))
    , m_albumsModel(new MediaItemModel(this))
    , m_tracksModel(new MediaItemModel(this))
    , m_playlistsModel(new MediaItemModel(this))
    , m_radiosModel(new MediaItemModel(this))
    , m_searchResultsModel(new MediaItemModel(this))
{
}

void LibraryController::setClient(MaClient *client)
{
    m_client = client;
}

MediaItemModel *LibraryController::artistsModel() const { return m_artistsModel; }
MediaItemModel *LibraryController::albumsModel() const { return m_albumsModel; }
MediaItemModel *LibraryController::tracksModel() const { return m_tracksModel; }
MediaItemModel *LibraryController::playlistsModel() const { return m_playlistsModel; }
MediaItemModel *LibraryController::radiosModel() const { return m_radiosModel; }
MediaItemModel *LibraryController::searchResultsModel() const { return m_searchResultsModel; }

QString LibraryController::currentMediaType() const { return m_currentMediaType; }

void LibraryController::setCurrentMediaType(const QString &type)
{
    if (m_currentMediaType != type) {
        m_currentMediaType = type;
        Q_EMIT currentMediaTypeChanged();
    }
}

bool LibraryController::loading() const { return m_loading; }

void LibraryController::setLoading(bool loading)
{
    if (m_loading != loading) {
        m_loading = loading;
        Q_EMIT loadingChanged();
    }
}

void LibraryController::loadLibrary(const QString &mediaType, bool favoriteOnly)
{
    if (!m_client) return;

    setLoading(true);
    QString command = QStringLiteral("music/%1/library_items").arg(mediaType);

    QJsonObject args;
    if (favoriteOnly)
        args[QStringLiteral("favorite")] = true;
    args[QStringLiteral("limit")] = 500;
    args[QStringLiteral("offset")] = 0;

    MediaItemModel *targetModel = nullptr;
    if (mediaType == QStringLiteral("artists")) targetModel = m_artistsModel;
    else if (mediaType == QStringLiteral("albums")) targetModel = m_albumsModel;
    else if (mediaType == QStringLiteral("tracks")) targetModel = m_tracksModel;
    else if (mediaType == QStringLiteral("playlists")) targetModel = m_playlistsModel;
    else if (mediaType == QStringLiteral("radios")) targetModel = m_radiosModel;

    if (!targetModel) {
        setLoading(false);
        return;
    }

    m_client->sendCommand(command, args,
        [this, targetModel](const QJsonValue &result, const QString &error) {
            setLoading(false);
            if (error.isEmpty() && result.isArray()) {
                targetModel->setItems(result.toArray());
            }
        });
}

void LibraryController::search(const QString &query)
{
    if (!m_client || query.isEmpty()) return;

    setLoading(true);
    QJsonObject args;
    args[QStringLiteral("search_query")] = query;
    args[QStringLiteral("limit")] = 25;

    m_client->sendCommand(QStringLiteral("music/search"), args,
        [this](const QJsonValue &result, const QString &error) {
            setLoading(false);
            if (error.isEmpty() && result.isObject()) {
                auto obj = result.toObject();
                // Combine all results into a flat list
                QJsonArray combined;
                const auto types = {QStringLiteral("artists"), QStringLiteral("albums"),
                                   QStringLiteral("tracks"), QStringLiteral("playlists"),
                                   QStringLiteral("radios")};
                for (const auto &type : types) {
                    const auto arr = obj.value(type).toArray();
                    for (const auto &item : arr)
                        combined.append(item);
                }
                m_searchResultsModel->setItems(combined);
            }
        });
}

void LibraryController::loadAlbumTracks(const QString &itemId, const QString &provider)
{
    if (!m_client) return;

    QJsonObject args;
    args[QStringLiteral("item_id")] = itemId;
    args[QStringLiteral("provider_instance_id_or_domain")] = provider;

    m_client->sendCommand(QStringLiteral("music/albums/album_tracks"), args,
        [this](const QJsonValue &result, const QString &error) {
            if (error.isEmpty() && result.isArray()) {
                Q_EMIT albumTracksLoaded(result.toArray());
            }
        });
}

void LibraryController::loadArtistAlbums(const QString &itemId, const QString &provider)
{
    if (!m_client) return;

    QJsonObject args;
    args[QStringLiteral("item_id")] = itemId;
    args[QStringLiteral("provider_instance_id_or_domain")] = provider;

    m_client->sendCommand(QStringLiteral("music/artists/artist_albums"), args,
        [this](const QJsonValue &result, const QString &error) {
            if (error.isEmpty() && result.isArray()) {
                Q_EMIT artistAlbumsLoaded(result.toArray());
            }
        });
}

void LibraryController::addToFavorites(const QString &uri)
{
    if (!m_client) return;
    QJsonObject args;
    args[QStringLiteral("item")] = uri;
    m_client->sendCommand(QStringLiteral("music/favorites/add_item"), args);
}

void LibraryController::removeFromFavorites(const QString &mediaType, const QString &itemId)
{
    if (!m_client) return;
    QJsonObject args;
    args[QStringLiteral("media_type")] = mediaType;
    args[QStringLiteral("library_item_id")] = itemId;
    m_client->sendCommand(QStringLiteral("music/favorites/remove_item"), args);
}
