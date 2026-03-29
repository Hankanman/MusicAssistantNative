#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QStringList>
#include <QVariantMap>

class PlayerController;

class Mpris2RootAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

    Q_PROPERTY(bool CanQuit READ canQuit)
    Q_PROPERTY(bool CanRaise READ canRaise)
    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    Q_PROPERTY(QString Identity READ identity)
    Q_PROPERTY(QString DesktopEntry READ desktopEntry)

public:
    explicit Mpris2RootAdaptor(QObject *parent);

    bool canQuit() const { return true; }
    bool canRaise() const { return true; }
    bool hasTrackList() const { return false; }
    QString identity() const { return QStringLiteral("Music Assistant Native"); }
    QString desktopEntry() const { return QStringLiteral("io.github.musicassistant.native"); }

public Q_SLOTS:
    void Raise();
    void Quit();

Q_SIGNALS:
    void raiseRequested();
};

class Mpris2PlayerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double MinimumRate READ minimumRate)
    Q_PROPERTY(double MaximumRate READ maximumRate)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPause)
    Q_PROPERTY(bool CanGoNext READ canGoNext)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ canControl)

public:
    explicit Mpris2PlayerAdaptor(PlayerController *controller, QObject *parent);

    QString playbackStatus() const;
    QVariantMap metadata() const;
    double volume() const;
    void setVolume(double vol);
    qlonglong position() const;
    double minimumRate() const { return 1.0; }
    double maximumRate() const { return 1.0; }
    double rate() const { return 1.0; }
    void setRate(double) {}
    bool canPlay() const { return true; }
    bool canPause() const { return true; }
    bool canGoNext() const { return true; }
    bool canGoPrevious() const { return true; }
    bool canSeek() const { return true; }
    bool canControl() const { return true; }

public Q_SLOTS:
    void Play();
    void Pause();
    void PlayPause();
    void Stop();
    void Next();
    void Previous();
    void Seek(qlonglong offset);
    void SetPosition(const QDBusObjectPath &trackId, qlonglong position);

Q_SIGNALS:
    void Seeked(qlonglong position);

private Q_SLOTS:
    void onPlayerStateChanged();
    void onElapsedChanged();

private:
    void emitPropertyChanged(const QString &property, const QVariant &value);
    void emitMultiplePropertiesChanged(const QVariantMap &properties);

    PlayerController *m_controller;
    QString m_lastPlaybackStatus;
    QVariantMap m_lastMetadata;
    double m_lastVolume = -1.0;
    qlonglong m_lastEmittedPosition = -1;
};

class Mpris2Player : public QObject
{
    Q_OBJECT

public:
    explicit Mpris2Player(PlayerController *controller, QObject *parent = nullptr);

Q_SIGNALS:
    void raiseRequested();
};
