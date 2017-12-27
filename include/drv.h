#ifndef __DRV_H__
#define __DRV_H__

#include <RF24.h>

struct nrf24_drv {
	char *name;
};

struct nrf24_drv * nrf24_driver_setup(struct rf24 *pnrf, void *data);
int nrf24_driver_wait_for(struct nrf24_drv *pdrv);

#endif /* __DRV_H__ */
