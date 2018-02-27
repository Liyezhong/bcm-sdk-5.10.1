/*
 * $Id: trill.h 1.15.6.2 Broadcom SDK $
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

#ifndef __BCM_TRILL_H__
#define __BCM_TRILL_H__

#include <bcm/types.h>
#include <bcm/multicast.h>
#include <bcm/l3.h>

#if defined(INCLUDE_L3)
/* BCM_TRILL_* flags. */
#define BCM_TRILL_PORT_WITH_ID              (1 << 0)   /* create trill port with
                                                          specified ID */
#define BCM_TRILL_PORT_DROP                 (1 << 1)   /* Drop matching packets */
#define BCM_TRILL_PORT_COPYTOCPU            (1 << 2)   /* Copy matching packets
                                                          to CPU */
#define BCM_TRILL_PORT_NETWORK              (1 << 3)   /* Network facing
                                                          interface */
#define BCM_TRILL_PORT_COUNTED              (1 << 4)   /* Maintain packet/byte
                                                          counts */
#define BCM_TRILL_PORT_REPLACE              (1 << 5)   /* Replace existing entry */
#define BCM_TRILL_PORT_MULTICAST            (1 << 6)   /* Create Root RBridge */
#define BCM_TRILL_PORT_LOCAL                (1 << 7)   /* Create Local RBridge */
#define BCM_TRILL_MULTIPATH                 (1 << 8)   /* Multipath flag */
#define BCM_TRILL_MULTICAST_ACCESS_TO_NETWORK (1 << 9)   /* Multicast from Access
                                                          to Network */
#endif

#if defined(INCLUDE_L3)
/* TRILL port type. */
typedef struct bcm_trill_port_s {
    bcm_gport_t trill_port_id;  /* GPORT identifier. */
    uint32 flags;               /* BCM_TRILL_PORT_xxx. */
    int if_class;               /* Interface class ID. */
    bcm_gport_t port;           /* Match port and/or egress port. */
    bcm_if_t egress_if;         /* TRILL egress object. */
    bcm_trill_name_t name;      /* Destination RBridge Nickname. */
    int mtu;                    /* TRILL port MTU. */
    int hopcount;               /* Unicast Hopcount for TRILL. */
    int multicast_hopcount;     /* Multicast Hopcount for TRILL. */
    bcm_if_t encap_id;          /* Encap Identifier. */
} bcm_trill_port_t;
#endif

#if defined(INCLUDE_L3)
/* Initialize the TRILL port structure. */
extern void bcm_trill_port_t_init(
    bcm_trill_port_t *trill_port);
#endif

#ifndef BCM_HIDE_DISPATCHABLE

#if defined(INCLUDE_L3)
/* Initialize trill module. */
extern int bcm_trill_init(
    int unit);
#endif

#if defined(INCLUDE_L3)
/* Detach trill module. */
extern int bcm_trill_cleanup(
    int unit);
#endif

#if defined(INCLUDE_L3)
/* Add Trill port to TRILL Cloud. */
extern int bcm_trill_port_add(
    int unit, 
    bcm_trill_port_t *trill_port);
#endif

#if defined(INCLUDE_L3)
/* Delete Trill port from TRILL Cloud. */
extern int bcm_trill_port_delete(
    int unit, 
    bcm_gport_t trill_port_id);
#endif

#if defined(INCLUDE_L3)
/* Delete all Trill ports from TRILL Cloud. */
extern int bcm_trill_port_delete_all(
    int unit);
#endif

#if defined(INCLUDE_L3)
/* Get a TRILL port */
extern int bcm_trill_port_get(
    int unit, 
    bcm_trill_port_t *trill_port);
#endif

#if defined(INCLUDE_L3)
/* Get all TRILL ports */
extern int bcm_trill_port_get_all(
    int unit, 
    int port_max, 
    bcm_trill_port_t *port_array, 
    int *port_count);
#endif

#if defined(INCLUDE_L3)
/* Add TRILL multicast entry */
extern int bcm_trill_multicast_add(
    int unit, 
    uint32 flags, 
    bcm_trill_name_t root_name, 
    bcm_vlan_t vlan, 
    bcm_mac_t c_dmac, 
    bcm_multicast_t group);
#endif

