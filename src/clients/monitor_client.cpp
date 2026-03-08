#include <iostream>
#include <string>

// Bedside monitor client - displays patient vitals locally
// In production: connect to medical devices and display on monitor

int main() {
    std::cout << "NeuraDoc Bedside Monitor" << std::endl;
    std::cout << "=========================" << std::endl;
    
    std::string patientId;
    std::cout << "\nEnter Patient ID: ";
    std::cin >> patientId;
    
    std::cout << "\nMonitoring patient " << patientId << "..." << std::endl;
    std::cout << "Connecting to medical devices..." << std::endl;
    
    // In production: connect to actual medical equipment
    std::cout << "Connected to vital signs monitor" << std::endl;
    std::cout << "\nDisplaying real-time vitals..." << std::endl;
    
    // In production: display real-time vitals on bedside monitor
    while (true) {
        // Update display with current vitals
    }
    
    return 0;
}
