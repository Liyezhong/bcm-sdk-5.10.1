/*
 * $Id: sbZfFabBm9600InaNmPriorityUpdate.c 1.1.44.4 Broadcom SDK $
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
 */


#include "sbTypes.h"
#include "sbZfFabBm9600InaNmPriorityUpdate.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm9600InaNmPriorityUpdate_Pack(sbZfFabBm9600InaNmPriorityUpdate_t *pFrom,
                                      uint8_t *pToData,
                                      uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_INANMPRIUPDATE_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uIna */
  (pToData)[6] |= ((pFrom)->m_uIna & 0x01) <<7;
  (pToData)[5] |= ((pFrom)->m_uIna >> 1) & 0x3f;

  /* Pack Member: m_uSystemPort */
  (pToData)[7] |= ((pFrom)->m_uSystemPort & 0x1f) <<3;
  (pToData)[6] |= ((pFrom)->m_uSystemPort >> 5) & 0x7f;

  /* Pack Member: m_uEset */
  (pToData)[0] |= ((pFrom)->m_uEset & 0x7f) <<1;
  (pToData)[7] |= ((pFrom)->m_uEset >> 7) & 0x07;

  /* Pack Member: m_uPortSetAddress */
  (pToData)[1] |= ((pFrom)->m_uPortSetAddress & 0x7f) <<1;
  (pToData)[0] |= ((pFrom)->m_uPortSetAddress >> 7) & 0x01;

  /* Pack Member: m_uPortSetOffset */
  (pToData)[2] |= ((pFrom)->m_uPortSetOffset & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_uPortSetOffset >> 3) & 0x01;

  /* Pack Member: m_bNoCriticalUpdate */
  (pToData)[2] |= ((pFrom)->m_bNoCriticalUpdate & 0x01) <<4;

  /* Pack Member: m_bCriticalUpdate */
  (pToData)[2] |= ((pFrom)->m_bCriticalUpdate & 0x01) <<3;

  /* Pack Member: m_bMulticast */
  (pToData)[2] |= ((pFrom)->m_bMulticast & 0x01) <<2;

  /* Pack Member: m_uPriority */
  (pToData)[3] |= ((pFrom)->m_uPriority & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_uPriority >> 2) & 0x03;

  /* Pack Member: m_bMaxPriority */
  (pToData)[3] |= ((pFrom)->m_bMaxPriority & 0x01) <<5;

  /* Pack Member: m_uNextPriority */
  (pToData)[3] |= ((pFrom)->m_uNextPriority & 0x0f) <<1;

  /* Pack Member: m_bNextMaxPriority */
  (pToData)[3] |= ((pFrom)->m_bNextMaxPriority & 0x01);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_INANMPRIUPDATE_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uIna */
  (pToData)[5] |= ((pFrom)->m_uIna & 0x01) <<7;
  (pToData)[6] |= ((pFrom)->m_uIna >> 1) & 0x3f;

  /* Pack Member: m_uSystemPort */
  (pToData)[4] |= ((pFrom)->m_uSystemPort & 0x1f) <<3;
  (pToData)[5] |= ((pFrom)->m_uSystemPort >> 5) & 0x7f;

  /* Pack Member: m_uEset */
  (pToData)[3] |= ((pFrom)->m_uEset & 0x7f) <<1;
  (pToData)[4] |= ((pFrom)->m_uEset >> 7) & 0x07;

  /* Pack Member: m_uPortSetAddress */
  (pToData)[2] |= ((pFrom)->m_uPortSetAddress & 0x7f) <<1;
  (pToData)[3] |= ((pFrom)->m_uPortSetAddress >> 7) & 0x01;

  /* Pack Member: m_uPortSetOffset */
  (pToData)[1] |= ((pFrom)->m_uPortSetOffset & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_uPortSetOffset >> 3) & 0x01;

  /* Pack Member: m_bNoCriticalUpdate */
  (pToData)[1] |= ((pFrom)->m_bNoCriticalUpdate & 0x01) <<4;

  /* Pack Member: m_bCriticalUpdate */
  (pToData)[1] |= ((pFrom)->m_bCriticalUpdate & 0x01) <<3;

  /* Pack Member: m_bMulticast */
  (pToData)[1] |= ((pFrom)->m_bMulticast & 0x01) <<2;

  /* Pack Member: m_uPriority */
  (pToData)[0] |= ((pFrom)->m_uPriority & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_uPriority >> 2) & 0x03;

  /* Pack Member: m_bMaxPriority */
  (pToData)[0] |= ((pFrom)->m_bMaxPriority & 0x01) <<5;

  /* Pack Member: m_uNextPriority */
  (pToData)[0] |= ((pFrom)->m_uNextPriority & 0x0f) <<1;

  /* Pack Member: m_bNextMaxPriority */
  (pToData)[0] |= ((pFrom)->m_bNextMaxPriority & 0x01);
