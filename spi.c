#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>

#include <linux/types.h>
#include <inttypes.h>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

/* */

uint32_t speed = 2000000;

uint8_t mode = 0;
uint8_t bits = 8;
uint8_t lsb = 0;

int fd;

/* */

void pcduino_spi_init(char *spidev)
{
	int ret;

	fd = open(spidev, O_RDWR);
	if (fd < 0)
	{
		perror("can't open device");
		abort();
	}

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		perror("can't set spi mode");
		abort();
	}

	ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb);
	if (ret == -1)
	{
		perror("can't set bit order");
		abort();
	}

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		perror("can't set bits per word");
		abort();
	}

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		perror("can't set max speed hz");
		abort();
	}
}

void pcduino_spi_info(char *spidev)
{
	uint8_t m, b, o;
	uint32_t s;

	int ret;

	ret = ioctl(fd, SPI_IOC_RD_MODE, &m);
	if (ret == -1)
	{
		perror("can't read spi mode");
		abort();
	}

	ret = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &o);
	if (ret == -1)
	{
		perror("can't read bit order");
		abort();
	}

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &b);
	if (ret == -1)
	{
		perror("can't read bits per word");
		abort();
	}

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &s);
	if (ret == -1)
	{
		perror("can't set max speed hz");
		abort();
	}

	printf("spi setup: mode = %u lsb = %u bits = %u speed = %u Hz\n",
		(unsigned int) m, (unsigned int) o, (unsigned int) b, (unsigned int) s);

	return;
}

uint8_t pcduino_spi_xfer(uint8_t txdata)
{
	struct spi_ioc_transfer xfer;
	uint8_t rxdata;
	int ret;

	xfer.tx_buf = (unsigned long) &txdata;
	xfer.rx_buf = (unsigned long) &rxdata;
	xfer.len = 1;

	xfer.bits_per_word = bits;
	xfer.speed_hz = speed;
	xfer.delay_usecs = 0;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
	if (ret < 1)
	{
		perror("can't perform spi transfer");
		abort();
	}

	return rxdata;
}

void pcduino_spi_close(void)
{
	close(fd);
}
