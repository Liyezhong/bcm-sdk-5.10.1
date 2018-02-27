/*
 * $Id: sbZfKaEpPortTableEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEpPortTableEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEpPortTableEntry_Print(sbZfKaEpPortTableEntry_t *pFromStruct) {
  SB_LOG("KaEpPortTableEntry:: enable15=0x%01x", (unsigned int)  pFromStruct->m_nEnable15);
  SB_LOG(" enable14=0x%01x", (unsigned int)  pFromStruct->m_nEnable14);
  SB_LOG(" enable13=0x%01x", (unsigned int)  pFromStruct->m_nEnable13);
  SB_LOG(" enable12=0x%01x", (unsigned int)  pFromStruct->m_nEnable12);
  SB_LOG("\n");

  SB_LOG("KaEpPortTableEntry:: enable11=0x%01x", (unsigned int)  pFromStruct->m_nEnable11);
  SB_LOG(" enable10=0x%01x", (unsigned int)  pFromStruct->m_nEnable10);
  SB_LOG(" enable9=0x%01x", (unsigned int)  pFromStruct->m_nEnable9);
  SB_LOG(" enable8=0x%01x", (unsigned int)  pFromStruct->m_nEnable8);
  SB_LOG("\n");

  SB_LOG("KaEpPortTableEntry:: enable7=0x%01x", (unsigned int)  pFromStruct->m_nEnable7);
  SB_LOG(" enable6=0x%01x", (unsigned int)  pFromStruct->m_nEnable6);
  SB_LOG(" enable5=0x%01x", (unsigned int)  pFromStruct->m_nEnable5);
  SB_LOG(" enable4=0x%01x", (unsigned int)  pFromStruct->m_nEnable4);
  SB_LOG("\n");

  SB_LOG("KaEpPortTableEntry:: enable3=0x%01x", (unsigned int)  pFromStruct->m_nEnable3);
  SB_LOG(" enable2=0x%01x", (unsigned int)  pFromStruct->m_nEnable2);
  SB_LOG(" enable1=0x%01x", (unsigned int)  pFromStruct->m_nEnable1);
  SB_LOG(" enable0=0x%01x", (unsigned int)  pFromStruct->m_nEnable0);
  SB_LOG("\n");

  SB_LOG("KaEpPortTableEntry:: resrv0=0x%01x", (unsigned int)  pFromStruct->m_nReserved0);
  SB_LOG(" trans=0x%02x", (unsigned int)  pFromStruct->m_nCountTrans);
  SB_LOG(" resrv1=0x%02x", (unsigned int)  pFromStruct->m_nReserved1);
  SB_LOG(" prepend=0x%01x", (unsigned int)  pFromStruct->m_nPrepend);
  SB_LOG("\n");

  SB_LOG("KaEpPortTableEntry:: inst=0x%08x", (unsigned int)  pFromStruct->m_Instruction);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEpPortTableEntry_SPrint(sbZfKaEpPortTableEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable15=0x%01x", (unsigned int)  pFromStruct->m_nEnable15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable14=0x%01x", (unsigned int)  pFromStruct->m_nEnable14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable13=0x%01x", (unsigned int)  pFromStruct->m_nEnable13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable12=0x%01x", (unsigned int)  pFromStruct->m_nEnable12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable11=0x%01x", (unsigned int)  pFromStruct->m_nEnable11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable10=0x%01x", (unsigned int)  pFromStruct->m_nEnable10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable9=0x%01x", (unsigned int)  pFromStruct->m_nEnable9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable8=0x%01x", (unsigned int)  pFromStruct->m_nEnable8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable7=0x%01x", (unsigned int)  pFromStruct->m_nEnable7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable6=0x%01x", (unsigned int)  pFromStruct->m_nEnable6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable5=0x%01x", (unsigned int)  pFromStruct->m_nEnable5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable4=0x%01x", (unsigned int)  pFromStruct->m_nEnable4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: enable3=0x%01x", (unsigned int)  pFromStruct->m_nEnable3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable2=0x%01x", (unsigned int)  pFromStruct->m_nEnable2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable1=0x%01x", (unsigned int)  pFromStruct->m_nEnable1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable0=0x%01x", (unsigned int)  pFromStruct->m_nEnable0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: resrv0=0x%01x", (unsigned int)  pFromStruct->m_nReserved0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," trans=0x%02x", (unsigned int)  pFromStruct->m_nCountTrans);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resrv1=0x%02x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," prepend=0x%01x", (unsigned int)  pFromStruct->m_nPrepend);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpPortTableEntry:: inst=0x%08x", (unsigned int)  pFromStruct->m_Instruction);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpPortTableEntry_Validate(sbZfKaEpPortTableEntry_t *pZf) {

  if (pZf->m_nEnable15 > 0x1) return 0;
  if (pZf->m_nEnable14 > 0x1) return 0;
  if (pZf->m_nEnable13 > 0x1) return 0;
  if (pZf->m_nEnable12 > 0x1) return 0;
  if (pZf->m_nEnable11 > 0x1) return 0;
  if (pZf->m_nEnable10 > 0x1) return 0;
  if (pZf->m_nEnable9 > 0x1) return 0;
  if (pZf->m_nEnable8 > 0x1) return 0;
  if (pZf->m_nEnable7 > 0x1) return 0;
  if (pZf->m_nEnable6 > 0x1) return 0;
  if (pZf->m_nEnable5 > 0x1) return 0;
  if (pZf->m_nEnable4 > 0x1) return 0;
  if (pZf->m_nEnable3 > 0x1) return 0;
  if (pZf->m_nEnable2 > 0x1) return 0;
  if (pZf->m_nEnable1 > 0x1) return 0;
  if (pZf->m_nEnable0 > 0x1) return 0;
  if (pZf->m_nReserved0 > 0x7) return 0;
  if (pZf->m_nCountTrans > 0x1f) return 0;
  if (pZf->m_nReserved1 > 0x7f) return 0;
  if (pZf->m_nPrepend > 0x1) return 0;
  /* pZf->m_Instruction implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpPortTableEntry_SetField(sbZfKaEpPortTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nenable15") == 0) {
    s->m_nEnable15 = value;
  } else if (SB_STRCMP(name, "m_nenable14") == 0) {
    s->m_nEnable14 = value;
  } else if (SB_STRCMP(name, "m_nenable13") == 0) {
    s->m_nEnable13 = value;
  } else if (SB_STRCMP(name, "m_nenable12") == 0) {
    s->m_nEnable12 = value;
  } else if (SB_STRCMP(name, "m_nenable11") == 0) {
    s->m_nEnable11 = value;
  } else if (SB_STRCMP(name, "m_nenable10") == 0) {
    s->m_nEnable10 = value;
  } else if (SB_STRCMP(name, "m_nenable9") == 0) {
    s->m_nEnable9 = value;
  } else if (SB_STRCMP(name, "m_nenable8") == 0) {
    s->m_nEnable8 = value;
  } else if (SB_STRCMP(name, "m_nenable7") == 0) {
    s->m_nEnable7 = value;
  } else if (SB_STRCMP(name, "m_nenable6") == 0) {
    s->m_nEnable6 = value;
  } else if (SB_STRCMP(name, "m_nenable5") == 0) {
    s->m_nEnable5 = value;
  } else if (SB_STRCMP(name, "m_nenable4") == 0) {
    s->m_nEnable4 = value;
  } else if (SB_STRCMP(name, "m_nenable3") == 0) {
    s->m_nEnable3 = value;
  } else if (SB_STRCMP(name, "m_nenable2") == 0) {
    s->m_nEnable2 = value;
  } else if (SB_STRCMP(name, "m_nenable1") == 0) {
    s->m_nEnable1 = value;
  } else if (SB_STRCMP(name, "m_nenable0") == 0) {
    s->m_nEnable0 = value;
  } else if (SB_STRCMP(name, "m_nreserved0") == 0) {
    s->m_nReserved0 = value;
  } else if (SB_STRCMP(name, "m_ncounttrans") == 0) {
    s->m_nCountTrans = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else if (SB_STRCMP(name, "m_nprepend") == 0) {
    s->m_nPrepend = value;
  } else if (SB_STRCMP(name, "m_instruction") == 0) {
    s->m_Instruction = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
