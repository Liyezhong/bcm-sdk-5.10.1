/*
 * $Id: sbZfFe2000ClsInstruction.c 1.3.36.4 Broadcom SDK $
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


#include "sbTypesGlue.h"
#include "sbZfFe2000ClsInstruction.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000ClsInstruction_Pack(sbZfFe2000ClsInstruction_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000CLSINSTRUCTION_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: uOpCodeMsb */
  (pToData)[2] |= ((pFrom)->uOpCodeMsb & 0x03) <<2;

  /* Pack Member: uOpCodeLsb */
  (pToData)[2] |= ((pFrom)->uOpCodeLsb & 0x03);

  /* Pack Member: uSField */
  (pToData)[1] |= ((pFrom)->uSField & 0x0f) <<4;

  /* Pack Member: uMField */
  (pToData)[1] |= ((pFrom)->uMField & 0x0f);

  /* Pack Member: uAField */
  (pToData)[0] |= ((pFrom)->uAField) & 0xFF;

  return SB_ZF_FE2000CLSINSTRUCTION_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000ClsInstruction_Unpack(sbZfFe2000ClsInstruction_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: uOpCodeMsb */
  (pToStruct)->uOpCodeMsb =  (uint32_t)  ((pFromData)[2] >> 2) & 0x03;

  /* Unpack Member: uOpCodeLsb */
  (pToStruct)->uOpCodeLsb =  (uint32_t)  ((pFromData)[2] ) & 0x03;

  /* Unpack Member: uSField */
  (pToStruct)->uSField =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: uMField */
  (pToStruct)->uMField =  (uint32_t)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: uAField */
  (pToStruct)->uAField =  (uint32_t)  (pFromData)[0] ;

}



/* initialize an instance of this zframe */
void
sbZfFe2000ClsInstruction_InitInstance(sbZfFe2000ClsInstruction_t *pFrame) {

  pFrame->uOpCodeMsb =  (unsigned int)  0;
  pFrame->uOpCodeLsb =  (unsigned int)  0;
  pFrame->uSField =  (unsigned int)  0;
  pFrame->uMField =  (unsigned int)  0;
  pFrame->uAField =  (unsigned int)  0;

}
