#pragma once
#include "VitalSigns.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace NeuraDoc {

struct AnalysisResult {
    std::string patientId;
    std::chrono::system_clock::time_point timestamp;
    
    bool anomalyDetected;
    std::string anomalyType;
    float riskScore;
    
    std::vector<std::string> trends;
    std::vector<std::string> predictions;
    std::vector<std::string> recommendations;
};

class RealTimeAnalyzer {
public:
    RealTimeAnalyzer();
    ~RealTimeAnalyzer();
    
    AnalysisResult analyze(const VitalSigns& vitals);
    AnalysisResult analyzeTrend(const std::vector<VitalSigns>& history);
    
    void setAnomalyThreshold(float threshold);
    void enablePredictiveAnalysis(bool enable);
    
    using AnalysisCallback = std::function<void(const AnalysisResult&)>;
    void setAnalysisCallback(AnalysisCallback callback);
    
    float calculateRiskScore(const VitalSigns& vitals);
    bool detectAnomalies(const VitalSigns& vitals);
    std::vector<std::string> predictTrends(const std::vector<VitalSigns>& history);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
