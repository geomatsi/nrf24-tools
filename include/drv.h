#ifndef __DRV_H__
#define __DRV_H__

int nrf24_driver_setup(char *spidev);

void f_csn(int level);
void f_ce(int level);
void f_spi_set_speed(int khz);
uint8_t f_spi_xfer(uint8_t dat);

#endif /* __DRV_H__ */
