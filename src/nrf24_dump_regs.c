#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "nRF24L01.h"
#include "RF24.h"
#include "drv.h"

/* */

struct cmd_handler {
	char *cmd;
	char *desc;
	int (*handler)(struct rf24 *pnrf, int argc, char *argv[]);
};

struct reg_handler {
	uint8_t reg;
	char *name;
	char *desc;
	char *(*handler)(struct rf24 *pnrf, struct reg_handler *ph);
};

/* */

char *default_handler(struct rf24 *pnrf, struct reg_handler *ph)
{
	char buffer[64] = { 0 };
	int pos = 0;
	uint8_t val;
	int i;

	val = rf24_get_register(pnrf, ph->reg);
	pos += snprintf(buffer, sizeof(buffer) - 1, "0x%02x ", val);
	for (i = 0; i < 8; i++)
		pos += snprintf(buffer + pos, sizeof(buffer) - pos - 1, "%01d",
			(val & (0x1 << (7 - i))) ? 1 : 0);

	return strdup(buffer);
}

char *addr_handler(struct rf24 *pnrf, struct reg_handler *ph)
{
	char buffer[64] = { 0 };
	uint8_t val;
	int pos = 0;
	int len;
	int i;

	switch (ph->reg) {
		case RX_ADDR_P0:
		case RX_ADDR_P1:
		case TX_ADDR:
			len = rf24_get_register(pnrf, SETUP_AW);
			if ((len >= 1) && (len <= 3)) {
				len += 2;
			} else {
				snprintf(buffer, sizeof(buffer), "invalid SETUP_AW: %d", len);
				break;
			}

			for (i = 0; i < len; i++) {
				val = rf24_get_register(pnrf, ph->reg);
				pos += snprintf(buffer + pos, sizeof(buffer) - pos - 1,
						"0x%02x ", val);
			}
			break;
		case RX_ADDR_P2:
		case RX_ADDR_P3:
		case RX_ADDR_P4:
		case RX_ADDR_P5:
			val = rf24_get_register(pnrf, ph->reg);
			snprintf(buffer, sizeof(buffer), "0x%02x", val);
			break;
		default:
			snprintf(buffer, sizeof(buffer), "register %02x is not addr", ph->reg);
			break;
	}

	return strdup(buffer);
}

struct reg_handler nrf24_regs[] = {
	{
		.reg = 0x00,
		.name = "CONFIG",
		.desc = "Configuration register",
		.handler = default_handler,
	},
	{
		.reg = 0x01,
		.name = "EN_AA",
		.desc = "Enable 'Auto Acknowledgement' function",
		.handler = default_handler,
	},
	{
		.reg = 0x02,
		.name = "EN_RXADDR",
		.desc = "Enabled RX addresses",
		.handler = default_handler,
	},
	{
		.reg = 0x03,
		.name = "SETUP_AW",
		.desc = "Setup of address  widths (common to all data pipes)",
		.handler = default_handler,
	},
	{
		.reg = 0x04,
		.name = "SETUP_RETR",
		.desc = "Setup of automatic retransmission",
		.handler = default_handler,
	},
	{
		.reg = 0x05,
		.name = "RF_CH",
		.desc = "RF channel",
		.handler = default_handler,
	},
	{
		.reg = 0x06,
		.name = "RF_SETUP",
		.desc = "RF setup register",
		.handler = default_handler,
	},
	{
		.reg = 0x07,
		.name = "STATUS",
		.desc = "Status register",
		.handler = default_handler,
	},
	{
		.reg = 0x08,
		.name = "OBSERVE_TX",
		.desc = "Transmit observe register",
		.handler = default_handler,
	},
	{
		.reg = 0x09,
		.name = "RPD",
		.desc = "Received power detector/Carrier detector",
		.handler = default_handler,
	},
	{
		.reg = 0x0A,
		.name = "RX_ADDR_P0",
		.desc = "Receive address data pipe 0, 5 bytes maximum length",
		.handler = addr_handler,
	},
	{
		.reg = 0x0B,
		.name = "RX_ADDR_P1",
		.desc = "Receive address data pipe 1, 5 bytes maximum length",
		.handler = addr_handler,
	},
	{
		.reg = 0x0C,
		.name = "RX_ADDR_P2",
		.desc = "Receive address data pipe 2, only LSB",
		.handler = addr_handler,
	},
	{
		.reg = 0x0D,
		.name = "RX_ADDR_P3",
		.desc = "Receive address data pipe 3, only LSB",
		.handler = addr_handler,
	},
	{
		.reg = 0x0E,
		.name = "RX_ADDR_P4",
		.desc = "Receive address data pipe 4, only LSB",
		.handler = addr_handler,
	},
	{
		.reg = 0x0F,
		.name = "RX_ADDR_P5",
		.desc = "Receive address data pipe 5, only LSB",
		.handler = addr_handler,
	},
	{
		.reg = 0x10,
		.name = "TX_ADDR",
		.desc = "Transmit address, used for a PTX device only",
		.handler = addr_handler,
	},
	{
		.reg = 0x11,
		.name = "RX_PW_P0",
		.desc = "Number of bytes in RX payload in data pipe 0 (1 .. 32 bytes)",
		.handler = default_handler,
	},
	{
		.reg = 0x12,
		.name = "RX_PW_P1",
		.desc = "Number of bytes in RX payload in data pipe 1 (1 .. 32 bytes)",
		.handler = default_handler,
	},
	{
		.reg = 0x13,
		.name = "RX_PW_P2",
		.desc = "Number of bytes in RX payload in data pipe 2 (1 .. 32 bytes)",
		.handler = default_handler,
	},
	{
		.reg = 0x14,
		.name = "RX_PW_P3",
		.desc = "Number of bytes in RX payload in data pipe 3 (1 .. 32 bytes)",
		.handler = default_handler,
	},
	{
		.reg = 0x15,
		.name = "RX_PW_P4",
		.desc = "Number of bytes in RX payload in data pipe 4 (1 .. 32 bytes)",
		.handler = default_handler,
	},
	{
		.reg = 0x16,
		.name = "RX_PW_P5",
		.desc = "Number of bytes in RX payload in data pipe 5 (1 .. 32 bytes)",
		.handler = default_handler,
	},
	{
		.reg = 0x17,
		.name = "FIFO_STATUS",
		.desc = "FIFO status register",
		.handler = default_handler,
	},
	{
		.reg = 0x1C,
		.name = "DYNPD",
		.desc = "Enable dynamic payload length",
		.handler = default_handler,
	},
	{
		.reg = 0x1D,
		.name = "FEATURE",
		.desc = "Feature register",
		.handler = default_handler,
	},
};

