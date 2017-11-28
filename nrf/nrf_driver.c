#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "nrf_driver.h"
#include "rf.h"
#include "rfhelp.h"
#include "crc.h"
#include "appconf.h"

// Settings
#define MAX_PL_LEN				25
#define RX_BUFFER_SIZE			1024

#define ALIVE_INTERVAL			50  // Send alive packets at this rate
#define NRF_RESTART_TIMEOUT		1000  // Restart the NRF if nothing has been received or acked for this time

// Variables
static THD_WORKING_AREA(rx_thread_wa, 2048);
static THD_WORKING_AREA(tx_thread_wa, 512);
//static mote_state mstate;
//static uint8_t rx_buffer[RX_BUFFER_SIZE];
static int nosend_cnt;
static int nrf_restart_rx_time;
static int nrf_restart_tx_time;

static systime_t pairing_time_end = 0;
static bool pairing_active = false;

static volatile bool tx_running = false;
static volatile bool tx_stop = true;
static volatile bool rx_running = false;
static volatile bool rx_stop = true;

// This is a hack to prevent race conditions when updating the appconf
// from the nrf thread
static volatile bool from_nrf = false;

// Functions
static THD_FUNCTION(rx_thread, arg);
static THD_FUNCTION(tx_thread, arg);
static int rf_tx_wrapper(char *data, int len);

extern uint32_t init_errors;
extern SerialUSBDriver SDU1;

char nrf_driver_init(void) {
    rfhelp_init();

	nosend_cnt = 0;
	nrf_restart_rx_time = 0;
	nrf_restart_tx_time = 0;

	pairing_time_end = 0;
	pairing_active = false;

	rx_stop = false;
	tx_stop = false;
#ifdef RX_MODE

	chThdCreateStatic(rx_thread_wa, sizeof(rx_thread_wa), NORMALPRIO - 1, rx_thread, NULL);
#else

	chThdCreateStatic(tx_thread_wa, sizeof(tx_thread_wa), NORMALPRIO - 1, tx_thread, NULL);
#endif

	rx_running = true;
	tx_running = true;

	return true;
}

void nrf_driver_stop(void) {
	if (from_nrf) {
		return;
	}

	tx_stop = true;
	rx_stop = true;

	if (rx_running || tx_running) {
		rfhelp_stop();
	}

	while (rx_running || tx_running) {
		chThdSleepMilliseconds(1);
	}
}

//void nrf_driver_start_pairing(int ms) {
//		nrf_config conf;;
//	if (!rx_running) {
//		return;
//	}
//
//	pairing_time_end = chVTGetSystemTimeX() + MS2ST(ms);
//
//	if (!pairing_active) {
//		pairing_active = true;
//
//		conf.address[0] = 0xC6;
//		conf.address[1] = 0xC5;
//		conf.address[2] = 0x0;
//		conf.channel = 124;
//		conf.crc_type = NRF_CRC_1B;
//		conf.retries = 3;
//		conf.retry_delay = NRF_RETR_DELAY_1000US;
//		conf.send_crc_ack = true;
//		conf.speed = NRF_SPEED_250K;
//
//		rfhelp_update_conf();
//	}
//}

static int rf_tx_wrapper(char *data, int len) {
	int res = rfhelp_send_data_crc(data, len);

	if (res == 0) {
		nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
	}

	return res;
}

static THD_FUNCTION(tx_thread, arg) {
	(void)arg;
	char cnt;
	chRegSetThreadName("Nrf TX");
	tx_running = true;

	for(;;) {
		nosend_cnt++;
		cnt++;
		if(cnt>'9')
		  cnt = '0';
//		cnt+='0';
//		if (nosend_cnt >= ALIVE_INTERVAL) {
			uint8_t pl[5];
			int32_t index = 0;
            pl[index++] = 'A';
            pl[index++] = 'B';
            pl[index++] = cnt;
			rf_tx_wrapper((char*)pl, index);
			nosend_cnt = 0;
//		}

		chThdSleepMilliseconds(100);
	}

}

