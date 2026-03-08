#pragma once
#include "VitalSigns.hpp"
#include "PatientDatabase.hpp"
#include <string>
#include <memory>
#include <vector>

namespace NeuraDoc {

struct DatabaseConfig {
    std::string host;
    int port;
    std::string database;
    std::string username;
    std::string password;
    int maxConnections;
};

class PostgreSQLDatabase {
public:
    PostgreSQLDatabase(const DatabaseConfig& config);
    ~PostgreSQLDatabase();
    
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    void storeVitals(const VitalSigns& vitals);
    void storeVitalsBatch(const std::vector<VitalSigns>& vitalsList);
    
    std::vector<VitalSigns> getVitalsHistory(const std::string& patientId, 
                                              std::chrono::hours duration) const;
    VitalSigns getLatestVitals(const std::string& patientId) const;
    
    void storePatient(const PatientInfo& patient);
    PatientInfo getPatient(const std::string& patientId) const;
    std::vector<PatientInfo> getAllPatients() const;
    
    void storeAlert(const std::string& patientId, const std::string& alertType,
                   const std::string& message, const std::string& severity);
    
    std::vector<std::string> getRecentAlerts(std::chrono::hours duration) const;
    
    void createTables();
    void optimizeForRealTime();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
