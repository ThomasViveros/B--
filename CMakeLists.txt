# Generate Visual Studio project files (VS 2022 x64)
#cmake -S . -B build -G "Visual Studio 17 2022" -A x64

#Build (multi-config requires --config)
#cmake --build build --config Release

cmake_minimum_required(VERSION 3.10)
project(Brain VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(include)


# All .cpp files in the src folder
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Brain ${SOURCES})