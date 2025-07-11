cmake_minimum_required(VERSION 3.23...4.0)
project(PackageTest CXX)

set(_IMPORT_CHECK_TARGETS "")  # Suppress undefined variable in Catch2Targets.cmake.
find_package(Catch2 CONFIG REQUIRED)

if (APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../lib")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

file(GLOB SRC_FILES CONFIGURE_DEPENDS test_*.cpp)

add_executable(unittest unittest.cpp ${SRC_FILES})
target_link_libraries(unittest libtcod-fov::libtcod-fov Catch2::Catch2 Catch2::Catch2WithMain)
target_compile_features(unittest PUBLIC cxx_std_20)
target_compile_definitions(unittest PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

if(MSVC)
  target_compile_options(unittest PRIVATE /W4)
  target_compile_options(unittest PRIVATE /utf-8)
else()
  target_compile_options(unittest PRIVATE -Wall -Wextra)
endif()

# CTest is a testing tool that can be used to test your project.
# enable_testing()
# add_test(NAME example
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#          COMMAND example)
