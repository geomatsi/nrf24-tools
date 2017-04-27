#include <RF24.h>

#define MAX_CONFIG_SIZE	4096

#define RADIO_TAG	"radio"

#define RADIO_TAG_CHAN	"channel"
#define RADIO_TAG_RATE	"rate"
#define RADIO_TAG_CRC	"crc"
#define RADIO_TAG_PWR	"pwr"

struct radio_conf {
	uint8_t channel;
	uint8_t rate;
	uint8_t crc;
	uint8_t pwr;
};

void init_radio_conf(struct radio_conf *c);
int validate_radio_conf(const struct radio_conf *c);
int read_radio_conf(struct radio_conf *c, const char *path);
