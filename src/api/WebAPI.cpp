#include "WebAPI.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

namespace NeuraDoc {

class WebAPI::Impl {
public:
    int port;
    bool running = false;
    bool corsEnabled = true;
    bool websocketEnabled = true;
    std::shared_ptr<SecurityManager> securityMgr;
    std::shared_ptr<PostgreSQLDatabase> database;
    std::vector<std::string> websocketClients;
    
    Impl(int p, std::shared_ptr<SecurityManager> sm, std::shared_ptr<PostgreSQLDatabase> db) 
        : port(p), securityMgr(sm), database(db) {}
};

WebAPI::WebAPI(int port, std::shared_ptr<SecurityManager> securityMgr,
               std::shared_ptr<PostgreSQLDatabase> database) 
    : pImpl(std::make_unique<Impl>(port, securityMgr, database)) {}

WebAPI::~WebAPI() {
    stop();
}

void WebAPI::start() {
    if (pImpl->running) return;
    
    pImpl->running = true;
    std::cout << "[WEB API] Started on port " << pImpl->port << std::endl;
    std::cout << "[WEB API] CORS: " << (pImpl->corsEnabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "[WEB API] WebSocket: " << (pImpl->websocketEnabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "[WEB API] Endpoints:" << std::endl;
    std::cout << "  POST /api/auth/login - Authenticate user" << std::endl;
    std::cout << "  GET  /api/patients - Get all patients" << std::endl;
    std::cout << "  GET  /api/patients/:id - Get patient details" << std::endl;
    std::cout << "  GET  /api/patients/:id/vitals - Get patient vitals history" << std::endl;
    std::cout << "  GET  /api/patients/:id/vitals/latest - Get latest vitals" << std::endl;
    std::cout << "  GET  /api/alerts - Get recent alerts" << std::endl;
    std::cout << "  WS   /ws - WebSocket for real-time updates" << std::endl;
}

void WebAPI::stop() {
    if (!pImpl->running) return;
    
    pImpl->running = false;
    std::cout << "[WEB API] Stopped" << std::endl;
}

bool WebAPI::isRunning() const {
    return pImpl->running;
}

void WebAPI::enableCORS(bool enable) {
    pImpl->corsEnabled = enable;
}

void WebAPI::enableWebSocket(bool enable) {
    pImpl->websocketEnabled = enable;
}

HTTPResponse WebAPI::handleRequest(const HTTPRequest& request) {
    HTTPResponse response;
    response.headers["Content-Type"] = "application/json";
    
    if (pImpl->corsEnabled) {
        response.headers["Access-Control-Allow-Origin"] = "*";
        response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    }
    
    if (request.method == "OPTIONS") {
        response.statusCode = 200;
        response.body = "{}";
        return response;
    }
    
    std::string authHeader = request.headers.count("Authorization") ? 
                            request.headers.at("Authorization") : "";
    
    if (request.path == "/api/auth/login") {
        return handleAuthenticate(request);
    }
    
    if (authHeader.empty()) {
        response.statusCode = 401;
        response.body = "{\"error\":\"Authentication required\"}";
        return response;
    }
    
    if (request.path == "/api/patients" && request.method == "GET") {
        return handleGetPatients(request);
    } else if (request.path.find("/api/patients/") == 0 && request.path.find("/vitals") != std::string::npos) {
        if (request.path.find("/latest") != std::string::npos) {
            return handleGetLatestVitals(request);
        }
        return handleGetVitals(request);
    } else if (request.path.find("/api/patients/") == 0) {
        return handleGetPatient(request);
    } else if (request.path == "/api/alerts") {
        return handleGetAlerts(request);
    }
    
    response.statusCode = 404;
    response.body = "{\"error\":\"Endpoint not found\"}";
    return response;
}

void WebAPI::broadcastVitals(const VitalSigns& vitals) {
    if (!pImpl->websocketEnabled) return;
    
    std::string json = toJSON(vitals);
    std::cout << "[WEB API] Broadcasting vitals to " << pImpl->websocketClients.size() 
              << " WebSocket clients" << std::endl;
}

void WebAPI::broadcastAlert(const std::string& patientId, const std::string& message) {
    if (!pImpl->websocketEnabled) return;
    
    std::stringstream ss;
    ss << "{\"type\":\"alert\",\"patientId\":\"" << patientId 
       << "\",\"message\":\"" << message << "\"}";
    
    std::cout << "[WEB API] Broadcasting alert: " << message << std::endl;
}

HTTPResponse WebAPI::handleGetPatients(const HTTPRequest& request) {
    HTTPResponse response;
    auto patients = pImpl->database->getAllPatients();
    response.statusCode = 200;
    response.body = toJSON(patients);
    return response;
}

HTTPResponse WebAPI::handleGetPatient(const HTTPRequest& request) {
    HTTPResponse response;
    
    size_t pos = request.path.find("/api/patients/");
    std::string patientId = request.path.substr(pos + 14);
    
    auto patient = pImpl->database->getPatient(patientId);
    if (patient.patientId.empty()) {
        response.statusCode = 404;
        response.body = "{\"error\":\"Patient not found\"}";
    } else {
        response.statusCode = 200;
        response.body = toJSON(patient);
    }
    
    return response;
}

HTTPResponse WebAPI::handleGetVitals(const HTTPRequest& request) {
    HTTPResponse response;
    
    size_t pos = request.path.find("/api/patients/");
    size_t endPos = request.path.find("/vitals");
    std::string patientId = request.path.substr(pos + 14, endPos - pos - 14);
    
    int hours = 24;
    if (request.params.count("hours")) {
        hours = std::stoi(request.params.at("hours"));
    }
    
    auto vitals = pImpl->database->getVitalsHistory(patientId, std::chrono::hours(hours));
    response.statusCode = 200;
    response.body = toJSON(vitals);
    
    return response;
}

HTTPResponse WebAPI::handleGetLatestVitals(const HTTPRequest& request) {
    HTTPResponse response;
    
    size_t pos = request.path.find("/api/patients/");
    size_t endPos = request.path.find("/vitals");
    std::string patientId = request.path.substr(pos + 14, endPos - pos - 14);
    
    auto vitals = pImpl->database->getLatestVitals(patientId);
    response.statusCode = 200;
    response.body = toJSON(vitals);
    
    return response;
}

HTTPResponse WebAPI::handleGetAlerts(const HTTPRequest& request) {
    HTTPResponse response;
    
    int hours = 24;
    if (request.params.count("hours")) {
        hours = std::stoi(request.params.at("hours"));
    }
    
    auto alerts = pImpl->database->getRecentAlerts(std::chrono::hours(hours));
    
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < alerts.size(); i++) {
        ss << "\"" << alerts[i] << "\"";
        if (i < alerts.size() - 1) ss << ",";
    }
    ss << "]";
    
    response.statusCode = 200;
    response.body = ss.str();
    
    return response;
}

HTTPResponse WebAPI::handleAuthenticate(const HTTPRequest& request) {
    HTTPResponse response;
    response.statusCode = 200;
    response.body = "{\"token\":\"sample_token\",\"userId\":\"user123\"}";
    return response;
}

std::string WebAPI::toJSON(const VitalSigns& vitals) {
    std::stringstream ss;
    ss << "{"
       << "\"patientId\":\"" << vitals.patientId << "\","
       << "\"heartRate\":" << vitals.heartRate << ","
       << "\"systolicBP\":" << vitals.systolicBP << ","
       << "\"diastolicBP\":" << vitals.diastolicBP << ","
       << "\"oxygenSaturation\":" << vitals.oxygenSaturation << ","
       << "\"temperature\":" << vitals.temperature << ","
       << "\"respiratoryRate\":" << vitals.respiratoryRate
       << "}";
    return ss.str();
}

std::string WebAPI::toJSON(const PatientInfo& patient) {
    std::stringstream ss;
    ss << "{"
       << "\"patientId\":\"" << patient.patientId << "\","
       << "\"name\":\"" << patient.name << "\","
       << "\"age\":" << patient.age << ","
       << "\"roomNumber\":\"" << patient.roomNumber << "\","
       << "\"assignedDoctor\":\"" << patient.assignedDoctor << "\""
       << "}";
    return ss.str();
}

std::string WebAPI::toJSON(const std::vector<VitalSigns>& vitalsList) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vitalsList.size(); i++) {
        ss << toJSON(vitalsList[i]);
        if (i < vitalsList.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

std::string WebAPI::toJSON(const std::vector<PatientInfo>& patients) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < patients.size(); i++) {
        ss << toJSON(patients[i]);
        if (i < patients.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

} // namespace NeuraDoc
