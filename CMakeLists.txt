cmake_minimum_required(VERSION 3.0)
project (firware_sim CXX C)

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

