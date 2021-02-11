# CMakeList.txt : CMake project for GSTransferTests, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
add_definitions (-mfloat-abi=hard -mfpu=neon) 
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")

# Add source to this project's executable.
add_executable (GSTransferTests "GSTransferTests.cpp" "GSTransferTests.h"  "Baseline.cpp" "SSEVersion.cpp")

# TODO: Add tests and install targets if needed.
