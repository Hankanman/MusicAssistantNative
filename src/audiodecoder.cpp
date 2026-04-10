#include "audiodecoder.h"
#include "streambuffer.h"
#include <QDebug>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QtEndian>

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
    Q_UNUSED(codec) // Always FLAC from Sendspin
    stop();

    m_sampleRate = sampleRate;
    m_channels = channels;
    m_bitDepth = bitDepth;

    // Clear FLAC input buffer
    m_flacInput.clear();
    m_flacReadPos = 0;

    // Create PCM output buffer
    m_pcmBuffer = new StreamBuffer(this);
    m_pcmBuffer->open(QIODevice::ReadOnly);

    // Set up audio format for QAudioSink
    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleFormat(QAudioFormat::Int16);

    m_audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);
    m_audioSink->setVolume(m_volume);
    m_audioSink->start(m_pcmBuffer);

    // Init FLAC decoder
    initFlacDecoder();

    m_playing = true;
    qDebug() << "AudioDecoder: started - rate:" << sampleRate << "ch:" << channels;
}

void AudioDecoder::stop()
{
    m_playing = false;

    cleanupFlacDecoder();

    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    if (m_pcmBuffer) {
        m_pcmBuffer->finish();
        m_pcmBuffer->close();
        delete m_pcmBuffer;
        m_pcmBuffer = nullptr;
    }

    m_flacInput.clear();
    m_flacReadPos = 0;
}

void AudioDecoder::initFlacDecoder()
{
    m_flacDecoder = FLAC__stream_decoder_new();
    if (!m_flacDecoder) {
        qWarning() << "AudioDecoder: failed to create FLAC decoder";
        return;
    }

    FLAC__StreamDecoderInitStatus status = FLAC__stream_decoder_init_stream(
        m_flacDecoder,
        flacReadCallback,
        nullptr,  // seek
        nullptr,  // tell
        nullptr,  // length
        nullptr,  // eof
        flacWriteCallback,
        nullptr,  // metadata
        flacErrorCallback,
        this      // client_data
    );

    if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        qWarning() << "AudioDecoder: FLAC init failed:" << FLAC__StreamDecoderInitStatusString[status];
        FLAC__stream_decoder_delete(m_flacDecoder);
        m_flacDecoder = nullptr;
    }
}

void AudioDecoder::cleanupFlacDecoder()
{
    if (m_flacDecoder) {
        FLAC__stream_decoder_finish(m_flacDecoder);
        FLAC__stream_decoder_delete(m_flacDecoder);
        m_flacDecoder = nullptr;
    }
}

void AudioDecoder::feedData(const QByteArray &encodedData)
{
    if (!m_flacDecoder || !m_playing) return;

    {
        QMutexLocker lock(&m_inputMutex);
        m_flacInput.append(encodedData);
    }

    processAvailableData();
}

void AudioDecoder::processAvailableData()
{
    if (!m_flacDecoder) return;

    // Process FLAC frames as long as we have input data
    while (m_playing && m_flacDecoder) {
        qint64 available;
        {
            QMutexLocker lock(&m_inputMutex);
            available = m_flacInput.size() - m_flacReadPos;
        }

        if (available <= 0) break;

        bool ok = FLAC__stream_decoder_process_single(m_flacDecoder);
        if (!ok) {
            FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(m_flacDecoder);
            if (state == FLAC__STREAM_DECODER_END_OF_STREAM) {
                break;
            }
            if (state == FLAC__STREAM_DECODER_ABORTED) {
                // Aborted because read_callback had no data — that's fine, we'll retry on next feedData
                FLAC__stream_decoder_flush(m_flacDecoder);
                break;
            }
            qWarning() << "AudioDecoder: FLAC process error, state:" << FLAC__StreamDecoderStateString[state];
            break;
        }
    }

    // Compact input buffer periodically
    QMutexLocker lock(&m_inputMutex);
    if (m_flacReadPos > 512 * 1024) {
        m_flacInput.remove(0, m_flacReadPos);
        m_flacReadPos = 0;
    }
}

void AudioDecoder::setVolume(float vol)
{
    m_volume = vol;
    if (m_audioSink) {
        m_audioSink->setVolume(vol);
    }
}

bool AudioDecoder::isPlaying() const
{
    return m_playing;
}

// --- FLAC callbacks ---

FLAC__StreamDecoderReadStatus AudioDecoder::flacReadCallback(
    const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
    size_t *bytes, void *clientData)
{
    Q_UNUSED(decoder)
    auto *self = static_cast<AudioDecoder *>(clientData);

    QMutexLocker lock(&self->m_inputMutex);
    qint64 available = self->m_flacInput.size() - self->m_flacReadPos;

    if (available <= 0) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    size_t toRead = qMin(static_cast<qint64>(*bytes), available);
    memcpy(buffer, self->m_flacInput.constData() + self->m_flacReadPos, toRead);
    self->m_flacReadPos += toRead;
    *bytes = toRead;

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderWriteStatus AudioDecoder::flacWriteCallback(
    const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
    const FLAC__int32 *const buffer[], void *clientData)
{
    Q_UNUSED(decoder)
    auto *self = static_cast<AudioDecoder *>(clientData);

    if (!self->m_pcmBuffer || !self->m_playing) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    const unsigned samples = frame->header.blocksize;
    const unsigned channels = frame->header.channels;
    const unsigned bps = frame->header.bits_per_sample;

    // Convert to interleaved 16-bit PCM
    QByteArray pcm;
    pcm.resize(samples * channels * 2);
    qint16 *out = reinterpret_cast<qint16 *>(pcm.data());

    for (unsigned i = 0; i < samples; ++i) {
        for (unsigned ch = 0; ch < channels; ++ch) {
            qint32 sample = buffer[ch][i];
            // Shift to 16-bit if source is higher/lower bit depth
            if (bps > 16) {
                sample >>= (bps - 16);
            } else if (bps < 16) {
                sample <<= (16 - bps);
            }
            *out++ = static_cast<qint16>(qBound(-32768, static_cast<int>(sample), 32767));
        }
    }

    self->m_pcmBuffer->feedData(pcm);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void AudioDecoder::flacErrorCallback(
    const FLAC__StreamDecoder *decoder,
    FLAC__StreamDecoderErrorStatus status, void *clientData)
{
    Q_UNUSED(decoder)
    Q_UNUSED(clientData)
    qWarning() << "AudioDecoder: FLAC decode error:" << FLAC__StreamDecoderErrorStatusString[status];
}
