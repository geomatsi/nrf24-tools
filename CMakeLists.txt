cmake_minimum_required(VERSION "2.8.0")
project("pcDuino Tools")

##

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/Modules)

find_package(ProtobufC REQUIRED)

## options

option (WITH_NRF24_TEST	"Build nRF24L01 test tool"	ON)
option (WITH_NRF24_MOSQUITTO_TEST	"Build nRF24L01 mosquitto test publisher"	ON)

## executable targets

if (WITH_NRF24_TEST)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS msg.proto)
    add_executable(nrf24_test nrf24_test.c delay.c gpio.c spi.c ${PROTO_SRCS} ${PROTO_HDRS})
endif (WITH_NRF24_TEST)

if (WITH_NRF24_MOSQUITTO_TEST)
	PROTOBUFC_GENERATE_C(PROTO_SRCS PROTO_HDRS msg.proto)
    add_executable(nrf24_mosquitto_test nrf24_mosquitto_test.c delay.c gpio.c spi.c ${PROTO_SRCS} ${PROTO_HDRS})
endif (WITH_NRF24_MOSQUITTO_TEST)

## find headers

FIND_PATH(NRF24_INCLUDE_DIR
NAMES RF24.h nRF24L01.h
PATHS /usr/include/nrf24 /usr/include/linux/nrf24
)

FIND_PATH(MOSQUITTO_INCLUDE_DIR
NAMES mosquitto.h
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

## set headers paths

include_directories(${PROJECT_SOURCE_DIR})

include_directories(${NRF24_INCLUDE_DIR})
include_directories(${MOSQUITTO_INCLUDE_DIR})

include_directories(${PROTOBUFC_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## set libraries paths

target_link_libraries(nrf24_test ${NRF24_LIBRARY})
target_link_libraries(nrf24_test ${PROTOBUFC_LIBRARIES})

target_link_libraries(nrf24_mosquitto_test ${NRF24_LIBRARY})
target_link_libraries(nrf24_mosquitto_test ${MOSQUITTO_LIBRARY})
target_link_libraries(nrf24_mosquitto_test ${PROTOBUFC_LIBRARIES})

## misc

# set path to spidev.h header for specific kernel
if(DEFINED KERNEL_DIR)
    include_directories(${KERNEL_DIR}/include/linux/spi)
    include_directories(${KERNEL_DIR}/include/uapi/linux/spi)
endif(DEFINED KERNEL_DIR)

