/* sysMipsLib.c - MIPS system-dependent routines */

/* Copyright 2001 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: sysMipsLib.c 1.2.70.1 Broadcom SDK $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01g,13mar02,agf  replace (SM_OBJ || SM_NET) with SM_COMMON, SPR 74321
01f,27feb02,pes  Fix conditional to include sysProcNumSet
01e,17jan02,tlc  Add call to vxTas() to sysBusTas() (SPR #70336 fix).
01d,16jul01,tlc  Add CofE copyright.
01c,27jun01,tlc  Add conditional for (INCLUDE_SM_OBJ) and (INCLUDE_SM_NET) for
                 sysLocalToBusAdrs, sysBusToLocalAdrs, sysBusIntAck, and
                 sysBusIntGen.
01b,15jun01,tlc  Make each routine conditionally compiled so that BSPs may
                 define thier own routines, if necessary.
01a,13jun01,tlc  Add sysIntDisable().  Remove sysBusTas().
*/

/*
DESCRIPTION
DO NOT EDIT THIS FILE.

This library provides board-specific routines that are shared by *all* MIPS-based
BSPs.  MIPS BSPs utilize this file by creating a symbolic link from their
directory to target/config/mipsCommon/sysMipsLib.c and include the file at the 
top of sysLib.c using

	#include "sysMipsLib.c"

A list of provided routines follows.  If a BSP requires a specialized routine,
then #define the appropriate MACRO corresponding to the routine to be
specialized in the BSPs sysLib.c file.  

       ROUTINE              MACRO
       ------------------------------------------------------
       sysProcNumGet        SYS_PROC_NUM_GET
       sysProcNumSet        SYS_PROC_NUM_SET
       sysBusEid            SYS_BUS_EID
       sysBusEar            SYS_BUS_EAR
       sysClearTlb          SYS_CLEAR_TLB
       sysMaskVmeErr        SYS_MASK_VME_ERR
       sysUnmaskVmeErr      SYS_UNMASK_VMR_ERR
       sysLocalToBusAdrs    SYS_LOCAL_TO_BUS_ADRS
       sysBusToLocalAdrs    SYS_BUS_TO_LOCAL_ADRS
       sysBusIntAck         SYS_BUS_INT_ACK
       sysBusIntGen         SYS_BUS_INT_GEN
       sysIntEnable         SYS_INT_ENABLE
       sysIntDisable        SYS_INT_DISABLE
       sysSw0Gen            SYS_SW0_GEN
       sysSw1Gen            SYS_SW1_GEN
       sysSw0Ack            SYS_SW0_ACK
       sysSw1Ack            SYS_SW1_ACK
       sysBusTas            SYS_BUS_TAS

*/

/* globals */
int   sysBus      = BUS;                /* system bus type (VME_BUS, etc) */
int   sysCpu      = CPU;                /* system CPU type (MIPS_Vr5400) */
char *sysMemTopAdr= NULL;		/* top of memory */
char *sysBootLine = BOOT_LINE_ADRS;	/* address of boot line */
char *sysExcMsg   = EXC_MSG_ADRS;	/* catastrophic message area */
int   sysProcNum;			/* processor number of this CPU */
int   sysFlags;				/* boot flags */
char  sysBootHost [BOOT_FIELD_LEN];	/* name of host from which we booted */
char  sysBootFile [BOOT_FIELD_LEN];	/* name of file from which we booted */

/* externals */
IMPORT BOOL vxTas();

/* Initialize cache function pointer */
LOCAL STATUS      sysCacheInit ();
FUNCPTR sysCacheLibInit = (FUNCPTR) sysCacheInit;

#ifndef SYS_PROC_NUM_GET
/******************************************************************************
*
* sysProcNumGet - get the processor number
*
* This routine returns the processor number for the CPU board, which is
* set with sysProcNumSet().
*
* RETURNS: The processor number for the CPU board.
*
* SEE ALSO: sysProcNumSet()
*/

int sysProcNumGet (void)
    {
    return (sysProcNum);
    }
#endif

#ifndef SYS_PROC_NUM_SET
/******************************************************************************
*
* sysProcNumSet - set the processor number
*
* This routine sets the processor number for this board.  Processor numbers
* should be unique on a single backplane.
*
* RETURNS: N/A
*
* SEE ALSO: sysProcNumGet()
*/

void sysProcNumSet
    (
    int procNum
    )
    {
    sysProcNum = procNum;
    }
#endif

#ifndef SYS_BUS_EID
/******************************************************************************
*
* sysBusEid - get the value of the error ID register
*
* This routine returns the contents of the bus error status register
* MIPS devices have no such register, so they simply return zero.
*
* RETURNS: 0, always.
*/

USHORT sysBusEid (void)
    {
    return (0);
    }
#endif

#ifndef SYS_BUS_EAR
/******************************************************************************
*
* sysBusEar - get the access address of a bus error
*
* This routine returns the address of a bus error.
*
* NOTE:
* This routine must be provided on all MIPS board support packages.
* MIPS devices cannot determine the address that caused
* a bus error.  It is possible to determine the source of a read bus error
* by interpreting the instruction stream that caused the bus error.
*
* RETURNS: -1, always.
*/

ULONG sysBusEar (void)
    {
    return (-1);
    }
#endif

#ifndef SYS_CLEAR_TLB
/******************************************************************************
*
* sysClearTlb - clear the translation lookaside buffer
*
* This routine clears the entries in the translation lookaside buffer (TLB)
* for the MIPS devices.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void sysClearTlb (void)
   {
   int tlbEntry;

   for (tlbEntry = 0; tlbEntry < TLB_ENTRIES; tlbEntry++)
       sysClearTlbEntry (tlbEntry);
   }
#endif

#ifndef SYS_MASK_VME_ERR
/******************************************************************************
*
* sysMaskVmeErr - mask the VMEbus error interrupt
*
* This routine is required for all MIPS BSPs.  It has no effect.
*
* RETURNS: 0.
*
* NOMANUAL
*/

