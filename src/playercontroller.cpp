#include "playercontroller.h"
#include "maclient.h"
#include <QDateTime>

PlayerController::PlayerController(QObject *parent)
    : QObject(parent)
    , m_interpolateTimer(new QTimer(this))
{
    // Tick every second to update the interpolated elapsed time
    m_interpolateTimer->setInterval(1000);
    connect(m_interpolateTimer, &QTimer::timeout, this, [this]() {
        if (isPlaying()) Q_EMIT elapsedChanged();
    });
}

void PlayerController::setClient(MaClient *client)
{
    m_client = client;
    connect(m_client, &MaClient::eventReceived, this, &PlayerController::onEvent);
    connect(m_client, &MaClient::authenticatedChanged, this, [this]() {
        if (m_client->isAuthenticated()) {
            fetchPlayerState();
        }
    });
}

QString PlayerController::currentPlayerId() const { return m_currentPlayerId; }

void PlayerController::setCurrentPlayerId(const QString &id)
{
    if (m_currentPlayerId != id) {
        m_currentPlayerId = id;
        Q_EMIT currentPlayerIdChanged();
        fetchPlayerState();
    }
}

QString PlayerController::playerName() const
{
    return m_playerState.value(QStringLiteral("name")).toString();
}

QString PlayerController::playbackState() const
{
    return m_playerState.value(QStringLiteral("playback_state")).toString(QStringLiteral("idle"));
}

int PlayerController::volumeLevel() const
{
    return m_playerState.value(QStringLiteral("volume_level")).toInt(0);
}

bool PlayerController::volumeMuted() const
{
    return m_playerState.value(QStringLiteral("volume_muted")).toBool(false);
}

bool PlayerController::powered() const
{
    return m_playerState.value(QStringLiteral("powered")).toBool(false);
}

QString PlayerController::currentTrackTitle() const
{
    auto media = m_playerState.value(QStringLiteral("current_media")).toObject();
    return media.value(QStringLiteral("title")).toString();
}

QString PlayerController::currentTrackArtist() const
{
    auto media = m_playerState.value(QStringLiteral("current_media")).toObject();
    return media.value(QStringLiteral("artist")).toString();
}

QString PlayerController::currentTrackAlbum() const
{
    auto media = m_playerState.value(QStringLiteral("current_media")).toObject();
    return media.value(QStringLiteral("album")).toString();
}

QString PlayerController::currentTrackImageUrl() const
{
    auto media = m_playerState.value(QStringLiteral("current_media")).toObject();
    return media.value(QStringLiteral("image_url")).toString();
}

int PlayerController::elapsed() const
{
    // Interpolate from last server update if playing
    if (isPlaying() && m_serverElapsedAt > 0) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qreal delta = (now - m_serverElapsedAt) / 1000.0;
        return qBound(0, static_cast<int>(m_serverElapsed + delta), m_duration > 0 ? m_duration : 999999);
    }
    return static_cast<int>(m_serverElapsed);
}

int PlayerController::duration() const
{
    if (m_duration > 0) return m_duration;
    auto media = m_playerState.value(QStringLiteral("current_media")).toObject();
    return media.value(QStringLiteral("duration")).toInt(0);
}

bool PlayerController::isPlaying() const
{
    return playbackState() == QStringLiteral("playing");
}

void PlayerController::play() { sendPlayerCommand(QStringLiteral("players/cmd/play")); }
void PlayerController::pause() { sendPlayerCommand(QStringLiteral("players/cmd/pause")); }
void PlayerController::playPause() { sendPlayerCommand(QStringLiteral("players/cmd/play_pause")); }
void PlayerController::stop() { sendPlayerCommand(QStringLiteral("players/cmd/stop")); }
void PlayerController::next() { sendPlayerCommand(QStringLiteral("players/cmd/next")); }
void PlayerController::previous() { sendPlayerCommand(QStringLiteral("players/cmd/previous")); }

void PlayerController::seek(int position)
{
    m_serverElapsed = position;
    m_serverElapsedAt = QDateTime::currentMSecsSinceEpoch();
    Q_EMIT elapsedChanged();
    sendPlayerCommand(QStringLiteral("players/cmd/seek"), {{QStringLiteral("position"), position}});
}

