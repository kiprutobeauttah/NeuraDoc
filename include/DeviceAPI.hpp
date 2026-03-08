#pragma once
#include "VitalSigns.hpp"
#include "SecurityManager.hpp"
#include <string>
#include <memory>
#include <functional>

namespace NeuraDoc {

struct DeviceRegistration {
    std::string deviceId;
    std::string deviceType;
    std::string manufacturer;
    std::string model;
    std::string serialNumber;
    std::string firmwareVersion;
    std::string location;
};

struct DeviceStatus {
    std::string deviceId;
    bool online;
    std::chrono::system_clock::time_point lastHeartbeat;
    float batteryLevel;
    std::string status;
    std::vector<std::string> errors;
};

class DeviceAPI {
public:
    DeviceAPI(int port, std::shared_ptr<SecurityManager> securityMgr);
    ~DeviceAPI();
    
    void start();
    void stop();
    bool isRunning() const;
    
    bool registerDevice(const DeviceRegistration& device, const std::string& apiKey);
    void unregisterDevice(const std::string& deviceId);
    
    bool authenticateDevice(const std::string& deviceId, const std::string& apiKey);
    
    bool submitVitals(const std::string& deviceId, const VitalSigns& vitals, 
                     const std::string& apiKey);
    bool submitVitalsBatch(const std::string& deviceId, 
                          const std::vector<VitalSigns>& vitalsList,
                          const std::string& apiKey);
    
    void updateDeviceStatus(const std::string& deviceId, const DeviceStatus& status);
    DeviceStatus getDeviceStatus(const std::string& deviceId) const;
    std::vector<DeviceStatus> getAllDeviceStatuses() const;
    
    using VitalsReceivedCallback = std::function<void(const VitalSigns&)>;
    void setVitalsReceivedCallback(VitalsReceivedCallback callback);
    
    std::string generateAPIKey(const std::string& deviceId);
    bool validateAPIKey(const std::string& deviceId, const std::string& apiKey);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
