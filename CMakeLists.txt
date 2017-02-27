cmake_minimum_required(VERSION "2.8.0")
project("nRF24 Tools")

##

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

find_package(ProtobufC REQUIRED)

## options

option (WITH_NRF24_TEST_PTX "Build nRF24L01 PTX test tool" ON)
option (WITH_NRF24_TEST_PRX "Build nRF24L01 PRX test tool" ON)
option (WITH_NRF24_TEST_HUB "Build nRF24L01 PRX MultiCeiver test tool" ON)
option (WITH_NRF24_DUMP_REGS "Build nRF24L01 register dump tool" ON)
option (WITH_NRF24_MOSQUITTO_PUB "Build nRF24L01 mosquitto publisher" OFF)
option (WITH_SERIAL_MOSQUITTO_TEST "Build serial port mosquitto test publisher" OFF)

## select nRF24 connector: SPIDEV/UM232H/...

file(GLOB_RECURSE UM232H_SRCS ${PROJECT_SOURCE_DIR}/drv/um232h/*.c)
file(GLOB_RECURSE PCDUINO_UPSTREAM_SRCS ${PROJECT_SOURCE_DIR}/drv/pcduino-upstream/*.c)
file(GLOB_RECURSE PCDUINO_LEGACY_SRCS ${PROJECT_SOURCE_DIR}/drv/pcduino-legacy/*.c)
file(GLOB_RECURSE BONE_SRCS ${PROJECT_SOURCE_DIR}/drv/bone/*.c)
file(GLOB_RECURSE ORANGE_PI_ONE_SRCS ${PROJECT_SOURCE_DIR}/drv/orange-pi-one/*.c)

set(NRF24_CONN "UM232H" CACHE STRING "nRF24x connector chosen by the user at CMake configure time")

if ( NRF24_CONN STREQUAL "PCDUINO_UPSTREAM" )
	set (CONN_SRCS ${PCDUINO_UPSTREAM_SRCS}
		common/gpio-sysfs.c common/spi-spidev.c common/delay.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

if ( NRF24_CONN STREQUAL "PCDUINO_LEGACY" )
	set (CONN_SRCS ${PCDUINO_LEGACY_SRCS}
		common/gpio-sysfs.c common/spi-spidev.c common/delay.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

if ( NRF24_CONN STREQUAL "BONE" )
	set (CONN_SRCS ${BONE_SRCS}
		common/gpio-sysfs.c common/spi-spidev.c common/delay.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

if ( NRF24_CONN STREQUAL "UM232H" )
	set (CONN_SRCS ${UM232H_SRCS} common/delay.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

if ( NRF24_CONN STREQUAL "ORANGE_PI_ONE" )
	set (CONN_SRCS ${ORANGE_PI_ONE_SRCS}
		common/gpio-sysfs.c common/spi-spidev.c common/delay.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN} (${CONN_SRCS})")
endif ()

## executable targets

if (WITH_NRF24_TEST_PTX)
	add_executable( nrf24_test_ptx src/nrf24_test_ptx.c ${CONN_SRCS})
endif ()

if (WITH_NRF24_TEST_PRX)
	add_executable( nrf24_test_prx src/nrf24_test_prx.c ${CONN_SRCS})
endif ()

if (WITH_NRF24_DUMP_REGS)
	add_executable( nrf24_dump_regs src/nrf24_dump_regs.c ${CONN_SRCS})
endif ()

if (WITH_NRF24_TEST_HUB)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS proto msg.proto)
	add_executable(
		nrf24_test_hub
		src/nrf24_test_hub.c
		${CONN_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
endif ()

if (WITH_NRF24_MOSQUITTO_PUB)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS proto msg.proto)
	add_executable(
		nrf24_mosquitto_pub
		src/nrf24_mosquitto_pub.c
		${CONN_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
endif ()

if (WITH_SERIAL_MOSQUITTO_TEST)
	add_executable(
		serial_mosquitto_test
		src/serial_mosquitto_test.c)
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

FIND_LIBRARY(NRF24_MB_LIBRARY
NAMES nrf24_mb
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

include_directories(${NRF24_INCLUDE_DIR})
include_directories(${PROTOBUFC_INCLUDE_DIRS})

if ( NRF24_CONN STREQUAL "UM232H" )
	include_directories(${FTDI_INCLUDE_DIR})
endif ()

if (WITH_NRF24_MOSQUITTO_TEST)
	include_directories(${MOSQUITTO_INCLUDE_DIR})
endif ()

if (WITH_SERIAL_MOSQUITTO_TEST)
	include_directories(${MOSQUITTO_INCLUDE_DIR})
endif ()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## set libraries paths

if (WITH_NRF24_TEST_PTX)
	if ( NRF24_CONN STREQUAL "ORANGE_PI_ONE" )
		target_link_libraries(nrf24_test_ptx ${NRF24_MB_LIBRARY})
	else ()
		target_link_libraries(nrf24_test_ptx ${NRF24_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_test_ptx ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_TEST_PRX)
	if ( NRF24_CONN STREQUAL "ORANGE_PI_ONE" )
		target_link_libraries(nrf24_test_prx ${NRF24_MB_LIBRARY})
	else ()
		target_link_libraries(nrf24_test_prx ${NRF24_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_test_prx ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_DUMP_REGS)
	if ( NRF24_CONN STREQUAL "ORANGE_PI_ONE" )
		target_link_libraries(nrf24_dump_regs ${NRF24_MB_LIBRARY})
	else ()
		target_link_libraries(nrf24_dump_regs ${NRF24_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_dump_regs ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_TEST_HUB)
	target_link_libraries(nrf24_test_hub ${PROTOBUFC_LIBRARIES})

	if ( NRF24_CONN STREQUAL "ORANGE_PI_ONE" )
		target_link_libraries(nrf24_test_hub ${NRF24_MB_LIBRARY})
	else ()
		target_link_libraries(nrf24_test_hub ${NRF24_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_test_hub ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_MOSQUITTO_PUB)
	target_link_libraries(nrf24_mosquitto_pub ${MOSQUITTO_LIBRARY})
	target_link_libraries(nrf24_mosquitto_pub ${PROTOBUFC_LIBRARIES})

	if ( NRF24_CONN STREQUAL "ORANGE_PI_ONE" )
		target_link_libraries(nrf24_mosquitto_pub ${NRF24_MB_LIBRARY})
	else ()
		target_link_libraries(nrf24_mosquitto_pub ${NRF24_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_mosquitto_pub ${FTDI_LIBRARY})
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
