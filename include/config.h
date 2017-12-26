#ifndef __CFG_H__
#define __CFG_H__

#include <stdbool.h>
#include <string.h>

#include <RF24.h>

#define DEFAULT_CONFIG	"/etc/nrf24/default.cfg"
#define MAX_CONFIG_SIZE	4096
#define PIPE_ADDR_SIZE	5
#define PIPE_MAX_NUM	6

/* */

extern uint8_t pipe0_addr[PIPE_ADDR_SIZE];

/* common cfg ops */

#ifdef WITH_JSON_CONFIG

int cfg_from_file(const char *path);
int cfg_from_string(const char *buf);

#else

static inline int cfg_from_file(const char *path)
{
	return 0;
}

static inline int cfg_from_string(const char *buf)
{
	return 0;
}

#endif

/* config 'nrf24 radio' section */

#define RADIO_TAG	"radio"

#define RADIO_TAG_LEN	"payload"
#define RADIO_TAG_CHAN	"channel"
#define RADIO_TAG_RATE	"rate"
#define RADIO_TAG_CRC	"crc"
#define RADIO_TAG_PWR	"pwr"

#define RADIO_TAG_PIPE0	"pipe0"
#define RADIO_TAG_PIPE1	"pipe1"
#define RADIO_TAG_PIPE2	"pipe2"
#define RADIO_TAG_PIPE3	"pipe3"
#define RADIO_TAG_PIPE4	"pipe4"
#define RADIO_TAG_PIPE5	"pipe5"

struct cfg_radio {
	/* payload 0 .. 32, 0 for dynamic payload */
	uint8_t payload;
	uint8_t channel;
	uint8_t rate;
	uint8_t crc;
	uint8_t pwr;
	uint8_t *pipe[PIPE_MAX_NUM];
};

static inline bool cfg_payload_is_dynamic(struct cfg_radio *c)
{
	return (c->payload == 0);
}

static inline void cfg_radio_init(struct cfg_radio *c)
{
	if (!c)
		return;

	memset(c, 0x0, sizeof(*c));

	c->payload = 32;
	c->channel = 10;
	c->rate = RF24_RATE_MIN;
	c->crc = RF24_CRC_MAX;
	c->pwr = RF24_PA_MAX;
	c->pipe[0] = pipe0_addr;
}

static inline int cfg_radio_validate(const struct cfg_radio *c)
{
	if ((c->payload < 0) || (c->payload > 32))
		return -1;

	if (c->channel > RF24_MAX_CHANNEL)
		return -1;

	if ((c->rate < RF24_RATE_MIN) || (c->rate > RF24_RATE_MAX))
		return -1;

	if ((c->crc < RF24_CRC_MIN) || (c->crc > RF24_CRC_MAX))
		return -1;

	if ((c->pwr < RF24_PA_MIN) || (c->pwr > RF24_PA_MAX))
		return -1;

	return 0;
}

static inline void cfg_radio_dump(const struct cfg_radio *c)
{
	printf("rconf: payload[%d] channel[%d] rate[%d] crc[%d] pwr[%d]\n",
		c->payload, c->channel, c->rate, c->crc, c->pwr);

	for (int i = 0; i < PIPE_MAX_NUM; i++) {
		if (c->pipe[i]) {
			printf("rconf: pipe%d[0x%02x:0x%02x:0x%02x:0x%02x:0x%02x]\n", i,
				c->pipe[i][0], c->pipe[i][1], c->pipe[i][2],
				c->pipe[i][3], c->pipe[i][4]);
		} else {
			printf("rconf: pipe[%d](null)\n", i);
		}
	}

}

#ifdef WITH_JSON_CONFIG

int cfg_radio_read(struct cfg_radio *c);

#else

static inline int cfg_radio_read(struct cfg_radio *c)
{
	return 0;
}

#endif

/* config 'sbc' section */

#define PLAT_TAG		"sbc"
#define PLAT_TAG_NAME		"name"

#define PLAT_TAG_GPIO		"gpio"
#define PLAT_TAG_GPIO_CE	"ce_gpio"
#define PLAT_TAG_GPIO_CE_NAME	"ce_name"
#define PLAT_TAG_GPIO_CSN	"csn_gpio"
#define PLAT_TAG_GPIO_CSN_NAME	"csn_name"
#define PLAT_TAG_GPIO_IRQ	"irq_gpio"
#define PLAT_TAG_GPIO_IRQ_EDGE	"irq_edge"
#define PLAT_TAG_GPIO_IRQ_NAME	"irq_name"

#define PLAT_TAG_SPIDEV		"spidev"
#define PLAT_TAG_SPIDEV_NAME	"name"
#define PLAT_TAG_SPIDEV_SPEED	"speed"
#define PLAT_TAG_SPIDEV_MODE	"mode"
#define PLAT_TAG_SPIDEV_BITS	"bits"
#define PLAT_TAG_SPIDEV_LSB	"lsb"

#define DFLT_SPIDEV	"/dev/spidev0.0"

struct cfg_platform {
	char *name;

	int pin_ce;
	char *pin_ce_name;
	int pin_csn;
	char *pin_csn_name;
	int pin_irq;
	int pin_irq_edge;
	char *pin_irq_name;

	char *spidev;
	uint32_t speed;
	uint8_t mode;
	uint8_t bits;
	uint8_t lsb;
};

static inline void cfg_platform_init(struct cfg_platform *c)
{
	if (!c)
		return;

	memset(c, 0x0, sizeof(*c));

	c->name = NULL;

	c->pin_ce_name = NULL;
	c->pin_ce = 0;
	c->pin_csn_name = NULL;
	c->pin_csn = 0;
	c->pin_irq_name = NULL;
	c->pin_irq_edge = 0;
	c->pin_irq = 0;

	c->spidev = (char *)DFLT_SPIDEV;
	c->speed = 1000000;
	c->mode = 0;
	c->bits = 8;
	c->lsb = 0;
}

static inline void cfg_platform_dump(const struct cfg_platform *c)
{
	printf("pconf: platform[%s]\n", c->name);
	printf("pconf: spidev[%s] speed(%u) mode(%u) bits(%u) lsb(%u)\n",
		c->spidev, c->speed, c->mode, c->bits, c->lsb);
	printf("pconf: pin_ce[%d][%s] pin_csn[%d][%s] pin_irq[%d][%s]\n",
		c->pin_ce, c->pin_ce_name, c->pin_csn, c->pin_csn_name,
		c->pin_irq, c->pin_irq_name);
}

#ifdef WITH_JSON_CONFIG

int cfg_platform_read(struct cfg_platform *c);

#else

static inline int cfg_platform_read(struct cfg_platform *c)
{
	return 0;
}

#endif

#endif /* __CFG_H__ */