#endif

  return SB_ZF_FAB_BM9600_INANMPRIUPDATE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600InaNmPriorityUpdate_Unpack(sbZfFabBm9600InaNmPriorityUpdate_t *pToStruct,
                                        uint8_t *pFromData,
                                        uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uIna */
  (pToStruct)->m_uIna =  (uint32_t)  ((pFromData)[6] >> 7) & 0x01;
  (pToStruct)->m_uIna |=  (uint32_t)  ((pFromData)[5] & 0x3f) << 1;

  /* Unpack Member: m_uSystemPort */
  (pToStruct)->m_uSystemPort =  (uint32_t)  ((pFromData)[7] >> 3) & 0x1f;
  (pToStruct)->m_uSystemPort |=  (uint32_t)  ((pFromData)[6] & 0x7f) << 5;

  /* Unpack Member: m_uEset */
  (pToStruct)->m_uEset =  (uint32_t)  ((pFromData)[0] >> 1) & 0x7f;
  (pToStruct)->m_uEset |=  (uint32_t)  ((pFromData)[7] & 0x07) << 7;

  /* Unpack Member: m_uPortSetAddress */
  (pToStruct)->m_uPortSetAddress =  (uint32_t)  ((pFromData)[1] >> 1) & 0x7f;
  (pToStruct)->m_uPortSetAddress |=  (uint32_t)  ((pFromData)[0] & 0x01) << 7;

  /* Unpack Member: m_uPortSetOffset */
  (pToStruct)->m_uPortSetOffset =  (uint32_t)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_uPortSetOffset |=  (uint32_t)  ((pFromData)[1] & 0x01) << 3;

  /* Unpack Member: m_bNoCriticalUpdate */
  (pToStruct)->m_bNoCriticalUpdate =  (uint8_t)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_bCriticalUpdate */
  (pToStruct)->m_bCriticalUpdate =  (uint8_t)  ((pFromData)[2] >> 3) & 0x01;

  /* Unpack Member: m_bMulticast */
  (pToStruct)->m_bMulticast =  (uint8_t)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_uPriority */
  (pToStruct)->m_uPriority =  (uint32_t)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_uPriority |=  (uint32_t)  ((pFromData)[2] & 0x03) << 2;

  /* Unpack Member: m_bMaxPriority */
  (pToStruct)->m_bMaxPriority =  (uint8_t)  ((pFromData)[3] >> 5) & 0x01;

  /* Unpack Member: m_uNextPriority */
  (pToStruct)->m_uNextPriority =  (uint32_t)  ((pFromData)[3] >> 1) & 0x0f;

  /* Unpack Member: m_bNextMaxPriority */
  (pToStruct)->m_bNextMaxPriority =  (uint8_t)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uIna */
  (pToStruct)->m_uIna =  (uint32_t)  ((pFromData)[5] >> 7) & 0x01;
  (pToStruct)->m_uIna |=  (uint32_t)  ((pFromData)[6] & 0x3f) << 1;

  /* Unpack Member: m_uSystemPort */
  (pToStruct)->m_uSystemPort =  (uint32_t)  ((pFromData)[4] >> 3) & 0x1f;
  (pToStruct)->m_uSystemPort |=  (uint32_t)  ((pFromData)[5] & 0x7f) << 5;

  /* Unpack Member: m_uEset */
  (pToStruct)->m_uEset =  (uint32_t)  ((pFromData)[3] >> 1) & 0x7f;
  (pToStruct)->m_uEset |=  (uint32_t)  ((pFromData)[4] & 0x07) << 7;

  /* Unpack Member: m_uPortSetAddress */
  (pToStruct)->m_uPortSetAddress =  (uint32_t)  ((pFromData)[2] >> 1) & 0x7f;
  (pToStruct)->m_uPortSetAddress |=  (uint32_t)  ((pFromData)[3] & 0x01) << 7;

  /* Unpack Member: m_uPortSetOffset */
  (pToStruct)->m_uPortSetOffset =  (uint32_t)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_uPortSetOffset |=  (uint32_t)  ((pFromData)[2] & 0x01) << 3;

  /* Unpack Member: m_bNoCriticalUpdate */
  (pToStruct)->m_bNoCriticalUpdate =  (uint8_t)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_bCriticalUpdate */
  (pToStruct)->m_bCriticalUpdate =  (uint8_t)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_bMulticast */
  (pToStruct)->m_bMulticast =  (uint8_t)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_uPriority */
  (pToStruct)->m_uPriority =  (uint32_t)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_uPriority |=  (uint32_t)  ((pFromData)[1] & 0x03) << 2;

  /* Unpack Member: m_bMaxPriority */
  (pToStruct)->m_bMaxPriority =  (uint8_t)  ((pFromData)[0] >> 5) & 0x01;

  /* Unpack Member: m_uNextPriority */
  (pToStruct)->m_uNextPriority =  (uint32_t)  ((pFromData)[0] >> 1) & 0x0f;

  /* Unpack Member: m_bNextMaxPriority */
  (pToStruct)->m_bNextMaxPriority =  (uint8_t)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600InaNmPriorityUpdate_InitInstance(sbZfFabBm9600InaNmPriorityUpdate_t *pFrame) {

  pFrame->m_uIna =  (unsigned int)  0;
  pFrame->m_uSystemPort =  (unsigned int)  0;
  pFrame->m_uEset =  (unsigned int)  0;
  pFrame->m_uPortSetAddress =  (unsigned int)  0;
  pFrame->m_uPortSetOffset =  (unsigned int)  0;
  pFrame->m_bNoCriticalUpdate =  (unsigned int)  0;
  pFrame->m_bCriticalUpdate =  (unsigned int)  0;
  pFrame->m_bMulticast =  (unsigned int)  0;
  pFrame->m_uPriority =  (unsigned int)  0;
  pFrame->m_bMaxPriority =  (unsigned int)  0;
  pFrame->m_uNextPriority =  (unsigned int)  0;
  pFrame->m_bNextMaxPriority =  (unsigned int)  0;

}
