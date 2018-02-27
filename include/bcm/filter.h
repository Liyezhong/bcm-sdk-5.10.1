/*
 * $Id: filter.h 1.28.6.2 Broadcom SDK $
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
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */

#ifndef __BCM_FILTER_H__
#define __BCM_FILTER_H__

#include <bcm/meter.h>

/* Backward compatibility. */
#define bcmActionSetPort        bcmActionSetPortUcast 

/* Filter Actions. */
typedef enum bcm_filter_action_e {
    bcmActionCancelAll,         /* Pseudo-action that clears all actions. */
    bcmActionDoNotSwitch,       /* Discard packet (still copy to cpu/mirr). */
    bcmActionDoSwitch,          /* Overrides a DoNotSwitch that is matched by a
                                   different rule. */
    bcmActionCopyToCpu,         /* Send copy of packet to CPU. */
    bcmActionCopyToMirror,      /* Send copy of packet to Mirror-To port. */
    bcmActionIncrCounter,       /* Increment an FFP counter. PARAMETER: counter
                                   number (cnum). */
    bcmActionSetPrio,           /* Set 802.1p priority for COSQ assignment
                                   without modifying packet. PARAMETER:
                                   priority. */
    bcmActionSetPortUcast,      /* Redirect unicast packet to specified port.
                                   PARAMETER: port number. */
    bcmActionSetPortNonUcast,   /* Redirect non-unicast packet (BCAST, MCAST,
                                   DLF) to specified port. PARAMETER: port
                                   number. */
    bcmActionSetPortAll,        /* Redirect any packet to specified port.
                                   PARAMETER: port number. */
    bcmActionSetPortUntag,      /* Specify whether packet redirected using above
                                   rules should go out untagged. PARAMETER:
                                   boolean. */
    bcmActionInsPrio,           /* Set 802.1p priority in both packet and COSQ
                                   assignment. PARAMETER: priority. */
    bcmActionInsPrioFromTOSP,   /* Set 802.1p priority from IP TOS. */
    bcmActionInsTOSP,           /* Set IPv4 TOS value in packet. PARAMETER: TOS. */
    bcmActionInsTOSPFromPrio,   /* Set IPv4 TOS value in packet from 802.1p
                                   priority. */
    bcmActionInsDiffServ,       /* Set IPv4 DiffServe value in packet.
                                   PARAMETER: DiffServ value. */
    bcmActionDropPrecedence,    /* Set packet to drop-precedence. */
    bcmActionSetClassificnTag,  /* Set HIGIG classification tag. PARAMETER:
                                   classification tag. */
    bcmActionInsVlanId,         /* Set VLAN ID in packet. PARAMETER: VLAN ID. */
    bcmActionEgressMask,        /* Specify port bitmap with which to mask AND)
                                   the normal destination bitmap. PARAMETER:
                                   Port bitmap. */
    bcmActionSetModule,         /* If redirected, send to specified module.
                                   PARAMETER: module number. */
    bcmActionSetECN,            /* Set ECN in ECN field. */
    bcmActionSetPortBitmap,     /* Set port bitmap in classification tag.
                                   PARAMETER: Port bitmap. */
    bcmActionInsDiffServIPv6,   /* Set IPv6 DiffServe value in packet.
                                   PARAMETER: DiffServ value. */
    bcmActionDoNotCopyToCpu,    /* Do not send copy of packet to CPU. Overrides
                                   bcmActionCopyToCpu. */
    bcmActionDoNotSetPortAny,   /* Do not redirect packet to specific port.
                                   Overrides all "SetPort" actions. */
    bcmActionSetDestMacAddrLo,  /* Set destination MAC addr low word. PARAMETER:
                                   MAC addr [31:0]. */
    bcmActionSetDestMacAddrHi,  /* Set destination MAC addr high word.
                                   PARAMETER: MAC addr [47:32]. */
    bcmActionSetPortBitmapHi,   /* Set port bitmap for hi module. PARAMETER:
                                   Port bitmap hi module. */
    bcmActionEgressMaskHi,      /* Specify high pbmp with which to mask AND) the
                                   normal destination bitmap. PARAMETER: Port
                                   bitmap hi module. */
    bcmActionSetVclabel,        /* Set VC label. PARAMETER: VC label. */
    bcmActionInvalid = -1 
} bcm_filter_action_t;