/* */

int nrf24_detect_model(struct rf24 *pnrf, int argc, char *argv[])
{
	enum rf24_data_rate rate;

	rf24_set_data_rate(pnrf, RF24_RATE_250K);
	rate = rf24_get_data_rate(pnrf);
	switch (rate) {
		case RF24_RATE_250K:
			printf("model: nRF24L01+\n");
			break;
		default:
			printf("model: nRF24L01, no support for 250K rate\n");
			break;
	}

	return 0;
}

int nrf24_list_all_regs(struct rf24 *pnrf, int argc, char *argv[])
{
	struct reg_handler *ph;
	int i;

	for (i = 0; i < sizeof(nrf24_regs) / sizeof(struct reg_handler); i++) {
		ph = &nrf24_regs[i];
		printf("%-15s: %s\n", ph->name, ph->desc);
	}

	return 0;
}

int nrf24_dump_all_regs(struct rf24 *pnrf, int argc, char *argv[])
{
	struct reg_handler *ph;
	int i;

	for (i = 0; i < sizeof(nrf24_regs) / sizeof(struct reg_handler); i++) {
		ph = &nrf24_regs[i];
		printf("%-15s: %s\n", ph->name, ph->handler(pnrf, ph));
	}

	return 0;
}

int nrf24_dump_regs(struct rf24 *pnrf, int argc, char *argv[])
{
	struct reg_handler *ph;
	char *name;
	char *reg;
	int i;

	if (!argc || !argv[0])
		return 0;

	while ((reg = strsep(&argv[0], ","))) {
		for (i = 0; i < sizeof(nrf24_regs) / sizeof(struct reg_handler); i++) {
			name = nrf24_regs[i].name;
			if (0 == strncmp(reg, name, strlen(name))) {
				ph = &nrf24_regs[i];
				printf("%-15s: %s\n", ph->name, ph->handler(pnrf, ph));
				break;
			}
		}
	}

	return 0;
}

/* */

struct cmd_handler commands[] = {
	{
		.cmd = "model",
		.desc = "detect nRF24x model",
		.handler = nrf24_detect_model,
	},
	{
		.cmd = "list",
		.desc = "list available registers",
		.handler = nrf24_list_all_regs,
	},
	{
		.cmd = "all",
		.desc = "dump all registers",
		.handler = nrf24_dump_all_regs,
	},
	{
		.cmd = "regs",
		.desc = "dump specified subset of registers: regs r1,r2,r3",
		.handler = nrf24_dump_regs,
	},
};

/* */

void nrf24_dump_usage(char *name)
{
	int i;

	printf("usage: %s [-h] -d <spidev> <cmd> <cmd args>\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-d, --device <spidev>", "spidev for nRF24x, default is '/dev/spidev0.0");
	printf("commands:\n");

	for (i = 0; i < sizeof(commands) / sizeof(struct cmd_handler); i++)
		printf("%-5s%-15s%s\n", "", commands[i].cmd, commands[i].desc);
}


/* */

int main(int argc, char *argv[])
{
	struct rf24 *pnrf;
	struct rf24 nrf;

	char *spidev = "/dev/spidev0.0";
	char *cmd;
	int i;

	int opt;
	const char opts[] = "d:h";
	const struct option longopts[] = {
		{"device", required_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{NULL,}
	};

	/* parse command line */

	if (argc <= 1) {
		nrf24_dump_usage(argv[0]);
		exit(0);
	}

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
			case 'd':
				spidev = strdup(optarg);
				break;
			case 'h':
			default:
				nrf24_dump_usage(argv[0]);
				exit(0);
		}
	}

	/* setup nRF24L01 */

	pnrf = &nrf;
	memset(pnrf, 0x0, sizeof(*pnrf));

	if (0 > nrf24_driver_setup(pnrf, spidev)) {
		printf("ERR: can't setup gpio\n");
		exit(-1);
	}

	rf24_init(pnrf);

	/* execute command */

	for (i = 0; i < sizeof(commands) / sizeof(struct cmd_handler); i++) {
		cmd = commands[i].cmd;
		if (0 == strncmp(cmd, argv[optind], strlen(cmd))) {
			commands[i].handler(pnrf, argc - optind - 1, &argv[optind + 1]);
			break;
		}
	}

	return 0;
}
