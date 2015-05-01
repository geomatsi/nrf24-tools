#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>

#include <linux/types.h>
#include <inttypes.h>

/* */

enum {
	DIR_IN = 0,
	DIR_OUT
} gpio_dir_t;

void pcduino_gpio_setup(int port, char *name, int dir);
void pcduino_gpio_close(int port);

int pcduino_gpio_read(char *name);
void pcduino_gpio_write(char *name, int value);

#endif /* GPIO_HAL_H */
