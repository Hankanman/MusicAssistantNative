#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QQmlEngine>

class MaClient;

class LocalPlayer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QString playerName READ playerName CONSTANT)
    Q_PROPERTY(bool isLocalPlayer READ isLocalPlayer CONSTANT)

public:
    explicit LocalPlayer(QObject *parent = nullptr);

    bool available() const;
    bool playing() const;
    int volume() const;
    void setVolume(int vol);
    bool muted() const;
    void setMuted(bool m);
    QString playerName() const;
    bool isLocalPlayer() const { return true; }

    Q_INVOKABLE void playUrl(const QString &url, const QString &token);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();

Q_SIGNALS:
    void playingChanged();
    void volumeChanged();
    void mutedChanged();
    void errorOccurred(const QString &error);

private Q_SLOTS:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlayerError(QMediaPlayer::Error error, const QString &errorString);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
};
