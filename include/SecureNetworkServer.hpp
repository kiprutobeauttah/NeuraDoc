#pragma once
#include "VitalSigns.hpp"
#include "SecurityManager.hpp"
#include <string>
#include <memory>
#include <functional>

namespace NeuraDoc {

struct ClientConnection {
    std::string clientId;
    std::string ipAddress;
    AuthToken authToken;
    std::string sessionKey;
    std::chrono::system_clock::time_point connectedAt;
    bool authenticated;
};

class SecureNetworkServer {
public:
    SecureNetworkServer(int port, std::shared_ptr<SecurityManager> securityMgr);
    ~SecureNetworkServer();
    
    // Server lifecycle
    void start();
    void stop();
    bool isRunning() const;
    
    // Client authentication
    bool authenticateClient(const std::string& clientId, const std::string& credentials);
    void disconnectClient(const std::string& clientId);
    
    // Secure data transmission
    void sendSecure(const std::string& clientId, const VitalSigns& vitals);
    void broadcastSecure(const VitalSigns& vitals);
    
    // Connection callbacks
    using ClientAuthCallback = std::function<void(const ClientConnection&)>;
    void setClientAuthCallback(ClientAuthCallback callback);
    void setClientDisconnectCallback(ClientAuthCallback callback);
    
    // Security monitoring
    std::vector<ClientConnection> getActiveConnections() const;
    void auditLog(const std::string& event, const std::string& details);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
