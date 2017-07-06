#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "RF24.h"
#include "drv.h"

/* */

static struct rf24 nrf;

/* */

void nrf24_test_usage(char *name)
{
	printf("usage: %s [-h] -c /path/to/config\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-c, --config </path/to/config/file>", "config file");
}

void dump_data(uint8_t *b, int n)
{
	int p;

	for(p = 0; p < n; p++) {
		printf("0x%02x ", *(b + p));
		if ((p > 0) && ((p % 64) == 0))
			printf("\n");
	}

	printf("\n");
}

/* */

int main(int argc, char *argv[])
{
	enum rf24_rx_status ret;
	uint8_t recv_buffer[32];
	int recv_length = 32;
	struct rf24 *pnrf;
	int pipe;
	int tmp;
	int rc;

	struct cfg_platform pconf = {0};
	struct cfg_radio rconf = {0};
	char *config_name = DEFAULT_CONFIG;

	/* command line options */

	int opt;
	const char opts[] = "c:h";
	const struct option longopts[] = {
		{"config", required_argument, NULL, 'c'},
		{"help", optional_argument, NULL, 'h'},
		{NULL,}
	};

	/* use sane config defaults */

	cfg_radio_init(&rconf);
	cfg_platform_init(&pconf);

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
		case 'c':
			config_name = strdup(optarg);
			break;
		case 'h':
		default:
			nrf24_test_usage(argv[0]);
			exit(0);
		}
	}

	/* read and validate config */

	rc = cfg_from_file(config_name);
	if (rc < 0) {
		printf("ERR: failed to parse config\n");
		exit(-1);
	}

	rc = cfg_radio_read(&rconf);
	if (rc < 0) {
		printf("ERR: failed to get radio config\n");
		exit(-1);
	}

	rc = cfg_platform_read(&pconf);
	if (rc < 0) {
		printf("ERR: failed to get platform config\n");
		exit(-1);
	}

	cfg_platform_dump(&pconf);
	cfg_radio_dump(&rconf);

	rc = cfg_radio_validate(&rconf);
	if (rc < 0) {
		printf("ERR: invalid radio config\n");
		exit(-1);
	}

	/* setup nRF24 driver */

	pnrf = &nrf;
	memset(pnrf, 0x0, sizeof(*pnrf));

	if (0 > nrf24_driver_setup(pnrf, (void *)&pconf)) {
		printf("ERR: can't setup driver for nrf24 radio\n");
		exit(-1);
	}

	/* setup nRF24L01 */

	rf24_init(pnrf);
	rf24_print_status(pnrf);

	/* modify default settings */

	if (cfg_payload_is_dynamic(&rconf))
		rf24_enable_dyn_payload(pnrf);
	else
		rf24_set_payload_size(pnrf, rconf.payload);

	rf24_set_channel(pnrf, rconf.channel);
	tmp = rf24_get_channel(pnrf);
	if (tmp != rconf.channel) {
		printf("couldn't set channel: expected %d actual %d\n",
			rconf.channel, tmp);
		exit(-1);
	}

	rf24_set_data_rate(pnrf, rconf.rate);
	tmp = rf24_get_data_rate(pnrf);
	if (tmp != rconf.rate) {
		printf("couldn't set data rate: expected %d actual %d\n",
			rconf.rate, tmp);
		exit(-1);
	}

	rf24_set_crc_mode(pnrf, rconf.crc);
	tmp = rf24_get_crc_mode(pnrf);
	if (tmp != rconf.crc) {
		printf("couldn't set CRC mode: expected %d actual %d\n",
			rconf.crc, tmp);
		exit(-1);
	}

	rf24_set_pa_level(pnrf, rconf.pwr);
	tmp = rf24_get_pa_level(pnrf);
	if (tmp != rconf.pwr) {
		printf("couldn't set TX power level: expected %d actual %d\n",
			rconf.pwr, tmp);
		exit(-1);
	}

	/* start PRX mode */

	if (rconf.pipe[0]) {
		rf24_setup_prx(pnrf, 0x0, rconf.pipe[0]);
	} else {
		printf("pipe0 addr is not set in configuration file\n");
		exit(-1);
	}

	rf24_start_prx(pnrf);
	rf24_print_status(pnrf);

	/* */

	while (1) {

		while(!rf24_rx_ready(pnrf, &pipe))
			usleep(100000);

		ret = rf24_rx_pipe_check(pnrf, pipe);

		if (ret != RF24_RX_OK) {
			printf("WARN: pipe check error 0x%02x\n", (int)pipe);
			rf24_flush_rx(pnrf);
			continue;
		}

		printf("INFO: data ready in pipe 0x%02x\n", pipe);
		memset(recv_buffer, 0x0, sizeof(recv_buffer));

		if (rf24_is_dyn_payload(pnrf)) {
			recv_length = (int)rf24_get_dyn_payload_size(pnrf);
			if (recv_length == 0xff) {
				printf("WARN: failed to get dynamic payload length\n");
				rf24_flush_rx(pnrf);
				continue;
			}
		}

		ret = rf24_recv(pnrf, recv_buffer, recv_length);
		if (ret != RF24_RX_OK) {
			printf("WARN: failed to receive rx data: 0x%02x\n", ret);
			rf24_flush_rx(pnrf);
			continue;
		}

		dump_data(recv_buffer, recv_length);
	}
}
