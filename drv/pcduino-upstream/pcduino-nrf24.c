#include <RF24.h>

#include "gpio.h"
#include "spi.h"

/*
 * Hardware setup:
 * pcDuino Lite Wifi + Wireless Gate Shield v1.0b + nRF24L01+
 *
 */

/* GPIO PH9 */
#define PCDUINO_NRF24_CE	233
#define PCDUINO_NRF24_CE_NAME	"gpio233"

/* GPIO PH10 */
#define PCDUINO_NRF24_CSN	234
#define PCDUINO_NRF24_CSN_NAME	"gpio234"

/* GPIO PH5 */
#define PCDUINO_RFM69_CSN	229
#define PCDUINO_RFM69_CSN_NAME	"gpio229"

/* GPIO PH7 */
#define PCDUINO_NRF24_IRQ	231
#define PCDUINO_NRF24_IRQ_NAME	"gpio231"

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
	(void) gpio_write(PCDUINO_NRF24_CSN_NAME, level);
}

static void f_ce(int level)
{
	(void) gpio_write(PCDUINO_NRF24_CE_NAME, level);
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
