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

#include "proto/msg.proto.h"

/* */

static struct rf24 nrf;

/* */

void nrf24_test_usage(char *name)
{
	printf("usage: %s [-h] [-p] -c /path/to/config\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-c, --config /path/to/config/file", "config file");
	printf("%-30s%s\n", "-p, --parse-message", "parse messages according to protobuf description");
}

void dump_data(uint8_t *b, int n)
{
	int p;

	for (p = 0; p < n; p++) {
		printf("0x%02x ", *(b + p));
		if ((p > 0) && ((p % 64) == 0))
			printf("\n");
	}

	printf("\n");
}

void decode_data(uint8_t *b, int n)
{
	NodeSensorList *msg;
	int i;

	msg = node_sensor_list__unpack(NULL, n, b);

	if (msg == NULL) {
		printf("error: can't unpack message\n");
		return;
	}

	printf("node = %u\n", msg->node->node);
	for (i = 0; i < msg->n_sensor; i++) {
		printf("sensor[0x%x] = %u\n", msg->sensor[i]->type, msg->sensor[i]->data);
	}

	node_sensor_list__free_unpacked(msg, NULL);

	return;
}

/* */

int main(int argc, char *argv[])
{
	bool parse_message = false;

	uint8_t recv_buffer[32];
	enum rf24_rx_status ret;
	int recv_length = 32;
	struct rf24 *pnrf;
	int pipe;
	int rc;
	int i;

	struct cfg_platform pconf = {0};
	struct cfg_radio rconf = {0};
	char *config_name = DEFAULT_CONFIG;

	/* command line options */

	int opt;
	const char opts[] = "c:ph";
	const struct option longopts[] = {
		{"config", required_argument, NULL, 'c'},
		{"parse-message", no_argument, NULL, 'p'},
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
		case 'p':
			parse_message = true;
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

	/* */

	if (cfg_payload_is_dynamic(&rconf))
		rf24_enable_dyn_payload(pnrf);
	else
		rf24_set_payload_size(pnrf, rconf.payload);

	rf24_set_channel(pnrf, rconf.channel);
	rf24_set_data_rate(pnrf, rconf.rate);
	rf24_set_crc_mode(pnrf, rconf.crc);
	rf24_set_pa_level(pnrf, rconf.pwr);

	for (i = 0; i < PIPE_MAX_NUM; i++) {
		if (rconf.pipe[i])
			rf24_setup_prx(pnrf, i, rconf.pipe[i]);
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

		if (parse_message)
			decode_data(recv_buffer, recv_length);
		else
			dump_data(recv_buffer, recv_length);
	}
}
