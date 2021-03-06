/*
 * $Id: sbZfCaPpAggregateHashFormatConsole.c,v 1.1.48.3 2011/05/22 05:39:26 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfCaPpAggregateHashFormatConsole.hx"



/* Print members in struct */
void
sbZfCaPpAggregateHashFormat_Print(sbZfCaPpAggregateHashFormat_t *pFromStruct) {
  SB_LOG("CaPpAggregateHashFormat:: aggr0=0x%01x", (unsigned int)  pFromStruct->m_uAggregateHash_31_28);
  SB_LOG(" aggr1=0x%03x", (unsigned int)  pFromStruct->m_uAggregateHash_27_16);
  SB_LOG(" aggr2=0x%04x", (unsigned int)  pFromStruct->m_uAggregateHash_15_0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPpAggregateHashFormat_SPrint(sbZfCaPpAggregateHashFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpAggregateHashFormat:: aggr0=0x%01x", (unsigned int)  pFromStruct->m_uAggregateHash_31_28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," aggr1=0x%03x", (unsigned int)  pFromStruct->m_uAggregateHash_27_16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," aggr2=0x%04x", (unsigned int)  pFromStruct->m_uAggregateHash_15_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPpAggregateHashFormat_Validate(sbZfCaPpAggregateHashFormat_t *pZf) {

  if (pZf->m_uAggregateHash_31_28 > 0xf) return 0;
  if (pZf->m_uAggregateHash_27_16 > 0xfff) return 0;
  if (pZf->m_uAggregateHash_15_0 > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPpAggregateHashFormat_SetField(sbZfCaPpAggregateHashFormat_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uaggregatehash_31_28") == 0) {
    s->m_uAggregateHash_31_28 = value;
  } else if (SB_STRCMP(name, "m_uaggregatehash_27_16") == 0) {
    s->m_uAggregateHash_27_16 = value;
  } else if (SB_STRCMP(name, "m_uaggregatehash_15_0") == 0) {
    s->m_uAggregateHash_15_0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
