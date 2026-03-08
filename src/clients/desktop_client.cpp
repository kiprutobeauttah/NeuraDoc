#include <iostream>
#include <string>

// Desktop client for doctors to monitor patients
// In production: connect to NetworkServer and display real-time vitals

int main() {
    std::cout << "NeuraDoc Desktop Client" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "\nConnecting to NeuraDoc server..." << std::endl;
    
    // In production: establish network connection
    std::cout << "Connected to server at localhost:8080" << std::endl;
    std::cout << "\nMonitoring patients..." << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    
    // In production: receive and display real-time patient data
    while (true) {
        // Display patient vitals in GUI
    }
    
    return 0;
}
