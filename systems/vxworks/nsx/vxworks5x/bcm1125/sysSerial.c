/* sysSerial.c - template BSP serial device initialization */
/* Copyright 1984-2002 Wind River Systems, Inc. */
/* $Id: sysSerial.c 1.2.436.1 Broadcom SDK $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,15nov01,agf  writen
*/

/*
DESCRIPTION
This file contains the board-specific routines for serial channel
initialization.

During development, it is easiest to #include the
generic driver (in this case "src/drv/sio/templateSio.c").
Once the BSP initialization code is working with the generic
driver, it should no longer be #included, but rather should be
linked from the library archives.
*/

#include "vxWorks.h"
#include "config.h"

#include "intLib.h"
#include "iv.h"
#include "sysLib.h"

#include "bcm1250DuartSio.h"
#include "bcm1250JTAGSio.h"

/* static variables */

#if defined(INCLUDE_BCM1250_UART_CHAN_A) || defined(INCLUDE_BCM1250_UART_CHAN_B)
static BCM1250_DUART_CHAN	sysSioChan[2];  /* indexed by hardware # */
#endif

#if defined(INCLUDE_BCM1250_JTAG_CHAN_A)
static BCM1250_JTAG_CHAN		sysJTAGChan[1];   /* indexed by hardware # */
#endif


/* definitions */


/******************************************************************************
*
* sysSerialHwInit - initialize the BSP serial devices to a quiescent state
*
* This routine initializes the BSP serial device descriptors and puts the
* devices in a quiescent state.  It is called from sysHwInit() with
* interrupts locked.
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit()
*/ 

void sysSerialHwInit (void)
    {

#ifdef INCLUDE_BCM1250_UART_CHAN_A
      BCM1250_DUART_CHAN * pChannelA;
#endif

#ifdef INCLUDE_BCM1250_UART_CHAN_B
      BCM1250_DUART_CHAN * pChannelB;
#endif

#ifdef INCLUDE_BCM1250_JTAG_CHAN_A
      BCM1250_JTAG_CHAN * pJTAGA;
#endif

#ifdef INCLUDE_BCM1250_UART_CHAN_A
      pChannelA = (BCM1250_DUART_CHAN * ) sysSerialChanGet(BCM1250_UART_CHAN_A_IDX);
    /* let the driver perform generic chip initialization and reset */

      pChannelA->channel = BCM1250_DUART_CHANNEL_A;
      pChannelA->baudRate = BCM1250_UART_DEFAULT_BAUD;
      pChannelA->intVecNum = IV_INT1_VEC;
      bcm1250DuartDevInit(pChannelA);
#endif

#ifdef INCLUDE_BCM1250_UART_CHAN_B
      pChannelB = (BCM1250_DUART_CHAN * ) sysSerialChanGet(BCM1250_UART_CHAN_B_IDX);
    /* let the driver perform generic chip initialization and reset */

      pChannelB->channel = BCM1250_DUART_CHANNEL_B;
      pChannelB->baudRate = BCM1250_UART_DEFAULT_BAUD;
      pChannelB->intVecNum = IV_INT3_VEC;
      bcm1250DuartDevInit(pChannelB);
#endif      

#ifdef INCLUDE_BCM1250_JTAG_CHAN_A
      pJTAGA = (BCM1250_JTAG_CHAN * ) sysSerialChanGet(BCM1250_JTAG_CHAN_A_IDX);
    /* let the driver perform generic chip initialization and reset */

      pJTAGA->channel = JTAG_CHANNEL_A;
      pJTAGA->base_reg = 0xB001FFA0;
      bcm1250JTAGDevInit(pJTAGA);
#endif
    }

/******************************************************************************
*
* sysSerialHwInit2 - connect BSP serial device interrupts
*
* This routine connects the BSP serial device interrupts.  It is called from
* sysHwInit2().  
*
* Serial device interrupts cannot be connected in sysSerialHwInit() because
* the  kernel memory allocator is not initialized at that point and
* intConnect() calls malloc().
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit2()
*/ 

void sysSerialHwInit2 (void)
    {

#ifdef INCLUDE_BCM1250_UART_CHAN_A
      BCM1250_DUART_CHAN * pChannelA;
#endif

#ifdef INCLUDE_BCM1250_UART_CHAN_B
      BCM1250_DUART_CHAN * pChannelB;
#endif

#ifdef INCLUDE_BCM1250_JTAG_CHAN_A
      BCM1250_JTAG_CHAN * pJTAGA;
#endif

#ifdef INCLUDE_BCM1250_UART_CHAN_A
      pChannelA = (BCM1250_DUART_CHAN * ) sysSerialChanGet(BCM1250_UART_CHAN_A_IDX);
    /* second stage initialization will activate interrupt operation */
      bcm1250DuartDevInit2(pChannelA);
#endif

#ifdef INCLUDE_BCM1250_UART_CHAN_B
      pChannelB = (BCM1250_DUART_CHAN * ) sysSerialChanGet(BCM1250_UART_CHAN_B_IDX);
    /* second stage initialization will activate interrupt operation */
      bcm1250DuartDevInit2(pChannelB);
#endif

#ifdef INCLUDE_BCM1250_JTAG_CHAN_A
      pJTAGA = (BCM1250_JTAG_CHAN * ) sysSerialChanGet(BCM1250_JTAG_CHAN_A_IDX);
    /* second stage initialization will activate interrupt operation */
      bcm1250JTAGDevInit2(pJTAGA);
#endif
    }

