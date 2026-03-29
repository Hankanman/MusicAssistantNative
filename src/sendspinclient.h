#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QBuffer>
#include <QJsonObject>

class MaClient;

class SendspinClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool registered READ isRegistered NOTIFY registeredChanged)
    Q_PROPERTY(QString playerId READ playerId CONSTANT)
    Q_PROPERTY(QString playerName READ playerName CONSTANT)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)

public:
    explicit SendspinClient(QObject *parent = nullptr);
    ~SendspinClient() override;

    bool isRegistered() const;
    QString playerId() const;
    QString playerName() const;
    int volume() const;
    void setVolume(int vol);
    bool muted() const;
    void setMuted(bool m);
    bool isPlaying() const;

    Q_INVOKABLE void connectToServer(const QString &serverUrl, const QString &token);
    Q_INVOKABLE void disconnect();

Q_SIGNALS:
    void registeredChanged();
    void volumeChanged();
    void mutedChanged();
    void playingChanged();
    void errorOccurred(const QString &error);

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &data);
    void sendState();
    void onWsError(QAbstractSocket::SocketError error);

private:
    void sendJson(const QJsonObject &msg);
    void sendHello();
    void handleServerState(const QJsonObject &payload);
    void handleServerCommand(const QJsonObject &payload);
    void handleStreamStart(const QJsonObject &payload);
    void handleStreamEnd();
    void handleStreamClear();
    void handleTimeResponse(const QJsonObject &payload);
    void sendTimePing();

    QWebSocket m_socket;
    QTimer m_stateTimer;
    QTimer m_timeTimer;
    QString m_playerId;
    QString m_playerName;
    QString m_token;
    bool m_registered = false;
    bool m_authenticated = false;
    int m_volume = 80;
    bool m_muted = false;
    bool m_playing = false;
    QString m_lastServerUrl;

    // Audio playback
    int m_binFrameCount = 0;
    QAudioOutput *m_audioOutput;
    QString m_currentCodec;
    int m_sampleRate = 48000;
    int m_channels = 2;
};
