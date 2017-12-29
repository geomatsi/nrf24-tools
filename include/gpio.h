#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <linux/types.h>
#include <inttypes.h>

/* */

enum {
	DIR_IN = 0,
	DIR_OUT
} gpio_dir_t;

enum {
	EDGE_NONE	= 0,
	EDGE_RISING	= 1,
	EDGE_FALLING	= 2,
	EDGE_BOTH	= 3,
} gpio_edge_t;

int gpio_setup(int port, char *name, int dir);
int gpio_edge(int port, char *name, int edge);
int gpio_active_low(int port, char *name, int active_low);
int gpio_close(int port, char *name);

int gpio_read(char *name);
int gpio_write(char *name, int value);
int gpio_wait_for_irq(char *name);

#endif /* GPIO_HAL_H */
