/*
 * $Id: l3.c 1.242.2.24 Broadcom SDK $
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
 * File:    l3.c
 * Purpose: Manages L3 interface table, forwarding table, routing table
 */

#include <soc/defs.h>
#include <bcm/error.h>

#ifdef INCLUDE_L3
#include <sal/core/libc.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/hash.h>


#include <bcm/l3.h>
#include <bcm/debug.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/vlan.h>

#ifdef  BCM_XGS_SWITCH_SUPPORT
#include <bcm_int/esw/draco.h>
#include <bcm_int/esw/firebolt.h>
#if defined(BCM_EASYRIDER_SUPPORT)
#include <bcm_int/esw/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
#include <bcm_int/esw/triumph.h>
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <bcm_int/esw/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#endif  /* BCM_XGS_SWITCH_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/flex_ctr.h>

#define L3_LOCK(_unit_)   _bcm_esw_l3_lock(_unit_)
#define L3_UNLOCK(_unit_) _bcm_esw_l3_unlock(_unit_)
        

/*
 * Special IP addresses
 */
#define INADDR_ANY        (uint32)0x00000000
#define INADDR_LOOPBACK   (uint32)0x7F000001
#define INADDR_NONE       (uint32)0xffffffff

/*
 * L3 book keeping global info
 */
_bcm_l3_bookkeeping_t   _bcm_l3_bk_info[BCM_MAX_NUM_UNITS];
static int              l3_internal_initialized = 0;

#define L3_INIT(unit) \
    if (!soc_feature(unit, soc_feature_l3)) { \
        return BCM_E_UNAVAIL; \
    } else if (!_bcm_l3_bk_info[unit].l3_initialized) { return BCM_E_INIT; }

int
_bcm_l3_reinit(int unit)
{
    _bcm_l3_bookkeeping_t *l3;
  
    l3 = &_bcm_l3_bk_info[unit];
    sal_memset(l3, 0, sizeof(_bcm_l3_bookkeeping_t));
  
#ifdef BCM_XGS3_SWITCH_SUPPORT
    if ((SOC_IS_FB_FX_HX(unit) && !SOC_IS_RAPTOR(unit)) ||
        SOC_IS_TR_VL(unit) || SOC_IS_HB_GW(unit) || 
        SOC_IS_SC_CQ(unit) || SOC_IS_EASYRIDER(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_reinit(unit));
    }
#endif

    l3->l3_initialized = 1;
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_l3_gport_construct
 * Purpose:
 *      Constructs a gport out of given parameter for L3 Module.
 * Parameters:
 *      unit - SOC device unit number
 *      port_tgid - Physical port 
 *      modid - Module ID
 *      tgid  - Trunk group ID
 *      flags  - L3 Flags
 *      gport  - (Out) gport constructed from given arguments
 * Returns:
 *      BCM_E_XXX              
 */

int
_bcm_esw_l3_gport_construct(int unit, bcm_port_t port, bcm_module_t modid,
                            bcm_trunk_t tgid, uint32 flags, bcm_port_t *gport) 
{
    int                 isGport, rv;
    _bcm_gport_dest_t   dest;
    bcm_module_t        mymodid;

    if (NULL == gport) {
        return BCM_E_PARAM;
    }

    if (flags & BCM_L3_TGID) {
        if (BCM_FAILURE(_bcm_trunk_id_validate(unit, tgid))) {
            return BCM_E_PARAM;
        }
    } else if ((port < 0) && (modid < 0)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));

    /* if output should not be gport then do nothing*/
    if (!isGport) {
        return BCM_E_NONE;
    }

    _bcm_gport_dest_t_init(&dest);

    if (flags & BCM_L3_TGID) {
        dest.tgid = tgid;
        dest.gport_type = _SHR_GPORT_TYPE_TRUNK;
    } else {
        /* Stacking ports should be encoded as devport */
        if (IS_ST_PORT(unit, port)) {
            rv = bcm_esw_stk_my_modid_get(unit, &mymodid);
            if (BCM_E_UNAVAIL == rv) {
                dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
            } else {
                dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
                dest.modid = modid;
            }
        } else {
            dest.modid = modid;
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
        }
        dest.port = port;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_construct(unit, &dest, gport));

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_esw_l3_gport_resolve
 * Purpose:
 *      Resolves a gport for L3 Module.
 * Parameters:
 *      unit - SOC device unit number
 *      gport - gport provided 
 *      port  - (Out) Physical port decoded from gport
 *      modid - (Out) Module ID decodec from gport 
 *      tgid  - (Out) Trunk group ID decoded from gport
 *      gport  - (Out) gport constructed from given arguments
 * Returns:
 *      BCM_E_XXX              
 */
int
_bcm_esw_l3_gport_resolve(int unit, bcm_gport_t gport, bcm_port_t *port, 
                          bcm_module_t *modid, bcm_trunk_t *tgid, uint32 *flags)
{
    int id;
    bcm_trunk_t     local_tgid;
    bcm_module_t    local_modid;
    bcm_port_t      local_port;

    if ((NULL == port) || (NULL == modid) || (NULL == tgid) || 
        (NULL == flags)) {
        return BCM_E_PARAM;
    }
   
    if (BCM_GPORT_IS_BLACK_HOLE(gport)) {
         *port = gport;
         return BCM_E_NONE;
    } else {
         BCM_IF_ERROR_RETURN(
              _bcm_esw_gport_resolve(unit, gport, &local_modid, &local_port, &local_tgid, &id));
    }

    if (-1 != id) {
        return BCM_E_PARAM;
    }
    if (BCM_TRUNK_INVALID != local_tgid) {
        *flags |= BCM_L3_TGID;
        *tgid = local_tgid;
    } else {
        *modid = local_modid;
        *port = local_port;
    }

    return BCM_E_NONE;
}

#if defined(BCM_XGS3_SWITCH_SUPPORT)
#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_2                SOC_SCACHE_VERSION(1,2)
#define BCM_WB_VERSION_1_1                SOC_SCACHE_VERSION(1,1)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_2

/*
 * Function:
 *      _bcm_esw_l3_sync
 * Purpose:
 *      Record L3 module persisitent info for Level 2 Warm Boot
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_esw_l3_sync(int unit)
{
    soc_scache_handle_t scache_handle;
    uint8 *l3_scache_ptr;
    int  alloc_sz = 2 * sizeof(int32); /* Egress, HostAdd mode */
    int32 l3EgressMode = 0;
    int32 l3HostAddMode = 0;
#ifdef BCM_TRIUMPH_SUPPORT
    int32 l3IntfMapMode = 0;
    int32 l3IngressMode = 0;
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
        alloc_sz  += sizeof(int32); /* Ingress mode */
        alloc_sz  += sizeof(int32); /* Interface Map mode */
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    L3_INIT(unit);

    SOC_SCACHE_HANDLE_SET(scache_handle,  unit, BCM_MODULE_L3, 0);
    BCM_IF_ERROR_RETURN
        (_bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                 alloc_sz, &l3_scache_ptr, 
                                 BCM_WB_DEFAULT_VERSION, NULL));

    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_egress_mode_get(unit, &l3EgressMode));
    sal_memcpy(l3_scache_ptr, &l3EgressMode,  sizeof(l3EgressMode));
    l3_scache_ptr += sizeof(l3EgressMode);

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
        BCM_IF_ERROR_RETURN(bcm_xgs3_l3_ingress_mode_get(unit, &l3IngressMode));
        sal_memcpy(l3_scache_ptr, &l3IngressMode,  sizeof(l3IngressMode));
        l3_scache_ptr += sizeof(l3IngressMode);
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_host_as_route_return_get(unit, &l3HostAddMode));
    sal_memcpy(l3_scache_ptr, &l3HostAddMode,  sizeof(l3HostAddMode));
    l3_scache_ptr += sizeof(l3HostAddMode);

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
       BCM_IF_ERROR_RETURN(bcm_xgs3_l3_ingress_intf_map_get(unit, &l3IntfMapMode));
       sal_memcpy(l3_scache_ptr, &l3IntfMapMode,  sizeof(l3IntfMapMode));
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_l3_warm_boot_alloc
 * Purpose:
 *      Allocate persistent info memory for L3 module - Level 2 Warm Boot
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_l3_warm_boot_alloc(int unit)
{
    int rv;
    int  alloc_sz = 2 * sizeof(int32); /* Egress, Host modes */
    uint8 *l3_scache_ptr;
    soc_scache_handle_t scache_handle;

    if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_1) {
#ifdef BCM_TRIUMPH_SUPPORT 
        if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
            alloc_sz += sizeof(int32); /* Ingress mode */
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }
    
    if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_2) {
#ifdef BCM_TRIUMPH_SUPPORT 
        if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
            alloc_sz += sizeof(int32); /* intfMap mode */
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_L3, 0);

    /* This function is only used for Cold Boot */
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, TRUE,
                                 alloc_sz, &l3_scache_ptr, 
                                 BCM_WB_DEFAULT_VERSION, NULL);

    if (BCM_E_NOT_FOUND == rv) {
        /* Proceed with Level 1 Warm Boot */
        rv = BCM_E_NONE;
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
#endif /* defined(BCM_XGS3_SWITCH_SUPPORT) */

/*
 * Function:
 *      bcm_l3_init
 * Purpose:
 *      Initialize the L3 table, default IP table and the L3 interface table.
 * Parameters:
 *      unit - SOC device unit number
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_MEMORY            Cannot allocate memory
 *      BCM_E_INTERNAL          Chip access failure
 * Notes:
 *      This function has to be called before any other L3 functions.
 */

int
bcm_esw_l3_init(int unit)
{
    _bcm_l3_bookkeeping_t *l3;

    if (!soc_feature(unit, soc_feature_l3)) {
        return BCM_E_UNAVAIL;
    }

    if (!l3_internal_initialized) {
        sal_memset(_bcm_l3_bk_info, 0,
                   BCM_MAX_NUM_UNITS * sizeof(_bcm_l3_bookkeeping_t));
        l3_internal_initialized = 1;
    }

    l3 = &_bcm_l3_bk_info[unit];

    /* Create protection mutex. */
    if (NULL == l3->lock) {
        l3->lock = sal_mutex_create("L3 module mutex");
    }

#if defined(BCM_WARM_BOOT_SUPPORT) && defined(BCM_XGS3_SWITCH_SUPPORT)
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_warm_boot_alloc(unit));
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        return (_bcm_l3_reinit(unit));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

   BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_l3_tables_init(unit));
   
   l3->l3_initialized = 1;

   return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_esw_l3_intf_create
 * Purpose:
 *     Create an L3 interface
 * Parameters:
 *     unit  - StrataXGS unit number
 *     intf  - interface info: l3a_mac_addr - MAC address;
 *             l3a_vid - VLAN ID;
 *             flag BCM_L3_ADD_TO_ARL: add mac address to arl table
 *                                     with static bit and L3 bit set.
 *             flag BCM_L3_WITH_ID: use specified interface ID l3a_intf.
 *             flag BCM_L3_REPLACE: overwrite if interface already exists.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The VLAN table should be set up properly before this call.
 *      The L3 interface ID is automatically assigned unless a specific
 *      ID is requested with the BCM_L3_WITH_ID flag.
 */

