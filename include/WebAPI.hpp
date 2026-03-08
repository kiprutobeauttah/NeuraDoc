#pragma once
#include "VitalSigns.hpp"
#include "SecurityManager.hpp"
#include "PostgreSQLDatabase.hpp"
#include <string>
#include <memory>
#include <functional>

namespace NeuraDoc {

struct HTTPRequest {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> params;
};

struct HTTPResponse {
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;
};

class WebAPI {
public:
    WebAPI(int port, std::shared_ptr<SecurityManager> securityMgr,
           std::shared_ptr<PostgreSQLDatabase> database);
    ~WebAPI();
    
    void start();
    void stop();
    bool isRunning() const;
    
    void enableCORS(bool enable);
    void enableWebSocket(bool enable);
    
    HTTPResponse handleRequest(const HTTPRequest& request);
    
    void broadcastVitals(const VitalSigns& vitals);
    void broadcastAlert(const std::string& patientId, const std::string& message);
    
private:
    HTTPResponse handleGetPatients(const HTTPRequest& request);
    HTTPResponse handleGetPatient(const HTTPRequest& request);
    HTTPResponse handleGetVitals(const HTTPRequest& request);
    HTTPResponse handleGetLatestVitals(const HTTPRequest& request);
    HTTPResponse handleGetAlerts(const HTTPRequest& request);
    HTTPResponse handleAuthenticate(const HTTPRequest& request);
    
    std::string toJSON(const VitalSigns& vitals);
    std::string toJSON(const PatientInfo& patient);
    std::string toJSON(const std::vector<VitalSigns>& vitalsList);
    std::string toJSON(const std::vector<PatientInfo>& patients);
    
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
