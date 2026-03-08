#include "DeviceInterface.hpp"
#include "SecureRouter.hpp"
#include "AlertEngine.hpp"
#include "PatientDatabase.hpp"
#include "SecureNetworkServer.hpp"
#include "SecurityManager.hpp"
#include "AuditLogger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace NeuraDoc;

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "NeuraDoc Secure Patient Monitoring System" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // Initialize security components
    auto securityMgr = std::make_shared<SecurityManager>();
    auto auditLogger = std::make_shared<AuditLogger>();
    
    std::cout << "\n[SECURITY] Initializing secure components..." << std::endl;
    
    // Register users
    securityMgr->registerUser("dr_smith", "secure123", UserRole::DOCTOR);
    securityMgr->registerUser("nurse_jane", "nurse456", UserRole::NURSE);
    std::cout << "[SECURITY] Users registered" << std::endl;
    
    // Initialize components
    PatientDatabase db;
    SecureRouter router(securityMgr);
    AlertEngine alertEngine;
    SecureNetworkServer server(8443, securityMgr); // HTTPS port
    
    // Add sample patients
    PatientInfo patient1{"P001", "Beauttah Kipruto", 45, "Room 101", "Dr. Mary"};
    PatientInfo patient2{"P002", "Margaret Beauttah", 62, "Room 102", "Dr. Jeniffer"};
    db.addPatient(patient1);
    db.addPatient(patient2);
    
    std::cout << "[DATABASE] Patients registered" << std::endl;
    
    // Authenticate admin for setup
    AuthToken adminToken = securityMgr->authenticate("admin", "admin123");
    if (!adminToken.isValid()) {
        std::cerr << "Failed to authenticate admin!" << std::endl;
        return 1;
    }
    
    std::cout << "[SECURITY] Admin authenticated" << std::endl;
    
    // Register secure endpoints
    SecureEndpoint doctorPC;
    doctorPC.endpointId = "doctor_pc_1";
    doctorPC.address = "192.168.1.100:9000";
    doctorPC.publicKey = "PUBLIC_KEY_PLACEHOLDER";
    doctorPC.authToken = adminToken;
    
    SecureEndpoint doctorWatch;
    doctorWatch.endpointId = "doctor_watch_1";
    doctorWatch.address = "192.168.1.101:9001";
    doctorWatch.publicKey = "PUBLIC_KEY_PLACEHOLDER";
    doctorWatch.authToken = adminToken;
    
    if (router.registerEndpoint(doctorPC, adminToken)) {
        std::cout << "[ROUTER] Doctor PC endpoint registered securely" << std::endl;
    }
    if (router.registerEndpoint(doctorWatch, adminToken)) {
        std::cout << "[ROUTER] Doctor smartwatch endpoint registered securely" << std::endl;
    }
    
    // Setup alert callback
    alertEngine.setAlertCallback([&](const Alert& alert) {
        std::cout << "\n*** CRITICAL ALERT ***" << std::endl;
        std::cout << "Patient: " << alert.patientId << std::endl;
        std::cout << "Level: " << static_cast<int>(alert.level) << std::endl;
        std::cout << "Message: " << alert.message << std::endl;
        std::cout << "HR: " << alert.vitals.heartRate 
                  << " BP: " << alert.vitals.systolicBP << "/" << alert.vitals.diastolicBP
                  << " O2: " << alert.vitals.oxygenSaturation << "%" << std::endl;
        
        // Log alert
        auditLogger->log({
            AuditEventType::ALERT_TRIGGERED,
            "system",
            alert.patientId,
            alert.message,
            "",
            std::chrono::system_clock::now(),
            true
        });
    });
    
    // Start secure network server
    server.setClientAuthCallback([](const ClientConnection& conn) {
        std::cout << "[SERVER] Client connected: " << conn.clientId 
                  << " (User: " << conn.authToken.userId << ")" << std::endl;
    });
    
    server.setClientDisconnectCallback([](const ClientConnection& conn) {
        std::cout << "[SERVER] Client disconnected: " << conn.clientId << std::endl;
    });
    
    server.start();
    
    // Simulate client authentication
    std::cout << "\n[DEMO] Simulating doctor client connection..." << std::endl;
    if (server.authenticateClient("client_001", "dr_smith:secure123")) {
        std::cout << "[DEMO] Doctor client authenticated successfully" << std::endl;
    }
    
    // Create device interfaces for each patient
    auto device1 = createDeviceInterface("simulated");
    auto device2 = createDeviceInterface("simulated");
    
    device1->connect("P001");
    device2->connect("P002");
    
    std::cout << "\n[DEVICES] Medical devices connected" << std::endl;
    
    // Setup secure monitoring callbacks
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
        
        // Secure routing to doctor devices
        router.routeSecure(vitals, "doctor_pc_1", adminToken);
        router.routeSecure(vitals, "doctor_watch_1", adminToken);
        
        // Secure broadcast to network clients
        server.sendSecure("client_001", vitals);
    });
    
    device2->setCallback([&](const VitalSigns& vitals) {
        std::cout << "\n[Patient " << vitals.patientId << "] Vitals received:" << std::endl;
        std::cout << "  HR: " << vitals.heartRate << " bpm" << std::endl;
        std::cout << "  BP: " << vitals.systolicBP << "/" << vitals.diastolicBP << " mmHg" << std::endl;
        std::cout << "  O2: " << vitals.oxygenSaturation << "%" << std::endl;
        std::cout << "  Temp: " << vitals.temperature << "°C" << std::endl;
        
        db.storeVitals(vitals);
        alertEngine.checkVitals(vitals);
        router.routeSecure(vitals, "doctor_pc_1", adminToken);
        router.routeSecure(vitals, "doctor_watch_1", adminToken);
        server.sendSecure("client_001", vitals);
    });
    
    // Start monitoring
    std::cout << "\n[MONITORING] Starting secure patient monitoring..." << std::endl;
    std::cout << "[SECURITY] All data encrypted in transit" << std::endl;
    std::cout << "[SECURITY] All access logged for audit" << std::endl;
    std::cout << "\nMonitoring for 60 seconds. Press Ctrl+C to stop." << std::endl;
    
    device1->startMonitoring();
    device2->startMonitoring();
    
    // Run for 60 seconds
    std::this_thread::sleep_for(std::chrono::seconds(60));
    
    // Cleanup
    std::cout << "\n[SHUTDOWN] Stopping monitoring..." << std::endl;
    device1->stopMonitoring();
    device2->stopMonitoring();
    device1->disconnect();
    device2->disconnect();
    server.stop();
    
    // Export audit log
    std::cout << "[AUDIT] Exporting audit log..." << std::endl;
    auditLogger->exportToFile("neuradoc_audit_export.csv");
    
    std::cout << "\n[SHUTDOWN] System shutdown complete" << std::endl;
    std::cout << "All activities logged for compliance" << std::endl;
    
    return 0;
}
