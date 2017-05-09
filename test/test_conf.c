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
	struct cfg_platform pconf;
	struct cfg_radio rconf;
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

	/* init default radio config */
	cfg_radio_init(&rconf);
	ret = cfg_radio_validate(&rconf);
	printf("default radio config validation %s\n",
		(ret < 0) ? "FAILED" : "PASSED");
	cfg_radio_dump(&rconf);

	/* init default radio config */
	cfg_platform_init(&pconf);
	cfg_platform_dump(&pconf);

	/* open and parse config */
	ret = cfg_init(name);
	if (ret < 0) {
		printf("failed to read config\n");
		exit(ret);
	}

	/* read and validate radio config */
	cfg_radio_read(&rconf);
	ret = cfg_radio_validate(&rconf);
	printf("new radio config validation %s\n",
		(ret < 0) ? "FAILED" : "PASSED");
	cfg_radio_dump(&rconf);

	/* read platform config */
	cfg_platform_read(&pconf);
	cfg_platform_dump(&pconf);

	return 0;
}
