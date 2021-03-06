/*
 * $Id: trunk.h 1.51.6.3 Broadcom SDK $
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

#ifndef __BCM_TRUNK_H__
#define __BCM_TRUNK_H__

#include <bcm/types.h>

#define BCM_TRUNK_MAX_PORTCNT   16         /* Maximum number of ports in trunk
                                              in most chips, except BCM56840 */
#define BCM_TRUNK_UNSPEC_INDEX  -1         /* Let software set DLF/MC/IPMC. */

/* Port Selection Criteria. */
#define BCM_TRUNK_PSC_SRCMAC            1          /* Source MAC address. */
#define BCM_TRUNK_PSC_DSTMAC            2          /* Destination MAC address. */
#define BCM_TRUNK_PSC_SRCDSTMAC         3          /* Source+dest MAC address. */
#define BCM_TRUNK_PSC_SRCIP             4          /* Source IP address. */
#define BCM_TRUNK_PSC_DSTIP             5          /* Destination IP address. */
#define BCM_TRUNK_PSC_SRCDSTIP          6          /* Source+dest IP address. */
#define BCM_TRUNK_PSC_REDUNDANT         7          /* Redundant (xgs_fabric). */
#define BCM_TRUNK_PSC_PORTINDEX         8          /* Port index. */
#define BCM_TRUNK_PSC_PORTFLOW          9          /* Enhanced hashing. */
#define BCM_TRUNK_PSC_VLANINDEX         10         /* Static port selection
                                                      based on VLAN ID. */
#define BCM_TRUNK_PSC_DYNAMIC           11         /* Dynamic load balancing
                                                      normal mode. */
#define BCM_TRUNK_PSC_DYNAMIC_ASSIGNED  12         /* Dynamic load balancing
                                                      assigned mode. */
#define BCM_TRUNK_PSC_DYNAMIC_OPTIMAL   13         /* Dynamic load balancing
                                                      optimal mode. */

/* 
 * BCM5675 has additional fields for hashing in trunk mode.
 * BCM_TRUNK_PSC_EGRESS_VID and BCM_TRUNK_PSC_RANDOM are for software
 * IPMC trunk resolution.
 */
#define BCM_TRUNK_PSC_IPMACSA       0x00010    /* Include IP: MAC source
                                                  address. */
#define BCM_TRUNK_PSC_IPMACDA       0x00020    /* Include IP: MAC dest address. */
#define BCM_TRUNK_PSC_IPTYPE        0x00040    /* Include IP: Ethertype. */
#define BCM_TRUNK_PSC_IPVID         0x00080    /* Include IP: VLAN ID. */
#define BCM_TRUNK_PSC_IPSA          0x00100    /* Include IP: source address. */
#define BCM_TRUNK_PSC_IPDA          0x00200    /* Include IP: dest address. */
#define BCM_TRUNK_PSC_L4SS          0x00400    /* Include TCP/UDP source socket. */
#define BCM_TRUNK_PSC_L4DS          0x00800    /* Include TCP/UDP dest socket. */
#define BCM_TRUNK_PSC_MACSA         0x01000    /* Include Non-IP: MAC source
                                                  address. */
#define BCM_TRUNK_PSC_MACDA         0x02000    /* Include Non-IP: MAC dest
                                                  address. */
#define BCM_TRUNK_PSC_TYPE          0x04000    /* Include Non-IP:  Ethertype. */
#define BCM_TRUNK_PSC_VID           0x08000    /* Include Non-IP: VLAN ID. */
#define BCM_TRUNK_PSC_EGRESS_VID    0x10000    /* Include Egress VLAN ID. */
#define BCM_TRUNK_PSC_RANDOM        0x20000    /* Include random number. */

#define BCM_TRUNK_PSC_DEFAULT   BCM_TRUNK_PSC_SRCDSTMAC 

