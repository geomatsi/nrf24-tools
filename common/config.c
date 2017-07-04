#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "config.h"

/* */

uint8_t pipe0_addr[PIPE_ADDR_SIZE] = { 0xE1, 0xE1, 0xE1, 0xE1, 0xE1 };

/* */

static char cfg_txt[MAX_CONFIG_SIZE] = {0};
static json_object *cfg_obj = NULL;

int cfg_from_file(const char *path)
{
	FILE *fp;
	int ret;

	printf("config=%s\n", path);

	if (!path) {
		printf("null config path\n");
		return -1;
	}

	fp = fopen(path, "r");
	if (!fp) {
		perror("couldn't open config");
		return -1;
	}

	ret = fread(cfg_txt, 1, MAX_CONFIG_SIZE - 1, fp);
	if (!ret) {
		perror("couldn't read config");
		return -1;
	}

	return cfg_from_string(cfg_txt);
}

int cfg_from_string(const char *buffer)
{
	if (!buffer) {
		printf("null input string\n");
		return -1;
	}

	cfg_obj = json_tokener_parse(buffer);
	if (!cfg_obj) {
		printf("failed to parse config\n");
		return -1;
	}

	return 0;
}

/* */

static int cfg_parse_pipe_addr_long(const char *saddr, uint8_t **paddr, const char *tag)
{
	unsigned int addr[PIPE_ADDR_SIZE];
	int tmp;

	tmp = sscanf(saddr, "%x:%x:%x:%x:%x",
			&addr[0], &addr[1], &addr[2], &addr[3], &addr[4]);

	if (tmp != 5) {
		printf("ERR: invalid %s entry: [%s]\n", tag, saddr);
		return -1;
	}

	for(int i = 0; i < PIPE_ADDR_SIZE; i++) {
		if (addr[i] > 0xff) {
			printf("ERR: invalid %s address [%s]\n", tag, saddr);
			return -1;
		}
	}

	*paddr = malloc(PIPE_ADDR_SIZE);

	for(int i = 0; i < PIPE_ADDR_SIZE; i++)
		*(*paddr + i) = addr[i];

	return 0;
}

static int cfg_parse_pipe_addr_short(const char *saddr, uint8_t *maddr,
					uint8_t **paddr, const char *tag)
{
	unsigned int digit;
	int tmp;
	int i;

	tmp = sscanf(saddr, "%x", &digit);

	if ((tmp != 1) || (digit > 0xff)) {
		printf("ERR: invalid %s entry: [%s]\n", tag, saddr);
		return -1;
	}

	*paddr = malloc(PIPE_ADDR_SIZE);

	/* first 4 bytes are the same as maddr (pipe1) */
	for(i = 0; i < PIPE_ADDR_SIZE - 1; i++)
		*(*paddr + i) = maddr[i];

	/* last byte differs */
	*(*paddr + i) = digit;

	return 0;
}

int cfg_radio_read(struct cfg_radio *c)
{
	struct json_object_iter iter;
	enum json_type type;
	json_object *robj;
	const char *saddr;
	int ret;

	if (!c) {
		printf("invalid rconf pointer\n");
		return -1;
	}

	/* 'radio' section is not available */
	if (!json_object_object_get_ex(cfg_obj, RADIO_TAG, &robj)) {
		printf("json tag %s is not available\n", RADIO_TAG);
		return 0;
	}

	json_object_object_foreachC(robj, iter) {
		type = json_object_get_type(iter.val);

		if (type == json_type_string) {
			if (!strcmp(iter.key, RADIO_TAG_PIPE0)) {
				saddr = json_object_get_string(iter.val);

				ret = cfg_parse_pipe_addr_long(saddr, &c->pipe[0], RADIO_TAG_PIPE0);
				if (ret < 0)
					return ret;
			}

			if (!strcmp(iter.key, RADIO_TAG_PIPE1)) {
				saddr = json_object_get_string(iter.val);

				ret = cfg_parse_pipe_addr_long(saddr, &c->pipe[1], RADIO_TAG_PIPE1);
				if (ret < 0)
					return ret;
			}

			if (c->pipe[1]) {
				if (!strcmp(iter.key, RADIO_TAG_PIPE2)) {
					saddr = json_object_get_string(iter.val);

					ret = cfg_parse_pipe_addr_short(saddr, c->pipe[1],
							&c->pipe[2], RADIO_TAG_PIPE2);
					if (ret < 0)
						return ret;
				}

				if (!strcmp(iter.key, RADIO_TAG_PIPE3)) {
					saddr = json_object_get_string(iter.val);

					ret = cfg_parse_pipe_addr_short(saddr, c->pipe[1],
							&c->pipe[3], RADIO_TAG_PIPE3);
					if (ret < 0)
						return ret;
				}

				if (!strcmp(iter.key, RADIO_TAG_PIPE4)) {
					saddr = json_object_get_string(iter.val);

					ret = cfg_parse_pipe_addr_short(saddr, c->pipe[1],
							&c->pipe[4], RADIO_TAG_PIPE4);
					if (ret < 0)
						return ret;
				}

				if (!strcmp(iter.key, RADIO_TAG_PIPE5)) {
					saddr = json_object_get_string(iter.val);

					ret = cfg_parse_pipe_addr_short(saddr, c->pipe[1],
							&c->pipe[5], RADIO_TAG_PIPE5);
					if (ret < 0)
						return ret;
				}
			}

		}

		if (type == json_type_int) {
			if (!strcmp(iter.key, RADIO_TAG_LEN))
				c->payload = json_object_get_int(iter.val);

			if (!strcmp(iter.key, RADIO_TAG_CHAN))
				c->channel = json_object_get_int(iter.val);

			if (!strcmp(iter.key, RADIO_TAG_RATE))
				c->rate = json_object_get_int(iter.val);

			if (!strcmp(iter.key, RADIO_TAG_CRC))
				c->crc = json_object_get_int(iter.val);

			if (!strcmp(iter.key, RADIO_TAG_PWR))
				c->pwr = json_object_get_int(iter.val);
		}
	}

	return 0;
}

