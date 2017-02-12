#include "gpio.h"
#include "spi.h"

/*
 * Hardware setup:
 * pcDuino Lite Wifi + Wireless Gate Shield v1.0b + nRF24L01+
 *
 */

#define PCDUINO_NRF24_CE	7
#define PCDUINO_NRF24_CE_NAME	"gpio7_ph9"

#define PCDUINO_NRF24_CSN	8
#define PCDUINO_NRF24_CSN_NAME	"gpio8_ph10"

#define PCDUINO_RFM69_CSN	9
#define PCDUINO_RFM69_CSN_NAME	"gpio9_ph5"

#define PCDUINO_NRF24_IRQ	2
#define PCDUINO_NRF24_IRQ_NAME	"gpio2_ph7"

/* */

static uint32_t speed = 1000000;

/* hack for linux-sunxi-3.4 */
static uint8_t mode = SPI_LOOP;

static uint8_t bits = 8;
static uint8_t lsb = 0;

/* */

void f_csn(int level)
{
	(void) gpio_write(PCDUINO_NRF24_CSN_NAME, level);
}

void f_ce(int level)
{
	(void) gpio_write(PCDUINO_NRF24_CE_NAME, level);
}

uint8_t f_spi_xfer(uint8_t dat)
{
	return spi_xfer_fdx(dat);
}

int (*f_spi_multi_xfer)(uint8_t *tx, uint8_t *rx, int len) = NULL;

/* */

int nrf24_driver_setup(char *spidev)
{
	/* GPIO */

	if (0 > gpio_setup(PCDUINO_NRF24_CE, PCDUINO_NRF24_CE_NAME, DIR_OUT))
		return -1;

	if (0 > gpio_setup(PCDUINO_NRF24_CSN, PCDUINO_NRF24_CSN_NAME, DIR_OUT))
		return -1;

	if (0 > gpio_setup(PCDUINO_NRF24_IRQ, PCDUINO_NRF24_IRQ_NAME, DIR_IN))
		return -1;

	/* RFM69 radio chip on Wireless Gate Shield v1.0b: disable spi chip select */
	if (0 > gpio_setup(PCDUINO_RFM69_CSN, PCDUINO_RFM69_CSN_NAME, DIR_OUT))
		return -1;

	if (0 > gpio_write(PCDUINO_RFM69_CSN_NAME, 1))
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
