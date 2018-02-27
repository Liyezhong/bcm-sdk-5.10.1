/*
 * $Id: sbZfFabBm9600LinkFailureInfo.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfFabBm9600LinkFailureInfo.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm9600LinkFailureInfo_Pack(sbZfFabBm9600LinkFailureInfo_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_bQeLinkState */
  (pToData)[26] |= ((pFrom)->m_bQeLinkState & 0x1f) <<3;

  /* Pack Member: m_bExpectingLinkError */
  (pToData)[26] |= ((pFrom)->m_bExpectingLinkError & 0x01) <<2;

  /* Pack Member: m_uDigestScanStartTime */
  (pToData)[18] |= ((pFrom)->m_uDigestScanStartTime & 0x3f) <<2;
  (pToData)[17] |= ((pFrom)->m_uDigestScanStartTime >> 6) &0xFF;
  (pToData)[16] |= ((pFrom)->m_uDigestScanStartTime >> 14) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uDigestScanStartTime >> 22) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uDigestScanStartTime >> 30) &0xFF;
  (pToData)[21] |= ((pFrom)->m_uDigestScanStartTime >> 38) &0xFF;
  (pToData)[20] |= ((pFrom)->m_uDigestScanStartTime >> 46) &0xFF;
  (pToData)[27] |= ((pFrom)->m_uDigestScanStartTime >> 54) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uDigestScanStartTime >> 62) & 0x03;

  /* Pack Member: m_uDigestScanEndTime */
  (pToData)[10] |= ((pFrom)->m_uDigestScanEndTime & 0x3f) <<2;
  (pToData)[9] |= ((pFrom)->m_uDigestScanEndTime >> 6) &0xFF;
  (pToData)[8] |= ((pFrom)->m_uDigestScanEndTime >> 14) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uDigestScanEndTime >> 22) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uDigestScanEndTime >> 30) &0xFF;
  (pToData)[13] |= ((pFrom)->m_uDigestScanEndTime >> 38) &0xFF;
  (pToData)[12] |= ((pFrom)->m_uDigestScanEndTime >> 46) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uDigestScanEndTime >> 54) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uDigestScanEndTime >> 62) & 0x03;

  /* Pack Member: m_uDUTBaseAddress */
  (pToData)[6] |= ((pFrom)->m_uDUTBaseAddress & 0x3f) <<2;
  (pToData)[5] |= ((pFrom)->m_uDUTBaseAddress >> 6) &0xFF;
  (pToData)[4] |= ((pFrom)->m_uDUTBaseAddress >> 14) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uDUTBaseAddress >> 22) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uDUTBaseAddress >> 30) & 0x03;

  /* Pack Member: m_uActiveBmeFoMode */
  (pToData)[6] |= ((pFrom)->m_uActiveBmeFoMode & 0x03);

  /* Pack Member: m_uActiveBmeSciLink */
  (pToData)[7] |= ((pFrom)->m_uActiveBmeSciLink & 0x01) <<7;

  /* Pack Member: m_uExpectedGlobalLinkState */
  (pToData)[3] |= ((pFrom)->m_uExpectedGlobalLinkState & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_uExpectedGlobalLinkState >> 1) &0xFF;
  (pToData)[1] |= ((pFrom)->m_uExpectedGlobalLinkState >> 9) &0xFF;
  (pToData)[0] |= ((pFrom)->m_uExpectedGlobalLinkState >> 17) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uExpectedGlobalLinkState >> 25) & 0x7f;

  /* Pack Member: m_uInaNumber */
  (pToData)[3] |= ((pFrom)->m_uInaNumber & 0x7f);
