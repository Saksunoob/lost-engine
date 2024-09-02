# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(LostEngine VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compile shaders
message("CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

set(SHADERS_IN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
set(SHADERS_OUT_DIR "${CMAKE_BINARY_DIR}/shaders")

file(GLOB_RECURSE SHADERS "${SHADERS_IN_DIR}/*.vert" "${SHADERS_IN_DIR}/*.frag")

file(MAKE_DIRECTORY ${SHADERS_OUT_DIR})

foreach(SHADER ${SHADERS})
    message("Building shader: ${SHADER}")
	file(RELATIVE_PATH SHADER_OUT_NAME ${SHADERS_IN_DIR} ${SHADER})
    set(SHADER_OUT_NAME "${SHADERS_OUT_DIR}/${SHADER_OUT_NAME}")
	list(APPEND SHADER_OUT_NAMES ${SHADER_OUT_NAME})
    get_filename_component(SHADER_DIR ${SHADER_OUT_NAME} DIRECTORY)
    file(MAKE_DIRECTORY ${SHADER_DIR})
	add_custom_command(
		MAIN_DEPENDENCY ${SHADER}
		OUTPUT ${SHADER_OUT_NAME}
		COMMAND ${Vulkan_GLSLC_EXECUTABLE}  ${SHADER} "-o" ${SHADER_OUT_NAME}
		VERBATIM)
endforeach()

add_custom_target(build_shaders DEPENDS ${SHADER_OUT_NAMES})

# Load Libraries
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIR})

# Include the src directory
include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/demos ${PROJECT_SOURCE_DIR}/include)

# Find all source files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp" "demos/square/*.cpp")

# Add the executable
add_executable(LostEngineDemo ${SOURCES})

# Link SDL2
target_link_libraries(LostEngineDemo ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})

file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")

# Add the library
add_library(LostEngine STATIC ${LIB_SOURCES})

# Link SDL2
target_link_libraries(LostEngine ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})

add_dependencies(${PROJECT_NAME} build_shaders)