#if defined(INCLUDE_L3)
/* Delete TRILL multicast entry */
extern int bcm_trill_multicast_delete(
    int unit, 
    uint32 flags, 
    bcm_trill_name_t root_name, 
    bcm_vlan_t vlan, 
    bcm_mac_t c_dmac, 
    bcm_multicast_t group);
#endif

#if defined(INCLUDE_L3)
/* Delete all TRILL multicast entries */
extern int bcm_trill_multicast_delete_all(
    int unit, 
    bcm_trill_name_t root_name);
#endif

#endif /* BCM_HIDE_DISPATCHABLE */

/* TRILL multicast traverse function prototype. */
#if defined(INCLUDE_L3)
typedef int (*bcm_trill_multicast_traverse_cb)(
    int unit, 
    bcm_trill_name_t root_name, 
    bcm_vlan_t vlan, 
    bcm_mac_t c_dmac, 
    bcm_multicast_t group, 
    void *user_data);
#endif

#ifndef BCM_HIDE_DISPATCHABLE

#if defined(INCLUDE_L3)
/* 
 * Traverse all valid TRILL Multicast entries and call the supplied
 * callback routine.
 */
extern int bcm_trill_multicast_traverse(
    int unit, 
    bcm_trill_multicast_traverse_cb cb, 
    void *user_data);
#endif

#if defined(INCLUDE_L3)
/* Add TRILL multicast RPF entry */
extern int bcm_trill_multicast_source_add(
    int unit, 
    bcm_trill_name_t root_name, 
    bcm_trill_name_t source_rbridge_name, 
    bcm_gport_t port);
#endif

#if defined(INCLUDE_L3)
/* Delete TRILL multicast RPF entry */
extern int bcm_trill_multicast_source_delete(
    int unit, 
    bcm_trill_name_t root_name, 
    bcm_trill_name_t source_rbridge_name, 
    bcm_gport_t port);
#endif

#if defined(INCLUDE_L3)
/* Get TRILL Multicast RPF entry */
extern int bcm_trill_multicast_source_get(
    int unit, 
    bcm_trill_name_t root_name, 
    bcm_trill_name_t source_rbridge_name, 
    bcm_gport_t *port);
#endif

#endif /* BCM_HIDE_DISPATCHABLE */

/* TRILL multicast RPF traverse function prototype. */
#if defined(INCLUDE_L3)
typedef int (*bcm_trill_multicast_source_traverse_cb)(
    int unit, 
    bcm_trill_name_t root_name, 
    bcm_trill_name_t source_rbridge_name, 
    bcm_gport_t *port, 
    void *user_data);
#endif

#ifndef BCM_HIDE_DISPATCHABLE

#if defined(INCLUDE_L3)
/* 
 * Traverse all valid TRILL Multicast RPF entries and call the supplied
 * callback routine.
 */
extern int bcm_trill_multicast_source_traverse(
    int unit, 
    bcm_trill_multicast_source_traverse_cb cb, 
    void *user_data);
#endif

#endif /* BCM_HIDE_DISPATCHABLE */

#if defined(INCLUDE_L3)
/* TRILL statistics counters. */
typedef enum bcm_trill_stat_e {
    bcmTrillInPkts, 
    bcmTrillOutPkts, 
    bcmTrillErrorPkts, 
    bcmTrillNameMissPkts, 
    bcmTrillRpfFailPkts, 
    bcmTrillTtlFailPkts 
} bcm_trill_stat_t;
#endif

#ifndef BCM_HIDE_DISPATCHABLE

#if defined(INCLUDE_L3)
/* bcm_trill_stat_get */
extern int bcm_trill_stat_get(
    int unit, 
    bcm_port_t port, 
    bcm_trill_stat_t stat, 
    uint64 *val);
#endif

#if defined(INCLUDE_L3)
/* bcm_trill_stat_get32 */
extern int bcm_trill_stat_get32(
    int unit, 
    bcm_port_t port, 
    bcm_trill_stat_t stat, 
    uint32 *val);
#endif

#if defined(INCLUDE_L3)
/* bcm_trill_stat_clear */
extern int bcm_trill_stat_clear(
    int unit, 
    bcm_port_t port, 
    bcm_trill_stat_t stat);
#endif

#endif /* BCM_HIDE_DISPATCHABLE */

#endif /* __BCM_TRILL_H__ */
