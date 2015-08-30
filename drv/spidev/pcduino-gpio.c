/* */

#include "pcduino-gpio.h"

/* */

int pcduino_gpio_setup(int port, char *name, int dir)
{
	char gpio[128];
	FILE *file;
	int ret = 0;

	printf("setup gpio=%d name=%s dir=%d\n", port, name, dir);

	file = fopen("/sys/class/gpio/export", "w");
	if (!file)
	{
		perror("can't open gpio export");
		return -errno;
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0)
	{
		perror("can't export gpio");
		return ret;
	}

	fclose(file);

	sprintf(gpio, "/sys/class/gpio/%s/direction", name);

	file = fopen(gpio, "w");
	if (!file)
	{
		perror("can't open gpio direction");
		return -errno;
	}

	if (dir == 0)
	{
		ret = fprintf(file, "in\n");
		if (ret < 0)
		{
			perror("can't write gpio direction");
			return ret;
		}
	}
	else
	{
		ret = fprintf(file, "out\n");
		if (ret < 0)
		{
			perror("can't write gpio direction");
			return ret;
		}
	}

	fclose(file);
	return ret;
}

int pcduino_gpio_close(int port)
{
	FILE *file;
	int ret = 0;

	file = fopen("/sys/class/gpio/unexport", "w");
	if (!file)
	{
		perror("can't open gpio unexport");
		return -errno;
	}

	ret = fprintf(file, "%d\n", port);
	if (ret < 0)
	{
		perror("can't unexport gpio");
		return ret;
	}

	fclose(file);
	return ret;
}

int pcduino_gpio_read(char *name)
{
	char gpio[128];
	int val, ret = 0;
	FILE *file;

	sprintf(gpio, "/sys/class/gpio/%s/value", name);

	file = fopen(gpio, "r");
	if (!file)
	{
		perror("can't open gpio value for read");
		return -errno;
	}

	ret = fscanf(file, "%d", &val);
	if (ret < 0)
	{
		perror("can't read gpio value");
		return ret;
	}

	fclose(file);

	return val;
}

int pcduino_gpio_write(char *name, int value)
{
	char gpio[128];
	FILE *file;
	int ret = 0;

	sprintf(gpio, "/sys/class/gpio/%s/value", name);
	file = fopen(gpio, "w");
	if (!file)
	{
		perror("can't open gpio value for write");
		return -errno;
	}

	if (value == 0)
    {
        ret = fprintf(file, "0\n");
        if (ret < 0)
        {
            perror("can't write 0 to gpio");
            return ret;
        }
    }
	else
    {
        ret = fprintf(file, "1\n");
        if (ret < 0)
        {
            perror("can't write 1 to gpio");
            return ret;
        }
    }

	fclose(file);
}
