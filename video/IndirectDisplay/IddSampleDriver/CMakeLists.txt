cmake_minimum_required(VERSION 3.16)
project(VddSdk VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX- /wd4100)  # 忽略未使用參數警告
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(VDD_SDK_EXPORTS)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

# Find required packages
# D3D11 and DXGI are part of Windows SDK, no need to find_package

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(VDD_SDK_SOURCES
    vddsdk.cpp
)

set(VDD_SDK_HEADERS
    vddsdk.h
)

# Create the SDK library
add_library(vddsdk SHARED ${VDD_SDK_SOURCES} ${VDD_SDK_HEADERS})

# Set target properties
set_target_properties(vddsdk PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "vddsdk"
    PREFIX ""
)

# Link libraries
target_link_libraries(vddsdk
    PRIVATE
    setupapi
    dxgi
    d3d11
    advapi32
    user32
    kernel32
)

# Create the example executable
add_executable(vdd_example vdd_example.cpp)
target_link_libraries(vdd_example vddsdk)

# Create the CLI tool executable
add_executable(vddctl vddctl.cpp)
target_link_libraries(vddctl vddsdk)

# Create the performance test executable
add_executable(performance_test performance_test.cpp)
target_link_libraries(performance_test vddsdk_static)

# Installation
install(TARGETS vddsdk vdd_example vddctl performance_test
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${VDD_SDK_HEADERS}
    DESTINATION include
)

# Create a static library version as well
add_library(vddsdk_static STATIC ${VDD_SDK_SOURCES} ${VDD_SDK_HEADERS})
set_target_properties(vddsdk_static PROPERTIES
    OUTPUT_NAME "vddsdk"
    PREFIX ""
)

target_link_libraries(vddsdk_static
    PRIVATE
    setupapi
    dxgi
    d3d11
    advapi32
    user32
    kernel32
)

# Package configuration (simplified)
# 簡化版本，移除複雜的包配置

# Documentation
option(BUILD_DOCS "Build documentation" OFF)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(docs
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
    endif()
endif()

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    # 添加簡化測試
    add_executable(simple_test tests/simple_test.cpp)
    target_link_libraries(simple_test
        PRIVATE
        vddsdk_static
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 添加 Activate 測試
    add_executable(activate_test tests/test_activate.cpp)
    target_link_libraries(activate_test
        PRIVATE
        vddsdk_static
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 添加 Deactivate 測試
    add_executable(deactivate_test tests/test_deactivate_simple.cpp)
    target_link_libraries(deactivate_test
        PRIVATE
        vddsdk_static
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 添加 SetMode 測試
    add_executable(setmode_test tests/test_setmode.cpp)
    target_link_libraries(setmode_test
        PRIVATE
        vddsdk_static
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 添加 SetLocation 測試
    add_executable(setlocation_test tests/test_setlocation.cpp)
    target_link_libraries(setlocation_test
        PRIVATE
        vddsdk_static
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 添加 SetPrimary 測試
    add_executable(setprimary_test tests/test_setprimary.cpp)
    target_link_libraries(setprimary_test
        PRIVATE
        vddsdk_static
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 添加驅動整合測試
    add_executable(driver_integration_test tests/test_driver_integration.cpp)
    target_link_libraries(driver_integration_test
        PRIVATE
        vddsdk_static
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 添加測試到 CTest
    add_test(NAME SimpleTest COMMAND simple_test)
    set_tests_properties(SimpleTest PROPERTIES TIMEOUT 60)
    
    # 嘗試添加完整測試（如果 Google Test 可用）
    option(BUILD_FULL_TESTS "Build full tests with Google Test" OFF)
    if(BUILD_FULL_TESTS)
        add_subdirectory(tests)
    endif()
endif()

# Print configuration summary
message(STATUS "VDD SDK Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build docs: ${BUILD_DOCS}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
