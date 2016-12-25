#include "gpio.h"
#include "spi.h"

/*
 * Hardware setup: orange-pi-one
 */

/* GPIO TODO */
#define ORANGE_NRF24_CE		0
#define ORANGE_NRF24_CE_NAME	"gpio0"

/* GPIO TODO */
#define ORANGE_NRF24_CSN	0
#define ORANGE_NRF24_CSN_NAME	"gpio0"

/* GPIO TODO */
#define ORANGE_NRF24_IRQ	0
#define ORANGE_NRF24_IRQ_NAME	"gpio0"

/* */

static uint32_t speed = 1000000;

static uint8_t mode = 0;
static uint8_t bits = 8;
static uint8_t lsb = 0;

/* */

void f_csn(int level)
{
	(void) gpio_write(ORANGE_NRF24_CSN_NAME, level);
}

void f_ce(int level)
{
	(void) gpio_write(ORANGE_NRF24_CE_NAME, level);
}

void f_spi_set_speed(int khz)
{
	/* not implemented */
}

uint8_t f_spi_xfer(uint8_t dat)
{
	return spi_xfer_fdx(dat);
}

/* */

int nrf24_driver_setup(char *spidev)
{
	/* GPIO */

	if (0 > gpio_setup(ORANGE_NRF24_CE, ORANGE_NRF24_CE_NAME, DIR_OUT))
		return -1;

	if (0 > gpio_setup(ORANGE_NRF24_CSN, ORANGE_NRF24_CSN_NAME, DIR_OUT))
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
