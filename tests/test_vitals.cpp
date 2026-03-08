#include "VitalSigns.hpp"
#include <iostream>
#include <cassert>

using namespace NeuraDoc;

void testVitalSignsValidation() {
    VitalSigns vitals;
    vitals.patientId = "TEST001";
    vitals.timestamp = std::chrono::system_clock::now();
    vitals.heartRate = 75;
    vitals.systolicBP = 120;
    vitals.diastolicBP = 80;
    vitals.oxygenSaturation = 98.0f;
    vitals.temperature = 37.0f;
    vitals.respiratoryRate = 16;
    
    assert(vitals.isValid());
    assert(!vitals.isCritical());
    
    std::cout << "✓ Vital signs validation test passed" << std::endl;
}

void testCriticalVitals() {
    VitalSigns vitals;
    vitals.patientId = "TEST002";
    vitals.timestamp = std::chrono::system_clock::now();
    vitals.heartRate = 35; // Critical low
    vitals.systolicBP = 120;
    vitals.diastolicBP = 80;
    vitals.oxygenSaturation = 98.0f;
    vitals.temperature = 37.0f;
    vitals.respiratoryRate = 16;
    
    assert(vitals.isValid());
    assert(vitals.isCritical());
    
    std::cout << "✓ Critical vitals detection test passed" << std::endl;
}

int main() {
    std::cout << "Running VitalSigns tests..." << std::endl;
    testVitalSignsValidation();
    testCriticalVitals();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
