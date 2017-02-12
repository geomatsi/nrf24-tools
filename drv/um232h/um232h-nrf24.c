#include <RF24.h>

#include "um232h.h"

/* */

struct ftdi_context fc;

/* */

static void f_csn(int level)
{
	um232h_gpiol_set(&fc, BIT_L2, level);
	return;
}

static void f_ce(int level)
{
	um232h_gpiol_set(&fc, BIT_L1, level);
	return;
}

static uint8_t f_spi_xfer(uint8_t data)
{
	return um232h_spi_byte_xfer(&fc, data);
}

/* */

int nrf24_driver_setup(struct rf24 *pnrf, char *spidev)
{
	/* um232 gets device by USB Vendor ID and Product ID */
	(void) spidev;

	/* rf24 ops */

	pnrf->csn = f_csn;
	pnrf->ce = f_ce;
	pnrf->spi_xfer = f_spi_xfer;
	pnrf->spi_multi_xfer = NULL;

	/* um232 init */

	um232h_mpsse_simple_init(&fc);
	um232h_set_loopback(&fc, 0);
	um232h_set_speed(&fc, 100000);
}
