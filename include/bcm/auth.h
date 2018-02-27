/*
 * $Id: auth.h 1.23.6.2 Broadcom SDK $
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

#ifndef __BCM_AUTH_H__
#define __BCM_AUTH_H__

#include <bcm/types.h>

#define BCM_AUTH_MODE_UNCONTROLLED  0x00000001 
#define BCM_AUTH_MODE_UNAUTH        0x00000002 
#define BCM_AUTH_MODE_AUTH          0x00000004 

#define BCM_AUTH_BLOCK_IN       0x00000008 
#define BCM_AUTH_BLOCK_INOUT    0x00000010 

#define BCM_AUTH_LEARN              0x00000020 
#define BCM_AUTH_IGNORE_LINK        0x00000040 
#define BCM_AUTH_IGNORE_VIOLATION   0x00000080 

#define BCM_AUTH_REASON_UNKNOWN     0x00002000 
#define BCM_AUTH_REASON_LINK        0x00004000 
#define BCM_AUTH_REASON_VIOLATION   0x00008000 

#ifndef BCM_HIDE_DISPATCHABLE

/* Initialize bcm_auth API module. */
extern int bcm_auth_init(
    int unit);

/* De-configure bcm_auth API module. */
extern int bcm_auth_detach(
    int unit);

/* Sets or gets authorization operating mode. */
extern int bcm_auth_mode_set(
    int unit, 
    int port, 
    uint32 mode);

/* Sets or gets authorization operating mode. */
extern int bcm_auth_mode_get(
    int unit, 
    int port, 
    uint32 *modep);

#endif /* BCM_HIDE_DISPATCHABLE */

/* Definition for 802.1X callout functions. */
typedef void (*bcm_auth_cb_t)(
    void *cookie, 
    int unit, 
    int port, 
    int reason);

#ifndef BCM_HIDE_DISPATCHABLE

/* 
 * Register a callback function to be called when a port becomes
 * asynchronously unauthorized.
 */
extern int bcm_auth_unauth_callback(
    int unit, 
    bcm_auth_cb_t func, 
    void *cookie);

/* Enable/disable the ability of packets to be sent out a port. */
extern int bcm_auth_egress_set(
    int unit, 
    int port, 
    int enable);

/* Enable/disable the ability of packets to be sent out a port. */
extern int bcm_auth_egress_get(
    int unit, 
    int port, 
    int *enable);

/* Add Switch's MAC address. */
extern int bcm_auth_mac_add(
    int unit, 
    int port, 
    bcm_mac_t mac);

/* Remove Switch's MAC address. */
extern int bcm_auth_mac_delete(
    int unit, 
    int port, 
    bcm_mac_t mac);

/* Remove Switch's MAC address. */
extern int bcm_auth_mac_delete_all(
    int unit, 
    int port);

#endif /* BCM_HIDE_DISPATCHABLE */

/* Features that can be controlled for EAP packets. */
typedef enum bcm_auth_mac_control_e {
    bcmEapControlL2UserAddr,    /* enable L2 User Address frame bypass EAP Port
                                   State Filter and SA Filter. */
    bcmEapControlDHCP,          /* enable DHCP frame bypass EAP Port State
                                   Filter and SA Filter. */
    bcmEapControlARP,           /* enable ARP frame bypass EAP Port State Filter
                                   and SA Filter. */
    bcmEapControlMAC2X,         /* enable(DA=01-80-c2-00-00-22,23,....,2f) frame
                                   bypass EAP Port State Filter and SA Filter. */
    bcmEapControlGVRP,          /* enable(DA=01-80-c2-00-00-21) frame bypass EAP
                                   Port State FIlter and SA Filter. */
    bcmEapControlGMRP,          /* enable(DA=01-80-c2-00-00-20) frame bypass EAP
                                   Port State FIlter and SA Filter. */
    bcmEapControlMAC1X,         /* enable(DA=01-80-c2-00-00-11,12,....,1f) frame
                                   bypass EAP Port State FIlter and SA Filter. */
    bcmEapControlAllBridges,    /* enable(DA=01-80-c2-00-00-10) frame bypass EAP
                                   Port State FIlter and SA Filter. */
    bcmEapControlMAC0X,         /* enable(DA=01-80-c2-00-00-02)or
                                   (DA=01-80-c2-00-00-04,05,....,0f) frame
                                   bypass EAP Port State FIlter and SA Filter. */
    bcmEapControlMACBPDU        /* enable BPDU frame bypass EAP Port State
                                   FIlter and SA Filter. */
} bcm_auth_mac_control_t;

#ifndef BCM_HIDE_DISPATCHABLE

/* Set to control bypass for an EAP mac control type. */
extern int bcm_auth_mac_control_set(
    int unit, 
    bcm_auth_mac_control_t type, 
    uint32 value);

/* Get the EAP control status for an auth mac control type. */
extern int bcm_auth_mac_control_get(
    int unit, 
    bcm_auth_mac_control_t type, 
    uint32 *value);

#endif /* BCM_HIDE_DISPATCHABLE */

#endif /* __BCM_AUTH_H__ */
