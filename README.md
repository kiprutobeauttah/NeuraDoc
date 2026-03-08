# NeuraDoc - Real-Time Patient Monitoring System

## Important Notice

**This repository contains documentation only. The source code is proprietary and not publicly available.**

For inquiries about licensing, partnerships, or access to the source code, please contact: kiprutobeauttah@gmail.com

---

| Property | Value |
|----------|-------|
| Version | 1.0.0 |
| Author | Beauttah Kipruto |
| License | Apache License 2.0 |
| Language | C++17 |
| Build System | CMake 3.15+ |

## Important Notice

**This repository contains documentation only. The source code is proprietary and not publicly available.**

For inquiries about licensing, partnerships, or access to the source code, please contact: kiprutobeauttah@gmail.com

---

## Project Information

NeuraDoc is an enterprise-grade, real-time patient monitoring system designed for hospital environments. The system captures vital signs from bedside medical equipment, performs real-time analysis, stores data in a PostgreSQL database, and securely routes information to healthcare providers' devices through encrypted channels.

## System Architecture

### High-Level Architecture

The system follows a layered architecture pattern with clear separation of concerns:

| Layer | Components | Responsibility |
|-------|-----------|----------------|
| Presentation | Web API, WebSocket Server | Client communication and data presentation |
| Application | Device API, Alert Engine, Real-Time Analyzer | Business logic and data processing |
| Security | Security Manager, Secure Router, Audit Logger | Authentication, authorization, encryption |
| Data | PostgreSQL Database, Patient Database | Data persistence and retrieval |
| Device | Device Interface, Device API | Medical device integration |

### Component Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Client Layer                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ Web Browser  │  │ Mobile App   │  │ Desktop App  │          │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘          │
└─────────┼──────────────────┼──────────────────┼─────────────────┘
          │                  │                  │
          └──────────────────┼──────────────────┘
                             │
┌────────────────────────────┼─────────────────────────────────────┐
│                    API Gateway Layer                              │
│         ┌──────────────────┴──────────────────┐                  │
│         │                                     │                  │
│  ┌──────▼──────┐                    ┌────────▼────────┐         │
│  │  Web API    │                    │ WebSocket Server│         │
│  │  (Port 8080)│                    │   (Port 8080)   │         │
│  └──────┬──────┘                    └────────┬────────┘         │
└─────────┼────────────────────────────────────┼──────────────────┘
          │                                     │
┌─────────┼─────────────────────────────────────┼──────────────────┐
│         │        Security Layer               │                  │
│  ┌──────▼──────────────────────────────────────▼──────┐         │
│  │           Security Manager                         │         │
│  │  - Authentication    - Authorization               │         │
│  │  - Encryption        - Token Management            │         │
│  └──────┬─────────────────────────────────────────────┘         │
│         │                                                        │
│  ┌──────▼──────┐              ┌──────────────┐                 │
│  │Secure Router│              │ Audit Logger │                 │
│  └──────┬──────┘              └──────────────┘                 │
└─────────┼─────────────────────────────────────────────────────┘
          │
┌─────────┼─────────────────────────────────────────────────────┐
│         │        Application Layer                             │
│  ┌──────▼──────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │ Real-Time       │  │ Alert Engine │  │ Patient         │  │
│  │ Analyzer        │  │              │  │ Database        │  │
│  └──────┬──────────┘  └──────┬───────┘  └────────┬────────┘  │
│         │                    │                    │            │
│         └────────────────────┼────────────────────┘            │
│                              │                                 │
└──────────────────────────────┼─────────────────────────────────┘
                               │
┌──────────────────────────────┼─────────────────────────────────┐
│                    Data Layer│                                  │
│                    ┌─────────▼──────────┐                      │
│                    │ PostgreSQL Database│                      │
│                    │  - Patients Table  │                      │
│                    │  - Vitals Table    │                      │
│                    │  - Alerts Table    │                      │
│                    └─────────┬──────────┘                      │
└──────────────────────────────┼─────────────────────────────────┘
                               │
