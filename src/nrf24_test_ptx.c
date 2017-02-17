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
	printf("%-30s%s\n", "-r, --rate <rate>", "set data rate: 0 - 1M, 1 - 2M, 2 - 250K");
	printf("%-30s%s\n", "-p, --power <level>", "set TX output power: 0(-18dBm), 1(-12dBm), 2(-6dBm), 3(0dBm)");
	printf("%-30s%s\n", "--dynamic-payload", "enable dynamic payload support");
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
	int tmp;

	/* use sane defaults */
	int channel = 76;
	int power = RF24_PA_MAX;
	int rate = RF24_RATE_1M;

	/* command line options */

	char *spidev_name = "/dev/spidev0.0";

	int opt;
	const char opts[] = "p:r:c:d:h";
	const struct option longopts[] = {
		{"device", required_argument, NULL, 'd'},
		{"channel", required_argument, NULL, 'c'},
		{"rate", required_argument, NULL, 'r'},
		{"power", required_argument, NULL, 'p'},
		{"dynamic-payload", no_argument, NULL, '0'},
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
			case 'r':
				rate = atoi(optarg);
				if ((rate < 0) || (rate > 2)) {
					printf("ERR: invalid rate %d\n", rate);
					exit(-1);
				}
				break;
			case 'p':
				power = atoi(optarg);
				if ((power < 0) || (power > 3)) {
					printf("ERR: invalid power %d\n", power);
					exit(-1);
				}
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

	/* modify default settings */

	if (dynamic_payload)
		rf24_enable_dyn_payload(pnrf);
	else
		rf24_set_payload_size(pnrf, sizeof(send_buffer));

	rf24_set_channel(pnrf, channel);
	/* TODO: read and check channel */

	rf24_set_data_rate(pnrf, rate);
	tmp = rf24_get_data_rate(pnrf);
	if (tmp != rate) {
		printf("couldn't set data rate: expected %d actual %d\n", rate, tmp);
		exit(-1);
	}

	rf24_set_pa_level(pnrf, power);
	tmp = rf24_get_pa_level(pnrf);
	if (tmp != power) {
		printf("couldn't set TX power level: expected %d actual %d\n", power, tmp);
		exit(-1);
	}

	/* start PTX mode */

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
