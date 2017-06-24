#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <CppUTest/TestHarness.h>

extern "C" {
#include "config.h"
}

TEST_GROUP(conf)
{
	uint8_t pipe1[PIPE_ADDR_SIZE] = { 0xa1, 0xb1, 0xc1, 0xd1, 0xe1 };
	uint8_t pipe2[PIPE_ADDR_SIZE] = { 0xa2, 0xb2, 0xc2, 0xd2, 0xe2 };

	void setup()
	{

	}

	void teardown()
	{

	}
};

TEST(conf, test_rconf_check_default_sanity)
{
	struct cfg_radio rconf = {0};

	cfg_radio_init(&rconf);

	CHECK_EQUAL(0, cfg_radio_validate(&rconf));
}

TEST(conf, test_rconf_check_default_values)
{
	struct cfg_radio rconf;

	cfg_radio_init(&rconf);
	cfg_radio_dump(&rconf);

	CHECK_EQUAL(32, rconf.payload);
	CHECK_EQUAL(10, rconf.channel);
	CHECK_EQUAL(0, rconf.rate);
	CHECK_EQUAL(2, rconf.crc);
	CHECK_EQUAL(3, rconf.pwr);

	MEMCMP_EQUAL(pipe1_addr, rconf.pipe1, sizeof(pipe1_addr));
}

TEST(conf, test_rconf_basic)
{
	struct cfg_radio rconf = {0};

	const char jconf[] = "			\
		{				\
			\"radio\": {		\
				\"payload\": 0,	\
				\"channel\": 1,	\
				\"rate\": 1,	\
				\"crc\": 2,	\
				\"pwr\": 3,	\
			}			\
		}";

	CHECK_EQUAL(0, cfg_from_string(jconf));
	CHECK_EQUAL(0, cfg_radio_read(&rconf));
	CHECK_EQUAL(0, cfg_radio_validate(&rconf));

	cfg_radio_dump(&rconf);

	CHECK_EQUAL(0, rconf.payload);
	CHECK_EQUAL(1, rconf.channel);
	CHECK_EQUAL(1, rconf.rate);
	CHECK_EQUAL(2, rconf.crc);
	CHECK_EQUAL(3, rconf.pwr);
}

TEST(conf, test_rconf_simple_full)
{
	struct cfg_radio rconf = {0};

	const char jconf[] = "							\
		{								\
			\"radio\": {						\
				\"payload\": 0,					\
				\"channel\": 1,					\
				\"rate\": 1,					\
				\"crc\": 2,					\
				\"pwr\": 3,					\
				\"pipe1\": \"0xa1:0xb1:0xc1:0xd1:0xe1\",	\
				\"pipe2\": \"0xa2:0xb2:0xc2:0xd2:0xe2\",	\
				\"pipe3\": \"0xe3\",				\
				\"pipe4\": \"0xe4\",				\
				\"pipe5\": \"0xe5\",				\
				\"pipe6\": \"0xe6\",				\
			}							\
		}";

	CHECK_EQUAL(0, cfg_from_string(jconf));
	CHECK_EQUAL(0, cfg_radio_read(&rconf));
	CHECK_EQUAL(0, cfg_radio_validate(&rconf));

	cfg_radio_dump(&rconf);

	CHECK_EQUAL(0, rconf.payload);
	CHECK_EQUAL(1, rconf.channel);
	CHECK_EQUAL(1, rconf.rate);
	CHECK_EQUAL(2, rconf.crc);
	CHECK_EQUAL(3, rconf.pwr);

	MEMCMP_EQUAL(pipe1, rconf.pipe1, PIPE_ADDR_SIZE);
	MEMCMP_EQUAL(pipe2, rconf.pipe2, PIPE_ADDR_SIZE);

	CHECK_EQUAL(0xe3, rconf.pipe3);
	CHECK_EQUAL(0xe4, rconf.pipe4);
	CHECK_EQUAL(0xe5, rconf.pipe5);
	CHECK_EQUAL(0xe6, rconf.pipe6);
}


TEST(conf, test_pconf_name_ok)
{
	struct cfg_platform pconf = {0};

	const char jconf[] = "				\
		{					\
			\"sbc\": {			\
				\"name\": \"a\",	\
			}				\
		}";

	CHECK_EQUAL(0, cfg_from_string(jconf));
	CHECK_EQUAL(0, cfg_platform_read(&pconf));

	cfg_platform_dump(&pconf);

	STRCMP_EQUAL("a", pconf.name);
}

TEST(conf, test_pconf_spidev_ok)
{
	struct cfg_platform pconf = {0};

	const char jconf[] = "					\
		{						\
			\"sbc\": {				\
				\"spidev\": {			\
					\"name\": \"a\",	\
					\"speed\": 100,		\
					\"mode\": 5,		\
					\"bits\":4,		\
					\"lsb\":1,		\
				},				\
			},					\
		}";

	CHECK_EQUAL(0, cfg_from_string(jconf));
	CHECK_EQUAL(0, cfg_platform_read(&pconf));

	cfg_platform_dump(&pconf);

	STRCMP_EQUAL("a", pconf.spidev);
	CHECK_EQUAL(100, pconf.speed);
	CHECK_EQUAL(5, pconf.mode);
	CHECK_EQUAL(4, pconf.bits);
	CHECK_EQUAL(1, pconf.lsb);
}

TEST(conf, test_pconf_gpio_ok)
{
	struct cfg_platform pconf = {0};

	const char jconf[] = "					\
		{						\
			\"sbc\": {				\
				\"gpio\": {			\
					\"ce_name\": \"a\",	\
					\"ce_gpio\": 100,	\
					\"csn_name\": \"b\",	\
					\"csn_gpio\": 200,	\
				},				\
			},					\
		}";

	CHECK_EQUAL(0, cfg_from_string(jconf));
	CHECK_EQUAL(0, cfg_platform_read(&pconf));

	cfg_platform_dump(&pconf);

	STRCMP_EQUAL("a", pconf.pin_ce_name);
	CHECK_EQUAL(100, pconf.pin_ce);
	STRCMP_EQUAL("b", pconf.pin_csn_name);
	CHECK_EQUAL(200, pconf.pin_csn);
}

TEST(conf, test_pconf_complete_ok)
{
	struct cfg_platform pconf = {0};

	const char jconf[] = "					\
		{						\
			\"sbc\": {				\
				\"name\": \"test\",		\
				\"spidev\": {			\
					\"name\": \"/dev/a\",	\
					\"speed\": 100,		\
					\"mode\": 5,		\
					\"bits\":4,		\
					\"lsb\":1,		\
				},				\
				\"gpio\": {			\
					\"ce_name\": \"pin1\",	\
					\"ce_gpio\": 100,	\
					\"csn_name\": \"pin2\",	\
					\"csn_gpio\": 200,	\
				},				\
			},					\
		}";

	CHECK_EQUAL(0, cfg_from_string(jconf));
	CHECK_EQUAL(0, cfg_platform_read(&pconf));

	cfg_platform_dump(&pconf);

	STRCMP_EQUAL("test", pconf.name);

	STRCMP_EQUAL("/dev/a", pconf.spidev);
	CHECK_EQUAL(100, pconf.speed);
	CHECK_EQUAL(5, pconf.mode);
	CHECK_EQUAL(4, pconf.bits);
	CHECK_EQUAL(1, pconf.lsb);

	STRCMP_EQUAL("pin1", pconf.pin_ce_name);
	CHECK_EQUAL(100, pconf.pin_ce);
	STRCMP_EQUAL("pin2", pconf.pin_csn_name);
	CHECK_EQUAL(200, pconf.pin_csn);
}