/* Backward compatibility. */
#define BCM_RTAG_SRCMAC         BCM_TRUNK_PSC_SRCMAC 
#define BCM_RTAG_DSTMAC         BCM_TRUNK_PSC_DSTMAC 
#define BCM_RTAG_SRCDSTMAC      BCM_TRUNK_PSC_SRCDSTMAC 
#define BCM_RTAG_SRCIP          BCM_TRUNK_PSC_SRCIP 
#define BCM_RTAG_DSTIP          BCM_TRUNK_PSC_DSTIP 
#define BCM_RTAG_SRCDSTIP       BCM_TRUNK_PSC_SRCDSTIP 
#define BCM_TRUNK_DEF_RTAG      BCM_TRUNK_PSC_DEFAULT 

#define BCM_TRUNK_PSC_NAMES_INITIALIZER \
{ \
    "unknown", \
    "srcmac", \
    "destmac", \
    "srcdestmac", \
    "srcip", \
    "destip", \
    "srcdestip", \
    "redundant", \
    "portindex", \
    "portflow", \
    "invalid" \
} 

/* Trunk group set flags. */
#define BCM_TRUNK_FLAG_FAILOVER_NEXT        0x0001     /* Failover port defaults
                                                          to the next port in
                                                          the trunk port list. */
#define BCM_TRUNK_FLAG_FAILOVER_NEXT_LOCAL  0x0002     /* Failover port defaults
                                                          to the next local port
                                                          in the trunk port
                                                          list, if any. */
#define BCM_TRUNK_FLAG_FAILOVER_ALL         0x0004     /* Failover ports default
                                                          to all other ports in
                                                          this trunk. */
#define BCM_TRUNK_FLAG_FAILOVER_ALL_LOCAL   0x0008     /* Failover ports default
                                                          to all other local
                                                          ports in this trunk. */
#define BCM_TRUNK_FLAG_FAILOVER             BCM_TRUNK_FLAG_FAILOVER_NEXT_LOCAL /* Enable trunk failover
                                                          support (deprecated). */
#define BCM_TRUNK_FLAG_MORE_MEMBERS         0x0010     /* There are more trunk
                                                          members than can be
                                                          fitted in
                                                          bcm_trunk_add_info_t. */
#define BCM_TRUNK_FLAG_IPMC_CLEAVE          0x0020     /* Disable trunk
                                                          resolution for IPMC
                                                          packets in hardware. */

/* Trunk member flags. */
#define BCM_TRUNK_MEMBER_INGRESS_DISABLE    0x0001     /* Member will not
                                                          receive traffic.
                                                          Receive traffic will
                                                          be treated as normal
                                                          port.
                                                          To drop all receive
                                                          traffic use:
                                                          bcm_port_discard_set, */
#define BCM_TRUNK_MEMBER_EGRESS_DISABLE     0x0002     /* Member will not be a
                                                          part of the
                                                          distributor members to
                                                          be hashed. */
#define BCM_TRUNK_MEMBER_EGRESS_DROP        0x0004     /* Member is part of the
                                                          distributor hash but
                                                          any traffic hashed to
                                                          this member will be
                                                          dropped. */
#define BCM_TRUNK_MEMBER_UNICAST_EGRESS_DISABLE 0x0008     /* Member will not be a
                                                          part of the unicast
                                                          distributor members to
                                                          be hashed. */

/* 
 * Trunk group port addition structure, describes all the ports to be
 * added to a trunk group. Note that the stack ports are carried along
 * for strata; both the real port number (for XGS) and the stack port
 * number (local unit for strata) are needed.
 */
typedef struct bcm_trunk_add_info_s {
    uint32 flags;                       /* BCM_TRUNK_FLAG_xxx. */
    int num_ports;                      /* Number of ports in the trunk group. */
    int psc;                            /* Port selection criteria. */
    int ipmc_psc;                       /* Port selection criteria for software
                                           IPMC trunk resolution. */
    int dlf_index;                      /* DLF/broadcast port for trunk group. */
    int mc_index;                       /* Multicast port for trunk group. */
    int ipmc_index;                     /* IPMC port for trunk group. */
    uint32 member_flags[BCM_TRUNK_MAX_PORTCNT]; /* BCM_TRUNK_MEMBER_xxx */
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT]; /* Ports in trunk. */
    bcm_module_t tm[BCM_TRUNK_MAX_PORTCNT]; /* Modules per port. */
    uint32 dynamic_size;                /* Number of flows for dynamic load
                                           balancing. Valid values are 512, 1k,
                                           doubling up to 32k */
    uint32 dynamic_age;                 /* Inactivity duration, in microseconds. */
} bcm_trunk_add_info_t;

/* Initialize a trunk chip information structure. */
typedef struct bcm_trunk_chip_info_s {
    int trunk_group_count;      /* Total number of (front panel) trunk groups. */
    int trunk_id_min;           /* Minimum (front panel) trunk ID number. */
    int trunk_id_max;           /* Maximum (front panel) trunk ID number. */
    int trunk_ports_max;        /* Maximum number of ports per (front panel)
                                   trunk group. */
    int trunk_fabric_id_min;    /* Minimum fabric trunk ID number. */
    int trunk_fabric_id_max;    /* Maximum fabric trunk ID number. */
    int trunk_fabric_ports_max; /* Maximum number of ports per fabric trunk
                                   group. */
} bcm_trunk_chip_info_t;

/* Structure describing a trunk member. */
typedef struct bcm_trunk_member_s {
    uint32 flags;       /* BCM_TRUNK_MEMBER_xxx */
    bcm_gport_t gport;  /* Trunk member GPORT ID. */
} bcm_trunk_member_t;

/* bcm_trunk_member_traverse_cb */
typedef int (*bcm_trunk_member_traverse_cb)(
    int unit, 
    bcm_trunk_member_t *member, 
    void *user_data);

/* bcm_trunk_add_info_t_init */
extern void bcm_trunk_add_info_t_init(
    bcm_trunk_add_info_t *trunk_add_info);

/* bcm_trunk_member_t_init */
extern void bcm_trunk_member_t_init(
    bcm_trunk_member_t *trunk_member);

#ifndef BCM_HIDE_DISPATCHABLE

/* Initialize the trunk module and SOC trunk hardware. */
extern int bcm_trunk_init(
    int unit);

/* Shut down (uninitialize) the trunk module. */
extern int bcm_trunk_detach(
    int unit);

/* 
 * Create the software data structure for a new trunk, using next
 * available trunk ID.
 */
extern int bcm_trunk_create(
    int unit, 
    bcm_trunk_t *tid);

/* 
 * Create the software data structure for a new trunk, using the
 * specified trunk ID.
 */
extern int bcm_trunk_create_id(
    int unit, 
    bcm_trunk_t tid);

#endif /* BCM_HIDE_DISPATCHABLE */

#define bcm_trunk_create_with_tid   bcm_trunk_create_id 

#ifndef BCM_HIDE_DISPATCHABLE

/* Get a trunk's Port Selection Criteria (PSC). */
extern int bcm_trunk_psc_get(
    int unit, 
    bcm_trunk_t tid, 
    int *psc);

/* Set a trunk's Port Selection Criteria (PSC). */
extern int bcm_trunk_psc_set(
    int unit, 
    bcm_trunk_t tid, 
    int psc);

/* 
 * Gets the underlying SOC device's trunk support limits, or bcmx system
 * trunk limits.
 */
extern int bcm_trunk_chip_info_get(
    int unit, 
    bcm_trunk_chip_info_t *ta_info);

/* Get the current parameters for the specified trunk group. */
extern int bcm_trunk_get(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_add_info_t *t_data);

/* Add ports to a trunk group. */
extern int bcm_trunk_set(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_add_info_t *add_info);

/* Removes a trunk group. */
extern int bcm_trunk_destroy(
    int unit, 
    bcm_trunk_t tid);

/* Expand a port bitmap to include all associated trunk member ports. */
extern int bcm_trunk_bitmap_expand(
    int unit, 
    bcm_pbmp_t *pbmp_ptr);

/* Add the specified trunk group to an existing MAC multicast entry. */
extern int bcm_trunk_mcast_join(
    int unit, 
    bcm_trunk_t tid, 
    bcm_vlan_t vid, 
    bcm_mac_t mac);

/* 
 * Retrieve the current bitmap of ports for which switching is enabled
 * for trunking.
 */
extern int bcm_trunk_egress_get(
    int unit, 
    bcm_trunk_t tid, 
    bcm_pbmp_t *pbmp);

/* Restrict trunk traffic only to specified trunk member ports. */
extern int bcm_trunk_egress_set(
    int unit, 
    bcm_trunk_t tid, 
    bcm_pbmp_t pbmp);

/* 
 * Retrieve the current state of trunk hashing override for unicast
 * packets.
 */
extern int bcm_trunk_override_ucast_get(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int modid, 
    int *enable);

/* Configure the override mode of trunk hashing for unicast packets. */
extern int bcm_trunk_override_ucast_set(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int modid, 
    int enable);

/* 
 * Retrieve the current state of trunk hashing override for multicast
 * packets.
 */
extern int bcm_trunk_override_mcast_get(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int idx, 
    int *enable);

/* Configure the overriding of trunk hashing for multicast packets. */
extern int bcm_trunk_override_mcast_set(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int idx, 
    int enable);

/* Retrieve the current state of trunk hashing override for IPMC packets. */
extern int bcm_trunk_override_ipmc_get(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int idx, 
    int *enable);

/* Configure the overriding of trunk hashing for IPMC packets. */
extern int bcm_trunk_override_ipmc_set(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int idx, 
    int enable);

/* 
 * Retrieve the current state of trunk hashing override for broadcast or
 * unknown unicast packets.
 */
extern int bcm_trunk_override_vlan_get(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    bcm_vlan_t vid, 
    int *enable);

/* 
 * Configure the overriding of trunk hashing for broadcast or unknown
 * unicast packets.
 */
extern int bcm_trunk_override_vlan_set(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    bcm_vlan_t vid, 
    int enable);

/* Retrieve the current weighted trunk hashing table. */
extern int bcm_trunk_pool_get(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int *size, 
    int weights[BCM_TRUNK_MAX_PORTCNT]);

/* Configure the weighted trunk hashing table. */
extern int bcm_trunk_pool_set(
    int unit, 
    bcm_port_t port, 
    bcm_trunk_t tid, 
    int size, 
    const int weights[BCM_TRUNK_MAX_PORTCNT]);

#endif /* BCM_HIDE_DISPATCHABLE */

/* Initialize a trunk chip information structure. */
extern void bcm_trunk_chip_info_t_init(
    bcm_trunk_chip_info_t *trunk_chip_info);

#ifndef BCM_HIDE_DISPATCHABLE

/* 
 * Get the trunk group ID for a given system port, specified by Module ID
 * and Port number.
 */
extern int bcm_trunk_find(
    int unit, 
    bcm_module_t modid, 
    bcm_port_t port, 
    bcm_trunk_t *tid);

/* Assign the failover port list for a specific trunk port. */
extern int bcm_trunk_failover_set(
    int unit, 
    bcm_trunk_t tid, 
    bcm_gport_t failport, 
    int psc, 
    uint32 flags, 
    int count, 
    bcm_gport_t *fail_to_array);

/* Retrieve the failover port list for a specific trunk port. */
extern int bcm_trunk_failover_get(
    int unit, 
    bcm_trunk_t tid, 
    bcm_gport_t failport, 
    int *psc, 
    uint32 *flags, 
    int array_size, 
    bcm_gport_t *fail_to_array, 
    int *array_count);

/* Add a member to a trunk group */
extern int bcm_trunk_member_add(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_member_t *member);

/* Delete a member from a trunk group */
extern int bcm_trunk_member_delete(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_member_t *member);

/* Delete all members from a trunk group */
extern int bcm_trunk_member_delete_all(
    int unit, 
    bcm_trunk_t tid);

/* Assign a set of members to a trunk group */
extern int bcm_trunk_member_set(
    int unit, 
    bcm_trunk_t tid, 
    int member_count, 
    bcm_trunk_member_t *member_array);

/* Get members of a trunk group */
extern int bcm_trunk_member_get(
    int unit, 
    bcm_trunk_t tid, 
    int member_max, 
    bcm_trunk_member_t *member_array, 
    int *member_count);

/* 
 * Traverse through members of a trunk group and run callback for each
 * member
 */
extern int bcm_trunk_member_traverse(
    int unit, 
    bcm_trunk_t tid, 
    bcm_trunk_member_traverse_cb cb, 
    void *user_data);

#endif /* BCM_HIDE_DISPATCHABLE */

#endif /* __BCM_TRUNK_H__ */
