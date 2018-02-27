/*
 * $Id: sbZfKaQmQueueStateEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaQmQueueStateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaQmQueueStateEntry_Pack(sbZfKaQmQueueStateEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nAllocatedBuffsCnt */
  (pToData)[14] |= ((pFrom)->m_nAllocatedBuffsCnt & 0x01) <<7;
  (pToData)[13] |= ((pFrom)->m_nAllocatedBuffsCnt >> 1) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nAllocatedBuffsCnt >> 9) &0xFF;

  /* Pack Member: m_nQTailPtr */
  (pToData)[9] |= ((pFrom)->m_nQTailPtr & 0x03) <<6;
  (pToData)[8] |= ((pFrom)->m_nQTailPtr >> 2) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nQTailPtr >> 10) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nQTailPtr >> 18) & 0x7f;

  /* Pack Member: m_nQHeadPtr */
  (pToData)[4] |= ((pFrom)->m_nQHeadPtr & 0x07) <<5;
  (pToData)[11] |= ((pFrom)->m_nQHeadPtr >> 3) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nQHeadPtr >> 11) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nQHeadPtr >> 19) & 0x3f;

  /* Pack Member: m_nNoBuffsAllocated */
  (pToData)[4] |= ((pFrom)->m_nNoBuffsAllocated & 0x01) <<4;

  /* Pack Member: m_nOverflow */
  (pToData)[4] |= ((pFrom)->m_nOverflow & 0x01) <<3;

  /* Pack Member: m_nMinBuffers */
  (pToData)[6] |= ((pFrom)->m_nMinBuffers & 0x07) <<5;
  (pToData)[5] |= ((pFrom)->m_nMinBuffers >> 3) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nMinBuffers >> 11) & 0x07;

  /* Pack Member: m_nMaxBuffers */
  (pToData)[0] |= ((pFrom)->m_nMaxBuffers & 0x01) <<7;
  (pToData)[7] |= ((pFrom)->m_nMaxBuffers >> 1) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nMaxBuffers >> 9) & 0x1f;

  /* Pack Member: m_nLocal */
  (pToData)[0] |= ((pFrom)->m_nLocal & 0x01) <<6;

  /* Pack Member: m_nQueueDepthInLine16B */
  (pToData)[3] |= ((pFrom)->m_nQueueDepthInLine16B & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nQueueDepthInLine16B >> 3) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nQueueDepthInLine16B >> 11) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nQueueDepthInLine16B >> 19) & 0x3f;

  /* Pack Member: m_nAnemicWatermarkSel */
  (pToData)[3] |= ((pFrom)->m_nAnemicWatermarkSel & 0x07) <<2;

  /* Pack Member: m_nQeType */
  (pToData)[3] |= ((pFrom)->m_nQeType & 0x01) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[3] |= ((pFrom)->m_nEnable & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nAllocatedBuffsCnt */
  (pToData)[13] |= ((pFrom)->m_nAllocatedBuffsCnt & 0x01) <<7;
  (pToData)[14] |= ((pFrom)->m_nAllocatedBuffsCnt >> 1) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nAllocatedBuffsCnt >> 9) &0xFF;

  /* Pack Member: m_nQTailPtr */
  (pToData)[10] |= ((pFrom)->m_nQTailPtr & 0x03) <<6;
  (pToData)[11] |= ((pFrom)->m_nQTailPtr >> 2) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nQTailPtr >> 10) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nQTailPtr >> 18) & 0x7f;

  /* Pack Member: m_nQHeadPtr */
  (pToData)[7] |= ((pFrom)->m_nQHeadPtr & 0x07) <<5;
  (pToData)[8] |= ((pFrom)->m_nQHeadPtr >> 3) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nQHeadPtr >> 11) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nQHeadPtr >> 19) & 0x3f;

  /* Pack Member: m_nNoBuffsAllocated */
  (pToData)[7] |= ((pFrom)->m_nNoBuffsAllocated & 0x01) <<4;

  /* Pack Member: m_nOverflow */
  (pToData)[7] |= ((pFrom)->m_nOverflow & 0x01) <<3;

  /* Pack Member: m_nMinBuffers */
  (pToData)[5] |= ((pFrom)->m_nMinBuffers & 0x07) <<5;
  (pToData)[6] |= ((pFrom)->m_nMinBuffers >> 3) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nMinBuffers >> 11) & 0x07;

  /* Pack Member: m_nMaxBuffers */
  (pToData)[3] |= ((pFrom)->m_nMaxBuffers & 0x01) <<7;
  (pToData)[4] |= ((pFrom)->m_nMaxBuffers >> 1) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nMaxBuffers >> 9) & 0x1f;

  /* Pack Member: m_nLocal */
  (pToData)[3] |= ((pFrom)->m_nLocal & 0x01) <<6;

  /* Pack Member: m_nQueueDepthInLine16B */
  (pToData)[0] |= ((pFrom)->m_nQueueDepthInLine16B & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nQueueDepthInLine16B >> 3) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nQueueDepthInLine16B >> 11) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nQueueDepthInLine16B >> 19) & 0x3f;

  /* Pack Member: m_nAnemicWatermarkSel */
  (pToData)[0] |= ((pFrom)->m_nAnemicWatermarkSel & 0x07) <<2;

  /* Pack Member: m_nQeType */
  (pToData)[0] |= ((pFrom)->m_nQeType & 0x01) <<1;

  /* Pack Member: m_nEnable */
  (pToData)[0] |= ((pFrom)->m_nEnable & 0x01);
