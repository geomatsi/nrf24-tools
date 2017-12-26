#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <RF24.h>

#include "config.h"
#include "gpio.h"
#include "spi.h"

/* */

extern int pcduino_upstream_fixup(void);
extern int pcduino_legacy_fixup(void);

/* */

struct sbc_handler {
	char *name;
	char *desc;

	char *pin_ce_name;
	int pin_ce_dflt;

	char *pin_csn_name;
	int pin_csn_dflt;

	char *pin_irq_name;
	int pin_irq_dflt;
	int pin_irq_edge_dflt;


	void    (*delay_ms)(int);
	void    (*delay_us)(int);
	void    (*csn)(int level);
	void    (*ce)(int level);
	uint8_t (*spi_sb_xfer)(uint8_t dat);
	int     (*spi_mb_xfer)(uint8_t *tx, uint8_t *rx, int len);
	int	(*fixup)(void);
};

/* */

static char *pin_ce_name = NULL;
static char *pin_csn_name = NULL;
static char *pin_irq_name = NULL;

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

static void f_ce(int level)
{
	(void) gpio_write(pin_ce_name, level);
}

static void f_csn(int level)
{
	(void) gpio_write(pin_csn_name, level);
}

/* */

static uint8_t f_spi_sb_xfer(uint8_t dat)
{
	return spi_xfer_fdx(dat);
}

static int f_spi_mb_xfer(uint8_t *tx, uint8_t *rx, int len)
{
	return spi_xfer_mfdx(tx, rx, len);
}

/* */

struct sbc_handler boards[] = {

	/*
	 * Orange Pi One
	 * Notes:
	 *   - SPI h/w CSN line is used, no need to toggle GPIO line
	 */
	{

		.name = "orange-pi-one",
		.desc = "Orange Pi One",
		.pin_ce_dflt = 21,
		.pin_ce_name = "gpio21",
		.pin_csn_dflt = 0,
		.pin_csn_name = NULL,
		.pin_irq_dflt = 0,
		.pin_irq_edge_dflt = 0,
		.pin_irq_name = NULL,
		.delay_ms = f_delay_ms,
		.delay_us = f_delay_us,
		.csn = NULL,
		.ce = f_ce,
		.spi_sb_xfer = NULL,
		.spi_mb_xfer = f_spi_mb_xfer,
		.fixup = NULL,
	},

	/*
	 * Orange Pi Zero
	 * Notes:
	 *   - SPI h/w CSN line is used, no need to toggle GPIO line
	 */
	{

		.name = "orange-pi-zero",
		.desc = "Orange Pi Zero ",
		.pin_ce_dflt = 10,
		.pin_ce_name = "gpio10",
		.pin_csn_dflt = 0,
		.pin_csn_name = NULL,
		.pin_irq_dflt = 0,
		.pin_irq_edge_dflt = 0,
		.pin_irq_name = NULL,
		.delay_ms = f_delay_ms,
		.delay_us = f_delay_us,
		.csn = NULL,
		.ce = f_ce,
		.spi_sb_xfer = NULL,
		.spi_mb_xfer = f_spi_mb_xfer,
		.fixup = NULL,
	},

	/*
	 * pcDuino Lite WiFi and Wireless Gate Shield v1.0b
	 * Notes:
	 *   - upstream Linux kernel and U-Boot
	 */
	{

		.name = "pcduino-lite-wifi",
		.desc = "pcDuino WiFi Lite board with Wireless Gate Shiled v 1.0b",
		.pin_ce_dflt = 233,
		.pin_ce_name = "gpio233",
		.pin_csn_dflt = 234,
		.pin_csn_name = "gpio234",
		.pin_irq_dflt = 0,
		.pin_irq_edge_dflt = 0,
		.pin_irq_name = NULL,
		.delay_ms = f_delay_ms,
		.delay_us = f_delay_us,
		.csn = f_csn,
		.ce = f_ce,
		.spi_sb_xfer = f_spi_sb_xfer,
		.spi_mb_xfer = NULL,
		.fixup = pcduino_upstream_fixup,
	},

	/*
	 * pcDuino Lite WiFi and Wireless Gate Shield v1.0b
	 * Notes:
	 *   - legacy sunxi Linux kernel v3.4 and legacy sunxy U-Boot
	 *   - spidev mode should be cusom sunxi SPI_LOOP
	 */
	{

		.name = "pcduino-lite-wifi-legacy",
		.desc = "pcDuino WiFi Lite board with Wireless Gate Shiled v 1.0b",
		.pin_ce_dflt = 7,
		.pin_ce_name = "gpio7_ph9",
		.pin_csn_dflt = 8,
		.pin_csn_name = "gpio8_ph10",
		.pin_irq_dflt = 0,
		.pin_irq_edge_dflt = 0,
		.pin_irq_name = NULL,
		.delay_ms = f_delay_ms,
		.delay_us = f_delay_us,
		.csn = f_csn,
		.ce = f_ce,
		.spi_sb_xfer = f_spi_sb_xfer,
		.spi_mb_xfer = NULL,
		.fixup = pcduino_legacy_fixup,
	},

