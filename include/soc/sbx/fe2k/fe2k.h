/*
 * $Id: fe2k.h 1.4 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * This file contains aggregated definitions for Guadalupe 2.x microcode
 */

#ifndef _SOC_SBX_FE2K_H
#define _SOC_SBX_FE2K_H

extern int sbFe2000MemSetField(int unit, char *memname, int addr, int val);
extern int sbFe2000MemShow(int unit, char *memname, int rangemin, int rangemax);
extern void sbFe2000ShowMemNames(void);

enum {
        FE2K_MEM_PRPORTTOQUEUEAG0 = 0,
        FE2K_MEM_PRPORTTOQUEUEAG1,
        FE2K_MEM_PRPORTTOQUEUEPCI,
        FE2K_MEM_PRPORTTOQUEUESR0,
        FE2K_MEM_PRPORTTOQUEUESR1,
        FE2K_MEM_PRPORTTOQUEUEXG0,
        FE2K_MEM_PRPORTTOQUEUEXG1,
        FE2K_MEM_PDLRPOUTPUTHEADERCOPYBUFFER,
        FE2K_MEM_PDPDOUTPUTHEADERCOPYBUFFER,
        FE2K_MEM_LRLRPINSTRUCTIONMEMORY,
        FE2K_MEM_QMBUFFERSTATE0,
        FE2K_MEM_QMBUFFERSTATE1,
        FE2K_MEM_QMBUFFERSTATE2,
        FE2K_MEM_QMBUFFERSTATE3,
        FE2K_MEM_QMDEQUEUESTATE0,
        FE2K_MEM_QMDEQUEUESTATE1,
        FE2K_MEM_QMFREEPAGEFIFO,
        FE2K_MEM_QMNEXTBUFFER  ,
        FE2K_MEM_QMNEXTPAGE    ,
        FE2K_MEM_QMQUEUECONFIG ,
        FE2K_MEM_QMQUEUECOUNTERS,
        FE2K_MEM_QMQUEUEHEADPTR,
        FE2K_MEM_QMQUEUESTATE0 ,
        FE2K_MEM_QMQUEUESTATE0EN,
        FE2K_MEM_QMQUEUESTATE1 ,
        FE2K_MEM_QMREPLICATIONSTATE,
        FE2K_MEM_PMCOUNTERMEMORY,
        FE2K_MEM_PMPROFILEMEMORY,
        FE2K_MEM_SR0COUNTER    ,
        FE2K_MEM_SR1COUNTER    ,
        FE2K_MEM_PPAGGREGATEHASHBITCONFIG,
        FE2K_MEM_PPAGGREGATEHASHBYTECONFIG,
        FE2K_MEM_PPCAMCONFIGURATIONINSTANCE0,
        FE2K_MEM_PPCAMCONFIGURATIONINSTANCE1,
        FE2K_MEM_PPCAMCONFIGURATIONINSTANCE2,
        FE2K_MEM_PPCAMCONFIGURATIONINSTANCE3,
        FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE0,
        FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE1,
        FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE2,
        FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE3,
        FE2K_MEM_PPHEADERRECORDSIZE,
        FE2K_MEM_PPINITIALQUEUESTATE,
        FE2K_MEM_PPPPOUTHEADERCOPY,
       FE2K_MEM_PPQUEUEPRIORITYGROUP,
        FE2K_MEM_PPRXPORTDATA  ,
        FE2K_MEM_RC0DATA,
        FE2K_MEM_RC1DATA,
        FE2K_MEM_ST0COUNTER    ,
        FE2K_MEM_ST1COUNTER    ,
        FE2K_MEM_MM0INTERNAL0MEMORY,
        FE2K_MEM_MM0INTERNAL1MEMORY,
        FE2K_MEM_MM0NARROWPORT0MEMORY,
        FE2K_MEM_MM0NARROWPORT1MEMORY,
        FE2K_MEM_MM0WIDEPORTMEMORY,
        FE2K_MEM_MM1INTERNAL0MEMORY,
        FE2K_MEM_MM1INTERNAL1MEMORY,
        FE2K_MEM_MM1NARROWPORT0MEMORY,
        FE2K_MEM_MM1NARROWPORT1MEMORY,
        FE2K_MEM_MM1WIDEPORTMEMORY,
        FE2K_MEM_PB0DATA,
        FE2K_MEM_PB1DATA,
        FE2K_MEM_PBCOUNTERMEMORY,
        FE2K_MEM_PTMIRRORINDEX ,
        FE2K_MEM_PTPORTTOQUEUEAG0,
        FE2K_MEM_PTPORTTOQUEUEAG1,
        FE2K_MEM_PTPORTTOQUEUEPCI,
        FE2K_MEM_PTPORTTOQUEUEST0,
        FE2K_MEM_PTPORTTOQUEUEST1,
        FE2K_MEM_PTPORTTOQUEUEXG0,
        FE2K_MEM_PTPORTTOQUEUEXG1,
        FE2K_MEM_PTPTE0MIRRORPORTSTATE,
        FE2K_MEM_PTPTE0PORTSTATE,
        FE2K_MEM_PTPTE1MIRRORPORTSTATE,
        FE2K_MEM_PTPTE1PORTSTATE,
        FE2K_MEM_PTPTE2MIRRORPORTSTATE,
        FE2K_MEM_PTPTE2PORTSTATE,
        FE2K_MEM_PTPTE3MIRRORPORTSTATE,
        FE2K_MEM_PTPTE3PORTSTATE,
        FE2K_MEM_PTPTE4MIRRORPORTSTATE,
        FE2K_MEM_PTPTE4PORTSTATE,
        FE2K_MEM_PTPTE5MIRRORPORTSTATE,
        FE2K_MEM_PTPTE5PORTSTATE,
        FE2K_MEM_PTPTE6MIRRORPORTSTATE,
        FE2K_MEM_PTPTE6PORTSTATE,
        FE2K_MEM_PTQUEUETOPORT ,
        FE2K_MEM_MAX_INDEX
};



#endif  /* !_SOC_SBX_FE2K_H */