#endif

  return SB_ZF_ZFKAQMQUEUESTATEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmQueueStateEntry_Unpack(sbZfKaQmQueueStateEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nAllocatedBuffsCnt */
  (pToStruct)->m_nAllocatedBuffsCnt =  (uint32_t)  ((pFromData)[14] >> 7) & 0x01;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32_t)  (pFromData)[13] << 1;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32_t)  (pFromData)[12] << 9;

  /* Unpack Member: m_nQTailPtr */
  (pToStruct)->m_nQTailPtr =  (uint32_t)  ((pFromData)[9] >> 6) & 0x03;
  (pToStruct)->m_nQTailPtr |=  (uint32_t)  (pFromData)[8] << 2;
  (pToStruct)->m_nQTailPtr |=  (uint32_t)  (pFromData)[15] << 10;
  (pToStruct)->m_nQTailPtr |=  (uint32_t)  ((pFromData)[14] & 0x7f) << 18;

  /* Unpack Member: m_nQHeadPtr */
  (pToStruct)->m_nQHeadPtr =  (uint32_t)  ((pFromData)[4] >> 5) & 0x07;
  (pToStruct)->m_nQHeadPtr |=  (uint32_t)  (pFromData)[11] << 3;
  (pToStruct)->m_nQHeadPtr |=  (uint32_t)  (pFromData)[10] << 11;
  (pToStruct)->m_nQHeadPtr |=  (uint32_t)  ((pFromData)[9] & 0x3f) << 19;

  /* Unpack Member: m_nNoBuffsAllocated */
  (pToStruct)->m_nNoBuffsAllocated =  (uint8_t)  ((pFromData)[4] >> 4) & 0x01;

  /* Unpack Member: m_nOverflow */
  (pToStruct)->m_nOverflow =  (uint8_t)  ((pFromData)[4] >> 3) & 0x01;

  /* Unpack Member: m_nMinBuffers */
  (pToStruct)->m_nMinBuffers =  (uint32_t)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->m_nMinBuffers |=  (uint32_t)  (pFromData)[5] << 3;
  (pToStruct)->m_nMinBuffers |=  (uint32_t)  ((pFromData)[4] & 0x07) << 11;

  /* Unpack Member: m_nMaxBuffers */
  (pToStruct)->m_nMaxBuffers =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;
  (pToStruct)->m_nMaxBuffers |=  (uint32_t)  (pFromData)[7] << 1;
  (pToStruct)->m_nMaxBuffers |=  (uint32_t)  ((pFromData)[6] & 0x1f) << 9;

  /* Unpack Member: m_nLocal */
  (pToStruct)->m_nLocal =  (uint32_t)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nQueueDepthInLine16B */
  (pToStruct)->m_nQueueDepthInLine16B =  (uint32_t)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32_t)  (pFromData)[2] << 3;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32_t)  (pFromData)[1] << 11;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32_t)  ((pFromData)[0] & 0x3f) << 19;

  /* Unpack Member: m_nAnemicWatermarkSel */
  (pToStruct)->m_nAnemicWatermarkSel =  (uint32_t)  ((pFromData)[3] >> 2) & 0x07;

  /* Unpack Member: m_nQeType */
  (pToStruct)->m_nQeType =  (uint32_t)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8_t)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nAllocatedBuffsCnt */
  (pToStruct)->m_nAllocatedBuffsCnt =  (uint32_t)  ((pFromData)[13] >> 7) & 0x01;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32_t)  (pFromData)[14] << 1;
  (pToStruct)->m_nAllocatedBuffsCnt |=  (uint32_t)  (pFromData)[15] << 9;

  /* Unpack Member: m_nQTailPtr */
  (pToStruct)->m_nQTailPtr =  (uint32_t)  ((pFromData)[10] >> 6) & 0x03;
  (pToStruct)->m_nQTailPtr |=  (uint32_t)  (pFromData)[11] << 2;
  (pToStruct)->m_nQTailPtr |=  (uint32_t)  (pFromData)[12] << 10;
  (pToStruct)->m_nQTailPtr |=  (uint32_t)  ((pFromData)[13] & 0x7f) << 18;

  /* Unpack Member: m_nQHeadPtr */
  (pToStruct)->m_nQHeadPtr =  (uint32_t)  ((pFromData)[7] >> 5) & 0x07;
  (pToStruct)->m_nQHeadPtr |=  (uint32_t)  (pFromData)[8] << 3;
  (pToStruct)->m_nQHeadPtr |=  (uint32_t)  (pFromData)[9] << 11;
  (pToStruct)->m_nQHeadPtr |=  (uint32_t)  ((pFromData)[10] & 0x3f) << 19;

  /* Unpack Member: m_nNoBuffsAllocated */
  (pToStruct)->m_nNoBuffsAllocated =  (uint8_t)  ((pFromData)[7] >> 4) & 0x01;

  /* Unpack Member: m_nOverflow */
  (pToStruct)->m_nOverflow =  (uint8_t)  ((pFromData)[7] >> 3) & 0x01;

  /* Unpack Member: m_nMinBuffers */
  (pToStruct)->m_nMinBuffers =  (uint32_t)  ((pFromData)[5] >> 5) & 0x07;
  (pToStruct)->m_nMinBuffers |=  (uint32_t)  (pFromData)[6] << 3;
  (pToStruct)->m_nMinBuffers |=  (uint32_t)  ((pFromData)[7] & 0x07) << 11;

  /* Unpack Member: m_nMaxBuffers */
  (pToStruct)->m_nMaxBuffers =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_nMaxBuffers |=  (uint32_t)  (pFromData)[4] << 1;
  (pToStruct)->m_nMaxBuffers |=  (uint32_t)  ((pFromData)[5] & 0x1f) << 9;

  /* Unpack Member: m_nLocal */
  (pToStruct)->m_nLocal =  (uint32_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nQueueDepthInLine16B */
  (pToStruct)->m_nQueueDepthInLine16B =  (uint32_t)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32_t)  (pFromData)[1] << 3;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32_t)  (pFromData)[2] << 11;
  (pToStruct)->m_nQueueDepthInLine16B |=  (uint32_t)  ((pFromData)[3] & 0x3f) << 19;

  /* Unpack Member: m_nAnemicWatermarkSel */
  (pToStruct)->m_nAnemicWatermarkSel =  (uint32_t)  ((pFromData)[0] >> 2) & 0x07;

  /* Unpack Member: m_nQeType */
  (pToStruct)->m_nQeType =  (uint32_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8_t)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmQueueStateEntry_InitInstance(sbZfKaQmQueueStateEntry_t *pFrame) {

  pFrame->m_nAllocatedBuffsCnt =  (unsigned int)  0;
  pFrame->m_nQTailPtr =  (unsigned int)  0;
  pFrame->m_nQHeadPtr =  (unsigned int)  0;
  pFrame->m_nNoBuffsAllocated =  (unsigned int)  0;
  pFrame->m_nOverflow =  (unsigned int)  0;
  pFrame->m_nMinBuffers =  (unsigned int)  0;
  pFrame->m_nMaxBuffers =  (unsigned int)  0;
  pFrame->m_nLocal =  (unsigned int)  0;
  pFrame->m_nQueueDepthInLine16B =  (unsigned int)  0;
  pFrame->m_nAnemicWatermarkSel =  (unsigned int)  0;
  pFrame->m_nQeType =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;

}
