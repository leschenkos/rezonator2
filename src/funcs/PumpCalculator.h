#ifndef PUMP_CALCULATOR_H
#define PUMP_CALCULATOR_H

#include <memory>

class PumpParams;

namespace Z {
class Matrix;
}

struct BeamResult
{
    double beamRadius; ///< Beam rdius.
    double frontRadius; ///< Wavefront ROC.
    double halfAngle; ///< Half of divergence angle.
};

class PumpCalculator final
{
public:
    static std::shared_ptr<PumpCalculator> T();
    static std::shared_ptr<PumpCalculator> S();

    ~PumpCalculator();

    bool init(PumpParams* pump, double lambdaSI, const char* ident = nullptr, bool writeProtocol = false);
    BeamResult calc(const Z::Matrix& matrix, double ior);

private:
    PumpCalculator() = default;
    PumpCalculator(const PumpCalculator& other) = delete;
    PumpCalculator(PumpCalculator&& other) = delete;
    PumpCalculator& operator =(const PumpCalculator& other) = delete;
    PumpCalculator& operator =(PumpCalculator&& other) = delete;

    class PumpCalculatorImpl *_impl = nullptr;
};

#endif // PUMP_CALCULATOR_H
