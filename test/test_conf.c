#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


#include "config.h"

void test_conf_usage(char *name)
{
	printf("usage: %s [-h] -c <spidev>\n", name);
	printf("%-30s%s\n", "-h, --help", "this help message");
	printf("%-30s%s\n", "-c, --config <filen name>", "path to config file");
}

int main(int argc, char *argv[])
{
	struct radio_conf rconf;
	char *name = NULL;
	int ret;

	int opt;
	const char opts[] = "c:h";
	const struct option longopts[] = {
		{"config", required_argument, NULL, 'c'},
		{"help", optional_argument, NULL, 'h'},
		{NULL,}
	};

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
		case 'c':
			name = strdup(optarg);
			break;
		case 'h':
		default:
			test_conf_usage(argv[0]);
			exit(0);
		}
	}

	init_radio_conf(&rconf);
	ret = validate_radio_conf(&rconf);
	printf("default radio config validation %s\n",
		(ret < 0) ? "FAILED" : "PASSED");

	read_radio_conf(&rconf, name);
	ret = validate_radio_conf(&rconf);
	printf("new radio config validation %s\n",
		(ret < 0) ? "FAILED" : "PASSED");

	printf("r: chan[%d] rate[%d] crc[%d] pwr[%d]\n",
		rconf.channel, rconf.rate, rconf.crc, rconf.pwr);

	return 0;
}
