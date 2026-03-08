#include "DeviceInterface.hpp"
#include "DataRouter.hpp"
#include "AlertEngine.hpp"
#include "PatientDatabase.hpp"
#include "NetworkServer.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace NeuraDoc;

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "NeuraDoc Patient Monitoring System" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Initialize components
    PatientDatabase db;
    DataRouter router;
    AlertEngine alertEngine;
    NetworkServer server(8080);
    
    // Add sample patients
    PatientInfo patient1{"P001", "John Doe", 45, "Room 101", "Dr. Smith"};
    PatientInfo patient2{"P002", "Jane Smith", 62, "Room 102", "Dr. Johnson"};
    db.addPatient(patient1);
    db.addPatient(patient2);
    
    // Register doctor endpoints
    router.registerEndpoint("doctor1_pc", "192.168.1.100:9000");
    router.registerEndpoint("doctor1_watch", "192.168.1.101:9001");
    
    // Setup alert callback
    alertEngine.setAlertCallback([](const Alert& alert) {
        std::cout << "\n*** ALERT ***" << std::endl;
        std::cout << "Patient: " << alert.patientId << std::endl;
        std::cout << "Level: " << static_cast<int>(alert.level) << std::endl;
        std::cout << "Message: " << alert.message << std::endl;
        std::cout << "HR: " << alert.vitals.heartRate 
                  << " BP: " << alert.vitals.systolicBP << "/" << alert.vitals.diastolicBP
                  << " O2: " << alert.vitals.oxygenSaturation << "%" << std::endl;
    });
    
    // Start network server
    server.start();
    
    // Create device interfaces for each patient
    auto device1 = createDeviceInterface("simulated");
    auto device2 = createDeviceInterface("simulated");
    
    device1->connect("P001");
    device2->connect("P002");
    
    // Setup monitoring callbacks
    device1->setCallback([&](const VitalSigns& vitals) {
        std::cout << "\n[Patient " << vitals.patientId << "] Vitals received:" << std::endl;
        std::cout << "  HR: " << vitals.heartRate << " bpm" << std::endl;
        std::cout << "  BP: " << vitals.systolicBP << "/" << vitals.diastolicBP << " mmHg" << std::endl;
        std::cout << "  O2: " << vitals.oxygenSaturation << "%" << std::endl;
        std::cout << "  Temp: " << vitals.temperature << "°C" << std::endl;
        
        // Store in database
        db.storeVitals(vitals);
        
        // Check for alerts
        alertEngine.checkVitals(vitals);
        
        // Route to doctor devices
        router.routeData(vitals);
        
        // Broadcast to network clients
        server.broadcast(vitals);
    });
    
    device2->setCallback([&](const VitalSigns& vitals) {
        std::cout << "\n[Patient " << vitals.patientId << "] Vitals received:" << std::endl;
        std::cout << "  HR: " << vitals.heartRate << " bpm" << std::endl;
        std::cout << "  BP: " << vitals.systolicBP << "/" << vitals.diastolicBP << " mmHg" << std::endl;
        std::cout << "  O2: " << vitals.oxygenSaturation << "%" << std::endl;
        std::cout << "  Temp: " << vitals.temperature << "°C" << std::endl;
        
        db.storeVitals(vitals);
        alertEngine.checkVitals(vitals);
        router.routeData(vitals);
        server.broadcast(vitals);
    });
    
    // Start monitoring
    std::cout << "\nStarting patient monitoring..." << std::endl;
    device1->startMonitoring();
    device2->startMonitoring();
    
    // Run for 60 seconds
    std::cout << "Monitoring for 60 seconds. Press Ctrl+C to stop." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));
    
    // Cleanup
    device1->stopMonitoring();
    device2->stopMonitoring();
    device1->disconnect();
    device2->disconnect();
    server.stop();
    
    std::cout << "\nShutting down..." << std::endl;
    return 0;
}
