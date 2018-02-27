/*
 * $Id: sbZfFabBm3200BwRepAddr.c 1.2.28.4 Broadcom SDK $
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
#include "sbZfFabBm3200BwRepAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200BwRepAddr_Pack(sbZfFabBm3200BwRepAddr_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_NMRANKADDR_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nBank */
  (pToData)[1] |= ((pFrom)->m_nBank & 0x01) <<3;

  /* Pack Member: m_nTableId */
  (pToData)[2] |= ((pFrom)->m_nTableId & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nTableId >> 1) & 0x07;

  /* Pack Member: m_nOffset */
  (pToData)[3] |= ((pFrom)->m_nOffset) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nOffset >> 8) & 0x7f;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_NMRANKADDR_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nBank */
  (pToData)[2] |= ((pFrom)->m_nBank & 0x01) <<3;

  /* Pack Member: m_nTableId */
  (pToData)[1] |= ((pFrom)->m_nTableId & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nTableId >> 1) & 0x07;

  /* Pack Member: m_nOffset */
  (pToData)[0] |= ((pFrom)->m_nOffset) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nOffset >> 8) & 0x7f;
#endif

  return SB_ZF_FAB_BM3200_NMRANKADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwRepAddr_Unpack(sbZfFabBm3200BwRepAddr_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nBank */
  (pToStruct)->m_nBank =  (int32_t)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_nTableId */
  (pToStruct)->m_nTableId =  (int32_t)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nTableId |=  (int32_t)  ((pFromData)[1] & 0x07) << 1;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (int32_t)  (pFromData)[3] ;
  (pToStruct)->m_nOffset |=  (int32_t)  ((pFromData)[2] & 0x7f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nBank */
  (pToStruct)->m_nBank =  (int32_t)  ((pFromData)[2] >> 3) & 0x01;

  /* Unpack Member: m_nTableId */
  (pToStruct)->m_nTableId =  (int32_t)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nTableId |=  (int32_t)  ((pFromData)[2] & 0x07) << 1;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (int32_t)  (pFromData)[0] ;
  (pToStruct)->m_nOffset |=  (int32_t)  ((pFromData)[1] & 0x7f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwRepAddr_InitInstance(sbZfFabBm3200BwRepAddr_t *pFrame) {

  pFrame->m_nBank =  (unsigned int)  0;
  pFrame->m_nTableId =  (unsigned int)  0;
  pFrame->m_nOffset =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwRepAddr.c,v 1.2.28.4 2011/05/22 05:39:14 iakramov Exp $
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
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwRepAddr.hx"



/* Print members in struct */
void
sbZfFabBm3200BwRepAddr_Print(sbZfFabBm3200BwRepAddr_t *pFromStruct) {
  SB_LOG("FabBm3200BwRepAddr:: bank=0x%01x", (unsigned int)  pFromStruct->m_nBank);
  SB_LOG(" table_id=0x%01x", (unsigned int)  pFromStruct->m_nTableId);
  SB_LOG(" offset=0x%04x", (unsigned int)  pFromStruct->m_nOffset);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200BwRepAddr_SPrint(sbZfFabBm3200BwRepAddr_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwRepAddr:: bank=0x%01x", (unsigned int)  pFromStruct->m_nBank);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," table_id=0x%01x", (unsigned int)  pFromStruct->m_nTableId);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," offset=0x%04x", (unsigned int)  pFromStruct->m_nOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwRepAddr_Validate(sbZfFabBm3200BwRepAddr_t *pZf) {

  if (pZf->m_nBank > 0x1) return 0;
  if (pZf->m_nTableId > 0xf) return 0;
  if (pZf->m_nOffset > 0x7fff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwRepAddr_SetField(sbZfFabBm3200BwRepAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nbank") == 0) {
    s->m_nBank = value;
  } else if (SB_STRCMP(name, "m_ntableid") == 0) {
    s->m_nTableId = value;
  } else if (SB_STRCMP(name, "m_noffset") == 0) {
    s->m_nOffset = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
