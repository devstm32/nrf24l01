/*
 * nrfdatatype.h
 *
 *  Created on: 18-Nov-2017
 *      Author: 
 */

#ifndef NRF_NRFDATATYPE_H_
#define NRF_NRFDATATYPE_H_

#include <stdbool.h>
// NRF Datatypes
typedef enum {
    NRF_SPEED_250K = 0,
    NRF_SPEED_1M,
    NRF_SPEED_2M
} NRF_SPEED;

typedef enum {
    NRF_POWER_M18DBM = 0,
    NRF_POWER_M12DBM,
    NRF_POWER_M6DBM,
    NRF_POWER_0DBM
} NRF_POWER;

typedef enum {
    NRF_AW_3 = 0,
    NRF_AW_4,
    NRF_AW_5
} NRF_AW;

typedef enum {
    NRF_CRC_DISABLED = 0,
    NRF_CRC_1B,
    NRF_CRC_2B
} NRF_CRC;

typedef enum {
    NRF_RETR_DELAY_250US = 0,
    NRF_RETR_DELAY_500US,
    NRF_RETR_DELAY_750US,
    NRF_RETR_DELAY_1000US,
    NRF_RETR_DELAY_1250US,
    NRF_RETR_DELAY_1500US,
    NRF_RETR_DELAY_1750US,
    NRF_RETR_DELAY_2000US,
    NRF_RETR_DELAY_2250US,
    NRF_RETR_DELAY_2500US,
    NRF_RETR_DELAY_2750US,
    NRF_RETR_DELAY_3000US,
    NRF_RETR_DELAY_3250US,
    NRF_RETR_DELAY_3500US,
    NRF_RETR_DELAY_3750US,
    NRF_RETR_DELAY_4000US
} NRF_RETR_DELAY;

typedef struct {
    NRF_SPEED speed;
    NRF_POWER power;
    NRF_CRC crc_type;
    NRF_RETR_DELAY retry_delay;
    unsigned char retries;
    unsigned char channel;
    unsigned char address[3];
    bool send_crc_ack;
} nrf_config;



#endif /* NRF_NRFDATATYPE_H_ */