/* Packet formats. */
#define BCM_FILTER_PKTFMT_ETH_II    0x00000001 /* Ethernet II frame. */
#define BCM_FILTER_PKTFMT_802_3     0x00000002 /* IEEE 802.3 frame. */
#define BCM_FILTER_PKTFMT_SNAP      0x00000004 /* SNAP frame. */
#define BCM_FILTER_PKTFMT_LLC       0x00000008 /* LLC frame. */
#define BCM_FILTER_PKTFMT_INNER_TAG 0x00000010 /* Inner tag. */
#define BCM_FILTER_PKTFMT_OUTER_TAG 0x00000020 /* Outer tag. */
#define BCM_FILTER_PKTFMT_TAG       0x00000040 /* TAG */
#define BCM_FILTER_PKTFMT_UNTAG     0x00000080 /* Untagged. */
#define BCM_FILTER_PKTFMT_IPV4      0x00000100 /* Exclude IPv4. */
#define BCM_FILTER_PKTFMT_IPV6      0x00000200 /* Exclude IPv6. */
#define BCM_FILTER_PKTFMT_NONIP     0x00000400 /* Exclude IPv4/IPv6. */

/* Double tagged combines inner and outer tag. */
#define BCM_FILTER_PKTFMT_DOUBLE_TAG    (BCM_FILTER_PKTFMT_INNER_TAG | BCM_FILTER_PKTFMT_OUTER_TAG) 

/* Backward compatibility. */
typedef int bcm_filter_format_t;

/* Backward compatibility. */
#define bcmFormatUntaggedEthII  \
    (BCM_FILTER_PKTFMT_UNTAG | BCM_FILTER_PKTFMT_ETH_II) 

/* Backward compatibility. */
#define bcmFormatUntagged802_3  \
    (BCM_FILTER_PKTFMT_UNTAG | BCM_FILTER_PKTFMT_802_3) 

/* Backward compatibility. */
#define bcmFormatTaggedEthII    \
    (BCM_FILTER_PKTFMT_TAG | BCM_FILTER_PKTFMT_ETH_II) 

/* Backward compatibility. */
#define bcmFormatTagged802_3    \
    (BCM_FILTER_PKTFMT_TAG | BCM_FILTER_PKTFMT_802_3) 

/* Backward compatibility. */
#define bcmFormatInvalid        (-1)       

/* Filter type. */
typedef uint32 bcm_filterid_t;

/* Used by common filter definitions. */
extern const bcm_mac_t _bcm_filter_mac_all_ones;

/* Common filter definition: Destination MAC address. */
#define BCM_FILTER_QUALIFY_MAC_DST(unit, f, mac_addr)  \
    bcm_filter_qualify_data(unit, f, 0, 6, mac_addr, \
                            (uint8 *)_bcm_filter_mac_all_ones) 

/* Common filter definition: Source MAC address. */
#define BCM_FILTER_QUALIFY_MAC_SRC(unit, f, mac_addr)  \
    bcm_filter_qualify_data(unit, f, 6, 6, mac_addr, \
                            (uint8 *)_bcm_filter_mac_all_ones) 

/* Common filter definition: VLAN ID. */
#define BCM_FILTER_QUALIFY_VID(unit, f, vlan_id)  \
    bcm_filter_qualify_data16(unit, f, 14, vlan_id, 0x0fff) 

/* Common filter definition: IP source address. */
#define BCM_FILTER_QUALIFY_IP_SRC(unit, f, ip_addr)  \
    bcm_filter_qualify_data32(unit, f, 30, ip_addr, 0xffffffff) 

/* Common filter definition: IP destination address. */
#define BCM_FILTER_QUALIFY_IP_DST(unit, f, ip_addr)  \
    bcm_filter_qualify_data32(unit, f, 34, ip_addr, 0xffffffff) 

#ifndef BCM_HIDE_DISPATCHABLE

/* Initialize filter software subsystem. */
extern int bcm_filter_init(
    int unit);

/* Create a blank filter template. */
extern int bcm_filter_create(
    int unit, 
    bcm_filterid_t *f_return);

/* Create a blank filter template with requested filter ID. */
extern int bcm_filter_create_id(
    int unit, 
    bcm_filterid_t f);

/* Destroy a filter template. */
extern int bcm_filter_destroy(
    int unit, 
    bcm_filterid_t f);

/* Copy a filter template. */
extern int bcm_filter_copy(
    int unit, 
    bcm_filterid_t f_src, 
    bcm_filterid_t *f_return);

/* 
 * Copy a filter template to a new filter template with requested filter
 * ID.
 */
extern int bcm_filter_copy_id(
    int unit, 
    bcm_filterid_t f_src, 
    bcm_filterid_t f_dest);

#endif /* BCM_HIDE_DISPATCHABLE */

#define BCM_FILTER_QUALIFY_PRIO_MIN 0          

#define BCM_FILTER_QUALIFY_PRIO_MAX 255        

