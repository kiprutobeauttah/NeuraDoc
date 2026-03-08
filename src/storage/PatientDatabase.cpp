#include "PatientDatabase.hpp"
#include <map>
#include <vector>
#include <algorithm>

namespace NeuraDoc {

class PatientDatabase::Impl {
public:
    std::map<std::string, PatientInfo> patients;
    std::map<std::string, std::vector<VitalSigns>> vitalsHistory;
};

PatientDatabase::PatientDatabase() : pImpl(std::make_unique<Impl>()) {}
PatientDatabase::~PatientDatabase() = default;

void PatientDatabase::addPatient(const PatientInfo& patient) {
    pImpl->patients[patient.patientId] = patient;
}

void PatientDatabase::removePatient(const std::string& patientId) {
    pImpl->patients.erase(patientId);
    pImpl->vitalsHistory.erase(patientId);
}

PatientInfo PatientDatabase::getPatient(const std::string& patientId) const {
    auto it = pImpl->patients.find(patientId);
    if (it != pImpl->patients.end()) {
        return it->second;
    }
    return PatientInfo{};
}

std::vector<PatientInfo> PatientDatabase::getAllPatients() const {
    std::vector<PatientInfo> result;
    for (const auto& [id, patient] : pImpl->patients) {
        result.push_back(patient);
    }
    return result;
}

void PatientDatabase::storeVitals(const VitalSigns& vitals) {
    pImpl->vitalsHistory[vitals.patientId].push_back(vitals);
}

std::vector<VitalSigns> PatientDatabase::getVitalsHistory(const std::string& patientId, 
                                                           std::chrono::hours duration) const {
    auto it = pImpl->vitalsHistory.find(patientId);
    if (it == pImpl->vitalsHistory.end()) {
        return {};
    }
    
    auto cutoff = std::chrono::system_clock::now() - duration;
    std::vector<VitalSigns> result;
    
    for (const auto& vitals : it->second) {
        if (vitals.timestamp >= cutoff) {
            result.push_back(vitals);
        }
    }
    
    return result;
}

VitalSigns PatientDatabase::getLatestVitals(const std::string& patientId) const {
    auto it = pImpl->vitalsHistory.find(patientId);
    if (it != pImpl->vitalsHistory.end() && !it->second.empty()) {
        return it->second.back();
    }
    return VitalSigns{};
}

} // namespace NeuraDoc
