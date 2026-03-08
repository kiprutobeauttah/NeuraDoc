#include "PostgreSQLDatabase.hpp"
#include <libpq-fe.h>
#include <iostream>
#include <sstream>

namespace NeuraDoc {

class PostgreSQLDatabase::Impl {
public:
    PGconn* conn = nullptr;
    DatabaseConfig config;
    
    Impl(const DatabaseConfig& cfg) : config(cfg) {}
    
    ~Impl() {
        if (conn) {
            PQfinish(conn);
        }
    }
};

PostgreSQLDatabase::PostgreSQLDatabase(const DatabaseConfig& config) 
    : pImpl(std::make_unique<Impl>(config)) {}

PostgreSQLDatabase::~PostgreSQLDatabase() {
    disconnect();
}

bool PostgreSQLDatabase::connect() {
    std::stringstream connStr;
    connStr << "host=" << pImpl->config.host
            << " port=" << pImpl->config.port
            << " dbname=" << pImpl->config.database
            << " user=" << pImpl->config.username
            << " password=" << pImpl->config.password;
    
    pImpl->conn = PQconnectdb(connStr.str().c_str());
    
    if (PQstatus(pImpl->conn) != CONNECTION_OK) {
        std::cerr << "PostgreSQL connection failed: " << PQerrorMessage(pImpl->conn) << std::endl;
        PQfinish(pImpl->conn);
        pImpl->conn = nullptr;
        return false;
    }
    
    std::cout << "Connected to PostgreSQL database" << std::endl;
    createTables();
    return true;
}

void PostgreSQLDatabase::disconnect() {
    if (pImpl->conn) {
        PQfinish(pImpl->conn);
        pImpl->conn = nullptr;
    }
}

bool PostgreSQLDatabase::isConnected() const {
    return pImpl->conn && PQstatus(pImpl->conn) == CONNECTION_OK;
}

void PostgreSQLDatabase::storeVitals(const VitalSigns& vitals) {
    if (!isConnected()) return;
    
    std::stringstream query;
    query << "INSERT INTO vitals (patient_id, timestamp, heart_rate, systolic_bp, "
          << "diastolic_bp, oxygen_saturation, temperature, respiratory_rate) VALUES ("
          << "'" << vitals.patientId << "', "
          << "NOW(), "
          << vitals.heartRate << ", "
          << vitals.systolicBP << ", "
          << vitals.diastolicBP << ", "
          << vitals.oxygenSaturation << ", "
          << vitals.temperature << ", "
          << vitals.respiratoryRate << ")";
    
    PGresult* res = PQexec(pImpl->conn, query.str().c_str());
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Insert failed: " << PQerrorMessage(pImpl->conn) << std::endl;
    }
    
    PQclear(res);
}

void PostgreSQLDatabase::storeVitalsBatch(const std::vector<VitalSigns>& vitalsList) {
    if (!isConnected() || vitalsList.empty()) return;
    
    PQexec(pImpl->conn, "BEGIN");
    
    for (const auto& vitals : vitalsList) {
        storeVitals(vitals);
    }
    
    PQexec(pImpl->conn, "COMMIT");
}

std::vector<VitalSigns> PostgreSQLDatabase::getVitalsHistory(const std::string& patientId, 
                                                              std::chrono::hours duration) const {
    std::vector<VitalSigns> result;
    if (!isConnected()) return result;
    
    std::stringstream query;
    query << "SELECT * FROM vitals WHERE patient_id = '" << patientId << "' "
          << "AND timestamp >= NOW() - INTERVAL '" << duration.count() << " hours' "
          << "ORDER BY timestamp DESC";
    
    PGresult* res = PQexec(pImpl->conn, query.str().c_str());
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            VitalSigns vitals;
            vitals.patientId = PQgetvalue(res, i, 1);
            vitals.heartRate = std::stoi(PQgetvalue(res, i, 3));
            vitals.systolicBP = std::stoi(PQgetvalue(res, i, 4));
            vitals.diastolicBP = std::stoi(PQgetvalue(res, i, 5));
            vitals.oxygenSaturation = std::stof(PQgetvalue(res, i, 6));
            vitals.temperature = std::stof(PQgetvalue(res, i, 7));
            vitals.respiratoryRate = std::stoi(PQgetvalue(res, i, 8));
            result.push_back(vitals);
        }
    }
    
    PQclear(res);
    return result;
}

VitalSigns PostgreSQLDatabase::getLatestVitals(const std::string& patientId) const {
    VitalSigns vitals;
    if (!isConnected()) return vitals;
    
    std::stringstream query;
    query << "SELECT * FROM vitals WHERE patient_id = '" << patientId << "' "
          << "ORDER BY timestamp DESC LIMIT 1";
    
    PGresult* res = PQexec(pImpl->conn, query.str().c_str());
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        vitals.patientId = PQgetvalue(res, 0, 1);
        vitals.heartRate = std::stoi(PQgetvalue(res, 0, 3));
        vitals.systolicBP = std::stoi(PQgetvalue(res, 0, 4));
        vitals.diastolicBP = std::stoi(PQgetvalue(res, 0, 5));
        vitals.oxygenSaturation = std::stof(PQgetvalue(res, 0, 6));
        vitals.temperature = std::stof(PQgetvalue(res, 0, 7));
        vitals.respiratoryRate = std::stoi(PQgetvalue(res, 0, 8));
    }
    
    PQclear(res);
    return vitals;
}

