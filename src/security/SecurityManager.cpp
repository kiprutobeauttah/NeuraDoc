#include "SecurityManager.hpp"
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

namespace NeuraDoc {

bool AuthToken::isValid() const {
    return std::chrono::system_clock::now() < expiresAt;
}

class SecurityManager::Impl {
public:
    std::map<std::string, Credentials> users;
    std::map<std::string, AuthToken> activeTokens;
    std::map<std::string, std::vector<std::string>> patientAccess; // userId -> patientIds
    
    std::string generateToken() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::stringstream ss;
        for (int i = 0; i < 32; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        }
        return ss.str();
    }
};

SecurityManager::SecurityManager() : pImpl(std::make_unique<Impl>()) {
    // Create default admin user
    registerUser("admin", "admin123", UserRole::ADMIN);
}

SecurityManager::~SecurityManager() = default;

AuthToken SecurityManager::authenticate(const std::string& userId, const std::string& password) {
    auto it = pImpl->users.find(userId);
    if (it == pImpl->users.end()) {
        return AuthToken{"", "", UserRole::DOCTOR, std::chrono::system_clock::now()};
    }
    
    std::string passwordHash = hashPassword(password);
    if (passwordHash != it->second.passwordHash) {
        return AuthToken{"", "", UserRole::DOCTOR, std::chrono::system_clock::now()};
    }
    
    // Generate token valid for 8 hours
    AuthToken token;
    token.token = pImpl->generateToken();
    token.userId = userId;
    token.role = it->second.role;
    token.expiresAt = std::chrono::system_clock::now() + std::chrono::hours(8);
    
    pImpl->activeTokens[token.token] = token;
    return token;
}

bool SecurityManager::validateToken(const AuthToken& token) {
    auto it = pImpl->activeTokens.find(token.token);
    if (it == pImpl->activeTokens.end()) {
        return false;
    }
    return it->second.isValid();
}

void SecurityManager::revokeToken(const std::string& token) {
    pImpl->activeTokens.erase(token);
}

void SecurityManager::registerUser(const std::string& userId, const std::string& password, UserRole role) {
    Credentials creds;
    creds.userId = userId;
    creds.passwordHash = hashPassword(password);
    creds.role = role;
    
    // Set default permissions based on role
    switch (role) {
        case UserRole::ADMIN:
            creds.permissions = {"read", "write", "delete", "admin", "access_all_patients"};
            break;
        case UserRole::DOCTOR:
            creds.permissions = {"read", "write", "access_assigned_patients"};
            break;
        case UserRole::NURSE:
            creds.permissions = {"read", "access_assigned_patients"};
            break;
        case UserRole::TECHNICIAN:
            creds.permissions = {"read", "device_config"};
            break;
    }
    
    pImpl->users[userId] = creds;
}

void SecurityManager::removeUser(const std::string& userId) {
    pImpl->users.erase(userId);
    pImpl->patientAccess.erase(userId);
}

bool SecurityManager::changePassword(const std::string& userId, const std::string& oldPassword, 
                                    const std::string& newPassword) {
    auto it = pImpl->users.find(userId);
    if (it == pImpl->users.end()) {
        return false;
    }
    
    if (hashPassword(oldPassword) != it->second.passwordHash) {
        return false;
    }
    
    it->second.passwordHash = hashPassword(newPassword);
    return true;
}

bool SecurityManager::hasPermission(const AuthToken& token, const std::string& permission) {
    if (!validateToken(token)) {
        return false;
    }
    
    auto it = pImpl->users.find(token.userId);
    if (it == pImpl->users.end()) {
        return false;
    }
    
    const auto& perms = it->second.permissions;
    return std::find(perms.begin(), perms.end(), permission) != perms.end();
}

bool SecurityManager::canAccessPatient(const AuthToken& token, const std::string& patientId) {
    if (!validateToken(token)) {
        return false;
    }
    
    // Admins can access all patients
    if (hasPermission(token, "access_all_patients")) {
        return true;
    }
    
    // Check if user has access to this specific patient
    auto it = pImpl->patientAccess.find(token.userId);
    if (it != pImpl->patientAccess.end()) {
        const auto& patients = it->second;
        return std::find(patients.begin(), patients.end(), patientId) != patients.end();
    }
    
    return false;
}

std::vector<uint8_t> SecurityManager::encrypt(const std::vector<uint8_t>& data, const std::string& key) {
    // Simple XOR encryption for demonstration
    // In production: use AES-256-GCM or similar
    std::vector<uint8_t> encrypted = data;
    for (size_t i = 0; i < encrypted.size(); ++i) {
        encrypted[i] ^= key[i % key.size()];
    }
    return encrypted;
}

std::vector<uint8_t> SecurityManager::decrypt(const std::vector<uint8_t>& data, const std::string& key) {
    // XOR is symmetric
    return encrypt(data, key);
}

std::string SecurityManager::generateSessionKey() {
    return pImpl->generateToken();
}

std::string SecurityManager::hashPassword(const std::string& password) {
    // Simple hash for demonstration
    // In production: use bcrypt, scrypt, or Argon2
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

} // namespace NeuraDoc
