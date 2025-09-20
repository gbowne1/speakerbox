#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <map>

namespace speakerbox {

struct TSParameters {
    double fs;  // Hz
    double qts;
    double vas;  // liters
    double re;  // ohms
    double sd;  // cm²
    double xmax;  // mm
    double vd;  // liters (Sd * Xmax / 1000)
    double le;  // mH
    double cms;  // m/N
    double mms;  // grams
    double bl;  // Tm

    TSParameters();  // Default ctor
    ~TSParameters() = default;
};

enum class EnclosureType {
    Sealed,
    Ported,
    Bandpass,
    TransmissionLine,
    PassiveRadiator
};

struct EnclosureResult {
    std::string type;
    double vb;  // liters
    double fc_or_fb;  // Hz
    std::string freq_response;
    double port_length;  // cm (if applicable)
    double port_diameter;  // cm
    double air_velocity;  // m/s (basic)
    double width, height, depth;  // cm, golden ratio
    bool within_xmax;
    std::vector<std::string> warnings;
};

class Calculator {
public:
    Calculator();
    ~Calculator();

    EnclosureResult calculate(const TSParameters& params, EnclosureType type, const std::map<std::string, double>& options = {});

    std::string recommendType(double qts) const;

private:
    EnclosureResult calculateSealed(const TSParameters& params, double desired_qtc);
    EnclosureResult calculatePorted(const TSParameters& params, double desired_fb);
    EnclosureResult calculateBandpass(const TSParameters& params, double s);
    EnclosureResult calculateTransmissionLine(const TSParameters& params, double tr);
    EnclosureResult calculatePassiveRadiator(const TSParameters& params, double delta);

    void applyGoldenRatio(double vb, double& width, double& height, double& depth) const;
    double calculatePortAirVelocity(double sd, double xmax, double fb) const;  // Basic
    bool checkExcursion(double xmax) const;  // Placeholder
    double subtractDisplacements(double vb, const TSParameters& params, double port_vol = 0.0) const;
};

}  // namespace speakerbox
