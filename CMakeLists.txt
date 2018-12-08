cmake_minimum_required(VERSION 3.0)
project (firware_sim CXX C)

# Make sure we're building with C++ 11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/firmware )

set (FIRMWARE_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/command_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/focuser_state.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/hardware_interface.cpp
)

set (FIRMWARE_SIM_SOURCES ${FIRMWARE_SOURCES} )
LIST(APPEND FIRMWARE_SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/firmware_sim/main.cpp)

# Testing
find_package (GTest)
find_package (GMock)

IF (GTEST_FOUND)
  MESSAGE (STATUS  "GTEST found, running unit tests")
  ADD_SUBDIRECTORY(unit_tests)
ELSE()
  MESSAGE (STATUS  "GTEST not found, skipping unit tests")
ENDIF (GTEST_FOUND)

add_executable(firware_sim ${FIRMWARE_SIM_SOURCES})