static THD_FUNCTION(rx_thread, arg) {
	(void)arg;

	int res;
	chRegSetThreadName("Nrf RX");
	rx_running = true;

	for(;;) {
//		if (rx_stop) {
//			rx_running = false;
//			return;
//		}

		uint8_t buf[16];
		int len;
		int pipe;

		for(;;) {
			res = rfhelp_read_rx_data_crc((char*)buf, &len, &pipe);

			// If something was read
			if (res >= 0) {
              chprintf((BaseSequentialStream *)&SDU1, "NRF RX0 : 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\r\n",buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8]);
				nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
			}
			pipe = 1;
            res = rfhelp_read_rx_data_crc((char*)buf, &len, &pipe);
            if (res >= 0) {
              chprintf((BaseSequentialStream *)&SDU1, "NRF RX1 : 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\r\n",buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8]);
                nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
            }

			// Stop when there is no more data to read.
			if (res <= 0) {
				break;
			} else {
				// Sleep a bit to prevent locking the other threads.
				chThdSleepMilliseconds(1);
			}
		}

		chThdSleepMilliseconds(5);

		// Restart the nrf if nothing has been received for a while
		if (nrf_restart_rx_time > 0 && nrf_restart_tx_time > 0) {
			nrf_restart_rx_time -= 5;
			nrf_restart_tx_time -= 5;
		} else {
          chprintf((BaseSequentialStream *)&SDU1, "\033[1;31mNRF Restarting\033[0m\r\n");
			rfhelp_power_up();
			rfhelp_restart();
			nrf_restart_rx_time = NRF_RESTART_TIMEOUT;
			nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
		}
	}
}

void nrf_driver_send_buffer(unsigned char *data, unsigned int len) {
  unsigned int i;
	uint8_t send_buffer[MAX_PL_LEN];
    unsigned short crc = crc16(data, len);

	if (len <= (MAX_PL_LEN - 1)) {
		uint32_t ind = 0;
		send_buffer[ind++] = 0x04;
		memcpy(send_buffer + ind, data, len);
		ind += len;
		rf_tx_wrapper((char*)send_buffer, ind);
		nosend_cnt = 0;
	} else {
		unsigned int end_a = 0;
		unsigned int len2 = len - (MAX_PL_LEN - 5);

		for (i = 0;i < len2;i += (MAX_PL_LEN - 2)) {
			if (i > 255) {
				break;
			}

			end_a = i + (MAX_PL_LEN - 2);

			uint8_t send_len = (MAX_PL_LEN - 2);
			send_buffer[0] = 0x03;
			send_buffer[1] = i;

			if ((i + (MAX_PL_LEN - 2)) <= len2) {
				memcpy(send_buffer + 2, data + i, send_len);
			} else {
				send_len = len2 - i;
				memcpy(send_buffer + 2, data + i, send_len);
			}

			rf_tx_wrapper((char*)send_buffer, send_len + 2);
			nosend_cnt = 0;
		}

		for (i = end_a;i < len2;i += (MAX_PL_LEN - 3)) {
			uint8_t send_len = (MAX_PL_LEN - 3);
			send_buffer[0] = 0x02;
			send_buffer[1] = i >> 8;
			send_buffer[2] = i & 0xFF;

			if ((i + (MAX_PL_LEN - 3)) <= len2) {
				memcpy(send_buffer + 3, data + i, send_len);
			} else {
				send_len = len2 - i;
				memcpy(send_buffer + 3, data + i, send_len);
			}

			rf_tx_wrapper((char*)send_buffer, send_len + 3);
			nosend_cnt = 0;
		}

		uint32_t ind = 0;
		send_buffer[ind++] = 0x01;
		send_buffer[ind++] = len >> 8;
		send_buffer[ind++] = len & 0xFF;
		crc = crc16(data, len);
		send_buffer[ind++] = (uint8_t)(crc >> 8);
		send_buffer[ind++] = (uint8_t)(crc & 0xFF);
		memcpy(send_buffer + 5, data + len2, len - len2);
		ind += len - len2;

		rf_tx_wrapper((char*)send_buffer, ind);
		nosend_cnt = 0;
	}
}
