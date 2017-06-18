#include <string.h>

#include "spi.h"

/* */

static int fd;

/* */

int spi_open(char *spidev)
{
	printf("open spi device %s\n", spidev);

	fd = open(spidev, O_RDWR);
	if (fd < 0) {
		perror("can't open device");
		return -1;
	}

	return 0;
}

int spi_init(uint32_t speed, uint8_t mode, uint8_t bits, uint8_t lsb)
{
	int ret;

	printf("init spi device\n");

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1) {
		perror("can't set spi mode");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb);
	if (ret == -1) {
		perror("can't set bit order");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) {
		perror("can't set bits per word");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror("can't set max speed hz");
		return -1;
	}

	return 0;
}

int spi_info(void)
{
	uint8_t m, b, o;
	uint32_t s;
	int ret;

	printf("get spi device info\n");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &m);
	if (ret == -1) {
		perror("can't read spi mode");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &o);
	if (ret == -1) {
		perror("can't read bit order");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &b);
	if (ret == -1) {
		perror("can't read bits per word");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &s);
	if (ret == -1) {
		perror("can't read max speed hz");
		return -1;
	}

	printf("spi setup: mode = %u lsb = %u bits = %u speed = %u Hz\n",
			(unsigned int) m, (unsigned int) o, (unsigned int) b, (unsigned int) s);

	return 0;
}

/* full-duplex */
uint8_t spi_xfer_fdx(uint8_t txdata)
{
	struct spi_ioc_transfer xfer[1];
	uint8_t rxdata = 0xff;
	int ret;

	memset(xfer, 0, sizeof(xfer));

	xfer[0].tx_buf = (unsigned long) &txdata;
	xfer[0].rx_buf = (unsigned long) &rxdata;
	xfer[0].len = 1;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) {
		perror("can't perform spi transfer");
		return rxdata;
	}

	return rxdata;
}

/* full-duplex multi-byte */
int spi_xfer_mfdx(uint8_t *txbuf, uint8_t *rxbuf, int len)
{
	struct spi_ioc_transfer xfer[1];
	int ret;

	memset(xfer, 0, sizeof(xfer));

	xfer[0].tx_buf = (unsigned long)txbuf;
	xfer[0].rx_buf = (unsigned long)rxbuf;
	xfer[0].len = len;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) {
		perror("can't perform spi transfer");
		return ret;
	}

	return 0;
}

/* half-duplex */
uint8_t spi_xfer_hdx(uint8_t txdata)
{
	struct spi_ioc_transfer xfer[2];
	uint8_t rxdata = 0;
	int ret;

	memset(xfer, 0, sizeof(xfer));

	xfer[0].tx_buf = (unsigned long) &txdata;
	xfer[0].len = 1;

	xfer[1].rx_buf = (unsigned long) &rxdata;
	xfer[1].len = 1;

	ret = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	if (ret < 0) {
		perror("can't perform spi transfer");
		return rxdata;
	}

	return rxdata;
}

void spi_close(void)
{
	close(fd);
}
