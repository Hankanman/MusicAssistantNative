#pragma once

#include <QObject>
#include <QProcess>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QBuffer>
#include <QIODevice>

class AudioDecoder : public QObject
{
    Q_OBJECT

public:
    explicit AudioDecoder(QObject *parent = nullptr);
    ~AudioDecoder() override;

    void start(const QString &codec, int sampleRate, int channels, int bitDepth);
    void stop();
    void feedData(const QByteArray &flacData);
    void setVolume(float vol);

    bool isPlaying() const;

Q_SIGNALS:
    void playbackStarted();
    void playbackStopped();

private Q_SLOTS:
    void onDecoderReadyRead();
    void onDecoderFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess *m_decoder = nullptr;
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;
    float m_volume = 0.8f;
    bool m_playing = false;
};
