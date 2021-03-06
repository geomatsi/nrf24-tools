cmake_minimum_required(VERSION "2.8.0")
project("nRF24 Tools" C)

##

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -D_GNU_SOURCE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic -D_GNU_SOURCE")

find_package(ProtobufC REQUIRED)
set(PROTOBUF_DESC "node_msg_v1.proto" CACHE STRING "Protobuf protocol description")
message(STATUS "Protobuf message description: ${PROTOBUF_DESC}")

## options

option (WITH_NRF24_TEST_PTX "Build nRF24L01 PTX test tool" ON)
option (WITH_NRF24_TEST_PRX "Build nRF24L01 PRX test tool" ON)
option (WITH_NRF24_TEST_HUB "Build nRF24L01 PRX MultiCeiver test tool" ON)
option (WITH_NRF24_DUMP_REGS "Build nRF24L01 register dump tool" ON)
option (WITH_NRF24_MOSQUITTO_PUB "Build nRF24L01 mosquitto publisher" ON)
option (WITH_JSON_CONFIG "JSON config support" ON)
option (WITH_UNIT_TESTS "Build unit tests" OFF)

## select nRF24 connector: SBC/UM232H/...

file(GLOB_RECURSE SBC_SRCS ${PROJECT_SOURCE_DIR}/drv/sbc/*.c)
file(GLOB_RECURSE UM232H_SRCS ${PROJECT_SOURCE_DIR}/drv/um232h/*.c)

set(NRF24_CONN "UM232H" CACHE STRING "nRF24x connector chosen by the user at CMake configure time")

if ( NRF24_CONN STREQUAL "SBC" )
	set (CONN_SRCS ${SBC_SRCS} common/gpio-sysfs.c common/spi-spidev.c)
	message(STATUS "nRF24 connector: ${NRF24_CONN}")
endif ()

if ( NRF24_CONN STREQUAL "UM232H" )
	set (CONN_SRCS ${UM232H_SRCS})
	message(STATUS "nRF24 connector: ${NRF24_CONN}")
endif ()

##

if (WITH_JSON_CONFIG)
	set (CONF_SRCS common/config.c)
	add_definitions(-DWITH_JSON_CONFIG)
	message(STATUS "enable JSON config")
endif ()

## executable targets

if (WITH_NRF24_TEST_PTX)
	add_executable( nrf24_test_ptx src/nrf24_test_ptx.c ${CONN_SRCS} ${CONF_SRCS})
endif ()

if (WITH_NRF24_TEST_PRX)
	add_executable( nrf24_test_prx src/nrf24_test_prx.c ${CONN_SRCS} ${CONF_SRCS})
endif ()

if (WITH_NRF24_DUMP_REGS)
	add_executable( nrf24_dump_regs src/nrf24_dump_regs.c ${CONN_SRCS} ${CONF_SRCS})
endif ()

if (WITH_NRF24_TEST_HUB)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS proto ${PROTOBUF_DESC})
	add_executable(
		nrf24_test_hub
		src/nrf24_test_hub.c
		${CONN_SRCS} ${PROTO_SRCS} ${PROTO_HDRS} ${CONF_SRCS})
endif ()

if (WITH_NRF24_MOSQUITTO_PUB)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS proto ${PROTOBUF_DESC})
	add_executable(
		nrf24_mosquitto_pub
		src/nrf24_mosquitto_pub.c
		${CONN_SRCS} ${PROTO_SRCS} ${PROTO_HDRS} ${CONF_SRCS})
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
PATHS /usr/include /usr/include/libftdi1
)

FIND_PATH(JSON_INCLUDE_DIR
NAMES json.h
PATHS /usr/include/json-c
)

FIND_PATH(CPPUNIT_INCLUDE_DIR
NAMES CppUTest/TestHarness.h
PATHS /usr/include /usr/local/include
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
NAMES ftdi ftdi1
PATHS /usr/lib /usr/lib/x86_64-linux-gnu
)

FIND_LIBRARY(JSON_LIBRARY
NAMES json-c
PATHS /usr/lib /usr/lib/x86_64-linux-gnu
)

FIND_LIBRARY(CPPUNIT_LIBRARY
NAMES CppUTest CppUTestExt
PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
)

## set headers paths

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories(${NRF24_INCLUDE_DIR})

if ( NRF24_CONN STREQUAL "UM232H" )
	include_directories(${FTDI_INCLUDE_DIR})
endif ()

if (WITH_NRF24_MOSQUITTO_TEST)
	include_directories(${MOSQUITTO_INCLUDE_DIR})
endif ()

if (WITH_JSON_CONFIG)
	include_directories(${JSON_INCLUDE_DIR})
endif ()

if (WITH_UNIT_TESTS)
	include_directories(${CPPUNIT_INCLUDE_DIR})
endif ()

# set path to spidev.h header for specific kernel
if(DEFINED KERNEL_DIR)
	include_directories(${KERNEL_DIR}/include/linux/spi)
	include_directories(${KERNEL_DIR}/include/uapi/linux/spi)
endif ()

## set libraries paths

if (WITH_NRF24_TEST_PTX)
	target_link_libraries(nrf24_test_ptx ${NRF24_LIBRARY})

	if (WITH_JSON_CONFIG)
		target_link_libraries(nrf24_test_ptx ${JSON_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_test_ptx ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_TEST_PRX)
	target_link_libraries(nrf24_test_prx ${NRF24_LIBRARY})

	if (WITH_JSON_CONFIG)
		target_link_libraries(nrf24_test_prx ${JSON_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_test_prx ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_DUMP_REGS)
	target_link_libraries(nrf24_dump_regs ${NRF24_LIBRARY})

	if (WITH_JSON_CONFIG)
		target_link_libraries(nrf24_dump_regs ${JSON_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_dump_regs ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_TEST_HUB)
	target_link_libraries(nrf24_test_hub ${PROTOBUFC_LIBRARIES})
	target_link_libraries(nrf24_test_hub ${NRF24_LIBRARY})

	if (WITH_JSON_CONFIG)
		target_link_libraries(nrf24_test_hub ${JSON_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_test_hub ${FTDI_LIBRARY})
	endif ()
endif ()

if (WITH_NRF24_MOSQUITTO_PUB)
	target_link_libraries(nrf24_mosquitto_pub ${MOSQUITTO_LIBRARY})
	target_link_libraries(nrf24_mosquitto_pub ${PROTOBUFC_LIBRARIES})
	target_link_libraries(nrf24_mosquitto_pub ${NRF24_LIBRARY})

	if (WITH_JSON_CONFIG)
		target_link_libraries(nrf24_mosquitto_pub ${JSON_LIBRARY})
	endif ()

	if ( NRF24_CONN STREQUAL "UM232H" )
		target_link_libraries(nrf24_mosquitto_pub ${FTDI_LIBRARY})
	endif ()
endif ()

## tests

if (WITH_UNIT_TESTS)
	enable_language(CXX)
	enable_testing()

	add_executable(
		test1
		test/main_test.cpp
		test/conf_tests.cpp
		${CONF_SRCS})

	target_link_libraries(test1 ${CPPUNIT_LIBRARY})

	if (WITH_JSON_CONFIG)
		target_link_libraries(test1 ${JSON_LIBRARY})
	endif ()

	add_test(test1 "./test1")
endif ()
