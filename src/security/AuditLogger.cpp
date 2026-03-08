#include "AuditLogger.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace NeuraDoc {

class AuditLogger::Impl {
public:
    std::vector<AuditEvent> events;
    std::string logFile = "neuradoc_audit.log";
};

AuditLogger::AuditLogger() : pImpl(std::make_unique<Impl>()) {}
AuditLogger::~AuditLogger() = default;

void AuditLogger::log(const AuditEvent& event) {
    pImpl->events.push_back(event);
    
    // Also write to file immediately
    std::ofstream file(pImpl->logFile, std::ios::app);
    if (file.is_open()) {
        auto time = std::chrono::system_clock::to_time_t(event.timestamp);
        file << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << " | "
             << static_cast<int>(event.type) << " | "
             << event.userId << " | "
             << event.patientId << " | "
             << event.ipAddress << " | "
             << (event.success ? "SUCCESS" : "FAILURE") << " | "
             << event.details << std::endl;
        file.close();
    }
}

void AuditLogger::logLogin(const std::string& userId, const std::string& ipAddress, bool success) {
    AuditEvent event;
    event.type = AuditEventType::USER_LOGIN;
    event.userId = userId;
    event.ipAddress = ipAddress;
    event.timestamp = std::chrono::system_clock::now();
    event.success = success;
    event.details = success ? "User logged in" : "Login failed";
    log(event);
}

void AuditLogger::logDataAccess(const std::string& userId, const std::string& patientId) {
    AuditEvent event;
    event.type = AuditEventType::DATA_ACCESS;
    event.userId = userId;
    event.patientId = patientId;
    event.timestamp = std::chrono::system_clock::now();
    event.success = true;
    event.details = "Accessed patient data";
    log(event);
}

void AuditLogger::logTransmission(const std::string& userId, const std::string& patientId, 
                                  const std::string& destination) {
    AuditEvent event;
    event.type = AuditEventType::DATA_TRANSMISSION;
    event.userId = userId;
    event.patientId = patientId;
    event.timestamp = std::chrono::system_clock::now();
    event.success = true;
    event.details = "Data transmitted to " + destination;
    log(event);
}

void AuditLogger::logAuthFailure(const std::string& userId, const std::string& ipAddress, 
                                const std::string& reason) {
    AuditEvent event;
    event.type = AuditEventType::AUTHENTICATION_FAILURE;
    event.userId = userId;
    event.ipAddress = ipAddress;
    event.timestamp = std::chrono::system_clock::now();
    event.success = false;
    event.details = "Authentication failed: " + reason;
    log(event);
}

std::vector<AuditEvent> AuditLogger::getEvents(std::chrono::hours duration) const {
    auto cutoff = std::chrono::system_clock::now() - duration;
    std::vector<AuditEvent> result;
    
    for (const auto& event : pImpl->events) {
        if (event.timestamp >= cutoff) {
            result.push_back(event);
        }
    }
    
    return result;
}

std::vector<AuditEvent> AuditLogger::getEventsForUser(const std::string& userId) const {
    std::vector<AuditEvent> result;
    
    for (const auto& event : pImpl->events) {
        if (event.userId == userId) {
            result.push_back(event);
        }
    }
    
    return result;
}

std::vector<AuditEvent> AuditLogger::getEventsForPatient(const std::string& patientId) const {
    std::vector<AuditEvent> result;
    
    for (const auto& event : pImpl->events) {
        if (event.patientId == patientId) {
            result.push_back(event);
        }
    }
    
    return result;
}

void AuditLogger::exportToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    
    file << "Timestamp,Event Type,User ID,Patient ID,IP Address,Success,Details" << std::endl;
    
    for (const auto& event : pImpl->events) {
        auto time = std::chrono::system_clock::to_time_t(event.timestamp);
        file << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << ","
             << static_cast<int>(event.type) << ","
             << event.userId << ","
             << event.patientId << ","
             << event.ipAddress << ","
             << (event.success ? "SUCCESS" : "FAILURE") << ","
             << event.details << std::endl;
    }
    
    file.close();
    std::cout << "Audit log exported to " << filename << std::endl;
}

} // namespace NeuraDoc
