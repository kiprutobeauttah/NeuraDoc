#include "DeviceInterface.hpp"
#include "DeviceAPI.hpp"
#include "WebAPI.hpp"
#include "SecureRouter.hpp"
#include "AlertEngine.hpp"
#include "PostgreSQLDatabase.hpp"
#include "SecureNetworkServer.hpp"
#include "SecurityManager.hpp"
#include "AuditLogger.hpp"
#include "RealTimeAnalyzer.hpp"
#include <iostream>
#include <thread>
#include <signal.h>

using namespace NeuraDoc;

std::atomic<bool> running(true);

void signalHandler(int signum) {
    std::cout << "\n[SHUTDOWN] Interrupt signal received" << std::endl;
    running = false;
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "============================================" << std::endl;
    std::cout << "NeuraDoc Production Patient Monitoring System" << std::endl;
    std::cout << "============================================" << std::endl;
    
    auto securityMgr = std::make_shared<SecurityManager>();
    auto auditLogger = std::make_shared<AuditLogger>();
    
    std::cout << "\n[INIT] Initializing security..." << std::endl;
    securityMgr->registerUser("dr_smith", "secure123", UserRole::DOCTOR);
    securityMgr->registerUser("nurse_jane", "nurse456", UserRole::NURSE);
    
    DatabaseConfig dbConfig;
    dbConfig.host = "localhost";
    dbConfig.port = 5432;
    dbConfig.database = "neuradoc";
    dbConfig.username = "neuradoc_user";
    dbConfig.password = "neuradoc_pass";
    dbConfig.maxConnections = 20;
    
    std::cout << "[INIT] Connecting to PostgreSQL..." << std::endl;
    auto database = std::make_shared<PostgreSQLDatabase>(dbConfig);
    if (!database->connect()) {
        std::cerr << "[ERROR] Failed to connect to database" << std::endl;
        return 1;
    }
    database->optimizeForRealTime();
    
    PatientInfo patient1{"P001", "John Doe", 45, "Room 101", "Dr. Smith"};
    PatientInfo patient2{"P002", "Jane Smith", 62, "Room 102", "Dr. Smith"};
    database->storePatient(patient1);
    database->storePatient(patient2);
    
    std::cout << "[INIT] Initializing components..." << std::endl;
    SecureRouter router(securityMgr);
    AlertEngine alertEngine;
    RealTimeAnalyzer analyzer;
    SecureNetworkServer server(8443, securityMgr);
    DeviceAPI deviceAPI(9000, securityMgr);
    WebAPI webAPI(8080, securityMgr, database);
    
    AuthToken adminToken = securityMgr->authenticate("admin", "admin123");
    
    SecureEndpoint doctorPC;
    doctorPC.endpointId = "doctor_pc_1";
    doctorPC.address = "192.168.1.100:9000";
    doctorPC.publicKey = "PUBLIC_KEY";
    doctorPC.authToken = adminToken;
    router.registerEndpoint(doctorPC, adminToken);
    
    alertEngine.setAlertCallback([&](const Alert& alert) {
        std::cout << "\n*** ALERT ***" << std::endl;
        std::cout << "Patient: " << alert.patientId << std::endl;
        std::cout << "Level: " << static_cast<int>(alert.level) << std::endl;
        std::cout << "Message: " << alert.message << std::endl;
        
        database->storeAlert(alert.patientId, "CRITICAL", alert.message, 
                           std::to_string(static_cast<int>(alert.level)));
        
        webAPI.broadcastAlert(alert.patientId, alert.message);
        
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
    
    analyzer.setAnalysisCallback([](const AnalysisResult& result) {
        if (result.anomalyDetected) {
            std::cout << "[ANALYSIS] Anomaly detected for patient " << result.patientId 
                      << ": " << result.anomalyType << " (Risk: " << result.riskScore << ")" << std::endl;
        }
    });
    
    deviceAPI.setVitalsReceivedCallback([&](const VitalSigns& vitals) {
        std::cout << "\n[VITALS] Patient " << vitals.patientId << ": "
                  << "HR=" << vitals.heartRate << " BP=" << vitals.systolicBP << "/" << vitals.diastolicBP
                  << " O2=" << vitals.oxygenSaturation << "% T=" << vitals.temperature << "°C" << std::endl;
        
        database->storeVitals(vitals);
        
        auto analysis = analyzer.analyze(vitals);
        
        alertEngine.checkVitals(vitals);
        
        router.routeSecure(vitals, "doctor_pc_1", adminToken);
        
        server.sendSecure("client_001", vitals);
        
        webAPI.broadcastVitals(vitals);
    });
    
    std::cout << "\n[START] Starting services..." << std::endl;
    server.start();
    deviceAPI.start();
    webAPI.start();
    
    server.authenticateClient("client_001", "dr_smith:secure123");
    
    DeviceRegistration device1;
    device1.deviceId = "DEV001";
    device1.deviceType = "VitalSignsMonitor";
    device1.manufacturer = "MedTech";
    device1.model = "VSM-3000";
    device1.serialNumber = "SN123456";
    device1.firmwareVersion = "2.1.0";
    device1.location = "Room 101";
    
    std::string apiKey1 = deviceAPI.generateAPIKey(device1.deviceId);
    deviceAPI.registerDevice(device1, apiKey1);
    
    auto simDevice = createDeviceInterface("simulated");
    simDevice->connect("P001");
    
    simDevice->setCallback([&](const VitalSigns& vitals) {
        deviceAPI.submitVitals("DEV001", vitals, apiKey1);
    });
    
    std::cout << "\n[MONITORING] System operational" << std::endl;
    std::cout << "[MONITORING] Press Ctrl+C to stop" << std::endl;
    
    simDevice->startMonitoring();
    
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\n[SHUTDOWN] Stopping services..." << std::endl;
    simDevice->stopMonitoring();
    simDevice->disconnect();
    deviceAPI.stop();
    webAPI.stop();
    server.stop();
    database->disconnect();
    
    auditLogger->exportToFile("neuradoc_final_audit.csv");
    
    std::cout << "[SHUTDOWN] System stopped" << std::endl;
    return 0;
}
