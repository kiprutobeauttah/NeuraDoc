#include "DeviceAPI.hpp"
#include "AuditLogger.hpp"
#include <map>
#include <iostream>
#include <sstream>
#include <random>

namespace NeuraDoc {

class DeviceAPI::Impl {
public:
    int port;
    bool running = false;
    std::shared_ptr<SecurityManager> securityMgr;
    std::shared_ptr<AuditLogger> auditLogger;
    std::map<std::string, DeviceRegistration> devices;
    std::map<std::string, std::string> apiKeys;
    std::map<std::string, DeviceStatus> deviceStatuses;
    VitalsReceivedCallback vitalsCallback;
    
    Impl(int p, std::shared_ptr<SecurityManager> sm) 
        : port(p), securityMgr(sm), auditLogger(std::make_shared<AuditLogger>()) {}
    
    std::string generateKey() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::stringstream ss;
        ss << "NDK_";
        for (int i = 0; i < 32; ++i) {
            ss << std::hex << dis(gen);
        }
        return ss.str();
    }
};

DeviceAPI::DeviceAPI(int port, std::shared_ptr<SecurityManager> securityMgr) 
    : pImpl(std::make_unique<Impl>(port, securityMgr)) {}

DeviceAPI::~DeviceAPI() {
    stop();
}

void DeviceAPI::start() {
    if (pImpl->running) return;
    
    pImpl->running = true;
    std::cout << "[DEVICE API] Started on port " << pImpl->port << std::endl;
    std::cout << "[DEVICE API] Endpoints:" << std::endl;
    std::cout << "  POST /api/device/register - Register new device" << std::endl;
    std::cout << "  POST /api/device/vitals - Submit vital signs" << std::endl;
    std::cout << "  POST /api/device/vitals/batch - Submit batch vitals" << std::endl;
    std::cout << "  GET  /api/device/status - Get device status" << std::endl;
    std::cout << "  POST /api/device/heartbeat - Device heartbeat" << std::endl;
}

void DeviceAPI::stop() {
    if (!pImpl->running) return;
    
    pImpl->running = false;
    std::cout << "[DEVICE API] Stopped" << std::endl;
}

bool DeviceAPI::isRunning() const {
    return pImpl->running;
}

bool DeviceAPI::registerDevice(const DeviceRegistration& device, const std::string& apiKey) {
    if (!pImpl->running) return false;
    
    if (!validateAPIKey(device.deviceId, apiKey)) {
        std::string newKey = generateAPIKey(device.deviceId);
        pImpl->apiKeys[device.deviceId] = newKey;
        std::cout << "[DEVICE API] Generated API key for device: " << device.deviceId << std::endl;
        std::cout << "[DEVICE API] API Key: " << newKey << std::endl;
    }
    
    pImpl->devices[device.deviceId] = device;
    
    DeviceStatus status;
    status.deviceId = device.deviceId;
    status.online = true;
    status.lastHeartbeat = std::chrono::system_clock::now();
    status.batteryLevel = 100.0f;
    status.status = "registered";
    pImpl->deviceStatuses[device.deviceId] = status;
    
    std::cout << "[DEVICE API] Device registered: " << device.deviceId 
              << " (" << device.manufacturer << " " << device.model << ")" << std::endl;
    
    pImpl->auditLogger->log({
        AuditEventType::CONFIGURATION_CHANGE,
        "system",
        "",
        "Device registered: " + device.deviceId,
        device.location,
        std::chrono::system_clock::now(),
        true
    });
    
    return true;
}

void DeviceAPI::unregisterDevice(const std::string& deviceId) {
    pImpl->devices.erase(deviceId);
    pImpl->apiKeys.erase(deviceId);
    pImpl->deviceStatuses.erase(deviceId);
    
    std::cout << "[DEVICE API] Device unregistered: " << deviceId << std::endl;
}

bool DeviceAPI::authenticateDevice(const std::string& deviceId, const std::string& apiKey) {
    return validateAPIKey(deviceId, apiKey);
}

bool DeviceAPI::submitVitals(const std::string& deviceId, const VitalSigns& vitals, 
                             const std::string& apiKey) {
    if (!pImpl->running) return false;
    
    if (!validateAPIKey(deviceId, apiKey)) {
        std::cout << "[DEVICE API] Authentication failed for device: " << deviceId << std::endl;
        pImpl->auditLogger->logAuthFailure(deviceId, "device", "Invalid API key");
        return false;
    }
    
    auto it = pImpl->devices.find(deviceId);
    if (it == pImpl->devices.end()) {
        std::cout << "[DEVICE API] Device not registered: " << deviceId << std::endl;
        return false;
    }
    
    std::cout << "[DEVICE API] Vitals received from device " << deviceId 
              << " for patient " << vitals.patientId << std::endl;
    
    auto& status = pImpl->deviceStatuses[deviceId];
    status.lastHeartbeat = std::chrono::system_clock::now();
    status.online = true;
    
    if (pImpl->vitalsCallback) {
        pImpl->vitalsCallback(vitals);
    }
    
    pImpl->auditLogger->log({
        AuditEventType::DATA_TRANSMISSION,
        deviceId,
        vitals.patientId,
        "Vitals submitted via Device API",
        it->second.location,
        std::chrono::system_clock::now(),
        true
    });
    
    return true;
}

bool DeviceAPI::submitVitalsBatch(const std::string& deviceId, 
                                  const std::vector<VitalSigns>& vitalsList,
                                  const std::string& apiKey) {
    if (!validateAPIKey(deviceId, apiKey)) {
        return false;
    }
    
    std::cout << "[DEVICE API] Batch vitals received from device " << deviceId 
              << " (" << vitalsList.size() << " records)" << std::endl;
    
    for (const auto& vitals : vitalsList) {
        if (pImpl->vitalsCallback) {
            pImpl->vitalsCallback(vitals);
        }
    }
    
    return true;
}

void DeviceAPI::updateDeviceStatus(const std::string& deviceId, const DeviceStatus& status) {
    pImpl->deviceStatuses[deviceId] = status;
}

DeviceStatus DeviceAPI::getDeviceStatus(const std::string& deviceId) const {
    auto it = pImpl->deviceStatuses.find(deviceId);
    if (it != pImpl->deviceStatuses.end()) {
        return it->second;
    }
    return DeviceStatus{};
}

std::vector<DeviceStatus> DeviceAPI::getAllDeviceStatuses() const {
    std::vector<DeviceStatus> statuses;
    for (const auto& [id, status] : pImpl->deviceStatuses) {
        statuses.push_back(status);
    }
    return statuses;
}

void DeviceAPI::setVitalsReceivedCallback(VitalsReceivedCallback callback) {
    pImpl->vitalsCallback = callback;
}

std::string DeviceAPI::generateAPIKey(const std::string& deviceId) {
    std::string key = pImpl->generateKey();
    pImpl->apiKeys[deviceId] = key;
    return key;
}

bool DeviceAPI::validateAPIKey(const std::string& deviceId, const std::string& apiKey) {
    auto it = pImpl->apiKeys.find(deviceId);
    if (it == pImpl->apiKeys.end()) {
        return false;
    }
    return it->second == apiKey;
}

} // namespace NeuraDoc
