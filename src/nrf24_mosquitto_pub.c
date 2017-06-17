#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>

#include "config.h"
#include "RF24.h"
#include "drv.h"

#include "proto/msg.proto.h"

/* */

void mqtt_callback_log(struct mosquitto *mqtt, void *user, int level, const char *str)
{
	printf("INFO MOSQUITTO<%d> %s\n", level, str);
}

void mqtt_callback_publish(struct mosquitto *mqtt, void *user, int mid)
{
	printf("INFO MOSQUITTO: message %d published\n", mid);
}

void mqtt_callback_connect(struct mosquitto *mqtt, void *user, int result)
{
	printf("INFO MOSQUITTO: connection response %d\n", result);
}

/* */

static struct rf24 nrf;

/* */

void nrf24_test_usage(char *name)
{
	printf("usage: %s [-h] -d <spidev>\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-C, --config </path/to/config/file>", "config file");
	printf("%-30s%s\n", "-a, --address <addrs>", "PRX pipes addresses in the form XX:XX:XX:XX:XX[,XX:XX:XX:XX:XX[,XX[,XX[,XX[,XX]]]]]");
	printf("%-30s%s\n", "-d, --device <spidev>", "spidev for nRF24L01, default is '/dev/spidev0.0");
	printf("%-30s%s\n", "-c, --channel <num>", "set channel number: 0 .. 127");
	printf("%-30s%s\n", "-r, --rate <rate>", "set data rate: 0(1M), 1(2M), 2(250K)");
	printf("%-30s%s\n", "-e, --crc <mode>", "set CRC encoding scheme: 0(none), 1 (8 bits), 2(16 bits)");
	printf("%-30s%s\n", "-p, --power <level>", "set TX output power: 0(-18dBm), 1(-12dBm), 2(-6dBm), 3(0dBm)");
	printf("%-30s%s\n", "--payload-length <length>", "0 - dynamic payload, 1 .. 32 - static payload length");
	printf("%-30s%s\n", "--publish-message", "publish messages to MQTT broker");
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

void publish_data(struct mosquitto *m, uint8_t *b, int n)
{
	char mqtt_message[64];
	char mqtt_topic[64];
	NodeSensorList *msg;
	int i;

	msg = node_sensor_list__unpack(NULL, n, b);

	if (msg == NULL) {
		printf("error: can't unpack message\n");
		return;
	}

	printf("node = %u\n", msg->node->node);
	for (i = 0; i < msg->n_sensor; i++) {
		memset(mqtt_message, 0x0, sizeof(mqtt_message));
		memset(mqtt_topic, 0x0, sizeof(mqtt_topic));
		printf("sensor[%u] = %u\n", msg->sensor[i]->type, msg->sensor[i]->data);
		snprintf(mqtt_message, sizeof(mqtt_message) - 1, "%u", msg->sensor[i]->data);
		snprintf(mqtt_topic, sizeof(mqtt_message) - 1, "node%u/sensor%u",
			 msg->node->node, msg->sensor[i]->type);
		if(mosquitto_publish(m, NULL, mqtt_topic, strlen(mqtt_message), mqtt_message, 0, 0)) {
			printf("ERR: mosquitto could not publish message\n");
		}
	}

	node_sensor_list__free_unpacked(msg, NULL);

	return;
}

/* */

int main(int argc, char *argv[])
{
	uint8_t pipe_addr[6][5] = {
		{ 0xE1, 0xE1, 0xE1, 0xE1, 0xE1 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00 },
	};

	/* by default only the first pipe is enabled */
	uint8_t pipe_en[6] = {1, 0, 0, 0, 0, 0};

	bool publish_message = false;

	uint8_t recv_buffer[32];
	int recv_length;

	enum rf24_rx_status ret;
	struct rf24 *pnrf;
	int pipe;

	char *sa, *sb, *sc, *sd;
	unsigned int digit;
	int i, j, tmp;
	int rc;

	struct cfg_platform pconf = {0};
	struct cfg_radio rconf = {0};
	char *config_name = NULL;

	char *host = "localhost";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;
	struct mosquitto *mqtt = NULL;
	char *mqtt_id = "nrf24hub";

	/* command line options */

	int opt;
	const char opts[] = "a:e:p:r:c:d:C:h";
	const struct option longopts[] = {
		{"config", required_argument, NULL, 'C'},
		{"address", required_argument, NULL, 'a'},
		{"device", required_argument, NULL, 'd'},
		{"channel", required_argument, NULL, 'c'},
		{"rate", required_argument, NULL, 'r'},
		{"crc", required_argument, NULL, 'e'},
		{"power", required_argument, NULL, 'p'},
		{"payload-length", required_argument, NULL, '1'},
		{"publish-message", no_argument, NULL, '2'},
		{"help", optional_argument, NULL, 'h'},
		{NULL,}
	};

	/* use sane config defaults */
	cfg_radio_init(&rconf);
	cfg_platform_init(&pconf);

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
		case 'd':
			pconf.spidev = strdup(optarg);
			break;
		case 'C':
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
		case 'a':
			sa = strdup(optarg);
			i = 0;

			while ((sb = strsep(&sa, ",")) && (i < 6)) {
				sc = strdup(sb);
				pipe_en[i] = 1;
				j = 0;

				while ((sd = strsep(&sc, ":")) && (j < 5)) {
					tmp = sscanf(sd, "%x", &digit);
					if (tmp != 1) {
						printf("ERR: invalid pipe%d address entry <%s>\n",
								i, sd);
						exit(-1);
					}

					if (digit > 0xff) {
						printf("ERR: invalid pipe%d address <%s>\n",
								i, sb);
						exit(-1);
					}

					pipe_addr[i][j] = digit;
					j++;
				}

				/* data pipes 1-5 share 4 most significant bytes */
				if (((i < 2) && (j != 5)) || ((i >= 2) && (j != 1))) {
					printf("ERR: invalid pipe%d address length <%s>\n",
						i, sb);
					exit(-1);
				}

				i++;
			}
		break;
		case 'c':
			rconf.channel = atoi(optarg);
			break;
		case 'r':
			rconf.rate = atoi(optarg);
			break;
		case 'e':
			rconf.crc = atoi(optarg);
			break;
		case 'p':
			rconf.pwr = atoi(optarg);
			break;
		case '1':
			rconf.payload = atoi(optarg);
			break;
		case '2':
			publish_message = true;
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

	/* setup mosquitto */

	mosquitto_lib_init();

	mqtt = mosquitto_new(mqtt_id, clean_session, NULL);
	if (!mqtt) {
		printf("ERR: can not allocate mosquitto context\n");
		exit(-1);
	}

	mosquitto_log_callback_set(mqtt, mqtt_callback_log);
	mosquitto_publish_callback_set(mqtt, mqtt_callback_publish);
	mosquitto_connect_callback_set(mqtt, mqtt_callback_connect);

	if(mosquitto_connect(mqtt, host, port, keepalive)) {
		printf("ERR: can not connect to mosquitto server %s:%d\n", host, port);
		exit(-1);
	}

	if (mosquitto_loop_start(mqtt)) {
		printf("ERR: can not start mosquitto thread\n");
		exit(-1);
	}

	/* setup SPI and GPIO */

	pnrf = &nrf;
	memset(pnrf, 0x0, sizeof(*pnrf));

	if (0 > nrf24_driver_setup(pnrf, (void *)&pconf)) {
		printf("ERR: can't setup gpio\n");
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

	for (i = 0; i < 6; i++) {
		if (pipe_en[i]) {
			/* for pipes 2-5 only last addr byte is passed */
			rf24_setup_prx(pnrf, i, pipe_addr[i]);

			/* data pipes 1-5 share 4 most significant bytes */
			printf("enable pipe%d with address: %02x:%02x:%02x:%02x:%02x\n", i,
				pipe_addr[(i < 2) ? i : 1][0],
				pipe_addr[(i < 2) ? i : 1][1],
				pipe_addr[(i < 2) ? i : 1][2],
				pipe_addr[(i < 2) ? i : 1][3],
				pipe_addr[i][(i < 2) ? 4 : 0]);
		}
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
		} else {
			recv_length = rconf.payload;
		}

		ret = rf24_recv(pnrf, recv_buffer, recv_length);
		if (ret != RF24_RX_OK) {
			printf("WARN: failed to receive rx data: 0x%02x\n", ret);
			rf24_flush_rx(pnrf);
			continue;
		}

		if (publish_message)
			publish_data(mqtt, recv_buffer, recv_length);
		else
			dump_data(recv_buffer, recv_length);
	}
}