┌──────────────────────────────┼─────────────────────────────────┐
│                    Device Layer                                 │
│  ┌──────────────┐  ┌─────────▼──────┐  ┌──────────────┐      │
│  │ Medical      │──│  Device API    │  │ Device       │      │
│  │ Device 1     │  │  (Port 9000)   │  │ Interface    │      │
│  └──────────────┘  └────────────────┘  └──────────────┘      │
│  ┌──────────────┐                                             │
│  │ Medical      │                                             │
│  │ Device 2     │                                             │
│  └──────────────┘                                             │
└─────────────────────────────────────────────────────────────┘
```

### Data Flow Architecture

| Flow Stage | Component | Action | Output |
|------------|-----------|--------|--------|
| 1. Capture | Medical Device | Measures patient vitals | Raw vital signs data |
| 2. Ingestion | Device API | Validates and authenticates | Authenticated data packet |
| 3. Analysis | Real-Time Analyzer | Analyzes for anomalies | Analysis result with risk score |
| 4. Alert | Alert Engine | Checks thresholds | Alert notifications (if critical) |
| 5. Storage | PostgreSQL Database | Persists data | Database record |
| 6. Routing | Secure Router | Encrypts and routes | Encrypted data packets |
| 7. Delivery | Web API / WebSocket | Delivers to clients | JSON response / WebSocket message |
| 8. Audit | Audit Logger | Logs all activities | Audit trail record |

## System Components

### Core Components

| Component | Type | Port | Purpose | Dependencies |
|-----------|------|------|---------|--------------|
| Device API | REST API | 9000 | Medical device data ingestion | SecurityManager, AuditLogger |
| Web API | REST API | 8080 | Client data access | SecurityManager, PostgreSQLDatabase |
| WebSocket Server | WebSocket | 8080 | Real-time data streaming | SecurityManager |
| Secure Network Server | TCP/TLS | 8443 | Encrypted device communication | SecurityManager, AuditLogger |
| Real-Time Analyzer | Service | N/A | Vital signs analysis | None |
| Alert Engine | Service | N/A | Critical condition detection | None |
| Secure Router | Service | N/A | Encrypted data routing | SecurityManager |
| PostgreSQL Database | Database | 5432 | Data persistence | libpq |
| Security Manager | Service | N/A | Authentication and encryption | OpenSSL |
| Audit Logger | Service | N/A | Compliance logging | None |

### Component Interactions

| Source Component | Target Component | Interaction Type | Data Format | Security |
|------------------|------------------|------------------|-------------|----------|
| Medical Device | Device API | HTTP POST | JSON | API Key |
| Device API | Real-Time Analyzer | Function Call | VitalSigns struct | N/A |
| Real-Time Analyzer | Alert Engine | Function Call | VitalSigns struct | N/A |
| Alert Engine | PostgreSQL Database | SQL INSERT | SQL | Encrypted connection |
| Web API | PostgreSQL Database | SQL SELECT | SQL | Encrypted connection |
| Secure Router | Doctor Devices | Encrypted TCP | Binary | AES-256 |
| All Components | Audit Logger | Function Call | AuditEvent struct | N/A |
| All Components | Security Manager | Function Call | Various | N/A |

## Data Models

### Database Schema

#### Patients Table

| Column | Type | Constraints | Description |
|--------|------|-------------|-------------|
| patient_id | VARCHAR(50) | PRIMARY KEY | Unique patient identifier |
| name | VARCHAR(255) | NOT NULL | Patient full name |
| age | INTEGER | | Patient age in years |
| room_number | VARCHAR(50) | | Hospital room assignment |
| assigned_doctor | VARCHAR(255) | | Assigned physician name |

#### Vitals Table

| Column | Type | Constraints | Description |
|--------|------|-------------|-------------|
| id | SERIAL | PRIMARY KEY | Auto-incrementing record ID |
| patient_id | VARCHAR(50) | FOREIGN KEY | References patients(patient_id) |
| timestamp | TIMESTAMP | DEFAULT NOW() | Measurement timestamp |
| heart_rate | INTEGER | | Heart rate in bpm |
| systolic_bp | INTEGER | | Systolic blood pressure in mmHg |
| diastolic_bp | INTEGER | | Diastolic blood pressure in mmHg |
| oxygen_saturation | REAL | | Oxygen saturation percentage |
| temperature | REAL | | Body temperature in Celsius |
| respiratory_rate | INTEGER | | Respiratory rate in breaths/min |

#### Alerts Table

| Column | Type | Constraints | Description |
|--------|------|-------------|-------------|
| id | SERIAL | PRIMARY KEY | Auto-incrementing alert ID |
| patient_id | VARCHAR(50) | FOREIGN KEY | References patients(patient_id) |
| alert_type | VARCHAR(100) | | Type of alert |
| message | TEXT | | Alert message content |
| severity | VARCHAR(50) | | Alert severity level |
| timestamp | TIMESTAMP | DEFAULT NOW() | Alert generation time |

### Data Structures

#### VitalSigns Structure

```cpp
struct VitalSigns {
    std::string patientId;
    std::chrono::system_clock::time_point timestamp;
    int heartRate;
    int systolicBP;
    int diastolicBP;
    float oxygenSaturation;
    float temperature;
    int respiratoryRate;
};
```

#### AnalysisResult Structure

```cpp
struct AnalysisResult {
    std::string patientId;
    std::chrono::system_clock::time_point timestamp;
    bool anomalyDetected;
    std::string anomalyType;
    float riskScore;
    std::vector<std::string> trends;
    std::vector<std::string> predictions;
    std::vector<std::string> recommendations;
};
```

## Security Architecture

### Authentication Flow

| Step | Actor | Action | Validation | Result |
|------|-------|--------|------------|--------|
| 1 | User | Submits credentials | Password hash comparison | AuthToken generated |
| 2 | System | Validates token | Token expiration check | Access granted/denied |
| 3 | System | Checks permissions | Role-based authorization | Operation allowed/denied |
| 4 | System | Logs activity | Audit trail creation | Compliance record |

### Authorization Matrix

| Role | Read Patients | Write Patients | Read Vitals | Access All Patients | Device Config | Admin Functions |
|------|---------------|----------------|-------------|---------------------|---------------|-----------------|
| Admin | Yes | Yes | Yes | Yes | Yes | Yes |
| Doctor | Yes | Yes | Yes | Assigned only | No | No |
| Nurse | Yes | No | Yes | Assigned only | No | No |
| Technician | No | No | No | No | Yes | No |

### Encryption Specifications

| Data Type | Encryption Method | Key Length | Protocol |
|-----------|------------------|------------|----------|
| Data in Transit | TLS 1.3 | 256-bit | HTTPS/WSS |
| Data at Rest | AES-GCM | 256-bit | Database encryption |
| Session Keys | AES | 256-bit | Symmetric encryption |
| Passwords | SHA-256 (upgrade to bcrypt) | 256-bit | Hash with salt |
| API Keys | Random generation | 256-bit | Secure random |

## API Specifications

### Device API Endpoints

| Endpoint | Method | Authentication | Request Body | Response | Purpose |
|----------|--------|----------------|--------------|----------|---------|
| /api/device/register | POST | API Key | DeviceRegistration | API Key | Register new device |
| /api/device/vitals | POST | API Key | VitalSigns | Success status | Submit single vitals |
| /api/device/vitals/batch | POST | API Key | VitalSigns[] | Success status | Submit batch vitals |
| /api/device/heartbeat | POST | API Key | DeviceStatus | Success status | Device health check |
| /api/device/status | GET | API Key | None | DeviceStatus | Get device status |

### Web API Endpoints

| Endpoint | Method | Authentication | Query Parameters | Response | Purpose |
|----------|--------|----------------|------------------|----------|---------|
| /api/auth/login | POST | None | None | AuthToken | User authentication |
| /api/patients | GET | Bearer Token | None | PatientInfo[] | List all patients |
| /api/patients/:id | GET | Bearer Token | None | PatientInfo | Get patient details |
| /api/patients/:id/vitals | GET | Bearer Token | hours (optional) | VitalSigns[] | Get vitals history |
| /api/patients/:id/vitals/latest | GET | Bearer Token | None | VitalSigns | Get latest vitals |
| /api/alerts | GET | Bearer Token | hours (optional) | Alert[] | Get recent alerts |

## Deployment Architecture

### System Requirements

| Component | Minimum | Recommended | Purpose |
|-----------|---------|-------------|---------|
| CPU | 4 cores | 8 cores | Concurrent processing |
| RAM | 8 GB | 16 GB | In-memory operations |
| Storage | 100 GB SSD | 500 GB SSD | Database and logs |
| Network | 100 Mbps | 1 Gbps | Real-time data transfer |
| OS | Ubuntu 20.04 | Ubuntu 22.04 | Linux environment |

### Software Dependencies

| Dependency | Version | Purpose | Installation |
|------------|---------|---------|--------------|
| CMake | 3.15+ | Build system | apt-get install cmake |
| GCC/Clang | 7+/5+ | C++ compiler | apt-get install build-essential |
| OpenSSL | 1.1.1+ | Encryption | apt-get install libssl-dev |
| PostgreSQL | 12+ | Database | apt-get install postgresql |
| libpq | 12+ | PostgreSQL client | apt-get install libpq-dev |

### Network Configuration

| Service | Port | Protocol | Access | Firewall Rule |
|---------|------|----------|--------|---------------|
| Device API | 9000 | HTTP/HTTPS | Internal network | Allow from device subnet |
| Web API | 8080 | HTTP/HTTPS | Public/VPN | Allow from authorized IPs |
| WebSocket | 8080 | WSS | Public/VPN | Allow from authorized IPs |
| Secure Server | 8443 | TLS | Internal network | Allow from doctor devices |
| PostgreSQL | 5432 | TCP | Localhost only | Deny external access |

## Build and Installation

### Prerequisites Installation

Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev postgresql-server-dev-all
```

