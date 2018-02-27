/*
 * $Id: sbZfKaRbClassHashSVlanIPv6Console.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbClassHashSVlanIPv6Console.hx"



/* Print members in struct */
void
sbZfKaRbClassHashSVlanIPv6_Print(sbZfKaRbClassHashSVlanIPv6_t *pFromStruct) {
  SB_LOG("KaRbClassHashSVlanIPv6:: protocol=0x%02x", (unsigned int)  pFromStruct->m_nProtocol);
  SB_LOG(" flow=0x%05x", (unsigned int)  pFromStruct->m_nFlow);
  SB_LOG(" pad2=0x%07x", (unsigned int)  pFromStruct->m_nPadWord2);
  SB_LOG("\n");

  SB_LOG("KaRbClassHashSVlanIPv6:: pad1=0x%015llx", (uint64_t)  pFromStruct->m_nPadWord1);
  SB_LOG(" ipsahigh=0x%016llx", (uint64_t)  pFromStruct->m_nnIpSaHigh);
  SB_LOG("\n");

  SB_LOG("KaRbClassHashSVlanIPv6:: ipsalow=0x%016llx", (uint64_t)  pFromStruct->m_nnIpSaLow);
  SB_LOG("\n");

  SB_LOG("KaRbClassHashSVlanIPv6:: ipdahigh=0x%016llx", (uint64_t)  pFromStruct->m_nnIpDaHigh);
  SB_LOG("\n");

  SB_LOG("KaRbClassHashSVlanIPv6:: ipdalow=0x%016llx", (uint64_t)  pFromStruct->m_nnIpDaLow);
  SB_LOG(" skt=0x%08x", (unsigned int)  pFromStruct->m_nSocket);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbClassHashSVlanIPv6_SPrint(sbZfKaRbClassHashSVlanIPv6_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: protocol=0x%02x", (unsigned int)  pFromStruct->m_nProtocol);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," flow=0x%05x", (unsigned int)  pFromStruct->m_nFlow);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pad2=0x%07x", (unsigned int)  pFromStruct->m_nPadWord2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: pad1=0x%015llx", (uint64_t)  pFromStruct->m_nPadWord1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipsahigh=0x%016llx", (uint64_t)  pFromStruct->m_nnIpSaHigh);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: ipsalow=0x%016llx", (uint64_t)  pFromStruct->m_nnIpSaLow);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: ipdahigh=0x%016llx", (uint64_t)  pFromStruct->m_nnIpDaHigh);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassHashSVlanIPv6:: ipdalow=0x%016llx", (uint64_t)  pFromStruct->m_nnIpDaLow);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," skt=0x%08x", (unsigned int)  pFromStruct->m_nSocket);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassHashSVlanIPv6_Validate(sbZfKaRbClassHashSVlanIPv6_t *pZf) {

  if (pZf->m_nProtocol > 0xff) return 0;
  if (pZf->m_nFlow > 0xfffff) return 0;
  if (pZf->m_nPadWord2 > 0x7ffffff) return 0;
  if (pZf->m_nPadWord1 > 0x1FFFFFFFFFFFFFFULL) return 0;
  /* pZf->m_nnIpSaHigh implicitly masked by data type */
  /* pZf->m_nnIpSaLow implicitly masked by data type */
  /* pZf->m_nnIpDaHigh implicitly masked by data type */
  /* pZf->m_nnIpDaLow implicitly masked by data type */
  /* pZf->m_nSocket implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassHashSVlanIPv6_SetField(sbZfKaRbClassHashSVlanIPv6_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nprotocol") == 0) {
    s->m_nProtocol = value;
  } else if (SB_STRCMP(name, "m_nflow") == 0) {
    s->m_nFlow = value;
  } else if (SB_STRCMP(name, "m_npadword2") == 0) {
    s->m_nPadWord2 = value;
  } else if (SB_STRCMP(name, "m_npadword1") == 0) {
    s->m_nPadWord1 = value;
  } else if (SB_STRCMP(name, "m_nnipsahigh") == 0) {
    s->m_nnIpSaHigh = value;
  } else if (SB_STRCMP(name, "m_nnipsalow") == 0) {
    s->m_nnIpSaLow = value;
  } else if (SB_STRCMP(name, "m_nnipdahigh") == 0) {
    s->m_nnIpDaHigh = value;
  } else if (SB_STRCMP(name, "m_nnipdalow") == 0) {
    s->m_nnIpDaLow = value;
  } else if (SB_STRCMP(name, "m_nsocket") == 0) {
    s->m_nSocket = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
