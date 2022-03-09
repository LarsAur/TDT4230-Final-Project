cmake_minimum_required(VERSION 3.15)
project(PortalProject)


file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp)

message(STATUS ${COMPILE_DEFINITIONS})
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")