#pragma once
#include <string>
#include <chrono>

namespace NeuraDoc {

struct VitalSigns {
    std::string patientId;
    std::chrono::system_clock::time_point timestamp;
    
    // Vital measurements
    int heartRate;           // bpm
    int systolicBP;          // mmHg
    int diastolicBP;         // mmHg
    float oxygenSaturation;  // %
    float temperature;       // Celsius
    int respiratoryRate;     // breaths per minute
    
    bool isValid() const;
    bool isCritical() const;
};

} // namespace NeuraDoc
