#pragma once
#include "VitalSigns.hpp"
#include <vector>
#include <string>

namespace NeuraDoc {

struct PatientInfo {
    std::string patientId;
    std::string name;
    int age;
    std::string roomNumber;
    std::string assignedDoctor;
};

class PatientDatabase {
public:
    PatientDatabase();
    ~PatientDatabase();
    
    // Patient management
    void addPatient(const PatientInfo& patient);
    void removePatient(const std::string& patientId);
    PatientInfo getPatient(const std::string& patientId) const;
    std::vector<PatientInfo> getAllPatients() const;
    
    // Store vital signs history
    void storeVitals(const VitalSigns& vitals);
    std::vector<VitalSigns> getVitalsHistory(const std::string& patientId, 
                                              std::chrono::hours duration) const;
    
    // Get latest vitals
    VitalSigns getLatestVitals(const std::string& patientId) const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
