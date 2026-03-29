#include "audiodecoder.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

AudioDecoder::AudioDecoder(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.8f);

    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &AudioDecoder::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::errorOccurred, this,
            [](QMediaPlayer::Error err, const QString &msg) {
                qWarning() << "AudioDecoder: error:" << err << msg;
            });
}

AudioDecoder::~AudioDecoder()
{
    stop();
}

void AudioDecoder::start(const QString &codec, int sampleRate, int channels, int bitDepth)
{
    Q_UNUSED(channels)
    Q_UNUSED(bitDepth)
    stop();

    m_codec = codec;
    m_sampleRate = sampleRate;
    m_bytesWritten = 0;
    m_playbackStarted = false;

    // Create temp file for the audio stream
    QString suffix = codec == QStringLiteral("flac") ? QStringLiteral(".flac")
                   : codec == QStringLiteral("opus") ? QStringLiteral(".opus")
                   : QStringLiteral(".raw");

    m_tempFile = new QTemporaryFile(QDir::tempPath() + QStringLiteral("/ma_audio_XXXXXX") + suffix, this);
    m_tempFile->setAutoRemove(true);
    if (!m_tempFile->open()) {
        qWarning() << "AudioDecoder: failed to create temp file:" << m_tempFile->errorString();
        delete m_tempFile;
        m_tempFile = nullptr;
        return;
    }

    m_playing = true;
}

void AudioDecoder::stop()
{
    m_player->stop();

    if (m_tempFile) {
        m_tempFile->close();
        delete m_tempFile;
        m_tempFile = nullptr;
    }

    m_playbackStarted = false;

    if (m_playing) {
        m_playing = false;
        Q_EMIT playbackStopped();
    }
}

void AudioDecoder::feedData(const QByteArray &encodedData)
{
    if (!m_tempFile || !m_tempFile->isOpen()) return;

    m_tempFile->write(encodedData);
    m_tempFile->flush();
    m_bytesWritten += encodedData.size();

    // Start playback after accumulating ~100KB of data (enough for smooth start)
    if (!m_playbackStarted && m_bytesWritten > 100000) {
        startPlaybackIfReady();
    }
}

void AudioDecoder::startPlaybackIfReady()
{
    if (m_playbackStarted || !m_tempFile) return;
    m_playbackStarted = true;
    m_player->setSource(QUrl::fromLocalFile(m_tempFile->fileName()));
    m_player->play();
    Q_EMIT playbackStarted();
}

void AudioDecoder::setVolume(float vol)
{
    m_audioOutput->setVolume(vol);
}

bool AudioDecoder::isPlaying() const
{
    return m_playing;
}

void AudioDecoder::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::InvalidMedia) {
        qWarning() << "AudioDecoder: invalid media - codec:" << m_codec;
    }
}
