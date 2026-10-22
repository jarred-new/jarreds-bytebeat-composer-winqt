#ifndef CUSTOPERATORS_H
#define CUSTOPERATORS_H

#include <cmath>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#define exprtk_disable_string_capabilities // Optimizes compilation size/speed
#include "exprtk.hpp"

// Custom operators for exprtk library to support unsigned right shift and bitwise NOT operations.
class BytebeatUnsignedRightShift : public exprtk::ifunction<double> {
public:
    BytebeatUnsignedRightShift() : exprtk::ifunction<double>(2) {}

    double operator()(const double &value, const double &shift) override {
        const std::uint32_t shiftAmount = toUint32(shift) & 31u;
        return static_cast<double>(toUint32(value) >> shiftAmount);
    }

private:
    static std::uint32_t toUint32(double value) {
        if (!std::isfinite(value) || value == 0.0) {
            return 0;
        }

        double modulo = std::fmod(std::trunc(value), 4294967296.0);
        if (modulo < 0.0) {
            modulo += 4294967296.0;
        }
        return static_cast<std::uint32_t>(modulo);
    }
};

class BytebeatBitwiseNot : public exprtk::ifunction<double> {
public:
    BytebeatBitwiseNot() : exprtk::ifunction<double>(1) {}

    double operator()(const double &value) override {
        const std::uint32_t bits = ~toUint32(value);
        if (bits <= static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
            return static_cast<double>(bits);
        }
        return static_cast<double>(bits) - 4294967296.0;
    }

private:
    static std::uint32_t toUint32(double value) {
        if (!std::isfinite(value) || value == 0.0) {
            return 0;
        }

        double modulo = std::fmod(std::trunc(value), 4294967296.0);
        if (modulo < 0.0) {
            modulo += 4294967296.0;
        }
        return static_cast<std::uint32_t>(modulo);
    }
};

class BytebeatBitwiseBinary : public exprtk::ifunction<double> {
public:
    enum class Operation { And, Or, Xor, LeftShift, RightShift, UnsignedRightShift };

    explicit BytebeatBitwiseBinary(Operation operation)
        : exprtk::ifunction<double>(2), operation(operation) {}

    double operator()(const double &left, const double &right) override {
        const std::uint32_t leftBits = toUint32(left);
        const std::uint32_t shift = toUint32(right) & 31u;
        std::uint32_t result = 0;

        switch (operation) {
        case Operation::And:
            result = leftBits & toUint32(right);
            break;
        case Operation::Or:
            result = leftBits | toUint32(right);
            break;
        case Operation::Xor:
            result = leftBits ^ toUint32(right);
            break;
        case Operation::LeftShift:
            result = leftBits << shift;
            break;
        case Operation::RightShift:
            result = leftBits >> shift;
            if ((leftBits & 0x80000000u) && shift != 0) {
                result |= 0xFFFFFFFFu << (32u - shift);
            }
            break;
        case Operation::UnsignedRightShift:
            result = leftBits >> shift;
            return static_cast<double>(result);
        }

        if (result <= static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
            return static_cast<double>(result);
        }
        return static_cast<double>(result) - 4294967296.0;
    }

private:
    static std::uint32_t toUint32(double value) {
        if (!std::isfinite(value) || value == 0.0) {
            return 0;
        }

        double modulo = std::fmod(std::trunc(value), 4294967296.0);
        if (modulo < 0.0) {
            modulo += 4294967296.0;
        }
        return static_cast<std::uint32_t>(modulo);
    }

    Operation operation;
};

class BytebeatFormulaTranslator {
public:
    bool translate(const std::string &formula, std::string &translated) {
        tokens.clear();
        position = 0;
        if (!tokenize(formula) || !parseConditional(translated) || current().kind != TokenKind::End) {
            return false;
        }
        return true;
    }

private:
    enum class TokenKind { Number, Identifier, Operator, LeftParen, RightParen, Comma, End };

    struct Token {
        TokenKind kind;
        std::string text;
    };

