#include <RF24.h>

struct radio_conf {
	uint8_t channel;
	uint8_t rate;
	uint8_t crc;
	uint8_t pwr;
};

void init_radio_conf(struct radio_conf *conf);
int validate_radio_conf(const struct radio_conf *conf);
int read_radio_conf(const char *path);
