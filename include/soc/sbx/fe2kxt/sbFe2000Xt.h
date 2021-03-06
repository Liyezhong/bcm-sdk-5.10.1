/* -*- Mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
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
 * $Id: sbFe2000Xt.h 1.6.184.1 Broadcom SDK $
 *
 * sbFe2000XtCommon.h : FE2000XT Common defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE_2000XT_H_
#define _SB_FE_2000XT_H_

#include <soc/sbx/fe2k_common/sbFe2000Common.h>

#define SB_FE2000XT_HEADER_CAPTURE_SIZE_IN_BYTES 192

#define SB_FE2000XT_DMA_SRC_MM0_NARROW0  ((SB_FE2000XT_MM0_ID << SB_FE2000XT_MMU_POSITION) | 0x0)
#define SB_FE2000XT_DMA_SRC_MM0_NARROW1  ((SB_FE2000XT_MM0_ID << SB_FE2000XT_MMU_POSITION) | 0x1)
#define SB_FE2000XT_DMA_SRC_MM0_WIDE     ((SB_FE2000XT_MM0_ID << SB_FE2000XT_MMU_POSITION) | 0x2)
#define SB_FE2000XT_DMA_SRC_MM0_INT0      ((SB_FE2000XT_MM0_ID << SB_FE2000XT_MMU_POSITION) | 0x3)
#define SB_FE2000XT_DMA_SRC_MM0_INT1      ((SB_FE2000XT_MM0_ID << SB_FE2000XT_MMU_POSITION) | 0x4)
#define SB_FE2000XT_DMA_SRC_MM1_NARROW0  ((SB_FE2000XT_MM1_ID << SB_FE2000XT_MMU_POSITION) | 0x5)
#define SB_FE2000XT_DMA_SRC_MM1_NARROW1  ((SB_FE2000XT_MM1_ID << SB_FE2000XT_MMU_POSITION) | 0x6)
#define SB_FE2000XT_DMA_SRC_MM1_WIDE     ((SB_FE2000XT_MM1_ID << SB_FE2000XT_MMU_POSITION) | 0x7)
#define SB_FE2000XT_DMA_SRC_MM1_INT0      ((SB_FE2000XT_MM1_ID << SB_FE2000XT_MMU_POSITION) | 0x8)
#define SB_FE2000XT_DMA_SRC_MM1_INT1      ((SB_FE2000XT_MM1_ID << SB_FE2000XT_MMU_POSITION) | 0x9)
#define SB_FE2000XT_DMA_SRC_LR                ((SB_FE2000XT_LR_ID << SB_FE2000XT_LR_POSITION) | 0xA)
#define SB_FE2000XT_DMA_SRC_RC_SC0            ((SB_FE2000XT_RC_ID << SB_FE2000XT_RC_POSITION) | 0xB)
#define SB_FE2000XT_DMA_SRC_RC_SC1            ((SB_FE2000XT_RC_ID << SB_FE2000XT_RC_POSITION) | 0xC)
#define SB_FE2000XT_DMA_SRC_CM_MEM            ((SB_FE2000XT_CM_ID << SB_FE2000XT_CM_POSITION) | 0xD)
#define SB_FE2000XT_DMA_SRC_CM_FIFO           ((SB_FE2000XT_CM_ID << SB_FE2000XT_CM_POSITION) | 0xE)

#define SB_FE2000XT_MM0_ID                       0
#define SB_FE2000XT_MM1_ID                       1
#define SB_FE2000XT_LR_ID                        2
#define SB_FE2000XT_RC_ID                        3
#define SB_FE2000XT_CM_ID                        4
#define SB_FE2000XT_MMU_POSITION                 24
#define SB_FE2000XT_LR_POSITION                  24
#define SB_FE2000XT_RC_POSITION                  24
#define SB_FE2000XT_CM_POSITION                  24
#define SB_FE2000XT_MMU_MASK                     1
#define SB_FE2000XT_LR_MASK                      1
#define SB_FE2000XT_CM_MASK                      1
#define SB_FE2000XT_RC_MASK                      1


#define SB_FE2000XT_PR_NUM_PRE 8
#define SB_FE2000XT_PR_NUM_CC 4
#define SB_FE2000XT_PR_INIT_TIMEOUT 100
#define SB_FE2000XT_PR_ENABLE_SR0 0
#define SB_FE2000XT_PR_ENABLE_SR1 1
#define SB_FE2000XT_PR_ENABLE_AG0 2
#define SB_FE2000XT_PR_ENABLE_XG2 2
#define SB_FE2000XT_PR_ENABLE_AG1 3
#define SB_FE2000XT_PR_ENABLE_XG3 3
#define SB_FE2000XT_PR_ENABLE_XG0 4
#define SB_FE2000XT_PR_ENABLE_XG1 5
#define SB_FE2000XT_PR_ENABLE_PCI 6
#define SB_FE2000XT_PR_ENABLE_PED 7
#define SB_FE2000XT_PR_PAGES_PREFETCHED 8
#define SB_FE2000XT_PRE7_PAGES_PREFETCHED 16
#define SB_FE2000XT_PTE7_PAGES_PREFETCHED 16

#define SB_FE2000XT_PR_CC_KEY_WIDTH_BITS (214)
#define SB_FE2000XT_PR_CC_KEY_PKT_DATA_WIDTH_BITS (200)
#define SB_FE2000XT_PR_CC_KEY_STATE_WIDTH_BITS (14)
#define SB_FE2000XT_PR_CC_KEY_VALID_WIDTH_BITS (2)
#define SB_FE2000XT_PR_CC_KEY_VALID (3)
#define SB_FE2000XT_PR_CC_KEY_INVALID (0)
#define SB_FE2000XT_PR_CC_LAST_VALID_RULE (255)
#define SB_FE2000XT_PR_CC_LOWEST_PRE (2)
#define SB_FE2000XT_PR_CC_HIGHEST_PRE (5)

#define SB_FE2000XT_PT_NUM_PTE 8
#define SB_FE2000XT_PT_INIT_TIMEOUT 100
#define SB_FE2000XT_PT_ENABLE_SR0 0
#define SB_FE2000XT_PT_ENABLE_SR1 1
#define SB_FE2000XT_PT_ENABLE_AG0 2
#define SB_FE2000XT_PT_ENABLE_XG2 2
#define SB_FE2000XT_PT_ENABLE_AG1 3
#define SB_FE2000XT_PT_ENABLE_XG3 3
#define SB_FE2000XT_PT_ENABLE_XG0 4
#define SB_FE2000XT_PT_ENABLE_XG1 5
#define SB_FE2000XT_PT_ENABLE_PCI 6
#define SB_FE2000XT_PT_ENABLE_PPE 7
#define SB_FE2000XT_PT_PORT_TO_CHANNEL_XG0 2
#define SB_FE2000XT_PT_PORT_TO_CHANNEL_XG1 3
#define SB_FE2000XT_PT_PORT_TO_CHANNEL_XG2 4
#define SB_FE2000XT_PT_PORT_TO_CHANNEL_XG3 5
#define SB_FE2000XT_PT_PORT_TO_CHANNEL_DISABLE 7

#define SB_FE2000XT_PB_BUFFER_SIZE_IN_BYTES 192
#define SB_FE2000XT_PB_NUM_PACKET_BUFFERS 2
#define SB_FE2000XT_PB_PAGES_PER_PACKET_BUFFER 4094

#define SB_FE2000XT_PP_NUM_GLOBAL_STATION_MACS 16
#define SB_FE2000XT_PP_NUM_PORT_STATION_MACS   4
#define SB_FE2000XT_PP_NUM_IPV4_FILTERS        8
#define SB_FE2000XT_PP_NUM_IPV6_FILTERS        4
#define SB_FE2000XT_PP_NUM_QUEUES              256
#define SB_FE2000XT_PP_NUM_NATIVE_MAPNUMBERS   32
#define SB_FE2000XT_PP_NUM_CHANNELS            2

#define SB_FE2000XT_LR_INSTRUCTION_SIZE  12
#define SB_FE2000XT_LR_NUMBER_OF_STREAMS 8
#define SB_FE2000XT_LR_NUMBER_OF_INSTRS 1024

#define SB_FE2000XT_RC_INST_PER_DMA 8
#define SB_FE2000XT_RC_RULES_PER_FILTERSET 768
#define SB_FE2000XT_RC_NUM_SUPERBLOCKS 6
#define SB_FE2000XT_RC_WORDS_PER_PATTERN 4
#define SB_FE2000XT_RC_INSTRUCTION_WORDS_IN_DMA_BUFFER 32
#define SB_FE2000XT_RC_NUM_RR_MEM_LOGICAL 2
#define SB_FE2000XT_RC_RR_MAX_ADDRESS 0x6000

/* These enums needs to be here so that tests can use them also. */
typedef enum _sbFe2000XtInitParamsMmCmu0MemoryConnection
{
  SB_FE2000XT_MM_CMU0_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_CMU0_CONNECTED_TO_INTERNAL_SINGLE_RAM0 = 1,
  SB_FE2000XT_MM_CMU0_CONNECTED_TO_NARROWPORT_DDRII_PORT0 = 2,
  SB_FE2000XT_MM_CMU0_CONNECTED_TO_ILLEGAL = 3
} sbFe2000XtInitParamsMmCmu0MemoryConnection_t;

