cmake_minimum_required(VERSION 3.16)

# 設置測試框架
include(CTest)
enable_testing()

# 查找測試框架
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

# 設置編譯選項
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 測試源文件
set(TEST_SOURCES
    test_vddsdk_api.cpp
    test_vddsdk_core.cpp
    test_vddsdk_display.cpp
    test_vddsdk_session.cpp
    test_vddsdk_mock.cpp
    test_vddsdk_integration.cpp
    test_first_tdd.cpp
)

# 創建測試可執行文件
add_executable(vddsdk_tests ${TEST_SOURCES})

# 創建第一個 TDD 測試可執行文件
add_executable(first_tdd_test test_first_tdd.cpp)

# 鏈接庫
target_link_libraries(vddsdk_tests
    PRIVATE
    vddsdk
    GTest::GTest
    GTest::Main
    GMock::GMock
    setupapi
    dxgi
    d3d11
    advapi32
    user32
    kernel32
)

# 鏈接第一個 TDD 測試庫
target_link_libraries(first_tdd_test
    PRIVATE
    vddsdk
    GTest::GTest
    GTest::Main
    setupapi
    dxgi
    d3d11
    advapi32
    user32
    kernel32
)

# 設置測試屬性
set_target_properties(vddsdk_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 添加測試到 CTest
add_test(NAME VddSdkTests COMMAND vddsdk_tests)

# 設置測試超時
set_tests_properties(VddSdkTests PROPERTIES TIMEOUT 300)

# 添加測試標籤
set_tests_properties(VddSdkTests PROPERTIES LABELS "unit;integration")

# 創建測試數據目錄
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_data)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_logs)

# 設置測試環境變量
set_tests_properties(VddSdkTests PROPERTIES
    ENVIRONMENT "VDD_TEST_MODE=1;VDD_LOG_LEVEL=debug"
)
