#pragma once
#include <string>
#include <chrono>
#include <memory>

namespace NeuraDoc {

enum class AuditEventType {
    USER_LOGIN,
    USER_LOGOUT,
    DATA_ACCESS,
    DATA_TRANSMISSION,
    ALERT_TRIGGERED,
    CONFIGURATION_CHANGE,
    AUTHENTICATION_FAILURE,
    UNAUTHORIZED_ACCESS
};

struct AuditEvent {
    AuditEventType type;
    std::string userId;
    std::string patientId;
    std::string details;
    std::string ipAddress;
    std::chrono::system_clock::time_point timestamp;
    bool success;
};

class AuditLogger {
public:
    AuditLogger();
    ~AuditLogger();
    
    // Log events
    void log(const AuditEvent& event);
    void logLogin(const std::string& userId, const std::string& ipAddress, bool success);
    void logDataAccess(const std::string& userId, const std::string& patientId);
    void logTransmission(const std::string& userId, const std::string& patientId, 
                        const std::string& destination);
    void logAuthFailure(const std::string& userId, const std::string& ipAddress, 
                       const std::string& reason);
    
    // Query logs
    std::vector<AuditEvent> getEvents(std::chrono::hours duration) const;
    std::vector<AuditEvent> getEventsForUser(const std::string& userId) const;
    std::vector<AuditEvent> getEventsForPatient(const std::string& patientId) const;
    
    // Export logs
    void exportToFile(const std::string& filename) const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