#else
  int i;
  int size = SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_bQeLinkState */
  (pToData)[25] |= ((pFrom)->m_bQeLinkState & 0x1f) <<3;

  /* Pack Member: m_bExpectingLinkError */
  (pToData)[25] |= ((pFrom)->m_bExpectingLinkError & 0x01) <<2;

  /* Pack Member: m_uDigestScanStartTime */
  (pToData)[17] |= ((pFrom)->m_uDigestScanStartTime & 0x3f) <<2;
  (pToData)[18] |= ((pFrom)->m_uDigestScanStartTime >> 6) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uDigestScanStartTime >> 14) &0xFF;
  (pToData)[20] |= ((pFrom)->m_uDigestScanStartTime >> 22) &0xFF;
  (pToData)[21] |= ((pFrom)->m_uDigestScanStartTime >> 30) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uDigestScanStartTime >> 38) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uDigestScanStartTime >> 46) &0xFF;
  (pToData)[24] |= ((pFrom)->m_uDigestScanStartTime >> 54) &0xFF;
  (pToData)[25] |= ((pFrom)->m_uDigestScanStartTime >> 62) & 0x03;

  /* Pack Member: m_uDigestScanEndTime */
  (pToData)[9] |= ((pFrom)->m_uDigestScanEndTime & 0x3f) <<2;
  (pToData)[10] |= ((pFrom)->m_uDigestScanEndTime >> 6) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uDigestScanEndTime >> 14) &0xFF;
  (pToData)[12] |= ((pFrom)->m_uDigestScanEndTime >> 22) &0xFF;
  (pToData)[13] |= ((pFrom)->m_uDigestScanEndTime >> 30) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uDigestScanEndTime >> 38) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uDigestScanEndTime >> 46) &0xFF;
  (pToData)[16] |= ((pFrom)->m_uDigestScanEndTime >> 54) &0xFF;
  (pToData)[17] |= ((pFrom)->m_uDigestScanEndTime >> 62) & 0x03;

  /* Pack Member: m_uDUTBaseAddress */
  (pToData)[5] |= ((pFrom)->m_uDUTBaseAddress & 0x3f) <<2;
  (pToData)[6] |= ((pFrom)->m_uDUTBaseAddress >> 6) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uDUTBaseAddress >> 14) &0xFF;
  (pToData)[8] |= ((pFrom)->m_uDUTBaseAddress >> 22) &0xFF;
  (pToData)[9] |= ((pFrom)->m_uDUTBaseAddress >> 30) & 0x03;

  /* Pack Member: m_uActiveBmeFoMode */
  (pToData)[5] |= ((pFrom)->m_uActiveBmeFoMode & 0x03);

  /* Pack Member: m_uActiveBmeSciLink */
  (pToData)[4] |= ((pFrom)->m_uActiveBmeSciLink & 0x01) <<7;

  /* Pack Member: m_uExpectedGlobalLinkState */
  (pToData)[0] |= ((pFrom)->m_uExpectedGlobalLinkState & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_uExpectedGlobalLinkState >> 1) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uExpectedGlobalLinkState >> 9) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uExpectedGlobalLinkState >> 17) &0xFF;
  (pToData)[4] |= ((pFrom)->m_uExpectedGlobalLinkState >> 25) & 0x7f;

  /* Pack Member: m_uInaNumber */
  (pToData)[0] |= ((pFrom)->m_uInaNumber & 0x7f);
