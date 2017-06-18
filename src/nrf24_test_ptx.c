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
	printf("usage: %s [-h] -m <message> -a <addr> -c /path/to/config\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-m, --message <messge>", "set TX message");
	printf("%-30s%s\n", "-a, --address XX:XX:XX:XX:XX", "pipe 0 PTX address");
	printf("%-30s%s\n", "-c, --config </path/to/config/file>", "config file");
}

/* */

int main(int argc, char *argv[])
{
	uint8_t addr[] = {0xE1, 0xE1, 0xE1, 0xE1, 0xE1};
	uint8_t send_buffer[32] = { 0 };
	int send_length = 32;
	enum rf24_tx_status ret;
	unsigned int digit;
	struct rf24 *pnrf;
	char *sa;
	char *sb;
	int tmp;
	int i = 0;
	int rc;

	struct cfg_platform pconf = {0};
	struct cfg_radio rconf = {0};
	char *config_name = NULL;

	/* command line options */

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

	/* use sane config defaults */

	cfg_radio_init(&rconf);
	cfg_platform_init(&pconf);

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
		case 'c':
			config_name = strdup(optarg);
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
			break;
		case 'm':
			memcpy(send_buffer, optarg, sizeof(send_buffer) - 1);
			break;
		case 'h':
		default:
			nrf24_test_usage(argv[0]);
			exit(0);
		}
	}

	/* validate radio settings */

	cfg_radio_dump(&rconf);
	rc = cfg_radio_validate(&rconf);
	if (rc < 0) {
		printf("ERR: invalid radio config\n");
		exit(-1);
	}

	cfg_platform_dump(&pconf);

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

	printf("PTX ADDR %x:%x:%x:%x:%x\n", addr[0], addr[1], addr[2], addr[3], addr[4]);

	/* modify default settings */

	if (cfg_payload_is_dynamic(&rconf))
		rf24_enable_dyn_payload(pnrf);
	else
		rf24_set_payload_size(pnrf, send_length);

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
