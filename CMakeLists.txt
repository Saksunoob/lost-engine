# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(LostEngine VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the root path
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/..)

# Load Libraries
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIR})

# Include the src directory
include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/demos ${PROJECT_SOURCE_DIR}/include)

# Find all source files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp" "demos/square/*.cpp")

# Add the executable
add_executable(LostEngine ${SOURCES})

# Link SDL2
target_link_libraries(LostEngine ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})