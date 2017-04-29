#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "config.h"

int read_radio_conf(struct radio_conf *c, const char *path)
{
	char s[MAX_CONFIG_SIZE] = {0};
	struct json_object_iter iter;
	enum json_type type;
	json_object *jobj;
	json_object *robj;
	FILE *fp;
	int ret;

	if (!c) {
		printf("invalid rconf pointer\n");
		return -1;
	}

	if (!path) {
		printf("%s: invalid config path\n", path);
		return -1;
	}

	fp = fopen(path, "r");
	if (!fp) {
		perror("couldn't open config");
		return -1;
	}

	ret = fread(s, 1, MAX_CONFIG_SIZE - 1, fp);
	if (!ret) {
		perror("couldn't read config");
		return -1;
	}

	printf("json string:\n%s\n", s);

	jobj = json_tokener_parse(s);
	if (!jobj) {
		printf("failed to parse config\n");
		return -1;
	}

	if (!json_object_object_get_ex(jobj, RADIO_TAG, &robj)) {
		printf("failed to get json object %s\n", RADIO_TAG);
		return -1;
	}

	json_object_object_foreachC(robj, iter) {
		printf("key[%s]\n", iter.key);

		type = json_object_get_type(iter.val);
		if (type != json_type_int)
			continue;

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


