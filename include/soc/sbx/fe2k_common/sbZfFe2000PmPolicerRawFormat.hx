/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000PmPolicerRawFormat.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_FE_2000_PM_POLICERRAWFORMAT_H
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_H

#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_SIZE_IN_BYTES 8
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_SIZE 8
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_UPROFILEID_BITS "63:53"
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_UMETERSTATE_BITS "52:0"



/** @brief  MMU Policer Raw Format 

  It's for internal use only
  Given that the actual layout per policer 
  as per the group configuration & profile configuration.
*/

typedef struct _sbZfFe2000PmPolicerRawFormat {
/** @brief <p> Profile Id</p> */

  uint32_t uProfileId;
/** @brief <p> timestamp+bkte+bktc</p> */

  uint64_t uMeterState;
} sbZfFe2000PmPolicerRawFormat_t;

uint32_t
sbZfFe2000PmPolicerRawFormat_Pack(sbZfFe2000PmPolicerRawFormat_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex);
void
sbZfFe2000PmPolicerRawFormat_Unpack(sbZfFe2000PmPolicerRawFormat_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex);
void
sbZfFe2000PmPolicerRawFormat_InitInstance(sbZfFe2000PmPolicerRawFormat_t *pFrame);

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_SET_PROFID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_SET_METERSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x1f) | (((nFromData) >> 48) & 0x1f); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_GET_PROFID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[7] << 3; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_GET_METERSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[3]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#endif
