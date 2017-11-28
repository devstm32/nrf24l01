/*General tool for measuring ADC/USB2Serial/*/

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"
#include "main.h"
#include "usbcfg.h"
#include "nrf/rfhelp.h"

/* Virtual serial port over USB.*/
SerialUSBDriver SDU1;
static BaseSequentialStream *usbchp;

uint32_t encv;
uint32_t init_errors;

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
#define USB2SER_WA_SIZE    THD_WORKING_AREA_SIZE(512)


static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  size_t n, size;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}


static void encoderData(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: enc\r\n");
    return;
  }
  while(1)
  {
    encv = qeiGetCount(&QEID4);
    chprintf(chp, "Value : %5d\r",encv);
    chThdSleepMilliseconds(500);
  }
}

static void statusApp(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: enc\r\n");
    return;
  }
  chprintf((BaseSequentialStream *)&SDU1, "Error: %x\r\n",init_errors);
  printNrfAdd();
}
static void statusThreads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%08lx %08lx %4lu %4lu %9s\r\n",
             (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
             (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
             states[tp->p_state]);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}


static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"enc", encoderData},
  {"sts", statusApp},
  {"thd", statusThreads},

  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

/*
 * Blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static __attribute__((noreturn)) THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    systime_t time = serusbcfg.usbp->state == USB_ACTIVE ? 250 : 1000;
    palClearPad(GPIOC, 13);
    chThdSleepMilliseconds(time);
    palSetPad(GPIOC, 13);
    chThdSleepMilliseconds(time);
  }
}


int __attribute__((noreturn)) main(void) {
  thread_t *shelltp = NULL;

  halInit();
  chSysInit();

  /*Other Init*/
  nrf_driver_init();
  rfhelp_update_conf();


  /*USB COM Init*/
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);
  shellInit();
  /*Threads*/
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*Inti-Over Brodcast it*/
//  lcd_print(0x80,"Init Over");
  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
      shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    else if (chThdTerminatedX(shelltp)) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
    chThdSleepMilliseconds(1000);
  }
}

//void trace(void )
