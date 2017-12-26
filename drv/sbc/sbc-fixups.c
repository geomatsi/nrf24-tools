#include <RF24.h>

#include "gpio.h"
#include "spi.h"


int pcduino_upstream_fixup(void)
{
	int ret = 0;

	/* RFM69 chip on Wireless Gate Shield v1.0b: disable spi chip select */

#define PCDUINO_RFM69_CSN	229
#define PCDUINO_RFM69_CSN_NAME	"gpio229"

	ret = gpio_setup(PCDUINO_RFM69_CSN, PCDUINO_RFM69_CSN_NAME, DIR_OUT, EDGE_NONE);
	if (ret)
		goto out;

	ret = gpio_write(PCDUINO_RFM69_CSN_NAME, 1);
	if (ret)
		goto out;

out:
	return ret;
}

int pcduino_legacy_fixup(void)
{
	int ret = 0;

	/* RFM69 chip on Wireless Gate Shield v1.0b: disable spi chip select */

#define PCDUINO_LEGACY_RFM69_CSN	9
#define PCDUINO_LEGACY_RFM69_CSN_NAME	"gpio9_ph5"

	ret = gpio_setup(PCDUINO_LEGACY_RFM69_CSN, PCDUINO_LEGACY_RFM69_CSN_NAME, DIR_OUT, EDGE_NONE);
	if (ret)
		goto out;

	ret = gpio_write(PCDUINO_LEGACY_RFM69_CSN_NAME, 1);
	if (ret)
		goto out;

out:
	return ret;
}
