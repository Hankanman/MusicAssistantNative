#include "sendspinclient.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QSettings>
#include <QSysInfo>
#include <QDebug>
#include <QDateTime>

SendspinClient::SendspinClient(QObject *parent)
    : QObject(parent)
    , m_audioOutput(new QAudioOutput(this))
{
    m_audioOutput->setVolume(0.8);

    // Generate or load persistent player ID
    QSettings settings;
    m_playerId = settings.value(QStringLiteral("Sendspin/playerId")).toString();
    if (m_playerId.isEmpty()) {
        // Generate ID like MA frontend: ma_<random>
        m_playerId = QStringLiteral("ma_") + QUuid::createUuid().toString(QUuid::Id128).left(10);
        settings.setValue(QStringLiteral("Sendspin/playerId"), m_playerId);
    }

    m_playerName = QStringLiteral("Music Assistant Native (%1)").arg(QSysInfo::machineHostName());

    connect(&m_socket, &QWebSocket::connected, this, &SendspinClient::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &SendspinClient::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &SendspinClient::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::binaryMessageReceived, this, &SendspinClient::onBinaryMessageReceived);
    connect(&m_socket, &QWebSocket::errorOccurred, this, &SendspinClient::onWsError);

    // Send state every 5 seconds
    m_stateTimer.setInterval(5000);
    connect(&m_stateTimer, &QTimer::timeout, this, &SendspinClient::sendState);

    // Time sync every 10 seconds
    m_timeTimer.setInterval(10000);
    connect(&m_timeTimer, &QTimer::timeout, this, &SendspinClient::sendTimePing);
}

SendspinClient::~SendspinClient()
{
    disconnect();
}

bool SendspinClient::isRegistered() const { return m_registered; }
QString SendspinClient::playerId() const { return m_playerId; }
QString SendspinClient::playerName() const { return m_playerName; }

int SendspinClient::volume() const { return m_volume; }
void SendspinClient::setVolume(int vol)
{
    m_volume = qBound(0, vol, 100);
    m_audioOutput->setVolume(m_volume / 100.0);
    Q_EMIT volumeChanged();
    sendState();
}

bool SendspinClient::muted() const { return m_muted; }
void SendspinClient::setMuted(bool m)
{
    m_muted = m;
    m_audioOutput->setMuted(m);
    Q_EMIT mutedChanged();
    sendState();
}

bool SendspinClient::isPlaying() const { return m_playing; }

void SendspinClient::connectToServer(const QString &serverUrl, const QString &token)
{
    m_token = token;

    m_lastServerUrl = serverUrl;

    // Connect directly to Sendspin server on port 8927 (no auth needed)
    QUrl serverQUrl(serverUrl);
    QString wsUrl = QStringLiteral("ws://%1:8927/sendspin").arg(serverQUrl.host());

    qDebug() << "SendspinClient: connecting to" << wsUrl;
    m_socket.open(QUrl(wsUrl));
}

void SendspinClient::disconnect()
{
    m_stateTimer.stop();
    m_timeTimer.stop();

    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
        // Send goodbye
        QJsonObject goodbye;
        goodbye[QStringLiteral("type")] = QStringLiteral("client/goodbye");
        QJsonObject payload;
        payload[QStringLiteral("reason")] = QStringLiteral("shutdown");
        goodbye[QStringLiteral("payload")] = payload;
        sendJson(goodbye);
        m_socket.close();
    }

    if (m_registered) {
        m_registered = false;
        Q_EMIT registeredChanged();
    }
    m_authenticated = false;
}

void SendspinClient::onConnected()
{
    qDebug() << "SendspinClient: WebSocket connected, sending hello directly...";
    m_authenticated = true;
    sendHello();
}

void SendspinClient::onDisconnected()
{
    qDebug() << "SendspinClient: disconnected - close code:" << m_socket.closeCode()
             << "reason:" << m_socket.closeReason();
    m_stateTimer.stop();
    m_timeTimer.stop();
    m_authenticated = false;
    bool wasRegistered = m_registered;
    if (m_registered) {
        m_registered = false;
        Q_EMIT registeredChanged();
    }
    if (m_playing) {
        m_playing = false;
        Q_EMIT playingChanged();
    }

    // Auto-reconnect after 3 seconds if we were previously registered
    if (wasRegistered && !m_lastServerUrl.isEmpty()) {
        qDebug() << "SendspinClient: will reconnect in 3s...";
        QTimer::singleShot(3000, this, [this]() {
            if (!m_registered) {
                connectToServer(m_lastServerUrl, m_token);
            }
        });
    }
}

