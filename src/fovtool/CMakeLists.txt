cmake_minimum_required(VERSION 3.23...4.0)

project(
    fovtool
    LANGUAGES CXX
)

file(
    GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/*.hpp
)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /Zc:__cplusplus)
    target_sources(${PROJECT_NAME} PRIVATE "app.manifest")

endif()

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

find_package(CLI11 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        libtcod-fov::libtcod-fov
        CLI11::CLI11
        fmt::fmt
        Microsoft.GSL::GSL
        utf8::cpp
)
