#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>

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

private Q_SLOTS:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void startPlaybackIfReady();

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    StreamBuffer *m_streamBuffer = nullptr;
    QString m_codec;
    int m_sampleRate = 44100;
    bool m_playing = false;
    bool m_playbackStarted = false;
    int m_bytesWritten = 0;
};
