#include "bytebeatdevice.h"

BytebeatDevice::BytebeatDevice(BytebeatFormula* backend, QObject *parent)
    : QIODevice(parent), m_backend(backend) {}

bool BytebeatDevice::open(OpenMode mode) {
    if (mode != QIODevice::ReadOnly) return false;
    return QIODevice::open(mode);
}

void BytebeatDevice::close() {
    QIODevice::close();
}

qint64 BytebeatDevice::readData(char *data, qint64 maxlen) {
    if (!m_backend) return 0;

    for (qint64 i = 0; i < maxlen; ++i) {
        // Query the formula backend dynamically
        quint8 sample = m_backend->getFormula(t);

        data[i] = static_cast<char>(sample);
        t++;
    }
    return maxlen;
}

qint64 BytebeatDevice::writeData(const char *data, qint64 len) {
    Q_UNUSED(data); Q_UNUSED(len);
    return 0;
}
