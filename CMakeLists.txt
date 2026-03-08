cmake_minimum_required(VERSION 3.15)
project(NeuraDoc VERSION 1.0.0 LANGUAGES CXX)

set(PROJECT_AUTHOR "Beauttah Kipruto")
set(PROJECT_COPYRIGHT "Copyright (c) 2026 Beauttah Kipruto")
set(PROJECT_LICENSE "Apache-2.0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/device/*.cpp"
    "src/router/*.cpp"
    "src/alert/*.cpp"
    "src/storage/*.cpp"
    "src/network/*.cpp"
    "src/security/*.cpp"
    "src/database/*.cpp"
    "src/api/*.cpp"
    "src/analysis/*.cpp"
)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find PostgreSQL
find_package(PostgreSQL REQUIRED)

# Main executable (basic)
add_executable(neuradoc_server src/main.cpp ${SOURCES})
target_link_libraries(neuradoc_server OpenSSL::SSL OpenSSL::Crypto)

# Secure main executable
add_executable(neuradoc_secure src/secure_main.cpp ${SOURCES})
target_link_libraries(neuradoc_secure OpenSSL::SSL OpenSSL::Crypto)

# Production main with all features
add_executable(neuradoc_production src/production_main.cpp ${SOURCES})
target_link_libraries(neuradoc_production OpenSSL::SSL OpenSSL::Crypto PostgreSQL::PostgreSQL)

# Client applications
add_executable(neuradoc_desktop src/clients/desktop_client.cpp ${SOURCES})
target_link_libraries(neuradoc_desktop OpenSSL::SSL OpenSSL::Crypto)

add_executable(neuradoc_monitor src/clients/monitor_client.cpp ${SOURCES})
target_link_libraries(neuradoc_monitor OpenSSL::SSL OpenSSL::Crypto)

# Tests
enable_testing()
add_subdirectory(tests)
