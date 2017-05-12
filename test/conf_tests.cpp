#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <CppUTest/TestHarness.h>
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "config.h"
}

TEST_GROUP(conf)
{
	struct cfg_platform pconf;
	struct cfg_radio rconf;

	void setup()
	{
		mock().disable();

		memset(&pconf, 0x0, sizeof(pconf));
		memset(&rconf, 0x0, sizeof(rconf));
	}

	void teardown()
	{

	}
};

TEST(conf, test_rconf_default)
{
	cfg_radio_init(&rconf);

	CHECK_EQUAL(0, cfg_radio_validate(&rconf));
}

TEST(conf, test_rconf_simple_ok)
{
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

TEST(conf, test_pconf_simple_ok)
{
	const char jconf[] = "				\
		{					\
			\"platform\": {			\
				\"name\": \"a\",	\
				\"spidev\": \"b\",	\
			}				\
		}";

	CHECK_EQUAL(0, cfg_from_string(jconf));
	CHECK_EQUAL(0, cfg_platform_read(&pconf));

	cfg_platform_dump(&pconf);

	STRCMP_EQUAL("a", pconf.name);
	STRCMP_EQUAL("b", pconf.spidev);
}
