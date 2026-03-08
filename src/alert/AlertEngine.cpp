#include "AlertEngine.hpp"
#include <vector>

namespace NeuraDoc {

class AlertEngine::Impl {
public:
    int minHeartRate = 40;
    int maxHeartRate = 140;
    int maxSystolicBP = 180;
    int maxDiastolicBP = 120;
    float minOxygenSat = 90.0f;
    float minTemp = 35.0f;
    float maxTemp = 40.0f;
    
    AlertCallback callback;
};

AlertEngine::AlertEngine() : pImpl(std::make_unique<Impl>()) {}
AlertEngine::~AlertEngine() = default;

void AlertEngine::setHeartRateThreshold(int min, int max) {
    pImpl->minHeartRate = min;
    pImpl->maxHeartRate = max;
}

void AlertEngine::setBloodPressureThreshold(int systolicMax, int diastolicMax) {
    pImpl->maxSystolicBP = systolicMax;
    pImpl->maxDiastolicBP = diastolicMax;
}

void AlertEngine::setOxygenSaturationThreshold(float min) {
    pImpl->minOxygenSat = min;
}

void AlertEngine::setTemperatureThreshold(float min, float max) {
    pImpl->minTemp = min;
    pImpl->maxTemp = max;
}

std::vector<Alert> AlertEngine::checkVitals(const VitalSigns& vitals) {
    std::vector<Alert> alerts;
    
    // Check heart rate
    if (vitals.heartRate < pImpl->minHeartRate) {
        Alert alert;
        alert.patientId = vitals.patientId;
        alert.level = AlertLevel::CRITICAL;
        alert.message = "Bradycardia detected: Heart rate too low";
        alert.vitals = vitals;
        alert.timestamp = std::chrono::system_clock::now();
        alerts.push_back(alert);
    } else if (vitals.heartRate > pImpl->maxHeartRate) {
        Alert alert;
        alert.patientId = vitals.patientId;
        alert.level = AlertLevel::CRITICAL;
        alert.message = "Tachycardia detected: Heart rate too high";
        alert.vitals = vitals;
        alert.timestamp = std::chrono::system_clock::now();
        alerts.push_back(alert);
    }
    
    // Check blood pressure
    if (vitals.systolicBP > pImpl->maxSystolicBP || vitals.diastolicBP > pImpl->maxDiastolicBP) {
        Alert alert;
        alert.patientId = vitals.patientId;
        alert.level = AlertLevel::EMERGENCY;
        alert.message = "Hypertensive crisis: Blood pressure critically high";
        alert.vitals = vitals;
        alert.timestamp = std::chrono::system_clock::now();
        alerts.push_back(alert);
    }
    
    // Check oxygen saturation
    if (vitals.oxygenSaturation < pImpl->minOxygenSat) {
        Alert alert;
        alert.patientId = vitals.patientId;
        alert.level = AlertLevel::EMERGENCY;
        alert.message = "Hypoxemia: Oxygen saturation critically low";
        alert.vitals = vitals;
        alert.timestamp = std::chrono::system_clock::now();
        alerts.push_back(alert);
    }
    
    // Check temperature
    if (vitals.temperature < pImpl->minTemp || vitals.temperature > pImpl->maxTemp) {
        Alert alert;
        alert.patientId = vitals.patientId;
        alert.level = AlertLevel::CRITICAL;
        alert.message = vitals.temperature < pImpl->minTemp ? "Hypothermia detected" : "Hyperthermia detected";
        alert.vitals = vitals;
        alert.timestamp = std::chrono::system_clock::now();
        alerts.push_back(alert);
    }
    
    // Trigger callbacks
    if (pImpl->callback) {
        for (const auto& alert : alerts) {
            pImpl->callback(alert);
        }
    }
    
    return alerts;
}

void AlertEngine::setAlertCallback(AlertCallback callback) {
    pImpl->callback = callback;
}

} // namespace NeuraDoc
