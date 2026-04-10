#include "streambuffer.h"

StreamBuffer::StreamBuffer(QObject *parent)
    : QIODevice(parent)
{
}

qint64 StreamBuffer::bytesAvailable() const
{
    QMutexLocker lock(&m_mutex);
    return (m_buffer.size() - m_readPos) + QIODevice::bytesAvailable();
}

void StreamBuffer::feedData(const QByteArray &data)
{
    QMutexLocker lock(&m_mutex);
    m_buffer.append(data);
    lock.unlock();
    Q_EMIT readyRead();
}

void StreamBuffer::finish()
{
    QMutexLocker lock(&m_mutex);
    m_finished = true;
    lock.unlock();
    Q_EMIT readyRead();
}

void StreamBuffer::resetBuffer()
{
    QMutexLocker lock(&m_mutex);
    m_buffer.clear();
    m_readPos = 0;
    m_finished = false;
}

qint64 StreamBuffer::readData(char *data, qint64 maxSize)
{
    QMutexLocker lock(&m_mutex);
    qint64 available = m_buffer.size() - m_readPos;

    if (available <= 0) {
        // If stream is finished, signal EOF
        if (m_finished)
            return -1;
        // No data yet — QAudioSink will retry on readyRead
        return 0;
    }

    qint64 toRead = qMin(maxSize, available);
    memcpy(data, m_buffer.constData() + m_readPos, toRead);
    m_readPos += toRead;

    // Compact buffer periodically to avoid unbounded growth
    if (m_readPos > 1024 * 1024) {
        m_buffer.remove(0, m_readPos);
        m_readPos = 0;
    }

    return toRead;
}

qint64 StreamBuffer::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1; // Not used — feedData() is the public API
}
