#include "gpio.h"
#include "spi.h"

/*
 * Hardware setup: orange-pi-one
 * Notes:
 *   - SPI h/w CSN line is used, no need to toggle GPIO line
 */

#define ORANGE_NRF24_CE		21
#define ORANGE_NRF24_CE_NAME	"gpio21"

#define ORANGE_NRF24_IRQ	68
#define ORANGE_NRF24_IRQ_NAME	"gpio68"

/* */

static uint32_t speed = 1000000;

static uint8_t mode = 0;
static uint8_t bits = 8;
static uint8_t lsb = 0;

/* */

void (*f_csn)(int level) = NULL;

void f_ce(int level)
{
	(void) gpio_write(ORANGE_NRF24_CE_NAME, level);
}

uint8_t (*f_spi_xfer)(uint8_t dat) = NULL;

int f_spi_multi_xfer(uint8_t *tx, uint8_t *rx, int len)
{
	return spi_xfer_mfdx(tx, rx, len);
}

/* */

int nrf24_driver_setup(char *spidev)
{
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
