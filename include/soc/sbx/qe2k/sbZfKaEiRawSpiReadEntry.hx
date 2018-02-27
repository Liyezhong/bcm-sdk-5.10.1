/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEiRawSpiReadEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAEIRAWSPIREADENTRY_H
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_H

#define SB_ZF_ZFKAEIRAWSPIREADENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_SIZE 4
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NDESTCHANNEL_BITS "31:24"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NSIZEMASK8_BITS "23:23"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NSIZEMASK7_0_BITS "22:15"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NRBLOOPBACKONLY_BITS "14:14"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NLINEPTR_BITS "13:4"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NBYTEPTR_BITS "3:0"


typedef struct _sbZfKaEiRawSpiReadEntry {
  uint32_t m_nDestChannel;
  uint32_t m_nSizeMask8;
  uint32_t m_nSizeMask7_0;
  uint32_t m_nRbLoopbackOnly;
  uint32_t m_nLinePtr;
  uint32_t m_nBytePtr;
} sbZfKaEiRawSpiReadEntry_t;

uint32_t
sbZfKaEiRawSpiReadEntry_Pack(sbZfKaEiRawSpiReadEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex);
void
sbZfKaEiRawSpiReadEntry_Unpack(sbZfKaEiRawSpiReadEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex);
void
sbZfKaEiRawSpiReadEntry_InitInstance(sbZfKaEiRawSpiReadEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif