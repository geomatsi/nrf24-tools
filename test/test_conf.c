#include "config.h"

int main(void)
{
	struct radio_conf rconf;
	int ret;

	init_radio_conf(&rconf);

	ret = validate_radio_conf(&rconf);
	printf("radio config validation %s\n",
		(ret < 0) ? "FAILED" : "PASSED");

	return 0;
}
