#ifndef SPI_HAL_H
#define SPI_HAL_H

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <linux/types.h>
#include <inttypes.h>

#include <sys/ioctl.h>

#include <spidev.h>

/* */

int pcduino_spi_open(char *spidev);
int pcduino_spi_init(void);
int pcduino_spi_info(void);
void pcduino_spi_close(void);

uint8_t pcduino_spi_xfer_fdx(uint8_t txdata);
uint8_t pcduino_spi_xfer_hdx(uint8_t txdata);

#endif /* SPI_HAL_H */
