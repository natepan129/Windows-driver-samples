cmake_minimum_required(VERSION 3.16)
project(VDD_SDK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 設定輸出目錄
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目錄
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 編譯選項
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

# VDD SDK 庫
add_library(vddsdk SHARED
    vddsdk.cpp
    vddsdk.h
)

# 設定 DLL 導出
target_compile_definitions(vddsdk PRIVATE VDD_EXPORTS)

# 測試程式
add_executable(test_basic_functionality
    test_basic_functionality.cpp
)

add_executable(test_driver_ioctl
    test_driver_ioctl.cpp
)

add_executable(test_vdd
    test_vdd.cpp
)

# 連結庫
target_link_libraries(test_basic_functionality vddsdk)
target_link_libraries(test_driver_ioctl vddsdk)
target_link_libraries(test_vdd vddsdk)

# Windows 特定設定
if(WIN32)
    # 連結 Windows 庫
    target_link_libraries(vddsdk
        setupapi
        advapi32
        user32
        kernel32
    )
    
    # 設定子系統
    set_target_properties(test_basic_functionality PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    
    set_target_properties(test_driver_ioctl PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    
    set_target_properties(test_vdd PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 安裝規則
install(TARGETS vddsdk
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES vddsdk.h
    DESTINATION include
)

# 測試目標
enable_testing()
add_test(NAME basic_functionality_test COMMAND test_basic_functionality)
add_test(NAME driver_ioctl_test COMMAND test_driver_ioctl)
add_test(NAME vdd_test COMMAND test_vdd)

# 自定義目標：執行所有測試
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_basic_functionality test_driver_ioctl test_vdd
    COMMENT "執行所有測試"
)

# 自定義目標：清理
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/lib
    COMMENT "清理所有輸出檔案"
)