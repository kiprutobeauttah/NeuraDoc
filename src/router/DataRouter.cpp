#include "DataRouter.hpp"
#include <map>
#include <iostream>

namespace NeuraDoc {

class DataRouter::Impl {
public:
    std::map<std::string, std::string> endpoints;
};

DataRouter::DataRouter() : pImpl(std::make_unique<Impl>()) {}
DataRouter::~DataRouter() = default;

void DataRouter::registerEndpoint(const std::string& endpointId, const std::string& address) {
    pImpl->endpoints[endpointId] = address;
    std::cout << "Registered endpoint: " << endpointId << " at " << address << std::endl;
}

void DataRouter::unregisterEndpoint(const std::string& endpointId) {
    pImpl->endpoints.erase(endpointId);
    std::cout << "Unregistered endpoint: " << endpointId << std::endl;
}

void DataRouter::routeData(const VitalSigns& vitals) {
    broadcast(vitals);
}

void DataRouter::routeToEndpoint(const VitalSigns& vitals, const std::string& endpointId) {
    auto it = pImpl->endpoints.find(endpointId);
    if (it != pImpl->endpoints.end()) {
        std::cout << "Routing data for patient " << vitals.patientId 
                  << " to endpoint " << endpointId << std::endl;
        // In production: send data over network to endpoint
    }
}

void DataRouter::broadcast(const VitalSigns& vitals) {
    for (const auto& [endpointId, address] : pImpl->endpoints) {
        routeToEndpoint(vitals, endpointId);
    }
}

} // namespace NeuraDoc
