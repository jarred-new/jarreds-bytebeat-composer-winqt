#ifndef BYTEBEATGENERATOR_H
#define BYTEBEATGENERATOR_H

#include <QIODevice>
#include <QAudioFormat>
#include <QString>
#include <QTimer>
#include <cmath>
#include <cstring>

#define exprtk_disable_string_capabilities // Optimizes compilation size/speed
#include "exprtk.hpp"

class BytebeatDevice : public QIODevice {
    Q_OBJECT
public:
    explicit BytebeatDevice(QObject *parent = nullptr) : QIODevice(parent) {
        // Expose 't' to the expression parser
        symbol_table.add_variable("t", t_val);
        symbol_table.add_constants();
        expression.register_symbol_table(symbol_table);
    }

    bool setFormula(const QString &formulaStr) {
        exprtk::parser<double> parser;
        // Bytebeat often uses bitwise operators. ExprTk supports standard '&', '|', '^', '<<', '>>'
        if (!parser.compile(formulaStr.toStdString(), expression)) {
            return false; // Failed to parse formula
        }
        t_val = 0; // Reset time index on formula swap
        formula_ready = true;
        return true;
    }

    void setAudioFormat(const QAudioFormat &format) {
        audio_format = format;
    }

    bool isSequential() const override {
        return true;
    }

    qint64 bytesAvailable() const override {
        return 4096 * audio_format.bytesPerFrame() + QIODevice::bytesAvailable();
    }

    bool open(OpenMode mode) override {
        if (mode != QIODevice::ReadOnly || !formula_ready) {
            return false;
        }
        if (!QIODevice::open(mode)) {
            return false;
        }
        QTimer::singleShot(0, this, [this]() { emit readyRead(); });
        return true;
    }

protected:
    qint64 readData(char *data, qint64 maxlen) override {
        const qint64 bytesPerFrame = audio_format.bytesPerFrame();
        const int bytesPerSample = audio_format.bytesPerSample();
        const int channels = audio_format.channelCount();
        if (bytesPerFrame <= 0 || bytesPerSample <= 0 || channels <= 0) {
            return -1;
        }

        const qint64 frameCount = maxlen / bytesPerFrame;
        qint64 offset = 0;
        for (qint64 frame = 0; frame < frameCount; ++frame) {
            double result = expression.value();
            const double wrappedResult = std::isfinite(result)
                                             ? std::fmod(std::trunc(result), 256.0)
                                             : 0.0;
            const unsigned char sample = static_cast<unsigned char>(
                wrappedResult < 0 ? wrappedResult + 256.0 : wrappedResult);
            const double centeredSample = (static_cast<int>(sample) - 128) / 128.0;

            for (int channel = 0; channel < channels; ++channel) {
                switch (audio_format.sampleFormat()) {
                case QAudioFormat::UInt8:
                    data[offset] = static_cast<char>(sample);
                    break;
                case QAudioFormat::Int16: {
                    const qint16 pcmSample = static_cast<qint16>(centeredSample * 32768.0);
                    std::memcpy(data + offset, &pcmSample, sizeof(pcmSample));
                    break;
                }
                case QAudioFormat::Int32: {
                    const qint32 pcmSample = static_cast<qint32>(centeredSample * 2147483648.0);
                    std::memcpy(data + offset, &pcmSample, sizeof(pcmSample));
                    break;
                }
                case QAudioFormat::Float: {
                    const float pcmSample = static_cast<float>(centeredSample);
                    std::memcpy(data + offset, &pcmSample, sizeof(pcmSample));
                    break;
                }
                default:
                    return -1;
                }
                offset += bytesPerSample;
            }
            ++t_val;
        }
        return offset;
    }

    qint64 writeData(const char *data, qint64 len) override {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }

private:
    double t_val = 0;
    bool formula_ready = false;
    QAudioFormat audio_format;
    exprtk::symbol_table<double> symbol_table;
    exprtk::expression<double> expression;
};


#endif // BYTEBEATGENERATOR_H