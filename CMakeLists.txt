cmake_minimum_required(VERSION 3.21)

project(cadllm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(cadllm
    src/main.cpp
    src/version.cpp
    src/mainwindow.cpp
)

target_link_libraries(cadllm PRIVATE Qt6::Widgets)

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET cadllm POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations "$<TARGET_FILE:cadllm>"
            COMMENT "Copying Qt runtime DLLs next to cadllm.exe (windeployqt)"
        )
    endif()
endif()
