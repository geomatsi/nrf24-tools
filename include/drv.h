#ifndef __DRV_H__
#define __DRV_H__

#include <RF24.h>

int nrf24_driver_setup(struct rf24 *pnrf, void *data);

#endif /* __DRV_H__ */
