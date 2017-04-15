#include <RF24.h>

#include "gpio.h"
#include "spi.h"

/*
 * Hardware setup: orange-pi-zero
 * Notes:
 *   - SPI h/w CSN line is used, no need to toggle GPIO line
 */

#define ORANGE_NRF24_CE		10
#define ORANGE_NRF24_CE_NAME	"gpio10"

#define ORANGE_NRF24_IRQ	18
#define ORANGE_NRF24_IRQ_NAME	"gpio18"

/* */

static uint32_t speed = 1000000;

static uint8_t mode = 0;
static uint8_t bits = 8;
static uint8_t lsb = 0;

/* */

static void f_ce(int level)
{
	(void) gpio_write(ORANGE_NRF24_CE_NAME, level);
}

static int f_spi_multi_xfer(uint8_t *tx, uint8_t *rx, int len)
{
	return spi_xfer_mfdx(tx, rx, len);
}

/* */

int nrf24_driver_setup(struct rf24 *pnrf, char *spidev)
{
	/* rf24 ops */

	pnrf->csn = NULL;
	pnrf->ce = f_ce;
	pnrf->spi_xfer = NULL;
	pnrf->spi_multi_xfer = f_spi_multi_xfer;

	/* GPIO */

	if (0 > gpio_setup(ORANGE_NRF24_CE, ORANGE_NRF24_CE_NAME, DIR_OUT))
		return -1;

	if (0 > gpio_setup(ORANGE_NRF24_IRQ, ORANGE_NRF24_IRQ_NAME, DIR_IN))
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
