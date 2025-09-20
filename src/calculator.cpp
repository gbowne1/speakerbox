#include "calculator.h"

namespace speakerbox {

constexpr double SPEED_SOUND = 343.0;  // m/s
constexpr double GOLDEN_RATIO_H = 1.6;
constexpr double GOLDEN_RATIO_W = 1.0;
constexpr double GOLDEN_RATIO_D = 0.6;

TSParameters::TSParameters() : fs(0.0), qts(0.0), vas(0.0), re(0.0), sd(0.0), xmax(0.0), vd(0.0), le(0.0), cms(0.0), mms(0.0), bl(0.0) {}

Calculator::Calculator() = default;

Calculator::~Calculator() = default;

std::string Calculator::recommendType(double qts) const {
    if (qts < 0.4) return "Ported";
    if (qts >= 0.4 && qts <= 0.6) return "Sealed";
    return "Other";
}

EnclosureResult Calculator::calculate(const TSParameters& params, EnclosureType type, const std::map<std::string, double>& options) {
    EnclosureResult result;
    result.within_xmax = checkExcursion(params.xmax);
    result.type = "Unknown";

    double desired_qtc = options.count("qtc") ? options.at("qtc") : 0.707;
    double desired_fb = options.count("fb") ? options.at("fb") : params.fs;
    double s = options.count("s") ? options.at("s") : 0.6;
    double tr = options.count("tr") ? options.at("tr") : 1.0;
    double delta = options.count("delta") ? options.at("delta") : 1.0;

    switch (type) {
        case EnclosureType::Sealed:
            return calculateSealed(params, desired_qtc);
        case EnclosureType::Ported:
            return calculatePorted(params, desired_fb);
        case EnclosureType::Bandpass:
            return calculateBandpass(params, s);
        case EnclosureType::TransmissionLine:
            return calculateTransmissionLine(params, tr);
        case EnclosureType::PassiveRadiator:
            return calculatePassiveRadiator(params, delta);
    }
    result.warnings.push_back("Invalid type");
    return result;
}

EnclosureResult Calculator::calculateSealed(const TSParameters& params, double desired_qtc) {
    EnclosureResult result;
    result.type = "Sealed";
    double alpha = std::pow(desired_qtc / params.qts, 2) - 1.0;
    if (alpha <= 0.0) {
        result.warnings.push_back("Invalid alpha");
        return result;
    }
    result.vb = params.vas / alpha;
    result.fc_or_fb = params.fs * std::sqrt(1.0 + alpha);
    result.freq_response = "12 dB/octave roll-off below Fc";
    result.port_length = 0.0;
    result.port_diameter = 0.0;
    result.air_velocity = 0.0;
    result.vb = subtractDisplacements(result.vb, params);
    applyGoldenRatio(result.vb, result.width, result.height, result.depth);
    return result;
}

EnclosureResult Calculator::calculatePorted(const TSParameters& params, double desired_fb) {
    EnclosureResult result;
    result.type = "Ported";
    // Approximate Butterworth B4
    double h = desired_fb / params.fs;
    double alpha = (1.0 / (h * h)) - 1.0;
    result.vb = params.vas / alpha;
    result.fc_or_fb = desired_fb;
    result.freq_response = "24 dB/octave roll-off below Fb";
    result.port_diameter = 5.0;  // Default cm
    double r = result.port_diameter / 2.0;
    result.port_length = (23562.5 * r * r) / (desired_fb * desired_fb * result.vb) - 0.85 * result.port_diameter;  // Approx cm
    result.air_velocity = calculatePortAirVelocity(params.sd, params.xmax, desired_fb);
    double port_vol = PI * r * r * result.port_length / 1000.0;  // liters
    result.vb = subtractDisplacements(result.vb, params, port_vol);
    applyGoldenRatio(result.vb, result.width, result.height, result.depth);
    return result;
}

EnclosureResult Calculator::calculateBandpass(const TSParameters& params, double s) {
    EnclosureResult result;
    result.type = "Bandpass";
    double qbp = 1.0 / (2.0 * s);  // Approx from alignments
    double vf = std::pow(2.0 * s * params.qts, 2) * params.vas;
    double vr = params.vas / (std::pow(qbp / params.qts, 2) - 1.0);
    result.vb = vf + vr;
    result.fc_or_fb = qbp * (params.fs / params.qts);
    result.freq_response = "Bandpass response";
    result.port_diameter = 5.0;
    double r = result.port_diameter / 2.0;
    result.port_length = (94250.0 * r * r) / (result.fc_or_fb * result.fc_or_fb * vf) - 1.595 * r;
    result.air_velocity = calculatePortAirVelocity(params.sd, params.xmax, result.fc_or_fb);
    double port_vol = PI * r * r * result.port_length / 1000.0;
    result.vb = subtractDisplacements(result.vb, params, port_vol);
    applyGoldenRatio(result.vb, result.width, result.height, result.depth);
    return result;
}

EnclosureResult Calculator::calculateTransmissionLine(const TSParameters& params, double tr) {
    EnclosureResult result;
    result.type = "TransmissionLine";
    double alpha = 1.5198;  // From table example
    result.vb = params.vas / alpha;
    double h = 1.0;  // From table
    result.fc_or_fb = h * params.fs;
    double sf = 1.0;  // Default from table for TR=1
    if (tr == 0.1) sf = 0.62;  // From table
    // ... add other TR
    result.port_length = sf * (SPEED_SOUND) / (4.0 * result.fc_or_fb) * 100.0;  // cm
    result.port_diameter = 0.0;
    result.air_velocity = 0.0;
    result.vb = subtractDisplacements(result.vb, params);
    applyGoldenRatio(result.vb, result.width, result.height, result.depth);
    return result;
}

EnclosureResult Calculator::calculatePassiveRadiator(const TSParameters& params, double delta) {
    EnclosureResult result;
    result.type = "PassiveRadiator";
    double alpha = delta;  // Assume
    result.vb = params.vas / alpha;
    double h = 1.51;  // From example
    result.fc_or_fb = h * params.fs;
    result.freq_response = "Similar to ported";
    result.port_length = 0.0;
    result.port_diameter = 0.0;
    result.air_velocity = 0.0;
    result.vb = subtractDisplacements(result.vb, params);
    applyGoldenRatio(result.vb, result.width, result.height, result.depth);
    return result;
}

void Calculator::applyGoldenRatio(double vb, double& width, double& height, double& depth) const {
    double cube_root = std::pow(vb * 1000.0, 1.0 / 3.0);  // cm
    width = cube_root * GOLDEN_RATIO_W;
    height = cube_root * GOLDEN_RATIO_H;
    depth = cube_root * GOLDEN_RATIO_D;
}

double Calculator::calculatePortAirVelocity(double sd, double xmax, double fb) const {
    // Basic: v = (Sd * Xmax * 2 * PI * fb) / port_area , but placeholder
    return 10.0;  // m/s example
}

bool Calculator::checkExcursion(double xmax) const {
    // Placeholder: assume power, calc excursion < xmax
    return true;
}

double Calculator::subtractDisplacements(double vb, const TSParameters& params, double port_vol) const {
    double driver_disp = params.vd;  // liters
    double bracing_disp = 0.5;  // assume
    return vb - driver_disp - bracing_disp - port_vol;
}

}  // namespace speakerbox
