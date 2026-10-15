#ifndef BYTEBEATDEVICE_H
#define BYTEBEATDEVICE_H

#include <QIODevice>
#include "bytebeatformula.h"

class BytebeatDevice : public QIODevice {
    Q_OBJECT
public:
    // Pass our backend object pointer into the device loop
    explicit BytebeatDevice(BytebeatFormula* backend, QObject *parent = nullptr);

    bool open(OpenMode mode) override;
    void close() override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    bool isSequential() const override { return true; }

private:
    BytebeatFormula* m_backend;
    quint32 t = 0;
};

#endif // BYTEBEATDEVICE_H