	/*
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
	{
		.name = "beaglebone-black",
		.desc = "BeagleBone Black",
		.pin_ce_dflt = 48,
		.pin_ce_name = "gpio48",
		.pin_csn_dflt = 49,
		.pin_csn_name = "gpio49",
		.pin_irq_dflt = 0,
		.pin_irq_edge_dflt = 0,
		.pin_irq_name = NULL,
		.delay_ms = f_delay_ms,
		.delay_us = f_delay_us,
		.csn = f_csn,
		.ce = f_ce,
		.spi_sb_xfer = f_spi_sb_xfer,
		.spi_mb_xfer = NULL,
		.fixup = NULL,
	},
};

/* */

int nrf24_driver_setup(struct rf24 *pnrf, void *data)
{
	struct cfg_platform *pcfg = (struct cfg_platform *)data;
	struct sbc_handler *board = NULL;
	char *name;
	int ret;
	int i;

	/* find sbc handler */

	for (i = 0; i < sizeof(boards) / sizeof(boards[0]); i++) {
		name = boards[i].name;
		if (0 == strncmp(name, pcfg->name, strlen(name))) {
			board = &boards[i];
			break;
		}
	}

	if (!board) {
		printf("ERR: can't find board\n");
		return -ENOENT;
	}

	printf("Board: %s (%s)\n", board->name, board->desc);

	/* rf24 ops */

	pnrf->delay_ms = board->delay_ms;
	pnrf->delay_us = board->delay_us;
	pnrf->csn = board->csn;
	pnrf->ce = board->ce;
	pnrf->spi_xfer = board->spi_sb_xfer;
	pnrf->spi_multi_xfer = board->spi_mb_xfer;

	/* gpio: ce */

	if (pcfg->pin_ce && pcfg->pin_ce_name) {
		ret = gpio_setup(pcfg->pin_ce, pcfg->pin_ce_name, DIR_OUT, EDGE_NONE);
		if (ret) {
			printf("ERR: pin setup failed for CE(%d, %s)\n",
				pcfg->pin_ce, pcfg->pin_ce_name);
			goto out;
		}

		pin_ce_name = strdup(pcfg->pin_ce_name);

	} else if (board->pin_ce_dflt && board->pin_ce_name) {
		ret = gpio_setup(board->pin_ce_dflt, board->pin_ce_name, DIR_OUT, EDGE_NONE);
		if (ret) {
			printf("ERR: pin setup failed for CE(%d, %s)\n",
				board->pin_ce_dflt, board->pin_ce_name);
			goto out;
		}

		pin_ce_name = strdup(board->pin_ce_name);

	} else {
		/* CE is not needed */
	}

	/* gpio: csn */

	if (pcfg->pin_csn && pcfg->pin_csn_name) {
		ret = gpio_setup(pcfg->pin_csn, pcfg->pin_csn_name, DIR_OUT, EDGE_NONE);
		if (ret) {
			printf("ERR: pin setup failed for CSN(%d, %s)\n",
				pcfg->pin_csn, pcfg->pin_csn_name);
			goto out;
		}

		pin_csn_name = strdup(pcfg->pin_csn_name);

	} else if (board->pin_csn_dflt && board->pin_csn_name) {
		ret = gpio_setup(board->pin_csn_dflt, board->pin_csn_name, DIR_OUT, EDGE_NONE);
		if (ret) {
			printf("ERR: pin setup failed for CSN(%d, %s)\n",
				board->pin_csn_dflt, board->pin_csn_name);
			goto out;
		}

		pin_csn_name = strdup(board->pin_csn_name);

	} else {
		/* CSN is not needed */
	}

	/* gpio: irq */

	if (pcfg->pin_irq && pcfg->pin_irq_name) {
		ret = gpio_setup(pcfg->pin_irq, pcfg->pin_irq_name, DIR_IN, pcfg->pin_irq_edge);
		if (ret) {
			printf("ERR: pin setup failed for IRQ(%d, %s, %d)\n",
				pcfg->pin_irq, pcfg->pin_irq_name, pcfg->pin_irq_edge);
			goto out;
		}

		pin_irq_name = strdup(pcfg->pin_irq_name);

	} else if (board->pin_irq_dflt && board->pin_irq_name) {
		ret = gpio_setup(board->pin_irq_dflt, board->pin_irq_name, DIR_IN, board->pin_irq_edge_dflt);
		if (ret) {
			printf("ERR: pin setup failed for IRQ(%d, %s, %d)\n",
				board->pin_irq_dflt, board->pin_irq_name, board->pin_irq_edge_dflt);
			goto out;
		}

		pin_irq_name = strdup(board->pin_irq_name);

	} else {
		/* IRQ is not needed */
	}

	/* spidev */

	ret = spi_open(pcfg->spidev);
	if (ret)
		goto out;

	ret = spi_info();
	if (ret)
		goto out;

	ret = spi_init(pcfg->speed, pcfg->mode, pcfg->bits, pcfg->lsb);
	if (ret)
		goto out;

	ret = spi_info();
	if (ret)
		goto out;

	/* fixup */

	if (board->fixup) {
		ret = board->fixup();
		if (ret)
			goto out;
	}

out:
	return ret;
}
