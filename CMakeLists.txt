cmake_minimum_required(VERSION "2.8.0")
project("pcDuino Tools")

## options

option (WITH_NRF24_TEST	"Build nRF24L01 test tool"	ON)
option (SUNXI_KERNEL	"Build for linux-sunxi-3.4"	ON)

## executable targets

if (WITH_NRF24_TEST)
    add_executable(nrf24_test nrf24_test.c delay.c gpio.c spi.c)
endif (WITH_NRF24_TEST)

## find headers

FIND_PATH(NRF24_INCLUDE_DIR
NAMES RF24.h nRF24L01.h
PATHS /usr/include/nrf24 /usr/include/linux/nrf24
)

## find libraries

FIND_LIBRARY(NRF24_LIBRARY
NAMES nrf24
PATHS /usr/lib /usr/local/lib
)

## set headers paths

include_directories(${NRF24_INCLUDE_DIR})

## set libraries paths

target_link_libraries(nrf24_test ${NRF24_LIBRARY})

## misc

if (SUNXI_KERNEL)
	add_definitions(-DSUNXI_KERNEL)
endif (SUNXI_KERNEL)