void PostgreSQLDatabase::storePatient(const PatientInfo& patient) {
    if (!isConnected()) return;
    
    std::stringstream query;
    query << "INSERT INTO patients (patient_id, name, age, room_number, assigned_doctor) VALUES ("
          << "'" << patient.patientId << "', "
          << "'" << patient.name << "', "
          << patient.age << ", "
          << "'" << patient.roomNumber << "', "
          << "'" << patient.assignedDoctor << "') "
          << "ON CONFLICT (patient_id) DO UPDATE SET "
          << "name = EXCLUDED.name, age = EXCLUDED.age, "
          << "room_number = EXCLUDED.room_number, assigned_doctor = EXCLUDED.assigned_doctor";
    
    PGresult* res = PQexec(pImpl->conn, query.str().c_str());
    PQclear(res);
}

PatientInfo PostgreSQLDatabase::getPatient(const std::string& patientId) const {
    PatientInfo patient;
    if (!isConnected()) return patient;
    
    std::stringstream query;
    query << "SELECT * FROM patients WHERE patient_id = '" << patientId << "'";
    
    PGresult* res = PQexec(pImpl->conn, query.str().c_str());
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        patient.patientId = PQgetvalue(res, 0, 0);
        patient.name = PQgetvalue(res, 0, 1);
        patient.age = std::stoi(PQgetvalue(res, 0, 2));
        patient.roomNumber = PQgetvalue(res, 0, 3);
        patient.assignedDoctor = PQgetvalue(res, 0, 4);
    }
    
    PQclear(res);
    return patient;
}

std::vector<PatientInfo> PostgreSQLDatabase::getAllPatients() const {
    std::vector<PatientInfo> result;
    if (!isConnected()) return result;
    
    PGresult* res = PQexec(pImpl->conn, "SELECT * FROM patients");
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            PatientInfo patient;
            patient.patientId = PQgetvalue(res, i, 0);
            patient.name = PQgetvalue(res, i, 1);
            patient.age = std::stoi(PQgetvalue(res, i, 2));
            patient.roomNumber = PQgetvalue(res, i, 3);
            patient.assignedDoctor = PQgetvalue(res, i, 4);
            result.push_back(patient);
        }
    }
    
    PQclear(res);
    return result;
}

void PostgreSQLDatabase::storeAlert(const std::string& patientId, const std::string& alertType,
                                    const std::string& message, const std::string& severity) {
    if (!isConnected()) return;
    
    std::stringstream query;
    query << "INSERT INTO alerts (patient_id, alert_type, message, severity, timestamp) VALUES ("
          << "'" << patientId << "', "
          << "'" << alertType << "', "
          << "'" << message << "', "
          << "'" << severity << "', "
          << "NOW())";
    
    PGresult* res = PQexec(pImpl->conn, query.str().c_str());
    PQclear(res);
}

std::vector<std::string> PostgreSQLDatabase::getRecentAlerts(std::chrono::hours duration) const {
    std::vector<std::string> result;
    if (!isConnected()) return result;
    
    std::stringstream query;
    query << "SELECT patient_id, alert_type, message, severity FROM alerts "
          << "WHERE timestamp >= NOW() - INTERVAL '" << duration.count() << " hours' "
          << "ORDER BY timestamp DESC";
    
    PGresult* res = PQexec(pImpl->conn, query.str().c_str());
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            std::stringstream alert;
            alert << PQgetvalue(res, i, 0) << ": " << PQgetvalue(res, i, 2);
            result.push_back(alert.str());
        }
    }
    
    PQclear(res);
    return result;
}

void PostgreSQLDatabase::createTables() {
    if (!isConnected()) return;
    
    const char* createPatients = 
        "CREATE TABLE IF NOT EXISTS patients ("
        "patient_id VARCHAR(50) PRIMARY KEY, "
        "name VARCHAR(255) NOT NULL, "
        "age INTEGER, "
        "room_number VARCHAR(50), "
        "assigned_doctor VARCHAR(255))";
    
    const char* createVitals = 
        "CREATE TABLE IF NOT EXISTS vitals ("
        "id SERIAL PRIMARY KEY, "
        "patient_id VARCHAR(50) REFERENCES patients(patient_id), "
        "timestamp TIMESTAMP DEFAULT NOW(), "
        "heart_rate INTEGER, "
        "systolic_bp INTEGER, "
        "diastolic_bp INTEGER, "
        "oxygen_saturation REAL, "
        "temperature REAL, "
        "respiratory_rate INTEGER)";
    
    const char* createAlerts = 
        "CREATE TABLE IF NOT EXISTS alerts ("
        "id SERIAL PRIMARY KEY, "
        "patient_id VARCHAR(50) REFERENCES patients(patient_id), "
        "alert_type VARCHAR(100), "
        "message TEXT, "
        "severity VARCHAR(50), "
        "timestamp TIMESTAMP DEFAULT NOW())";
    
    PQexec(pImpl->conn, createPatients);
    PQexec(pImpl->conn, createVitals);
    PQexec(pImpl->conn, createAlerts);
    
    std::cout << "Database tables created/verified" << std::endl;
}

void PostgreSQLDatabase::optimizeForRealTime() {
    if (!isConnected()) return;
    
    PQexec(pImpl->conn, "CREATE INDEX IF NOT EXISTS idx_vitals_patient_time ON vitals(patient_id, timestamp DESC)");
    PQexec(pImpl->conn, "CREATE INDEX IF NOT EXISTS idx_alerts_time ON alerts(timestamp DESC)");
    
    std::cout << "Database optimized for real-time queries" << std::endl;
}

} // namespace NeuraDoc
