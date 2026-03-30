#include "maclient.h"
#include <QUuid>
#include <QDebug>
#include <QSettings>

MaClient::MaClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &MaClient::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &MaClient::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &MaClient::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::errorOccurred, this, &MaClient::onError);

    m_heartbeatTimer.setInterval(25000);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &MaClient::sendHeartbeat);
}

MaClient::~MaClient()
{
    disconnect();
}

bool MaClient::isConnected() const { return m_connected; }
bool MaClient::isAuthenticated() const { return m_authenticated; }
bool MaClient::isServerReady() const { return m_serverReady; }
QString MaClient::token() const { return m_token; }
QString MaClient::serverName() const { return m_serverName; }
QString MaClient::serverVersion() const { return m_serverVersion; }
QString MaClient::serverUrl() const { return m_serverUrl; }

void MaClient::setServerUrl(const QString &url)
{
    if (m_serverUrl != url) {
        m_serverUrl = url;
        Q_EMIT serverUrlChanged();
    }
}

void MaClient::connectToServer(const QString &url)
{
    if (m_connected) {
        disconnect();
    }
    setServerUrl(url);

    QString wsUrl = url;
    if (wsUrl.startsWith(QStringLiteral("http://"))) {
        wsUrl.replace(0, 7, QStringLiteral("ws://"));
    } else if (wsUrl.startsWith(QStringLiteral("https://"))) {
        wsUrl.replace(0, 8, QStringLiteral("wss://"));
    }
    if (!wsUrl.endsWith(QStringLiteral("/ws"))) {
        if (!wsUrl.endsWith(QLatin1Char('/')))
            wsUrl += QLatin1Char('/');
        wsUrl += QStringLiteral("ws");
    }

    m_socket.open(QUrl(wsUrl));
}

void MaClient::disconnect()
{
    m_heartbeatTimer.stop();
    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
        m_socket.close();
    }
    if (m_connected) {
        m_connected = false;
        Q_EMIT connectedChanged();
    }
    if (m_authenticated) {
        m_authenticated = false;
        Q_EMIT authenticatedChanged();
    }
}

void MaClient::authenticate(const QString &token)
{
    m_token = token;
    QJsonObject args;
    args[QStringLiteral("token")] = token;
    sendCommand(QStringLiteral("auth"), args, [this](const QJsonValue &/*result*/, const QString &error) {
        if (error.isEmpty()) {
            qDebug() << "MaClient: authenticated";
            m_authenticated = true;
            Q_EMIT authenticatedChanged();
        } else {
            qDebug() << "MaClient: authentication failed:" << error;
            Q_EMIT connectionError(QStringLiteral("Authentication failed: ") + error);
        }
    });
}

void MaClient::loginWithCredentials(const QString &username, const QString &password)
{
    QJsonObject args;
    args[QStringLiteral("username")] = username;
    args[QStringLiteral("password")] = password;
    args[QStringLiteral("provider_id")] = QStringLiteral("builtin");
    args[QStringLiteral("device_name")] = QStringLiteral("Music Assistant Native");
    sendCommand(QStringLiteral("auth/login"), args, [this](const QJsonValue &result, const QString &error) {
        if (error.isEmpty() && result.isObject()) {
            auto obj = result.toObject();
            bool success = obj.value(QStringLiteral("success")).toBool(false);
            if (!success) {
                QString errMsg = obj.value(QStringLiteral("error")).toString(QStringLiteral("Unknown error"));
                Q_EMIT connectionError(QStringLiteral("Login failed: ") + errMsg);
                return;
            }
            m_token = obj.value(QStringLiteral("access_token")).toString();
            if (!m_token.isEmpty()) {
                authenticate(m_token);
            } else {
                Q_EMIT connectionError(QStringLiteral("Login succeeded but no token returned"));
            }
        } else if (error.isEmpty() && result.isString()) {
            m_token = result.toString();
            authenticate(m_token);
        } else {
            Q_EMIT connectionError(QStringLiteral("Login failed: ") + error);
        }
    });
}

QString MaClient::sendCommand(const QString &command, const QJsonObject &args, ResponseCallback callback)
{
    QString msgId = QString::number(m_nextMessageId++);

    QJsonObject msg;
    msg[QStringLiteral("message_id")] = msgId;
    msg[QStringLiteral("command")] = command;
    if (!args.isEmpty()) {
        msg[QStringLiteral("args")] = args;
    }

    if (callback) {
        m_pendingCallbacks.insert(msgId, callback);
    }

    m_socket.sendTextMessage(QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact)));
    return msgId;
}

QString MaClient::getImageUrl(const QString &path, const QString &provider, int size) const
{
    if (path.isEmpty()) return {};

    QString baseUrl = m_serverUrl;
    if (baseUrl.endsWith(QLatin1Char('/')))
        baseUrl.chop(1);

    QString url = baseUrl + QStringLiteral("/imageproxy?path=")
                  + QString::fromUtf8(QUrl::toPercentEncoding(path))
                  + QStringLiteral("&provider=")
                  + QString::fromUtf8(QUrl::toPercentEncoding(provider));
    if (size > 0)
        url += QStringLiteral("&size=") + QString::number(size);
    return url;
}

