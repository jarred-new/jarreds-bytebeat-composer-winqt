#include "bytebeatdevice.h"

#include <cstring>
#include <QTimer>

BytebeatDevice::BytebeatDevice(BytebeatFormula* backend, QObject *parent)
    : QIODevice(parent), m_backend(backend) {}

void BytebeatDevice::setAudioFormat(const QAudioFormat& format) {
    m_format = format;
    m_pendingFrame.resize(m_format.bytesPerFrame());
    m_pendingFrameOffset = m_pendingFrame.size();
}

bool BytebeatDevice::open(OpenMode mode) {
    if (mode != QIODevice::ReadOnly) return false;
    if (!m_backend || m_format.bytesPerFrame() <= 0) return false;
    if (!QIODevice::open(mode)) return false;

    QTimer::singleShot(0, this, [this]() { emit readyRead(); });
    return true;
}

void BytebeatDevice::close() {
    QIODevice::close();
}

qint64 BytebeatDevice::readData(char *data, qint64 maxlen) {
    if (!m_backend || maxlen <= 0 || m_format.bytesPerFrame() <= 0) return 0;

    const qint64 frameSize = m_format.bytesPerFrame();
    const int channelCount = m_format.channelCount();
    qint64 bytesWritten = 0;

    while (bytesWritten < maxlen) {
        if (m_pendingFrameOffset >= frameSize) {
            const int centeredSample = static_cast<int>(m_backend->getFormula(t++)) - 128;

            for (int channel = 0; channel < channelCount; ++channel) {
                char* sampleData = m_pendingFrame.data() + channel * m_format.bytesPerSample();
                switch (m_format.sampleFormat()) {
                case QAudioFormat::UInt8: {
                    const quint8 sample = static_cast<quint8>(centeredSample + 128);
                    std::memcpy(sampleData, &sample, sizeof(sample));
                    break;
                }
                case QAudioFormat::Int16: {
                    const qint16 sample = static_cast<qint16>(centeredSample * 256);
                    std::memcpy(sampleData, &sample, sizeof(sample));
                    break;
                }
                case QAudioFormat::Int32: {
                    const qint32 sample = static_cast<qint32>(centeredSample) * 16777216;
                    std::memcpy(sampleData, &sample, sizeof(sample));
                    break;
                }
                case QAudioFormat::Float: {
                    const float sample = static_cast<float>(centeredSample) / 128.0f;
                    std::memcpy(sampleData, &sample, sizeof(sample));
                    break;
                }
                case QAudioFormat::Unknown:
                    std::memset(sampleData, 0, static_cast<size_t>(m_format.bytesPerSample()));
                    break;
                }
            }

            m_pendingFrameOffset = 0;
        }

        const qint64 bytesToCopy = qMin(maxlen - bytesWritten, frameSize - m_pendingFrameOffset);
        std::memcpy(data + bytesWritten, m_pendingFrame.constData() + m_pendingFrameOffset,
                    static_cast<size_t>(bytesToCopy));
        bytesWritten += bytesToCopy;
        m_pendingFrameOffset += bytesToCopy;
    }

    return bytesWritten;
}

qint64 BytebeatDevice::writeData(const char *data, qint64 len) {
    Q_UNUSED(data); Q_UNUSED(len);
    return 0;
}
