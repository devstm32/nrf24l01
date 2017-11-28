
#ifndef RFHELP_H_
#define RFHELP_H_

#include <stdbool.h>
#include "../main.h"

// Functions
char rfhelp_init(void);
void rfhelp_stop(void);
void rfhelp_update_conf(void);
void rfhelp_restart(void);
int rfhelp_send_data(char *data, int len, char ack);
int rfhelp_send_data_crc(char *data, int len);
int rfhelp_read_rx_data(char *data, int *len, int *pipe);
int rfhelp_read_rx_data_crc(char *data, int *len, int *pipe);
int rfhelp_rf_status(void);
void rfhelp_set_tx_addr(const char *addr, int addr_len);
void rfhelp_set_rx_addr(int pipe, const char *addr, int addr_len);
void rfhelp_power_down(void);
void rfhelp_power_up(void);

void printNrfAdd(void);


#endif /* RFHELP_H_ */
