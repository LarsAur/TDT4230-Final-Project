cmake_minimum_required(VERSION 3.6)
project(PortalProject)

#
# CMake setup
#
set (CMAKE_CXX_STANDARD 14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set (CMAKE_VERBOSE_MAKEFILE 0) # 1 should be used for debugging
set (CMAKE_SUPPRESS_REGENERATION TRUE) # Suppresses ZERO_CHECK
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
  if(NOT WIN32)
    set(GLAD_LIBRARIES dl)
  endif()
endif()

#
# GLFW options
#
option (GLFW_INSTALL OFF)
option (GLFW_BUILD_DOCS OFF)
option (GLFW_BUILD_EXAMPLES OFF)
option (GLFW_BUILD_TESTS OFF)
add_subdirectory (libs/glfw)

#
# GLAD
#
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/include)
    message(STATUS "Generating glad library files, using ${PYTHON_CMD}")
	execute_process(
		COMMAND python -m glad --profile core --out-path . --generator c --spec gl
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad
	)
	message("Finished generating glad library files")
endif()

file (GLOB GLAD_SOURCES libs/glad/src/glad.c)
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp)

add_definitions(
    -DGLFW_INCLUDE_NONE
    -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${GLAD_SOURCES})

target_link_libraries(
    ${PROJECT_NAME} 
    glfw 
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES}
)
