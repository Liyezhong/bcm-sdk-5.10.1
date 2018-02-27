/*
 * $Id: sbZfKaRbClassProtocolEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbClassProtocolEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassProtocolEntry_Print(sbZfKaRbClassProtocolEntry_t *pFromStruct) {
  SB_LOG("KaRbClassProtocolEntry:: protocol1usetos=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1UseTos);
  SB_LOG(" protocol1usesktinhash=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1UseSocketInHash);
  SB_LOG("\n");

  SB_LOG("KaRbClassProtocolEntry:: protocol1dp=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1Dp);
  SB_LOG(" protocol1lsb=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1Lsb);
  SB_LOG("\n");

  SB_LOG("KaRbClassProtocolEntry:: protocol0usetos=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0UseTos);
  SB_LOG(" protocol0usesktinhash=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0UseSocketInHash);
  SB_LOG("\n");

  SB_LOG("KaRbClassProtocolEntry:: protocol0dp=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0Dp);
  SB_LOG(" protocol0lsb=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0Lsb);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbClassProtocolEntry_SPrint(sbZfKaRbClassProtocolEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol1usetos=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1UseTos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol1usesktinhash=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1UseSocketInHash);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol1dp=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol1lsb=0x%01x", (unsigned int)  pFromStruct->m_nProtocol1Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol0usetos=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0UseTos);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol0usesktinhash=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0UseSocketInHash);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassProtocolEntry:: protocol0dp=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0Dp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," protocol0lsb=0x%01x", (unsigned int)  pFromStruct->m_nProtocol0Lsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassProtocolEntry_Validate(sbZfKaRbClassProtocolEntry_t *pZf) {

  if (pZf->m_nProtocol1UseTos > 0x1) return 0;
  if (pZf->m_nProtocol1UseSocketInHash > 0x1) return 0;
  if (pZf->m_nProtocol1Dp > 0x3) return 0;
  if (pZf->m_nProtocol1Lsb > 0xf) return 0;
  if (pZf->m_nProtocol0UseTos > 0x1) return 0;
  if (pZf->m_nProtocol0UseSocketInHash > 0x1) return 0;
  if (pZf->m_nProtocol0Dp > 0x3) return 0;
  if (pZf->m_nProtocol0Lsb > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassProtocolEntry_SetField(sbZfKaRbClassProtocolEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nprotocol1usetos") == 0) {
    s->m_nProtocol1UseTos = value;
  } else if (SB_STRCMP(name, "m_nprotocol1usesocketinhash") == 0) {
    s->m_nProtocol1UseSocketInHash = value;
  } else if (SB_STRCMP(name, "m_nprotocol1dp") == 0) {
    s->m_nProtocol1Dp = value;
  } else if (SB_STRCMP(name, "m_nprotocol1lsb") == 0) {
    s->m_nProtocol1Lsb = value;
  } else if (SB_STRCMP(name, "m_nprotocol0usetos") == 0) {
    s->m_nProtocol0UseTos = value;
  } else if (SB_STRCMP(name, "m_nprotocol0usesocketinhash") == 0) {
    s->m_nProtocol0UseSocketInHash = value;
  } else if (SB_STRCMP(name, "m_nprotocol0dp") == 0) {
    s->m_nProtocol0Dp = value;
  } else if (SB_STRCMP(name, "m_nprotocol0lsb") == 0) {
    s->m_nProtocol0Lsb = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