    bool tokenize(const std::string &formula) {
        for (std::size_t i = 0; i < formula.size();) {
            const unsigned char character = static_cast<unsigned char>(formula[i]);
            if (std::isspace(character)) {
                ++i;
                continue;
            }

            if (std::isdigit(character) || (formula[i] == '.' && i + 1 < formula.size()
                                             && std::isdigit(static_cast<unsigned char>(formula[i + 1])))) {
                const std::size_t start = i++;
                if (formula[start] == '0' && i < formula.size()
                    && (formula[i] == 'x' || formula[i] == 'X')) {
                    ++i;
                    while (i < formula.size() && std::isxdigit(static_cast<unsigned char>(formula[i]))) {
                        ++i;
                    }
                } else {
                    while (i < formula.size()
                           && (std::isdigit(static_cast<unsigned char>(formula[i])) || formula[i] == '.')) {
                        ++i;
                    }
                    if (i < formula.size() && (formula[i] == 'e' || formula[i] == 'E')) {
                        ++i;
                        if (i < formula.size() && (formula[i] == '+' || formula[i] == '-')) {
                            ++i;
                        }
                        while (i < formula.size() && std::isdigit(static_cast<unsigned char>(formula[i]))) {
                            ++i;
                        }
                    }
                }
                tokens.push_back({TokenKind::Number, formula.substr(start, i - start)});
                continue;
            }

            if (std::isalpha(character) || formula[i] == '_') {
                const std::size_t start = i++;
                while (i < formula.size()) {
                    const unsigned char next = static_cast<unsigned char>(formula[i]);
                    if (!std::isalnum(next) && formula[i] != '_') {
                        break;
                    }
                    ++i;
                }
                tokens.push_back({TokenKind::Identifier, formula.substr(start, i - start)});
                continue;
            }

            if (formula[i] == '(') {
                tokens.push_back({TokenKind::LeftParen, "("});
                ++i;
                continue;
            }
            if (formula[i] == ')') {
                tokens.push_back({TokenKind::RightParen, ")"});
                ++i;
                continue;
            }
            if (formula[i] == ',') {
                tokens.push_back({TokenKind::Comma, ","});
                ++i;
                continue;
            }

            const char *operators[] = {">>>", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||"};
            bool matched = false;
            for (const char *op : operators) {
                const std::size_t length = std::strlen(op);
                if (formula.compare(i, length, op) == 0) {
                    tokens.push_back({TokenKind::Operator, op});
                    i += length;
                    matched = true;
                    break;
                }
            }
            if (matched) {
                continue;
            }

            if (std::string("+-*/%^&|~!<>=?:").find(formula[i]) != std::string::npos) {
                tokens.push_back({TokenKind::Operator, formula.substr(i, 1)});
                ++i;
                continue;
            }
            return false;
        }

        tokens.push_back({TokenKind::End, ""});
        return true;
    }

    const Token &current() const { return tokens[position]; }

    bool accept(TokenKind kind, const char *text = nullptr) {
        if (current().kind != kind || (text && current().text != text)) {
            return false;
        }
        ++position;
        return true;
    }

    bool parseConditional(std::string &result) {
        if (!parseLogicalOr(result)) {
            return false;
        }
        if (accept(TokenKind::Operator, "?")) {
            std::string whenTrue;
            std::string whenFalse;
            if (!parseConditional(whenTrue) || !accept(TokenKind::Operator, ":")
                || !parseConditional(whenFalse)) {
                return false;
            }
            result = "(" + result + " ? " + whenTrue + " : " + whenFalse + ")";
        }
        return true;
    }

    bool parseLogicalOr(std::string &result) {
        if (!parseLogicalAnd(result)) return false;
        while ((current().kind == TokenKind::Operator && current().text == "||")
               || (current().kind == TokenKind::Identifier && current().text == "or")) {
            ++position;
            std::string right;
            if (!parseLogicalAnd(right)) return false;
            result = "(" + result + " or " + right + ")";
        }
        return true;
    }

    bool parseLogicalAnd(std::string &result) {
        if (!parseBitwiseOr(result)) return false;
        while ((current().kind == TokenKind::Operator && current().text == "&&")
               || (current().kind == TokenKind::Identifier && current().text == "and")) {
            ++position;
            std::string right;
            if (!parseBitwiseOr(right)) return false;
            result = "(" + result + " and " + right + ")";
        }
        return true;
    }

    bool parseBitwiseOr(std::string &result) {
        if (!parseBitwiseXor(result)) return false;
        while (accept(TokenKind::Operator, "|")) {
            std::string right;
            if (!parseBitwiseXor(right)) return false;
            result = "bb_or(" + result + "," + right + ")";
        }
        return true;
    }

    bool parseBitwiseXor(std::string &result) {
        if (!parseBitwiseAnd(result)) return false;
        while (accept(TokenKind::Operator, "^")) {
            std::string right;
            if (!parseBitwiseAnd(right)) return false;
            result = "bb_xor(" + result + "," + right + ")";
        }
        return true;
    }

    bool parseBitwiseAnd(std::string &result) {
        if (!parseEquality(result)) return false;
        while (accept(TokenKind::Operator, "&")) {
            std::string right;
            if (!parseEquality(right)) return false;
            result = "bb_and(" + result + "," + right + ")";
        }
        return true;
    }

    bool parseEquality(std::string &result) {
        if (!parseRelational(result)) return false;
        while (current().kind == TokenKind::Operator
               && (current().text == "==" || current().text == "=" || current().text == "!=")) {
            const std::string op = current().text == "==" ? "=" : current().text;
            ++position;
            std::string right;
            if (!parseRelational(right)) return false;
            result = "(" + result + " " + op + " " + right + ")";
        }
        return true;
    }

    bool parseRelational(std::string &result) {
        if (!parseShift(result)) return false;
        while (current().kind == TokenKind::Operator
               && (current().text == "<" || current().text == ">"
                   || current().text == "<=" || current().text == ">=")) {
            const std::string op = current().text;
            ++position;
            std::string right;
            if (!parseShift(right)) return false;
            result = "(" + result + " " + op + " " + right + ")";
        }
        return true;
    }

    bool parseShift(std::string &result) {
        if (!parseAdditive(result)) return false;
        while (current().kind == TokenKind::Operator
               && (current().text == "<<" || current().text == ">>" || current().text == ">>>")) {
            const std::string name = current().text == "<<" ? "bb_shl"
                                     : current().text == ">>" ? "bb_shr" : "ushr";
            ++position;
            std::string right;
            if (!parseAdditive(right)) return false;
            result = name + "(" + result + "," + right + ")";
        }
        return true;
    }

    bool parseAdditive(std::string &result) {
        if (!parseMultiplicative(result)) return false;
        while (current().kind == TokenKind::Operator
               && (current().text == "+" || current().text == "-")) {
            const std::string op = current().text;
            ++position;
            std::string right;
            if (!parseMultiplicative(right)) return false;
            result = "(" + result + " " + op + " " + right + ")";
        }
        return true;
    }

    bool parseMultiplicative(std::string &result) {
        if (!parseUnary(result)) return false;
        while (current().kind == TokenKind::Operator
               && (current().text == "*" || current().text == "/" || current().text == "%")) {
            const std::string op = current().text;
            ++position;
            std::string right;
            if (!parseUnary(right)) return false;
            result = "(" + result + " " + op + " " + right + ")";
        }
        return true;
    }

    bool parseUnary(std::string &result) {
        if (current().kind == TokenKind::Operator
            && (current().text == "+" || current().text == "-" || current().text == "!" || current().text == "~")) {
            const std::string op = current().text;
            ++position;
            if (!parseUnary(result)) return false;
            if (op == "~") result = "bitnot(" + result + ")";
            else if (op == "!") result = "(not " + result + ")";
            else result = "(" + op + result + ")";
            return true;
        }
        return parsePrimary(result);
    }

    bool parsePrimary(std::string &result) {
        if (current().kind == TokenKind::Number) {
            result = current().text;
            ++position;
            return true;
        }
        if (current().kind == TokenKind::Identifier) {
            const std::string name = current().text;
            ++position;
            if (!accept(TokenKind::LeftParen)) {
                result = name;
                return true;
            }

            result = name + "(";
            if (!accept(TokenKind::RightParen)) {
                std::string argument;
                if (!parseConditional(argument)) return false;
                result += argument;
                while (accept(TokenKind::Comma)) {
                    if (!parseConditional(argument)) return false;
                    result += "," + argument;
                }
                if (!accept(TokenKind::RightParen)) return false;
            }
            result += ")";
            return true;
        }
        if (accept(TokenKind::LeftParen)) {
            if (!parseConditional(result) || !accept(TokenKind::RightParen)) return false;
            result = "(" + result + ")";
            return true;
        }
        return false;
    }

    std::vector<Token> tokens;
    std::size_t position = 0;
};

#endif // CUSTOPERATORS_H