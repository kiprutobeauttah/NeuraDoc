#pragma once
#include "VitalSigns.hpp"
#include "SecurityManager.hpp"
#include <string>
#include <memory>

namespace NeuraDoc {

struct SecureEndpoint {
    std::string endpointId;
    std::string address;
    std::string publicKey;
    AuthToken authToken;
    bool verified;
};

class SecureRouter {
public:
    SecureRouter(std::shared_ptr<SecurityManager> securityMgr);
    ~SecureRouter();
    
    // Endpoint management with authentication
    bool registerEndpoint(const SecureEndpoint& endpoint, const AuthToken& token);
    void unregisterEndpoint(const std::string& endpointId, const AuthToken& token);
    
    // Secure data routing with encryption
    bool routeSecure(const VitalSigns& vitals, const std::string& endpointId, 
                     const AuthToken& token);
    bool broadcastSecure(const VitalSigns& vitals, const AuthToken& token);
    
    // Verify endpoint authenticity
    bool verifyEndpoint(const std::string& endpointId);
    
    // Get encrypted data for transmission
    std::vector<uint8_t> encryptVitals(const VitalSigns& vitals, const std::string& sessionKey);
    VitalSigns decryptVitals(const std::vector<uint8_t>& encrypted, const std::string& sessionKey);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
