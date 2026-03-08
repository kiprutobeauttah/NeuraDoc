#include "RealTimeAnalyzer.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace NeuraDoc {

class RealTimeAnalyzer::Impl {
public:
    float anomalyThreshold = 0.7f;
    bool predictiveEnabled = true;
    AnalysisCallback callback;
};

RealTimeAnalyzer::RealTimeAnalyzer() : pImpl(std::make_unique<Impl>()) {}
RealTimeAnalyzer::~RealTimeAnalyzer() = default;

AnalysisResult RealTimeAnalyzer::analyze(const VitalSigns& vitals) {
    AnalysisResult result;
    result.patientId = vitals.patientId;
    result.timestamp = std::chrono::system_clock::now();
    result.riskScore = calculateRiskScore(vitals);
    result.anomalyDetected = detectAnomalies(vitals);
    
    if (result.anomalyDetected) {
        if (vitals.heartRate < 40 || vitals.heartRate > 140) {
            result.anomalyType = "Cardiac Arrhythmia";
            result.recommendations.push_back("Immediate cardiac assessment required");
        }
        if (vitals.oxygenSaturation < 90) {
            result.anomalyType = "Hypoxemia";
            result.recommendations.push_back("Oxygen therapy recommended");
        }
        if (vitals.systolicBP > 180) {
            result.anomalyType = "Hypertensive Crisis";
            result.recommendations.push_back("Emergency blood pressure management");
        }
    }
    
    if (vitals.heartRate > 100) {
        result.trends.push_back("Elevated heart rate");
    }
    if (vitals.temperature > 38.0f) {
        result.trends.push_back("Fever detected");
    }
    
    if (pImpl->callback) {
        pImpl->callback(result);
    }
    
    return result;
}

AnalysisResult RealTimeAnalyzer::analyzeTrend(const std::vector<VitalSigns>& history) {
    AnalysisResult result;
    if (history.empty()) return result;
    
    result.patientId = history[0].patientId;
    result.timestamp = std::chrono::system_clock::now();
    
    std::vector<int> heartRates;
    std::vector<float> temps;
    
    for (const auto& v : history) {
        heartRates.push_back(v.heartRate);
        temps.push_back(v.temperature);
    }
    
    float avgHR = std::accumulate(heartRates.begin(), heartRates.end(), 0.0f) / heartRates.size();
    float avgTemp = std::accumulate(temps.begin(), temps.end(), 0.0f) / temps.size();
    
    if (heartRates.back() > avgHR * 1.2f) {
        result.trends.push_back("Heart rate increasing");
    }
    if (temps.back() > avgTemp + 0.5f) {
        result.trends.push_back("Temperature rising");
    }
    
    if (pImpl->predictiveEnabled) {
        result.predictions = predictTrends(history);
    }
    
    result.riskScore = calculateRiskScore(history.back());
    
    return result;
}

void RealTimeAnalyzer::setAnomalyThreshold(float threshold) {
    pImpl->anomalyThreshold = threshold;
}

void RealTimeAnalyzer::enablePredictiveAnalysis(bool enable) {
    pImpl->predictiveEnabled = enable;
}

void RealTimeAnalyzer::setAnalysisCallback(AnalysisCallback callback) {
    pImpl->callback = callback;
}

float RealTimeAnalyzer::calculateRiskScore(const VitalSigns& vitals) {
    float score = 0.0f;
    
    if (vitals.heartRate < 40 || vitals.heartRate > 140) score += 0.3f;
    else if (vitals.heartRate < 50 || vitals.heartRate > 120) score += 0.15f;
    
    if (vitals.systolicBP > 180 || vitals.diastolicBP > 120) score += 0.3f;
    else if (vitals.systolicBP > 160 || vitals.diastolicBP > 100) score += 0.15f;
    
    if (vitals.oxygenSaturation < 90) score += 0.3f;
    else if (vitals.oxygenSaturation < 95) score += 0.1f;
    
    if (vitals.temperature < 35 || vitals.temperature > 40) score += 0.2f;
    else if (vitals.temperature < 36 || vitals.temperature > 38.5f) score += 0.1f;
    
    return std::min(score, 1.0f);
}

bool RealTimeAnalyzer::detectAnomalies(const VitalSigns& vitals) {
    return calculateRiskScore(vitals) >= pImpl->anomalyThreshold;
}

std::vector<std::string> RealTimeAnalyzer::predictTrends(const std::vector<VitalSigns>& history) {
    std::vector<std::string> predictions;
    
    if (history.size() < 3) return predictions;
    
    int hrTrend = 0;
    for (size_t i = 1; i < history.size(); i++) {
        if (history[i].heartRate > history[i-1].heartRate) hrTrend++;
        else if (history[i].heartRate < history[i-1].heartRate) hrTrend--;
    }
    
    if (hrTrend > 2) {
        predictions.push_back("Heart rate likely to continue increasing");
    } else if (hrTrend < -2) {
        predictions.push_back("Heart rate likely to continue decreasing");
    }
    
    float tempTrend = history.back().temperature - history[0].temperature;
    if (tempTrend > 0.5f) {
        predictions.push_back("Temperature trending upward - monitor for infection");
    }
    
    return predictions;
}

} // namespace NeuraDoc
