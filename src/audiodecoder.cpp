#include "audiodecoder.h"
#include <QDebug>

AudioDecoder::AudioDecoder(QObject *parent)
    : QObject(parent)
{
}

AudioDecoder::~AudioDecoder()
{
    stop();
}

void AudioDecoder::start(const QString &codec, int sampleRate, int channels, int bitDepth)
{
    stop();

    qDebug() << "AudioDecoder: starting - codec:" << codec
             << "rate:" << sampleRate << "ch:" << channels << "bits:" << bitDepth;

    // Set up audio output format
    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleFormat(bitDepth == 24 ? QAudioFormat::Int32 : QAudioFormat::Int16);

    auto device = QMediaDevices::defaultAudioOutput();
    if (!device.isFormatSupported(format)) {
        qDebug() << "AudioDecoder: format not supported by device, trying default";
        format = device.preferredFormat();
    }

    m_audioSink = new QAudioSink(device, format, this);
    m_audioSink->setVolume(m_volume);
    m_audioSink->setBufferSize(sampleRate * channels * (bitDepth / 8) * 2); // 2 seconds buffer

    // Start ffmpeg to decode FLAC stream to raw PCM
    m_decoder = new QProcess(this);
    connect(m_decoder, &QProcess::readyReadStandardOutput, this, &AudioDecoder::onDecoderReadyRead);
    connect(m_decoder, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &AudioDecoder::onDecoderFinished);

    // ffmpeg reads FLAC from stdin, outputs raw PCM to stdout
    QString sampleFmt = bitDepth == 24 ? QStringLiteral("s32le") : QStringLiteral("s16le");
    QStringList args = {
        QStringLiteral("-hide_banner"),
        QStringLiteral("-loglevel"), QStringLiteral("error"),
        QStringLiteral("-f"), codec,           // input format (flac)
        QStringLiteral("-i"), QStringLiteral("pipe:0"),  // read from stdin
        QStringLiteral("-f"), sampleFmt,       // output raw PCM
        QStringLiteral("-ar"), QString::number(sampleRate),
        QStringLiteral("-ac"), QString::number(channels),
        QStringLiteral("pipe:1")               // write to stdout
    };

    qDebug() << "AudioDecoder: launching ffmpeg" << args;
    m_decoder->start(QStringLiteral("ffmpeg"), args);
    if (!m_decoder->waitForStarted(3000)) {
        qDebug() << "AudioDecoder: ERROR - ffmpeg failed to start:" << m_decoder->errorString();
        delete m_decoder;
        m_decoder = nullptr;
        return;
    }

    // Start audio output
    m_audioDevice = m_audioSink->start();
    if (!m_audioDevice) {
        qDebug() << "AudioDecoder: ERROR - audio sink failed to start";
        m_decoder->kill();
        delete m_decoder;
        m_decoder = nullptr;
        return;
    }

    m_playing = true;
    Q_EMIT playbackStarted();
    qDebug() << "AudioDecoder: pipeline started (ffmpeg PID:" << m_decoder->processId() << ")";
}

void AudioDecoder::stop()
{
    if (m_decoder) {
        m_decoder->closeWriteChannel();
        m_decoder->waitForFinished(1000);
        if (m_decoder->state() != QProcess::NotRunning)
            m_decoder->kill();
        delete m_decoder;
        m_decoder = nullptr;
    }

    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    m_audioDevice = nullptr;

    if (m_playing) {
        m_playing = false;
        Q_EMIT playbackStopped();
    }
}

void AudioDecoder::feedData(const QByteArray &flacData)
{
    if (m_decoder && m_decoder->state() == QProcess::Running) {
        m_decoder->write(flacData);
    }
}

void AudioDecoder::setVolume(float vol)
{
    m_volume = vol;
    if (m_audioSink)
        m_audioSink->setVolume(vol);
}

bool AudioDecoder::isPlaying() const
{
    return m_playing;
}

void AudioDecoder::onDecoderReadyRead()
{
    if (!m_audioDevice || !m_decoder) return;

    QByteArray pcmData = m_decoder->readAllStandardOutput();
    if (!pcmData.isEmpty()) {
        m_audioDevice->write(pcmData);
    }
}

void AudioDecoder::onDecoderFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(status)
    // Flush any remaining data
    if (m_audioDevice && m_decoder) {
        QByteArray remaining = m_decoder->readAllStandardOutput();
        if (!remaining.isEmpty())
            m_audioDevice->write(remaining);
    }
    qDebug() << "AudioDecoder: ffmpeg exited with code" << exitCode;
}
