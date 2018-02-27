#ifndef __SB_ELIB_STATUS_H__
#define __SB_ELIB_STATUS_H__
/**
 * @file sbElibStatus.h Enumerated Return Codes
 *
 * <pre>
 * ====================================================
 * ==  sbElibStatus.h - Egress Library public API
 * ====================================================
 *
 * WORKING REVISION: $Id: sbElibStatus.h 1.3.196.1 Broadcom SDK $
 *
 * 
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
 *
 *
 * MODULE NAME:
 *
 *     sbElibStatus.h
 *
 * ABSTRACT:
 *
 *     Egress Library Error Enumeration
 *
 * </pre>
 */

#include "sbStatus.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Enumerated Type of Return Codes
 *
 **/
typedef enum sbElibStatus_et_e {
  SB_ELIB_OK = 0,
  SB_ELIB_BAD_ARGS = SB_BUILD_ERR_CODE(SB_VENDOR_ID_SANDBURST, SB_MODULE_ID_QE_ELIB, 0x1),
  SB_ELIB_BAD_IDX,
  SB_ELIB_INIT_FAIL,
  SB_ELIB_IND_MEM_TIMEOUT,
  SB_ELIB_INIT_CIT_FAIL,
  SB_ELIB_INIT_CRT_FAIL,
  SB_ELIB_GENERAL_FAIL,
  SB_ELIB_PORT_ST_GET_FAIL,
  SB_ELIB_PORT_ST_SET_FAIL,
  SB_ELIB_VIT_SET_FAIL,
  SB_ELIB_VIT_GET_FAIL,
  SB_ELIB_VRT_SET_FAIL,
  SB_ELIB_VRT_GET_FAIL,
  SB_ELIB_MEM_ALLOC_FAIL,
  SB_ELIB_PORT_CFG_GET_FAIL,
  SB_ELIB_PORT_CFG_SET_FAIL,
  SB_ELIB_BIST_FAIL,
  SB_ELIB_SEM_GET_FAIL,
  SB_ELIB_SEM_GIVE_FAIL,
  SB_ELIB_DMA_FAIL,
  SB_ELIB_COUNT_FAIL,
  SB_ELIB_VLAN_MEM_ALLOC_FAIL,
  SB_ELIB_VLAN_MEM_FREE_FAIL,
  SB_ELIB_BAD_VID,
  /* leave as last */
  SB_ELIB_STATUS_LAST
} sbElibStatus_et;

char * sbGetElibStatusString(sbStatus_t status);
#ifdef __cplusplus
}
#endif

#endif /* __SB_ELIB_STATUS_H__ */
