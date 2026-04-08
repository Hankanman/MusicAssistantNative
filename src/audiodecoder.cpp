#include "audiodecoder.h"
#include "streambuffer.h"
#include <QDebug>

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

    m_streamBuffer = new StreamBuffer(this);
    m_streamBuffer->open(QIODevice::ReadOnly);

    m_playing = true;
}

void AudioDecoder::stop()
{
    m_player->stop();
    m_player->setSource(QUrl());

    if (m_streamBuffer) {
        m_streamBuffer->finish();
        m_streamBuffer->close();
        m_streamBuffer->deleteLater();
        m_streamBuffer = nullptr;
    }

    m_playbackStarted = false;

    if (m_playing) {
        m_playing = false;
        Q_EMIT playbackStopped();
    }
}

void AudioDecoder::feedData(const QByteArray &encodedData)
{
    if (!m_streamBuffer) return;

    m_streamBuffer->feedData(encodedData);
    m_bytesWritten += encodedData.size();

    // Start playback after accumulating ~100KB of data (enough for smooth start)
    if (!m_playbackStarted && m_bytesWritten > 100000) {
        startPlaybackIfReady();
    }
}

void AudioDecoder::startPlaybackIfReady()
{
    if (m_playbackStarted || !m_streamBuffer) return;
    m_playbackStarted = true;
    m_player->setSourceDevice(m_streamBuffer);
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
    } else if (status == QMediaPlayer::EndOfMedia) {
        qDebug() << "AudioDecoder: end of media reached";
    }
}
