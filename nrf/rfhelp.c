
#include "rfhelp.h"
#include "rf.h"
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "crc.h"
#include <string.h>
#include "appconf.h"

// Variables
static mutex_t rf_mutex;
static char rx_addr[6][5];
static char tx_addr[5];
static int address_length;
//static char tx_pipe0_addr_eq;
static nrf_config nrf_conf;
static char init_done = false;

extern uint32_t init_errors;
extern SerialUSBDriver SDU1;

//#define TX_MODE

void printNrfAdd(void)
{
  int i;
  chprintf((BaseSequentialStream *)&SDU1, "\033[1;32m\r\n");
  for(i=0;i<6;i++)
  {
    rf_read_reg(NRF_REG_RX_ADDR_P0 + i, rx_addr[i], address_length);
    {
      chprintf((BaseSequentialStream *)&SDU1, "Rx_add[%d] : 0x%X 0x%X 0x%X 0x%X 0x%X\r\n",i,rx_addr[i][0],rx_addr[i][1],rx_addr[i][2],rx_addr[i][3],rx_addr[i][4]);
    }
  }
  rf_read_reg(NRF_REG_TX_ADDR, tx_addr, 3);
  chprintf((BaseSequentialStream *)&SDU1, "Tx_add : 0x%X 0x%X 0x%X 0x%X 0x%X\r\n",tx_addr[0],tx_addr[1],tx_addr[2],tx_addr[3],tx_addr[4]);
  chprintf((BaseSequentialStream *)&SDU1, "\033[0m\r\n");
}
/**
 * Initialize the nrf24l01 driver
 *
 * @return
 * true: Writing an address and reading it back worked.
 * false: Writing an address and reading it failed. This means that something
 * is wrong with the SPI communication.
 */
char rfhelp_init(void) {
  int i;
	chMtxObjectInit(&rf_mutex);
	rf_init();
	address_length = 3; // We assume length 3

#ifdef RX_MODE
	char addr_test[3] = {0x25, 0x26, 0x27};
#else
	char addr_test[3] = {0x15, 0x16, 0x17};
#endif
	rf_write_reg(NRF_REG_TX_ADDR, addr_test, 3);
	char addr_test_read[3];
	rf_read_reg(NRF_REG_TX_ADDR, addr_test_read, 3);

	if (memcmp(addr_test, addr_test_read, 3) != 0) {
		rf_stop();
		init_errors |= ERROR_SPI_TXRX;
		return false;
	}
    init_errors  = 0x132456;

//	rf_read_reg(NRF_REG_TX_ADDR, tx_addr, address_length);
//	tx_pipe0_addr_eq = memcmp(rx_addr[0], tx_addr, address_length) == 0;
	init_done = true;
	return true;
}

void rfhelp_stop(void) {
	rf_stop();
	init_done = false;
}

void rfhelp_update_conf(void) {

	nrf_conf.speed = APPCONF_NRF_SPEED;
	nrf_conf.power = APPCONF_NRF_POWER;
	nrf_conf.crc_type = APPCONF_NRF_CRC;
	nrf_conf.retry_delay = APPCONF_NRF_RETR_DELAY;
	nrf_conf.retries = APPCONF_NRF_RETRIES;
	nrf_conf.channel = APPCONF_NRF_CHANNEL;
	nrf_conf.address[0] = APPCONF_NRF_ADDR_B0;
	nrf_conf.address[1] = APPCONF_NRF_ADDR_B1;
	nrf_conf.address[2] = APPCONF_NRF_ADDR_B2;
	nrf_conf.send_crc_ack = APPCONF_NRF_SEND_CRC_ACK;

	if (init_done) {
		rfhelp_restart();
	}
}

/**
 * Re-init the rf chip
 */
