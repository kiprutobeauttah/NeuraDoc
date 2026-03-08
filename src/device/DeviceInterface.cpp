#include "DeviceInterface.hpp"
#include <thread>
#include <atomic>
#include <random>

namespace NeuraDoc {

// Simulated device for testing
class SimulatedDevice : public DeviceInterface {
public:
    SimulatedDevice() : connected_(false), monitoring_(false) {}
    
    bool connect(const std::string& deviceId) override {
        deviceId_ = deviceId;
        connected_ = true;
        return true;
    }
    
    void disconnect() override {
        stopMonitoring();
        connected_ = false;
    }
    
    bool isConnected() const override {
        return connected_;
    }
    
    VitalSigns readVitals() override {
        VitalSigns vitals;
        vitals.patientId = deviceId_;
        vitals.timestamp = std::chrono::system_clock::now();
        
        // Simulate realistic vital signs
        std::random_device rd;
        std::mt19937 gen(rd());
        
        std::uniform_int_distribution<> hr(60, 100);
        std::uniform_int_distribution<> sys(110, 130);
        std::uniform_int_distribution<> dia(70, 85);
        std::uniform_real_distribution<> o2(95.0, 100.0);
        std::uniform_real_distribution<> temp(36.5, 37.5);
        std::uniform_int_distribution<> rr(12, 20);
        
        vitals.heartRate = hr(gen);
        vitals.systolicBP = sys(gen);
        vitals.diastolicBP = dia(gen);
        vitals.oxygenSaturation = o2(gen);
        vitals.temperature = temp(gen);
        vitals.respiratoryRate = rr(gen);
        
        return vitals;
    }
    
    void setCallback(VitalsCallback callback) override {
        callback_ = callback;
    }
    
    void startMonitoring() override {
        if (monitoring_) return;
        
        monitoring_ = true;
        monitorThread_ = std::thread([this]() {
            while (monitoring_) {
                if (callback_) {
                    callback_(readVitals());
                }
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        });
    }
    
    void stopMonitoring() override {
        monitoring_ = false;
        if (monitorThread_.joinable()) {
            monitorThread_.join();
        }
    }
    
private:
    std::string deviceId_;
    bool connected_;
    std::atomic<bool> monitoring_;
    VitalsCallback callback_;
    std::thread monitorThread_;
};

std::unique_ptr<DeviceInterface> createDeviceInterface(const std::string& deviceType) {
    // For now, return simulated device
    // In production, create specific device interfaces based on type
    return std::make_unique<SimulatedDevice>();
}

} // namespace NeuraDoc
