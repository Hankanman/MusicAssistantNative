#pragma once

#include <QIODevice>
#include <QByteArray>
#include <QMutex>

/**
 * Sequential QIODevice for streaming audio data to QMediaPlayer.
 * Data is appended via feedData() and consumed by QMediaPlayer reads.
 * Reports as sequential so QMediaPlayer doesn't try to determine file size upfront.
 */
class StreamBuffer : public QIODevice
{
    Q_OBJECT

public:
    explicit StreamBuffer(QObject *parent = nullptr);

    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override;

    void feedData(const QByteArray &data);
    void finish();
    void resetBuffer();

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    QByteArray m_buffer;
    qint64 m_readPos = 0;
    mutable QMutex m_mutex;
    bool m_finished = false;
};
