/*
 * $Id: sbZfKaRbClassHashSVlanIPv6.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaRbClassHashSVlanIPv6.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaRbClassHashSVlanIPv6_Pack(sbZfKaRbClassHashSVlanIPv6_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHSVLANIPV6_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nProtocol */
  (pToData)[50] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nFlow */
  (pToData)[45] |= ((pFrom)->m_nFlow & 0x0f) <<4;
  (pToData)[44] |= ((pFrom)->m_nFlow >> 4) &0xFF;
  (pToData)[51] |= ((pFrom)->m_nFlow >> 12) &0xFF;

  /* Pack Member: m_nPadWord2 */
  (pToData)[40] |= ((pFrom)->m_nPadWord2 & 0x7f) <<1;
  (pToData)[47] |= ((pFrom)->m_nPadWord2 >> 7) &0xFF;
  (pToData)[46] |= ((pFrom)->m_nPadWord2 >> 15) &0xFF;
  (pToData)[45] |= ((pFrom)->m_nPadWord2 >> 23) & 0x0f;

  /* Pack Member: m_nPadWord1 */
  (pToData)[39] |= ((pFrom)->m_nPadWord1) & 0xFF;
  (pToData)[38] |= ((pFrom)->m_nPadWord1 >> 8) &0xFF;
  (pToData)[37] |= ((pFrom)->m_nPadWord1 >> 16) &0xFF;
  (pToData)[36] |= ((pFrom)->m_nPadWord1 >> 24) &0xFF;
  (pToData)[43] |= ((pFrom)->m_nPadWord1 >> 32) &0xFF;
  (pToData)[42] |= ((pFrom)->m_nPadWord1 >> 40) &0xFF;
  (pToData)[41] |= ((pFrom)->m_nPadWord1 >> 48) &0xFF;
  (pToData)[40] |= ((pFrom)->m_nPadWord1 >> 56) & 0x01;

  /* Pack Member: m_nnIpSaHigh */
  (pToData)[31] |= ((pFrom)->m_nnIpSaHigh) & 0xFF;
  (pToData)[30] |= ((pFrom)->m_nnIpSaHigh >> 8) &0xFF;
  (pToData)[29] |= ((pFrom)->m_nnIpSaHigh >> 16) &0xFF;
  (pToData)[28] |= ((pFrom)->m_nnIpSaHigh >> 24) &0xFF;
  (pToData)[35] |= ((pFrom)->m_nnIpSaHigh >> 32) &0xFF;
  (pToData)[34] |= ((pFrom)->m_nnIpSaHigh >> 40) &0xFF;
  (pToData)[33] |= ((pFrom)->m_nnIpSaHigh >> 48) &0xFF;
  (pToData)[32] |= ((pFrom)->m_nnIpSaHigh >> 56) &0xFF;

  /* Pack Member: m_nnIpSaLow */
  (pToData)[23] |= ((pFrom)->m_nnIpSaLow) & 0xFF;
  (pToData)[22] |= ((pFrom)->m_nnIpSaLow >> 8) &0xFF;
  (pToData)[21] |= ((pFrom)->m_nnIpSaLow >> 16) &0xFF;
  (pToData)[20] |= ((pFrom)->m_nnIpSaLow >> 24) &0xFF;
  (pToData)[27] |= ((pFrom)->m_nnIpSaLow >> 32) &0xFF;
  (pToData)[26] |= ((pFrom)->m_nnIpSaLow >> 40) &0xFF;
  (pToData)[25] |= ((pFrom)->m_nnIpSaLow >> 48) &0xFF;
  (pToData)[24] |= ((pFrom)->m_nnIpSaLow >> 56) &0xFF;

  /* Pack Member: m_nnIpDaHigh */
  (pToData)[15] |= ((pFrom)->m_nnIpDaHigh) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nnIpDaHigh >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nnIpDaHigh >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nnIpDaHigh >> 24) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nnIpDaHigh >> 32) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nnIpDaHigh >> 40) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nnIpDaHigh >> 48) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nnIpDaHigh >> 56) &0xFF;

  /* Pack Member: m_nnIpDaLow */
  (pToData)[7] |= ((pFrom)->m_nnIpDaLow) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nnIpDaLow >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nnIpDaLow >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nnIpDaLow >> 24) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nnIpDaLow >> 32) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nnIpDaLow >> 40) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nnIpDaLow >> 48) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nnIpDaLow >> 56) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[3] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nSocket >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHSVLANIPV6_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nProtocol */
  (pToData)[49] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nFlow */
  (pToData)[46] |= ((pFrom)->m_nFlow & 0x0f) <<4;
  (pToData)[47] |= ((pFrom)->m_nFlow >> 4) &0xFF;
  (pToData)[48] |= ((pFrom)->m_nFlow >> 12) &0xFF;

  /* Pack Member: m_nPadWord2 */
  (pToData)[43] |= ((pFrom)->m_nPadWord2 & 0x7f) <<1;
  (pToData)[44] |= ((pFrom)->m_nPadWord2 >> 7) &0xFF;
  (pToData)[45] |= ((pFrom)->m_nPadWord2 >> 15) &0xFF;
  (pToData)[46] |= ((pFrom)->m_nPadWord2 >> 23) & 0x0f;

  /* Pack Member: m_nPadWord1 */
  (pToData)[36] |= ((pFrom)->m_nPadWord1) & 0xFF;
  (pToData)[37] |= ((pFrom)->m_nPadWord1 >> 8) &0xFF;
  (pToData)[38] |= ((pFrom)->m_nPadWord1 >> 16) &0xFF;
  (pToData)[39] |= ((pFrom)->m_nPadWord1 >> 24) &0xFF;
  (pToData)[40] |= ((pFrom)->m_nPadWord1 >> 32) &0xFF;
  (pToData)[41] |= ((pFrom)->m_nPadWord1 >> 40) &0xFF;
  (pToData)[42] |= ((pFrom)->m_nPadWord1 >> 48) &0xFF;
  (pToData)[43] |= ((pFrom)->m_nPadWord1 >> 56) & 0x01;

  /* Pack Member: m_nnIpSaHigh */
  (pToData)[28] |= ((pFrom)->m_nnIpSaHigh) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_nnIpSaHigh >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_nnIpSaHigh >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_nnIpSaHigh >> 24) &0xFF;
  (pToData)[32] |= ((pFrom)->m_nnIpSaHigh >> 32) &0xFF;
  (pToData)[33] |= ((pFrom)->m_nnIpSaHigh >> 40) &0xFF;
  (pToData)[34] |= ((pFrom)->m_nnIpSaHigh >> 48) &0xFF;
  (pToData)[35] |= ((pFrom)->m_nnIpSaHigh >> 56) &0xFF;

  /* Pack Member: m_nnIpSaLow */
  (pToData)[20] |= ((pFrom)->m_nnIpSaLow) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_nnIpSaLow >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_nnIpSaLow >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_nnIpSaLow >> 24) &0xFF;
  (pToData)[24] |= ((pFrom)->m_nnIpSaLow >> 32) &0xFF;
  (pToData)[25] |= ((pFrom)->m_nnIpSaLow >> 40) &0xFF;
  (pToData)[26] |= ((pFrom)->m_nnIpSaLow >> 48) &0xFF;
  (pToData)[27] |= ((pFrom)->m_nnIpSaLow >> 56) &0xFF;

  /* Pack Member: m_nnIpDaHigh */
  (pToData)[12] |= ((pFrom)->m_nnIpDaHigh) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nnIpDaHigh >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nnIpDaHigh >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nnIpDaHigh >> 24) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nnIpDaHigh >> 32) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nnIpDaHigh >> 40) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nnIpDaHigh >> 48) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nnIpDaHigh >> 56) &0xFF;

  /* Pack Member: m_nnIpDaLow */
  (pToData)[4] |= ((pFrom)->m_nnIpDaLow) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nnIpDaLow >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nnIpDaLow >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nnIpDaLow >> 24) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nnIpDaLow >> 32) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nnIpDaLow >> 40) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nnIpDaLow >> 48) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nnIpDaLow >> 56) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[0] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nSocket >> 24) &0xFF;
