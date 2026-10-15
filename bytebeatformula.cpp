#include "bytebeatformula.h"

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
qint8 BytebeatFormula::getFormula(quint32 currentT)
{
    QMutexLocker locker(&m_mutex);

    // Update the bound reference variable
    m_tVar = static_cast<double>(currentT);

    // Compute the expression and clamp it safely down to raw 8-bit PCM (0-255)
    double result = m_expression.value();
    return static_cast<quint8>(static_cast<quint32>(result) & 0xFF);
}
