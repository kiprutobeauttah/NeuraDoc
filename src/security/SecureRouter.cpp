#include "SecureRouter.hpp"
#include "AuditLogger.hpp"
#include <map>
#include <iostream>
#include <sstream>

namespace NeuraDoc {

class SecureRouter::Impl {
public:
    std::shared_ptr<SecurityManager> securityMgr;
    std::shared_ptr<AuditLogger> auditLogger;
    std::map<std::string, SecureEndpoint> endpoints;
    std::map<std::string, std::string> sessionKeys; // endpointId -> sessionKey
    
    Impl(std::shared_ptr<SecurityManager> sm) 
        : securityMgr(sm), auditLogger(std::make_shared<AuditLogger>()) {}
};

SecureRouter::SecureRouter(std::shared_ptr<SecurityManager> securityMgr) 
    : pImpl(std::make_unique<Impl>(securityMgr)) {}

SecureRouter::~SecureRouter() = default;

bool SecureRouter::registerEndpoint(const SecureEndpoint& endpoint, const AuthToken& token) {
    if (!pImpl->securityMgr->validateToken(token)) {
        std::cout << "Authentication failed: Invalid token" << std::endl;
        pImpl->auditLogger->logAuthFailure(token.userId, endpoint.address, "Invalid token");
        return false;
    }
    
    if (!pImpl->securityMgr->hasPermission(token, "write")) {
        std::cout << "Authorization failed: Insufficient permissions" << std::endl;
        pImpl->auditLogger->logAuthFailure(token.userId, endpoint.address, "Insufficient permissions");
        return false;
    }
    
    // Generate session key for this endpoint
    std::string sessionKey = pImpl->securityMgr->generateSessionKey();
    pImpl->sessionKeys[endpoint.endpointId] = sessionKey;
    
    SecureEndpoint secureEp = endpoint;
    secureEp.verified = true;
    secureEp.authToken = token;
    
    pImpl->endpoints[endpoint.endpointId] = secureEp;
    
    std::cout << "Secure endpoint registered: " << endpoint.endpointId 
              << " at " << endpoint.address << std::endl;
    
    pImpl->auditLogger->log({
        AuditEventType::CONFIGURATION_CHANGE,
        token.userId,
        "",
        "Registered endpoint: " + endpoint.endpointId,
        endpoint.address,
        std::chrono::system_clock::now(),
        true
    });
    
    return true;
}

void SecureRouter::unregisterEndpoint(const std::string& endpointId, const AuthToken& token) {
    if (!pImpl->securityMgr->validateToken(token)) {
        return;
    }
    
    pImpl->endpoints.erase(endpointId);
    pImpl->sessionKeys.erase(endpointId);
    
    std::cout << "Endpoint unregistered: " << endpointId << std::endl;
    
    pImpl->auditLogger->log({
        AuditEventType::CONFIGURATION_CHANGE,
        token.userId,
        "",
        "Unregistered endpoint: " + endpointId,
        "",
        std::chrono::system_clock::now(),
        true
    });
}

bool SecureRouter::routeSecure(const VitalSigns& vitals, const std::string& endpointId, 
                               const AuthToken& token) {
    if (!pImpl->securityMgr->validateToken(token)) {
        std::cout << "Routing failed: Invalid authentication" << std::endl;
        return false;
    }
    
    if (!pImpl->securityMgr->canAccessPatient(token, vitals.patientId)) {
        std::cout << "Routing failed: No access to patient " << vitals.patientId << std::endl;
        pImpl->auditLogger->log({
            AuditEventType::UNAUTHORIZED_ACCESS,
            token.userId,
            vitals.patientId,
            "Attempted unauthorized data access",
            "",
            std::chrono::system_clock::now(),
            false
        });
        return false;
    }
    
    auto it = pImpl->endpoints.find(endpointId);
    if (it == pImpl->endpoints.end()) {
        std::cout << "Endpoint not found: " << endpointId << std::endl;
        return false;
    }
    
    if (!it->second.verified) {
        std::cout << "Endpoint not verified: " << endpointId << std::endl;
        return false;
    }
    
    // Get session key for encryption
    auto keyIt = pImpl->sessionKeys.find(endpointId);
    if (keyIt == pImpl->sessionKeys.end()) {
        std::cout << "No session key for endpoint: " << endpointId << std::endl;
        return false;
    }
    
    // Encrypt vitals data
    auto encrypted = encryptVitals(vitals, keyIt->second);
    
    std::cout << "[SECURE] Routing encrypted data for patient " << vitals.patientId 
              << " to endpoint " << endpointId << " (" << encrypted.size() << " bytes)" << std::endl;
    
    // Log transmission
    pImpl->auditLogger->logTransmission(token.userId, vitals.patientId, 
                                       it->second.address);
    
    return true;
}

bool SecureRouter::broadcastSecure(const VitalSigns& vitals, const AuthToken& token) {
    if (!pImpl->securityMgr->validateToken(token)) {
        return false;
    }
    
    bool success = true;
    for (const auto& [endpointId, endpoint] : pImpl->endpoints) {
        if (!routeSecure(vitals, endpointId, token)) {
            success = false;
        }
    }
    
    return success;
}

bool SecureRouter::verifyEndpoint(const std::string& endpointId) {
    auto it = pImpl->endpoints.find(endpointId);
    if (it == pImpl->endpoints.end()) {
        return false;
    }
    
    return it->second.verified && 
           pImpl->securityMgr->validateToken(it->second.authToken);
}

std::vector<uint8_t> SecureRouter::encryptVitals(const VitalSigns& vitals, 
                                                  const std::string& sessionKey) {
    // Serialize vitals to bytes
    std::stringstream ss;
    ss << vitals.patientId << "|"
       << vitals.heartRate << "|"
       << vitals.systolicBP << "|"
       << vitals.diastolicBP << "|"
       << vitals.oxygenSaturation << "|"
       << vitals.temperature << "|"
       << vitals.respiratoryRate;
    
    std::string data = ss.str();
    std::vector<uint8_t> bytes(data.begin(), data.end());
    
    // Encrypt using SecurityManager
    return pImpl->securityMgr->encrypt(bytes, sessionKey);
}

VitalSigns SecureRouter::decryptVitals(const std::vector<uint8_t>& encrypted, 
                                       const std::string& sessionKey) {
    // Decrypt
    auto decrypted = pImpl->securityMgr->decrypt(encrypted, sessionKey);
    
    // Deserialize
    std::string data(decrypted.begin(), decrypted.end());
    std::stringstream ss(data);
    
    VitalSigns vitals;
    char delimiter;
    ss >> vitals.patientId >> delimiter
       >> vitals.heartRate >> delimiter
       >> vitals.systolicBP >> delimiter
       >> vitals.diastolicBP >> delimiter
       >> vitals.oxygenSaturation >> delimiter
       >> vitals.temperature >> delimiter
       >> vitals.respiratoryRate;
    
    vitals.timestamp = std::chrono::system_clock::now();
    return vitals;
}

} // namespace NeuraDoc
