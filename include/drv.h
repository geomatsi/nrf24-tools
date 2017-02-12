#ifndef __DRV_H__
#define __DRV_H__

int nrf24_driver_setup(char *spidev);

void f_csn(int level);
void f_ce(int level);
uint8_t f_spi_xfer(uint8_t dat);
int f_spi_multi_xfer(uint8_t *tx, uint8_t *rx, int len);

#endif /* __DRV_H__ */
