cmake_minimum_required(VERSION 3.26)
cmake_policy(SET CMP0048 NEW)

# Fetch libDaisy
include(FetchContent)
FetchContent_Declare(daisy SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../libDaisy )
FetchContent_MakeAvailable(daisy)

FetchContent_Declare(daisysp SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../DaisySP)
FetchContent_MakeAvailable(daisysp)

# Our project declaration
project(mutable_rings VERSION 0.0.1)

set(FIRMWARE_NAME ${CMAKE_PROJECT_NAME})
set(FIRMWARE_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/../hothouse.cpp
  ${CMAKE_CURRENT_LIST_DIR}/mutable_rings.cpp
)

# Output a .bin file
set(DAISY_GENERATE_BIN YES)

# DaisyProject.cmake uses FIRMWARE_NAME and FIRMWARE_SOURCES to build a target called ${FIRMWARE_NAME}
include(DaisyProject)

target_include_directories(${FIRMWARE_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/../
)

set_target_properties(${FIRMWARE_NAME} PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
)
target_link_libraries(${FIRMWARE_NAME} PUBLIC DaisySP)