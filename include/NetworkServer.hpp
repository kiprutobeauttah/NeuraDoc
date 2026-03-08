#pragma once
#include "VitalSigns.hpp"
#include <string>
#include <functional>

namespace NeuraDoc {

class NetworkServer {
public:
    NetworkServer(int port);
    ~NetworkServer();
    
    // Start/stop server
    void start();
    void stop();
    bool isRunning() const;
    
    // Send data to connected clients
    void sendToClient(const std::string& clientId, const VitalSigns& vitals);
    void broadcast(const VitalSigns& vitals);
    
    // Client connection callbacks
    using ClientConnectCallback = std::function<void(const std::string& clientId)>;
    void setClientConnectCallback(ClientConnectCallback callback);
    void setClientDisconnectCallback(ClientConnectCallback callback);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
