#ifndef __CFG_H__
#define __CFG_H__

#include <RF24.h>

#define MAX_CONFIG_SIZE	4096

#define RADIO_TAG	"radio"

#define RADIO_TAG_LEN	"payload"
#define RADIO_TAG_CHAN	"channel"
#define RADIO_TAG_RATE	"rate"
#define RADIO_TAG_CRC	"crc"
#define RADIO_TAG_PWR	"pwr"

#define PLAT_TAG	"platform"

#define PLAT_TAG_NAME	"name"
#define PLAT_TAG_SPIDEV	"spidev"

struct radio_conf {
	uint8_t payload;
	uint8_t channel;
	uint8_t rate;
	uint8_t crc;
	uint8_t pwr;
};

struct plat_conf {
	char *name;
	char *spidev;
};

static inline int payload_is_dynamic(struct radio_conf *c)
{
	return !!(c->payload);
}

static inline void init_radio_conf(struct radio_conf *c)
{
	if (!c)
		return;

	memset(c, 0x0, sizeof(*c));

	c->payload = 32;
	c->channel = 10;
	c->rate = RF24_RATE_1M;
	c->crc = RF24_CRC_16_BITS;
	c->pwr = RF24_PA_MAX;
}

static inline void init_plat_conf(struct plat_conf *c)
{
	if (!c)
		return;

	memset(c, 0x0, sizeof(*c));

	c->name = strdup("default");
	c->spidev = strdup("/dev/spidev0.0");
}

static inline int validate_radio_conf(const struct radio_conf *c)
{
	if ((c->payload < 0) || (c->payload > 32))
		return -1;

	if (c->channel > RF24_MAX_CHANNEL)
		return -1;

	if ((c->rate < RF24_RATE_1M) || (c->rate > RF24_RATE_250K))
		return -1;

	if ((c->crc < RF24_CRC_NONE) || (c->crc > RF24_CRC_16_BITS))
		return -1;

	if ((c->pwr < RF24_PA_MIN) || (c->pwr > RF24_PA_MAX))
		return -1;

	return 0;
}

#ifdef WITH_JSON_CONFIG

int read_radio_conf(struct radio_conf *c, const char *path);
int read_plat_conf(struct plat_conf *c, const char *path);

#else

static inline int read_radio_conf(struct radio_conf *c, const char *path)
{
	return 0;
}

static inline int read_plat_conf(struct plat_conf *c, const char *path)
{
	return 0;
}

#endif

#endif /* __CFG_H__ */
