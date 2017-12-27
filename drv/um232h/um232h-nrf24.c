#include <RF24.h>

#include "um232h.h"

/* */

struct nrf24_drv_sbc {
	char *name;
	struct ftdi_context fc;
};

/* */

struct nrf24_drv_sbc drv = {
	.name = "um232",

	.fc = {0},
};

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
	um232h_gpiol_set(&drv.fc, BIT_L2, level);
	return;
}

static void f_ce(int level)
{
	um232h_gpiol_set(&drv.fc, BIT_L1, level);
	return;
}

static uint8_t f_spi_xfer(uint8_t data)
{
	return um232h_spi_byte_xfer(&drv.fc, data);
}

/* */

struct nrf24_drv * nrf24_driver_setup(struct rf24 *pnrf, void *data)
{
	/* um232 gets device by USB Vendor ID and Product ID */
	(void) data;

	/* rf24 ops */

	pnrf->delay_ms = f_delay_ms;
	pnrf->delay_us = f_delay_us;
	pnrf->csn = f_csn;
	pnrf->ce = f_ce;
	pnrf->spi_xfer = f_spi_xfer;
	pnrf->spi_multi_xfer = NULL;

	/* um232 init */

	um232h_mpsse_simple_init(&drv.fc);
	um232h_set_loopback(&drv.fc, 0);
	um232h_set_speed(&drv.fc, 100000);

	return (struct nrf24_drv *)&drv;
}

int nrf24_driver_wait_for(struct nrf24_drv *pdrv)
{
	usleep(100000);
	return 1;
}
