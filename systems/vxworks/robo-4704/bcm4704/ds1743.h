#ifndef __DS1743_H
#define __DS1743_H

/* $Id: ds1743.h 1.1.490.1 Broadcom SDK $
 * DS1743 -Timekeeping Battery backed SRAM.
 */

STATUS ds1743_tod_init(UINT8 *addr);

STATUS ds1743_tod_set(int year,
		   int month,
		   int day,
		   int hour,
		   int minute,
		   int second);

STATUS ds1743_tod_get(int *year,
		   int *month,
		   int *day,
		   int *hour,
		   int *minute,
		   int *second);

int ds1743_tod_get_second(void);

#endif /*!__DS1743_H */
