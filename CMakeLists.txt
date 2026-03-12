# Cutting edge
cmake_minimum_required(VERSION 4.2.3)
project(ccompiler LANGUAGES CXX)

# Require c++23 features
#target_compile_features(ccompiler PRIVATE cxx_std_23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 3 REQUIRED)

# Core lib
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cc)
add_library(core STATIC ${SOURCES})
target_include_directories(core
  PUBLIC
    include
  PRIVATE
    third_party/cpp-base64
)

# Entrypoint
add_executable(ccompiler main.cc)
target_link_libraries(ccompiler PRIVATE core)

# Tests
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/*.cc)
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE core Catch2::Catch2WithMain)

# Catch2 integration
include(CTest)
include(Catch)
catch_discover_tests(tests)
