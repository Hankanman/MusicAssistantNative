#pragma once

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QByteArray>
#include <QMutex>
#include <FLAC/stream_decoder.h>

class StreamBuffer;

class AudioDecoder : public QObject
{
    Q_OBJECT

public:
    explicit AudioDecoder(QObject *parent = nullptr);
    ~AudioDecoder() override;

    void start(const QString &codec, int sampleRate, int channels, int bitDepth);
    void stop();
    void feedData(const QByteArray &encodedData);
    void setVolume(float vol);
    bool isPlaying() const;

Q_SIGNALS:
    void playbackStarted();
    void playbackStopped();

private:
    void processAvailableData();
    void initFlacDecoder();
    void cleanupFlacDecoder();

    // libFLAC callbacks
    static FLAC__StreamDecoderReadStatus flacReadCallback(
        const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
        size_t *bytes, void *clientData);
    static FLAC__StreamDecoderWriteStatus flacWriteCallback(
        const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
        const FLAC__int32 *const buffer[], void *clientData);
    static void flacErrorCallback(
        const FLAC__StreamDecoder *decoder,
        FLAC__StreamDecoderErrorStatus status, void *clientData);

    FLAC__StreamDecoder *m_flacDecoder = nullptr;
    QAudioSink *m_audioSink = nullptr;
    StreamBuffer *m_pcmBuffer = nullptr;
    float m_volume = 0.8f;

    // FLAC input buffer (fed by WebSocket frames)
    QByteArray m_flacInput;
    qint64 m_flacReadPos = 0;
    QMutex m_inputMutex;

    int m_sampleRate = 44100;
    int m_channels = 2;
    int m_bitDepth = 16;
    bool m_playing = false;
};