#endif

  return SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600LinkFailureInfo_Unpack(sbZfFabBm9600LinkFailureInfo_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_bQeLinkState */
  (pToStruct)->m_bQeLinkState =  (uint32_t)  ((pFromData)[26] >> 3) & 0x1f;

  /* Unpack Member: m_bExpectingLinkError */
  (pToStruct)->m_bExpectingLinkError =  (uint8_t)  ((pFromData)[26] >> 2) & 0x01;

  /* Unpack Member: m_uDigestScanStartTime */
  COMPILER_64_SET((pToStruct)->m_uDigestScanStartTime, 0,  (unsigned int) (pFromData)[18]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uDigestScanEndTime */
  COMPILER_64_SET((pToStruct)->m_uDigestScanEndTime, 0,  (unsigned int) (pFromData)[10]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uDUTBaseAddress */
  (pToStruct)->m_uDUTBaseAddress =  (uint32_t)  ((pFromData)[6] >> 2) & 0x3f;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[5] << 6;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[4] << 14;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[11] << 22;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  ((pFromData)[10] & 0x03) << 30;

  /* Unpack Member: m_uActiveBmeFoMode */
  (pToStruct)->m_uActiveBmeFoMode =  (uint32_t)  ((pFromData)[6] ) & 0x03;

  /* Unpack Member: m_uActiveBmeSciLink */
  (pToStruct)->m_uActiveBmeSciLink =  (uint32_t)  ((pFromData)[7] >> 7) & 0x01;

  /* Unpack Member: m_uExpectedGlobalLinkState */
  (pToStruct)->m_uExpectedGlobalLinkState =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  (pFromData)[2] << 1;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  (pFromData)[1] << 9;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  (pFromData)[0] << 17;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  ((pFromData)[7] & 0x7f) << 25;

  /* Unpack Member: m_uInaNumber */
  (pToStruct)->m_uInaNumber =  (uint32_t)  ((pFromData)[3] ) & 0x7f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_bQeLinkState */
  (pToStruct)->m_bQeLinkState =  (uint32_t)  ((pFromData)[25] >> 3) & 0x1f;

  /* Unpack Member: m_bExpectingLinkError */
  (pToStruct)->m_bExpectingLinkError =  (uint8_t)  ((pFromData)[25] >> 2) & 0x01;

  /* Unpack Member: m_uDigestScanStartTime */
  COMPILER_64_SET((pToStruct)->m_uDigestScanStartTime, 0,  (unsigned int) (pFromData)[17]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanStartTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[24]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uDigestScanEndTime */
  COMPILER_64_SET((pToStruct)->m_uDigestScanEndTime, 0,  (unsigned int) (pFromData)[9]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_uDigestScanEndTime;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_uDUTBaseAddress */
  (pToStruct)->m_uDUTBaseAddress =  (uint32_t)  ((pFromData)[5] >> 2) & 0x3f;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[6] << 6;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[7] << 14;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  (pFromData)[8] << 22;
  (pToStruct)->m_uDUTBaseAddress |=  (uint32_t)  ((pFromData)[9] & 0x03) << 30;

  /* Unpack Member: m_uActiveBmeFoMode */
  (pToStruct)->m_uActiveBmeFoMode =  (uint32_t)  ((pFromData)[5] ) & 0x03;

  /* Unpack Member: m_uActiveBmeSciLink */
  (pToStruct)->m_uActiveBmeSciLink =  (uint32_t)  ((pFromData)[4] >> 7) & 0x01;

  /* Unpack Member: m_uExpectedGlobalLinkState */
  (pToStruct)->m_uExpectedGlobalLinkState =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  (pFromData)[1] << 1;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  (pFromData)[2] << 9;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  (pFromData)[3] << 17;
  (pToStruct)->m_uExpectedGlobalLinkState |=  (uint32_t)  ((pFromData)[4] & 0x7f) << 25;

  /* Unpack Member: m_uInaNumber */
  (pToStruct)->m_uInaNumber =  (uint32_t)  ((pFromData)[0] ) & 0x7f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600LinkFailureInfo_InitInstance(sbZfFabBm9600LinkFailureInfo_t *pFrame) {

  pFrame->m_bQeLinkState =  (unsigned int)  0;
  pFrame->m_bExpectingLinkError =  (unsigned int)  0;
  pFrame->m_uDigestScanStartTime =  (uint64_t)  0;
  pFrame->m_uDigestScanEndTime =  (uint64_t)  0;
  pFrame->m_uDUTBaseAddress =  (unsigned int)  0;
  pFrame->m_uActiveBmeFoMode =  (unsigned int)  0;
  pFrame->m_uActiveBmeSciLink =  (unsigned int)  0;
  pFrame->m_uExpectedGlobalLinkState =  (unsigned int)  0;
  pFrame->m_uInaNumber =  (unsigned int)  0;

}
