/*
 * $Id: sbZfFabBm9600FoTestInfo.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfFabBm9600FoTestInfo.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm9600FoTestInfo_Pack(sbZfFabBm9600FoTestInfo_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_FOTESTINFO_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_bIsActive */
  (pToData)[22] |= ((pFrom)->m_bIsActive & 0x01) <<2;

  /* Pack Member: m_uuDUTTimeslotsize */
  (pToData)[14] |= ((pFrom)->m_uuDUTTimeslotsize & 0x3f) <<2;
  (pToData)[13] |= ((pFrom)->m_uuDUTTimeslotsize >> 6) &0xFF;
  (pToData)[12] |= ((pFrom)->m_uuDUTTimeslotsize >> 14) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uuDUTTimeslotsize >> 22) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uuDUTTimeslotsize >> 30) &0xFF;
  (pToData)[17] |= ((pFrom)->m_uuDUTTimeslotsize >> 38) &0xFF;
  (pToData)[16] |= ((pFrom)->m_uuDUTTimeslotsize >> 46) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uuDUTTimeslotsize >> 54) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uuDUTTimeslotsize >> 62) & 0x03;

  /* Pack Member: m_uuSimTimeThatQEFailoverOccurred */
  (pToData)[6] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred & 0x3f) <<2;
  (pToData)[5] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 6) &0xFF;
  (pToData)[4] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 14) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 22) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 30) &0xFF;
  (pToData)[9] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 38) &0xFF;
  (pToData)[8] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 46) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 54) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 62) & 0x03;

  /* Pack Member: m_uDUTBaseAddress */
  (pToData)[2] |= ((pFrom)->m_uDUTBaseAddress & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_uDUTBaseAddress >> 6) &0xFF;
  (pToData)[0] |= ((pFrom)->m_uDUTBaseAddress >> 14) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uDUTBaseAddress >> 22) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uDUTBaseAddress >> 30) & 0x03;

  /* Pack Member: m_uNumQEsThatFailedOver */
  (pToData)[3] |= ((pFrom)->m_uNumQEsThatFailedOver & 0x1f) <<3;
  (pToData)[2] |= ((pFrom)->m_uNumQEsThatFailedOver >> 5) & 0x03;

  /* Pack Member: m_uPreviousActiveBm */
  (pToData)[3] |= ((pFrom)->m_uPreviousActiveBm & 0x01) <<2;

  /* Pack Member: m_bFailoverPreviouslyAsserted */
  (pToData)[3] |= ((pFrom)->m_bFailoverPreviouslyAsserted & 0x01) <<1;

  /* Pack Member: m_bExpectFailover */
  (pToData)[3] |= ((pFrom)->m_bExpectFailover & 0x01);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_FOTESTINFO_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_bIsActive */
  (pToData)[21] |= ((pFrom)->m_bIsActive & 0x01) <<2;

  /* Pack Member: m_uuDUTTimeslotsize */
  (pToData)[13] |= ((pFrom)->m_uuDUTTimeslotsize & 0x3f) <<2;
  (pToData)[14] |= ((pFrom)->m_uuDUTTimeslotsize >> 6) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uuDUTTimeslotsize >> 14) &0xFF;
  (pToData)[16] |= ((pFrom)->m_uuDUTTimeslotsize >> 22) &0xFF;
  (pToData)[17] |= ((pFrom)->m_uuDUTTimeslotsize >> 30) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uuDUTTimeslotsize >> 38) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uuDUTTimeslotsize >> 46) &0xFF;
  (pToData)[20] |= ((pFrom)->m_uuDUTTimeslotsize >> 54) &0xFF;
  (pToData)[21] |= ((pFrom)->m_uuDUTTimeslotsize >> 62) & 0x03;

  /* Pack Member: m_uuSimTimeThatQEFailoverOccurred */
  (pToData)[5] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred & 0x3f) <<2;
  (pToData)[6] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 6) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 14) &0xFF;
  (pToData)[8] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 22) &0xFF;
  (pToData)[9] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 30) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 38) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 46) &0xFF;
  (pToData)[12] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 54) &0xFF;
  (pToData)[13] |= ((pFrom)->m_uuSimTimeThatQEFailoverOccurred >> 62) & 0x03;

  /* Pack Member: m_uDUTBaseAddress */
  (pToData)[1] |= ((pFrom)->m_uDUTBaseAddress & 0x3f) <<2;
  (pToData)[2] |= ((pFrom)->m_uDUTBaseAddress >> 6) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uDUTBaseAddress >> 14) &0xFF;
  (pToData)[4] |= ((pFrom)->m_uDUTBaseAddress >> 22) &0xFF;
  (pToData)[5] |= ((pFrom)->m_uDUTBaseAddress >> 30) & 0x03;

  /* Pack Member: m_uNumQEsThatFailedOver */
  (pToData)[0] |= ((pFrom)->m_uNumQEsThatFailedOver & 0x1f) <<3;
  (pToData)[1] |= ((pFrom)->m_uNumQEsThatFailedOver >> 5) & 0x03;

  /* Pack Member: m_uPreviousActiveBm */
  (pToData)[0] |= ((pFrom)->m_uPreviousActiveBm & 0x01) <<2;

  /* Pack Member: m_bFailoverPreviouslyAsserted */
  (pToData)[0] |= ((pFrom)->m_bFailoverPreviouslyAsserted & 0x01) <<1;

  /* Pack Member: m_bExpectFailover */
  (pToData)[0] |= ((pFrom)->m_bExpectFailover & 0x01);
