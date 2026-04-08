#include "playercontroller.h"
#include "maclient.h"
#include "sendspinclient.h"
#include <QDateTime>
#include <QJsonArray>

static QStringList jsonArrayToStringList(const QJsonArray &arr)
{
    QStringList result;
    result.reserve(arr.size());
    for (const auto &v : arr) result.append(v.toString());
    return result;
}

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

int PlayerController::volumeLevel() const { return m_volumeLevel; }
bool PlayerController::volumeMuted() const { return m_volumeMuted; }

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

QString PlayerController::mediaType() const
{
    auto media = m_playerState.value(QStringLiteral("current_media")).toObject();
    return media.value(QStringLiteral("media_type")).toString();
}

bool PlayerController::canSeek() const
{
    // Radio streams aren't seekable
    return mediaType() != QStringLiteral("radio") && duration() > 0;
}

bool PlayerController::hasVolumeControl() const
{
    return hasFeature(QStringLiteral("volume_set"));
}

bool PlayerController::hasMuteControl() const
{
    return hasFeature(QStringLiteral("volume_mute"));
}

bool PlayerController::loading() const { return m_loading; }

QString PlayerController::activeSource() const
{
    return m_playerState.value(QStringLiteral("active_source")).toString();
}

QJsonArray PlayerController::sourceList() const
{
    return m_playerState.value(QStringLiteral("source_list")).toArray();
}

QStringList PlayerController::groupMembers() const
{
    return jsonArrayToStringList(m_playerState.value(QStringLiteral("group_members")).toArray());
}

QStringList PlayerController::canGroupWith() const
{
    return jsonArrayToStringList(m_playerState.value(QStringLiteral("can_group_with")).toArray());
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
    // Fallback to current_media from player state
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

void PlayerController::setSendspinClient(SendspinClient *sendspin)
{
    m_sendspin = sendspin;
}

void PlayerController::setVolume(int level)
{
    level = qBound(0, level, 100);
    if (m_volumeLevel == level) return;

    m_volumeLevel = level;
    Q_EMIT volumeChanged();

    // Directly update local audio if the current player is the Sendspin player
    if (isCurrentPlayerSendspin()) {
        m_sendspin->setVolume(level);
    }

    sendPlayerCommand(QStringLiteral("players/cmd/volume_set"), {{QStringLiteral("volume_level"), level}});
}

void PlayerController::volumeUp() { sendPlayerCommand(QStringLiteral("players/cmd/volume_up")); }
void PlayerController::volumeDown() { sendPlayerCommand(QStringLiteral("players/cmd/volume_down")); }

void PlayerController::toggleMute()
{
    m_volumeMuted = !m_volumeMuted;
    Q_EMIT volumeChanged();

    if (isCurrentPlayerSendspin()) {
        m_sendspin->setMuted(m_volumeMuted);
    }

    sendPlayerCommand(QStringLiteral("players/cmd/volume_mute"),
                      {{QStringLiteral("muted"), m_volumeMuted}});
}

void PlayerController::setPower(bool on)
{
    sendPlayerCommand(QStringLiteral("players/cmd/power"), {{QStringLiteral("powered"), on}});
}

bool PlayerController::isCurrentPlayerSendspin() const
{
    return m_sendspin && m_currentPlayerId.contains(m_sendspin->playerId());
}

bool PlayerController::hasFeature(const QString &feature) const
{
    const auto features = m_playerState.value(QStringLiteral("supported_features")).toArray();
    for (const auto &f : features) {
        if (f.toString() == feature) return true;
    }
    return false;
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
                m_volumeLevel = m_playerState.value(QStringLiteral("volume_level")).toInt(0);
                m_volumeMuted = m_playerState.value(QStringLiteral("volume_muted")).toBool(false);
                Q_EMIT volumeChanged();
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
        // Loading state from queue extra_attributes
        auto extra = data.value(QStringLiteral("extra_attributes")).toObject();
        bool newLoading = extra.value(QStringLiteral("play_action_in_progress")).toBool(false);
        if (newLoading != m_loading) {
            m_loading = newLoading;
            Q_EMIT loadingChanged();
        }
        return;
    }

    if (objectId != m_currentPlayerId) return;

    if (event == QStringLiteral("player_updated")) {
        m_playerState = data;

        int vol = data.value(QStringLiteral("volume_level")).toInt(m_volumeLevel);
        bool isSendspin = isCurrentPlayerSendspin();
        // Server doesn't track mute state for Sendspin players — keep local value
        bool muted = isSendspin ? m_volumeMuted
                                : data.value(QStringLiteral("volume_muted")).toBool(m_volumeMuted);
        if (vol != m_volumeLevel || muted != m_volumeMuted) {
            m_volumeLevel = vol;
            m_volumeMuted = muted;
            Q_EMIT volumeChanged();
        }

        // Pick up duration from current_media if queue hasn't provided it
        auto media = data.value(QStringLiteral("current_media")).toObject();
        int d = media.value(QStringLiteral("duration")).toInt(0);
        if (d > 0) m_duration = d;
        Q_EMIT playerStateChanged();
        if (isPlaying()) m_interpolateTimer->start();
        else m_interpolateTimer->stop();
    }
}
