#include <unistd.h>

void delay_ms(int mdelay)
{
	usleep(1000*mdelay);
}

void delay_us(int udelay)
{
	usleep(udelay);
}
