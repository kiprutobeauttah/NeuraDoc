#pragma once
#include "VitalSigns.hpp"
#include <vector>
#include <string>

namespace NeuraDoc {

class DataRouter {
public:
    DataRouter();
    ~DataRouter();
    
    // Register endpoints (doctor devices)
    void registerEndpoint(const std::string& endpointId, const std::string& address);
    void unregisterEndpoint(const std::string& endpointId);
    
    // Route patient data to registered endpoints
    void routeData(const VitalSigns& vitals);
    
    // Route to specific endpoint
    void routeToEndpoint(const VitalSigns& vitals, const std::string& endpointId);
    
    // Broadcast to all endpoints
    void broadcast(const VitalSigns& vitals);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