void SendspinClient::onTextMessageReceived(const QString &message)
{
    auto doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;

    auto msg = doc.object();
    QString type = msg.value(QStringLiteral("type")).toString();
    QJsonObject payload = msg.value(QStringLiteral("payload")).toObject();

    qDebug() << "SendspinClient: received" << type
             << "payload keys:" << payload.keys();

    if (type == QStringLiteral("auth_ok")) {
        qDebug() << "SendspinClient: auth OK, sending hello...";
        m_authenticated = true;
        sendHello();
    } else if (type == QStringLiteral("server/time")) {
        // Must respond to time probes to stay connected
        sendTimePing();
        return;
    } else if (type == QStringLiteral("group/update")) {
        // Player group state changed — informational
        return;
    } else if (type == QStringLiteral("server/hello")) {
        qDebug() << "SendspinClient: server/hello received:"
                 << "active_roles:" << payload.value(QStringLiteral("active_roles"))
                 << "connection_reason:" << payload.value(QStringLiteral("connection_reason")).toString()
                 << "server_id:" << payload.value(QStringLiteral("server_id")).toString()
                 << "version:" << payload.value(QStringLiteral("version"));
        m_registered = true;
        Q_EMIT registeredChanged();
        m_stateTimer.start();
        m_timeTimer.start();
        // Send initial time sync burst (8 probes like web frontend) and state
        for (int i = 0; i < 8; ++i)
            sendTimePing();
        sendState();
        qDebug() << "SendspinClient: sent initial time sync burst + state";
    } else if (type == QStringLiteral("server/state")) {
        handleServerState(payload);
    } else if (type == QStringLiteral("server/command")) {
        handleServerCommand(payload);
    } else if (type == QStringLiteral("stream/start")) {
        handleStreamStart(payload);
    } else if (type == QStringLiteral("stream/end")) {
        handleStreamEnd();
    } else if (type == QStringLiteral("stream/clear")) {
        handleStreamClear();
    } else if (type == QStringLiteral("server/time")) {
        handleTimeResponse(payload);
    } else if (type == QStringLiteral("server/goodbye")) {
        qDebug() << "SendspinClient: server said goodbye -"
                 << payload.value(QStringLiteral("reason")).toString();
    } else if (type == QStringLiteral("error")) {
        qDebug() << "SendspinClient: ERROR from server:"
                 << QJsonDocument(payload).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "SendspinClient: UNHANDLED message type:" << type
                 << "full:" << QJsonDocument(msg).toJson(QJsonDocument::Compact).left(500);
    }
}

void SendspinClient::onBinaryMessageReceived(const QByteArray &data)
{
    // Binary frames: byte 0 = role+slot, bytes 1-8 = timestamp, bytes 9+ = audio
    if (data.size() < 9) return;

    // For now, just track that we're receiving audio
    if (!m_playing) {
        m_playing = true;
        Q_EMIT playingChanged();
        qDebug() << "SendspinClient: receiving audio data, codec:" << m_currentCodec;
    }

    // TODO: Decode and play audio frames
    // The audio data is encoded in the negotiated codec (opus/flac)
    // Bytes 1-8 are the server timestamp for sync
    // Bytes 9+ are the encoded audio payload
}

void SendspinClient::onWsError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    qDebug() << "SendspinClient: WebSocket error:" << m_socket.errorString();
    Q_EMIT errorOccurred(m_socket.errorString());
}

void SendspinClient::sendJson(const QJsonObject &msg)
{
    m_socket.sendTextMessage(QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact)));
}

void SendspinClient::sendHello()
{
    QJsonObject hello;
    hello[QStringLiteral("type")] = QStringLiteral("client/hello");

    QJsonObject payload;
    payload[QStringLiteral("client_id")] = m_playerId;
    payload[QStringLiteral("name")] = m_playerName;
    payload[QStringLiteral("version")] = 1;

    QJsonArray roles;
    roles.append(QStringLiteral("player@v1"));
    roles.append(QStringLiteral("controller@v1"));
    roles.append(QStringLiteral("metadata@v1"));
    payload[QStringLiteral("supported_roles")] = roles;

    QJsonObject deviceInfo;
    deviceInfo[QStringLiteral("product_name")] = QStringLiteral("Desktop Application");
    deviceInfo[QStringLiteral("manufacturer")] = QStringLiteral("Music Assistant Native");
    deviceInfo[QStringLiteral("software_version")] = QStringLiteral("0.1.0");
    payload[QStringLiteral("device_info")] = deviceInfo;

    // Supported audio formats
    QJsonObject playerSupport;
    QJsonArray formats;

    // FLAC 44.1kHz
    QJsonObject flac44;
    flac44[QStringLiteral("codec")] = QStringLiteral("flac");
    flac44[QStringLiteral("sample_rate")] = 44100;
    flac44[QStringLiteral("channels")] = 2;
    flac44[QStringLiteral("bit_depth")] = 16;
    formats.append(flac44);

    // FLAC 48kHz
    QJsonObject flac48;
    flac48[QStringLiteral("codec")] = QStringLiteral("flac");
    flac48[QStringLiteral("sample_rate")] = 48000;
    flac48[QStringLiteral("channels")] = 2;
    flac48[QStringLiteral("bit_depth")] = 16;
    formats.append(flac48);

    playerSupport[QStringLiteral("supported_formats")] = formats;
    playerSupport[QStringLiteral("buffer_capacity")] = 5242880; // 5MB
    QJsonArray supportedCmds;
    supportedCmds.append(QStringLiteral("volume"));
    supportedCmds.append(QStringLiteral("mute"));
    playerSupport[QStringLiteral("supported_commands")] = supportedCmds;

    payload[QStringLiteral("player@v1_support")] = playerSupport;

    hello[QStringLiteral("payload")] = payload;

    qDebug() << "SendspinClient: sending hello as" << m_playerName << "id:" << m_playerId;
    sendJson(hello);

    // CRITICAL: Send time sync burst immediately after hello, before server timeout
    for (int i = 0; i < 8; ++i)
        sendTimePing();
}