typedef enum _sbFe2000XtInitParamsMmCmu1MemoryConnection
{
  SB_FE2000XT_MM_CMU1_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_CMU1_CONNECTED_TO_INTERNAL_SINGLE_RAM1 =1,
  SB_FE2000XT_MM_CMU1_CONNECTED_TO_NARROWPORT_DDRII_PORT1 = 2,
  SB_FE2000XT_MM_CMU1_CONNECTED_TO_ILLEGAL = 3
} sbFe2000XtInitParamsMmCmu1MemoryConnection_t;

typedef enum _sbFe2000XtInitParamsMmPmuMemoryConnection
{
  SB_FE2000XT_MM_PMU_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_PMU_CONNECTED_TO_INTERNAL_SINGLE_RAM0 = 1,
  SB_FE2000XT_MM_PMU_CONNECTED_TO_INTERNAL_SINGLE_RAM1 = 2,
  SB_FE2000XT_MM_PMU_CONNECTED_TO_WIDEPORT_DDRII = 3
} sbFe2000XtInitParamsMmPmuMemoryConnection_t;

typedef enum _sbFe2000XtInitParamsMmLrp4MemoryConnection
{
  SB_FE2000XT_MM_LRP4_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_LRP4_CONNECTED_TO_WIDEPORT_DDRII_PORT=1
} sbFe2000XtInitParamsMmLrp4MemoryConnection_t;