#ifndef BCM_HIDE_DISPATCHABLE

/* 
 * Set priority of this filter relative to other filters that match
 * simultaneously.
 */
extern int bcm_filter_qualify_priority(
    int unit, 
    bcm_filterid_t f, 
    int prio);

/* Set ingress port(s) that the packet must match to trigger filter. */
extern int bcm_filter_qualify_ingress(
    int unit, 
    bcm_filterid_t f, 
    bcm_pbmp_t pbmp);

/* Add egress port(s) that the packet must match to trigger filter. */
extern int bcm_filter_qualify_egress(
    int unit, 
    bcm_filterid_t f, 
    bcm_pbmp_t pbmp);

/* Set a filter to match only a particular egress module ID. */
extern int bcm_filter_qualify_egress_modid(
    int unit, 
    bcm_filterid_t f, 
    int module_id);

/* Set a filter to match only unknown unicast packets. */
extern int bcm_filter_qualify_unknown_ucast(
    int unit, 
    bcm_filterid_t f);

/* Set a filter to match only unknown multicast packets. */
extern int bcm_filter_qualify_unknown_mcast(
    int unit, 
    bcm_filterid_t f);

/* Set a filter to match only known unicast packets. */
extern int bcm_filter_qualify_known_ucast(
    int unit, 
    bcm_filterid_t f);

/* Set a filter to match only known multicast packets. */
extern int bcm_filter_qualify_known_mcast(
    int unit, 
    bcm_filterid_t f);

/* Set a filter to match only broadcast packets. */
extern int bcm_filter_qualify_broadcast(
    int unit, 
    bcm_filterid_t f);

/* Set a filter to stop lower priority rules check on a match. */
extern int bcm_filter_qualify_stop(
    int unit, 
    bcm_filterid_t f, 
    int partial_match);

/* Set a filter to match only a particular packet format. */
extern int bcm_filter_qualify_format(
    int unit, 
    bcm_filterid_t f, 
    bcm_filter_format_t format);

/* Add data field that the packet must match to trigger filter. */
extern int bcm_filter_qualify_data(
    int unit, 
    bcm_filterid_t f, 
    int offset, 
    int len, 
    const uint8 *data, 
    const uint8 *mask);

/* Add data field that the packet must match to trigger filter. */
extern int bcm_filter_qualify_data8(
    int unit, 
    bcm_filterid_t f, 
    int offset, 
    uint8 val, 
    uint8 mask);

/* Add data field that the packet must match to trigger filter. */
extern int bcm_filter_qualify_data16(
    int unit, 
    bcm_filterid_t f, 
    int offset, 
    uint16 val, 
    uint16 mask);

/* Add data field that the packet must match to trigger filter. */
extern int bcm_filter_qualify_data32(
    int unit, 
    bcm_filterid_t f, 
    int offset, 
    uint32 val, 
    uint32 mask);

/* Associate action to be performed when filter matches a packet. */
extern int bcm_filter_action_match(
    int unit, 
    bcm_filterid_t f, 
    bcm_filter_action_t action, 
    uint32 param);

/* 
 * Associate action to be performed with filter when filter rule is NOT
 * matched.
 */
extern int bcm_filter_action_no_match(
    int unit, 
    bcm_filterid_t f, 
    bcm_filter_action_t action, 
    uint32 param);

/* 
 * Associate action to be performed with filter when filter rule is
 * matched for an out-of-profile packet.
 */
extern int bcm_filter_action_out_profile(
    int unit, 
    bcm_filterid_t f, 
    bcm_filter_action_t action, 
    uint32 param, 
    int meter_id);

/* Install a filter into the hardware tables. */
extern int bcm_filter_install(
    int unit, 
    bcm_filterid_t f);

/* Re-Install a filter into the hardware tables. */
extern int bcm_filter_reinstall(
    int unit, 
    bcm_filterid_t f);

/* Remove a filter from the hardware tables. */
extern int bcm_filter_remove(
    int unit, 
    bcm_filterid_t f);

/* Quickly remove all filters from the hardware tables. */
extern int bcm_filter_remove_all(
    int unit);

#if defined(BROADCOM_DEBUG)
/* Show current S/W state if compiled in debug mode. */
extern int bcm_filter_show(
    int unit, 
    const char *pfx);
#endif

#if defined(BROADCOM_DEBUG)
/* Show contents of a filter template if compiled in debug mode. */
extern int bcm_filter_dump(
    int unit, 
    bcm_filterid_t f);
#endif

#endif /* BCM_HIDE_DISPATCHABLE */

#endif /* __BCM_FILTER_H__ */
