cmake_minimum_required(VERSION 3.15)

# Test executable
add_executable(test_neuradoc
    test_vitals.cpp
    test_alerts.cpp
)

target_link_libraries(test_neuradoc PRIVATE)
target_include_directories(test_neuradoc PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_test(NAME VitalsTest COMMAND test_neuradoc)
