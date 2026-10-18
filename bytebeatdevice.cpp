#include "bytebeatdevice.h"

#include <cstring>

BytebeatDevice::BytebeatDevice(BytebeatFormula* backend, QObject *parent)
    : QIODevice(parent), m_backend(backend) {}

void BytebeatDevice::setAudioFormat(const QAudioFormat& format) {
    m_format = format;
}

bool BytebeatDevice::open(OpenMode mode) {
    if (mode != QIODevice::ReadOnly) return false;
    return QIODevice::open(mode);
}

void BytebeatDevice::close() {
    QIODevice::close();
}

qint64 BytebeatDevice::readData(char *data, qint64 maxlen) {
    if (!m_backend || m_format.bytesPerFrame() == 0) return 0;

    const qint64 frameSize = m_format.bytesPerFrame();
    const qint64 frameCount = maxlen / frameSize;
    const int channelCount = m_format.channelCount();

    for (qint64 frame = 0; frame < frameCount; ++frame) {
        const int centeredSample = static_cast<int>(m_backend->getFormula(t++)) - 128;
        char* frameData = data + frame * frameSize;

        for (int channel = 0; channel < channelCount; ++channel) {
            char* sampleData = frameData + channel * m_format.bytesPerSample();
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
    }

    return frameCount * frameSize;
}

qint64 BytebeatDevice::writeData(const char *data, qint64 len) {
    Q_UNUSED(data); Q_UNUSED(len);
    return 0;
}
