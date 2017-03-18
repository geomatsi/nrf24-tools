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
	printf("%-30s%s\n", "-a, --address XX:XX:XX:XX:XX", "pipe 0 PTX address");
	printf("%-30s%s\n", "-d, --device <spidev>", "spidev for nRF24L01, default is '/dev/spidev0.0");
	printf("%-30s%s\n", "-c, --channel <num>", "set channel number: 0 .. 127");
	printf("%-30s%s\n", "-r, --rate <rate>", "set data rate: 0(1M), 1(2M), 2(250K)");
	printf("%-30s%s\n", "-e, --crc <mode>", "set CRC encoding scheme: 0(none), 1 (8 bits), 2(16 bits)");
	printf("%-30s%s\n", "-p, --power <level>", "set TX output power: 0(-18dBm), 1(-12dBm), 2(-6dBm), 3(0dBm)");
	printf("%-30s%s\n", "-m, --message <messge>", "set TX message");
	printf("%-30s%s\n", "--dynamic-payload", "enable dynamic payload support");
	printf("%-30s%s\n", "--payload-length <length>", "set static payload length to 0..32 bytes (default value is 32)");
}

/* */

int main(int argc, char *argv[])
{
	uint8_t addr[] = {0xE1, 0xE1, 0xE1, 0xE1, 0xE1};
	uint8_t send_buffer[32] = { 0 };
	int send_length = 32;
	bool dynamic_payload = false;
	enum rf24_tx_status ret;
	unsigned int digit;
	struct rf24 *pnrf;
	char *sa;
	char *sb;
	int tmp;
	int i = 0;

	/* use sane defaults */
	int channel = 76;
	int rate = RF24_RATE_1M;
	int crc = RF24_CRC_16_BITS;
	int power = RF24_PA_MAX;

	/* command line options */

	char *spidev_name = "/dev/spidev0.0";

	int opt;
	const char opts[] = "a:m:e:p:r:c:d:h";
	const struct option longopts[] = {
		{"address", required_argument, NULL, 'a'},
		{"device", required_argument, NULL, 'd'},
		{"channel", required_argument, NULL, 'c'},
		{"rate", required_argument, NULL, 'r'},
		{"crc", required_argument, NULL, 'e'},
		{"power", required_argument, NULL, 'p'},
		{"message", required_argument, NULL, 'm'},
		{"dynamic-payload", no_argument, NULL, '0'},
		{"payload-length", required_argument, NULL, '1'},
		{"help", optional_argument, NULL, 'h'},
		{NULL,}
	};

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
			case 'a':
				sa = strdup(optarg);
				while ((sb = strsep(&sa, ":")) && (i < 5)) {
					tmp = sscanf(sb, "%x", &digit);
					if (digit > 0xff) {
						printf("ERR: invalid pipe address <%s>\n", optarg);
						exit(-1);
					}

					addr[i++] = digit;
				}

				if (i != 5) {
					printf("ERR: invalid pipe address length <%s>\n", optarg);
					exit(-1);
				}

				break;
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
			case 'e':
				crc = atoi(optarg);
				if ((crc < 0) || (crc > 2)) {
					printf("ERR: invalid CRC mode %d\n", crc);
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
			case 'm':
				memcpy(send_buffer, optarg, sizeof(send_buffer) - 1);
				break;
			case '0':
				dynamic_payload = true;
				break;
			case '1':
				send_length = atoi(optarg);
				if ((send_length < 1) || (send_length > 32)) {
					printf("ERR: invalid static payload length %d\n", send_length);
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

	printf("PTX ADDR %x:%x:%x:%x:%x\n", addr[0], addr[1], addr[2], addr[3], addr[4]);

	/* modify default settings */

	if (dynamic_payload)
		rf24_enable_dyn_payload(pnrf);
	else
		rf24_set_payload_size(pnrf, send_length);

	rf24_set_channel(pnrf, channel);
	tmp = rf24_get_channel(pnrf);
	if (tmp != channel) {
		printf("couldn't set channel: expected %d actual %d\n", channel, tmp);
		exit(-1);
	}

	rf24_set_data_rate(pnrf, rate);
	tmp = rf24_get_data_rate(pnrf);
	if (tmp != rate) {
		printf("couldn't set data rate: expected %d actual %d\n", rate, tmp);
		exit(-1);
	}

	rf24_set_crc_mode(pnrf, crc);
	tmp = rf24_get_crc_mode(pnrf);
	if (tmp != crc) {
		printf("couldn't set CRC mode: expected %d actual %d\n", crc, tmp);
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

		ret = rf24_send(pnrf, send_buffer, send_length);
		if (ret != RF24_TX_OK) {
			printf("send error: %d\n", ret);
			rf24_flush_tx(pnrf);
			rf24_flush_rx(pnrf);
		} else {
			printf("written %d bytes\n", send_length);
		}

		sleep(1);
	}

}
