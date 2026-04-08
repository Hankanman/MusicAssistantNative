#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlEngine>
#include <QTimer>
#include <QStringList>

class MaClient;
class SendspinClient;

class PlayerController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentPlayerId READ currentPlayerId WRITE setCurrentPlayerId NOTIFY currentPlayerIdChanged)
    Q_PROPERTY(QString playerName READ playerName NOTIFY playerStateChanged)
    Q_PROPERTY(QString playbackState READ playbackState NOTIFY playerStateChanged)
    Q_PROPERTY(int volumeLevel READ volumeLevel NOTIFY volumeChanged)
    Q_PROPERTY(bool volumeMuted READ volumeMuted NOTIFY volumeChanged)
    Q_PROPERTY(bool powered READ powered NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackTitle READ currentTrackTitle NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackArtist READ currentTrackArtist NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackAlbum READ currentTrackAlbum NOTIFY playerStateChanged)
    Q_PROPERTY(QString currentTrackImageUrl READ currentTrackImageUrl NOTIFY playerStateChanged)
    Q_PROPERTY(QString mediaType READ mediaType NOTIFY playerStateChanged)
    Q_PROPERTY(int elapsed READ elapsed NOTIFY elapsedChanged)
    Q_PROPERTY(int duration READ duration NOTIFY playerStateChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playerStateChanged)
    Q_PROPERTY(bool canSeek READ canSeek NOTIFY playerStateChanged)
    Q_PROPERTY(bool hasVolumeControl READ hasVolumeControl NOTIFY playerStateChanged)
    Q_PROPERTY(bool hasMuteControl READ hasMuteControl NOTIFY playerStateChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString activeSource READ activeSource NOTIFY playerStateChanged)
    Q_PROPERTY(QJsonArray sourceList READ sourceList NOTIFY playerStateChanged)
    Q_PROPERTY(QStringList groupMembers READ groupMembers NOTIFY playerStateChanged)
    Q_PROPERTY(QStringList canGroupWith READ canGroupWith NOTIFY playerStateChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);

    void setClient(MaClient *client);
    void setSendspinClient(SendspinClient *sendspin);

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
    QString mediaType() const;
    int elapsed() const;
    int duration() const;
    bool isPlaying() const;
    bool canSeek() const;
    bool hasVolumeControl() const;
    bool hasMuteControl() const;
    bool loading() const;
    QString activeSource() const;
    QJsonArray sourceList() const;
    QStringList groupMembers() const;
    QStringList canGroupWith() const;

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
    void volumeChanged();
    void loadingChanged();

private Q_SLOTS:
    void onEvent(const QString &event, const QString &objectId, const QJsonObject &data);

private:
    bool isCurrentPlayerSendspin() const;
    bool hasFeature(const QString &feature) const;
    void sendPlayerCommand(const QString &cmd, const QJsonObject &extraArgs = {});
    void fetchPlayerState();

    MaClient *m_client = nullptr;
    SendspinClient *m_sendspin = nullptr;
    QString m_currentPlayerId;
    QJsonObject m_playerState;
    QTimer *m_interpolateTimer;
    qreal m_serverElapsed = 0;     // last elapsed from server
    qint64 m_serverElapsedAt = 0;  // when we received it (msec since epoch)
    int m_duration = 0;
    int m_volumeLevel = 0;
    bool m_volumeMuted = false;
    bool m_loading = false;
};
