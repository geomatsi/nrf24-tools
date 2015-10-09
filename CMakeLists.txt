cmake_minimum_required(VERSION "2.8.0")
project("nRF24 Tools")

##

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules)

find_package(ProtobufC REQUIRED)

## options

option (WITH_NRF24_TEST "Build nRF24L01 basic test tool" ON)
option (WITH_NRF24_MOSQUITTO_TEST "Build nRF24L01 mosquitto test publisher" OFF)
option (WITH_SERIAL_MOSQUITTO_TEST "Build serial port mosquitto test publisher" OFF)

## select nRF24 connector: SPIDEV/UM232H/...

file(GLOB_RECURSE UM232H_SRCS ${PROJECT_SOURCE_DIR}/drv/um232h/*.c)
file(GLOB_RECURSE PCDUINO_SRCS ${PROJECT_SOURCE_DIR}/drv/pcduino/*.c)
file(GLOB_RECURSE BONE_SRCS ${PROJECT_SOURCE_DIR}/drv/bone/*.c)

set(NRF24_CONN "UM232H" CACHE STRING "nRF24x connector chosen by the user at CMake configure time")

if ( NRF24_CONN STREQUAL "PCDUINO" )
    set (CONN_SRCS ${PCDUINO_SRCS} common/gpio.c common/spi.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

if ( NRF24_CONN STREQUAL "BONE" )
    set (CONN_SRCS ${BONE_SRCS} common/gpio.c common/spi.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

if ( NRF24_CONN STREQUAL "UM232H" )
	set (CONN_SRCS ${UM232H_SRCS})
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

## executable targets

if (WITH_NRF24_TEST)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS msg.proto)
    add_executable(nrf24_test nrf24_test.c delay.c ${CONN_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
endif ()

if (WITH_NRF24_MOSQUITTO_TEST)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS msg.proto)
    add_executable(nrf24_mosquitto_test nrf24_mosquitto_test.c delay.c ${CONN_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
endif ()

if (WITH_SERIAL_MOSQUITTO_TEST)
    add_executable(serial_mosquitto_test serial_mosquitto_test.c)
endif ()

## find headers

FIND_PATH(NRF24_INCLUDE_DIR
NAMES RF24.h nRF24L01.h
PATHS /usr/include/nrf24 /usr/include/linux/nrf24 /usr/local/include
)

FIND_PATH(MOSQUITTO_INCLUDE_DIR
NAMES mosquitto.h
PATHS /usr/include
)

FIND_PATH(FTDI_INCLUDE_DIR
NAMES ftdi.h
PATHS /usr/include
)

## find libraries

FIND_LIBRARY(NRF24_LIBRARY
NAMES nrf24
PATHS /usr/lib /usr/local/lib
)

FIND_LIBRARY(PROTOBUF_C_LIBRARY
NAMES protobuf-c
PATHS /usr/lib /usr/local/lib
)

FIND_LIBRARY(MOSQUITTO_LIBRARY
NAMES mosquitto
PATHS /usr/lib /usr/local/lib
)

FIND_LIBRARY(FTDI_LIBRARY
NAMES ftdi
PATHS /usr/lib /usr/lib/x86_64-linux-gnu
)

## set headers paths

include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${NRF24_INCLUDE_DIR})

if ( NRF24_CONN STREQUAL "UM232H" )
	include_directories(${FTDI_INCLUDE_DIR})
endif ()

if (WITH_NRF24_MOSQUITTO_TEST)
	include_directories(${MOSQUITTO_INCLUDE_DIR})
endif ()

if (WITH_SERIAL_MOSQUITTO_TEST)
	include_directories(${MOSQUITTO_INCLUDE_DIR})
endif ()

include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## set libraries paths

if (WITH_NRF24_TEST)
    target_link_libraries(nrf24_test ${NRF24_LIBRARY})
    target_link_libraries(nrf24_test ${PROTOBUFC_LIBRARIES})

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_test ${FTDI_LIBRARY})
	endif ()

endif ()

if (WITH_NRF24_MOSQUITTO_TEST)
    target_link_libraries(nrf24_mosquitto_test ${NRF24_LIBRARY})
    target_link_libraries(nrf24_mosquitto_test ${MOSQUITTO_LIBRARY})
    target_link_libraries(nrf24_mosquitto_test ${PROTOBUFC_LIBRARIES})

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_mosquitto_test ${FTDI_LIBRARY})
	endif ()

endif ()

if (WITH_SERIAL_MOSQUITTO_TEST)
    target_link_libraries(serial_mosquitto_test ${MOSQUITTO_LIBRARY})

endif ()

## misc

# set path to spidev.h header for specific kernel

if(DEFINED KERNEL_DIR)
    include_directories(${KERNEL_DIR}/include/linux/spi)
    include_directories(${KERNEL_DIR}/include/uapi/linux/spi)
endif ()
