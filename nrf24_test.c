#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "drv.h"
#include "RF24.h"
#include "msg.pb-c.h"

/* */

struct rf24 nrf = {
	.csn = f_csn,
	.ce = f_ce,
	.spi_set_speed = f_spi_set_speed,
	.spi_xfer = f_spi_xfer,
};

/* */

void nrf24_test_usage(char *name)
{
	printf("usage: %s [-h] -d <spidev>\n", name);
	printf("\t-h, --help\t\t\tthis help message\n");
	printf("\t-d, --device <spidev>\t\tspidev for nRF24L01, default is '/dev/spidev0.0\n");
	printf("\t--dynamic-payload\t\tenable dynamic payload support\n");
	printf("\t--payload-length <length>\tset static payload length to 0..32 bytes (default value is 32)\n");
	printf("\t--parse-message\t\t\tparse messages according to protobuf description\n");
	printf("\t--peer1 <addr>\t\t\tnot yet supported\n");
	printf("\t--peer2 <addr>\t\t\tnot yet supported\n");
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
	SensorDataList *msg;
	int i;

	msg = sensor_data_list__unpack(NULL, n, b);

	if (msg == NULL) {
		printf("error: can't unpack message\n");
		return;
	}

	for (i = 0; i < msg->n_sensor; i++) {
		printf("sensor[%lu] = %lu\n",
				msg->sensor[i]->type, msg->sensor[i]->data);
	}

	sensor_data_list__free_unpacked(msg, NULL);

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

	uint32_t more_data;
	uint8_t pipe;

	struct rf24 *pnrf;

	/* command line options */

	char *spidev_name = "/dev/spidev0.0";

	char peer[2][6] = {
		"EFCLI\0",
		"EFSN1\0",
	};

	int opt;
	const char opts[] = "d:h:";
	const struct option longopts[] = {
		{"device", required_argument, NULL, 'd'},
		{"peer1", required_argument, NULL, '0'},
		{"peer2", required_argument, NULL, '1'},
		{"dynamic-payload", no_argument, NULL, '2'},
		{"payload-length", required_argument, NULL, '3'},
		{"parse-message", no_argument, NULL, '4'},
		{"help", optional_argument, NULL, 'h'},
		{NULL,}
	};

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
			case 'd':
				spidev_name = strdup(optarg);
				break;
			case '0':
				if (strlen(optarg) >= 5)
					strncpy(peer[0], optarg, 5);
				break;
			case '1':
				if (strlen(optarg) >= 5)
					strncpy(peer[1], optarg, 5);
				break;
			case '2':
				dynamic_payload = true;
				break;
			case '3':
				recv_length = atoi(optarg);
				if ((recv_length < 1) || (recv_length > 32)) {
					printf("ERR: invalid static payload length %d\n", recv_length);
					nrf24_test_usage(argv[0]);
					exit(-1);
				}
				break;
			case '4':
				parse_message = true;
				break;
			case 'h':
			default:
				nrf24_test_usage(argv[0]);
				exit(0);
		}
	}

	/* setup nRF24 driver */

	if (0 > nrf24_driver_setup(spidev_name)) {
		printf("ERR: can't setup driver for nrf24 radio\n");
		exit(-1);
	}

	/* setup nRF24L01 */

	rf24_init(&nrf);
	pnrf = &nrf;

	rf24_print_status(rf24_get_status(pnrf));

	/* */

	if (dynamic_payload)
		rf24_enable_dynamic_payloads(pnrf);
	else
		rf24_set_payload_size(pnrf, recv_length);

	rf24_open_reading_pipe(pnrf, 0x0 /* pipe number */, addr0);
	rf24_open_reading_pipe(pnrf, 0x1 /* pipe number */, addr1);

	rf24_start_listening(pnrf);
	rf24_print_status(rf24_get_status(pnrf));
	rf24_print_details(pnrf);

	/* */

	while (1) {

		if (rf24_available(pnrf, &pipe)) {

			if ((pipe < 0) || (pipe > 5)) {
				printf("WARN: invalid pipe number 0x%02x\n", (int) pipe);
			} else {
				printf("INFO: data ready in pipe 0x%02x\n", pipe);
				memset(recv_buffer, 0x0, sizeof(recv_buffer));

				if (dynamic_payload)
					recv_length = rf24_get_dynamic_payload_size(pnrf);

				more_data = rf24_read(pnrf, recv_buffer, recv_length);
				printf("INFO: dump received %d bytes\n", recv_length);

				if (parse_message) {
					decode_data((char *)recv_buffer, recv_length);
				} else {
					dump_data((char *)recv_buffer, recv_length);
				}

				if (!more_data)
					printf("WARN: RX_FIFO not empty: %d\n", more_data);
			}
		}

		usleep(100000);
	}
}
