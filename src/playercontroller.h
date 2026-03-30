#pragma once

#include <QObject>
#include <QJsonObject>
#include <QQmlEngine>
#include <QTimer>

class MaClient;

class PlayerController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentPlayerId READ currentPlayerId WRITE setCurrentPlayerId NOTIFY currentPlayerIdChanged)
    Q_PROPERTY(QString playerName READ playerName NOTIFY playerStateChanged)
    Q_PROPERTY(QString playbackState READ playbackState NOTIFY playerStateChanged)
    Q_PROPERTY(int volumeLevel READ volumeLevel NOTIFY playerStateChanged)
    Q_PROPERTY(bool volumeMuted READ volumeMuted NOTIFY playerStateChanged)
    Q_PROPERTY(bool powered READ powered NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackTitle READ currentTrackTitle NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackArtist READ currentTrackArtist NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackAlbum READ currentTrackAlbum NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackImageUrl READ currentTrackImageUrl NOTIFY playerStateChanged)
    Q_PROPERTY(int elapsed READ elapsed NOTIFY elapsedChanged)
    Q_PROPERTY(int duration READ duration NOTIFY playerStateChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playerStateChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);

    void setClient(MaClient *client);

    QString currentPlayerId() const;
    void setCurrentPlayerId(const QString &id);
    QString playerName() const;
    QString playbackState() const;
    int volumeLevel() const;
    bool volumeMuted() const;
    bool powered() const;
    QString currentTrackTitle() const;
    QString currentTrackArtist() const;
    QString currentTrackAlbum() const;
    QString currentTrackImageUrl() const;
    int elapsed() const;
    int duration() const;
    bool isPlaying() const;

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seek(int position);
    Q_INVOKABLE void setVolume(int level);
    Q_INVOKABLE void volumeUp();
    Q_INVOKABLE void volumeDown();
    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void setPower(bool on);

Q_SIGNALS:
    void currentPlayerIdChanged();
    void playerStateChanged();
    void elapsedChanged();

private Q_SLOTS:
    void onEvent(const QString &event, const QString &objectId, const QJsonObject &data);

private:
    void sendPlayerCommand(const QString &cmd, const QJsonObject &extraArgs = {});
    void fetchPlayerState();

    MaClient *m_client = nullptr;
    QString m_currentPlayerId;
    QJsonObject m_playerState;
    QTimer *m_interpolateTimer;
    qreal m_serverElapsed = 0;     // last elapsed from server
    qint64 m_serverElapsedAt = 0;  // when we received it (msec since epoch)
    int m_duration = 0;
};
