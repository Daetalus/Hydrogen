
#
#  Hydrogen Build File
#

cmake_minimum_required(VERSION 2.8)
project(Hydrogen)

# Set the default build type to `release`.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Debug and release flags
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -O3")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

# Hydrogen include directory
include_directories(include)

# Components
add_subdirectory(src/vm)
add_subdirectory(src/std)
add_subdirectory(src/cli)
add_subdirectory(tests)
