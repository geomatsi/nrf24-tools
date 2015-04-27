#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>

#include <linux/types.h>
#include <inttypes.h>

/* */

void pcduino_gpio_setup(int port, int dir)
{
	char gpio[128];
	FILE *file;
	int ret;

	file = fopen("/sys/class/gpio/export", "w");
	if (!file)
	{
		perror("can't open gpio export");
		abort();
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0)
	{
		perror("can't export gpio");
		abort();
	}

	fclose(file);

	sprintf(gpio, "/sys/class/gpio/gpio%d/direction", port);

	file = fopen(gpio, "w");
	if (!file)
	{
		perror("can't open gpio direction");
		abort();
	}

	if (dir == 0)
	{
		ret = fprintf(file, "in\n");
		if (ret < 0)
		{
			perror("can't write gpio direction");
			abort();
		}
	}
	else
	{
		ret = fprintf(file, "out\n");
		if (ret < 0)
		{
			perror("can't write gpio direction");
			abort();
		}
	}

	fclose(file);
}

void pcduino_gpio_close(int port)
{
	FILE *file;
	int ret;

	file = fopen("/sys/class/gpio/unexport", "w");
	if (!file)
	{
		perror("can't open gpio unexport");
		abort();
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0)
	{
		perror("can't unexport gpio");
		abort();
	}

	fclose(file);
}

int pcduino_gpio_read(int port)
{
	char gpio[128];
	int ret, val;
	FILE *file;

	sprintf(gpio, "/sys/class/gpio/gpio%d/value", port);

	file = fopen(gpio, "r");
	if (!file)
	{
		perror("can't open gpio value for read");
		abort();
	}

	ret = fscanf(file, "%d", &val);
	if (ret < 0)
	{
		perror("can't read gpio value");
		abort();
	}

	fclose(file);

	return val;
}

void pcduino_gpio_write(int port, int value)
{
	char gpio[128];
	FILE *file;
	int ret;

	sprintf(gpio, "/sys/class/gpio/gpio%d/value", port);
	file = fopen(gpio, "w");
	if (!file)
	{
		perror("can't open gpio value for write");
		abort();
	}

	if (value == 0)
		fprintf(file, "0\n");
	else
		fprintf(file, "1\n");

	fclose(file);
}