#endif

  return SB_ZF_ZFKARBCLASSHASHSVLANIPV6_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassHashSVlanIPv6_Unpack(sbZfKaRbClassHashSVlanIPv6_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32_t)  (pFromData)[50] ;

  /* Unpack Member: m_nFlow */
  (pToStruct)->m_nFlow =  (uint32_t)  ((pFromData)[45] >> 4) & 0x0f;
  (pToStruct)->m_nFlow |=  (uint32_t)  (pFromData)[44] << 4;
  (pToStruct)->m_nFlow |=  (uint32_t)  (pFromData)[51] << 12;

  /* Unpack Member: m_nPadWord2 */
  COMPILER_64_SET((pToStruct)->m_nPadWord2, 0,  (unsigned int) (pFromData)[40]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[47]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPadWord1 */
  COMPILER_64_SET((pToStruct)->m_nPadWord1, 0,  (unsigned int) (pFromData)[39]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[40]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpSaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpSaHigh, 0,  (unsigned int) (pFromData)[31]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[28]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[35]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[34]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[33]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[32]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpSaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpSaLow, 0,  (unsigned int) (pFromData)[23]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[26]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[25]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[24]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpDaHigh, 0,  (unsigned int) (pFromData)[15]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpDaLow, 0,  (unsigned int) (pFromData)[7]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32_t)  (pFromData)[49] ;

  /* Unpack Member: m_nFlow */
  (pToStruct)->m_nFlow =  (uint32_t)  ((pFromData)[46] >> 4) & 0x0f;
  (pToStruct)->m_nFlow |=  (uint32_t)  (pFromData)[47] << 4;
  (pToStruct)->m_nFlow |=  (uint32_t)  (pFromData)[48] << 12;

  /* Unpack Member: m_nPadWord2 */
  COMPILER_64_SET((pToStruct)->m_nPadWord2, 0,  (unsigned int) (pFromData)[43]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[44]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPadWord1 */
  COMPILER_64_SET((pToStruct)->m_nPadWord1, 0,  (unsigned int) (pFromData)[36]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[37]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[38]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[40]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpSaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpSaHigh, 0,  (unsigned int) (pFromData)[28]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[29]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[30]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[31]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[32]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[33]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[34]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[35]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpSaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpSaLow, 0,  (unsigned int) (pFromData)[20]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[24]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[25]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[26]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpSaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaHigh */
  COMPILER_64_SET((pToStruct)->m_nnIpDaHigh, 0,  (unsigned int) (pFromData)[12]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaHigh;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnIpDaLow */
  COMPILER_64_SET((pToStruct)->m_nnIpDaLow, 0,  (unsigned int) (pFromData)[4]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnIpDaLow;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassHashSVlanIPv6_InitInstance(sbZfKaRbClassHashSVlanIPv6_t *pFrame) {

  pFrame->m_nProtocol =  (unsigned int)  0;
  pFrame->m_nFlow =  (unsigned int)  0;
  pFrame->m_nPadWord2 =  (unsigned int)  0;
  pFrame->m_nPadWord1 =  (uint64_t)  0;
  pFrame->m_nnIpSaHigh =  (uint64_t)  0;
  pFrame->m_nnIpSaLow =  (uint64_t)  0;
  pFrame->m_nnIpDaHigh =  (uint64_t)  0;
  pFrame->m_nnIpDaLow =  (uint64_t)  0;
  pFrame->m_nSocket =  (unsigned int)  0;

}
