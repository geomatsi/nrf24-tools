#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "RF24.h"
#include "drv.h"

/* */

static struct rf24 nrf;

/* */

void nrf24_test_usage(char *name)
{
	printf("usage: %s [-h] -d <spidev>\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-d, --device <spidev>", "spidev for nRF24L01, default is '/dev/spidev0.0");
	printf("%-30s%s\n", "-c, --channel <num>", "set channel number: 0 .. 127");
	printf("%-30s%s\n", "--dynamic-payload", "enable dynamic payload support");
	printf("%-30s%s\n", "--payload-length <length>", "set static payload length to 0..32 bytes (default value is 32)");
}

void dump_data(char *b, int n)
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
	uint8_t addr[] = {'E', 'F', 'C', 'L', 'I'};

	bool dynamic_payload = false;

	uint8_t recv_buffer[32];
	int recv_length = 32;
	int channel;

	enum rf24_rx_status ret;
	struct rf24 *pnrf;
	int pipe;

	/* command line options */

	char *spidev_name = "/dev/spidev0.0";

	int opt;
	const char opts[] = "c:d:h";
	const struct option longopts[] = {
		{"device", required_argument, NULL, 'd'},
		{"channel", required_argument, NULL, 'c'},
		{"dynamic-payload", no_argument, NULL, '0'},
		{"payload-length", required_argument, NULL, '1'},
		{"help", optional_argument, NULL, 'h'},
		{NULL,}
	};

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
			case 'd':
				spidev_name = strdup(optarg);
				break;
			case 'c':
				channel = atoi(optarg);
				if ((channel < 0) || (channel > 127)) {
					printf("ERR: invalid channel %d\n", channel);
					exit(-1);
				}
				break;
			case '0':
				dynamic_payload = true;
				break;
			case '1':
				recv_length = atoi(optarg);
				if ((recv_length < 1) || (recv_length > 32)) {
					printf("ERR: invalid static payload length %d\n", recv_length);
					exit(-1);
				}
				break;
			case 'h':
			default:
				nrf24_test_usage(argv[0]);
				exit(0);
		}
	}

	/* setup nRF24 driver */

	pnrf = &nrf;
	memset(pnrf, 0x0, sizeof(*pnrf));

	if (0 > nrf24_driver_setup(pnrf, spidev_name)) {
		printf("ERR: can't setup driver for nrf24 radio\n");
		exit(-1);
	}

	/* setup nRF24L01 */

	rf24_init(pnrf);
	rf24_print_status(pnrf);

	/* */

	if (dynamic_payload)
		rf24_enable_dyn_payload(pnrf);
	else
		rf24_set_payload_size(pnrf, recv_length);

	rf24_set_channel(pnrf, channel);

	rf24_setup_prx(pnrf, 0x0 /* pipe number */, addr);
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

		if (dynamic_payload) {
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

		dump_data((char *)recv_buffer, recv_length);
	}
}
