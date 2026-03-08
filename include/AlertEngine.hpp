#pragma once
#include "VitalSigns.hpp"
#include <functional>
#include <string>

namespace NeuraDoc {

enum class AlertLevel {
    INFO,
    WARNING,
    CRITICAL,
    EMERGENCY
};

struct Alert {
    std::string patientId;
    AlertLevel level;
    std::string message;
    VitalSigns vitals;
    std::chrono::system_clock::time_point timestamp;
};

class AlertEngine {
public:
    AlertEngine();
    ~AlertEngine();
    
    // Set thresholds for alerts
    void setHeartRateThreshold(int min, int max);
    void setBloodPressureThreshold(int systolicMax, int diastolicMax);
    void setOxygenSaturationThreshold(float min);
    void setTemperatureThreshold(float min, float max);
    
    // Check vitals and generate alerts
    std::vector<Alert> checkVitals(const VitalSigns& vitals);
    
    // Set alert callback
    using AlertCallback = std::function<void(const Alert&)>;
    void setAlertCallback(AlertCallback callback);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