void MaClient::saveSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Connection"));
    settings.setValue(QStringLiteral("serverUrl"), m_serverUrl);
    settings.setValue(QStringLiteral("token"), m_token);
    settings.endGroup();
    settings.sync();
}

void MaClient::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Connection"));
    m_serverUrl = settings.value(QStringLiteral("serverUrl")).toString();
    m_token = settings.value(QStringLiteral("token")).toString();
    settings.endGroup();
    Q_EMIT serverUrlChanged();
}

bool MaClient::hasSavedSettings() const
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Connection"));
    return !settings.value(QStringLiteral("serverUrl")).toString().isEmpty()
        && !settings.value(QStringLiteral("token")).toString().isEmpty();
}

void MaClient::connectWithSavedSettings()
{
    loadSettings();
    if (!m_serverUrl.isEmpty() && !m_token.isEmpty()) {
        connectToServer(m_serverUrl);
    }
}

void MaClient::onConnected()
{
    m_connected = true;
    m_heartbeatTimer.start();
    Q_EMIT connectedChanged();
}

void MaClient::onDisconnected()
{
    m_heartbeatTimer.stop();
    bool wasAuthenticated = m_authenticated;
    m_connected = false;
    m_authenticated = false;
    m_serverReady = false;
    m_pendingCallbacks.clear();
    m_partialResults.clear();
    Q_EMIT connectedChanged();
    Q_EMIT authenticatedChanged();
    Q_EMIT serverReadyChanged();
    qDebug() << "MaClient: disconnected";

    // Auto-reconnect if we were previously authenticated
    if (wasAuthenticated && !m_serverUrl.isEmpty()) {
        qDebug() << "MaClient: will reconnect in 5s...";
        QTimer::singleShot(5000, this, [this]() {
            if (!m_connected) {
                connectToServer(m_serverUrl);
            }
        });
    }
}

void MaClient::onTextMessageReceived(const QString &message)
{
    auto doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;

    auto msg = doc.object();

    // Server info message (has server_id field)
    if (msg.contains(QStringLiteral("server_id"))) {
        handleServerInfo(msg);
        return;
    }

    // Event message (has event field)
    if (msg.contains(QStringLiteral("event"))) {
        handleEvent(msg);
        return;
    }

    // Command result (has message_id)
    if (msg.contains(QStringLiteral("message_id"))) {
        handleCommandResult(msg);
        return;
    }
}

void MaClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    Q_EMIT connectionError(m_socket.errorString());
}

void MaClient::sendHeartbeat()
{
    if (m_connected) {
        m_socket.ping();
    }
}

void MaClient::handleServerInfo(const QJsonObject &msg)
{
    m_serverName = msg.value(QStringLiteral("name")).toString();
    m_serverVersion = msg.value(QStringLiteral("server_version")).toString();
    m_serverReady = true;
    qDebug() << "MaClient: server ready -" << m_serverName << "v" << m_serverVersion;
    Q_EMIT serverInfoChanged();
    Q_EMIT serverReadyChanged();

    // Auto-authenticate if we have a token
    if (!m_token.isEmpty()) {
        authenticate(m_token);
    }
}

void MaClient::handleCommandResult(const QJsonObject &msg)
{
    QString msgId = msg.value(QStringLiteral("message_id")).toString();

    // Error result
    if (msg.contains(QStringLiteral("error_code"))) {
        QString details = msg.value(QStringLiteral("details")).toString();
        auto it = m_pendingCallbacks.find(msgId);
        if (it != m_pendingCallbacks.end()) {
            it.value()(QJsonValue(), details);
            m_pendingCallbacks.erase(it);
        }
        m_partialResults.remove(msgId);
        return;
    }

    // Success result - check for partial
    bool partial = msg.value(QStringLiteral("partial")).toBool(false);
    QJsonValue result = msg.value(QStringLiteral("result"));

    if (partial && result.isArray()) {
        // Accumulate partial array results
        QJsonArray &accumulated = m_partialResults[msgId];
        const auto arr = result.toArray();
        for (const auto &item : arr) {
            accumulated.append(item);
        }
        return;
    }

    // Final result
    QJsonValue finalResult = result;
    if (m_partialResults.contains(msgId)) {
        // Merge any remaining items
        QJsonArray &accumulated = m_partialResults[msgId];
        if (result.isArray()) {
            const auto arr = result.toArray();
            for (const auto &item : arr) {
                accumulated.append(item);
            }
        }
        finalResult = accumulated;
        m_partialResults.remove(msgId);
    }

    auto it = m_pendingCallbacks.find(msgId);
    if (it != m_pendingCallbacks.end()) {
        it.value()(finalResult, QString());
        m_pendingCallbacks.erase(it);
    }
}

void MaClient::handleEvent(const QJsonObject &msg)
{
    QString event = msg.value(QStringLiteral("event")).toString();
    QString objectId = msg.value(QStringLiteral("object_id")).toString();
    QJsonValue dataVal = msg.value(QStringLiteral("data"));
    QJsonObject data = dataVal.toObject(); // empty for non-object data

    // For queue_time_updated, data is a float (elapsed seconds)
    if (event == QStringLiteral("queue_time_updated") && dataVal.isDouble()) {
        data[QStringLiteral("elapsed_time")] = dataVal.toDouble();
    }

    Q_EMIT eventReceived(event, objectId, data);
}
