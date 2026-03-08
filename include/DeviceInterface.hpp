#pragma once
#include "VitalSigns.hpp"
#include <memory>
#include <functional>

namespace NeuraDoc {

class DeviceInterface {
public:
    virtual ~DeviceInterface() = default;
    
    // Connect to medical device
    virtual bool connect(const std::string& deviceId) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    
    // Read vital signs from device
    virtual VitalSigns readVitals() = 0;
    
    // Set callback for continuous monitoring
    using VitalsCallback = std::function<void(const VitalSigns&)>;
    virtual void setCallback(VitalsCallback callback) = 0;
    
    // Start/stop continuous monitoring
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
};

// Factory for creating device interfaces
std::unique_ptr<DeviceInterface> createDeviceInterface(const std::string& deviceType);

} // namespace NeuraDoc
