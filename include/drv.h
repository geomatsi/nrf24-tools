#ifndef __DRV_H__
#define __DRV_H__

extern int nrf24_driver_setup(void);

extern void f_csn(int level);
extern void f_ce(int level);
extern void f_spi_set_speed(int khz);
extern uint8_t f_spi_xfer(uint8_t dat);

#endif /* __DRV_H__ */
