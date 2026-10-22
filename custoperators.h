#ifndef CUSTOPERATORS_H
#define CUSTOPERATORS_H

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

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

#endif // CUSTOPERATORS_H