#endif

  return SB_ZF_FAB_BM9600_FOTESTINFO_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600FoTestInfo_Unpack(sbZfFabBm9600FoTestInfo_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_bIsActive */
  (pToStruct)->m_bIsActive =  (uint8_t)  ((pFromData)[22] >> 2) & 0x01;

  /* Unpack Member: m_uuDUTTimeslotsize */
  COMPILER_64_SET((pToStruct)->m_uuDUTTimeslotsize, 0,  (unsigned int) (pFromData)[14]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uuSimTimeThatQEFailoverOccurred */
  COMPILER_64_SET((pToStruct)->m_uuSimTimeThatQEFailoverOccurred, 0,  (unsigned int) (pFromData)[6]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uDUTBaseAddress */
  (pToStruct)->m_uDUTBaseAddress =  (uint32_t)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[1] << 6;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[0] << 14;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[7] << 22;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  ((pFromData)[6] & 0x03) << 30;

  /* Unpack Member: m_uNumQEsThatFailedOver */
  (pToStruct)->m_uNumQEsThatFailedOver =  (uint32_t)  ((pFromData)[3] >> 3) & 0x1f;
  (pToStruct)->m_uNumQEsThatFailedOver |=  (uint32_t)  ((pFromData)[2] & 0x03) << 5;

  /* Unpack Member: m_uPreviousActiveBm */
  (pToStruct)->m_uPreviousActiveBm =  (uint32_t)  ((pFromData)[3] >> 2) & 0x01;

  /* Unpack Member: m_bFailoverPreviouslyAsserted */
  (pToStruct)->m_bFailoverPreviouslyAsserted =  (uint8_t)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_bExpectFailover */
  (pToStruct)->m_bExpectFailover =  (uint8_t)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_bIsActive */
  (pToStruct)->m_bIsActive =  (uint8_t)  ((pFromData)[21] >> 2) & 0x01;

  /* Unpack Member: m_uuDUTTimeslotsize */
  COMPILER_64_SET((pToStruct)->m_uuDUTTimeslotsize, 0,  (unsigned int) (pFromData)[13]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuDUTTimeslotsize;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uuSimTimeThatQEFailoverOccurred */
  COMPILER_64_SET((pToStruct)->m_uuSimTimeThatQEFailoverOccurred, 0,  (unsigned int) (pFromData)[5]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uuSimTimeThatQEFailoverOccurred;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uDUTBaseAddress */
  (pToStruct)->m_uDUTBaseAddress =  (uint32_t)  ((pFromData)[1] >> 2) & 0x3f;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[2] << 6;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[3] << 14;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[4] << 22;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  ((pFromData)[5] & 0x03) << 30;

  /* Unpack Member: m_uNumQEsThatFailedOver */
  (pToStruct)->m_uNumQEsThatFailedOver =  (uint32_t)  ((pFromData)[0] >> 3) & 0x1f;
  (pToStruct)->m_uNumQEsThatFailedOver |=  (uint32_t)  ((pFromData)[1] & 0x03) << 5;

  /* Unpack Member: m_uPreviousActiveBm */
  (pToStruct)->m_uPreviousActiveBm =  (uint32_t)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: m_bFailoverPreviouslyAsserted */
  (pToStruct)->m_bFailoverPreviouslyAsserted =  (uint8_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_bExpectFailover */
  (pToStruct)->m_bExpectFailover =  (uint8_t)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600FoTestInfo_InitInstance(sbZfFabBm9600FoTestInfo_t *pFrame) {

  pFrame->m_bIsActive =  (unsigned int)  0;
  pFrame->m_uuDUTTimeslotsize =  (uint64_t)  0;
  pFrame->m_uuSimTimeThatQEFailoverOccurred =  (uint64_t)  0;
  pFrame->m_uDUTBaseAddress =  (unsigned int)  0;
  pFrame->m_uNumQEsThatFailedOver =  (unsigned int)  0;
  pFrame->m_uPreviousActiveBm =  (unsigned int)  0;
  pFrame->m_bFailoverPreviouslyAsserted =  (unsigned int)  0;
  pFrame->m_bExpectFailover =  (unsigned int)  0;

}
