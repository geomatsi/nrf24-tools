#include "pcduino-gpio.h"
#include "pcduino-spi.h"

/* */

#define PCDUINO_NRF24_CE        7
#define PCDUINO_NRF24_CE_NAME   "gpio7_ph9"

#define PCDUINO_NRF24_CSN       8
#define PCDUINO_NRF24_CSN_NAME  "gpio8_ph10"

#define PCDUINO_RFM69_CSN       9
#define PCDUINO_RFM69_CSN_NAME  "gpio9_ph5"

#define PCDUINO_NRF24_IRQ       2
#define PCDUINO_NRF24_IRQ_NAME  "gpio2_ph7"

#define PCDUINO_NRF24_SPI	"/dev/spidev0.0"

/* */

void f_csn(int level)
{
	(void) pcduino_gpio_write(PCDUINO_NRF24_CSN_NAME, level);
}

void f_ce(int level)
{
	(void) pcduino_gpio_write(PCDUINO_NRF24_CE_NAME, level);
}

void f_spi_set_speed(int khz)
{
	/* not implemented */
}

uint8_t f_spi_xfer(uint8_t dat)
{
	return pcduino_spi_xfer_fdx(dat);
}

/* */

int nrf24_driver_setup(void)
{
    /* GPIO */

	if (0 > pcduino_gpio_setup(PCDUINO_NRF24_CE, PCDUINO_NRF24_CE_NAME, DIR_OUT))
		return -1;

	if (0 > pcduino_gpio_setup(PCDUINO_NRF24_CSN, PCDUINO_NRF24_CSN_NAME, DIR_OUT))
		return -1;

	if (0 > pcduino_gpio_setup(PCDUINO_NRF24_IRQ, PCDUINO_NRF24_IRQ_NAME, DIR_IN))
		return -1;

    /* disable spi chip select for RFM69 */
	if (0 > pcduino_gpio_setup(PCDUINO_RFM69_CSN, PCDUINO_RFM69_CSN_NAME, DIR_OUT))
		return -1;

	if (0 > pcduino_gpio_write(PCDUINO_RFM69_CSN_NAME, 1))
		return -1;

    /* SPI */

	if (0 > pcduino_spi_open(PCDUINO_NRF24_SPI))
		return -1;

	if (0 > pcduino_spi_info())
		return -1;

	if (0 > pcduino_spi_init())
		return -1;

	if (0 > pcduino_spi_info())
		return -1;

	return 0;
}
