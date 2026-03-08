#include "NetworkServer.hpp"
#include <map>
#include <iostream>

namespace NeuraDoc {

class NetworkServer::Impl {
public:
    int port;
    bool running = false;
    std::map<std::string, std::string> clients;
    ClientConnectCallback connectCallback;
    ClientConnectCallback disconnectCallback;
    
    Impl(int p) : port(p) {}
};

NetworkServer::NetworkServer(int port) : pImpl(std::make_unique<Impl>(port)) {}
NetworkServer::~NetworkServer() {
    stop();
}

void NetworkServer::start() {
    if (pImpl->running) return;
    
    pImpl->running = true;
    std::cout << "Network server started on port " << pImpl->port << std::endl;
    // In production: start actual network server
}

void NetworkServer::stop() {
    if (!pImpl->running) return;
    
    pImpl->running = false;
    std::cout << "Network server stopped" << std::endl;
}

bool NetworkServer::isRunning() const {
    return pImpl->running;
}

void NetworkServer::sendToClient(const std::string& clientId, const VitalSigns& vitals) {
    if (!pImpl->running) return;
    
    std::cout << "Sending vitals for patient " << vitals.patientId 
              << " to client " << clientId << std::endl;
    // In production: send data over network
}

void NetworkServer::broadcast(const VitalSigns& vitals) {
    for (const auto& [clientId, address] : pImpl->clients) {
        sendToClient(clientId, vitals);
    }
}

void NetworkServer::setClientConnectCallback(ClientConnectCallback callback) {
    pImpl->connectCallback = callback;
}

void NetworkServer::setClientDisconnectCallback(ClientConnectCallback callback) {
    pImpl->disconnectCallback = callback;
}

} // namespace NeuraDoc
