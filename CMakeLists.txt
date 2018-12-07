cmake_minimum_required(VERSION 3.0)
project (firware_sim CXX C)

# Make sure we're building with C++ 11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/firmware )

set (FIRMWARE_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/command_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/focuser_state.cpp
)

set (FIRMWARE_SIM_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/command_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/focuser_state.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/firmware_sim/main.cpp
)


add_executable(firward_sim ${FIRMWARE_SIM_SOURCES})

