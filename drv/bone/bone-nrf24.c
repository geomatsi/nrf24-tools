#include <RF24.h>

#include "gpio.h"
#include "spi.h"

/*
 * Hardware setup:
 * nRF24L01 connected to BeagleBone[Black] SPI0 using P9 Expanson Header
 *
 * Connection table:
 *   P9 pin | nRF24 | CPU MUX                   | Comment
 * ---------------------------------------------------------------------------
 *   15     | CE    | gpmc_a0.gpio1_16 mode7    | gpio1_16 = 32 + 16 = gpio48
 * ---------------------------------------------------------------------------
 *   23     | CSN   | gpmc_a1.gpio1_17 mode7    | gpio1_17 = 32 + 17 = gpio49
 * ---------------------------------------------------------------------------
 *   22     | SCK   | spi0_sclk.spi0_sclk mode0 |
 * ---------------------------------------------------------------------------
 *   21     | MISO  | spi0_d0.spi0_d0 mode0     |
 * ---------------------------------------------------------------------------
 *   18     | MOSI  | spi0_d1.spi0_d1 mode0     |
 * ---------------------------------------------------------------------------
 *   17     | CSN   | spi0_cs0.spi0_cs0 mode0   | not used: use gpio49
 * ---------------------------------------------------------------------------
 *   1      | GND   |                           |
 * ---------------------------------------------------------------------------
 *   3      | 3V3   |                           |
 * ---------------------------------------------------------------------------
 *
 */

#define BEAGLE_NRF24_CE		48
#define BEAGLE_NRF24_CE_NAME	"gpio48"

#define BEAGLE_NRF24_CSN	49
#define BEAGLE_NRF24_CSN_NAME	"gpio49"

/* */

static uint32_t speed = 1000000;

static uint8_t mode = 0;
static uint8_t bits = 8;
static uint8_t lsb = 0;

/* */

static void f_delay_ms(int mdelay)
{
	usleep(1000*mdelay);
}

static void f_delay_us(int udelay)
{
	usleep(udelay);
}

/* */

static void f_csn(int level)
{
	(void) gpio_write(BEAGLE_NRF24_CSN_NAME, level);
}

static void f_ce(int level)
{
	(void) gpio_write(BEAGLE_NRF24_CE_NAME, level);
}

static uint8_t f_spi_xfer(uint8_t dat)
{
	return spi_xfer_fdx(dat);
}

/* */

int nrf24_driver_setup(struct rf24 *pnrf, char *spidev)
{
	/* rf24 ops */

	pnrf->delay_ms = f_delay_ms;
	pnrf->delay_us = f_delay_us;
	pnrf->csn = f_csn;
	pnrf->ce = f_ce;
	pnrf->spi_xfer = f_spi_xfer;
	pnrf->spi_multi_xfer = NULL;

	/* GPIO */

	if (0 > gpio_setup(BEAGLE_NRF24_CE, BEAGLE_NRF24_CE_NAME, DIR_OUT))
		return -1;

	if (0 > gpio_setup(BEAGLE_NRF24_CSN, BEAGLE_NRF24_CSN_NAME, DIR_OUT))
		return -1;

	/* SPI */

	if (0 > spi_open(spidev))
		return -1;

	if (0 > spi_info())
		return -1;

	if (0 > spi_init(speed, mode, bits, lsb))
		return -1;

	if (0 > spi_info())
		return -1;

	return 0;
}
