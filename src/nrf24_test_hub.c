#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "RF24.h"
#include "drv.h"
#include "proto/msg.pb-c.h"

/* */

static struct rf24 nrf;

/* */

void nrf24_test_usage(char *name)
{
	printf("usage: %s [-h] -d <spidev>\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-d, --device <spidev>", "spidev for nRF24L01, default is '/dev/spidev0.0");
	printf("%-30s%s\n", "-c, --channel <num>", "set channel number: 0 .. 127");
	printf("%-30s%s\n", "-r, --rate <rate>", "set data rate: 0(1M), 1(2M), 2(250K)");
	printf("%-30s%s\n", "-e, --crc <mode>", "set CRC encoding scheme: 0(none), 1 (8 bits), 2(16 bits)");
	printf("%-30s%s\n", "-p, --power <level>", "set TX output power: 0(-18dBm), 1(-12dBm), 2(-6dBm), 3(0dBm)");
	printf("%-30s%s\n", "--dynamic-payload", "enable dynamic payload support");
	printf("%-30s%s\n", "--payload-length <length>", "set static payload length to 0..32 bytes (default value is 32)");
	printf("%-30s%s\n", "--parse-message", "parse messages according to protobuf description");
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

void decode_data(char *b, int n)
{
	NodeSensorList *msg;
	int i;

	msg = node_sensor_list__unpack(NULL, n, b);

	if (msg == NULL) {
		printf("error: can't unpack message\n");
		return;
	}

	printf("node = %lu\n", msg->node->node);
	for (i = 0; i < msg->n_sensor; i++) {
		printf("sensor[%lu] = %lu\n", msg->sensor[i]->type, msg->sensor[i]->data);
	}

	node_sensor_list__free_unpacked(msg, NULL);

	return;
}

/* */

int main(int argc, char *argv[])
{
	uint8_t addr0[] = {'E', 'F', 'C', 'L', 'I'};
	uint8_t addr1[] = {'E', 'F', 'S', 'N', '1'};

	bool dynamic_payload = false;
	bool parse_message = false;

	uint8_t recv_buffer[32];
	int recv_length = 32;

	enum rf24_rx_status ret;
	struct rf24 *pnrf;
	int pipe;

	/* use sane nRF24 defaults */

	int channel = 76;
	int rate = RF24_RATE_1M;
	int crc = RF24_CRC_16_BITS;
	int power = RF24_PA_MAX;

	/* command line options */

	char *spidev_name = "/dev/spidev0.0";

	int opt;
	const char opts[] = "p:e:r:c:d:h";
	const struct option longopts[] = {
		{"device", required_argument, NULL, 'd'},
		{"channel", required_argument, NULL, 'c'},
		{"rate", required_argument, NULL, 'r'},
		{"crc", required_argument, NULL, 'e'},
		{"power", required_argument, NULL, 'p'},
		{"dynamic-payload", no_argument, NULL, '0'},
		{"payload-length", required_argument, NULL, '1'},
		{"parse-message", no_argument, NULL, '2'},
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
			case '2':
				parse_message = true;
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
	rf24_set_data_rate(pnrf, rate);
	rf24_set_crc_mode(pnrf, crc);
	rf24_set_pa_level(pnrf, power);

	rf24_setup_prx(pnrf, 0x0 /* pipe number */, addr0);
	rf24_setup_prx(pnrf, 0x1 /* pipe number */, addr1);

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

		if (parse_message)
			decode_data((char *)recv_buffer, recv_length);
		else
			dump_data((char *)recv_buffer, recv_length);
	}
}