void rfhelp_restart(void) {
	chMtxLock(&rf_mutex);

	rf_power_down();

	// Set default register values.

    rf_write_reg_byte(NRF_REG_EN_RXADDR, NRF_MASK_PIPE0 | NRF_MASK_PIPE1);

    rf_write_reg_byte(NRF_REG_RX_PW_P0, 8);
    rf_write_reg_byte(NRF_REG_RX_PW_P1, 8);

	rf_set_crc_type(nrf_conf.crc_type);
	rf_set_retr_retries(nrf_conf.retries);
	rf_set_retr_delay(nrf_conf.retry_delay);
	rf_set_power(nrf_conf.power);
	rf_set_speed(nrf_conf.speed);
	rf_set_address_width(NRF_AW_3); // Always use 3 byte address
	rf_set_frequency(2400 + (unsigned int)nrf_conf.channel);

//	rf_enable_features(NRF_FEATURE_DPL | NRF_FEATURE_DYN_ACK);
    rf_enable_pipe_autoack(NRF_MASK_PIPE0 | NRF_MASK_PIPE1);
    rf_enable_pipe_address(NRF_MASK_PIPE0 | NRF_MASK_PIPE1);
//    rf_enable_pipe_dlp(NRF_MASK_PIPE0 | NRF_MASK_PIPE1);

#ifdef RX_MODE
    nrf_conf.address[0] = 0x25;
    nrf_conf.address[1] = 0x26;
    nrf_conf.address[2] = 0x27;
#else
    nrf_conf.address[0] = 0x15;
    nrf_conf.address[1] = 0x16;
    nrf_conf.address[2] = 0x17;
#endif

	memcpy(tx_addr, nrf_conf.address, 3);
	memcpy(rx_addr[0], nrf_conf.address, 3);
//	tx_pipe0_addr_eq = true;

	rf_set_tx_addr(tx_addr, address_length);
	rf_set_rx_addr(0, rx_addr[0], address_length);
#ifdef RX_MODE
    nrf_conf.address[0] = 0x15;
    nrf_conf.address[1] = 0x16;
    nrf_conf.address[2] = 0x17;
#else
    nrf_conf.address[0] = 0x25;
    nrf_conf.address[1] = 0x26;
    nrf_conf.address[2] = 0x27;
#endif
    memcpy(rx_addr[1], nrf_conf.address, 3);
    rf_set_rx_addr(1, rx_addr[1], address_length);

	rf_power_up();
	rf_mode_rx();
	rf_flush_all();
	rf_clear_irq();

	chMtxUnlock(&rf_mutex);
}

/**
 * Set TX mode, send data, wait for result, set RX mode.
 *
 * @param data
 * The data to be sent.
 *
 * @param len
 * Length of the data.
 *
 * @return
 * 0: Send OK.
 * -1: Max RT.
 * -2: Timeout
 */
int rfhelp_send_data(char *data, int len, char ack) {
	int timeout = 60;
	int retval = -1;

	chMtxLock(&rf_mutex);

	rf_mode_tx();
	rf_clear_irq();
	rf_flush_all();

	// Pipe0-address and tx-address must be equal for ack to work.
//	if (!tx_pipe0_addr_eq && ack) {
//		rf_set_rx_addr(0, tx_addr, address_length);
//	}

	if (ack) {
		rf_write_tx_payload(data, len);
	} else {
		rf_write_tx_payload_no_ack(data, len);
	}

	for(;;) {
		int s = rf_status();

		chThdSleepMilliseconds(1);
		timeout--;

		if (NRF_STATUS_GET_TX_DS(s)) {
			retval = 0;
			break;
		} else if (NRF_STATUS_GET_MAX_RT(s)) {
			rf_clear_maxrt_irq();
			retval = -1;
			break;
		} else if (timeout == 0) {
			retval = -2;
			break;
		}
	}

	// Restore pipe0 address
//	if (!tx_pipe0_addr_eq && ack) {
//		rf_set_rx_addr(0, rx_addr[0], address_length);
//	}

	rf_mode_rx();

	chMtxUnlock(&rf_mutex);

	return retval;
}

/**
 * Same as rfhelp_send_data, but will add a crc checksum to the end. This is
 * useful for protecting against corruption between the NRF and the MCU in case
 * there are errors on the SPI bus.
 *
 * @param data
 * The data to be sent.
 *
 * @param len
 * Length of the data. Should be no more than 30 bytes.
 *
 * @return
 * 0: Send OK.
 * -1: Max RT.
 * -2: Timeout
 */
