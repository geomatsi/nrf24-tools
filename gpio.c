/* */

#include "gpio.h"

/* */

void pcduino_gpio_setup(int port, char *name, int dir)
{
	char gpio[128];
	FILE *file;
	int ret;

	file = fopen("/sys/class/gpio/export", "w");
	if (!file)
	{
		perror("can't open gpio export");
		return;
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0)
	{
		perror("can't export gpio");
		return;
	}

	fclose(file);

	sprintf(gpio, "/sys/class/gpio/%s/direction", name);

	file = fopen(gpio, "w");
	if (!file)
	{
		perror("can't open gpio direction");
		return;
	}

	if (dir == 0)
	{
		ret = fprintf(file, "in\n");
		if (ret < 0)
		{
			perror("can't write gpio direction");
			return;
		}
	}
	else
	{
		ret = fprintf(file, "out\n");
		if (ret < 0)
		{
			perror("can't write gpio direction");
			return;
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
		return;
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0)
	{
		perror("can't unexport gpio");
		return;
	}

	fclose(file);
}

int pcduino_gpio_read(char *name)
{
	char gpio[128];
	int ret, val;
	FILE *file;

	sprintf(gpio, "/sys/class/gpio/%s/value", name);

	file = fopen(gpio, "r");
	if (!file)
	{
		perror("can't open gpio value for read");
		return;
	}

	ret = fscanf(file, "%d", &val);
	if (ret < 0)
	{
		perror("can't read gpio value");
		return;
	}

	fclose(file);

	return val;
}

void pcduino_gpio_write(char *name, int value)
{
	char gpio[128];
	FILE *file;
	int ret;

	sprintf(gpio, "/sys/class/gpio/%s/value", name);
	file = fopen(gpio, "w");
	if (!file)
	{
		perror("can't open gpio value for write");
		return;
	}

	if (value == 0)
    {
        ret = fprintf(file, "0\n");
        if (ret < 0)
        {
            perror("can't write 0 to gpio");
            return;
        }
    }
	else
    {
        ret = fprintf(file, "1\n");
        if (ret < 0)
        {
            perror("can't write 1 to gpio");
            return;
        }
    }

	fclose(file);
}
