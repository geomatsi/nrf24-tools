#ifndef __UM232H_H__
#define __UM232H_H__

#include <inttypes.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <ftdi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* */

#define BIT_SK 0x0001    /* AD0 pin */
#define BIT_DO 0x0002    /* AD1 pin */
#define BIT_DI 0x0004    /* AD2 pin */
#define BIT_CS 0x0008    /* AD3 pin */

#define BIT_L0 0x0010    /* AD4 pin */
#define BIT_L1 0x0020    /* AD5 pin */
#define BIT_L2 0x0040    /* AD6 pin */
#define BIT_L3 0x0080    /* AD7 pin */

#define BIT_H0 0x0001   /* AC0 pin */
#define BIT_H1 0x0002   /* AC1 pin */
#define BIT_H2 0x0004   /* AC2 pin */
#define BIT_H3 0x0008   /* AC3 pin */
#define BIT_H4 0x0010   /* AC4 pin */
#define BIT_H5 0x0020   /* AC5 pin */
#define BIT_H6 0x0040   /* AC6 pin */
#define BIT_H7 0x0080   /* AC7 pin */

#define BITS_LOW_OUT (BIT_SK|BIT_DO|BIT_CS|BIT_L0|BIT_L1|BIT_L2|BIT_L3)
#define BITS_HIGH_OUT (BIT_H0|BIT_H1|BIT_H2|BIT_H3|BIT_H4|BIT_H5|BIT_H6|BIT_H7)

#define CLOCK_MAX_SPEED 6000000

/* */

#define FTDI_CHECK(FUN, MSG, CTX) do { \
		if ((FUN) < 0) \
		{ \
			fprintf(stderr, \
				"%s: %s\n", \
				MSG, \
				ftdi_get_error_string(CTX)); \
			assert(0); \
		} \
	} while (0)


/* */

void um232h_gpiol_set(struct ftdi_context *fc, int bit, int level);
void um232h_gpioh_set(struct ftdi_context *fc, int bit, int level);
uint8_t um232h_spi_byte_xfer(struct ftdi_context *fc, uint8_t data);
void um232h_mpsse_simple_init(struct ftdi_context *fc);
void um232h_set_loopback(struct ftdi_context *fc, int loopback);
void um232h_set_speed(struct ftdi_context *fc, uint32_t speed);
void um232h_simple_close(struct ftdi_context *fc);

#ifdef __cplusplus
}
#endif

#endif /* __UM232H_H__ */
