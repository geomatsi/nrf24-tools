#ifndef SPI_HAL_H
#define SPI_HAL_H

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>

#include <linux/types.h>
#include <inttypes.h>

#include <sys/ioctl.h>

#ifdef SUNXI_KERNEL
#include "spidev/spidev_sunxi_3.4.h"
#else
#include "spidev/spidev_upstream_4.0.h"
#endif

/* */

void pcduino_spi_open(char *spidev);
void pcduino_spi_init(void);
void pcduino_spi_info(void);
void pcduino_spi_close(void);

uint8_t pcduino_spi_xfer_fdx(uint8_t txdata);
uint8_t pcduino_spi_xfer_hdx(uint8_t txdata);

#endif /* SPI_HAL_H */