void PlayerController::setVolume(int level)
{
    sendPlayerCommand(QStringLiteral("players/cmd/volume_set"), {{QStringLiteral("volume_level"), level}});
}

void PlayerController::volumeUp() { sendPlayerCommand(QStringLiteral("players/cmd/volume_up")); }
void PlayerController::volumeDown() { sendPlayerCommand(QStringLiteral("players/cmd/volume_down")); }

void PlayerController::toggleMute()
{
    sendPlayerCommand(QStringLiteral("players/cmd/volume_mute"),
                      {{QStringLiteral("muted"), !volumeMuted()}});
}

void PlayerController::setPower(bool on)
{
    sendPlayerCommand(QStringLiteral("players/cmd/power"), {{QStringLiteral("powered"), on}});
}

void PlayerController::sendPlayerCommand(const QString &cmd, const QJsonObject &extraArgs)
{
    if (!m_client || m_currentPlayerId.isEmpty()) return;

    QJsonObject args;
    args[QStringLiteral("player_id")] = m_currentPlayerId;
    for (auto it = extraArgs.begin(); it != extraArgs.end(); ++it) {
        args.insert(it.key(), it.value());
    }
    m_client->sendCommand(cmd, args);
}

void PlayerController::fetchPlayerState()
{
    if (!m_client || m_currentPlayerId.isEmpty() || !m_client->isAuthenticated()) return;

    m_client->sendCommand(QStringLiteral("players/get"),
        {{QStringLiteral("player_id"), m_currentPlayerId}},
        [this](const QJsonValue &result, const QString &error) {
            if (error.isEmpty() && result.isObject()) {
                m_playerState = result.toObject();
                Q_EMIT playerStateChanged();
                // Start/stop interpolation timer based on playback state
                if (isPlaying()) m_interpolateTimer->start();
                else m_interpolateTimer->stop();
            }
        });

    // Fetch queue for elapsed and duration
    m_client->sendCommand(QStringLiteral("player_queues/get"),
        {{QStringLiteral("queue_id"), m_currentPlayerId}},
        [this](const QJsonValue &result, const QString &error) {
            if (error.isEmpty() && result.isObject()) {
                auto q = result.toObject();
                m_serverElapsed = q.value(QStringLiteral("elapsed_time")).toDouble(m_serverElapsed);
                m_serverElapsedAt = QDateTime::currentMSecsSinceEpoch();
                auto ci = q.value(QStringLiteral("current_item")).toObject();
                int d = ci.value(QStringLiteral("duration")).toInt(0);
                if (d > 0) m_duration = d;
                Q_EMIT elapsedChanged();
                Q_EMIT playerStateChanged();
            }
        });
}

void PlayerController::onEvent(const QString &event, const QString &objectId, const QJsonObject &data)
{
    // queue_time_updated — server sends actual track position (infrequent but authoritative)
    if (event == QStringLiteral("queue_time_updated") && objectId == m_currentPlayerId) {
        qreal e = data.value(QStringLiteral("elapsed_time")).toDouble(-1);
        if (e >= 0) {
            m_serverElapsed = e;
            m_serverElapsedAt = QDateTime::currentMSecsSinceEpoch();
            Q_EMIT elapsedChanged();
        }
        return;
    }

    // queue_updated — queue state changes (track change, seek response, etc.)
    if (event == QStringLiteral("queue_updated") && objectId == m_currentPlayerId) {
        qreal e = data.value(QStringLiteral("elapsed_time")).toDouble(-1);
        if (e >= 0) {
            m_serverElapsed = e;
            m_serverElapsedAt = QDateTime::currentMSecsSinceEpoch();
            Q_EMIT elapsedChanged();
        }
        auto ci = data.value(QStringLiteral("current_item")).toObject();
        int d = ci.value(QStringLiteral("duration")).toInt(0);
        if (d > 0 && d != m_duration) {
            m_duration = d;
            Q_EMIT playerStateChanged();
        }
        return;
    }

    if (objectId != m_currentPlayerId) return;

    if (event == QStringLiteral("player_updated")) {
        m_playerState = data;
        Q_EMIT playerStateChanged();
        // Start/stop interpolation timer based on playback state
        if (isPlaying()) m_interpolateTimer->start();
        else m_interpolateTimer->stop();
    }
}
