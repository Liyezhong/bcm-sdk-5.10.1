/*
 * $Id: type23.h 1.2.20.2 Broadcom SDK $
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
 * File:        soc/shared/dcbformats/type23.h
 * Purpose:     Define dma control block (DCB) format for a type23 DCB
 *              used by the 56640 (Triumph3/Firebolt4)
 *
 *              This file is shared between the SDK and the embedded applications.
 */

#ifndef _SOC_SHARED_DCBFORMATS_TYPE23_H
#define _SOC_SHARED_DCBFOMATS_TYPE23_H

/*
 * DMA Control Block - Type 23
 * Used on 56640 devices
 * 16 words
 */
typedef struct {
        uint32  addr;                   /* T23.0: physical address */
                                        /* T23.1: Control 0 */
#ifdef  LE_HOST
        uint32  c_count:16,             /* Requested byte count */
                c_chain:1,              /* Chaining */
                c_sg:1,                 /* Scatter Gather */
                c_reload:1,             /* Reload */
                c_hg:1,                 /* Higig (TX) */
                c_stat:1,               /* Update stats (TX) */
                c_pause:1,              /* Pause packet (TX) */
                c_purge:1,              /* Purge packet (TX) */
                :9;                     /* Don't care */
#else
        uint32  :9,                     /* Don't care */
                c_purge:1,              /* Purge packet (TX) */
                c_pause:1,              /* Pause packet (TX) */
                c_stat:1,               /* Update stats (TX) */
                c_hg:1,                 /* Higig (TX) */
                c_reload:1,             /* Reload */
                c_sg:1,                 /* Scatter Gather */
                c_chain:1,              /* Chaining */
                c_count:16;             /* Requested byte count */
#endif  /* LE_HOST */
        uint32  reason_hi;              /* T23.2: CPU opcode (hi bits) */
        uint32  reason;                 /* T23.3: CPU opcode (low bits) */
#ifdef  LE_HOST
    union {                             /* T23.4 */
        struct {
            uint32  queue_num:16,       /* Queue number for CPU packets */
                    outer_vid:12,       /* Outer VLAN ID */
                    cpu_opcode_type:4;  /* CPU opcode type */
        } overlay1;
        struct {
            uint32  cpu_cos:6,          /* CPU COS */
                    :26;                /* Reserved */
        } overlay2;
    } word4;

                                        /* T23.5 */
        uint32  srcport:8,              /* Source port number */
                pkt_len:14,             /* Packet length */
                match_rule:8,           /* Matched FP rule */
                hgi:2;                  /* Higig Interface Format Indicator */
#else
    union {                             /* T23.4 */
        struct {
            uint32  cpu_opcode_type:4,  /* CPU opcode type */
                    outer_vid:12,       /* Outer VLAN ID */
                    queue_num:16;       /* Queue number for CPU packets */
        } overlay1;
        struct {
            uint32  :26,                /* Reserved */
                    cpu_cos:6;          /* CPU COS */
        } overlay2;
    } word4;

                                        /* T23.5 */
        uint32  hgi:2,                  /* Higig Interface Format Indicator */
                match_rule:8,           /* Matched FP rule */
                pkt_len:14,             /* Packet length */
                srcport:8;              /* Source port number */
#endif

        uint32  mh0;                    /* T23.6: Module Header word 0 */
        uint32  mh1;                    /* T23.7: Module Header word 1 */
        uint32  mh2;                    /* T23.8: Module Header word 2 */
        uint32  mh3;                    /* T23.9: Module Header word 3 */
#ifdef  LE_HOST
                                        /* T23.10 */
        uint32  inner_pri:3,            /* Inner priority */
                dvp_nhi_sel:1,          /* DVP overlay indicator */
                regen_crc:1,            /* Packet modified and needs new CRC */
                repl_nhi:19,            /* Replication or Next Hop index */
                vfi_valid:1,            /* Validates VFI field */
                :7;                     /* Reserved */

    union {                             /* T23.11 */
        struct {
            uint32  dscp:6,             /* New DSCP (+ special pkt type:3) */
                    chg_tos:1,          /* DSCP has been changed by HW */
                    vntag_action:2,     /* VN tag action */
                    outer_cfi:1,        /* Outer Canoncial Format Indicator */
                    outer_pri:3,        /* Outer priority */
                    decap_tunnel_type:5,/* Tunnel type that was decapsulated */
                    inner_vid:12,       /* Inner VLAN ID */
                    inner_cfi:1,        /* Inner Canoncial Format Indicator */
                    :1;                 /* Reserved */
        } overlay1;
        struct {
            uint32  special_pkt_type:3, /* Special packet type (OAM, BFD) */
                    :29;                /* Reserved */
        } overlay2;
    } word11;

    union {                             /* T23.12 */
        struct {
            uint32  timestamp;          /* Timestamp */
        } overlay1;
        struct {
            uint32  vfi:14,             /* VFI or FID */
                    :18;                /* Reserved */
        } overlay2;
    } word12;

                                        /* T23.13 */
        uint32  itag_status:2,          /* Ingress Inner tag status */
                otag_action:2,          /* Ingress Outer tag action */
                itag_action:2,          /* Ingress Inner tag action */
                service_tag:1,          /* SD tag present */
                switch_pkt:1,           /* Switched packet */
                all_switch_drop:1,      /* All switched copies dropped */
                hg_type:1,              /* 0: Higig+, 1: Higig2 */
                src_hg:1,               /* Source is Higig */
                l3routed:1,             /* Any IP routed packet */
                l3only:1,               /* L3 only IPMC packet */
                replicated:1,           /* Replicated copy */
                hg2_ext_hdr:1,          /* Extended Higig2 header valid */
                special_pkt:1,          /* Special Packet indicator */
                do_not_change_ttl:1,    /* Do not change TTL */
                bpdu:1,                 /* BPDU Packet */
                timestamp_type:2,       /* Timestamp type */
                loopback_pkt_type:4,    /* Loopback packet type */
                mtp_index:5,            /* MTP index */
                ecn:2,                  /* New ECN value */
                chg_ecn:1;              /* ECN changed */

    union {                             /* T23.14 */
        struct {
            uint32  timestamp_hi;       /* Timestamp upper bits */
        } overlay1;
        struct {
            uint32  eh_seg_sel:3,       /* HG2 EH seg sel */
                    eh_tag_type:2,      /* HG2 EH tag type */
                    eh_tm:1,            /* HG2 EH tm */
                    eh_queue_tag: 16,   /* HG2 EH queue tag */                
                    eh_tag_reserved:10; /* Reserved */
        } overlay2;
        struct {
            uint32  rx_bfd_session_index:11,    /* Rx BFD pkt session ID */
                    rx_bfd_start_offset_type:2, /* Rx BFD pkt st offset type */
                    rx_bfd_start_offset:8,      /* Rx BFD pkt start offset */
                   :11;                        /* Reserved */
        } overlay3;
    } word14;

                                        /* T23.15: DMA Status 0 */
        uint32  count:16,               /* Transferred byte count */
                end:1,                  /* End bit (RX) */
                start:1,                /* Start bit (RX) */
                error:1,                /* Cell Error (RX) */
                :12,                    /* Reserved */
                done:1;                 /* Descriptor Done */
#else
                                        /* T23.10 */
        uint32  :7,                     /* Reserved */
                vfi_valid:1,            /* Validates VFI field */
                repl_nhi:19,            /* Replication or Next Hop index */
                regen_crc:1,            /* Packet modified and needs new CRC */
                dvp_nhi_sel:1,          /* DVP overlay indicator */
                inner_pri:3;            /* Inner priority */

    union {                             /* T23.11 */
        struct {
            uint32  :1,                 /* Reserved */
                    inner_cfi:1,        /* Inner Canoncial Format Indicator */
                    inner_vid:12,       /* Inner VLAN ID */
                    decap_tunnel_type:5,/* Tunnel type that was decapsulated */
                    outer_pri:3,        /* Outer priority */
                    outer_cfi:1,        /* Outer Canoncial Format Indicator */
                    vntag_action:2,     /* VN tag action */
                    chg_tos:1,          /* DSCP has been changed by HW */
                    dscp:6;             /* New DSCP (+ special pkt type:3) */
        } overlay1;
        struct {
            uint32  :29,                /* Reserved */
                    special_pkt_type:3; /* Special packet type (OAM, BFD) */
        } overlay2;
    } word11;

    union {                             /* T23.12 */
        struct {
            uint32  timestamp;          /* Timestamp */
        } overlay1;
        struct {
            uint32  :18,                /* Reserved */
                    vfi:14;             /* VFI or FID */
        } overlay2;
    } word12;

                                        /* T23.13 */
        uint32  chg_ecn:1,              /* ECN changed */
                ecn:2,                  /* New ECN value */
                mtp_index:5,            /* MTP index */
                loopback_pkt_type:4,    /* Loopback packet type */
                timestamp_type:2,       /* Timestamp type */
                bpdu:1,                 /* BPDU Packet */
                do_not_change_ttl:1,    /* Do not change TTL */
                special_pkt:1,          /* Special Packet indicator */
                hg2_ext_hdr:1,          /* Extended Higig2 header valid */
                replicated:1,           /* Replicated copy */
                l3only:1,               /* L3 only IPMC packet */
                l3routed:1,             /* Any IP routed packet */
                src_hg:1,               /* Source is Higig */
                hg_type:1,              /* 0: Higig+, 1: Higig2 */
                all_switch_drop:1,      /* All switched copies dropped */
                switch_pkt:1,           /* Switched packet */
                service_tag:1,          /* SD tag present */
                itag_action:2,          /* Ingress Inner tag action */
                otag_action:2,          /* Ingress Outer tag action */
                itag_status:2;          /* Ingress Inner tag status */

    union {                             /* T23.14 */
        struct {
            uint32  timestamp_hi;       /* Timestamp upper bits */
        } overlay1;
        struct {
            uint32  eh_tag_reserved:10, /* Reserved */
                    eh_queue_tag: 16,   /* HG2 EH queue tag */
                    eh_tm:1,            /* HG2 EH tm */
                    eh_tag_type:2,      /* HG2 EH tag type */
                    eh_seg_sel:3;       /* HG2 EH seg sel */
        } overlay2;
        struct {
            uint32  :11,                        /* Reserved */
                    rx_bfd_start_offset:8,      /* Rx BFD pkt start offset */
                    rx_bfd_start_offset_type:2, /* Rx BFD pkt st offset type */
                    rx_bfd_session_index:11;    /* Rx BFD pkt session ID */
        } overlay3;
    } word14;

                                        /* T23.15: DMA Status 0 */
        uint32  done:1,                 /* Descriptor Done */
                :12,                    /* Reserved */
                error:1,                /* Cell Error (RX) */
                start:1,                /* Start bit (RX) */
                end:1,                  /* End bit (RX) */
                count:16;               /* Transferred byte count */
#endif
} dcb23_t;

#define SOC_CPU_OPCODE_TYPE_IP_0        0
#define SOC_CPU_OPCODE_TYPE_IP_1        1
#define SOC_CPU_OPCODE_TYPE_EP          2
#define SOC_CPU_OPCODE_TYPE_NLF         3

#endif