macOS:
```bash
brew install cmake openssl postgresql
```

### Database Setup

```sql
CREATE DATABASE neuradoc;
CREATE USER neuradoc_user WITH PASSWORD 'neuradoc_pass';
GRANT ALL PRIVILEGES ON DATABASE neuradoc TO neuradoc_user;
\c neuradoc
```

### Build Process

```bash
git clone https://github.com/kiprutobeauttah/NeuraDoc.git
cd NeuraDoc
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Configuration

Edit database configuration in `src/production_main.cpp`:

```cpp
DatabaseConfig dbConfig;
dbConfig.host = "localhost";
dbConfig.port = 5432;
dbConfig.database = "neuradoc";
dbConfig.username = "neuradoc_user";
dbConfig.password = "neuradoc_pass";
dbConfig.maxConnections = 20;
```

### Execution

```bash
./neuradoc_production
```

## Operational Workflows

### Patient Monitoring Workflow

| Step | Actor | System Component | Action | Data Flow |
|------|-------|------------------|--------|-----------|
| 1 | Medical Device | Device Interface | Captures vitals | Device → Device API |
| 2 | Device API | Authentication | Validates API key | API Key validation |
| 3 | Device API | Real-Time Analyzer | Analyzes data | VitalSigns → Analyzer |
| 4 | Real-Time Analyzer | Alert Engine | Checks thresholds | AnalysisResult → Alert Engine |
| 5 | Alert Engine | PostgreSQL | Stores alert (if critical) | Alert → Database |
| 6 | PostgreSQL | Database | Persists vitals | VitalSigns → Database |
| 7 | Secure Router | Encryption | Encrypts data | VitalSigns → Encrypted packet |
| 8 | Secure Router | Doctor Devices | Routes data | Encrypted packet → Devices |
| 9 | WebSocket Server | Web Clients | Broadcasts update | VitalSigns → Connected clients |
| 10 | Audit Logger | Log Storage | Records activity | AuditEvent → Log file |

### Alert Generation Workflow

| Condition | Threshold | Alert Level | Action | Notification Target |
|-----------|-----------|-------------|--------|---------------------|
| Heart Rate Low | < 40 bpm | CRITICAL | Immediate notification | All assigned doctors |
| Heart Rate High | > 140 bpm | CRITICAL | Immediate notification | All assigned doctors |
| Blood Pressure High | > 180/120 mmHg | EMERGENCY | Immediate notification + Alarm | All assigned doctors + Nursing station |
| Oxygen Saturation Low | < 90% | EMERGENCY | Immediate notification + Alarm | All assigned doctors + Nursing station |
| Temperature Low | < 35°C | CRITICAL | Immediate notification | Assigned doctor |
| Temperature High | > 40°C | CRITICAL | Immediate notification | Assigned doctor |
| Respiratory Rate Low | < 8 breaths/min | CRITICAL | Immediate notification | All assigned doctors |
| Respiratory Rate High | > 30 breaths/min | WARNING | Notification | Assigned doctor |

## Performance Specifications

### System Performance Metrics

| Metric | Target | Measurement Method | Acceptable Range |
|--------|--------|-------------------|------------------|
| Data Ingestion Rate | 1000 vitals/second | Load testing | 800-1200 vitals/second |
| API Response Time | < 100ms | Average response time | 50-150ms |
| Database Query Time | < 50ms | Query execution time | 20-80ms |
| Alert Generation Time | < 1 second | Time from vitals to alert | 0.5-2 seconds |
| WebSocket Latency | < 50ms | Message delivery time | 20-100ms |
| System Uptime | 99.9% | Monitoring tools | > 99.5% |

### Scalability Specifications

| Resource | Current Capacity | Maximum Capacity | Scaling Method |
|----------|------------------|------------------|----------------|
| Concurrent Patients | 100 | 1000 | Horizontal scaling |
| Concurrent Devices | 100 | 1000 | Load balancing |
| API Requests/min | 10,000 | 100,000 | Rate limiting + caching |
| Database Connections | 20 | 200 | Connection pooling |
| WebSocket Connections | 500 | 5000 | WebSocket clustering |

## Monitoring and Maintenance

### Health Check Endpoints

| Endpoint | Method | Response | Check Frequency | Alert Threshold |
|----------|--------|----------|-----------------|-----------------|
| /health | GET | System status | 30 seconds | 3 consecutive failures |
| /health/database | GET | Database connectivity | 60 seconds | 2 consecutive failures |
| /health/devices | GET | Device connectivity | 60 seconds | 5 minutes offline |
| /metrics | GET | System metrics | 60 seconds | N/A |

### Log Management

| Log Type | Location | Rotation | Retention | Format |
|----------|----------|----------|-----------|--------|
| Application | /var/log/neuradoc/app.log | Daily | 30 days | JSON |
| Audit | /var/log/neuradoc/audit.log | Daily | 7 years | CSV |
| Error | /var/log/neuradoc/error.log | Daily | 90 days | JSON |
| Access | /var/log/neuradoc/access.log | Daily | 30 days | Common Log Format |
| Security | /var/log/neuradoc/security.log | Daily | 1 year | JSON |

## Compliance and Standards

### Regulatory Compliance

| Standard | Requirement | Implementation | Verification |
|----------|-------------|----------------|--------------|
| HIPAA | Patient data encryption | AES-256 encryption | Security audit |
| HIPAA | Access control | Role-based authorization | Access logs review |
| HIPAA | Audit trails | Comprehensive logging | Audit log analysis |
| HIPAA | Data integrity | Database constraints | Data validation tests |
| ISO 27001 | Information security | Security policies | Annual assessment |
| IEC 62304 | Medical device software | Development process | Documentation review |

### Data Retention Policy

| Data Type | Retention Period | Storage Location | Backup Frequency | Archive Method |
|-----------|------------------|------------------|------------------|----------------|
| Patient Records | 7 years | PostgreSQL | Daily | Encrypted backup |
| Vital Signs | 7 years | PostgreSQL | Daily | Encrypted backup |
| Audit Logs | 7 years | File system | Daily | Compressed archive |
| Alert History | 7 years | PostgreSQL | Daily | Encrypted backup |
| System Logs | 90 days | File system | Weekly | Compressed archive |

## Disaster Recovery

### Backup Strategy

| Component | Backup Type | Frequency | Retention | Recovery Time Objective |
|-----------|-------------|-----------|-----------|------------------------|
| PostgreSQL Database | Full backup | Daily | 30 days | 4 hours |
| PostgreSQL Database | Incremental | Hourly | 7 days | 1 hour |
| Configuration Files | Full backup | Daily | 30 days | 30 minutes |
| Audit Logs | Full backup | Daily | 7 years | 24 hours |
| Application Binaries | Full backup | Per release | Indefinite | 1 hour |

### Recovery Procedures

| Failure Scenario | Detection Method | Recovery Procedure | Estimated Recovery Time |
|------------------|------------------|-------------------|------------------------|
| Database Failure | Health check failure | Restore from latest backup | 4 hours |
| Application Crash | Process monitoring | Automatic restart | 2 minutes |
| Network Failure | Connection timeout | Failover to backup network | 5 minutes |
| Device Disconnection | Heartbeat timeout | Alert + manual intervention | 10 minutes |
| Data Corruption | Integrity check failure | Restore from backup | 4 hours |

## Testing Strategy

### Test Coverage

| Test Type | Coverage Target | Tools | Execution Frequency |
|-----------|----------------|-------|---------------------|
| Unit Tests | 80% | Google Test | Every commit |
| Integration Tests | 70% | Custom framework | Daily |
| Security Tests | 100% critical paths | OWASP ZAP | Weekly |
| Performance Tests | Key endpoints | Apache JMeter | Weekly |
| Load Tests | System capacity | Locust | Monthly |
| Penetration Tests | Full system | External audit | Quarterly |

## Documentation

### Available Documentation

| Document | Location | Audience | Update Frequency |
|----------|----------|----------|------------------|
| System Architecture | README.md | Developers, Architects | Per release |
| API Documentation | API_DOCUMENTATION.md | Developers, Integrators | Per API change |
| Security Documentation | SECURITY.md | Security team, Auditors | Quarterly |
| User Manual | docs/USER_MANUAL.md | End users | Per release |
| Deployment Guide | docs/DEPLOYMENT.md | DevOps team | Per release |
| Troubleshooting Guide | docs/TROUBLESHOOTING.md | Support team | As needed |

## Support and Maintenance

### Issue Tracking

| Issue Type | Priority Levels | Response Time | Resolution Time |
|------------|----------------|---------------|-----------------|
| Critical Bug | P0 | 1 hour | 4 hours |
| Major Bug | P1 | 4 hours | 24 hours |
| Minor Bug | P2 | 24 hours | 1 week |
| Feature Request | P3 | 1 week | Next release |
| Documentation | P4 | 1 week | 2 weeks |

## Version History

| Version | Release Date | Major Changes | Breaking Changes |
|---------|--------------|---------------|------------------|
| 1.0.0 | 2026-03-08 | Initial release | N/A |

## License

Apache License 2.0

Copyright (c) 2026 Beauttah Kipruto

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## Contact Information

| Role | Contact |
|------|---------|
| Project Lead | Beauttah Kipruto |
| Technical Support | kiprutobeauttah@gmail.com |
| Security Issues | kiprutobeauttah@gmail.com |
| General Inquiries | kiprutobeauttah@gmail.com |
