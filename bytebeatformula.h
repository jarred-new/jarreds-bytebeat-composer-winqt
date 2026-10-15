#ifndef BYTEBEATFORMULA_H
#define BYTEBEATFORMULA_H

#include <QObject>

#include <QString>
#include <QMutex>

#include <string>
#include <stdint.h>

#include "exprtk.hpp"

class BytebeatFormula : public QObject
{
    Q_OBJECT
public:
    explicit BytebeatFormula(QObject *parent = nullptr);

    bool setFormula(const QString& formula);
    qint8 getFormula(quint32 currentT);

private:
    QMutex m_mutex;

    exprtk::symbol_table<double> symbols;
    exprtk::expression<double> expr;
    exprtk::parser<double> pars;

    double m_tVar = 0.0;
};

#endif // BYTEBEATFORMULA_H
