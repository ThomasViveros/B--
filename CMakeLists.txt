cmake_minimum_required(VERSION 3.10)
project(Brain VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(include)


# All .cpp files in the src folder
file(GLOB SOURCES src/*.cpp)

add_executable(Brain ${SOURCES})

