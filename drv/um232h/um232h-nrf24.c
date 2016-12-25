#include "um232h.h"

/* */

struct ftdi_context fc;

/* */

void f_csn(int level)
{
	um232h_gpiol_set(&fc, BIT_L2, level);
	return;
}

void f_ce(int level)
{
	um232h_gpiol_set(&fc, BIT_L1, level);
	return;
}

void f_spi_set_speed(int khz)
{
	/* not implemented */
}

uint8_t f_spi_xfer(uint8_t data)
{
	return um232h_spi_byte_xfer(&fc, data);
}

/* */

int nrf24_driver_setup(void)
{
	um232h_mpsse_simple_init(&fc);
	um232h_set_loopback(&fc, 0);
	um232h_set_speed(&fc, 100000);
}
