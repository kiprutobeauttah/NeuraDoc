#include "AlertEngine.hpp"
#include <iostream>
#include <cassert>

using namespace NeuraDoc;

void testAlertGeneration() {
    AlertEngine engine;
    
    VitalSigns vitals;
    vitals.patientId = "TEST001";
    vitals.timestamp = std::chrono::system_clock::now();
    vitals.heartRate = 150; // Too high
    vitals.systolicBP = 120;
    vitals.diastolicBP = 80;
    vitals.oxygenSaturation = 98.0f;
    vitals.temperature = 37.0f;
    vitals.respiratoryRate = 16;
    
    auto alerts = engine.checkVitals(vitals);
    assert(!alerts.empty());
    assert(alerts[0].level == AlertLevel::CRITICAL);
    
    std::cout << "✓ Alert generation test passed" << std::endl;
}

int main() {
    std::cout << "Running AlertEngine tests..." << std::endl;
    testAlertGeneration();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
