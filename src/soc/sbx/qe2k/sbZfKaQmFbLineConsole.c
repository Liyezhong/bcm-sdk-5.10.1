/*
 * $Id: sbZfKaQmFbLineConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaQmFbLineConsole.hx"



/* Print members in struct */
void
sbZfKaQmFbLine_Print(sbZfKaQmFbLine_t *pFromStruct) {
  SB_LOG("KaQmFbLine:: hec1=0x%02x", (unsigned int)  pFromStruct->m_nHec1);
  SB_LOG(" hec0=0x%02x", (unsigned int)  pFromStruct->m_nHec0);
  SB_LOG(" spr=0x%03x", (unsigned int)  pFromStruct->m_nSpare);
  SB_LOG(" pb05=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr5);
  SB_LOG(" pb04=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr4);
  SB_LOG("\n");

  SB_LOG("KaQmFbLine:: pb03=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr3);
  SB_LOG(" pb02=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr2);
  SB_LOG(" pb01=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr1);
  SB_LOG(" pb00=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaQmFbLine_SPrint(sbZfKaQmFbLine_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmFbLine:: hec1=0x%02x", (unsigned int)  pFromStruct->m_nHec1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hec0=0x%02x", (unsigned int)  pFromStruct->m_nHec0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spr=0x%03x", (unsigned int)  pFromStruct->m_nSpare);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb05=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb04=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQmFbLine:: pb03=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb02=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb01=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pb00=0x%05x", (unsigned int)  pFromStruct->m_nPbExtAddr0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQmFbLine_Validate(sbZfKaQmFbLine_t *pZf) {

  if (pZf->m_nHec1 > 0xff) return 0;
  if (pZf->m_nHec0 > 0xff) return 0;
  if (pZf->m_nSpare > 0x3ff) return 0;
  if (pZf->m_nPbExtAddr5 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr4 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr3 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr2 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr1 > 0x1ffff) return 0;
  if (pZf->m_nPbExtAddr0 > 0x1ffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQmFbLine_SetField(sbZfKaQmFbLine_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nhec1") == 0) {
    s->m_nHec1 = value;
  } else if (SB_STRCMP(name, "m_nhec0") == 0) {
    s->m_nHec0 = value;
  } else if (SB_STRCMP(name, "m_nspare") == 0) {
    s->m_nSpare = value;
  } else if (SB_STRCMP(name, "m_npbextaddr5") == 0) {
    s->m_nPbExtAddr5 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr4") == 0) {
    s->m_nPbExtAddr4 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr3") == 0) {
    s->m_nPbExtAddr3 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr2") == 0) {
    s->m_nPbExtAddr2 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr1") == 0) {
    s->m_nPbExtAddr1 = value;
  } else if (SB_STRCMP(name, "m_npbextaddr0") == 0) {
    s->m_nPbExtAddr0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
