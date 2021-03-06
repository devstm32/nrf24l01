/*
 * appconf.h
 *
 *  Created on: 14-Nov-2017
 *      Author: saag
 */

#ifndef APPCONF_H_
#define APPCONF_H_

//#define SPI_BITBANG FALSE

//#define RX_MODE


// NRF app
#ifndef APPCONF_NRF_SPEED
#define APPCONF_NRF_SPEED                   NRF_SPEED_1M
#endif
#ifndef APPCONF_NRF_POWER
#define APPCONF_NRF_POWER                   NRF_POWER_0DBM
#endif
#ifndef APPCONF_NRF_CRC
#define APPCONF_NRF_CRC                     NRF_CRC_1B
#endif
#ifndef APPCONF_NRF_RETR_DELAY
#define APPCONF_NRF_RETR_DELAY              NRF_RETR_DELAY_250US
#endif
#ifndef APPCONF_NRF_RETRIES
#define APPCONF_NRF_RETRIES                 3
#endif
#ifndef APPCONF_NRF_CHANNEL
#define APPCONF_NRF_CHANNEL                 76
#endif
#ifndef APPCONF_NRF_ADDR_B0
#define APPCONF_NRF_ADDR_B0                 0x25
#endif
#ifndef APPCONF_NRF_ADDR_B1
#define APPCONF_NRF_ADDR_B1                 0x26
#endif
#ifndef APPCONF_NRF_ADDR_B2
#define APPCONF_NRF_ADDR_B2                 0x27
#endif
#ifndef APPCONF_NRF_SEND_CRC_ACK
#define APPCONF_NRF_SEND_CRC_ACK            true
#endif


#endif /* APPCONF_H_ */
