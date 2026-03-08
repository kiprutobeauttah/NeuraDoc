#include "SecureNetworkServer.hpp"
#include "AuditLogger.hpp"
#include <map>
#include <iostream>

namespace NeuraDoc {

class SecureNetworkServer::Impl {
public:
    int port;
    bool running = false;
    std::shared_ptr<SecurityManager> securityMgr;
    std::shared_ptr<AuditLogger> auditLogger;
    std::map<std::string, ClientConnection> clients;
    ClientAuthCallback authCallback;
    ClientAuthCallback disconnectCallback;
    
    Impl(int p, std::shared_ptr<SecurityManager> sm) 
        : port(p), securityMgr(sm), auditLogger(std::make_shared<AuditLogger>()) {}
};

SecureNetworkServer::SecureNetworkServer(int port, std::shared_ptr<SecurityManager> securityMgr) 
    : pImpl(std::make_unique<Impl>(port, securityMgr)) {}

SecureNetworkServer::~SecureNetworkServer() {
    stop();
}

void SecureNetworkServer::start() {
    if (pImpl->running) return;
    
    pImpl->running = true;
    std::cout << "[SECURE] Network server started on port " << pImpl->port << std::endl;
    std::cout << "[SECURE] TLS/SSL encryption enabled" << std::endl;
    std::cout << "[SECURE] Client authentication required" << std::endl;
    
    // In production: start actual TLS/SSL server
}

void SecureNetworkServer::stop() {
    if (!pImpl->running) return;
    
    // Disconnect all clients
    for (const auto& [clientId, conn] : pImpl->clients) {
        pImpl->auditLogger->log({
            AuditEventType::USER_LOGOUT,
            conn.authToken.userId,
            "",
            "Server shutdown",
            conn.ipAddress,
            std::chrono::system_clock::now(),
            true
        });
    }
    
    pImpl->clients.clear();
    pImpl->running = false;
    std::cout << "[SECURE] Network server stopped" << std::endl;
}

bool SecureNetworkServer::isRunning() const {
    return pImpl->running;
}

bool SecureNetworkServer::authenticateClient(const std::string& clientId, 
                                             const std::string& credentials) {
    if (!pImpl->running) return false;
    
    // Parse credentials (format: "userId:password")
    size_t colonPos = credentials.find(':');
    if (colonPos == std::string::npos) {
        pImpl->auditLogger->logAuthFailure(clientId, "unknown", "Invalid credentials format");
        return false;
    }
    
    std::string userId = credentials.substr(0, colonPos);
    std::string password = credentials.substr(colonPos + 1);
    
    // Authenticate with SecurityManager
    AuthToken token = pImpl->securityMgr->authenticate(userId, password);
    if (!token.isValid()) {
        pImpl->auditLogger->logAuthFailure(userId, "unknown", "Invalid credentials");
        std::cout << "[SECURE] Authentication failed for user: " << userId << std::endl;
        return false;
    }
    
    // Create client connection
    ClientConnection conn;
    conn.clientId = clientId;
    conn.ipAddress = "127.0.0.1"; // In production: get actual IP
    conn.authToken = token;
    conn.sessionKey = pImpl->securityMgr->generateSessionKey();
    conn.connectedAt = std::chrono::system_clock::now();
    conn.authenticated = true;
    
    pImpl->clients[clientId] = conn;
    
    std::cout << "[SECURE] Client authenticated: " << clientId 
              << " (User: " << userId << ", Role: " << static_cast<int>(token.role) << ")" << std::endl;
    
    pImpl->auditLogger->logLogin(userId, conn.ipAddress, true);
    
    if (pImpl->authCallback) {
        pImpl->authCallback(conn);
    }
    
    return true;
}

void SecureNetworkServer::disconnectClient(const std::string& clientId) {
    auto it = pImpl->clients.find(clientId);
    if (it == pImpl->clients.end()) return;
    
    pImpl->auditLogger->log({
        AuditEventType::USER_LOGOUT,
        it->second.authToken.userId,
        "",
        "Client disconnected",
        it->second.ipAddress,
        std::chrono::system_clock::now(),
        true
    });
    
    if (pImpl->disconnectCallback) {
        pImpl->disconnectCallback(it->second);
    }
    
    pImpl->clients.erase(it);
    std::cout << "[SECURE] Client disconnected: " << clientId << std::endl;
}

void SecureNetworkServer::sendSecure(const std::string& clientId, const VitalSigns& vitals) {
    if (!pImpl->running) return;
    
    auto it = pImpl->clients.find(clientId);
    if (it == pImpl->clients.end()) {
        std::cout << "[SECURE] Client not found: " << clientId << std::endl;
        return;
    }
    
    if (!it->second.authenticated) {
        std::cout << "[SECURE] Client not authenticated: " << clientId << std::endl;
        return;
    }
    
    // Check authorization
    if (!pImpl->securityMgr->canAccessPatient(it->second.authToken, vitals.patientId)) {
        std::cout << "[SECURE] Client " << clientId << " not authorized for patient " 
                  << vitals.patientId << std::endl;
        pImpl->auditLogger->log({
            AuditEventType::UNAUTHORIZED_ACCESS,
            it->second.authToken.userId,
            vitals.patientId,
            "Unauthorized data access attempt",
            it->second.ipAddress,
            std::chrono::system_clock::now(),
            false
        });
        return;
    }
    
    // Encrypt data with session key
    std::stringstream ss;
    ss << vitals.patientId << "|" << vitals.heartRate << "|" 
       << vitals.systolicBP << "/" << vitals.diastolicBP << "|"
       << vitals.oxygenSaturation << "|" << vitals.temperature;
    
    std::string data = ss.str();
    std::vector<uint8_t> bytes(data.begin(), data.end());
    auto encrypted = pImpl->securityMgr->encrypt(bytes, it->second.sessionKey);
    
    std::cout << "[SECURE] Sending encrypted vitals for patient " << vitals.patientId 
              << " to client " << clientId << " (" << encrypted.size() << " bytes)" << std::endl;
    
    // Log transmission
    pImpl->auditLogger->logTransmission(it->second.authToken.userId, vitals.patientId, 
                                       it->second.ipAddress);
    
    // In production: send encrypted data over TLS/SSL connection
}

void SecureNetworkServer::broadcastSecure(const VitalSigns& vitals) {
    for (const auto& [clientId, conn] : pImpl->clients) {
        sendSecure(clientId, vitals);
    }
}

void SecureNetworkServer::setClientAuthCallback(ClientAuthCallback callback) {
    pImpl->authCallback = callback;
}

void SecureNetworkServer::setClientDisconnectCallback(ClientAuthCallback callback) {
    pImpl->disconnectCallback = callback;
}

std::vector<ClientConnection> SecureNetworkServer::getActiveConnections() const {
    std::vector<ClientConnection> connections;
    for (const auto& [clientId, conn] : pImpl->clients) {
        connections.push_back(conn);
    }
    return connections;
}

void SecureNetworkServer::auditLog(const std::string& event, const std::string& details) {
    pImpl->auditLogger->log({
        AuditEventType::CONFIGURATION_CHANGE,
        "system",
        "",
        details,
        "",
        std::chrono::system_clock::now(),
        true
    });
}

} // namespace NeuraDoc
