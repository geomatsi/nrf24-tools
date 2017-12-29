#include "gpio.h"

/* */

int gpio_setup(int port, char *name, int dir)
{
	char gpio[128] = {0};
	FILE *file;
	int ret = 0;

	printf("gpio setup: gpio=%d name=[%s] dir=%d\n",
		port, name, dir);

	file = fopen("/sys/class/gpio/export", "w");
	if (!file) {
		perror("ERR: can't open gpio export");
		ret = -1;
		goto out;
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0) {
		perror("ERR: can't export gpio");
		goto out;
	}

	fclose(file);

	sprintf(gpio, "/sys/class/gpio/%s/direction", name);
	file = fopen(gpio, "w");
	if (!file) {
		perror("ERR: can't open gpio direction");
		goto out;
	}

	switch (dir) {
		case DIR_IN: /* input */
			ret = fprintf(file, "in\n");
			break;
		case DIR_OUT: /* output */
			ret = fprintf(file, "out\n");
			break;
		default:
			printf("unknown direction: %d\n", dir);
			ret = -1;
			break;
	}

	if (ret < 0) {
		perror("ERR: can't write gpio direction");
		goto out;
	}
out:
	if (file)
		fclose(file);

	return (ret >= 0) ? 0 : ret;
}

int gpio_edge(int port, char *name, int edge)
{
	char gpio[128] = {0};
	FILE *file;
	int ret = 0;

	printf("gpio edge: gpio=%d name=[%s] edge=%d\n",
		port, name, edge);

	sprintf(gpio, "/sys/class/gpio/%s/edge", name);
	file = fopen(gpio, "w");
	if (!file) {
		perror("ERR: can't open gpio edge");
		ret = -1;
		goto out;
	}

	switch (edge) {
		case EDGE_NONE: /* none edge */
			ret = fprintf(file, "none\n");
			break;
		case EDGE_RISING: /* rising edge */
			ret = fprintf(file, "rising\n");
			break;
		case EDGE_FALLING: /* falling edge */
			ret = fprintf(file, "falling\n");
			break;
		case EDGE_BOTH: /* both edges */
			ret = fprintf(file, "both\n");
			break;
		default:
			printf("unknown edge: %d\n", edge);
			ret = -1;
			break;
	}

	if (ret < 0) {
		perror("ERR: can't write gpio edge");
		goto out;
	}

out:
	if (file)
		fclose(file);

	return (ret >= 0) ? 0 : ret;
}

int gpio_active_low(int port, char *name, int active_low)
{
	char gpio[128] = {0};
	FILE *file;
	int ret = 0;

	printf("gpio edge: gpio=%d name=[%s] active_low=%d\n",
		port, name, active_low);

	sprintf(gpio, "/sys/class/gpio/%s/active_low", name);
	file = fopen(gpio, "w");
	if (!file) {
		perror("ERR: can't open gpio active_low");
		ret = -1;
		goto out;
	}

	ret = fprintf(file, "%d\n", active_low);
	if (ret < 0) {
		perror("ERR: can't write gpio active_low");
		goto out;
	}

out:
	if (file)
		fclose(file);

	return (ret >= 0) ? 0 : ret;
}

int gpio_close(int port, char *name)
{
	FILE *file;
	int ret = 0;

	file = fopen("/sys/class/gpio/unexport", "w");
	if (!file) {
		perror("ERR: can't open gpio unexport");
		ret = -1;
		goto out;
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0) {
		perror("ERR: can't unexport gpio");
		goto out;
	}

out:
	if (file)
		fclose(file);

	return (ret >= 0) ? 0 : ret;
}

int gpio_read(char *name)
{
	char gpio[128];
	FILE *file;
	int val;
	int ret = 0;

	sprintf(gpio, "/sys/class/gpio/%s/value", name);

	file = fopen(gpio, "r");
	if (!file) {
		perror("ERR: can't open gpio value for read");
		ret = -1;
		goto out;
	}

	ret = fscanf(file, "%d", &val);
	if (ret < 0) {
		perror("ERR: can't read gpio value");
		goto out;
	}

	ret = val;

out:
	if (file)
		fclose(file);

	return ret;
}

int gpio_write(char *name, int value)
{
	char gpio[128];
	FILE *file;
	int ret = 0;

	sprintf(gpio, "/sys/class/gpio/%s/value", name);
	file = fopen(gpio, "w");
	if (!file) {
		perror("ERR: can't open gpio value for write");
		ret = -1;
		goto out;
	}

	if (value == 0) {
		ret = fprintf(file, "0\n");
		if (ret < 0) {
			perror("can't write 0 to gpio");
			return -1;
		}
	} else {
		ret = fprintf(file, "1\n");
		if (ret < 0) {
			perror("can't write 1 to gpio");
			return -1;
		}
	}

	fclose(file);

	return 0;
}