typedef enum _sbFe2000XtInitParamsMmLrp3MemoryConnection
{
  SB_FE2000XT_MM_LRP3_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_LRP3_CONNECTED_TO_INTERNAL_SINGLE_RAM1 = 1
} sbFe2000XtInitParamsMmLrp3MemoryConnection_t;

typedef enum _sbFe2000XtInitParamsMmLrp2MemoryConnection
{
  SB_FE2000XT_MM_LRP2_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_LRP2_CONNECTED_TO_INTERNAL_SINGLE_RAM0 = 1
} sbFe2000XtInitParamsMmLrp2MemoryConnection_t;

typedef enum _sbFe2000XtInitParamsMmLrp1MemoryConnection
{
  SB_FE2000XT_MM_LRP1_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_LRP1_CONNECTED_TO_NARROWPORT_DDRII_PORT1 =1
} sbFe2000XtInitParamsMmLrp1MemoryConnection_t;

typedef enum _sbFe2000XtInitParamsMmLrp0MemoryConnection
{
  SB_FE2000XT_MM_LRP0_CONNECTED_TO_NO_RESOURCE = 0,
  SB_FE2000XT_MM_LRP0_CONNECTED_TO_NARROWPORT_DDRII_PORT0 =1
} sbFe2000XtInitParamsMmLrp0MemoryConnection_t;


typedef enum _sbFe2000XtInitParamsMmInternalRamConfiguration
{
  SB_FE2000XT_MM_RAM0_8KBY72_AND_RAM1_8KBY72   = 0,
  SB_FE2000XT_MM_RAM0_16KBY36_AND_RAM1_8KBY72  = 1,
  SB_FE2000XT_MM_RAM0_8KBY72_AND_RAM1_16KBY36  = 2,
  SB_FE2000XT_MM_RAM0_16KBY36_AND_RAM1_16KBY36 = 3
} sbFe2000XtInitParamsMmInternalRamConfiguration_t;

typedef enum _sbFe2000XtInitParamsMmInternalRamDmaControllerConfig
{
  SB_FE2000XT_ONE_32_BIT_DAM_XFERS = 0,
  SB_FE2000XT_TWO_32_BIT_DAM_XFERS = 1
} sbFe2000XtInitParamsMmInternalRamDmaControllerConfig_t;

typedef enum _sbFe2000XtInitParamsNarrowPortRamConfiguration
{
  SB_FE2000XT_MM_TWO_BY_9_RAMS_CONNECTED = 0,
  SB_FE2000XT_MM_ONE_BY_18_RAM_CONNECTED = 1
} sbFe2000XtInitParamsNarrowPortRamConfiguration_t;

typedef enum _sbFe2000XtInitParamsWidePortRamConfiguration
{
  SB_FE2000XT_MM_TWO_BY_18_RAMS_CONNECTED = 0,
  SB_FE2000XT_MM_ONE_BY_36_RAM_CONNECTED  = 1,
  SB_FE2000XT_MM_TWO_BY_9_RAM_AND_TOLOWER_UPPER_DATAPIN_NOCONNECTION  = 2,
  SB_FE2000XT_MM_ONE_BY_18_RAM_AND_TOLOWER_UPPER_DATAPIN_NOCONNECTION = 3

} sbFe2000XtInitParamsWidePortRamConfiguration_t;

typedef enum _sbFe2000XtInitParamsMmProtectionSchemes
{
  SB_FE2000XT_MM_36BITS_NOPROTECTION = 0,
  SB_FE2000XT_MM_35BITS_1BITPARITY   = 1,
  SB_FE2000XT_MM_34BITS_2BITPARITY   = 2,
  SB_FE2000XT_MM_32BITS_4BITPARITY   = 3,
  SB_FE2000XT_MM_30BITS_6BITSECECC   = 4,
  SB_FE2000XT_MM_29BITS_6BITSECDED   = 5,
  SB_FE2000XT_MM_PROTECTION_ILLEGAL0 = 6,
  SB_FE2000XT_MM_PROTECTION_ILLEGAL1 = 7
} sbFe2000XtInitParamsMmProtectionSchemes_t;

#endif 
