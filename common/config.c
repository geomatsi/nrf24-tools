#include <stdio.h>
#include <string.h>

#include <json.h>

#include "config.h"

void init_radio_conf(struct radio_conf *c)
{
	if (!c)
		return;

	memset(c, 0x0, sizeof(*c));

	c->channel = 10;
	c->rate = RF24_RATE_1M;
	c->crc = RF24_CRC_16_BITS;
	c->pwr = RF24_PA_MAX;
}

int validate_radio_conf(const struct radio_conf *c)
{
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

int read_radio_conf(const char *path)
{
	return 0;
}
