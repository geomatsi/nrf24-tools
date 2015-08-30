#include <um232h.h>

void um232h_gpiol_set(struct ftdi_context *fc, int bit, int level)
{
    uint8_t cmd = GET_BITS_LOW;
    uint8_t out = 0x0;
	uint8_t buf[3];

    ftdi_usb_purge_buffers(fc);

	FTDI_CHECK(ftdi_write_data(fc, &cmd, 1), "GET_BITS_LOW (W)", fc);
	FTDI_CHECK(ftdi_read_data(fc, &out, 1), "GET_BITS_LOW (R)", fc);

    buf[0] = SET_BITS_LOW;
	buf[1] = level ? (out | bit) : (out & (~bit));
	buf[2] = BITS_LOW_OUT;

	FTDI_CHECK(ftdi_write_data(fc, buf, 3), "SET_BITS_LOW (W)", fc);

    return;
}

void um232h_gpioh_set(struct ftdi_context *fc, int bit, int level)
{
    uint8_t cmd = GET_BITS_HIGH;
    uint8_t out = 0x0;
	uint8_t buf[3];

    ftdi_usb_purge_buffers(fc);

	FTDI_CHECK(ftdi_write_data(fc, &cmd, 1), "GET_BITS_HIGH (W)", fc);
	FTDI_CHECK(ftdi_read_data(fc, &out, 1), "GET_BITS_HIGH (R)", fc);

    buf[0] = SET_BITS_HIGH;
	buf[1] = level ? (out | bit) : (out & (~bit));
	buf[2] = BITS_HIGH_OUT;

	FTDI_CHECK(ftdi_write_data(fc, buf, 3), "SET_BITS_HIGH (W)", fc);

    return;
}

uint8_t um232h_spi_byte_xfer(struct ftdi_context *fc, uint8_t data)
{
	uint8_t rbuf = 0;

	uint8_t buf[4];

	buf[0] = 0x31;  /* MPSSE command: clock data bytes in and out MSG first */
	buf[1] = 0;     /* LenL = (len - 1) & 0xF */
	buf[2] = 0;     /* LenH = ((len - 1) >> 8) & 0xF */
    buf[3] = data;

    ftdi_usb_purge_buffers(fc);

    FTDI_CHECK(ftdi_write_data(fc, buf, 4), "BYTE XFER (W)", fc);
    FTDI_CHECK(ftdi_read_data(fc, &rbuf, 1), "BYTE XFER (R)", fc);

	return rbuf;
}

void um232h_mpsse_simple_init(struct ftdi_context *fc)
{
	uint8_t buf[3] = { 0, 0, 0 };

    /* init mpsse */

    FTDI_CHECK(ftdi_init(fc), "INIT", fc);
	FTDI_CHECK(ftdi_usb_open(fc, 0x0403, 0x6014), "USB OPEN", fc);
	FTDI_CHECK(ftdi_write_data_set_chunksize(fc, 32), "SET CHUNK 32", fc);
	FTDI_CHECK(ftdi_set_interface(fc, INTERFACE_A), "SET INTERFACE", fc);
	FTDI_CHECK(ftdi_usb_reset(fc), "RESET", fc);
	FTDI_CHECK(ftdi_set_latency_timer(fc, 2), "SET LATENCY 2ms", fc);
	FTDI_CHECK(ftdi_setflowctrl(fc, SIO_RTS_CTS_HS), "RTS/CTS", fc);
	//FTDI_CHECK(ftdi_set_bitmode(&fc, 0, 0), "RESET MPSSE", fc);
	FTDI_CHECK(ftdi_set_bitmode(fc, 0, BITMODE_MPSSE), "SET MPSSE MODE", fc);
	FTDI_CHECK(ftdi_usb_purge_buffers(fc), "PURGE", fc);

    /* init low gpio */

    buf[0] = SET_BITS_LOW;
	buf[1] = 0;
	buf[2] = BITS_LOW_OUT;
	FTDI_CHECK(ftdi_write_data(fc, buf, 3), "INIT LOW GPIO", fc);

    buf[0] = SET_BITS_HIGH;
	buf[1] = 0;
	buf[2] = BITS_HIGH_OUT;
	FTDI_CHECK(ftdi_write_data(fc, buf, 3), "INIT GPIO", fc);

}

void um232h_set_loopback(struct ftdi_context *fc, int loopback)
{
    uint8_t cmd;

	cmd = loopback ? LOOPBACK_START : LOOPBACK_END;
	FTDI_CHECK(ftdi_write_data(fc, &cmd, 1), "CONFIG LOOP", fc);
}

void um232h_set_speed(struct ftdi_context *fc, uint32_t speed)
{
	uint8_t buf[3] = { 0, 0, 0 };
	uint32_t div;

	buf[0] = EN_DIV_5;
	FTDI_CHECK(ftdi_write_data(fc, buf, 1), "ENABLE DIV 5", fc);

	div = (CLOCK_MAX_SPEED / speed) - 1;

	if (div > 0xFFFF) {
        fprintf(stderr, "CLOCK IS TOO HIGH\n");
		assert(0);
	}

	buf[0] = TCK_DIVISOR;
	buf[1] = (div >> 0) & 0xFF;
	buf[2] = (div >> 8) & 0xFF;
	FTDI_CHECK(ftdi_write_data(fc, buf, 3), "SET CLK DIV", fc);
}

void um232h_simple_close(struct ftdi_context *fc)
{
	ftdi_usb_close(fc);
	ftdi_deinit(fc);
}