int
_bcm_esw_l3_intf_create(int unit, bcm_l3_intf_t *intf)
{
    _bcm_l3_intf_cfg_t l3i;
    bcm_l3_intf_t find_if;
    int rv;

    /* Input parameters check. */
    if (NULL == intf) {
        return (BCM_E_PARAM);
    }

    if ((intf->l3a_vrf > SOC_VRF_MAX(unit)) ||
        (intf->l3a_vrf < BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM);
    }

    if (BCM_MAC_IS_ZERO(intf->l3a_mac_addr)) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(intf->l3a_vid)) {
        return (BCM_E_PARAM);
    }

    if (!BCM_TTL_VALID(intf->l3a_ttl)) {
        return (BCM_E_PARAM);
    }

    if (intf->l3a_group && (0 == SOC_IS_EASYRIDER(unit))) {
        return (BCM_E_PARAM);
    }

    if ((intf->l3a_group > SOC_INTF_CLASS_MAX(unit)) ||
        (intf->l3a_group < 0)) {
        return (BCM_E_PARAM);
    }

    if ((0 == SOC_IS_TRX(unit)) && (intf->l3a_inner_vlan)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    sal_memcpy(l3i.l3i_mac_addr, intf->l3a_mac_addr, sizeof(bcm_mac_t));
    l3i.l3i_vid = intf->l3a_vid;
    l3i.l3i_inner_vlan = intf->l3a_inner_vlan;
    l3i.l3i_flags = intf->l3a_flags;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    l3i.l3i_vrf = intf->l3a_vrf;
    l3i.l3i_group = intf->l3a_group;
    l3i.l3i_ttl = intf->l3a_ttl;
    l3i.l3i_mtu = intf->l3a_mtu;
    l3i.l3i_tunnel_idx = intf->l3a_tunnel_idx;
    sal_memcpy(&l3i.vlan_qos, &intf->vlan_qos, sizeof(bcm_l3_intf_qos_t));
    sal_memcpy(&l3i.inner_vlan_qos, &intf->inner_vlan_qos, sizeof(bcm_l3_intf_qos_t));
    sal_memcpy(&l3i.dscp_qos, &intf->dscp_qos, sizeof(bcm_l3_intf_qos_t));
#endif

    /* L3 interface ID given */
    if (intf->l3a_flags & BCM_L3_WITH_ID) {
        /* Initilize interface lookup key. */
        bcm_l3_intf_t_init(&find_if);

        /* Set interface index. */
        find_if.l3a_intf_id = intf->l3a_intf_id;

        /* Read interface info by index. */
        rv = bcm_esw_l3_intf_get(unit, &find_if);

        if (rv == BCM_E_NONE) {
            /* BCM_L3_REPLACE flag must be set for interface update. */
            if (!(intf->l3a_flags & BCM_L3_REPLACE)) {
               return BCM_E_EXISTS;
            }

            /* Remove old interface layer 2 address if required. */
            if (BCM_L3_INTF_ARL_GET(unit, intf->l3a_intf_id)) {
                BCM_IF_ERROR_RETURN 
                    (bcm_esw_l2_addr_delete(unit, find_if.l3a_mac_addr,
                                         find_if.l3a_vid));
                BCM_L3_INTF_ARL_CLR(unit, find_if.l3a_intf_id);
            }
        } else if (rv != BCM_E_NOT_FOUND) { /* Other error */
            return rv;
        }
        /* Set interface index. */
        l3i.l3i_index = intf->l3a_intf_id;
        rv = mbcm_driver[unit]->mbcm_l3_intf_id_create(unit, &l3i);
    } else {
        rv = mbcm_driver[unit]->mbcm_l3_intf_create(unit, &l3i);
    }

    if (rv < 0) {
        return (rv);
    }

    if (!(intf->l3a_flags & BCM_L3_WITH_ID)) {
        intf->l3a_intf_id = l3i.l3i_index;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_esw_l3_intf_create
 * Purpose:
 *     Create an L3 interface
 * Parameters:
 *     unit  - StrataXGS unit number
 *     intf  - interface info: l3a_mac_addr - MAC address;
 *             l3a_vid - VLAN ID;
 *             flag BCM_L3_ADD_TO_ARL: add mac address to arl table
 *                                     with static bit and L3 bit set.
 *             flag BCM_L3_WITH_ID: use specified interface ID l3a_intf.
 *             flag BCM_L3_REPLACE: overwrite if interface already exists.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The VLAN table should be set up properly before this call.
 *      The L3 interface ID is automatically assigned unless a specific
 *      ID is requested with the BCM_L3_WITH_ID flag.
 */
int
bcm_esw_l3_intf_create(int unit, bcm_l3_intf_t *intf)
{
    int rv;

    L3_INIT(unit);
    L3_LOCK(unit);

    rv = _bcm_esw_l3_intf_create(unit, intf);

    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *     bcm_esw_l3_intf_delete
 * Purpose:
 *     Delete L3 interface based on L3 interface ID
 * Parameters:
 *     unit  - StrataXGS unit number
 *     intf  - interface structure with L3 interface ID as input
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_l3_intf_delete(int unit, bcm_l3_intf_t *intf)
{
    _bcm_l3_intf_cfg_t l3i;
    int rv;

    L3_INIT(unit);

    if (NULL == intf) {
        return (BCM_E_PARAM);
    } 

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    l3i.l3i_index = intf->l3a_intf_id;

    L3_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_l3_intf_delete(unit, &l3i);
    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *     bcm_esw_l3_intf_find
 * Purpose:
 *     Find the L3 intf number based on (MAC, VLAN)
 * Parameters:
 *     unit  - StrataXGS unit number
 *     intf  - (IN) interface (MAC, VLAN), (OUT)intf number
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_l3_intf_find(int unit, bcm_l3_intf_t *intf)
{
    _bcm_l3_intf_cfg_t l3i;
    int rv;

    L3_INIT(unit);

    if (NULL == intf) {
        return (BCM_E_PARAM);
    } 

    if ((BCM_MAC_IS_MCAST(intf->l3a_mac_addr)) || 
        (BCM_MAC_IS_ZERO(intf->l3a_mac_addr))) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(intf->l3a_vid)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    sal_memcpy(l3i.l3i_mac_addr, intf->l3a_mac_addr, sizeof(bcm_mac_t));
    l3i.l3i_vid = intf->l3a_vid;

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_intf_lookup(unit, &l3i);

    L3_UNLOCK(unit);

    intf->l3a_intf_id = l3i.l3i_index;

    return rv;
}

/*
 * Function:
 *     bcm_esw_l3_intf_get
 * Purpose:
 *     Given the L3 interface number, return the MAC and VLAN
 * Parameters:
 *     unit  - StrataXGS unit number
 *     intf  - (IN) L3 interface; (OUT)VLAN ID, 802.3 MAC for this L3 intf
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_l3_intf_get(int unit, bcm_l3_intf_t *intf)
{
    _bcm_l3_intf_cfg_t l3i;
    int rv;

    L3_INIT(unit);

    if (NULL == intf) {
        return (BCM_E_PARAM);
    } 

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    l3i.l3i_index = intf->l3a_intf_id;

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_intf_get(unit, &l3i);

    L3_UNLOCK(unit);
    if (rv < 0) {
        return (rv);
    }

    sal_memcpy(intf->l3a_mac_addr, l3i.l3i_mac_addr, sizeof(bcm_mac_t));
    intf->l3a_vid = l3i.l3i_vid;
    intf->l3a_inner_vlan = l3i.l3i_inner_vlan;
    intf->l3a_tunnel_idx = l3i.l3i_tunnel_idx;
    intf->l3a_flags = 0;
    intf->l3a_vrf = l3i.l3i_vrf;
    intf->l3a_ttl = l3i.l3i_ttl;
    intf->l3a_mtu = l3i.l3i_mtu;
    intf->l3a_group = l3i.l3i_group;
    sal_memcpy(&intf->vlan_qos, &l3i.vlan_qos, sizeof(bcm_l3_intf_qos_t));
    sal_memcpy(&intf->inner_vlan_qos, &l3i.inner_vlan_qos, sizeof(bcm_l3_intf_qos_t));
    sal_memcpy(&intf->dscp_qos, &l3i.dscp_qos, sizeof(bcm_l3_intf_qos_t));


    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_esw_l3_intf_find_vlan
 * Purpose:
 *     Find the L3 interface by VLAN
 * Parameters:
 *     unit - StrataXGS unit number
 *     intf  - (IN) VID, (OUT)L3 intf info
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_l3_intf_find_vlan(int unit, bcm_l3_intf_t *intf)
{
    int rv;
    _bcm_l3_intf_cfg_t l3i;

    L3_INIT(unit);

    if (NULL == intf) {
        return (BCM_E_PARAM);
    } 

    if (!BCM_VLAN_VALID(intf->l3a_vid)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    l3i.l3i_vid = intf->l3a_vid;

    L3_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_l3_intf_get_by_vid(unit, &l3i);

    L3_UNLOCK(unit);
    if (rv < 0) {
        return (rv);
    }

    intf->l3a_intf_id = l3i.l3i_index;
    sal_memcpy(intf->l3a_mac_addr, l3i.l3i_mac_addr, sizeof(bcm_mac_t));
    intf->l3a_tunnel_idx = l3i.l3i_tunnel_idx;
    intf->l3a_flags = l3i.l3i_flags;
    intf->l3a_vrf = l3i.l3i_vrf;
    intf->l3a_ttl = l3i.l3i_ttl;
    intf->l3a_mtu = l3i.l3i_mtu;
    intf->l3a_group = l3i.l3i_group;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_l3_intf_delete_all
 * Purpose:
 *      Delete all L3 interfaces
 * Parameters:
 *      unit - SOC device unit number
 * Returns:
 *      BCM_E_XXXX
 */
int
bcm_esw_l3_intf_delete_all(int unit)
{
    int rv;
    L3_INIT(unit);
    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_intf_delete_all(unit);

    L3_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_interface_create (to be deprecated)
 * Purpose:
 *      Create an L3 interface
 * Parameters:
 *      unit - SOC device unit number
 *      mac_addr - 802.3 MAC address.
 *      vid - VLAN identifier (VID).
 *      add_to_arl - Add the mac address to arl table with static bit and L3
 *                   bit set.
 * Returns:
 *      interface (>= 0)        Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_BADID           The vid is not in the vtable.
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_FULL      L3 interface table is full
 *      BCM_E_INIT              Unit Not initialized yet
 * Notes:
 *      Vtable should be setup properly before this call.
 */

int
bcm_esw_l3_interface_create(int unit, bcm_mac_t mac_addr, bcm_vlan_t vid,
                        int add_to_arl)
{
    bcm_l3_intf_t intf;
    int rv;

    L3_INIT(unit);

    if ((BCM_MAC_IS_MCAST(mac_addr)) || 
        (BCM_MAC_IS_ZERO(mac_addr))) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(vid)) {
        return (BCM_E_PARAM);
    }

    bcm_l3_intf_t_init(&intf);

    sal_memcpy(intf.l3a_mac_addr, mac_addr, sizeof(bcm_mac_t));
    intf.l3a_vid = vid;
    if (add_to_arl) {
        intf.l3a_flags |= BCM_L3_ADD_TO_ARL;
    }
    L3_LOCK(unit);
    rv = bcm_esw_l3_intf_create(unit, &intf);
    L3_UNLOCK(unit);
    if (rv < 0) {
        return (rv);
    }

    return intf.l3a_intf_id;
}

/*
 * Function:
 *      bcm_esw_l3_interface_id_create (to be deprecated)
 * Purpose:
 *      Create an interface using the indicated L3 interface ID
 * Parameters:
 *      unit       - SOC device unit number
 *      mac_addr   - 802.3 MAC address.
 *      vid        - VLAN identifier (VID).
 *      intf_idx   - L3 interface ID
 *      add_to_arl - Add the mac address to arl table with static bit and L3
 *                   bit set.
 * Returns:
 *      BCM_E_XXXX
 * Notes:
 *      If the ID exists with conflicting data, returns E_EXISTS.
 *      Parameter add_to_arl indicates the L2 table should be updated
 *      with the MAC address & VLAN
 */
int
bcm_esw_l3_interface_id_create(int unit,
                           bcm_mac_t mac_addr,
                           bcm_vlan_t vid,
                           int intf_idx,
                           int add_to_arl)
{
    bcm_l3_intf_t intf;
    int rv;

    L3_INIT(unit);

    if ((BCM_MAC_IS_MCAST(mac_addr)) || 
        (BCM_MAC_IS_ZERO(mac_addr))) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(vid)) {
        return (BCM_E_PARAM);
    }

    bcm_l3_intf_t_init(&intf);

    intf.l3a_intf_id = intf_idx;
    if (add_to_arl) {
        intf.l3a_flags |= BCM_L3_ADD_TO_ARL;
    }
    intf.l3a_vid     = vid;
    intf.l3a_flags   |= BCM_L3_WITH_ID;
    sal_memcpy(intf.l3a_mac_addr, mac_addr, sizeof(bcm_mac_t));

    L3_LOCK(unit);

    rv = bcm_esw_l3_intf_create(unit, &intf);

    L3_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_interface_id_update (to be deprecated)
 * Purpose:
 *      Create an interface using the indicated L3 interface ID,
 *      overwrite existing L3 interface if it exists already
 * Parameters:
 *      unit       - SOC device unit number
 *      mac_addr   - 802.3 MAC address.
 *      vid        - VLAN identifier (VID).
 *      intf_idx   - L3 interface ID
 *      add_to_arl - Add the mac address to arl table with static bit and L3
 *                   bit set.
 * Returns:
 *      BCM_E_XXXX
 * Notes:
 *      This function is the same as bcm_esw_l3_interface_id_create()
 *      except that it would over-write existing L3 intf entry.
 */
int
bcm_esw_l3_interface_id_update(int unit,
                           bcm_mac_t mac_addr,
                           bcm_vlan_t vid,
                           int intf_idx,
                           int add_to_arl)
{
    bcm_l3_intf_t intf;
    int rv;

    L3_INIT(unit);

    if ((BCM_MAC_IS_MCAST(mac_addr)) || 
        (BCM_MAC_IS_ZERO(mac_addr))) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(vid)) {
        return (BCM_E_PARAM);
    }

    bcm_l3_intf_t_init(&intf);

    sal_memcpy(intf.l3a_mac_addr, mac_addr, sizeof(bcm_mac_t));
    intf.l3a_vid     = vid;
    intf.l3a_intf_id = intf_idx;
    intf.l3a_flags |= BCM_L3_WITH_ID | BCM_L3_REPLACE;
    if (add_to_arl) {
        intf.l3a_flags |= BCM_L3_ADD_TO_ARL;
    }
    L3_LOCK(unit);

    rv = bcm_esw_l3_intf_create(unit, &intf);

    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_interface_lookup (to be deprecated)
 * Purpose:
 *      Search for L3 interface based on VID and MAC address
 * Parameters:
 *      unit       - SOC device unit number
 *      mac_addr   - 802.3 MAC address.
 *      vid        - VLAN identifier (VID).
 *      intf_id    - L3 interface ID
 * Returns:
 *      BCM_E_XXXX
 */
int
bcm_esw_l3_interface_lookup(int unit, bcm_mac_t mac_addr, bcm_vlan_t vid,
                        int *intf_id)
{
    int rv;
    _bcm_l3_intf_cfg_t l3i;

    L3_INIT(unit);

    if ((BCM_MAC_IS_MCAST(mac_addr)) || 
        (BCM_MAC_IS_ZERO(mac_addr))) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(vid)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    sal_memcpy(l3i.l3i_mac_addr, mac_addr, sizeof(bcm_mac_t));
    l3i.l3i_vid = vid;

    L3_LOCK(unit); 

    rv = mbcm_driver[unit]->mbcm_l3_intf_lookup(unit, &l3i);

    L3_UNLOCK(unit);

    if (BCM_SUCCESS(rv)) {
        *intf_id = l3i.l3i_index;
    }

    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_interface_destroy (to be deprecated)
 * Purpose:
 *      Destroy an entry in L3 interface table.
 * Parameters:
 *      unit    - SOC device unit number
 *      intf_id - the L3 interface number.
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_INIT              Unit Not initialized yet
 *      BCM_E_EMPTY     L3 interface table is empty
 */

int
bcm_esw_l3_interface_destroy(int unit, int intf_id)
{
    int rv;
    _bcm_l3_intf_cfg_t l3i;

    L3_INIT(unit);

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    l3i.l3i_index = intf_id;

    L3_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_l3_intf_delete(unit, &l3i);
    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_l3_interface_find (to be deprecated)
 * Purpose:
 *      Find an entry in L3 intf table based on interface number.
 * Parameters:
 *      unit     - SOC device unit number
 *      intf_id  - The interface number.
 *      vid      - VLAN identifier (VID).
 *      mac_addr - (OUT) 802.3 MAC address.
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INVALID_INDEX     Illegal index
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_INIT              Unit Not initialized yet
 */

int
bcm_esw_l3_interface_find(int unit, int intf_id, bcm_vlan_t *vid,
                      bcm_mac_t mac_addr)
{
    _bcm_l3_intf_cfg_t l3i;
    int rv;

    L3_INIT(unit);

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    l3i.l3i_index = intf_id;
    L3_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_l3_intf_get(unit, &l3i);
    L3_UNLOCK(unit);
    if (rv < 0) {
        return (rv);
    }

    if (mac_addr) {
        sal_memcpy(mac_addr, l3i.l3i_mac_addr, sizeof(bcm_mac_t));
    }

    if (vid) {
        *vid = l3i.l3i_vid;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_l3_interface_find_by_vlan (to be deprecated)
 * Purpose:
 *      Find the interface number in L3 intf table based on vlan ID.
 * Parameters:
 *      unit    - SOC device unit number
 *      vid     - VLAN identifier (VID).
 *      intf_id - (OUT) The interface number.
 * Returns:
 *      BCM_E_NONE         Success
 *      BCM_E_UNIT         Illegal unit number
 *      BCM_E_INIT         Unit Not initialized yet
 *      BCM_E_NOT_FOUND    The vlan is not found from L3 intf table
 */

int
bcm_esw_l3_interface_find_by_vlan(int unit, bcm_vlan_t vid, int *intf_id)
{
    int rv;
    _bcm_l3_intf_cfg_t l3i;

    L3_INIT(unit);

    if (intf_id == NULL) {
        return (BCM_E_PARAM);
    }

    if (!BCM_VLAN_VALID(vid)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&l3i, 0, sizeof(_bcm_l3_intf_cfg_t));
    l3i.l3i_vid = vid;
    L3_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_l3_intf_get_by_vid(unit, &l3i);
    L3_UNLOCK(unit);
    if (rv < 0) {
        return (rv);
    }

    *intf_id = l3i.l3i_index;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_l3_host_find
 * Purpose:
 *      Find an entry from the L3 host table given IP address.
 * Parameters:
 *      unit - SOC device unit number
 *      info - (Out) Pointer to memory for bcm_l3_host_t.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1) The hit returned may be either L3SH or L3DH or Both
 *      2) Flag set to BCM_L3_IP6 for IPv6, default is IPv4
 */

int
bcm_esw_l3_host_find(int unit, bcm_l3_host_t *info)
{
    _bcm_l3_cfg_t l3cfg;
    int           rt;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    /* Vrf is in device supported range check. */
    if ((info->l3a_vrf > SOC_VRF_MAX(unit)) || 
        (info->l3a_vrf < BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM);
    }
  
    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, info->l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_flags = info->l3a_flags;
    l3cfg.l3c_vrf = info->l3a_vrf;

    L3_LOCK(unit);
    if (info->l3a_flags & BCM_L3_IP6) {
        sal_memcpy(l3cfg.l3c_ip6, info->l3a_ip6_addr, BCM_IP6_ADDRLEN);
        rt = mbcm_driver[unit]->mbcm_l3_ip6_get(unit, &l3cfg);
    } else {
        l3cfg.l3c_ip_addr = info->l3a_ip_addr;
        rt = mbcm_driver[unit]->mbcm_l3_ip4_get(unit, &l3cfg);
    }
    L3_UNLOCK(unit);

    if (rt < 0) {
        return (rt);
    }

    info->l3a_flags = l3cfg.l3c_flags;
    if (l3cfg.l3c_flags & BCM_L3_IPMC) {
        info->l3a_ipmc_ptr = l3cfg.l3c_ipmc_ptr;
    }
    sal_memcpy(info->l3a_nexthop_mac, l3cfg.l3c_mac_addr, sizeof(bcm_mac_t));
    info->l3a_intf = l3cfg.l3c_intf;
    info->l3a_port_tgid = l3cfg.l3c_port_tgid;
    info->l3a_modid = l3cfg.l3c_modid;
    info->l3a_lookup_class = l3cfg.l3c_lookup_class;

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_wlan) && 
        BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, l3cfg.l3c_intf)) {
        /* Get DVP using the egress object ID and construct a gport */
        egr_l3_next_hop_entry_t next_hop;
        _bcm_gport_dest_t dest;
        int nh_idx = l3cfg.l3c_intf - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        BCM_IF_ERROR_RETURN
            (READ_EGR_L3_NEXT_HOPm(unit, MEM_BLOCK_ANY, nh_idx, &next_hop));
        dest.wlan_id = soc_EGR_L3_NEXT_HOPm_field32_get(unit, &next_hop, DVPf);
        dest.gport_type = _SHR_GPORT_TYPE_WLAN_PORT;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_construct(unit, &dest, &(info->l3a_port_tgid)));
    } else
#endif
    {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_l3_gport_construct(unit, info->l3a_port_tgid, 
                                         info->l3a_modid, info->l3a_port_tgid, 
                                         info->l3a_flags, 
                                         &(info->l3a_port_tgid)));
    }
    return BCM_E_NONE;
}

/* to be deprecated */
int
bcm_esw_l3_ip_find(int unit, bcm_l3_host_t *info)
{
    return (bcm_esw_l3_host_find(unit, info));
}

/*
 * Function:
 *      bcm_esw_l3_ip_find_index
 * Purpose:
 *      Find an entry from the L3 table based on index.
 * Parameters:
 *      unit - SOC device unit number
 *      index - The index number in L3 table
 *      info - (Out) Pointer to memory for bcm_l3_host_t.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *    1) The hit returned may be either L3SH or L3DH or Both
 *    2) This function will be deprecated soon
 */

/* to be deprecated */

int
bcm_esw_l3_ip_find_index(int unit, int index, bcm_l3_host_t *info)
{
    _bcm_l3_cfg_t l3cfg;
    int rt = BCM_E_UNAVAIL;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_hw_index = index;
    l3cfg.l3c_flags = info->l3a_flags;

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    L3_LOCK(unit);
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    if (SOC_IS_XGS12_SWITCH(unit)) {
        rt = bcm_xgs_l3_get_by_index(unit, index, &l3cfg);
    }
#endif /* BCM_XGS12_SWITCH_SUPPORT */
    L3_UNLOCK(unit);

    if (rt < 0) {
        return (rt);
    }

    info->l3a_vrf = l3cfg.l3c_vrf;
    info->l3a_ip_addr = l3cfg.l3c_ip_addr;
    info->l3a_flags = l3cfg.l3c_flags;
    sal_memcpy(info->l3a_nexthop_mac, l3cfg.l3c_mac_addr, sizeof(bcm_mac_t));
    info->l3a_intf = l3cfg.l3c_intf;
    info->l3a_port_tgid = l3cfg.l3c_port_tgid;
    info->l3a_modid = l3cfg.l3c_modid;

    BCM_IF_ERROR_RETURN(
        _bcm_esw_l3_gport_construct(unit, info->l3a_port_tgid, info->l3a_modid, 
                                    info->l3a_port_tgid, info->l3a_flags, 
                                    &(info->l3a_port_tgid)));
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_l3_host_add
 * Purpose:
 *      Add an entry to the L3 table.
 * Parameters:
 *      unit - SOC unit number
 *      info - Pointer to bcm_l3_host_t containing fields related to IP table.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *  1. Flag set to BCM_L3_IP6 for IPv6, default is IPv4
 *
 *  2. If BCM_L3_REPLACE flag is set, replace exisiting L3 entry
 *     with new information, otherwise, add only if key does not
 *     exist.
 *     On XGS, the DEFIP table points to the L3 entry, so this
 *     function will affect both L3 switching entry and routes.
 *     The affected route is one whose next hop IP address
 *     is the key (2nd parameter of this function).
 *     This is true for all XGS routes including ECMP
 *     routes, even though on Tucana/Lynx, the next hop IP
 *     address is not used in L3 hashing.
 */

int
bcm_esw_l3_host_add(int unit, bcm_l3_host_t *info)
{
    int rv;
    _bcm_l3_cfg_t l3cfg;
    bcm_l3_host_t info_orig, info_local;
    bcm_l3_route_t  info_route;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    /* Copy provided structure to local so it can be modified. */
    sal_memcpy(&info_local, info, sizeof(bcm_l3_host_t));

    /* Vrf is in device supported range check. */
    if ((info_local.l3a_vrf > SOC_VRF_MAX(unit)) || 
        (info_local.l3a_vrf < BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM);
    }
  
    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, info_local.l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }

    /* FP lookup class range check. */
    if ((info_local.l3a_lookup_class > SOC_ADDR_CLASS_MAX(unit)) || 
        (info_local.l3a_lookup_class < 0)) {
        return (BCM_E_PARAM);
    }

    if(!BCM_PRIORITY_VALID(info_local.l3a_pri)) {
        return (BCM_E_PARAM);
    }
    if (BCM_GPORT_IS_SET(info_local.l3a_port_tgid)) {
        
        /* l3a_port_tgid can contain either port or trunk */
        BCM_IF_ERROR_RETURN(
            _bcm_esw_l3_gport_resolve(unit, info_local.l3a_port_tgid, 
                                      &(info_local.l3a_port_tgid),
                                      &(info_local.l3a_modid), 
                                      &(info_local.l3a_port_tgid), 
                                      &(info_local.l3a_flags)));
    } else {
        PORT_DUALMODID_VALID(unit, info_local.l3a_port_tgid);
    }
    /* Check if host entry already exists. */
    L3_LOCK(unit);
    info_orig = info_local;
    rv = bcm_esw_l3_host_find(unit, &info_orig);
    if ((BCM_SUCCESS(rv)) && (0 == (info_local.l3a_flags & BCM_L3_REPLACE))) {
        /* If so BCM_L3_REPLACE flag must be set.  */
        L3_UNLOCK(unit);
        return (BCM_E_EXISTS);
    } else if ((BCM_FAILURE(rv)) && (BCM_E_NOT_FOUND != rv)) {
        L3_UNLOCK(unit);
        return (rv);
    }

    /* Initialize mbcm structure. */
    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    sal_memcpy(l3cfg.l3c_mac_addr, info_local.l3a_nexthop_mac, sizeof(bcm_mac_t));
    l3cfg.l3c_intf = info_local.l3a_intf;
    l3cfg.l3c_modid = info_local.l3a_modid;
    l3cfg.l3c_port_tgid = info_local.l3a_port_tgid;
    l3cfg.l3c_prio = info_local.l3a_pri;
    l3cfg.l3c_flags = info_local.l3a_flags;
    l3cfg.l3c_vrf = info_local.l3a_vrf;
    l3cfg.l3c_lookup_class = info_local.l3a_lookup_class;

    if (info_local.l3a_flags & BCM_L3_IP6) {
        sal_memcpy(l3cfg.l3c_ip6, info_local.l3a_ip6_addr, BCM_IP6_ADDRLEN);
        /* Add host entry - no prior entry found */
        if (BCM_FAILURE(rv)) {
            rv = mbcm_driver[unit]->mbcm_l3_ip6_add(unit, &l3cfg);
        } else {
            /* Remove Route entry and add to Host table */
            if (info_orig.l3a_flags & BCM_L3_HOST_AS_ROUTE) {

               /* Entry exists within ROUTE table */
               l3cfg.l3c_flags &=  ~BCM_L3_HOST_AS_ROUTE;

               /* Add entry to HOST Table */
               rv = mbcm_driver[unit]->mbcm_l3_ip6_add(unit, &l3cfg);

               if (BCM_SUCCESS(rv)) {

                  /* Init ROUTE entry */
                  sal_memset(&info_route, 0, sizeof(bcm_l3_route_t));
                  sal_memcpy(info_route.l3a_nexthop_mac, l3cfg.l3c_mac_addr, sizeof(bcm_mac_t));
                  info_route.l3a_vrf = l3cfg.l3c_vrf;
                  info_route.l3a_flags = BCM_L3_IP6;
                  sal_memcpy(info_route.l3a_ip6_net, l3cfg.l3c_ip6,
                                            sizeof(bcm_ip6_t));
                  bcm_ip6_mask_create(info_route.l3a_ip6_mask, 128);
                  info_route.l3a_intf = l3cfg.l3c_intf;
                  info_route.l3a_modid = l3cfg.l3c_modid;
                  info_route.l3a_port_tgid = l3cfg.l3c_port_tgid;
                  info_route.l3a_pri = l3cfg.l3c_prio;
                  info_route.l3a_lookup_class = l3cfg.l3c_lookup_class;

                  /* Delete ROUTE entry */
                  rv = bcm_esw_l3_route_delete(unit, &info_route);
                  if (BCM_SUCCESS(rv)) {
                     /* Special return value - as entry existed within ROUTE table */
#if defined(BCM_FIREBOLT_SUPPORT) || defined (BCM_EASYRIDER_SUPPORT)
                     (void) bcm_xgs3_l3_host_as_route_return_get(unit, &rv);
#endif /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT  */
                  }
               } else {
                  /* Update ROUTE entry */
                  l3cfg.l3c_flags |=  BCM_L3_HOST_AS_ROUTE;
                  rv = mbcm_driver[unit]->mbcm_l3_ip6_replace(unit, &l3cfg);
               }
            } else {
                 /* Replace prior host entry found */
                  rv = mbcm_driver[unit]->mbcm_l3_ip6_replace(unit, &l3cfg);
            }
        }
    } else {
        l3cfg.l3c_ip_addr = info_local.l3a_ip_addr;
        /* Add host entry - no prior entry found */
        if (BCM_FAILURE(rv)) {
            rv = mbcm_driver[unit]->mbcm_l3_ip4_add(unit, &l3cfg);
        } else {
            /* Remove Route entry and add to Host table */
            if (info_orig.l3a_flags & BCM_L3_HOST_AS_ROUTE) {

               /* Entry exists within ROUTE table */
               l3cfg.l3c_flags &=  ~BCM_L3_HOST_AS_ROUTE;

               /* Add entry to HOST Table */
               rv = mbcm_driver[unit]->mbcm_l3_ip4_add(unit, &l3cfg);

               if (BCM_SUCCESS(rv)) {

                  /* Init ROUTE entry */
                  sal_memset(&info_route, 0, sizeof(bcm_l3_route_t));
                  sal_memcpy(info_route.l3a_nexthop_mac, l3cfg.l3c_mac_addr, sizeof(bcm_mac_t));
                  info_route.l3a_vrf = l3cfg.l3c_vrf;
                  info_route.l3a_subnet = l3cfg.l3c_ip_addr;
                  info_route.l3a_ip_mask = bcm_ip_mask_create(32);
                  info_route.l3a_intf = l3cfg.l3c_intf;
                  info_route.l3a_modid = l3cfg.l3c_modid;
                  info_route.l3a_port_tgid = l3cfg.l3c_port_tgid;
                  info_route.l3a_pri = l3cfg.l3c_prio;
                  info_route.l3a_lookup_class = l3cfg.l3c_lookup_class;

                  /* Delete ROUTE entry */
                  rv = bcm_esw_l3_route_delete(unit, &info_route);
                  if (BCM_SUCCESS(rv)) {
                     /* Special return value - as entry existed within ROUTE table */
#if defined(BCM_FIREBOLT_SUPPORT) || defined (BCM_EASYRIDER_SUPPORT)
                     (void) bcm_xgs3_l3_host_as_route_return_get(unit, &rv);
#endif /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT  */
                  }
               } else {
                  /* Update ROUTE entry */
                  l3cfg.l3c_flags |=  BCM_L3_HOST_AS_ROUTE;
                  rv = mbcm_driver[unit]->mbcm_l3_ip4_replace(unit, &l3cfg);
               }
            } else {
                 /* Replace prior host entry found */
                 rv = mbcm_driver[unit]->mbcm_l3_ip4_replace(unit, &l3cfg);
            }
        }
    }
    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_ip_add(int unit, bcm_l3_host_t *info)
{
    return (bcm_esw_l3_host_add(unit, info));
}

/*
 * Function:
 *      bcm_esw_l3_host_delete
 * Purpose:
 *      Delete an entry from the L3 table.
 * Parameters:
 *      unit - SOC device unit number
 *      ip_addr - Pointer to host structure containing
 *                destination IP address.
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_INIT              Unit Not initialized yet
 *      BCM_E_NOT_FOUND      Cannot find a match
 * Notes:
 *     Flag set to BCM_L3_IP6 for IPv6, default is IPv4
 */

int
bcm_esw_l3_host_delete(int unit, bcm_l3_host_t *ip_addr)
{
    _bcm_l3_cfg_t l3cfg;
    bcm_l3_host_t info;
    int rt;

    L3_INIT(unit);

    if (NULL == ip_addr) {
        return (BCM_E_PARAM);
    }

    /* Vrf is in device supported range check. */
    if ((ip_addr->l3a_vrf > SOC_VRF_MAX(unit)) || 
        (ip_addr->l3a_vrf < BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM);
    }
  
    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, ip_addr->l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }

    bcm_l3_host_t_init(&info);
    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    info.l3a_vrf = ip_addr->l3a_vrf;

    L3_LOCK(unit);
    if (ip_addr->l3a_flags & BCM_L3_IP6) {
        info.l3a_flags   = BCM_L3_IP6;
        sal_memcpy(info.l3a_ip6_addr, ip_addr->l3a_ip6_addr, BCM_IP6_ADDRLEN);
        rt = bcm_esw_l3_host_find(unit, &info);
        if (rt != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return rt;
        }

        sal_memcpy(l3cfg.l3c_ip6,
                   ip_addr->l3a_ip6_addr, BCM_IP6_ADDRLEN);
        l3cfg.l3c_flags = info.l3a_flags;
        l3cfg.l3c_vrf = ip_addr->l3a_vrf;
        rt = mbcm_driver[unit]->mbcm_l3_ip6_delete(unit, &l3cfg);
    } else {
        info.l3a_ip_addr = ip_addr->l3a_ip_addr;
        rt = bcm_esw_l3_host_find(unit, &info);
        if (rt != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return rt;
        }

        l3cfg.l3c_ip_addr = ip_addr->l3a_ip_addr;
        l3cfg.l3c_flags = info.l3a_flags;  /* BCM_L3_L2TOCPU might be set */
        l3cfg.l3c_vrf = ip_addr->l3a_vrf;
        rt = mbcm_driver[unit]->mbcm_l3_ip4_delete(unit, &l3cfg);
    }
    L3_UNLOCK(unit);
    return rt;
}

/* to be deprecated */
int
bcm_esw_l3_ip_delete(int unit, ip_addr_t ip_addr)
{
    _bcm_l3_cfg_t l3cfg;
    bcm_l3_host_t info;
    int rt;

    L3_INIT(unit);

    bcm_l3_host_t_init(&info);
    info.l3a_ip_addr = ip_addr;
    L3_LOCK(unit);
    rt = bcm_esw_l3_host_find(unit, &info);
    if (rt != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return rt;
    }

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_ip_addr = ip_addr;
    l3cfg.l3c_flags = info.l3a_flags;  /* BCM_L3_L2TOCPU might be set */

    rt = mbcm_driver[unit]->mbcm_l3_ip4_delete(unit, &l3cfg);
    L3_UNLOCK(unit);

    return rt;
}

/*
 * Function:
 *      bcm_esw_l3_host_delete_by_network
 * Purpose:
 *      Delete all L3 entries that match the IP address mask
 * Parameters:
 *      unit    - SOC device unit number
 *      net_addr - Pointer to route structure containing the network
 *                address to be matched against
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_INIT              Unit Not initialized yet
 * Notes:
 *     Flag set to BCM_L3_IP6 for IPv6, default is IPv4
 */

int
bcm_esw_l3_host_delete_by_network(int unit, bcm_l3_route_t *net_addr)
{
    _bcm_l3_cfg_t l3cfg;
    int rv;

    L3_INIT(unit);

    if (NULL == net_addr) {
        return (BCM_E_PARAM);
    }

    /* Vrf is in device supported range check. */
    if ((net_addr->l3a_vrf > SOC_VRF_MAX(unit)) || 
        (net_addr->l3a_vrf < BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM);
    }

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, net_addr->l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_vrf        = net_addr->l3a_vrf;

    L3_LOCK(unit);
    if (net_addr->l3a_flags & BCM_L3_IP6) {
        sal_memcpy(l3cfg.l3c_ip6,
                   net_addr->l3a_ip6_net, BCM_IP6_ADDRLEN);
        sal_memcpy(l3cfg.l3c_ip6_mask,
                   net_addr->l3a_ip6_mask, BCM_IP6_ADDRLEN);
        l3cfg.l3c_flags = BCM_L3_IP6;
        rv = mbcm_driver[unit]->mbcm_l3_ip6_delete_prefix(unit, &l3cfg);
    } else { 
        l3cfg.l3c_ip_addr    = net_addr->l3a_subnet;
        l3cfg.l3c_ip_mask    = net_addr->l3a_ip_mask;
        rv = mbcm_driver[unit]->mbcm_l3_ip4_delete_prefix(unit, &l3cfg);
    } 
    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_ip_delete_by_prefix(int unit, ip_addr_t ip_addr, ip_addr_t mask)
{
    int rv;
    _bcm_l3_cfg_t l3cfg;

    L3_INIT(unit);

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_ip_addr = ip_addr;
    l3cfg.l3c_ip_mask = mask;
    L3_LOCK(unit);
    rv = mbcm_driver[unit]->mbcm_l3_ip4_delete_prefix(unit, &l3cfg);
    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_host_delete_by_interface
 * Purpose:
 *      Delete all L3 entries that match the L3 interface
 * Parameters:
 *      unit - SOC device unit number
 *      info - The host structure containing the to be matched interface ID
 *             (IN)l3a_intf_id member
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INIT              Unit Not initialized yet
 * Notes:
 *   - Flag set to BCM_L3_NEGATE to delete all host entries
 *     that do NOT match the L3 interface.
 */
int
bcm_esw_l3_host_delete_by_interface(int unit, bcm_l3_host_t *info)
{
    int rv;
    _bcm_l3_cfg_t l3cfg;
    int negate;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));

    l3cfg.l3c_intf = info->l3a_intf;
    negate = info->l3a_flags & BCM_L3_NEGATE ? 1 : 0;

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_delete_intf(unit, &l3cfg, negate);

    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_ip_delete_by_interface(int unit, int intf)
{
    int rv;
    _bcm_l3_cfg_t l3cfg;

    L3_INIT(unit);

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_intf = intf;

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_delete_intf(unit, &l3cfg, FALSE);

    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_host_delete_all
 * Purpose:
 *      Delete all entries from the L3 table.
 * Parameters:
 *      unit - SOC device unit number
 *      info - Pointer to host structure containing the flag
 *             indicating IPv4 or IPv6
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_INIT              Unit Not initialized yet
 */
int
bcm_esw_l3_host_delete_all(int unit, bcm_l3_host_t *info)
{
    int rv;
    L3_INIT(unit);

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_delete_all(unit);

    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_ip_delete_all(int unit)
{
    return  (bcm_esw_l3_host_delete_all(unit, NULL));
}

/* to be deprecated */
int
bcm_esw_l3_ip_update_entry_by_key(int unit, bcm_l3_host_t *info)
{
    int rv;
    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    info->l3a_flags |= BCM_L3_REPLACE;

    L3_LOCK(unit);

    rv = bcm_esw_l3_host_add(unit, info);

    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_host_conflict_get
 * Purpose:
 *      Given a IP address, return conflicts in the L3 table.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipkey    - IP address to test conflict condition
 *      cf_array - (OUT) arrary of conflicting addresses(at most 8)
 *      cf_max   - max number of conflicts wanted
 *      cf_count    - (OUT) actual # of conflicting addresses
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_host_conflict_get(int unit, bcm_l3_key_t *ipkey, bcm_l3_key_t *cf_array,
                    int cf_max, int *cf_count)
{
    int rv;
    L3_INIT(unit);

    if (NULL == ipkey) {
        return (BCM_E_PARAM);
    } 

    /* Vrf is in device supported range check. */
    if ((ipkey->l3k_vrf > SOC_VRF_MAX(unit)) || 
        (ipkey->l3k_vrf < BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM);
    }

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, ipkey->l3k_flags)) {
        return (BCM_E_UNAVAIL);
    }

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_conflict_get(unit, ipkey, cf_array, 
                                                 cf_max, cf_count);
    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_host_age
 * Purpose:
 *      Age out the L3 entry by clearing the HIT bit when appropriate,
 *      the L3 host entry itself is removed if HIT bit is not set.
 * Parameters:
 *      unit - SOC device unit number
 *      flags - The criteria used to age out L3 table.
 *      age_cb - Call back routine.
 *      user_data  - (IN) User provided cookie for callback.
 * Returns:
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INIT              Unit Not initialized yet
 */

int
bcm_esw_l3_host_age(int unit, uint32 flags, bcm_l3_host_traverse_cb age_cb, 
                    void *user_data)
{
    int rv;
    L3_INIT(unit);

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, flags)) {
        return (BCM_E_UNAVAIL);
    }

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_age(unit, flags, age_cb, user_data);

    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_host_traverse
 * Purpose:
 *      Go through all valid L3 entries, and call the callback function
 *      at each entry
 * Parameters:
 *      unit - SOC device unit number
 *      flags - BCM_L3_IP6
 *      start - Callback called after this many L3 entries
 *      end   - Callback called before this many L3 entries
 *      cb    - User supplied callback function
 *      user_data - User supplied cookie used in parameter in callback function
 * Returns:
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INIT              Unit Not initialized yet
 */

int
bcm_esw_l3_host_traverse(int unit, uint32 flags,
                         uint32 start, uint32 end,
                         bcm_l3_host_traverse_cb cb, void *user_data)
{
    int rv;

    L3_INIT(unit);

    if (NULL == cb) {
        return (BCM_E_PARAM);
    }

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, flags)) {
        return (BCM_E_UNAVAIL);
    }

    L3_LOCK(unit);

    if (flags & BCM_L3_IP6) {
        rv = mbcm_driver[unit]->mbcm_l3_ip6_traverse(unit,
                                start, end, cb, user_data);
    } else {
        rv = mbcm_driver[unit]->mbcm_l3_ip4_traverse(unit,
                                start, end, cb, user_data);
    }

    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
bcm_l3_age_cb old_age_out;

STATIC int
_bcm_l3_host_ageout(int unit, int index, bcm_l3_host_t *info, void *cookie)
{
    if (old_age_out) {
        old_age_out(unit, info->l3a_ip_addr);
    }

    return BCM_E_NONE;
}

int
bcm_esw_l3_age(int unit, uint32 flags, bcm_l3_age_cb age_out)
{
    int rv;
    old_age_out = age_out;

    L3_INIT(unit);

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, flags)) {
        return (BCM_E_UNAVAIL);
    }

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_age(unit, flags, 
                                        _bcm_l3_host_ageout, NULL);
    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
/*
 * Function:
 *      bcm_esw_l3_conflict_get
 * Purpose:
 *      Given a IP address, return conflicts in the L3 table.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipkey    - IP address to test conflict condition
 *      cf_array - (OUT) arrary of conflicting addresses(at most 8)
 *      cf_max   - max number of conflicts wanted
 *      cf_count    - (OUT) actual # of conflicting addresses
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_conflict_get(int unit, bcm_l3_key_t *ipkey, bcm_l3_key_t *cf_array,
                        int cf_max, int *cf_count)
{
    return (bcm_esw_l3_host_conflict_get(unit, ipkey, cf_array,
                                         cf_max, cf_count));
}

/*
 * Function:
 *      bcm_esw_l3_host_invalidate_entry
 * Purpose:
 *      Given a IP address, invalidate the L3 entry without
 *      clearing the entry information, so that the entry can be
 *      turned back to valid without resetting all the information.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipaddr   - IP address to test conflict condition
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_host_invalidate_entry(int unit, bcm_ip_t ipaddr)
{
#ifdef BCM_XGS12_SWITCH_SUPPORT
    int rv;
#endif  /* BCM_XGS_SWITCH_SUPPORT */
    L3_INIT(unit);

#ifdef BCM_XGS12_SWITCH_SUPPORT
    if (SOC_IS_XGS12_SWITCH(unit)) {
        L3_LOCK(unit);
        rv = bcm_xgs_l3_invalidate_entry(unit, ipaddr);
        L3_UNLOCK(unit);
        return (rv);
    }
#endif  /* BCM_XGS_SWITCH_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_l3_host_validate_entry
 * Purpose:
 *      Given a IP address, validate the L3 entry without
 *      resetting the entry information.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipaddr   - IP address to test conflict condition
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_host_validate_entry(int unit, bcm_ip_t ipaddr)
{
#ifdef BCM_XGS12_SWITCH_SUPPORT
    int rv;
#endif  /* BCM_XGS_SWITCH_SUPPORT */
    L3_INIT(unit);

#ifdef BCM_XGS12_SWITCH_SUPPORT
    if (SOC_IS_XGS12_SWITCH(unit)) {
        L3_LOCK(unit);
        rv = bcm_xgs_l3_validate_entry(unit, ipaddr);
        L3_UNLOCK(unit);
        return (rv);
    }
#endif  /* BCM_XGS_SWITCH_SUPPORT */

    return BCM_E_UNAVAIL;
}

/* to be deprecated */
/*
 * Function:
 *      bcm_esw_l3_invalidate_entry
 * Purpose:
 *      Given a IP address, invalidate the L3 entry without
 *      clearing the entry information, so that the entry can be
 *      turned back to valid without resetting all the information.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipaddr   - IP address to test conflict condition
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_invalidate_entry(int unit, bcm_ip_t ipaddr)
{
    return (bcm_esw_l3_host_invalidate_entry(unit, ipaddr));
}

/* to be deprecated */
/*
 * Function:
 *      bcm_esw_l3_validate_entry
 * Purpose:
 *      Given a IP address, validate the L3 entry without
 *      resetting the entry information.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipaddr   - IP address to test conflict condition
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_validate_entry(int unit, bcm_ip_t ipaddr)
{
    return (bcm_esw_l3_host_validate_entry(unit, ipaddr));
}

/*
 * Function:
 *      bcm_esw_l3_route_add
 * Purpose:
 *      Add an IP route to the DEFIP table.
 * Parameters:
 *      unit - SOC device unit number
 *      info - Pointer to bcm_l3_route_t containing all valid fields.
 * Returns:
 *      BCM_E_XXX
 * Note:
 *  1. In addition to the route info (i.e. the network number,
 *     nexthop MAC addr, L3 intf, port, modid, or trunk ID),
 *     on XGS chips, one must specify the nexthop IP addr.
 *     This is used as key to find entry in the L3 table.
 *
 *  2. XGS ECMP route must have BCM_L3_MULTIPATH flag set.
 *     ECMP is supported on Draco1.5, Tucana, Lynx.
 *
 *  3. For Draco1.5(BCM5695) per VLAN default route (0.0.0.0/0), VLAN ID is
 *     required to set default route for a particular VLAN;
 *     If VLAN ID is set to 0, it means to set global default route
 *     for all VLANs, except those VLANs whose per VLAN default routes
 *     are set.
 *
 *     For 5690/5665/5673 which only support a single
 *     global default route, the VLAN ID field is ignored,
 *     but ECMP is supported on this global default route.
 *
 *  4. BCM_L3_DEFIP_LOCAL flag is used, then packets that matches
 *     LPM table will be sent to CPU unchanged (XGS only).
 *     BCM_L3_DEFIP_LOCAL flag is used for last hop direct-connected
 *     subnet (e.g where workstations are attached to router).
 *     BCM_L3_DEFIP_LOCAL and BCM_L3_MULTIPATH can not be both set.
 *     When BCM_L3_DEFIP_LOCAL flag is used, nexthop IP addr
 *     is not needed.
 *
 *  5. For 5695, in order to achieve ECMP for default route (0.0.0.0/0),
 *     you should specify BCM_L3_MULTIPATH flag; otherwise,
 *     per VLAN based default route will be installed.
 *     If you want to delete all these ECMP default route in one shut,
 *     BCM_L3_LPM_DEFROUTE flag should be used in bcm_l3_route_delete,
 *     BCM_L3_MULTIPATH flag means delete one ECMP path only.
 */

int
bcm_esw_l3_route_add(int unit, bcm_l3_route_t *info)
{
    int max_prefix_length;
    _bcm_defip_cfg_t defip;
    uint8 ip6_zero[BCM_IP6_ADDRLEN] = {0};
    int rv;
    bcm_l3_route_t  info_local;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    /* Copy provided structure to local so it can be modified. */
    sal_memcpy(&info_local, info, sizeof(bcm_l3_route_t));

    if ((info_local.l3a_vrf > SOC_VRF_MAX(unit)) || 
        (info_local.l3a_vrf < BCM_L3_VRF_GLOBAL)) {
        return (BCM_E_PARAM);
    }

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, info_local.l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }

    if ((info_local.l3a_lookup_class > SOC_ADDR_CLASS_MAX(unit)) || 
        (info_local.l3a_lookup_class < 0)) {
        return (BCM_E_PARAM);
    }

    if(!BCM_PRIORITY_VALID(info_local.l3a_pri)) {
        return (BCM_E_PARAM);
    }

    if (BCM_GPORT_IS_SET(info_local.l3a_port_tgid)) {
        
        /* l3a_port_tgid can contain either port or trunk */
        BCM_IF_ERROR_RETURN(
            _bcm_esw_l3_gport_resolve(unit, info_local.l3a_port_tgid, 
                                      &(info_local.l3a_port_tgid),
                                      &(info_local.l3a_modid), 
                                      &(info_local.l3a_port_tgid), 
                                      &(info_local.l3a_flags)));
    } else {
        PORT_DUALMODID_VALID(unit, info_local.l3a_port_tgid);
    }

    sal_memset(&defip, 0, sizeof(_bcm_defip_cfg_t));

    sal_memcpy(defip.defip_mac_addr, info_local.l3a_nexthop_mac,
               sizeof(bcm_mac_t));
    defip.defip_intf       = info_local.l3a_intf;
    defip.defip_modid      = info_local.l3a_modid;
    defip.defip_port_tgid  = info_local.l3a_port_tgid;
    defip.defip_vid        = info_local.l3a_vid;
    defip.defip_flags      = info_local.l3a_flags;
    defip.defip_prio       = info_local.l3a_pri;
    defip.defip_vrf        = info_local.l3a_vrf;
    defip.defip_tunnel_option = (info_local.l3a_tunnel_option & 0xffff);
    defip.defip_mpls_label = info_local.l3a_mpls_label;
    defip.defip_lookup_class      = info_local.l3a_lookup_class;

    L3_LOCK(unit);
    if (info_local.l3a_flags & BCM_L3_IP6) {
        max_prefix_length = 
            soc_feature(unit, soc_feature_lpm_prefix_length_max_128) ? 128 : 64;
        if (bcm_ip6_mask_length(info_local.l3a_ip6_mask) == 0 &&
            sal_memcmp(info_local.l3a_ip6_net, ip6_zero, BCM_IP6_ADDRLEN) != 0) {
            L3_UNLOCK(unit);
            return (BCM_E_PARAM);
        }
        sal_memcpy(defip.defip_ip6_addr, info_local.l3a_ip6_net, BCM_IP6_ADDRLEN);
        defip.defip_sub_len = bcm_ip6_mask_length(info_local.l3a_ip6_mask);
        if (defip.defip_sub_len > max_prefix_length) {
            L3_UNLOCK(unit);
            return (BCM_E_PARAM);
        } 
        rv = mbcm_driver[unit]->mbcm_ip6_defip_add(unit, &defip);
    } else {
        if (info_local.l3a_ip_mask == 0 && info_local.l3a_subnet != 0) {
            L3_UNLOCK(unit);
            return (BCM_E_PARAM);
        }
        defip.defip_ip_addr    = info_local.l3a_subnet & info_local.l3a_ip_mask;
        defip.defip_sub_len    = bcm_ip_mask_length(info_local.l3a_ip_mask);
        defip.defip_nexthop_ip = info_local.l3a_nexthop_ip;
        rv = mbcm_driver[unit]->mbcm_ip4_defip_add(unit, &defip);
    }
    L3_UNLOCK(unit);

    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_defip_add(int unit, bcm_l3_route_t *info)
{
    return (bcm_esw_l3_route_add(unit, info));
}

/*
 * Function:
 *      bcm_esw_l3_route_delete
 * Purpose:
 *      Delete an entry from the Default IP Router table.
 * Parameters:
 *      unit - SOC device unit number
 *      info - Pointer to bcm_l3_route_t structure with valid IP subnet & mask.
 * Returns:
 *      BCM_E_XXX.
 * Note:
 *  1. For non-ECMP routes, it suffices to specify the network number
 *     in order to delete the route, since there is only one path.
 *
 *  2. For ECMP routes, in addition to specifying the network number,
 *     one must specify which ECMP path is to be deleted.
 *     This is done by specifying the following parameters:
 *     nexthop MAC addr, L3 intf, port, modid, next hop IP addr(Draco1.5 only).
 *
 *     This is how these parameters are used to identify the ECMP path:
 *        next hop IP addr(Draco 1.5 only) | nexthop MAC addr |
 *        (L3 intf, port, modid)
 *     Unspecified nexthop IP or MAC addr must be set to 0.
 *
 *     XGS ECMP route must have BCM_L3_MULTIPATH flag set.
 *     ECMP is supported on Draco1.5, Tucana, Lynx.
 *
 *  3. For Draco1.5(BCM5695) per VLAN default route (0.0.0.0/0), VLAN ID is
 *     required to delete default route for a particular VLAN;
 *     If VLAN ID is set to 0 (i.e. BCM_VLAN_NONE), it means to delete
 *     the global default route,  in this case, those VLANs whose
 *     per VLAN default routes are set stay intact.
 *     If the VLAN ID is set to BCM_VLAN_ALL (i.e. 0xffff), it means to
 *     delete all per VLAN default routes - the global and all
 *     per VLAN default routes.
 *
 *     For 5690/5665/5673 which only support a single
 *     global default route, the VLAN ID field is ignored,
 *     but ECMP is supported on this global default route.
 *
 *  4. For 5695, in order to achieve ECMP for default route (0.0.0.0/0),
 *     you should specify BCM_L3_MULTIPATH flag; otherwise,
 *     per VLAN based default route will be installed.
 *     If you want to delete all these ECMP default route in one shut,
 *     BCM_L3_LPM_DEFROUTE flag should be used in bcm_l3_route_delete,
 *     BCM_L3_MULTIPATH flag means delete one ECMP path only.
 */

int
bcm_esw_l3_route_delete(int unit, bcm_l3_route_t *info)
{
    int max_prefix_length;
    _bcm_defip_cfg_t defip;
    int rv;
    bcm_l3_route_t info_local;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    /* Copy provided structure to local so it can be modified. */
    sal_memcpy(&info_local, info, sizeof(bcm_l3_route_t));

    if ((info_local.l3a_vrf > SOC_VRF_MAX(unit)) || 
        (info_local.l3a_vrf < BCM_L3_VRF_GLOBAL)) {
        return (BCM_E_PARAM);
    }

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, info_local.l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }

    sal_memset(&defip, 0, sizeof(_bcm_defip_cfg_t));

    defip.defip_vid        = info_local.l3a_vid;
    defip.defip_flags      = info_local.l3a_flags;
    defip.defip_vrf        = info_local.l3a_vrf;

    if (info_local.l3a_flags & BCM_L3_MULTIPATH) {
         if (BCM_GPORT_IS_SET(info_local.l3a_port_tgid)) {
              /* l3a_port_tgid can contain either port or trunk */
              BCM_IF_ERROR_RETURN(
                   _bcm_esw_l3_gport_resolve(unit, info_local.l3a_port_tgid, 
                                  &(info_local.l3a_port_tgid),
                                  &(info_local.l3a_modid), 
                                  &(info_local.l3a_port_tgid), 
                                  &(info_local.l3a_flags)));
         } 

        sal_memcpy(defip.defip_mac_addr, info_local.l3a_nexthop_mac,
                   sizeof(bcm_mac_t));
        defip.defip_intf      = info_local.l3a_intf;
        defip.defip_port_tgid = info_local.l3a_port_tgid;
        defip.defip_modid     = info_local.l3a_modid;
        defip.defip_flags	   = info_local.l3a_flags;
    }

    L3_LOCK(unit);
    if (info_local.l3a_flags & BCM_L3_IP6) {
        max_prefix_length = 
            soc_feature(unit, soc_feature_lpm_prefix_length_max_128) ? 128 : 64;
        sal_memcpy(defip.defip_ip6_addr, info_local.l3a_ip6_net, BCM_IP6_ADDRLEN);
        defip.defip_sub_len    = bcm_ip6_mask_length(info_local.l3a_ip6_mask);
        if (defip.defip_sub_len > max_prefix_length) {
            L3_UNLOCK(unit);
            return (BCM_E_PARAM);
        } 
        rv = mbcm_driver[unit]->mbcm_ip6_defip_delete(unit, &defip);
    } else {
        defip.defip_ip_addr    = info_local.l3a_subnet & info_local.l3a_ip_mask;
        defip.defip_sub_len    = bcm_ip_mask_length(info_local.l3a_ip_mask);
        defip.defip_nexthop_ip = info_local.l3a_nexthop_ip;

        rv = mbcm_driver[unit]->mbcm_ip4_defip_delete(unit, &defip);
    }

    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_defip_delete(int unit, bcm_l3_route_t *info)
{
    return (bcm_esw_l3_route_delete(unit, info));
}

/*
 * Function:
 *      bcm_esw_l3_route_get
 * Purpose:
 *      Get an entry from the DEF IP table.
 * Parameters:
 *      unit - SOC device unit number
 *      info - (OUT)Pointer to bcm_l3_route_t for return information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *    - Application has to pass IP address, IP mask for the network
 *    - If the route is local, then the BCM_L3_DEFIP_LOCAL flag is
 *      set, and next hop MAC, port/tgid, interface, MODID fields
 *      are all cleared.
 */

int
bcm_esw_l3_route_get(int unit, bcm_l3_route_t *info)
{
    int max_prefix_length;
    _bcm_defip_cfg_t defip;
    int rv;

    L3_INIT(unit);

    if (NULL == info) {
        return BCM_E_PARAM;
    }

    if ((info->l3a_vrf > SOC_VRF_MAX(unit)) || 
        (info->l3a_vrf < BCM_L3_VRF_GLOBAL)) {
        return (BCM_E_PARAM);
    }

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, info->l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }

    sal_memset(&defip, 0, sizeof(_bcm_defip_cfg_t));
    defip.defip_flags = info->l3a_flags;
    defip.defip_vrf     = info->l3a_vrf;

    L3_LOCK(unit);
    if (info->l3a_flags & BCM_L3_IP6) {
        max_prefix_length = 
            soc_feature(unit, soc_feature_lpm_prefix_length_max_128) ? 128 : 64;
        sal_memcpy(defip.defip_ip6_addr, info->l3a_ip6_net, BCM_IP6_ADDRLEN);
        defip.defip_sub_len    = bcm_ip6_mask_length(info->l3a_ip6_mask);
        if (defip.defip_sub_len > max_prefix_length) {
            L3_UNLOCK(unit);
            return (BCM_E_PARAM);
        } 
        rv = mbcm_driver[unit]->mbcm_ip6_defip_cfg_get(unit, &defip);
    } else {
        defip.defip_ip_addr = info->l3a_subnet & info->l3a_ip_mask;
        defip.defip_sub_len = bcm_ip_mask_length(info->l3a_ip_mask);
        rv = mbcm_driver[unit]->mbcm_ip4_defip_cfg_get(unit, &defip);
    }

    L3_UNLOCK(unit);
    if (rv < 0) {
        return rv;
    }

    sal_memcpy(info->l3a_nexthop_mac, defip.defip_mac_addr,
               sizeof(bcm_mac_t));
    info->l3a_nexthop_ip = defip.defip_nexthop_ip;
    info->l3a_intf = defip.defip_intf;
    info->l3a_port_tgid = defip.defip_port_tgid;
    info->l3a_modid = defip.defip_modid;
    info->l3a_lookup_class = defip.defip_lookup_class;
    info->l3a_flags = defip.defip_flags;
    info->l3a_mpls_label = defip.defip_mpls_label;
    info->l3a_tunnel_option = defip.defip_tunnel_option;

    BCM_IF_ERROR_RETURN(
        _bcm_esw_l3_gport_construct(unit, info->l3a_port_tgid, info->l3a_modid, 
                                    info->l3a_port_tgid, info->l3a_flags, 
                                    &(info->l3a_port_tgid)));
    return BCM_E_NONE;
}

/* to be deprecated */
int
bcm_esw_l3_defip_get(int unit, bcm_l3_route_t *info)
{
    return (bcm_esw_l3_route_get(unit, info));
}

/*
 * Function:
 *      bcm_esw_l3_route_multipath_get
 * Purpose:
 *      Get all paths for a route, useful for ECMP route,
 *      For non-ECMP route, there is only one path returned.
 * Parameters:
 *      unit       - (IN) SOC device unit number
 *      the_route  - (IN) route's net/mask
 *      path_array - (OUT) Array of all ECMP paths
 *      path_count - (OUT) Actual number of ECMP paths
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_l3_route_multipath_get(int unit, bcm_l3_route_t *the_route,
       bcm_l3_route_t *path_array, int max_path, int *path_count)
{
    int max_prefix_length;
    _bcm_defip_cfg_t defip;
    int rv;

    L3_INIT(unit);

    if ((NULL == the_route) || (max_path < 1)) {
        return BCM_E_PARAM;
    }

    if ((the_route->l3a_vrf > SOC_VRF_MAX(unit)) || 
        (the_route->l3a_vrf < BCM_L3_VRF_GLOBAL)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&defip, 0, sizeof(_bcm_defip_cfg_t));
    defip.defip_flags = the_route->l3a_flags;
    defip.defip_vrf = the_route->l3a_vrf;

    L3_LOCK(unit);
    if (the_route->l3a_flags & BCM_L3_IP6) {
        max_prefix_length = 
            soc_feature(unit, soc_feature_lpm_prefix_length_max_128) ? 128 : 64;
        sal_memcpy(defip.defip_ip6_addr, the_route->l3a_ip6_net, BCM_IP6_ADDRLEN);
        defip.defip_sub_len    = bcm_ip6_mask_length(the_route->l3a_ip6_mask);
        if (defip.defip_sub_len > max_prefix_length) {
            L3_UNLOCK(unit);
            return (BCM_E_PARAM);
        } 
        rv = mbcm_driver[unit]->mbcm_ip6_defip_ecmp_get_all(unit, &defip,
                                 path_array, max_path, path_count);
    } else {
        defip.defip_ip_addr = the_route->l3a_subnet & the_route->l3a_ip_mask;
        defip.defip_sub_len = bcm_ip_mask_length(the_route->l3a_ip_mask);
        rv = mbcm_driver[unit]->mbcm_ip4_defip_ecmp_get_all(unit, &defip,
                                 path_array, max_path, path_count);
    }
    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *    bcm_esw_l3_defip_find_index
 * Purpose:
 *    Find an entry from the DEF IP table based on index.
 * Parameters:
 *    unit  - SOC unit number
 *    index - The route index number (not hardware table index)
 *    info - (OUT) Pointer to memory for bcm_l3_route_t.
 * Returns:
 *    BCM_E_NONE          Success
 *    BCM_E_UNIT          Illegal unit number
 *    BCM_E_INVALID_INDEX Illegal index
 *    BCM_E_INTERNAL      Chip access failure
 *    BCM_E_INIT          Unit Not initialized yet
 * Notes:
 *    The hit returned may be either L3SH or L3DH or Both
 */

/* to be deprecated */

int
bcm_esw_l3_defip_find_index(int unit, int index, bcm_l3_route_t *info)
{
    int rt = BCM_E_UNAVAIL;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    if ((info->l3a_vrf > SOC_VRF_MAX(unit)) || 
        (info->l3a_vrf < BCM_L3_VRF_GLOBAL)) {
        return (BCM_E_PARAM);
    }

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, info->l3a_flags)) {
        return (BCM_E_UNAVAIL);
    }
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_XGS12_SWITCH_SUPPORT)
    if (SOC_IS_XGS12_SWITCH(unit)) {
        L3_LOCK(unit);
        rt = bcm_xgs_defip_find_index(unit, index, info);
        L3_UNLOCK(unit);
    }
