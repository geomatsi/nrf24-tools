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

int spi_open(char *spidev);
int spi_info(void);
void spi_close(void);

uint8_t spi_xfer_fdx(uint8_t txdata);
int spi_xfer_mfdx(uint8_t *txdata, uint8_t *rxbuf, int len);
uint8_t spi_xfer_hdx(uint8_t txdata);

#endif /* SPI_HAL_H */
