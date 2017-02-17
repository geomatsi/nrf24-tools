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
	printf("\t-h, --help\t\t\tthis help message\n");
	printf("\t-d, --device <spidev>\t\tspidev for nRF24L01, default is '/dev/spidev0.0\n");
	printf("\t--dynamic-payload\t\tenable dynamic payload support\n");
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
	uint8_t send_buffer[] = {0x1, 0x2, 0x3, 0x4, 0x5};
	uint8_t addr[] = {'E', 'F', 'C', 'L', 'I'};

	bool dynamic_payload = false;
	enum rf24_tx_status ret;
	struct rf24 *pnrf;

	/* command line options */

	char *spidev_name = "/dev/spidev0.0";

	int opt;
	const char opts[] = "d:h:";
	const struct option longopts[] = {
		{"device", required_argument, NULL, 'd'},
		{"dynamic-payload", no_argument, NULL, '0'},
		{"help", optional_argument, NULL, 'h'},
		{NULL,}
	};

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
			case 'd':
				spidev_name = strdup(optarg);
				break;
			case '0':
				dynamic_payload = true;
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
		rf24_set_payload_size(pnrf, sizeof(send_buffer));

	rf24_setup_ptx(pnrf, addr);
	rf24_start_ptx(pnrf);
	rf24_print_status(pnrf);

	/* */

	while (1) {

		ret = rf24_send(pnrf, send_buffer, sizeof(send_buffer));
		if (ret != RF24_TX_OK) {
			printf("send error: %d\n", ret);
			rf24_flush_tx(pnrf);
			rf24_flush_rx(pnrf);
		} else {
			printf("written %d bytes\n", sizeof(send_buffer));
		}

		sleep(1);
	}

}
