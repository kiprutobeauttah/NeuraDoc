#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace NeuraDoc {

enum class UserRole {
    DOCTOR,
    NURSE,
    ADMIN,
    TECHNICIAN
};

struct Credentials {
    std::string userId;
    std::string passwordHash;
    UserRole role;
    std::vector<std::string> permissions;
};

struct AuthToken {
    std::string token;
    std::string userId;
    UserRole role;
    std::chrono::system_clock::time_point expiresAt;
    bool isValid() const;
};

class SecurityManager {
public:
    SecurityManager();
    ~SecurityManager();
    
    // User authentication
    AuthToken authenticate(const std::string& userId, const std::string& password);
    bool validateToken(const AuthToken& token);
    void revokeToken(const std::string& token);
    
    // User management
    void registerUser(const std::string& userId, const std::string& password, UserRole role);
    void removeUser(const std::string& userId);
    bool changePassword(const std::string& userId, const std::string& oldPassword, 
                       const std::string& newPassword);
    
    // Authorization
    bool hasPermission(const AuthToken& token, const std::string& permission);
    bool canAccessPatient(const AuthToken& token, const std::string& patientId);
    
    // Encryption
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, const std::string& key);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data, const std::string& key);
    
    // Key management
    std::string generateSessionKey();
    std::string hashPassword(const std::string& password);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace NeuraDoc
