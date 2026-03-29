#pragma once

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QUrl>
#include <QHash>
#include <functional>

class MaClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
    Q_PROPERTY(QString serverName READ serverName NOTIFY serverInfoChanged)
    Q_PROPERTY(QString serverVersion READ serverVersion NOTIFY serverInfoChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)

public:
    explicit MaClient(QObject *parent = nullptr);
    ~MaClient() override;

    using ResponseCallback = std::function<void(const QJsonValue &result, const QString &error)>;

    bool isConnected() const;
    bool isAuthenticated() const;
    QString serverName() const;
    QString serverVersion() const;
    QString serverUrl() const;
    void setServerUrl(const QString &url);

    Q_INVOKABLE void connectToServer(const QString &url);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void authenticate(const QString &token);
    Q_INVOKABLE void loginWithCredentials(const QString &username, const QString &password);
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE bool hasSavedSettings() const;
    Q_INVOKABLE void connectWithSavedSettings();

    QString sendCommand(const QString &command, const QJsonObject &args = {}, ResponseCallback callback = nullptr);

    QString getImageUrl(const QString &path, const QString &provider, int size = 0) const;
    QString token() const;

    Q_PROPERTY(bool serverReady READ isServerReady NOTIFY serverReadyChanged)

    bool isServerReady() const;

Q_SIGNALS:
    void connectedChanged();
    void authenticatedChanged();
    void serverInfoChanged();
    void serverReadyChanged();
    void serverUrlChanged();
    void eventReceived(const QString &event, const QString &objectId, const QJsonObject &data);
    void connectionError(const QString &error);

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);
    void sendHeartbeat();

private:
    void handleServerInfo(const QJsonObject &msg);
    void handleAuthResult(const QJsonObject &msg);
    void handleCommandResult(const QJsonObject &msg);
    void handleEvent(const QJsonObject &msg);

    QWebSocket m_socket;
    QTimer m_heartbeatTimer;
    bool m_connected = false;
    bool m_authenticated = false;
    bool m_serverReady = false;
    QString m_serverUrl;
    QString m_serverName;
    QString m_serverVersion;
    QString m_token;
    int m_nextMessageId = 1;
    QHash<QString, ResponseCallback> m_pendingCallbacks;
    // For partial results, accumulate arrays
    QHash<QString, QJsonArray> m_partialResults;
};
