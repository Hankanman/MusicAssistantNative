#include "localplayer.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QSysInfo>

LocalPlayer::LocalPlayer(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.5);

    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &LocalPlayer::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &LocalPlayer::onPlayerError);
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, [this]() { Q_EMIT playingChanged(); });
}

bool LocalPlayer::available() const { return true; }

bool LocalPlayer::playing() const
{
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

int LocalPlayer::volume() const
{
    return qRound(m_audioOutput->volume() * 100.0);
}

void LocalPlayer::setVolume(int vol)
{
    m_audioOutput->setVolume(vol / 100.0);
    Q_EMIT volumeChanged();
}

bool LocalPlayer::muted() const { return m_audioOutput->isMuted(); }

void LocalPlayer::setMuted(bool m)
{
    m_audioOutput->setMuted(m);
    Q_EMIT mutedChanged();
}

QString LocalPlayer::playerName() const
{
    return QStringLiteral("Music Assistant Native (%1)").arg(QSysInfo::machineHostName());
}

void LocalPlayer::playUrl(const QString &url, const QString &token)
{
    qDebug() << "LocalPlayer: playing" << url.left(80);
    QUrl mediaUrl(url);
    QNetworkRequest req(mediaUrl);
    if (!token.isEmpty()) {
        req.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1").arg(token).toUtf8());
    }
    m_player->setSource(mediaUrl);
    m_player->play();
}

void LocalPlayer::pause()
{
    m_player->pause();
}

void LocalPlayer::resume()
{
    m_player->play();
}

void LocalPlayer::stop()
{
    m_player->stop();
}

void LocalPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "LocalPlayer: media status:" << status;
}

void LocalPlayer::onPlayerError(QMediaPlayer::Error error, const QString &errorString)
{
    qDebug() << "LocalPlayer: error:" << error << errorString;
    Q_EMIT errorOccurred(errorString);
}
