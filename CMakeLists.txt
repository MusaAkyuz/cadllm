cmake_minimum_required(VERSION 3.21)

project(cadllm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(cadllm
    src/main.cpp
    src/version.cpp
)

target_link_libraries(cadllm PRIVATE Qt6::Widgets)
