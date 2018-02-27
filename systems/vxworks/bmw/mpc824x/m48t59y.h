#ifndef __M48_T59_Y_H
#define __M48_T59_Y_H

/* $Id: m48t59y.h 1.1.570.1 Broadcom SDK $
 * M48 T59Y -Timekeeping Battery backed SRAM.
 */

STATUS m48_tod_init(UINT8 *addr);

STATUS m48_tod_set(int year,
		   int month,
		   int day,
		   int hour,
		   int minute,
		   int second);

STATUS m48_tod_get(int *year,
		   int *month,
		   int *day,
		   int *hour,
		   int *minute,
		   int *second);

int m48_tod_get_second(void);

void m48_watchdog_arm(int usec);

#endif /*!__M48_T59_Y_H */