void SendspinClient::sendState()
{
    if (!m_registered) return;

    QJsonObject state;
    state[QStringLiteral("type")] = QStringLiteral("client/state");

    QJsonObject payload;
    QJsonObject playerState;
    playerState[QStringLiteral("state")] = m_playing ? QStringLiteral("synchronized") : QStringLiteral("idle");
    playerState[QStringLiteral("volume")] = m_volume;
    playerState[QStringLiteral("muted")] = m_muted;
    payload[QStringLiteral("player")] = playerState;

    state[QStringLiteral("payload")] = payload;
    sendJson(state);
}

void SendspinClient::sendTimePing()
{
    if (!m_registered) return;

    QJsonObject ping;
    ping[QStringLiteral("type")] = QStringLiteral("client/time");

    QJsonObject payload;
    payload[QStringLiteral("client_transmitted")] = static_cast<qint64>(
        QDateTime::currentMSecsSinceEpoch() * 1000); // microseconds
    ping[QStringLiteral("payload")] = payload;

    sendJson(ping);
}

void SendspinClient::handleServerState(const QJsonObject &payload)
{
    auto metadata = payload.value(QStringLiteral("metadata")).toObject();
    if (!metadata.isEmpty()) {
        qDebug() << "SendspinClient: now playing -"
                 << metadata.value(QStringLiteral("title")).toString()
                 << "by" << metadata.value(QStringLiteral("artist")).toString();
    }
}

void SendspinClient::handleServerCommand(const QJsonObject &payload)
{
    auto playerCmd = payload.value(QStringLiteral("player")).toObject();
    if (playerCmd.isEmpty()) return;

    QString cmd = playerCmd.value(QStringLiteral("command")).toString();
    qDebug() << "SendspinClient: server command:" << cmd;

    if (cmd == QStringLiteral("volume")) {
        m_volume = playerCmd.value(QStringLiteral("volume")).toInt(m_volume);
        m_audioOutput->setVolume(m_volume / 100.0);
        Q_EMIT volumeChanged();
    } else if (cmd == QStringLiteral("mute")) {
        m_muted = playerCmd.value(QStringLiteral("muted")).toBool(m_muted);
        m_audioOutput->setMuted(m_muted);
        Q_EMIT mutedChanged();
    }
}

void SendspinClient::handleStreamStart(const QJsonObject &payload)
{
    auto playerInfo = payload.value(QStringLiteral("player")).toObject();
    m_currentCodec = playerInfo.value(QStringLiteral("codec")).toString();
    m_sampleRate = playerInfo.value(QStringLiteral("sample_rate")).toInt(48000);
    m_channels = playerInfo.value(QStringLiteral("channels")).toInt(2);

    qDebug() << "SendspinClient: stream starting - codec:" << m_currentCodec
             << "rate:" << m_sampleRate << "ch:" << m_channels;

    m_playing = true;
    Q_EMIT playingChanged();
    sendState();
}

void SendspinClient::handleStreamEnd()
{
    qDebug() << "SendspinClient: stream ended";
    m_playing = false;
    Q_EMIT playingChanged();
    sendState();
}

void SendspinClient::handleStreamClear()
{
    qDebug() << "SendspinClient: stream cleared (skip/seek)";
}

void SendspinClient::handleTimeResponse(const QJsonObject &payload)
{
    // NTP-style time sync — store offset for future use
    Q_UNUSED(payload)
    // TODO: Implement Kalman time filter for sample-accurate sync
}