int rfhelp_send_data_crc(char *data, int len) {
	char buffer[len + 2];
	unsigned short crc = crc16((unsigned char*)data, len);

	memcpy(buffer, data, len);
	buffer[len] = (char)(crc >> 8);
	buffer[len + 1] = (char)(crc & 0xFF);

	return rfhelp_send_data(buffer, len + 2, nrf_conf.send_crc_ack);
}

/**
 * Read data from the RX fifo
 *
 * @param data
 * Pointer to the array in which to store the data.
 *
 * @param len
 * Pointer to variable storing the data length.
 *
 * @param pipe
 * Pointer to the pipe on which the data was received. Can be 0.
 *
 * @return
 * 1: Read OK, more data to read.
 * 0: Read OK
 * -1: No RX data
 * -2: Wrong length read. Something is likely wrong.
 */
int rfhelp_read_rx_data(char *data, int *len, int *pipe) {
	int retval = -1;

	chMtxLock(&rf_mutex);

	int s = rf_status();
	int pipe_n = NRF_STATUS_GET_RX_P_NO(s);
	if (pipe_n != 7) {
		*len = rf_get_payload_width();
	    chprintf((BaseSequentialStream *)&SDU1,"Payload Width : %d and pipe: %d\r\n",*len,pipe_n);
		if (pipe) {
			*pipe = pipe_n;
		}
		if (*len <= 32 && *len >= 0) {
			rf_read_rx_payload(data, *len);
			rf_clear_rx_irq();
//			rf_flush_rx();

			s = rf_status();
			if (NRF_STATUS_GET_RX_P_NO(s) == 7) {
				retval = 0;
			} else {
				retval = 1;
			}
		} else {
			*len = 0;
			retval = -2;
		}
	}

	chMtxUnlock(&rf_mutex);

	return retval;
}

/**
 * Same as rfhelp_read_rx_data, but will check if there is a valid CRC in the
 * end of the payload.
 *
 * @param data
 * Pointer to the array in which to store the data.
 *
 * @param len
 * Pointer to variable storing the data length.
 *
 * @param pipe
 * Pointer to the pipe on which the data was received. Can be 0.
 *
 * @return
 * 1: Read OK, more data to read.
 * 0: Read OK
 * -1: No RX data
 * -2: Wrong length read. Something is likely wrong.
 * -3: Data read, but CRC does not match.
 */
int rfhelp_read_rx_data_crc(char *data, int *len, int *pipe) {
	int res = rfhelp_read_rx_data(data, len, pipe);

	if (res >= 0 && *len > 2) {
		unsigned short crc = crc16((unsigned char*)data, *len - 2);

		if (crc	!= ((unsigned short) data[*len - 2] << 8 | (unsigned short) data[*len - 1])) {
			res = -3;
		}
	}

	*len -= 2;

	return res;
}

int rfhelp_rf_status(void) {
	chMtxLock(&rf_mutex);
	int s = rf_status();
	chMtxUnlock(&rf_mutex);

	return s;
}

void rfhelp_set_tx_addr(const char *addr, int addr_len) {
	chMtxLock(&rf_mutex);
	memcpy(tx_addr, addr, addr_len);
	address_length = addr_len;

//	tx_pipe0_addr_eq = memcmp(rx_addr[0], tx_addr, address_length) == 0;

	rf_set_tx_addr(tx_addr, address_length);
	chMtxUnlock(&rf_mutex);
}

void rfhelp_set_rx_addr(int pipe, const char *addr, int addr_len) {
	chMtxLock(&rf_mutex);
	memcpy(rx_addr[pipe], addr, addr_len);
	address_length = addr_len;

//	tx_pipe0_addr_eq = memcmp(rx_addr[0], tx_addr, address_length) == 0;

	rf_set_rx_addr(pipe, addr, address_length);
	chMtxUnlock(&rf_mutex);
}

void rfhelp_power_down(void) {
	chMtxLock(&rf_mutex);
	rf_power_down();
	chMtxUnlock(&rf_mutex);
}

void rfhelp_power_up(void) {
	chMtxLock(&rf_mutex);
	rf_power_up();
	chMtxUnlock(&rf_mutex);
}
