cmake_minimum_required(VERSION 3.0)
project (firware_sim CXX C)

OPTION (BEEFOCUS_CLANG, "Build using Clang" OFF)

# Make sure we're building with C++ 11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ( BEEFOCUS_CLANG )
    MESSAGE (STATUS  "Building using CLANG")
    SET (CMAKE_C_COMPILER             "/usr/bin/clang")
    SET (CMAKE_C_FLAGS                "-Wall -std=c99")
    SET (CMAKE_C_FLAGS_DEBUG          "-g")
    SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
    SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

    SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
    SET (CMAKE_CXX_FLAGS                "-Wall")
    SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
    SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
    SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

    SET (CMAKE_AR      "/usr/bin/llvm-ar")
    SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
    SET (CMAKE_NM      "/usr/bin/llvm-nm")
    SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
    SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")
ENDIF ( BEEFOCUS_CLANG )

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
  add_definitions( -DGTEST_FOUND )
  MESSAGE (STATUS  "GTEST found, running unit tests")
  ADD_SUBDIRECTORY(unit_tests)
ELSE()
  MESSAGE (STATUS  "GTEST not found, skipping unit tests")
ENDIF (GTEST_FOUND)

add_executable(firware_sim ${FIRMWARE_SIM_SOURCES})


