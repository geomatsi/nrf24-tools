cmake_minimum_required(VERSION "2.8.0")
project("pcDuino Tools")

##

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules)

find_package(ProtobufC REQUIRED)

## options

option (WITH_NRF24_SPIDEV_TEST "Build nRF24L01 test tool for spidev" ON)
option (WITH_NRF24_UM232H_TEST "Build nRF24L01 test tool for UM232H" ON)
option (WITH_NRF24_MOSQUITTO_TEST "Build nRF24L01 mosquitto test publisher" OFF)

## driver sources

file(GLOB_RECURSE UM232H_SRCS ${PROJECT_SOURCE_DIR}/drv/um232h/*.c)
file(GLOB_RECURSE SPIDEV_SRCS ${PROJECT_SOURCE_DIR}/drv/spidev/*.c)

## executable targets

if (WITH_NRF24_SPIDEV_TEST)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS msg.proto)
    add_executable(
        nrf24_spidev_test nrf24_test.c delay.c ${SPIDEV_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
endif (WITH_NRF24_SPIDEV_TEST)

if (WITH_NRF24_UM232H_TEST)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS msg.proto)
    add_executable(nrf24_um232h_test nrf24_test.c delay.c ${UM232H_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
endif (WITH_NRF24_UM232H_TEST)

if (WITH_NRF24_MOSQUITTO_TEST)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS msg.proto)
    add_executable(nrf24_mosquitto_test nrf24_mosquitto_test.c delay.c ${UM232H_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
endif (WITH_NRF24_MOSQUITTO_TEST)

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
include_directories(${MOSQUITTO_INCLUDE_DIR})
include_directories(${FTDI_INCLUDE_DIR})

include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## set libraries paths

if (WITH_NRF24_SPIDEV_TEST)
    target_link_libraries(nrf24_spidev_test ${NRF24_LIBRARY})
    target_link_libraries(nrf24_spidev_test ${PROTOBUFC_LIBRARIES})
endif (WITH_NRF24_SPIDEV_TEST)

if (WITH_NRF24_UM232H_TEST)
    target_link_libraries(nrf24_um232h_test ${NRF24_LIBRARY})
    target_link_libraries(nrf24_um232h_test ${PROTOBUFC_LIBRARIES})
    target_link_libraries(nrf24_um232h_test ${FTDI_LIBRARY})
endif (WITH_NRF24_UM232H_TEST)

if (WITH_NRF24_MOSQUITTO_TEST)
    target_link_libraries(nrf24_mosquitto_test ${NRF24_LIBRARY})
    target_link_libraries(nrf24_mosquitto_test ${MOSQUITTO_LIBRARY})
    target_link_libraries(nrf24_mosquitto_test ${PROTOBUFC_LIBRARIES})
endif (WITH_NRF24_MOSQUITTO_TEST)

## misc

# set path to spidev.h header for specific kernel
if(DEFINED KERNEL_DIR)
    include_directories(${KERNEL_DIR}/include/linux/spi)
    include_directories(${KERNEL_DIR}/include/uapi/linux/spi)
endif(DEFINED KERNEL_DIR)

