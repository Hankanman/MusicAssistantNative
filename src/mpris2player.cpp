#include "mpris2player.h"
#include "playercontroller.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>

// --- Mpris2RootAdaptor ---

Mpris2RootAdaptor::Mpris2RootAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

void Mpris2RootAdaptor::Raise()
{
    Q_EMIT raiseRequested();
}

void Mpris2RootAdaptor::Quit()
{
    QApplication::quit();
}

// --- Mpris2PlayerAdaptor ---

Mpris2PlayerAdaptor::Mpris2PlayerAdaptor(PlayerController *controller, QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_controller(controller)
{
    connect(m_controller, &PlayerController::playerStateChanged,
            this, &Mpris2PlayerAdaptor::onPlayerStateChanged);
    connect(m_controller, &PlayerController::volumeChanged,
            this, &Mpris2PlayerAdaptor::onPlayerStateChanged);
    connect(m_controller, &PlayerController::elapsedChanged,
            this, &Mpris2PlayerAdaptor::onElapsedChanged);
}

QString Mpris2PlayerAdaptor::playbackStatus() const
{
    const QString state = m_controller->playbackState();
    if (state == QStringLiteral("playing")) {
        return QStringLiteral("Playing");
    } else if (state == QStringLiteral("paused")) {
        return QStringLiteral("Paused");
    }
    return QStringLiteral("Stopped");
}

QVariantMap Mpris2PlayerAdaptor::metadata() const
{
    QVariantMap map;

    const QString title = m_controller->currentTrackTitle();
    if (title.isEmpty()) {
        return map;
    }

    map[QStringLiteral("mpris:trackid")] = QVariant::fromValue(
        QDBusObjectPath(QStringLiteral("/org/mpris/MediaPlayer2/Track/1")));
    map[QStringLiteral("xesam:title")] = title;
    map[QStringLiteral("xesam:artist")] = QStringList{m_controller->currentTrackArtist()};
    map[QStringLiteral("xesam:album")] = m_controller->currentTrackAlbum();

    const int dur = m_controller->duration();
    if (dur > 0) {
        map[QStringLiteral("mpris:length")] = static_cast<qlonglong>(dur) * 1000000LL;
    }

    const QString imageUrl = m_controller->currentTrackImageUrl();
    if (!imageUrl.isEmpty()) {
        map[QStringLiteral("mpris:artUrl")] = imageUrl;
    }

    return map;
}

double Mpris2PlayerAdaptor::volume() const
{
    return m_controller->volumeLevel() / 100.0;
}

void Mpris2PlayerAdaptor::setVolume(double vol)
{
    m_controller->setVolume(qBound(0, static_cast<int>(vol * 100.0), 100));
}

qlonglong Mpris2PlayerAdaptor::position() const
{
    return static_cast<qlonglong>(m_controller->elapsed()) * 1000000LL;
}

void Mpris2PlayerAdaptor::Play()
{
    m_controller->play();
}

void Mpris2PlayerAdaptor::Pause()
{
    m_controller->pause();
}

void Mpris2PlayerAdaptor::PlayPause()
{
    m_controller->playPause();
}

void Mpris2PlayerAdaptor::Stop()
{
    m_controller->stop();
}

void Mpris2PlayerAdaptor::Next()
{
    m_controller->next();
}

void Mpris2PlayerAdaptor::Previous()
{
    m_controller->previous();
}

void Mpris2PlayerAdaptor::Seek(qlonglong offset)
{
    // offset is in microseconds, seek() expects seconds
    int currentSec = m_controller->elapsed();
    int newPos = currentSec + static_cast<int>(offset / 1000000LL);
    m_controller->seek(qMax(0, newPos));
}

void Mpris2PlayerAdaptor::SetPosition(const QDBusObjectPath &trackId, qlonglong pos)
{
    Q_UNUSED(trackId)
    // position is in microseconds, seek() expects seconds
    m_controller->seek(static_cast<int>(pos / 1000000LL));
}

void Mpris2PlayerAdaptor::onPlayerStateChanged()
{
    QVariantMap changed;

    const QString status = playbackStatus();
    if (status != m_lastPlaybackStatus) {
        m_lastPlaybackStatus = status;
        changed[QStringLiteral("PlaybackStatus")] = status;
    }

    const QVariantMap meta = metadata();
    if (meta != m_lastMetadata) {
        m_lastMetadata = meta;
        changed[QStringLiteral("Metadata")] = meta;
    }

    const double vol = volume();
    if (qAbs(vol - m_lastVolume) > 0.001) {
        m_lastVolume = vol;
        changed[QStringLiteral("Volume")] = vol;
    }

    if (!changed.isEmpty()) {
        emitMultiplePropertiesChanged(changed);
    }
}

void Mpris2PlayerAdaptor::onElapsedChanged()
{
    // Debounce: only emit Position every ~1 second (elapsed resolution is 1s)
    const qlonglong pos = position();
    const qlonglong deltaMicros = qAbs(pos - m_lastEmittedPosition);
    if (deltaMicros < 900000LL) {
        return;
    }
    qlonglong oldPos = m_lastEmittedPosition;
    m_lastEmittedPosition = pos;

    // MPRIS2 spec says Position is not emitted via PropertiesChanged;
    // instead clients read it on demand. We only emit Seeked when there's
    // a discontinuity (jump of more than 2 seconds).
    if (deltaMicros > 2000000LL && oldPos >= 0) {
        Q_EMIT Seeked(pos);
    }
}

void Mpris2PlayerAdaptor::emitPropertyChanged(const QString &property, const QVariant &value)
{
    QVariantMap changed;
    changed[property] = value;
    emitMultiplePropertiesChanged(changed);
}

void Mpris2PlayerAdaptor::emitMultiplePropertiesChanged(const QVariantMap &properties)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        QStringLiteral("/org/mpris/MediaPlayer2"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("PropertiesChanged"));
    signal << QStringLiteral("org.mpris.MediaPlayer2.Player");
    signal << properties;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
}

// --- Mpris2Player (owner object) ---

Mpris2Player::Mpris2Player(PlayerController *controller, QObject *parent)
    : QObject(parent)
{
    auto *rootAdaptor = new Mpris2RootAdaptor(this);
    new Mpris2PlayerAdaptor(controller, this);

    connect(rootAdaptor, &Mpris2RootAdaptor::raiseRequested,
            this, &Mpris2Player::raiseRequested);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerService(QStringLiteral("org.mpris.MediaPlayer2.musicassistant_native"));
    bus.registerObject(QStringLiteral("/org/mpris/MediaPlayer2"), this);
}
