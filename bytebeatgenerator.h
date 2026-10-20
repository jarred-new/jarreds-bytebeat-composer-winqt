#ifndef BYTEBEATGENERATOR_H
#define BYTEBEATGENERATOR_H

#include <QAudioFormat>
#include <QByteArray>
#include <QIODevice>
#include <QString>
#include <QTimer>

#include <cmath>
#include <cstring>
#include <string>

#include "exprtk.hpp"

class BytebeatGenerator : public QIODevice {
    Q_OBJECT
public:
    explicit BytebeatGenerator(QObject *parent = nullptr)
        : QIODevice(parent)
    {
        symbol_table.add_variable("t", t);
        symbol_table.add_constants();
        expression.register_symbol_table(symbol_table);
    }

    bool setFormula(const QString& formula)
    {
        if (!parser.compile(formula.toStdString(), expression)) {
            return false;
        }
        t = 0.0;
        return true;
    }

    void setAudioFormat(const QAudioFormat& format)
    {
        audio_format = format;
        pending_frame.resize(audio_format.bytesPerFrame());
        pending_frame_offset = pending_frame.size();
    }

    bool open(OpenMode mode) override {
        if (mode != QIODevice::ReadOnly || !audio_format.isValid()) return false;
        if (!QIODevice::open(mode)) return false;

        QTimer::singleShot(0, this, [this]() { emit readyRead(); });
        return true;
    }

    void close() override {
        QIODevice::close();
    }

    qint64 readData(char *data, qint64 maxlen) override {
        const qint64 frame_size = audio_format.bytesPerFrame();
        if (maxlen <= 0 || frame_size <= 0 || audio_format.channelCount() <= 0) return 0;

        qint64 bytes_written = 0;
        while (bytes_written < maxlen) {
            if (pending_frame_offset >= frame_size) {
                const int centered_sample = static_cast<int>(nextSample()) - 128;
                for (int channel = 0; channel < audio_format.channelCount(); ++channel) {
                    char* sample_data = pending_frame.data() + channel * audio_format.bytesPerSample();
                    switch (audio_format.sampleFormat()) {
                    case QAudioFormat::UInt8: {
                        const quint8 sample = static_cast<quint8>(centered_sample + 128);
                        std::memcpy(sample_data, &sample, sizeof(sample));
                        break;
                    }
                    case QAudioFormat::Int16: {
                        const qint16 sample = static_cast<qint16>(centered_sample * 256);
                        std::memcpy(sample_data, &sample, sizeof(sample));
                        break;
                    }
                    case QAudioFormat::Int32: {
                        const qint32 sample = static_cast<qint32>(centered_sample) * 16777216;
                        std::memcpy(sample_data, &sample, sizeof(sample));
                        break;
                    }
                    case QAudioFormat::Float: {
                        const float sample = static_cast<float>(centered_sample) / 128.0f;
                        std::memcpy(sample_data, &sample, sizeof(sample));
                        break;
                    }
                    case QAudioFormat::Unknown:
                        std::memset(sample_data, 0, static_cast<size_t>(audio_format.bytesPerSample()));
                        break;
                    }
                }
                pending_frame_offset = 0;
            }

            const qint64 bytes_to_copy = qMin(maxlen - bytes_written, frame_size - pending_frame_offset);
            std::memcpy(data + bytes_written, pending_frame.constData() + pending_frame_offset,
                        static_cast<size_t>(bytes_to_copy));
            bytes_written += bytes_to_copy;
            pending_frame_offset += bytes_to_copy;
        }

        return bytes_written;
    }

    qint64 writeData(const char *data, qint64 len) override {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0; // Read-only device
    }

    bool isSequential() const override { return true; }

private:
    quint8 nextSample()
    {
        const double result = expression.value();
        t += 1.0;
        if (!std::isfinite(result)) return 0;

        double wrapped = std::fmod(std::trunc(result), 256.0);
        if (wrapped < 0.0) wrapped += 256.0;
        return static_cast<quint8>(wrapped);
    }

    double t = 0.0;
    exprtk::symbol_table<double> symbol_table;
    exprtk::expression<double> expression;
    exprtk::parser<double>       parser;
    QAudioFormat audio_format;
    QByteArray pending_frame;
    qint64 pending_frame_offset = 0;
};

#endif // BYTEBEATGENERATOR_H