UINT8 sysMaskVmeErr (void)
    {
    return (0);
    }
#endif

#ifndef SYS_UNMASK_VME_ERR
/******************************************************************************
*
* sysUnmaskVmeErr - unmask the VMEbus error interrupt
*
* This routine is required for all MIPS BSPs.  It has no effect.
*
* RETURNS: 0.
*
* NOMANUAL
*/

UINT8 sysUnmaskVmeErr (void)
    {
    return (0);
    }
#endif

#ifdef INCLUDE_SM_COMMON

#ifndef SYS_LOCAL_TO_BUS_ADRS
/******************************************************************************
*
* sysLocalToBusAdrs - convert a local address to a bus address
*
* Not Implemented
*
*  NOMANUAL
*/

STATUS sysLocalToBusAdrs
    (
    int  adrsSpace,     /* bus address space in which busAdrs resides,  */
                        /* use address modifier codes defined in vme.h, */
                        /* such as VME_AM_STD_SUP_DATA                  */
    char *localAdrs,    /* local address to convert                     */
    char **pBusAdrs     /* where to return bus address                  */
    )
    {
    return (ERROR);
    }
#endif

#ifndef SYS_BUS_TO_LOCAL_ADRS
/******************************************************************************
*
* sysBusToLocalAdrs - convert a bus address to a local address
*
* Not Implemented
*
* NOMANUAL 
*/
 
STATUS sysBusToLocalAdrs
    (
    int  adrsSpace,     /* bus address space in which busAdrs resides,  */
                        /* use address modifier codes defined in vme.h, */
                        /* such as VME_AM_STD_SUP_DATA                  */
    char *busAdrs,      /* bus address to convert                       */
    char **pLocalAdrs   /* where to return local address                */
    )
    {
    return (ERROR);
    }
#endif

#ifndef SYS_BUS_INT_ACK
/******************************************************************************
*
* sysBusIntAck - acknowledge a bus interrupt
*
* Not Implemented
*
* NOMANUAL
*/

int sysBusIntAck
    (
    int intLevel	/* interrupt level to acknowledge */
    )
    {
    return(ERROR);    
    }
#endif

#ifndef SYS_BUS_INT_GEN
/******************************************************************************
*
* sysBusIntGen - generate a bus interrupt
*
* Not Implemented
* RETURNS: ERROR, since there is no external bus.
*/

STATUS sysBusIntGen
    (
    int level,          /* bus interrupt level to generate          */
    int vector          /* interrupt vector to return (0-255)       */
    )
    {
    return (ERROR);
    }
#endif

#endif	/* INCLUDE_SM_COMMON */

#ifndef SYS_INT_ENABLE
/******************************************************************************
*
* sysIntEnable - enable a bus interrupt level
*
* Not Implemented
*
* NOMANUAL 
*/
 
STATUS sysIntEnable
    (
    int intLevel       /* interrupt level to enable (1-7) */
    )
    {
    return (ERROR);
    }
#endif

#ifndef SYS_INT_DISABLE
/******************************************************************************
*
* sysIntDisable - disable a bus interrupt level
*
* Not Implemented
*
* NOMANUAL 
*/
 
STATUS sysIntDisable
    (
    int intLevel       /* interrupt level to disable (1-7) */
    )
    {
    return (ERROR);
    }
#endif

#ifndef SYS_SW0_GEN
/*******************************************************************************
*
* sysSw0Gen - generate software interrupt 0
*
* This routine writes to the MIPS cause register to generate a software
* interrupt.
*
* RETURNS: N/A
*/

void sysSw0Gen (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg |= CAUSE_SW1;
    intCRSet (causeReg);
    }
#endif

#ifndef SYS_SW1_GEN
/*******************************************************************************
*
* sysSw1Gen - generate software interrupt 1
*
* This routine writes to the MIPS cause register to generate a software
* interrupt.
*
* RETURNS: N/A
*/

void sysSw1Gen (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg |= CAUSE_SW2;
    intCRSet (causeReg);
    }
#endif

#ifndef SYS_SW0_ACK
/*******************************************************************************
*
* sysSw0Ack - acknowledge software interrupt 0
*
* This routine writes to the MIPS cause register to acknowledge a software
* interrupt.
*
* NOTE:
* This routine is provided as a default interrupt service routine.
*
* RETURNS: N/A
*/

LOCAL int sysSw0Ack (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg &= ~CAUSE_SW1;
    intCRSet (causeReg);

    return (OK);
    }
#endif

#ifndef SYS_SW1_ACK
/*******************************************************************************
*
* sysSw1Ack - acknowledge software interrupt 1 
*
* This routine writes to the MIPS cause register to acknowledge a software
* interrupt.
*
* NOTE:
* This routine is provided as a default interrupt service routine.
*
* RETURNS: N/A
*/

LOCAL int sysSw1Ack (void)
    {
    unsigned causeReg;

    causeReg = intCRGet ();
    causeReg &= ~CAUSE_SW2;
    intCRSet (causeReg);

    return (OK);
    }
#endif

#ifndef SYS_BUS_TAS
/******************************************************************************
*
* sysBusTas - test and set a location across the bus
*
* This routine performs a test-and-set operation across the backplane.
*
* RETURNS: TRUE if the value had not been set but is now, or FALSE if the
* value was set already.
*
* SEE ALSO: vxTas()
*/

BOOL sysBusTas
    (
    char *adrs	/* address to be tested-and-set */
    )
    {
    return (vxTas(adrs));
    }
#endif
