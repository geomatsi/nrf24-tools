#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "config.h"

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

int cfg_radio_read(struct cfg_radio *c)
{
	struct json_object_iter iter;
	enum json_type type;
	json_object *robj;

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
		if (type != json_type_int)
			continue;

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
