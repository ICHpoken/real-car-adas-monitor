cmake_minimum_required(VERSION 3.20)
project(RealCarMonitor LANGUAGES CXX)

include_directories(${CMAKE_SOURCE_DIR}/src)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подсказка CMake, где искать OpenCV
set(OpenCV_DIR "C:/opencv/build")

# Ищем OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Ищем ONNX Runtime
set(ONNXRUNTIME_ROOT "C:/onnxruntime")
include_directories(${ONNXRUNTIME_ROOT}/include)
link_directories(${ONNXRUNTIME_ROOT}/lib)

# Собираем все .cpp файлы из папки src/
file(GLOB SOURCES "src/*.cpp")

# Главный исполняемый файл
add_executable(RealCarMonitor ${SOURCES})
target_link_libraries(RealCarMonitor ${OpenCV_LIBS} onnxruntime)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Тесты
add_executable(RunTests tests/test_obd_parser.cpp src/obd_parser.cpp)
target_link_libraries(RunTests gtest gtest_main ${OpenCV_LIBS})
enable_testing()