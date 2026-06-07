cmake_minimum_required(VERSION 3.20)
project(RealCarMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)

set(ONNXRUNTIME_ROOT "C:/onnxruntime")
set(ONNXRUNTIME_INCLUDE ${ONNXRUNTIME_ROOT}/include)
set(ONNXRUNTIME_LIB ${ONNXRUNTIME_ROOT}/lib/onnxruntime.lib)
if(NOT EXISTS ${ONNXRUNTIME_LIB})
    message(FATAL_ERROR "onnxruntime.lib not found at ${ONNXRUNTIME_LIB}")
endif()

set(SOURCES
    src/main.cpp
    src/obd_parser.cpp
    src/onnx_classifier.cpp
    src/dashboard.cpp
    src/dms_monitor.cpp
    src/dms_hud.cpp
)

add_executable(RealCarMonitor ${SOURCES})
target_include_directories(RealCarMonitor PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE}
    src
)
target_link_libraries(RealCarMonitor PRIVATE
    ${OpenCV_LIBS}
    ${ONNXRUNTIME_LIB}
)

# Google Test
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(RunTests tests/test_obd_parser.cpp src/obd_parser.cpp)
target_include_directories(RunTests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(RunTests PRIVATE GTest::gtest GTest::gtest_main ${OpenCV_LIBS})
include(GoogleTest)
gtest_discover_tests(RunTests)