/******************************************************************************
*
* sysSerialChanGet - get the SIO_CHAN device associated with a serial channel
*
* This routine returns a pointer to the SIO_CHAN device associated
* with a specified serial channel.  It is called by usrRoot() to obtain 
* pointers when creating the system serial devices, `/tyCo/x'.  It
* is also used by the WDB agent to locate its serial channel.
*
* RETURNS: A pointer to the SIO_CHAN structure for the channel, or ERROR
* if the channel is invalid.
*/

SIO_CHAN * sysSerialChanGet
    (
    int channel         /* serial channel */
    )
    {

#ifdef INCLUDE_BCM1250_UART_CHAN_A
    if (channel == BCM1250_UART_CHAN_A_IDX)
	return (SIO_CHAN *) &sysSioChan[0];
#endif

#ifdef INCLUDE_BCM1250_UART_CHAN_B
    if (channel == BCM1250_UART_CHAN_B_IDX)
	return (SIO_CHAN *) &sysSioChan[1];
#endif

#ifdef INCLUDE_BCM1250_JTAG_CHAN_A
    if (channel == BCM1250_JTAG_CHAN_A_IDX)
	return (SIO_CHAN *) &sysJTAGChan[1];
#endif

    return(SIO_CHAN *) ERROR;
    }

/***********************************************************************
 ***********************************************************************
 * Serial debugging print routines
 *
 * The following routines are for debugging, especially useful in early
 * boot ROM and ISR code.
 *
 * sysSerialPutc
 * sysSerialGetc
 * sysSerialPrintString
 * sysSerialPrintHex
 *
 * The sysSerialPrintString and sysSerialPrintHex routines should
 * basically ALWAYS work.  They re-initialize the UART and turn off
 * interrupts every time.
 */

#define NSX_SERIAL_CHAN   BCM1250_UART_CHAN_A_IDX

#define NSX_DUARTREAD(chan, reg) \
    ((SBREADCSR(A_DUART_CHANREG((chan), R_DUART_MODE_REG_1))), \
     (SBREADCSR(A_DUART_CHANREG((chan), (reg)))))

#define NSX_DUARTWRITE(chan, reg, val) \
    ((SBREADCSR(A_DUART_CHANREG((chan), R_DUART_MODE_REG_1))), \
     (SBWRITECSR(A_DUART_CHANREG((chan), (reg)), val)))

void sysSerialDelay(void)
{
    volatile int i;
    for (i = 0; i < 0x10000; i++)
        ;
}

int sysSerialGetc(void)
{
    BCM1250_DUART_CHAN *pChan;
    char inChar;

    /* call bcm1250DuartPollInput */
    pChan = (BCM1250_DUART_CHAN *) sysSerialChanGet(NSX_SERIAL_CHAN);
    while (pChan->sio.pDrvFuncs->pollInput((SIO_CHAN*)pChan, &inChar) == EAGAIN)
        ;
    return (inChar);
#if 0
    while (!(NSX_DUARTREAD(NSX_SERIAL_CHAN, R_DUART_STATUS) & M_DUART_RX_RDY))
        ;
    return ((NSX_DUARTREAD(NSX_SERIAL_CHAN, R_DUART_RX_HOLD) & 0xFF));
#endif
}

void sysSerialPutc(int c)
{
    BCM1250_DUART_CHAN * pChan;
    char outC;
    int i = 10000;

    /* call bcm1250DuartPollOutput */
    outC = (char)c;
    pChan = (BCM1250_DUART_CHAN *) sysSerialChanGet(NSX_SERIAL_CHAN);
    while (pChan->sio.pDrvFuncs->pollOutput((SIO_CHAN*)pChan, outC) == EAGAIN
           && i--) ;
#if 0
    while (!(NSX_DUARTREAD(NSX_SERIAL_CHAN, R_DUART_STATUS) & M_DUART_TX_RDY)
           && i--) ;
    NSX_DUARTWRITE(NSX_SERIAL_CHAN, R_DUART_TX_HOLD, c);
#endif
}

void sysSerialPrintString(char *s)
{
    int c, il;

    il = intLock();
    while ((c = *s++) != 0) {
        if (c == '\n')
            sysSerialPutc('\r');
        sysSerialPutc(c);
    }
    sysSerialDelay();   /* Allow last char to flush */
    intUnlock(il);
}

void sysSerialPrintHex(UINT32 value, int cr)
{
    const char hex[] = "0123456789abcdef";
    int i, il;

    il = intLock();
    for (i = 0; i < 8; i++)
        sysSerialPutc(hex[value >> (28 - i * 4) & 0xf]);
    if (cr) {
        sysSerialPutc('\r');
        sysSerialPutc('\n');
    }
    sysSerialDelay();   /* Allow last char to flush */
    intUnlock(il);
}
