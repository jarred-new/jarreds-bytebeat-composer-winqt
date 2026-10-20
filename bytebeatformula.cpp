#include "bytebeatformula.h"

#include <cmath>

// CONSTRUCTOR
BytebeatFormula::BytebeatFormula(QObject *parent)
    : QObject{parent}
{
    symbols.add_variable("t", this->m_tVar);
    symbols.add_constants();
    expr.register_symbol_table(this->symbols);

    setFormula("t"); // default formula
}

// setFormula()
bool BytebeatFormula::setFormula(const QString &formula)
{
    QMutexLocker locker(&this->m_mutex);

    std::string formulaStd = formula.toStdString(); // convert to std::string

    if (!pars.compile(formulaStd, this->expr)) {
        return false; // error!
    }

    return true; // compiled success!
}

// getFormula()
quint8 BytebeatFormula::getFormula(quint32 currentT)
{
    QMutexLocker locker(&m_mutex);

    // Update the bound reference variable
    m_tVar = static_cast<double>(currentT);

    // Wrap signed bytebeat expressions into the unsigned 8-bit sample range.
    double result = expr.value();
    if (!std::isfinite(result)) {
        return 0;
    }

    double wrapped = std::fmod(std::trunc(result), 256.0);
    if (wrapped < 0.0) {
        wrapped += 256.0;
    }
    return static_cast<quint8>(wrapped);
}
