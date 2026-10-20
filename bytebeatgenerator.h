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

#include <QIODevice>
#include <QString>
#include <string>
#include <cmath>
#define exprtk_disable_string_capabilities // Optimizes compilation size/speed
#include "exprtk.hpp"

class BytebeatDevice : public QIODevice {
    Q_OBJECT
public:
    BytebeatDevice(QObject *parent = nullptr) : QIODevice(parent) {
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
        return true;
    }

protected:
    qint64 readData(char *data, qint64 maxlen) override {
        // maxlen is in bytes. For 8-bit mono audio, 1 byte = 1 sample.
        for (qint64 i = 0; i < maxlen; ++i) {
            // Evaluate expression using current t value
            double result = expression.value();
            
            // Bytebeat output is traditionally an 8-bit unsigned integer (0-255)
            // Cast the double evaluation safely down to standard byte boundaries
            unsigned char sample = static_cast<unsigned char>(static_cast<unsigned long long>(result) & 0xFF);
            
            data[i] = static_cast<char>(sample);
            t_val++; // Increment time index step
        }
        return maxlen;
    }

    qint64 writeData(const char *data, qint64 len) override {
        Q_UNUSED(data);
        return len; // Read-only device stream
    }

private:
    double t_val = 0;
    exprtk::symbol_table<double> symbol_table;
    exprtk::expression<double> expression;
};


#endif // BYTEBEATGENERATOR_H