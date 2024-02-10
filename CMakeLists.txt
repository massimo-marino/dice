#
SET(THE_PROJECT dice)

cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_COMPILER clang++)  ## clang++ shows much better performance at run-time than g++

PROJECT(${THE_PROJECT} LANGUAGES CXX VERSION 1.0.0 DESCRIPTION "dice: test code for random number generation")

# C++ version
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)

#SET(CMAKE_BUILD_TYPE Debug) # Debug, Release, RelWithDebInfo, MinSizeRel

# Set the build type (Debug or Release)
if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
endif()

# Set more compiler flags for debug mode; -g is automatically set
if(CMAKE_BUILD_TYPE MATCHES Debug)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Set more compiler flags for release mode; -O3 is automatically set
if(CMAKE_BUILD_TYPE MATCHES Release)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# Set the variable source_files to the list of names of your C++ source code
# Note the lack of commas or other deliminators
SET(SOURCE_FILES
   dice.cpp
)

# Build a program called '${THE_PROJECT}' from the source files we specified above
ADD_EXECUTABLE(${THE_PROJECT} ${SOURCE_FILES})