/* */

static int cfg_platform_spidev_read(struct cfg_platform *c, json_object *pobj)
{
	struct json_object_iter iter;
	enum json_type type;

	json_object_object_foreachC(pobj, iter) {
		type = json_object_get_type(iter.val);
		if ((type != json_type_int) && (type != json_type_string))
			continue;

		if (!strcmp(iter.key, PLAT_TAG_SPIDEV_NAME))
			c->spidev = strdup(json_object_get_string(iter.val));

		if (!strcmp(iter.key, PLAT_TAG_SPIDEV_SPEED))
			c->speed = json_object_get_int(iter.val);

		if (!strcmp(iter.key, PLAT_TAG_SPIDEV_MODE))
			c->mode = json_object_get_int(iter.val);

		if (!strcmp(iter.key, PLAT_TAG_SPIDEV_BITS))
			c->bits = json_object_get_int(iter.val);

		if (!strcmp(iter.key, PLAT_TAG_SPIDEV_LSB))
			c->lsb = json_object_get_int(iter.val);
	}

	return 0;
}

static int cfg_platform_gpio_read(struct cfg_platform *c, json_object *pobj)
{
	struct json_object_iter iter;
	enum json_type type;

	json_object_object_foreachC(pobj, iter) {
		type = json_object_get_type(iter.val);
		if ((type != json_type_int) && (type != json_type_string))
			continue;

		if (!strcmp(iter.key, PLAT_TAG_GPIO_CE_NAME))
			c->pin_ce_name = strdup(json_object_get_string(iter.val));

		if (!strcmp(iter.key, PLAT_TAG_GPIO_CE))
			c->pin_ce = json_object_get_int(iter.val);

		if (!strcmp(iter.key, PLAT_TAG_GPIO_CSN_NAME))
			c->pin_csn_name = strdup(json_object_get_string(iter.val));

		if (!strcmp(iter.key, PLAT_TAG_GPIO_CSN))
			c->pin_csn = json_object_get_int(iter.val);
	}

	return 0;
}

int cfg_platform_read(struct cfg_platform *c)
{
	struct json_object_iter iter;
	json_object *pobj;
	int ret;

	if (!c) {
		printf("invalid pconf pointer\n");
		return -1;
	}

	/* 'sbc' section is not available */
	if (!json_object_object_get_ex(cfg_obj, PLAT_TAG, &pobj)) {
		printf("json tag %s is not available\n", PLAT_TAG);
		return 0;
	}

	json_object_object_foreachC(pobj, iter) {

		if (!strcmp(iter.key, PLAT_TAG_NAME))
			c->name = strdup(json_object_get_string(iter.val));

		if (!strcmp(iter.key, PLAT_TAG_SPIDEV)) {
			ret = cfg_platform_spidev_read(c, iter.val);
			if (ret) {
				printf("failed to parse %s config\n", PLAT_TAG_SPIDEV);
				return -1;
			}
		}

		if (!strcmp(iter.key, PLAT_TAG_GPIO)) {
			ret = cfg_platform_gpio_read(c, iter.val);
			if (ret) {
				printf("failed to parse %s config\n", PLAT_TAG_GPIO);
				return -1;
			}
		}
	}

	return 0;
}
