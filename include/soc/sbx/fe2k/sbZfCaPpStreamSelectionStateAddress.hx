/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPpStreamSelectionStateAddress.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_H
#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_H

#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_SIZE_IN_BYTES 2
#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_SIZE 2
#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_M_ULOCALSTATIONMATCH_BITS "11:11"
#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_M_UHEADERTRANSITIONNUMBER_BITS "10:5"
#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_M_ULASTSTATE_BITS "4:0"


typedef struct _sbZfCaPpStreamSelectionStateAddress {
  uint32_t m_uLocalStationMatch;
  uint32_t m_uHeaderTransitionNumber;
  uint32_t m_uLastState;
} sbZfCaPpStreamSelectionStateAddress_t;

uint32_t
sbZfCaPpStreamSelectionStateAddress_Pack(sbZfCaPpStreamSelectionStateAddress_t *pFrom,
                                         uint8_t *pToData,
                                         uint32_t nMaxToDataIndex);
void
sbZfCaPpStreamSelectionStateAddress_Unpack(sbZfCaPpStreamSelectionStateAddress_t *pToStruct,
                                           uint8_t *pFromData,
                                           uint32_t nMaxToDataIndex);
void
sbZfCaPpStreamSelectionStateAddress_InitInstance(sbZfCaPpStreamSelectionStateAddress_t *pFrame);

#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_SET_LCLSTAMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_SET_HDRTRANNUMBER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_SET_LASTSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_GET_LCLSTAMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_GET_HDRTRANNUMBER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x07) << 3; \
          } while(0)

#define SB_ZF_CAPPSTREAMSELECTIONSTATEADDRESS_GET_LASTSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