#endif /* BCM_XGS12_SWITCH_SUPPORT */

    BCM_IF_ERROR_RETURN(
        _bcm_esw_l3_gport_construct(unit, info->l3a_port_tgid, info->l3a_modid, 
                                    info->l3a_port_tgid, info->l3a_flags, 
                                    &(info->l3a_port_tgid)));
    return (rt);
}

/*
 * Function:
 *      bcm_esw_l3_route_delete_by_interface
 * Purpose:
 *      Delete all entries from the DEF IP table which have
 *      L3 interface number intf.
 * Parameters:
 *      unit - SOC device unit number
 *      intf - The L3 interface number
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_UNIT              Illegal unit number
 *      BCM_E_INIT              Unit Not initialized yet
 * Notes:
 *   - Flag set to BCM_L3_IP6 for IPv6, default is IPv4
 *   - Flag set to BCM_L3_NEGATE to delete all host entries
 *     that do NOT match the L3 interface.
 */

int
bcm_esw_l3_route_delete_by_interface(int unit, bcm_l3_route_t *info)
{
    int rv;
    _bcm_defip_cfg_t defip;

    L3_INIT(unit);

    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    sal_memset(&defip, 0, sizeof(_bcm_defip_cfg_t));
    defip.defip_intf = info->l3a_intf;
    defip.defip_flags = info->l3a_flags;

    L3_LOCK(unit);

    if (info->l3a_flags & BCM_L3_NEGATE) {
        rv = mbcm_driver[unit]->mbcm_defip_delete_intf(unit, &defip, 1);
    } else {
        rv = mbcm_driver[unit]->mbcm_defip_delete_intf(unit, &defip, 0);
    }

    L3_UNLOCK(unit);

    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_defip_delete_by_interface(int unit, int intf)
{
    bcm_l3_route_t info;

    L3_INIT(unit);

    bcm_l3_route_t_init(&info);
    info.l3a_intf = intf;

    return (bcm_esw_l3_route_delete_by_interface(unit, &info));
}

/*
 * Function:
 *      bcm_esw_l3_defip_delete_all
 * Purpose:
 *      Remove all defip entries for a specified unit.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE - delete successful.
 *      BCM_E_XXX - delete failed.
 */

int
bcm_esw_l3_route_delete_all(int unit, bcm_l3_route_t *info)
{
    int rv;
    L3_INIT(unit);

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_defip_delete_all(unit);

    L3_UNLOCK(unit);

    return (rv);
}


/* to be deprecated */
int
bcm_esw_l3_defip_delete_all(int unit)
{
    return (bcm_esw_l3_route_delete_all(unit, NULL));
}

/*
 * Function:
 *      bcm_esw_l3_route_traverse
 * Purpose:
 *      Find entries from the DEF IP table.
 * Parameters:
 *      unit - SOC device unit number
 *      flags - BCM_L3_IP6
 *      start - Starting point of interest.
 *      end - Ending point of interest.
 *      trav_fn - User callback function, called once per defip entry.
 *      user_data - User supplied cookie used in parameter in callback function
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The hit returned may be either L3SH or L3DH or Both
 */
int
bcm_esw_l3_route_traverse(int unit, uint32 flags,
                          uint32 start, uint32 end,
                          bcm_l3_route_traverse_cb trav_fn, void *user_data)
{
    int rv;

    L3_INIT(unit);
    L3_LOCK(unit);

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, flags)) {
        return (BCM_E_UNAVAIL);
    }

    if (flags & BCM_L3_IP6) {
        rv = mbcm_driver[unit]->mbcm_ip6_defip_traverse(unit,
                                start, end, trav_fn, user_data);
    } else {
        rv = mbcm_driver[unit]->mbcm_ip4_defip_traverse(unit,
                                start, end, trav_fn, user_data);
    }

    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_defip_traverse(int unit, bcm_l3_route_traverse_cb trav_fn,
                      uint32 start, uint32 end)
{
    int rv;
    L3_INIT(unit);

    if (start > end) {
        return (BCM_E_PARAM);
    }

    L3_LOCK(unit);

    rv  = mbcm_driver[unit]->mbcm_ip4_defip_traverse(unit,
                                     start, end, trav_fn, NULL);
    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_route_age
 * Purpose:
 *      Age on DEFIP routing table, clear HIT bit of entry if set
 * Parameters:
 *      unit       - (IN) BCM device unit number.
 *      age_out    - (IN) Call back routine.
 *      user_data  - (IN) User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_route_age(int unit, uint32 flags, bcm_l3_route_traverse_cb age_out,
                     void *user_data)
{
    int rv;
    L3_INIT(unit);

    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_defip_age(unit, age_out, user_data);

    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_defip_age(int unit, bcm_l3_route_traverse_cb age_out)
{
    return (bcm_esw_l3_route_age(unit, 0, age_out, NULL));
}

/* to be deprecated */
int
bcm_esw_l3_status(int unit, int *free_l3intf, int *free_l3,
                  int *free_defip, int *free_lpm_blk)
{
    int rv;
    bcm_l3_info_t       l3info;

    L3_INIT(unit);

    sal_memset(&l3info, 0, sizeof(bcm_l3_info_t));

    L3_LOCK(unit);
    rv = bcm_esw_l3_info(unit, &l3info);
    L3_UNLOCK(unit);
    BCM_IF_ERROR_RETURN(rv);

    if (free_l3intf != NULL) {
        *free_l3intf = l3info.l3info_max_intf - l3info.l3info_occupied_intf;
    }

    if (free_l3 != NULL) {
        *free_l3 = l3info.l3info_max_l3 - l3info.l3info_occupied_l3;
    }

    if (free_defip != NULL) {
        *free_defip = l3info.l3info_max_defip - l3info.l3info_occupied_defip;
    }

    if (free_lpm_blk != NULL) {
        *free_lpm_blk =
            l3info.l3info_max_lpm_block - l3info.l3info_used_lpm_block;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_esw_l3_route_max_ecmp_set
 * Purpose:
 *    Set the maximum ECMP paths allowed for a route
 *    for chips that supports ECMP
 * Parameters:
 *    unit - SOC device unit number
 *    max  - MAX number of paths for ECMP (number must be power of 2)
 * Returns:
 *      BCM_E_XXX
 * Note:
 *    There is a hardware limit on the number of ECMP paths,
 *    for bcm_ip_mask_length it is 32, BCM5665/50/73/74 is 8.
 *    The above parameter must be within the hardware limit.
 *    This function can be called before ECMP routes are added,
 *    normally at the beginning.  Once ECMP routes exist, cannot be reset.
 */
int
bcm_esw_l3_route_max_ecmp_set(int unit, int max)
{
    int rv = BCM_E_UNAVAIL;
    L3_INIT(unit);
    L3_LOCK(unit);

#if defined(BCM_XGS_SWITCH_SUPPORT)
    if (SOC_IS_LYNX(unit) || SOC_IS_TUCANA(unit) || SOC_IS_DRACO15(unit)) {
        rv = _bcm_xgs_max_ecmp_set(unit, max);
    }
#endif

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = bcm_xgs3_max_ecmp_set(unit, max);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_defip_max_ecmp_set(int unit, int max)
{
    return (bcm_esw_l3_route_max_ecmp_set(unit, max));
}

/*
 * Function:
 *    bcm_esw_l3_route_max_ecmp_get
 * Purpose:
 *    Get the maximum ECMP paths allowed for a route
 *    for chips that supports ECMP
 * Parameters:
 *    unit - SOC device unit number
 *    max  - (OUT)MAX number of paths for ECMP (number must be power of 2)
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_route_max_ecmp_get(int unit, int *max)
{
    int rv = BCM_E_UNAVAIL;
    L3_INIT(unit);

    if (NULL == max) {
        return (BCM_E_PARAM);
    }

    *max = 0;

    L3_LOCK(unit);
#if defined(BCM_XGS_SWITCH_SUPPORT)
    if (SOC_IS_LYNX(unit) || SOC_IS_TUCANA(unit) || SOC_IS_DRACO15(unit)) {
        rv = _bcm_xgs_max_ecmp_get(unit, max);
    }
#endif

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = bcm_xgs3_max_ecmp_get(unit, max);
    }
#endif   /* BCM_XGS3_SWITCH_SUPPORT */
    L3_UNLOCK(unit);
    return (rv);
}

/* to be deprecated */
int
bcm_esw_l3_defip_max_ecmp_get(int unit, int *max)
{
    return (bcm_esw_l3_route_max_ecmp_get(unit, max));
}

/*
 * Function:
 *      bcm_l3_cleanup
 * Purpose:
 *      Detach L3 function for a unit, clean up all tables and data structures
 * Parameters:
 *      unit - StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE - detach successful.
 *      BCM_E_XXX - detach failed.
 */

int
bcm_esw_l3_cleanup(int unit)
{
    int rv = BCM_E_NONE;

    if ((0 == l3_internal_initialized) || 
        (0 == _bcm_l3_bk_info[unit].l3_initialized)) {
        if (NULL != mbcm_driver[unit]) {
            rv = mbcm_driver[unit]->mbcm_l3_tables_cleanup(unit);
        }
        return rv;
    }

    L3_LOCK(unit);
    rv = bcm_esw_ipmc_detach(unit);
    if (BCM_FAILURE(rv)) {
        L3_UNLOCK(unit);
        return (rv);
    }
	
    /*no subport feature in Hurricane*/
    if(!SOC_IS_HURRICANE(unit)) {
	    rv = bcm_esw_subport_cleanup(unit);
	    if (BCM_FAILURE(rv)) {
	        L3_UNLOCK(unit);
	        return (rv);
	    }
    }/*SOC_IS_HURRICANE*/

    rv = bcm_esw_mpls_cleanup(unit);
    if (BCM_FAILURE(rv)) {
        L3_UNLOCK(unit);
        return (rv);
    }

    if(!SOC_IS_HURRICANE(unit)) {
	    rv = bcm_esw_wlan_detach(unit);
	    if (BCM_FAILURE(rv)) {
	        L3_UNLOCK(unit);
	        return (rv);
	    }
    }/*SOC_IS_HURRICANE*/

    if(!SOC_IS_HURRICANE(unit)) {
	    rv = bcm_esw_mim_detach(unit);
	    if (BCM_FAILURE(rv)) {
	        L3_UNLOCK(unit);
	        return (rv);
    }
    }

    if (0 == SOC_HW_ACCESS_DISABLE(unit)) {
        rv = bcm_esw_l3_route_delete_all(unit, NULL);
        if (BCM_FAILURE(rv)) {
            L3_UNLOCK(unit);
            return (rv);
        }
        rv = bcm_esw_l3_host_delete_all(unit, NULL);
        if (BCM_FAILURE(rv)) {
            L3_UNLOCK(unit);
            return (rv);
        }
        rv = bcm_esw_l3_intf_delete_all(unit);
        if (BCM_FAILURE(rv)) {
            L3_UNLOCK(unit);
            return (rv);
        }
    }

    rv = mbcm_driver[unit]->mbcm_l3_tables_cleanup(unit);
    _bcm_l3_bk_info[unit].l3_initialized = 0;
    L3_UNLOCK(unit);

    /* Destroy protection mutex. */
    if (NULL != _bcm_l3_bk_info[unit].lock) {
        sal_mutex_destroy(_bcm_l3_bk_info[unit].lock);
        _bcm_l3_bk_info[unit].lock = NULL;
    }

    return(rv);
}

/*
 * Function:
 *      bcm_esw_l3_enable_set
 * Purpose:
 *      Enable/disable L3 function for a unit without clearing any L3 tables
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      enable - 1 for enable, 0 for disable
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_l3_enable_set(int unit, int enable)
{
    L3_INIT(unit);
    L3_LOCK(unit);

    mbcm_driver[unit]->mbcm_l3_enable(unit, enable);

    L3_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_l3_info
 * Purpose:
 *      Get the status of hardware.
 * Parameters:
 *      unit - SOC device unit number
 *      l3_info - (OUT) Available hardware's L3 information.
 * Returns:
 *      BCM_E_XXX.
 */

int
bcm_esw_l3_info(int unit, bcm_l3_info_t *l3info)
{
    int rv;
    L3_INIT(unit);

    if (l3info == NULL) {
        return (BCM_E_PARAM);
    }
    L3_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_l3_info_get(unit, l3info);

    L3_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_untagged_update
 * Purpose:
 *      Reconcile L3 programming with VLAN tables' state.
 * Parameters:
 *      unit - SOC device unit number.
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *    This function has been deprecated.
 */

int
bcm_esw_l3_untagged_update(int unit)
{
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_l3_ingress_create
 * Purpose:
 *      Create an Ingress Interface object.
 * Parameters:
 *      unit    - (IN)  bcm device.
 *      flags   - (IN)  BCM_L3_INGRESS_REPLACE: replace existing.
 *                          BCM_L3_INGRESS_WITH_ID: intf argument is given.
 *                          BCM_L3_INGRESS_GLOBAL_ROUTE : 
 *                          BCM_L3_INGRESS_DSCP_TRUST : 
 *      ing_intf     - (IN) Ingress Interface information.
 *      intf_id    - (OUT) L3 Ingress interface id pointing to Ingress object.
 *                      This is an IN argument if either BCM_L3_INGRESS_REPLACE
 *                      or BCM_L3_INGRESS_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
*/

int 
bcm_esw_l3_ingress_create(int unit, bcm_l3_ingress_t *ing_intf, bcm_if_t *intf_id)
{
    int rv = BCM_E_UNAVAIL;
#if defined (BCM_TRIUMPH_SUPPORT)
    if (soc_feature(unit,soc_feature_l3_ingress_interface)) {
         L3_LOCK(unit);
         rv =  bcm_xgs3_l3_ingress_create(unit, ing_intf, intf_id);
         L3_UNLOCK(unit);
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_ingress_destroy
 * Purpose:
 *      Destroy an Ingress Interface object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf_id    - (IN) L3 Ingress interface id pointing to Ingress object.
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_esw_l3_ingress_destroy(int unit, bcm_if_t intf_id)
{
    int rv = BCM_E_UNAVAIL;
#if defined (BCM_TRIUMPH_SUPPORT)
    if (soc_feature(unit,soc_feature_l3_ingress_interface)) {
         L3_LOCK(unit);
         rv =  bcm_xgs3_l3_ingress_destroy(unit, intf_id);
         L3_UNLOCK(unit);
}
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_ingress_find
 * Purpose:
 *      Find an Ingress Interface object.     
 * Parameters:
 *      unit       - (IN) bcm device.
 *      ing_intf        - (IN) Ingress Interface information.
 *      intf_id       - (OUT) L3 Ingress interface id pointing to Ingress object.
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_esw_l3_ingress_find(int unit, bcm_l3_ingress_t *ing_intf, bcm_if_t *intf_id)
{
    int rv = BCM_E_UNAVAIL;
#if defined (BCM_TRIUMPH_SUPPORT)
    if (soc_feature(unit,soc_feature_l3_ingress_interface)) {
         L3_LOCK(unit);
         rv =  bcm_xgs3_l3_ingress_find(unit, ing_intf, intf_id);
         L3_UNLOCK(unit);
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_ingress_get
 * Purpose:
 *      Get an Ingress Interface object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf_id    - (IN) L3 Ingress interface id pointing to Ingress object.
 *      ing_intf  - (OUT) Ingress Interface information.
 * Returns:
 *      BCM_E_XXX
 */

int 
bcm_esw_l3_ingress_get(int unit, bcm_if_t intf_id, bcm_l3_ingress_t *ing_intf)
{
    int rv = BCM_E_UNAVAIL;
#if defined (BCM_TRIUMPH_SUPPORT)
    if (soc_feature(unit,soc_feature_l3_ingress_interface)) {
         L3_LOCK(unit);
         rv =  bcm_xgs3_l3_ingress_get(unit, intf_id, ing_intf);
         L3_UNLOCK(unit);
}
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_ingress_traverse
 * Purpose:
 *      Goes through ijgress interface objects table and runs the user callback
 *      function at each valid ingress objects entry passing back the
 *      information for that object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function. 
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_ingress_traverse(int unit, 
                           bcm_l3_ingress_traverse_cb trav_fn, void *user_data)
{
    int rv = BCM_E_UNAVAIL;
#if defined (BCM_TRIUMPH_SUPPORT)
    if (soc_feature(unit,soc_feature_l3_ingress_interface)) {
         L3_LOCK(unit);
         rv =  bcm_xgs3_l3_ingress_traverse(unit, trav_fn, user_data);
         L3_UNLOCK(unit);
}
#endif
    return rv;
}
#if defined(BCM_KATANA_SUPPORT)
/*
 * Function:
 *      _bcm_esw_l3_egress_stat_get_table_info
 * Description:
 *      Provides relevant flex table information(table-name,index with
 *      direction)  for given egress interface.
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      num_of_tables    - (OUT) Number of flex counter tables
 *      table_info       - (OUT) Flex counter tables information
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static 
bcm_error_t _bcm_esw_l3_egress_stat_get_table_info(
           int                        unit,
           bcm_if_t                   intf_id,
           uint32                     *num_of_tables,
           bcm_stat_flex_table_info_t *table_info)
{
    bcm_l3_egress_t egr_intf={0};
    int index=0;

    (*num_of_tables) = 0;
    if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
        return BCM_E_UNAVAIL;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_l3_egress_get(unit, intf_id, &egr_intf));
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_wlan) && egr_intf.encap_id > 0) {
        index = intf_id - BCM_XGS3_DVP_EGRESS_IDX_MIN ;
    } else
#endif
        {
           index = intf_id - BCM_XGS3_EGRESS_IDX_MIN ;
        }
    table_info[*num_of_tables].table=EGR_L3_NEXT_HOPm;
    table_info[*num_of_tables].index=index;
    table_info[*num_of_tables].direction=bcmStatFlexDirectionEgress;
    (*num_of_tables)++;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_l3_ingress_stat_get_table_info
 * Description:
 *      Provides relevant flex table information(table-name,index with
 *      direction)  for given ingress interface.
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a ingress L3 object.
 *      num_of_tables    - (OUT) Number of flex counter tables
 *      table_info       - (OUT) Flex counter tables information
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static 
bcm_error_t _bcm_esw_l3_ingress_stat_get_table_info(
            int                        unit,
            bcm_if_t                   intf_id,
            uint32                     *num_of_tables,
            bcm_stat_flex_table_info_t *table_info)
{
    bcm_l3_ingress_t ing_intf={0 };
    bcm_l3_intf_t    l3_intf={0};

    if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
        return BCM_E_UNAVAIL;
    }
    if (intf_id > BCM_VLAN_MAX) {
        BCM_IF_ERROR_RETURN(bcm_esw_l3_ingress_get(unit, intf_id, &ing_intf));
        /* Although above check should return error for invalid intf_id */
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SHR_BITGET(BCM_XGS3_L3_ING_IF_INUSE(unit), intf_id)) {
            return BCM_E_NOT_FOUND;
        }
#endif
    } else {
        /* Check whether it is valid vlan or not */
        l3_intf.l3a_vid=intf_id;
        BCM_IF_ERROR_RETURN(bcm_esw_l3_intf_find_vlan(unit,&l3_intf));
    }
    table_info[*num_of_tables].table=L3_IIFm;
    table_info[*num_of_tables].index=intf_id;
    table_info[*num_of_tables].direction=bcmStatFlexDirectionIngress;
    (*num_of_tables)++;
    return BCM_E_NONE;
}
#endif
/*
 * Function:
 *      bcm_esw_l3_egress_stat_attach
 * Description:
 *      Attach counters entries to the given L3 Egress interface
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_egress_stat_attach(
            int      unit, 
            bcm_if_t intf_id, 
            uint32   stat_counter_id)
{
#if defined(BCM_KATANA_SUPPORT)
    soc_mem_t                  table=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     pool_number=0;
    uint32                     base_index=0;
    bcm_stat_flex_mode_t       offset_mode=0;
    bcm_stat_object_t          object=bcmStatObjectIngPort;
    bcm_stat_group_mode_t      group_mode= bcmStatGroupModeSingle;
    uint32                     count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    _bcm_esw_stat_get_counter_id_info(
                  stat_counter_id,
                  &group_mode,&object,&offset_mode,&pool_number,&base_index);
    /* Validate object id first */
    if (!((object >= bcmStatObjectIngPort) &&
          (object <= bcmStatObjectEgrL3Intf))) {
           soc_cm_print("Invalid bcm_stat_object_t passed %d \n",object);
           return BCM_E_PARAM;
    }
    /* Validate group_mode */
    if(!((group_mode >= bcmStatGroupModeSingle) &&
         (group_mode <= bcmStatGroupModeDvpType))) {
          soc_cm_print("Invalid bcm_stat_group_mode_t passed %d \n",group_mode);
          return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_stat_flex_get_table_info(
                        object,&table,&direction));

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_egress_stat_get_table_info(
                        unit, intf_id,&num_of_tables,&table_info[0]));
    for (count=0; count < num_of_tables ; count++) {
         if ((table_info[count].direction == direction) &&
             (table_info[count].table == table) ) {
              if (direction == bcmStatFlexDirectionIngress) {
                  return _bcm_esw_stat_flex_attach_ingress_table_counters(
                         unit,
                         table_info[count].table,
                         table_info[count].index,
                         offset_mode,
                         base_index,
                         pool_number);
              } else {
                  return _bcm_esw_stat_flex_attach_egress_table_counters(
                         unit,
                         table_info[count].table,
                         table_info[count].index,
                         offset_mode,
                         base_index,
                         pool_number);
              } 
         }
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_l3_egress_stat_attach
 * Description:
 *      Detach  counters entries to the given L3 Egress interface. 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_egress_stat_detach(
            int      unit, 
            bcm_if_t intf_id)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    bcm_error_t                rv[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = 
                                  {BCM_E_FAIL};
    uint32                     flag[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = {0};

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_egress_stat_get_table_info(
                   unit, intf_id,&num_of_tables,&table_info[0]));

    for (count=0; count < num_of_tables ; count++) {
         if (table_info[count].direction == bcmStatFlexDirectionIngress) {
             rv[bcmStatFlexDirectionIngress]= 
                   _bcm_esw_stat_flex_detach_ingress_table_counters(
                        unit, table_info[count].table,table_info[count].index);
             flag[bcmStatFlexDirectionIngress] = 1;
         } else {
             rv[bcmStatFlexDirectionEgress] = 
                   _bcm_esw_stat_flex_detach_egress_table_counters(
                        unit, table_info[count].table, table_info[count].index);
             flag[bcmStatFlexDirectionIngress] = 1;
         }
    }
    if ((rv[bcmStatFlexDirectionIngress] == BCM_E_NONE) ||
        (rv[bcmStatFlexDirectionEgress] == BCM_E_NONE)) {
         return BCM_E_NONE;
    }
    if (flag[bcmStatFlexDirectionIngress] == 1) {
        return rv[bcmStatFlexDirectionIngress];
    }
    if (flag[bcmStatFlexDirectionEgress] == 1) {
        return rv[bcmStatFlexDirectionEgress];
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_l3_egress_stat_counter_get
 * Description:
 *      Get the specified counter statistic for a L3 egress interface 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (OUT) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_egress_stat_counter_get(
            int              unit, 
            bcm_if_t         intf_id, 
            bcm_l3_stat_t    stat, 
            uint32           num_entries, 
            uint32           *counter_indexes, 
            bcm_stat_value_t *counter_values)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     table_count=0;
    uint32                     index_count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     byte_flag=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    if ((stat == bcmL3StatOutPackets) ||
        (stat == bcmL3StatOutBytes)) {
         direction = bcmStatFlexDirectionEgress;
    } else {
         /* direction = bcmStatFlexDirectionIngress; */
         return BCM_E_PARAM;
    }
    if (stat == bcmL3StatOutPackets) {
        byte_flag=0;
    } else {
        byte_flag=1;
    }
    
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_egress_stat_get_table_info(
                        unit, intf_id,&num_of_tables,&table_info[0]));

    for (table_count=0; table_count < num_of_tables ; table_count++) {
         if (table_info[table_count].direction == direction) {
             for (index_count=0; index_count < num_entries ; index_count++) {
                  BCM_IF_ERROR_RETURN(_bcm_esw_stat_counter_get(
                                      unit,
                                      table_info[table_count].index,
                                      table_info[table_count].table,
                                      byte_flag,
                                      counter_indexes[index_count],
                                      &counter_values[index_count]));
             }
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_l3_egress_stat_counter_set
 * Description:
 *      Set the specified counter statistic for a L3 egress interface 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (IN) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 */
bcm_error_t bcm_esw_l3_egress_stat_counter_set(
            int              unit, 
            bcm_if_t         intf_id, 
            bcm_l3_stat_t    stat, 
            uint32           num_entries, 
            uint32           *counter_indexes, 
            bcm_stat_value_t *counter_values)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     table_count=0;
    uint32                     index_count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     byte_flag=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    if ((stat == bcmL3StatOutPackets) ||
        (stat == bcmL3StatOutBytes)) {
         direction = bcmStatFlexDirectionEgress;
    } else {
         /* direction = bcmStatFlexDirectionIngress; */
         return BCM_E_PARAM;
    }
    if (stat == bcmL3StatOutPackets) {
        byte_flag=0;
    } else {
        byte_flag=1;
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_egress_stat_get_table_info(
                        unit, intf_id,&num_of_tables,&table_info[0]));

    for (table_count=0; table_count < num_of_tables ; table_count++) {
         if (table_info[table_count].direction == direction) {
             for (index_count=0; index_count < num_entries ; index_count++) {
                  BCM_IF_ERROR_RETURN(_bcm_esw_stat_counter_set(
                                      unit,
                                      table_info[table_count].index,
                                      table_info[table_count].table,
                                      byte_flag,
                                      counter_indexes[index_count],
                                      &counter_values[index_count]));
            }
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_l3_egress_stat_id_get
 * Description:
 *      Get stat counter id associated with given egress interface id
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      Stat_counter_id  - (OUT) Stat Counter ID
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_egress_stat_id_get(
            int             unit,
            bcm_if_t        intf_id, 
            bcm_l3_stat_t   stat, 
            uint32          *stat_counter_id)
{
#if defined(BCM_KATANA_SUPPORT)
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32                     index=0;
    uint32                     num_stat_counter_ids=0;

    if ((stat == bcmL3StatInPackets) ||
        (stat == bcmL3StatInBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         direction = bcmStatFlexDirectionEgress;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_egress_stat_get_table_info(
                        unit,intf_id,&num_of_tables,&table_info[0]));
    for (index=0; index < num_of_tables ; index++) {
         if (table_info[index].direction == direction)
             return _bcm_esw_stat_flex_get_counter_id(
                             unit, 1, &table_info[index],
                             &num_stat_counter_ids,stat_counter_id);
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}


/*
 * Function:
 *      bcm_esw_l3_ingress_stat_attach
 * Description:
 *      Attach counters entries to the given L3 Ingress interface
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a Ingress L3 object.
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_ingress_stat_attach(
            int      unit, 
            bcm_if_t intf_id, 
            uint32   stat_counter_id)
{
#if defined(BCM_KATANA_SUPPORT)
    soc_mem_t                  table=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     pool_number=0;
    uint32                     base_index=0;
    bcm_stat_flex_mode_t       offset_mode=0;
    bcm_stat_object_t          object=bcmStatObjectIngPort;
    bcm_stat_group_mode_t      group_mode= bcmStatGroupModeSingle;
    uint32                     count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    _bcm_esw_stat_get_counter_id_info(
                  stat_counter_id,
                  &group_mode,&object,&offset_mode,&pool_number,&base_index);
    /* Validate object id first */
    if (!((object >= bcmStatObjectIngPort) &&
          (object <= bcmStatObjectEgrL3Intf))) {
           soc_cm_print("Invalid bcm_stat_object_t passed %d \n",object);
           return BCM_E_PARAM;
    }
    /* Validate group_mode */
    if(!((group_mode >= bcmStatGroupModeSingle) &&
         (group_mode <= bcmStatGroupModeDvpType))) {
          soc_cm_print("Invalid bcm_stat_group_mode_t passed %d \n",group_mode);
          return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_stat_flex_get_table_info(
                        object,&table,&direction));

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_ingress_stat_get_table_info(
                        unit, intf_id,&num_of_tables,&table_info[0]));
    for (count=0; count < num_of_tables ; count++) {
         if ((table_info[count].direction == direction) &&
             (table_info[count].table == table) ) {
              if (direction == bcmStatFlexDirectionIngress) {
                  return _bcm_esw_stat_flex_attach_ingress_table_counters(
                         unit,
                         table_info[count].table,
                         table_info[count].index,
                         offset_mode,
                         base_index,
                         pool_number);
              } else {
                  return _bcm_esw_stat_flex_attach_egress_table_counters(
                         unit,
                         table_info[count].table,
                         table_info[count].index,
                         offset_mode,
                         base_index,
                         pool_number);
              } 
         }
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_l3_ingress_stat_attach
 * Description:
 *      Detach  counters entries to the given L3 Ingress interface. 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

bcm_error_t bcm_esw_l3_ingress_stat_detach(
            int      unit, 
            bcm_if_t intf_id)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    bcm_error_t                rv[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = 
                                  {BCM_E_FAIL};
    uint32                     flag[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = {0};

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_ingress_stat_get_table_info(
                        unit, intf_id,&num_of_tables,&table_info[0]));

    for (count=0; count < num_of_tables ; count++) {
         if (table_info[count].direction == bcmStatFlexDirectionIngress) {
             rv[bcmStatFlexDirectionIngress]= 
                   _bcm_esw_stat_flex_detach_ingress_table_counters(
                        unit, table_info[count].table, table_info[count].index);
             flag[bcmStatFlexDirectionIngress] = 1;
         } else {
             rv[bcmStatFlexDirectionEgress] = 
                   _bcm_esw_stat_flex_detach_egress_table_counters(
                        unit, table_info[count].table, table_info[count].index);
             flag[bcmStatFlexDirectionIngress] = 1;
         }
    }
    if ((rv[bcmStatFlexDirectionIngress] == BCM_E_NONE) ||
        (rv[bcmStatFlexDirectionEgress] == BCM_E_NONE)) {
         return BCM_E_NONE;
    }
    if (flag[bcmStatFlexDirectionIngress] == 1) {
        return rv[bcmStatFlexDirectionIngress];
    }
    if (flag[bcmStatFlexDirectionEgress] == 1) {
        return rv[bcmStatFlexDirectionEgress];
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_l3_ingress_stat_counter_get
 * Description:
 *      Get the specified counter statistic for a L3 ingress interface 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a ingress L3 object.
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (OUT) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

bcm_error_t bcm_esw_l3_ingress_stat_counter_get(
            int              unit, 
            bcm_if_t         intf_id, 
            bcm_l3_stat_t    stat, 
            uint32           num_entries, 
            uint32           *counter_indexes, 
            bcm_stat_value_t *counter_values)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     table_count=0;
    uint32                     index_count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     byte_flag=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    if ((stat == bcmL3StatInPackets) ||
        (stat == bcmL3StatInBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         /* direction = bcmStatFlexDirectionEgress; */
         return BCM_E_PARAM;
    }
    if (stat == bcmL3StatInPackets) {
        byte_flag=0;
    } else {
        byte_flag=1;
    }
    
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_ingress_stat_get_table_info(
                        unit, intf_id,&num_of_tables,&table_info[0]));

    for (table_count=0; table_count < num_of_tables ; table_count++) {
        if (table_info[table_count].direction == direction) {
            for (index_count=0; index_count < num_entries ; index_count++) {
                 BCM_IF_ERROR_RETURN(_bcm_esw_stat_counter_get(
                                     unit,
                                     table_info[table_count].index,
                                     table_info[table_count].table,
                                     byte_flag,
                                     counter_indexes[index_count],
                                     &counter_values[index_count]));
            }
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}


/*
 * Function:
 *      bcm_esw_l3_ingress_stat_counter_set
 * Description:
 *      Set the specified counter statistic for a L3 ingress interface 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a egress L3 object.
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (IN) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 */

bcm_error_t bcm_esw_l3_ingress_stat_counter_set(
            int              unit, 
            bcm_if_t         intf_id, 
            bcm_l3_stat_t    stat, 
            uint32           num_entries, 
            uint32           *counter_indexes, 
            bcm_stat_value_t *counter_values)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     table_count=0;
    uint32                     index_count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     byte_flag=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    if ((stat == bcmL3StatInPackets) ||
        (stat == bcmL3StatInBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         /* direction = bcmStatFlexDirectionEgress; */
         return BCM_E_PARAM;
    }
    if (stat == bcmL3StatInPackets) {
        byte_flag=0;
    } else {
        byte_flag=1;
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_ingress_stat_get_table_info(
                        unit, intf_id,&num_of_tables,&table_info[0]));

    for (table_count=0; table_count < num_of_tables ; table_count++) {
         if (table_info[table_count].direction == direction) {
             for (index_count=0; index_count < num_entries ; index_count++) {
                  BCM_IF_ERROR_RETURN(_bcm_esw_stat_counter_set(
                                      unit,
                                      table_info[table_count].index,
                                      table_info[table_count].table,
                                      byte_flag,
                                      counter_indexes[index_count],
                                      &counter_values[index_count]));
             }
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_l3_egress_stat_id_get
 * Description:
 *      Get stat counter id associated with given ingress interface id
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      intf_id          - (IN) Interface id of a ingress L3 object.
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      Stat_counter_id  - (OUT) Stat Counter ID
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_ingress_stat_id_get(
            int           unit,
            bcm_if_t      intf_id,
            bcm_l3_stat_t stat, 
            uint32        *stat_counter_id)
{
#if defined(BCM_KATANA_SUPPORT)
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32                     index=0;
    uint32                     num_stat_counter_ids=0;

    if ((stat == bcmL3StatInPackets) ||
        (stat == bcmL3StatInBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         direction = bcmStatFlexDirectionEgress;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_ingress_stat_get_table_info(
                        unit,intf_id,&num_of_tables,&table_info[0]));
    for (index=0; index < num_of_tables ; index++) {
         if (table_info[index].direction == direction)
             return _bcm_esw_stat_flex_get_counter_id(
                            unit, 1, &table_info[index],
                            &num_stat_counter_ids,stat_counter_id);
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}




/*
 * Function:
 *      bcm_esw_l3_egress_create
 * Purpose:
 *      Create an Egress forwarding object.
 * Parameters:
 *      unit    - (IN)  bcm device.
 *      flags   - (IN)  BCM_L3_REPLACE: replace existing.
 *                      BCM_L3_WITH_ID: intf argument is given.
 *      egr     - (IN) Egress forwarding destination.
 *      intf    - (OUT) L3 interface id pointing to Egress object.
 *                      This is an IN argument if either BCM_L3_REPLACE
 *                      or BCM_L3_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_create(int unit, uint32 flags, bcm_l3_egress_t *egr, 
                         bcm_if_t *intf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        bcm_l3_egress_t egr_local;

        /* Input parameters check. */
        if ((NULL == egr) || (NULL == intf)) {
            return (BCM_E_PARAM);
        }

        /* Copy provided structure to local so it can be modified. */
        sal_memcpy(&egr_local, egr, sizeof(bcm_l3_egress_t));

        if (egr_local.vlan >= BCM_VLAN_INVALID) {
            return (BCM_E_PARAM);
        }
        if (BCM_GPORT_IS_BLACK_HOLE(egr_local.port)) {
             *intf = BCM_XGS3_EGRESS_IDX_MIN + BCM_XGS3_L3_BLACK_HOLE_NH_IDX;
             return BCM_E_NONE;
        } else if (BCM_GPORT_IS_SET(egr_local.port)) {
            if (BCM_GPORT_IS_WLAN_PORT(egr_local.port)) {
                /* Resolve the WLAN gport and store the VP in egr->encap_id */
                rv = _bcm_esw_gport_resolve(unit, egr_local.port, 
                                            &(egr_local.module), 
                                            &(egr_local.port), 
                                            &(egr_local.trunk), 
                                            &(egr_local.encap_id));  
            } else {
                rv = _bcm_esw_l3_gport_resolve(unit, egr_local.port, 
                                               &(egr_local.port), 
                                               &(egr_local.module), 
                                               &(egr_local.trunk), 
                                               &(egr_local.flags));
            }
            BCM_IF_ERROR_RETURN(rv);
        } else {
            PORT_DUALMODID_VALID(unit, egr_local.port);
        }
        L3_LOCK(unit);
        if (soc_feature(unit, soc_feature_mpls)) {
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
           rv = bcm_tr_mpls_lock(unit);
           if (BCM_FAILURE(rv)) {
               return rv;
           }
#endif /* BCM_TRIUMPH_SUPPORT &&  BCM_MPLS_SUPPORT */
        }
         rv = bcm_xgs3_l3_egress_create(unit, flags, &egr_local, intf);
         if (soc_feature(unit, soc_feature_mpls)) {
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
              bcm_tr_mpls_unlock(unit);
#endif /* BCM_TRIUMPH_SUPPORT &&  BCM_MPLS_SUPPORT */
         }
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_egress_destroy
 * Purpose:
 *      Destroy an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN) L3 interface id pointing to Egress object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_destroy(int unit, bcm_if_t intf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        if (soc_feature(unit, soc_feature_mpls)) {
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
           BCM_IF_ERROR_RETURN(bcm_tr_mpls_lock(unit));
#endif /* BCM_TRIUMPH_SUPPORT &&  BCM_MPLS_SUPPORT */
        }
         rv = bcm_xgs3_l3_egress_destroy(unit, intf);
         if (soc_feature(unit, soc_feature_mpls)) {
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
             bcm_tr_mpls_unlock(unit);
#endif /* BCM_TRIUMPH_SUPPORT &&  BCM_MPLS_SUPPORT */
         }
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_get
 * Purpose:
 *      Get an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN) L3 interface id pointing to Egress object.
 *      egr     - (OUT) Egress forwarding destination.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_get (int unit, bcm_if_t intf, bcm_l3_egress_t *egr) 
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)

    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_get (unit, intf, egr);
        L3_UNLOCK(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }


        rv = _bcm_esw_l3_gport_construct(unit, egr->port, egr->module, 
                                         egr->trunk, egr->flags, &(egr->port));

        if ((intf - BCM_XGS3_EGRESS_IDX_MIN) == BCM_XGS3_L3_BLACK_HOLE_NH_IDX) {
             egr->port = BCM_GPORT_BLACK_HOLE;
        }
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_find
 * Purpose:
 *      Find an egress forwarding object.     
 * Parameters:
 *      unit       - (IN) bcm device.
 *      egr        - (IN) Egress object properties to match.  
 *      intf       - (OUT) L3 interface id pointing to egress object
 *                         if found. 
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_find(int unit, bcm_l3_egress_t *egr, 
                                 bcm_if_t *intf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        bcm_l3_egress_t egr_local;

        /* Input parameters check. */
        if ((NULL == egr) || (NULL == intf)) {
            return (BCM_E_PARAM);
        }

        /* Copy provided structure to local so it can be modified. */
        sal_memcpy(&egr_local, egr, sizeof(bcm_l3_egress_t));

        if (BCM_GPORT_IS_SET(egr_local.port)) {
            if (BCM_GPORT_IS_WLAN_PORT(egr_local.port)) {
                /* Resolve the WLAN gport and store the VP in egr->encap_id */
                rv = _bcm_esw_gport_resolve(unit, egr_local.port, 
                                            &(egr_local.module), 
                                            &(egr_local.port), 
                                            &(egr_local.trunk), 
                                            &(egr_local.encap_id));  
            } else {
                rv = _bcm_esw_l3_gport_resolve(unit, egr_local.port, 
                                               &(egr_local.port), 
                                               &(egr_local.module), 
                                               &(egr_local.trunk), 
                                               &(egr_local.flags));
            }
            BCM_IF_ERROR_RETURN(rv); 
        } else {
            PORT_DUALMODID_VALID(unit, egr_local.port);
        }
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_find(unit, &egr_local, intf);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_egress_traverse
 * Purpose:
 *      Goes through egress objects table and runs the user callback
 *      function at each valid egress objects entry passing back the
 *      information for that object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function. 
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_traverse(int unit, 
                           bcm_l3_egress_traverse_cb trav_fn, void *user_data)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_traverse(unit, trav_fn, user_data);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_multipath_max_create 
 * Purpose:
 *      Create an Egress Multipath forwarding object with specified path-width
 * Parameters:
 *      unit       - (IN) bcm device.
 *      flags      - (IN) BCM_L3_REPLACE: replace existing.
 *                        BCM_L3_WITH_ID: intf argument is given.
 *      max_paths - (IN) configurable path-width
 *      intf_count - (IN) Number of elements in intf_array.
 *      intf_array - (IN) Array of Egress forwarding objects.
 *      mpintf     - (OUT) L3 interface id pointing to Egress multipath object.
 *                         This is an IN argument if either BCM_L3_REPLACE
 *                          or BCM_L3_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_egress_multipath_max_create(int unit, uint32 flags, int max_paths,
                                   int intf_count, bcm_if_t *intf_array, bcm_if_t *mpintf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_TRX(unit) && soc_feature(unit, soc_feature_l3)) {
         L3_LOCK(unit);
         rv = bcm_xgs3_l3_egress_multipath_max_create(unit, flags, max_paths, intf_count,
                                                              intf_array, mpintf);
         L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_multipath_create 
 * Purpose:
 *      Create an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      flags      - (IN) BCM_L3_REPLACE: replace existing.
 *                        BCM_L3_WITH_ID: intf argument is given.
 *      intf_count - (IN) Number of elements in intf_array.
 *      intf_array - (IN) Array of Egress forwarding objects.
 *      mpintf     - (OUT) L3 interface id pointing to Egress multipath object.
 *                         This is an IN argument if either BCM_L3_REPLACE
 *                          or BCM_L3_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_multipath_create(int unit, uint32 flags, int intf_count,
                                   bcm_if_t *intf_array, bcm_if_t *mpintf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_multipath_create(unit, flags, intf_count,
                                                   intf_array, mpintf);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_multipath_destroy
 * Purpose:
 *      Destroy an Egress Multipath forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) L3 interface id pointing to Egress multipath object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_multipath_destroy(int unit, bcm_if_t mpintf) 
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);

        rv = bcm_xgs3_l3_egress_multipath_destroy(unit, mpintf);

        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_multipath_get
 * Purpose:
 *      Get an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      mpintf     - (IN) L3 interface id pointing to Egress multipath object.
 *      intf_size  - (IN) Size of allocated entries in intf_array.
 *      intf_array - (OUT) Array of Egress forwarding objects.
 *      intf_count - (OUT) Number of entries of intf_count actually filled in.
 *                      This will be a value less than or equal to the value.
 *                      passed in as intf_size unless intf_size is 0.  If
 *                      intf_size is 0 then intf_array is ignored and
 *                      intf_count is filled in with the number of entries
 *                      that would have been filled into intf_array if
 *                      intf_size was arbitrarily large.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_multipath_get(int unit, bcm_if_t mpintf, int intf_size,
                                bcm_if_t *intf_array, int *intf_count)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_multipath_get(unit, mpintf, intf_size,
                                                intf_array, intf_count);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_multipath_add
 * Purpose:
 *      Add an Egress forwarding object to an Egress Multipath 
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) L3 interface id pointing to Egress multipath object.
 *      intf    - (IN) L3 interface id pointing to Egress forwarding object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_multipath_add(int unit, bcm_if_t mpintf, bcm_if_t intf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_multipath_add(unit, mpintf, intf);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_egress_multipath_delete
 * Purpose:
 *      Delete an Egress forwarding object to an Egress Multipath 
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) L3 interface id pointing to Egress multipath object
 *      intf    - (IN) L3 interface id pointing to Egress forwarding object
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_multipath_delete(int unit, bcm_if_t mpintf, bcm_if_t intf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_multipath_delete(unit, mpintf, intf);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_egress_multipath_find
 * Purpose:
 *      Find an egress multipath forwarding object.     
 * Parameters:
 *      unit       - (IN) bcm device.
 *      intf_count - (IN) Number of elements in intf_array. 
 *      intf_array - (IN) Array of egress forwarding objects.  
 *      intf       - (OUT) L3 interface id pointing to egress multipath object
 *                         if found. 
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_multipath_find(int unit, int intf_count, bcm_if_t
                                 *intf_array, bcm_if_t *mpintf)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_multipath_find(unit, intf_count,
                                                intf_array, mpintf);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}


/*
 * Function:
 *      bcm_esw_l3_egress_multipath_traverse
 * Purpose:
 *      Goes through multipath egress objects table and runs the user callback
 *      function at each multipath egress objects entry passing back the
 *      information for that multipath object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function. 
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_l3_egress_multipath_traverse(int unit, 
                          bcm_l3_egress_multipath_traverse_cb trav_fn,
                          void *user_data)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit) && soc_feature(unit, soc_feature_l3)) {
        L3_LOCK(unit);
        rv = bcm_xgs3_l3_egress_multipath_traverse(unit, trav_fn, user_data);
        L3_UNLOCK(unit);
    }
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_ip6_prefix_map_get
 * Purpose:
 *      Get a list of IPv6 96 bit prefixes which are mapped to ipv4 lookup
 *      space.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      map_size   - (IN) Size of allocated entries in ip6_array.
 *      ip6_array  - (OUT) Array of mapped prefixes.
 *      ip6_count  - (OUT) Number of entries of ip6_array actually filled in.
 *                      This will be a value less than or equal to the value.
 *                      passed in as map_size unless map_size is 0.  If
 *                      map_size is 0 then ip6_array is ignored and
 *                      ip6_count is filled in with the number of entries
 *                      that would have been filled into ip6_array if
 *                      map_size was arbitrarily large.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_ip6_prefix_map_get(int unit, int map_size, 
                              bcm_ip6_t *ip6_array, int *ip6_count)
{
    int rv = BCM_E_UNAVAIL;

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, BCM_L3_IP6)) {
        return (BCM_E_UNAVAIL);
    }
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    L3_LOCK(unit);
    rv = bcm_xgs3_l3_ip6_prefix_map_get(unit, map_size, 
                                          ip6_array, ip6_count);
    L3_UNLOCK(unit);
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return rv;
}
/*
 * Function:
 *      bcm_esw_l3_ip6_prefix_map_add
 * Purpose:
 *      Create an IPv6 prefix map into IPv4 entry. In case Ipv6 traffic
 *      destination or source IP address matches upper 96 bits of
 *      translation entry. traffic will be routed/switched  based on
 *      lower 32 bits of destination/source IP address treated as IPv4 address.
 * Parameters:
 *      unit     - (IN)  bcm device.
 *      ip6_addr - (IN)  New IPv6 translation address.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_ip6_prefix_map_add(int unit, bcm_ip6_t ip6_addr) 
{
    int rv = BCM_E_UNAVAIL;

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, BCM_L3_IP6)) {
        return (BCM_E_UNAVAIL);
    }
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    L3_LOCK(unit);
    rv = bcm_xgs3_l3_ip6_prefix_map_add(unit, ip6_addr);
    L3_UNLOCK(unit);
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_ip6_prefix_map_delete
 * Purpose:
 *      Destroy an IPv6 prefix map entry.
 * Parameters:
 *      unit     - (IN)  bcm device.
 *      ip6_addr - (IN)  IPv6 translation address.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_ip6_prefix_map_delete(int unit, bcm_ip6_t ip6_addr)
{
    int rv = BCM_E_UNAVAIL;

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, BCM_L3_IP6)) {
        return (BCM_E_UNAVAIL);
    }
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    L3_LOCK(unit);
    rv = bcm_xgs3_l3_ip6_prefix_map_delete(unit, ip6_addr);
    L3_UNLOCK(unit);
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_ip6_prefix_map_delete_all
 * Purpose:
 *      Flush all IPv6 prefix maps.
 * Parameters:
 *      unit     - (IN)  bcm device.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_l3_ip6_prefix_map_delete_all(int unit)
{
    int rv = BCM_E_UNAVAIL;

    /*  Check that device supports ipv6. */
    if (BCM_L3_NO_IP6_SUPPORT(unit, BCM_L3_IP6)) {
        return (BCM_E_UNAVAIL);
    }

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    L3_LOCK(unit);
    rv = bcm_xgs3_l3_ip6_prefix_map_delete_all(unit);
    L3_UNLOCK(unit);
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      bcm_esw_l3_vrrp_add
 * Purpose:
 *      Add VRID for the given VSI. Adding a VRID using this API means
 *      the physical node has become the master for the virtual router
 * Parameters:
 *      unit - (IN) Unit number.
 *      vlan - (IN) VLAN/VSI
 *      vrid - (IN) VRID - Virtual router ID to be added
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_l3_vrrp_add(int unit, bcm_vlan_t vlan, uint32 vrid)
{
    bcm_l2_addr_t l2addr;            /* Layer 2 address for interface. */
    int rv;                          /* Operation return status.       */
    bcm_mac_t mac = {0x00, 0x00, 0x5e, 0x00, 0x01, 0x00};/* Mac address.*/

    if (!BCM_VLAN_VALID(vlan)) {
        return (BCM_E_PARAM);
    }

    if (vrid > 0xff) {
        return (BCM_E_PARAM);
    }
    mac[5] = vrid;

    /* Set l2 address info. */
    bcm_l2_addr_t_init(&l2addr, mac, vlan);

#if defined(BCM_MIRAGE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_routing_mirage)) {
        /* Set l2 entry flags. */
        l2addr.flags = BCM_L2_STATIC | BCM_L2_REPLACE_DYNAMIC;

        /* Set l2 entry port to CPU port. */
        l2addr.port = BCM_MIRAGE_L3_PORT;
    } else 
#endif /* BCM_MIRAGE_SUPPORT */
    {
        /* Set l2 entry flags. */
        l2addr.flags = BCM_L2_L3LOOKUP | BCM_L2_STATIC | BCM_L2_REPLACE_DYNAMIC;

        /* Set l2 entry port to CPU port. */
        l2addr.port = CMIC_PORT(unit); 
    }

    /* Set l2 entry module id to local module. */
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &l2addr.modid));

    /* Overwrite existing entry if any. */
    bcm_esw_l2_addr_delete(unit, mac, vlan);

    /* Add entry to l2 table. */
    rv = bcm_esw_l2_addr_add(unit, &l2addr);

    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_vrrp_delete
 * Purpose:
 *      Delete VRID for a particulat VLAN/VSI
 * Parameters:
 *      unit - (IN) Unit number.
 *      vlan - (IN) VLAN/VSI
 *      vrid - (IN) VRID - Virtual router ID to be deleted
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_l3_vrrp_delete(int unit, bcm_vlan_t vlan, uint32 vrid)
{
    bcm_l2_addr_t l2addr;            /* Layer 2 address for interface. */
    int rv;                          /* Operation return status.       */
    bcm_mac_t mac = {0x00, 0x00, 0x5e, 0x00, 0x01, 0x00};/* Mac address.*/

    if (!BCM_VLAN_VALID(vlan)) {
        return (BCM_E_PARAM);
    }

    if (vrid > 0xff) {
        return (BCM_E_PARAM);
    }
    mac[5] = vrid;

    /* Set l2 address info. */
    bcm_l2_addr_t_init(&l2addr, mac, vlan);

    /* Overwrite existing entry if any. */
    rv = bcm_esw_l2_addr_delete(unit, mac, vlan);
    return rv;
}

/*
 * Function:
 *      bcm_esw_l3_vrrp_delete_all
 * Purpose:
 *      Delete all the VRIDs for a particular VLAN/VSI
 * Parameters:
 *      unit - (IN) Unit number.
 *      vlan - (IN) VLAN/VSI
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_l3_vrrp_delete_all(int unit, bcm_vlan_t vlan)
{
    int idx;     /* vr id iteration index.   */ 
    int rv;      /* Operation return status. */

    if (!BCM_VLAN_VALID(vlan)) {
        return (BCM_E_PARAM);
    }

    for (idx = 0; idx < 0x100; idx++) {
        rv = bcm_esw_l3_vrrp_delete(unit, vlan, idx);
        if (BCM_FAILURE(rv) && (BCM_E_NOT_FOUND != rv)) {
            return (rv);
        }
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_esw_l3_vrrp_get
 * Purpose:
 *      Get all the VRIDs for which the physical node is master for
 *      the virtual routers on the given VLAN/VSI
 * Parameters:
 *      unit - (IN) Unit number.
 *      vlan - (IN) VLAN/VSI
 *      alloc_size - (IN) Size of vrid_array
 *      vrid_array - (OUT) Pointer to the array to which the VRIDs will be copied
 *      count - (OUT) Number of VRIDs copied
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_l3_vrrp_get(int unit, bcm_vlan_t vlan, int alloc_size, 
                    int *vrid_array, int *count)
{
    bcm_l2_addr_t l2addr;             /* Layer 2 address for interface. */
    int idx;                          /* vr id iteration index.         */ 
    int tmp_cnt;                      /* Valie entry count.             */ 
    int rv;                           /* Operation return status.       */
    bcm_mac_t mac = {0x00, 0x00, 0x5e, 0x00, 0x01, 0x00};/* Mac address.*/

    if (!BCM_VLAN_VALID(vlan)) {
        return (BCM_E_PARAM);
    }

    if (NULL == count) {
        return (BCM_E_PARAM);
    }

    tmp_cnt = 0;
    for (idx = 0; idx < 0x100; idx++) {
        if ( alloc_size > tmp_cnt) {
            mac[5] = idx;

            /* Set l2 address info. */
            bcm_l2_addr_t_init(&l2addr, mac, vlan);

            /* Overwrite existing entry if any. */
            rv = bcm_esw_l2_addr_get(unit, mac, vlan, &l2addr);
            if (BCM_SUCCESS(rv)) {
                if (NULL != vrid_array) {
                    vrid_array[tmp_cnt] = idx;  
                }
                tmp_cnt++;
            }
        }
    }
    *count = tmp_cnt;
    return (BCM_E_NONE);
}


#if defined(BCM_TRIUMPH2_SUPPORT)
#define BCM_VRF_STAT_VALID(unit, vrf) \
    if (!soc_feature(unit, soc_feature_gport_service_counters)) { \
        return BCM_E_UNAVAIL; \
    } else if (((vrf) > SOC_VRF_MAX(unit)) || \
               ((vrf) < BCM_L3_VRF_DEFAULT)) { return (BCM_E_PARAM); }

STATIC int
_bcm_vrf_flex_stat_hw_index_set(int unit, _bcm_flex_stat_handle_t fsh,
                                int fs_idx, void *cookie)
{
    uint32 handle = _BCM_FLEX_STAT_HANDLE_WORD_GET(fsh, 0);
    COMPILER_REFERENCE(cookie);
    if (SOC_MEM_FIELD_VALID(unit, VRFm, USE_SERVICE_CTR_IDXf)) {
         soc_mem_field32_modify(unit, VRFm, handle,
                                  USE_SERVICE_CTR_IDXf, (fs_idx!=0) ? 1:0);
    }
    return soc_mem_field32_modify(unit, VRFm, handle,
                                  SERVICE_CTR_IDXf, fs_idx);
}

STATIC _bcm_flex_stat_t
_bcm_esw_l3_vrf_stat_to_flex_stat(bcm_l3_vrf_stat_t stat)
{
    _bcm_flex_stat_t flex_stat;

    switch (stat) {
    case bcmL3VrfStatIngressPackets:
        flex_stat = _bcmFlexStatIngressPackets;
        break;
    case bcmL3VrfStatIngressBytes:
        flex_stat = _bcmFlexStatIngressBytes;
        break;
    default:
        flex_stat = _bcmFlexStatNum;
    }

    return flex_stat;
}

/* Requires "idx" variable */
#define BCM_VRF_STAT_ARRAY_VALID(unit, nstat, value_arr) \
    for (idx = 0; idx < nstat; idx++) { \
        if (NULL == value_arr + idx) { \
            return (BCM_E_PARAM); \
        } \
    }

STATIC int
_bcm_l3_vrf_stat_array_convert(int unit, int nstat,
                               bcm_l3_vrf_stat_t *stat_arr, 
                               _bcm_flex_stat_t *fs_arr)
{
    int idx;

    if ((nstat <= 0) || (nstat > _bcmFlexStatEgressPackets)) {
        /* VRF only has ingress support, so the number of ingress types
         * is the first egress type. */
        return BCM_E_PARAM;
    }

    for (idx = 0; idx < nstat; idx++) {
        if (NULL == stat_arr + idx) {
            return (BCM_E_PARAM);
        }
        fs_arr[idx] = _bcm_esw_l3_vrf_stat_to_flex_stat(stat_arr[idx]);
    }
    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_KATANA_SUPPORT)
/*
 * Function:
 *      _bcm_esw_l3_vrf_stat_get_table_info
 * Description:
 *      Provides relevant flex table information(table-name,index with
 *      direction)  for given VRF(Virtual Router Interface) id.
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vrf              - (IN) VRF(Virtual Router Interface) id
 *      num_of_tables    - (OUT) Number of flex counter tables
 *      table_info       - (OUT) Flex counter tables information
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static 
bcm_error_t _bcm_esw_l3_vrf_stat_get_table_info(
            int                        unit,
            bcm_vrf_t                  vrf,
            uint32                     *num_of_tables,
            bcm_stat_flex_table_info_t *table_info)
{
    (*num_of_tables)=0;

    if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
        return BCM_E_UNAVAIL;
    }
#if defined(BCM_TRIUMPH2_SUPPORT)
    L3_INIT(unit);
    if ((vrf > SOC_VRF_MAX(unit)) || (vrf < BCM_L3_VRF_DEFAULT)) {
         return (BCM_E_PARAM);
    }
    table_info[*num_of_tables].table=VRFm;
    table_info[*num_of_tables].index=vrf;
    table_info[*num_of_tables].direction=bcmStatFlexDirectionIngress;
    (*num_of_tables)++;
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}
#endif

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_attach
 * Description:
 *      Attach counters entries to the given  VRF 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vrf              - (IN) Virtual router instance
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

bcm_error_t bcm_esw_l3_vrf_stat_attach(
            int       unit, 
            bcm_vrf_t vrf, 
            uint32    stat_counter_id)
{
#if defined(BCM_KATANA_SUPPORT)
    soc_mem_t                  table=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     pool_number=0;
    uint32                     base_index=0;
    bcm_stat_flex_mode_t       offset_mode=0;
    bcm_stat_object_t          object=bcmStatObjectIngPort;
    bcm_stat_group_mode_t      group_mode= bcmStatGroupModeSingle;
    uint32                     count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    _bcm_esw_stat_get_counter_id_info(
                  stat_counter_id,
                  &group_mode,&object,&offset_mode,&pool_number,&base_index);
    /* Validate object id first */
    if (!((object >= bcmStatObjectIngPort) &&
          (object <= bcmStatObjectEgrL3Intf))) {
           soc_cm_print("Invalid bcm_stat_object_t passed %d \n",object);
           return BCM_E_PARAM;
    }
    /* Validate group_mode */
    if(!((group_mode >= bcmStatGroupModeSingle) &&
         (group_mode <= bcmStatGroupModeDvpType))) {
          soc_cm_print("Invalid bcm_stat_group_mode_t passed %d \n",group_mode);
          return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_stat_flex_get_table_info(
                        object,&table,&direction));

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_vrf_stat_get_table_info(
                        unit, vrf,&num_of_tables,&table_info[0]));
    for (count=0; count < num_of_tables ; count++) {
         if ((table_info[count].direction == direction) &&
             (table_info[count].table == table) ) {
              if (direction == bcmStatFlexDirectionIngress) {
                  return _bcm_esw_stat_flex_attach_ingress_table_counters(
                         unit,
                         table_info[count].table,
                         table_info[count].index,
                         offset_mode,
                         base_index,
                         pool_number);
              } else {
                  return _bcm_esw_stat_flex_attach_egress_table_counters(
                         unit,
                         table_info[count].table,
                         table_info[count].index,
                         offset_mode,
                         base_index,
                         pool_number);
              } 
         }
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_detach
 * Description:
 *      Detach   counters entries to the given VRF
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vrf              - (IN) Virtual router instance
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_vrf_stat_detach(
            int       unit, 
            bcm_vrf_t vrf)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    bcm_error_t                rv[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = 
                                  {BCM_E_FAIL};
    uint32                     flag[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = {0};

    BCM_IF_ERROR_RETURN(_bcm_esw_l3_vrf_stat_get_table_info(
                        unit, vrf,&num_of_tables,&table_info[0]));

    for (count=0; count < num_of_tables ; count++) {
         if (table_info[count].direction == bcmStatFlexDirectionIngress) {
             rv[bcmStatFlexDirectionIngress]= 
                   _bcm_esw_stat_flex_detach_ingress_table_counters(
                        unit, table_info[count].table, table_info[count].index);
             flag[bcmStatFlexDirectionIngress] = 1;
         } else {
             rv[bcmStatFlexDirectionEgress] = 
                   _bcm_esw_stat_flex_detach_egress_table_counters(
                        unit, table_info[count].table, table_info[count].index);
             flag[bcmStatFlexDirectionIngress] = 1;
         }
    }
    if ((rv[bcmStatFlexDirectionIngress] == BCM_E_NONE) ||
        (rv[bcmStatFlexDirectionEgress] == BCM_E_NONE)) {
         return BCM_E_NONE;
    }
    if (flag[bcmStatFlexDirectionIngress] == 1) {
        return rv[bcmStatFlexDirectionIngress];
    }
    if (flag[bcmStatFlexDirectionEgress] == 1) {
        return rv[bcmStatFlexDirectionEgress];
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_counter_get
 * Description:
 * Get L3 VRF counter value for specified VRF statistic type 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vrf              - (IN) Virtual router instance
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (OUT) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_vrf_stat_counter_get(
            int               unit, 
            bcm_vrf_t         vrf, 
            bcm_l3_vrf_stat_t stat, 
            uint32            num_entries, 
            uint32            *counter_indexes, 
            bcm_stat_value_t  *counter_values)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     table_count=0;
    uint32                     index_count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     byte_flag=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    if ((stat == bcmL3VrfStatIngressPackets) ||
        (stat == bcmL3VrfStatIngressBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         /* direction = bcmStatFlexDirectionEgress; */
         return BCM_E_PARAM;
    }
    if (stat == bcmL3VrfStatIngressPackets) {
        byte_flag=0;
    } else {
        byte_flag=1;
    }
    
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_vrf_stat_get_table_info(
                        unit, vrf,&num_of_tables,&table_info[0]));

    for (table_count=0; table_count < num_of_tables ; table_count++) {
         if (table_info[table_count].direction == direction) {
             for (index_count=0; index_count < num_entries ; index_count++) {
                  BCM_IF_ERROR_RETURN(_bcm_esw_stat_counter_get(
                                      unit,
                                      table_info[table_count].index,
                                      table_info[table_count].table,
                                      byte_flag,
                                      counter_indexes[index_count],
                                      &counter_values[index_count]));
             }
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}


/*
 * Function:
 *      bcm_esw_l3_vrf_stat_counter_set
 * Description:
 *      Set L3 VRF counter value for specified VRF statistic type 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vrf              - (IN) Virtual router instance
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (OUT) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 */
bcm_error_t bcm_esw_l3_vrf_stat_counter_set(
            int               unit, 
            bcm_vrf_t         vrf, 
            bcm_l3_vrf_stat_t stat, 
            uint32            num_entries, 
            uint32            *counter_indexes, 
            bcm_stat_value_t  *counter_values)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     table_count=0;
    uint32                     index_count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     byte_flag=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];

    if ((stat == bcmL3VrfStatIngressPackets) ||
        (stat == bcmL3VrfStatIngressBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         /* direction = bcmStatFlexDirectionEgress; */
         return BCM_E_PARAM;
    }
    if (stat == bcmL3VrfStatIngressPackets) {
        byte_flag=0;
    } else {
        byte_flag=1;
    }
    
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_vrf_stat_get_table_info(
                        unit, vrf,&num_of_tables,&table_info[0]));

    for (table_count=0; table_count < num_of_tables ; table_count++) {
         if (table_info[table_count].direction == direction) {
             for (index_count=0; index_count < num_entries ; index_count++) {
                  BCM_IF_ERROR_RETURN(_bcm_esw_stat_counter_set(
                                      unit,
                                      table_info[table_count].index,
                                      table_info[table_count].table,
                                      byte_flag,
                                      counter_indexes[index_count],
                                      &counter_values[index_count]));
             }
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      bcm_esw_l3_vrf_stat_id_get
 * Description:
 *      Get stat counter id associated with given VRF
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      vrf              - (IN) Virtual router instance
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      Stat_counter_id  - (OUT) Stat Counter ID
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_l3_vrf_stat_id_get(
            int               unit,
            bcm_vrf_t         vrf, 
            bcm_l3_vrf_stat_t stat, 
            uint32            *stat_counter_id)
{
#if defined(BCM_KATANA_SUPPORT)
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32                     index=0;
    uint32                     num_stat_counter_ids=0;

    if ((stat == bcmL3VrfStatIngressPackets) ||
        (stat == bcmL3VrfStatIngressBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         direction = bcmStatFlexDirectionEgress;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_vrf_stat_get_table_info(
                        unit,vrf,&num_of_tables,&table_info[0]));
    for (index=0; index < num_of_tables ; index++) {
         if (table_info[index].direction == direction)
             return _bcm_esw_stat_flex_get_counter_id(
                                  unit, 1, &table_info[index],
                                  &num_stat_counter_ids,stat_counter_id);
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}





/*
 * Function:
 *      bcm_esw_l3_vrf_stat_enable_set
 * Purpose:
 *      Enable/disable packet and byte counters for the selected VRF.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      enable - (IN) Non-zero to enable counter collection, zero to disable.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_enable_set(int unit, bcm_vrf_t vrf, int enable)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int                        rv = BCM_E_UNAVAIL;
#if defined(BCM_KATANA_SUPPORT)
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    bcm_stat_object_t          object=bcmStatObjectIngPort;
    uint32                     num_entries=0;
    uint32                     num_stat_counter_ids=0;
    uint32                     stat_counter_id[
                                     BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]={0};
#endif /* BCM_KATANA_SUPPORT */
    L3_INIT(unit);
   if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);

    L3_LOCK(unit);
    rv = _bcm_esw_flex_stat_enable_set(unit, _bcmFlexStatTypeVrf,
                                       _bcm_vrf_flex_stat_hw_index_set,
                                       NULL, vrf, enable);
    L3_UNLOCK(unit);
    return rv;
   } 
#if defined(BCM_KATANA_SUPPORT)
    if (enable) { 
        BCM_IF_ERROR_RETURN(_bcm_esw_l3_vrf_stat_get_table_info(
                            unit,vrf,&num_of_tables,&table_info[0]));
        if (num_of_tables != 1) {
                return BCM_E_INTERNAL;
        }
        if (table_info[0].direction != bcmStatFlexDirectionIngress) {
            return BCM_E_INTERNAL;
        }
        BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_ingress_object(
                            table_info[0].table,&object)); 
        BCM_IF_ERROR_RETURN(bcm_esw_stat_group_create(
                            unit,object,bcmStatGroupModeSingle,
                            &stat_counter_id[table_info[0].direction],
                            &num_entries));
        BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_attach(
                            unit,vrf,stat_counter_id[table_info[0].direction])); 
    } else {
        BCM_IF_ERROR_RETURN(_bcm_esw_l3_vrf_stat_get_table_info(
                            unit,vrf,&num_of_tables,&table_info[0]));
        BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_counter_id(
                            unit, num_of_tables,&table_info[0],
                            &num_stat_counter_ids,&stat_counter_id[0])); 
        if (num_stat_counter_ids != 1) {
            return BCM_E_INTERNAL;
        }
        BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_detach(unit,vrf));
        BCM_IF_ERROR_RETURN(bcm_esw_stat_group_destroy(
                            unit,
                            stat_counter_id[bcmStatFlexDirectionIngress]));
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_KATANA_SUPPORT */
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_get
 * Purpose:
 *      Get 64-bit counter value for specified VRF statistic type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (OUT) Pointer to a counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_get(int unit, bcm_vrf_t vrf, bcm_l3_vrf_stat_t stat, 
                        uint64 *val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int              rv=BCM_E_UNAVAIL;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif
    L3_INIT(unit);
   if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_get(unit, _bcmFlexStatTypeVrf, vrf,
                           _bcm_esw_l3_vrf_stat_to_flex_stat(stat), val);
    L3_UNLOCK(unit);
    return rv;
   } 
#if defined(BCM_KATANA_SUPPORT)
    BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_get(
                        unit,vrf,stat,1, &counter_indexes, &counter_values));
    if (stat == bcmL3VrfStatIngressPackets) {
        COMPILER_64_SET(*val,0,counter_values.packets);
    } else {
        COMPILER_64_SET(*val,
                        COMPILER_64_HI(counter_values.bytes),
                        COMPILER_64_LO(counter_values.bytes));
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_get32
 * Purpose:
 *      Get lower 32-bit counter value for specified VRF statistic
 *      type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (OUT) Pointer to a counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_get32(int unit, bcm_vrf_t vrf, 
                          bcm_l3_vrf_stat_t stat, uint32 *val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif
   if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    L3_INIT(unit);
    BCM_VRF_STAT_VALID(unit, vrf);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_get32(unit, _bcmFlexStatTypeVrf, vrf,
                          _bcm_esw_l3_vrf_stat_to_flex_stat(stat), val);
    L3_UNLOCK(unit);
    return rv;
   }
#if defined(BCM_KATANA_SUPPORT)
    BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_get(
                        unit,vrf,stat,1, &counter_indexes, &counter_values));
    if (stat == bcmL3VrfStatIngressPackets) {
        *val = counter_values.packets;
    } else {
        /* Ignoring Hi bytes value */
        *val = COMPILER_64_LO(counter_values.bytes);
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_set
 * Purpose:
 *      Set 64-bit counter value for specified VRF statistic type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (IN) New counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_set(int unit, bcm_vrf_t vrf, bcm_l3_vrf_stat_t stat, 
                        uint64 val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif
    L3_INIT(unit);
   if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_set(unit, _bcmFlexStatTypeVrf, vrf,
                           _bcm_esw_l3_vrf_stat_to_flex_stat(stat), val);
    L3_UNLOCK(unit);
    return rv;
   } 
#if defined(BCM_KATANA_SUPPORT)
    if (stat == bcmL3VrfStatIngressPackets) {
        counter_values.packets = COMPILER_64_LO(val);
    } else {
        COMPILER_64_SET(counter_values.bytes,
                        COMPILER_64_HI(val),
                        COMPILER_64_LO(val));
    }
    BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_set(
                        unit,vrf,stat,1, &counter_indexes, &counter_values));
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_set32
 * Purpose:
 *      Set lower 32-bit counter value for specified VRF statistic
 *      type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (IN) New counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_set32(int unit, bcm_vrf_t vrf, 
                          bcm_l3_vrf_stat_t stat, uint32 val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif
    L3_INIT(unit);
   if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_set32(unit, _bcmFlexStatTypeVrf, vrf,
                            _bcm_esw_l3_vrf_stat_to_flex_stat(stat), val);
    L3_UNLOCK(unit);
    return rv;
   } 
#if defined(BCM_KATANA_SUPPORT)
    if (stat == bcmL3VrfStatIngressPackets) {
        counter_values.packets = val;
    } else {
        /* Ignoring high value */
        COMPILER_64_SET(counter_values.bytes,0,val);
    }
    BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_set(
                        unit,vrf, stat, 1, &counter_indexes, &counter_values));
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_multi_get
 * Purpose:
 *      Get 64-bit counter value for multiple VRF statistic types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (OUT) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_multi_get(int unit, bcm_vrf_t vrf, int nstat, 
                              bcm_l3_vrf_stat_t *stat_arr,
                              uint64 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int rv = BCM_E_UNAVAIL;
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int                 idx;       /* Statistics iteration index. */
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

    L3_INIT(unit);

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);
    BCM_IF_ERROR_RETURN
        (_bcm_l3_vrf_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_VRF_STAT_ARRAY_VALID(unit, nstat, value_arr);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_multi_get(unit, _bcmFlexStatTypeVrf, vrf,
                                      nstat, fs_arr, value_arr);
    L3_UNLOCK(unit);
    return rv; 
  } 
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_get( 
                             unit, vrf, stat_arr[idx], 
                             1, &counter_indexes, &counter_values));
         if (stat_arr[idx] == bcmL3VrfStatIngressPackets) {
             COMPILER_64_SET(value_arr[idx],0,counter_values.packets);
         } else {
             COMPILER_64_SET(value_arr[idx],
                             COMPILER_64_HI(counter_values.bytes),
                             COMPILER_64_LO(counter_values.bytes));
         }
    }
    return  BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_multi_get32
 * Purpose:
 *      Get lower 32-bit counter value for multiple VRF statistic
 *      types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (OUT) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_multi_get32(int unit, bcm_vrf_t vrf, int nstat, 
                                bcm_l3_vrf_stat_t *stat_arr,
                                uint32 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int rv = BCM_E_UNAVAIL;
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int                 idx;       /* Statistics iteration index. */
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

    L3_INIT(unit);
  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);
    BCM_IF_ERROR_RETURN
        (_bcm_l3_vrf_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_VRF_STAT_ARRAY_VALID(unit, nstat, value_arr);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_multi_get32(unit, _bcmFlexStatTypeVrf, vrf,
                                        nstat, fs_arr, value_arr);
    L3_UNLOCK(unit);
    return rv;
  } 
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_get(
                             unit, vrf, stat_arr[idx], 
                             1, &counter_indexes, &counter_values));
         if (stat_arr[idx] == bcmL3VrfStatIngressPackets) {
             value_arr[idx] = counter_values.packets;
         } else {
             value_arr[idx] = COMPILER_64_LO(counter_values.bytes);
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_multi_set
 * Purpose:
 *      Set 64-bit counter value for multiple VRF statistic types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (IN) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_multi_set(int unit, bcm_vrf_t vrf, int nstat, 
                              bcm_l3_vrf_stat_t *stat_arr, 
                              uint64 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int rv = BCM_E_UNAVAIL;
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int                 idx;       /* Statistics iteration index. */
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

    L3_INIT(unit);
  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);
    BCM_IF_ERROR_RETURN
        (_bcm_l3_vrf_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_VRF_STAT_ARRAY_VALID(unit, nstat, value_arr);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_multi_set(unit, _bcmFlexStatTypeVrf, vrf,
                                      nstat, fs_arr, value_arr);
    L3_UNLOCK(unit);
    return rv;
  } 
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         if (stat_arr[idx] == bcmL3VrfStatIngressPackets) {
             counter_values.packets = COMPILER_64_LO(value_arr[idx]);
         } else {
             COMPILER_64_SET(counter_values.bytes,
                             COMPILER_64_HI(value_arr[idx]),
                             COMPILER_64_LO(value_arr[idx]));
         }
         BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_set( 
                             unit, vrf, stat_arr[idx], 1, 
                             &counter_indexes, &counter_values));
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_vrf_stat_multi_set32
 * Purpose:
 *      Set lower 32-bit counter value for multiple VRF statistic
 *      types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      vrf - (IN) Virtual router instance.
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (IN) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_vrf_stat_multi_set32(int unit, bcm_vrf_t vrf, int nstat,
                                bcm_l3_vrf_stat_t *stat_arr, 
                                uint32 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int rv = BCM_E_UNAVAIL;
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int                 idx;       /* Statistics iteration index. */
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

    L3_INIT(unit);
 if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_VRF_STAT_VALID(unit, vrf);
    BCM_IF_ERROR_RETURN
        (_bcm_l3_vrf_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_VRF_STAT_ARRAY_VALID(unit, nstat, value_arr);
    L3_LOCK(unit);

    rv = _bcm_esw_flex_stat_multi_set32(unit, _bcmFlexStatTypeVrf, vrf,
                                        nstat, fs_arr, value_arr);
    L3_UNLOCK(unit);
    return rv;
  } 
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         if (stat_arr[idx] == bcmL3VrfStatIngressPackets) {
             counter_values.packets = value_arr[idx];
         } else {
             COMPILER_64_SET(counter_values.bytes,0,value_arr[idx]);
         }
         BCM_IF_ERROR_RETURN(bcm_esw_l3_vrf_stat_counter_set(
                             unit, vrf, stat_arr[idx], 1, 
                             &counter_indexes, &counter_values));
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_source_bind_enable_set
 * Purpose:
 *      Enable or disable l3 source binding checks on an ingress port.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Packet ingress port.
 *      enable - (IN) Non-zero to enable l3 source binding checks,
 *               zero to disable.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_source_bind_enable_set(int unit, bcm_port_t port, int enable)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int key_type;
    _bcm_port_config_t key_config, port_config;

    if (!soc_feature(unit, soc_feature_ip_source_bind)) {
        return BCM_E_UNAVAIL;
    }

    if (enable) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_get(unit, port,
                                      _bcmPortVTKeyTypeSecond, &key_type));
        if (key_type == TR_VLXLT_HASH_KEY_TYPE_OVID) {
            /* Overwrite OVID if it's in second slot */
            key_config = _bcmPortVTKeyTypeSecond;
            port_config = _bcmPortVTKeyPortSecond;
        } else { /* Otherwise just use first slot */
            key_config = _bcmPortVTKeyTypeFirst;
            port_config = _bcmPortVTKeyPortFirst;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_set(unit, port, key_config,
                                      TR_VLXLT_HASH_KEY_TYPE_HPAE));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_set(unit, port, port_config,
                                      TR_VLXLT_HASH_KEY_TYPE_OTAG));
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_get(unit, port,
                                      _bcmPortVTKeyTypeSecond, &key_type));
        if (key_type == TR_VLXLT_HASH_KEY_TYPE_HPAE) {
            key_config = _bcmPortVTKeyTypeSecond;
            port_config = _bcmPortVTKeyPortSecond;
        } else {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_config_get(unit, port,
                                          _bcmPortVTKeyTypeFirst,
                                          &key_type));
            if (key_type == TR_VLXLT_HASH_KEY_TYPE_HPAE) {
                key_config = _bcmPortVTKeyTypeFirst;
                port_config = _bcmPortVTKeyPortFirst;
            } else {
                /* Already not enabled */
                return BCM_E_NONE;
            }
        }
        /* Return to OVID usage */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_set(unit, port, key_config,
                                      TR_VLXLT_HASH_KEY_TYPE_OVID));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_set(unit, port, port_config,
                                      TR_VLXLT_HASH_KEY_TYPE_OTAG));
    }
    return BCM_E_NONE; 
#else
    return BCM_E_UNAVAIL; 
#endif
}

/*
 * Function:
 *      bcm_eswl3_source_bind_enable_get
 * Purpose:
 *      Retrieve whether l3 source binding checks are performed on an
 *      ingress port.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Packet ingress port.
 *      enable - (OUT) Non-zero if l3 source binding checks are enabled,
 *               zero if disabled.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_source_bind_enable_get(int unit, bcm_port_t port, int *enable)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int key_type;
    if (!soc_feature(unit, soc_feature_ip_source_bind)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_config_get(unit, port,
                                  _bcmPortVTKeyTypeSecond, &key_type));
    if (key_type == TR_VLXLT_HASH_KEY_TYPE_HPAE) {
        *enable = TRUE;
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_get(unit, port,
                                      _bcmPortVTKeyTypeFirst, &key_type));
        if (key_type == TR_VLXLT_HASH_KEY_TYPE_HPAE) {
            *enable = TRUE;
        } else {
            *enable = FALSE;
        }
    }

    return BCM_E_NONE; 
#else
    return BCM_E_UNAVAIL; 
#endif
} 
/*
 * Function:
 *      bcm_esw_l3_source_bind_add
 * Purpose:
 *      Add or replace an L3 source binding.
 * Parameters:
 *      unit - (IN) Unit number.
 *      info - (IN) L3 source binding information
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_source_bind_add(int unit, bcm_l3_source_bind_t *info)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int             tmp_id, rv, idx = 0;
    bcm_module_t    modid_out;
    bcm_port_t      port_out;
    bcm_trunk_t     trunk_out;
    vlan_mac_entry_t vment, res_vment;

    if (!soc_feature(unit, soc_feature_ip_source_bind)) {
        return BCM_E_UNAVAIL;
    }

    if (info->flags & BCM_L3_SOURCE_BIND_IP6) {
        /* Not supported yet */
        return BCM_E_UNAVAIL;
    }

    sal_memset(&vment, 0, sizeof(vlan_mac_entry_t));
    sal_memset(&res_vment, 0, sizeof(vlan_mac_entry_t));

    /* MAC_IP_BIND lookup key type */
    soc_VLAN_MACm_field32_set(unit, &vment, VALIDf, 1);
    soc_VLAN_MACm_field32_set(unit, &vment, KEY_TYPEf,
                              TR_VLXLT_HASH_KEY_TYPE_HPAE);
    soc_VLAN_MACm_field32_set(unit, &vment,
                              MAC_IP_BIND__SIPf, info->ip);

    rv = soc_mem_search(unit, VLAN_MACm, MEM_BLOCK_ALL, &idx, 
                        &vment, &res_vment, 0);

    if (BCM_SUCCESS(rv)) {
        if (soc_VLAN_MACm_field32_get(unit, &res_vment, VALIDf)) {
            /* Valid entry found */
            if ((info->flags & BCM_L3_SOURCE_BIND_REPLACE) == 0) {
                /* Entry exists! */
                return BCM_E_EXISTS;
            }
        }
    } else if (rv != SOC_E_NOT_FOUND) {
        return rv;
    }

    if (info->port == BCM_GPORT_INVALID) {
        /* wild card */
        soc_VLAN_MACm_field32_set(unit, &vment,
                                  MAC_IP_BIND__SRC_MODIDf, 0x7f);
        soc_VLAN_MACm_field32_set(unit, &vment,
                                  MAC_IP_BIND__SRC_Tf, 1);
        soc_VLAN_MACm_field32_set(unit, &vment,
                                  MAC_IP_BIND__SRC_PORTf, 0x3f);
    } else if (BCM_GPORT_IS_SET(info->port)) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_gport_resolve(unit, info->port, &modid_out,
                                    &port_out, &trunk_out, &tmp_id));
        if (-1 != tmp_id) {
            return BCM_E_PARAM;
        }
        if (BCM_TRUNK_INVALID != trunk_out) {
            soc_VLAN_MACm_field32_set(unit, &vment,
                                      MAC_IP_BIND__SRC_Tf, 1);
            soc_VLAN_MACm_field32_set(unit, &vment, MAC_IP_BIND__SRC_MODIDf,
                                      (trunk_out >> 6) & 0x1);
            soc_VLAN_MACm_field32_set(unit, &vment, MAC_IP_BIND__SRC_PORTf,
                                      trunk_out & 0x3f);
        } else {
            soc_VLAN_MACm_field32_set(unit, &vment, MAC_IP_BIND__SRC_MODIDf,
                                      modid_out);
            soc_VLAN_MACm_field32_set(unit, &vment, MAC_IP_BIND__SRC_PORTf,
                                      port_out);
        }
    } else {
        return BCM_E_PORT;
    }

    soc_VLAN_MACm_mac_addr_set(unit, &vment,
                               MAC_IP_BIND__HPAE_MAC_ADDRf, info->mac);

    if (soc_feature(unit, soc_feature_ipfix_rate) && info->rate_id != 0) {
        soc_VLAN_MACm_field32_set
            (unit, &vment, MAC_IP_BIND__IPFIX_FLOW_METER_IDf, info->rate_id);
    }

    rv = soc_mem_insert(unit, VLAN_MACm, MEM_BLOCK_ALL, &vment);

    if ((rv == SOC_E_EXISTS) &&
        (info->flags & BCM_L3_SOURCE_BIND_REPLACE)) {
        rv = BCM_E_NONE;
    }

    return rv;
#else
    return BCM_E_UNAVAIL; 
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_source_bind_delete
 * Purpose:
 *      Remove an existing L3 source binding.
 * Parameters:
 *      unit - (IN) Unit number.
 *      info - (IN) L3 source binding information
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_source_bind_delete(int unit, bcm_l3_source_bind_t *info)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    vlan_mac_entry_t vment;
    int rv; 

    if (!soc_feature(unit, soc_feature_ip_source_bind)) {
        return BCM_E_UNAVAIL;
    }

    if (info->flags & BCM_L3_SOURCE_BIND_IP6) {
        /* Not supported yet */
        return BCM_E_UNAVAIL;
    }

    sal_memset(&vment, 0, sizeof(vment));

    /* MAC_IP_BIND key type */
    soc_VLAN_MACm_field32_set(unit, &vment, VALIDf, 1);
    soc_VLAN_MACm_field32_set(unit, &vment, KEY_TYPEf,
                              TR_VLXLT_HASH_KEY_TYPE_HPAE);
    soc_VLAN_MACm_field32_set(unit, &vment,
                              MAC_IP_BIND__SIPf, info->ip);

    rv = soc_mem_delete(unit, VLAN_MACm, MEM_BLOCK_ALL, &vment);

    if (rv == SOC_E_NOT_FOUND) {
        rv = SOC_E_NONE;
    }
    return rv;
#else
    return BCM_E_UNAVAIL; 
#endif /* BCM_TRIUMPH2_SUPPORT */
}

/*
 * Function:
 *      bcm_esw_l3_source_bind_delete_all
 * Purpose:
 *      Remove all existing L3 source bindings.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_source_bind_delete_all(int unit)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int idx, imin, imax, nent, vmbytes, rv;
    vlan_mac_entry_t * vmtab, *vmtabp;
    
    if (!soc_feature(unit, soc_feature_ip_source_bind)) {
        return BCM_E_UNAVAIL;
    }

    imin = soc_mem_index_min(unit, VLAN_MACm);
    imax = soc_mem_index_max(unit, VLAN_MACm);
    nent = soc_mem_index_count(unit, VLAN_MACm);
    vmbytes = soc_mem_entry_words(unit, VLAN_MACm);
    vmbytes = WORDS2BYTES(vmbytes);
    vmtab = soc_cm_salloc(unit, nent * sizeof(*vmtab), "vlan_mac");

    if (vmtab == NULL) {
        return BCM_E_MEMORY;
    }
    
    soc_mem_lock(unit, VLAN_MACm);
    rv = soc_mem_read_range(unit, VLAN_MACm, MEM_BLOCK_ANY,
                            imin, imax, vmtab);
    if (BCM_FAILURE(rv)) {
        soc_mem_unlock(unit, VLAN_MACm);
        soc_cm_sfree(unit, vmtab);
        return rv; 
    }
    
    for(idx = 0; idx < nent; idx++) {
        vmtabp = soc_mem_table_idx_to_pointer(unit, VLAN_MACm,
                                              vlan_mac_entry_t *, 
                                              vmtab, idx);

        if ((0 == soc_VLAN_MACm_field32_get(unit, vmtabp, VALIDf)) ||
            (TR_VLXLT_HASH_KEY_TYPE_HPAE !=
             soc_VLAN_MACm_field32_get(unit, vmtabp, KEY_TYPEf))) {
            continue;
        }

        rv = soc_mem_delete(unit, VLAN_MACm, MEM_BLOCK_ALL, vmtabp);
        if (BCM_FAILURE(rv)) {
            soc_mem_unlock(unit, VLAN_MACm);
            soc_cm_sfree(unit, vmtab);
            return rv; 
        }
    }
    
    soc_mem_unlock(unit, VLAN_MACm);
    soc_cm_sfree(unit, vmtab);
    return rv;
#else
    return BCM_E_UNAVAIL; 
#endif /* BCM_TRIUMPH2_SUPPORT */
}

#if defined(BCM_TRIUMPH2_SUPPORT)
STATIC int
_bcm_esw_l3_source_bind_hw_entry_to_sw_info(int unit,
                                            vlan_mac_entry_t *vment,
                                            bcm_l3_source_bind_t *info)
{
    int             t_bit;
    bcm_module_t    modid;
    bcm_port_t      port;

    bcm_l3_source_bind_t_init(info);
    info->ip = soc_VLAN_MACm_field32_get(unit, vment, MAC_IP_BIND__SIPf);
    if (soc_feature(unit, soc_feature_ipfix_rate)) {
        info->rate_id = soc_VLAN_MACm_field32_get
            (unit, vment, MAC_IP_BIND__IPFIX_FLOW_METER_IDf);
    }
    soc_VLAN_MACm_mac_addr_get(unit, vment,
                               MAC_IP_BIND__HPAE_MAC_ADDRf, info->mac);

    /* Port analysis */

    port = soc_VLAN_MACm_field32_get(unit, vment,
                                           MAC_IP_BIND__SRC_PORTf);
    modid = soc_VLAN_MACm_field32_get(unit, vment,
                                      MAC_IP_BIND__SRC_MODIDf);
    t_bit = soc_VLAN_MACm_field32_get(unit, vment,
                                    MAC_IP_BIND__SRC_Tf);

    if (t_bit == 1) {
        if ((modid == 0x7f) && (port == 0x3f)) {
            info->port = BCM_GPORT_TYPE_BLACK_HOLE; /* Wild card */
        } else {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_l3_gport_construct(unit, 0, 0,
                                   (port & 0x3f) | ((modid & 0x1) << 6),
                                             BCM_L3_TGID, &(info->port)));
        }
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_l3_gport_construct(unit, modid, port,
                                         0, 0, &(info->port)));
    }

    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      bcm_esw_l3_source_bind_get
 * Purpose:
 *      Retrieve the details of an existing L3 source binding.
 * Parameters:
 *      unit - (IN) Unit number.
 *      info - (IN/OUT) L3 source binding information
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_source_bind_get(int unit, bcm_l3_source_bind_t *info)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int             rv, idx = 0;
    vlan_mac_entry_t vment, res_vment;

    if (!soc_feature(unit, soc_feature_ip_source_bind)) {
        return BCM_E_UNAVAIL;
    }

    if (info->flags & BCM_L3_SOURCE_BIND_IP6) {
        /* Not supported yet */
        return BCM_E_UNAVAIL;
    }

    sal_memset(&vment, 0, sizeof(vlan_mac_entry_t));
    sal_memset(&res_vment, 0, sizeof(vlan_mac_entry_t));

    /* MAC_IP_BIND lookup key type */
    soc_VLAN_MACm_field32_set(unit, &vment, VALIDf, 1);
    soc_VLAN_MACm_field32_set(unit, &vment, KEY_TYPEf,
                              TR_VLXLT_HASH_KEY_TYPE_HPAE);
    soc_VLAN_MACm_field32_set(unit, &vment,
                              MAC_IP_BIND__SIPf, info->ip);

    soc_mem_lock(unit, VLAN_MACm);
    rv = soc_mem_search(unit, VLAN_MACm, MEM_BLOCK_ALL, &idx, 
                        &vment, &res_vment, 0);
    soc_mem_unlock(unit, VLAN_MACm);
    BCM_IF_ERROR_RETURN(rv);

    return _bcm_esw_l3_source_bind_hw_entry_to_sw_info(unit, &res_vment,
                                                       info);
#else
    return BCM_E_UNAVAIL; 
#endif /* BCM_TRIUMPH2_SUPPORT */
}


/*
 * Function:
 *      bcm_esw_l3_source_bind_traverse
 * Purpose:
 *      Traverse through the L3 source bindings and run callback at
 *      each defined binding.
 * Parameters:
 *      unit - (IN) Unit number.
 *      cb - (IN) Call back routine.
 *      user_data - (IN) User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_l3_source_bind_traverse(int unit, bcm_l3_source_bind_traverse_cb cb, 
                                void *user_data)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    int idx, imin, imax, nent, vmbytes, rv;
    vlan_mac_entry_t * vmtab, *vmtabp;
    bcm_l3_source_bind_t info;

    if (!soc_feature(unit, soc_feature_ip_source_bind)) {
        return BCM_E_UNAVAIL;
    }

    /* Input parameters check. */
    if (NULL == cb) {
        return (BCM_E_PARAM);
    }

    imin = soc_mem_index_min(unit, VLAN_MACm);
    imax = soc_mem_index_max(unit, VLAN_MACm);
    nent = soc_mem_index_count(unit, VLAN_MACm);
    vmbytes = soc_mem_entry_words(unit, VLAN_MACm);
    vmbytes = WORDS2BYTES(vmbytes);
    vmtab = soc_cm_salloc(unit, nent * sizeof(*vmtab), "vlan_mac");

    if (vmtab == NULL) {
        return BCM_E_MEMORY;
    }
    
    soc_mem_lock(unit, VLAN_MACm);
    rv = soc_mem_read_range(unit, VLAN_MACm, MEM_BLOCK_ANY,
                            imin, imax, vmtab);
    if (BCM_FAILURE(rv)) {
        soc_mem_unlock(unit, VLAN_MACm);
        soc_cm_sfree(unit, vmtab);
        return rv; 
    }
    
    for(idx = 0; idx < nent; idx++) {
        vmtabp = soc_mem_table_idx_to_pointer(unit, VLAN_MACm,
                                              vlan_mac_entry_t *, 
                                              vmtab, idx);

        if ((0 == soc_VLAN_MACm_field32_get(unit, vmtabp, VALIDf)) ||
            (TR_VLXLT_HASH_KEY_TYPE_HPAE !=
             soc_VLAN_MACm_field32_get(unit, vmtabp, KEY_TYPEf))) {
            continue;
        }

        rv = _bcm_esw_l3_source_bind_hw_entry_to_sw_info(unit, vmtabp,
                                                         &info);

        if (BCM_SUCCESS(rv)) {
            /* Call traverse callback with the data. */
            rv = cb(unit, &info, user_data);
        }

        if (BCM_FAILURE(rv)) {
            soc_mem_unlock(unit, VLAN_MACm);
            soc_cm_sfree(unit, vmtab);
            return rv; 
        }
    }
    
    soc_mem_unlock(unit, VLAN_MACm);
    soc_cm_sfree(unit, vmtab);
    return rv;
#else
    return BCM_E_UNAVAIL; 
#endif /* BCM_TRIUMPH2_SUPPORT */
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_l3_cleanup(int unit)
{

    return(mbcm_driver[unit]->mbcm_l3_tables_cleanup(unit));
}
#endif /* BCM_WARM_BOOT_SUPPORT */
#endif  /* INCLUDE_L3 */
/*
 * Function:
 *      _bcm_esw_l3_lock
 * Purpose:
 *     Lock L3 module - if module was not initialized NOOP
 *    
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_esw_l3_lock(int unit)
{
#if defined(INCLUDE_L3)
    if (NULL != _bcm_l3_bk_info[unit].lock) {
        return sal_mutex_take(_bcm_l3_bk_info[unit].lock, 
                              sal_mutex_FOREVER);
    }
#endif /* INCLUDE_L3 */
    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_esw_l3_unlock
 * Purpose:
 *     Unlock L3 module - if module was not initialized NOOP
 *    
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_esw_l3_unlock(int unit)
{
#if defined(INCLUDE_L3)
    if (NULL != _bcm_l3_bk_info[unit].lock) {
        return sal_mutex_give(_bcm_l3_bk_info[unit].lock);
    }
#endif /* INCLUDE_L3 */
    return (BCM_E_NONE);
}
