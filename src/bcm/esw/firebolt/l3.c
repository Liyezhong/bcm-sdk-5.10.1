/*
 * $Id: l3.c 1.605.2.43 Broadcom SDK $
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
 * File:        l3.c
 * Purpose:     Firebolt L3 function implementations
 */
#include <soc/defs.h>
#ifdef INCLUDE_L3

#include <assert.h>

#include <sal/core/libc.h>

#include <shared/util.h>
#if defined(BCM_FIREBOLT_SUPPORT) || defined (BCM_EASYRIDER_SUPPORT)
#include <soc/mem.h>
#include <soc/cm.h>
#include <soc/drv.h>
#ifdef BCM_EASYRIDER_SUPPORT
#include <soc/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/l3x.h>
#include <soc/lpm.h>
#include <soc/tnl_term.h>

#include <bcm/l3.h>
#include <bcm/error.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#ifdef BCM_EASYRIDER_SUPPORT
#include <bcm_int/esw/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_TRX_SUPPORT) 
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/virtual.h>
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/mpls.h>
#endif /* BCM_TRIUMPH_SUPPORT*/
#ifdef BCM_SCORPION_SUPPORT
#include <bcm_int/esw/scorpion.h>
#endif /* BCM_SCORPION_SUPPORT*/
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <bcm_int/esw/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_HURRICANE_SUPPORT
#include <bcm_int/esw/hurricane.h>
#endif /* BCM_HURRICANE_SUPPORT*/
#if defined(BCM_TRIDENT_SUPPORT)
#include <bcm_int/esw/trident.h>
#endif /* BCM_TRIDENT_SUPPORT*/
#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif /* BCM_KATANA_SUPPORT */
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/xgs3.h>
#include <bcm_int/esw/stack.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#include <bcm_int/esw_dispatch.h>

#if defined(BCM_KATANA_SUPPORT)
uint32 ipv6_lpm_128b_enable;
extern _bcm_l3_defip_128_table_t *l3_trx_defip_128[BCM_MAX_NUM_UNITS];
extern _bcm_l3_defip_pair128_table_t *l3_kt_defip_pair128[BCM_MAX_NUM_UNITS];
extern uint32 kt_depth128;
extern uint32 kt_depth64;
#endif

_bcm_l3_module_data_t *l3_module_data[BCM_MAX_NUM_UNITS] = { 0 };

/* Functions prototypes. */
STATIC int _bcm_xgs3_l3_hw_op_init(int unit);
STATIC int _bcm_xgs3_tnl_init_hash_calc(int unit, void *buf, uint16 *hash);
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
STATIC int _bcm_rp_l3_deinit(int unit);
STATIC int _bcm_rp_l3_init(int unit);
#ifdef BCM_WARM_BOOT_SUPPORT
static int _bcm_rp_l3_group_reload(int unit, bcm_field_group_t group, 
    void *user_data);
#endif
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
STATIC int _bcm_fb_nh_intf_is_tnl_update(int unit, bcm_if_t intf, int
                                         tunnel_id);
STATIC void _bcm_fb_l3_ipmc_ent_init(int unit, uint32 *buf_p, 
                                     _bcm_l3_cfg_t *l3cfg);
#endif /* BCM_FIREBOLT_SUPPORT */

/* Forward declarations (defined below) */
int _bcm_xgs3_l3_ingress_interface_add(int unit, _bcm_l3_ingress_intf_t *iif) ;
int _bcm_xgs3_l3_ingress_interface_delete(int unit, int intf_id) ;
int _bcm_xgs3_l3_ingress_interface_get(int unit, _bcm_l3_ingress_intf_t *iif);
#ifdef BCM_WARM_BOOT_SUPPORT
STATIC int _bcm_xgs3_l3_ecmp_reinit(int unit);
#endif /*BCM_WARM_BOOT_SUPPORT*/
/*
 * Function:
 *      bcm_xgs3_l3_mask6_apply
 * Purpose:
 *      Apply IPv6 mask on address 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      mask     -  (IN)IPv6 Mask 
 *      addr     -  (IN/OUT)Address to apply the mask
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_mask6_apply(bcm_ip6_t mask, bcm_ip6_t addr)
{
    uint8 *mask_iter;         /* Mask bytes iterator.      */
    uint8 *addr_iter;         /* Mask bytes iterator.      */
    int idx;                  /* IPv6 addresses iterator.  */ 

    mask_iter = (uint8 *)mask;
    addr_iter = (uint8 *)addr;

    for (idx = 0; idx < 16; idx++) {
        (*addr_iter) &= (*mask_iter);
        addr_iter++;
        mask_iter++;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_mode_set
 * Purpose:
 *      Set unit l3 egress switching mode.
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      mode     -  (IN)Egress switching mode. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_egress_mode_set(int unit, int mode)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Set mode flag. */
    switch(mode) {
      case 0:
          BCM_XGS3_L3_FLAGS(unit) &= ~_BCM_L3_SHR_EGRESS_MODE; 
          break;
      case 1: 
          BCM_XGS3_L3_FLAGS(unit) |= _BCM_L3_SHR_EGRESS_MODE; 
          break;
      default:
          return (BCM_E_PARAM);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_egress_mode_get
 * Purpose:
 *      Get unit l3 egress switching mode.
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      mode     -  (IN)Egress switching mode. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_egress_mode_get(int unit, int *mode)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == mode) {
        return (BCM_E_PARAM);
    }

    *mode = (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) ? 1 : 0;
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_host_as_route_return_set
 * Purpose:
 *      Set mode for bcmSwitchL3HostAsRouteReturnValue
 *      Application controls the return value setting for bcm_l3_host_add API 
 *      with BCM_L3_HOST_AS_ROUTE flag. If prefix gets added to HOST table, 
 *      then return value=0. If prefix gets added to DEFIP table, 
 *      then return value = ret_val 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      ret_val     -  (IN) Return Value 
 * Returns:
 *      BCM_X_XXX
 */

int
bcm_xgs3_l3_host_as_route_return_set(int unit, int ret_val)
{
    ing_l3_next_hop_entry_t in_entry;   /* Buffer for ingress nh entry. */

    /* Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
         return (BCM_E_INIT);
    }

    /* Set mode */
    if (soc_mem_is_valid(unit, ING_L3_NEXT_HOPm)) {
         if (ret_val == 0) {
              BCM_XGS3_L3_FLAGS(unit) &= ~_BCM_L3_SHR_HOST_ADD_MODE; 
         } else if ((ret_val > 0) && (ret_val <= 0xFF)) {
             BCM_XGS3_L3_FLAGS(unit) |= _BCM_L3_SHR_HOST_ADD_MODE;
             /* Zero buffers. */
             sal_memset(&in_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, nh));
             /* Store RETURN_VALUE within unused field of Black-Hole Next-Hop */
             soc_mem_field32_set(unit, ING_L3_NEXT_HOPm, &in_entry, 
                             VLAN_IDf, ret_val);
             /* Write ingress next hop entry. */
             BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_WRITE(unit, 
                             ING_L3_NEXT_HOPm, 0, &in_entry));
         } else {
             return (BCM_E_PARAM);
         }

#ifdef BCM_WARM_BOOT_SUPPORT
SOC_CONTROL_LOCK(unit);
SOC_CONTROL(unit)->scache_dirty = 1;
SOC_CONTROL_UNLOCK(unit);
#endif
    }

return (BCM_E_NONE);

}

/*
 * Function:
 *      bcm_xgs3_l3_host_as_route_return_get
 * Purpose:
 *      Get mode for bcmSwitchL3HostAsRouteReturnValue
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      ret_val     -  (IN)Return Value 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_host_as_route_return_get(int unit, int *ret_val)
{
    int state;
    ing_l3_next_hop_entry_t in_entry;   /* Buffer for ingress nh entry. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == ret_val) {
        return (BCM_E_PARAM);
    }

    if (soc_mem_is_valid(unit, ING_L3_NEXT_HOPm)) {
         state = (BCM_XGS3_L3_HOST_AS_ROUTE_MODE_ISSET(unit));
         if (state == 0) {
             *ret_val = 0;
         } else {
             /* Zero buffers. */
             sal_memset(&in_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, nh));
             /* Read ingress next hop entry. */
             BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, 
                             ING_L3_NEXT_HOPm, 0, &in_entry));
             /* Obtain RETURN_VALUE within unused field of Black-Hole Next-Hop */
             *ret_val = soc_mem_field32_get(unit, ING_L3_NEXT_HOPm, 
                             &in_entry, VLAN_IDf);
         }
    } else {
         *ret_val = 0;
    }

    return (BCM_E_NONE);
}

#if defined(BCM_TRIUMPH_SUPPORT)
/*
 * Function:
 *      bcm_xgs3_l3_ingress_mode_set
 * Purpose:
 *      Set unit l3 ingress switching mode.
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      mode     -  (IN)Ingress switching mode. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_ingress_mode_set(int unit, int mode)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Set mode flag. */
    switch(mode) {
      case 0:
          BCM_XGS3_L3_FLAGS(unit) &= ~_BCM_L3_SHR_INGRESS_MODE; 
          break;
      case 1: 
          BCM_XGS3_L3_FLAGS(unit) |= _BCM_L3_SHR_INGRESS_MODE; 
          break;
      default:
          return (BCM_E_PARAM);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_ingress_mode_get
 * Purpose:
 *      Get unit l3 ingress switching mode.
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      mode     -  (OUT)Ingress switching mode. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_ingress_mode_get(int unit, int *mode)
{

    /* Input parameters check */
    if (NULL == mode) {
        return (BCM_E_PARAM);
    }

    *mode = (BCM_XGS3_L3_INGRESS_MODE_ISSET(unit)) ? 1 : 0;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_ingress_intf_map_set
 * Purpose:
 *      Set unit l3 ingress interface map mode 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      mode     -  (IN)ingress interface map mode 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_ingress_intf_map_set(int unit, int mode)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Set mode flag. */
    switch(mode) {
      case 0:
          BCM_XGS3_L3_FLAGS(unit) &= ~_BCM_L3_SHR_INGRESS_INTF_MAP; 
          break;
      case 1: 
          BCM_XGS3_L3_FLAGS(unit) |= _BCM_L3_SHR_INGRESS_INTF_MAP; 
          break;
      default:
          return (BCM_E_PARAM);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_ingress_intf_map_get
 * Purpose:
 *      Get unit ll3 ingress interface map mode 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      mode     -  (OUT))ingress interface map mode 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_xgs3_l3_ingress_intf_map_get(int unit, int *mode)
{

    /* Input parameters check */
    if (NULL == mode) {
        return (BCM_E_PARAM);
    }

    *mode = (BCM_XGS3_L3_INGRESS_INTF_MAP_MODE_ISSET(unit)) ? 1 : 0;
    return (BCM_E_NONE);
}
#endif /* BCM_TRIUMPH_SUPPORT */

/*
 * Function:
 *      _bcm_xgs3_l3_ecmp_group_alloc
 * Purpose:
 *      Allocate l3 ecmp group members array. 
 * Parameters:
 *      unit    -  (IN) BCM device number. 
 *      ptr     -  (IN) Allocated group pointer
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_l3_ecmp_group_alloc(int unit, int **ptr)
{
    int alloc_size;
    int max_val;

    /*  Make sure module was initialized. */
    if (NULL == ptr) {
        return (BCM_E_NONE);
    }


    alloc_size = sizeof(int) * BCM_XGS3_L3_ECMP_MAX(unit); 
    max_val = BCM_XGS3_L3_ECMP_MAX(unit);

    *ptr = NULL;
    BCM_XGS3_L3_ALLOC((*ptr), alloc_size, "ecmp group next hops array");

    /* Input parameters check */
    if (NULL == *ptr) {
        return (BCM_E_MEMORY);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_nh_hash_calc
 * Purpose:
 *      Calculate next hop entry hash(signature). 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      buf      -  (IN)Next hop entry information.
 *      hash     -  (OUT) Hash(signature) calculated value.  
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_nh_hash_calc(int unit, void *buf, uint16 *hash)
{
    uint32 sw_buf[10]; 
    bcm_l3_egress_t *nh_entry;
    uint32 mac_field[2];

    /* Input parameters check. */
    if ((NULL == buf) || (NULL == hash)) {
        return (BCM_E_PARAM);
    }
    sal_memset (sw_buf, 0, 10 * sizeof(uint32));
    nh_entry = (bcm_l3_egress_t *) buf;

    /*
     * Copy entry information to a temporary buffer, so we can  
     * mask some fields we don't want to include in hash calculation.
     */
     sw_buf[0] = nh_entry->intf;
     SAL_MAC_ADDR_TO_UINT32(nh_entry->mac_addr, mac_field);
     sw_buf[1] = mac_field[0];
     sw_buf[2] = ((mac_field[1] << 16) | nh_entry->vlan);
     sw_buf[3] = nh_entry->module;
     sw_buf[4] = nh_entry->port;
     sw_buf[5] = nh_entry->trunk;
#ifdef BCM_MPLS_SUPPORT
     if (soc_feature(unit, soc_feature_mpls)) {
         /* Include mpls info only if label is valid. */
         if (BCM_XGS3_L3_MPLS_LBL_VALID(nh_entry->mpls_label)) {
             sw_buf[6] = nh_entry->mpls_label;
             sw_buf[7] = nh_entry->mpls_qos_map_id;
             sw_buf[8] = nh_entry->mpls_ttl;
         }
     }
#endif /* BCM_MPLS_SUPPORT */
     sw_buf[9] = nh_entry->encap_id;

    *hash = _shr_crc16(0, (uint8 *)&sw_buf[0], sizeof(uint32)*10);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_cmp_int
 * Purpose:
 *      Compare two integers.
 * Parameters:
 *      b - first compared integer
 *      a - second compared integer
 * Returns:
 *      a<=>b
 */
STATIC INLINE int
_bcm_xgs3_cmp_int(void *a, void *b)
{
    int first;                  /* First compared integer. */
    int second;                 /* Second compared integer. */

    first = *(int *)a;
    second = *(int *)b;

    if (first < second) {
        return (BCM_L3_CMP_LESS);
    } else if (first > second) {
        return (BCM_L3_CMP_GREATER);
    }
    return (BCM_L3_CMP_EQUAL);
}

/*
 * Function:
 *      _bcm_xgs3_nh_map_hw_data_to_api
 * Purpose:
 *      Convert HW space nh entry format to API space nh entry.
 *      Currently service routine used to map hw port/trunk info 
 *      to next hop entry port_tgid/modid format. 
 * Parameters:
 *      unit     - (IN) SOC unit number.
 *      port     - (IN) Port/Trunk.
 *      module   - (IN) Module id.
 *      nh_entry - (IN/OUT) Updated next hop entry. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_map_hw_data_to_api(int unit, int port,
                                int mod, bcm_l3_egress_t *nh_entry)
{
    int port_out;               /* Calculated port value */
    int modid_out;              /* Calculated module id. */

    if (nh_entry->flags & BCM_L3_TGID) {
        /* Port is trunk. */
        nh_entry->module = 0;
#ifdef BCM_TRX_SUPPORT
        if (soc_feature(unit, soc_feature_trunk_group_overlay)) {
            nh_entry->trunk = port;
        } else 
#endif /* BCM_TRIUMPH_SUPPORT */
        {
            nh_entry->trunk = BCM_MODIDf_TGIDf_TO_TRUNK(unit, mod, port);
        }
    } else {
        /* Regular port. */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    mod, port, &modid_out, &port_out));

        nh_entry->module = modid_out;
        nh_entry->port = port_out;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_nh_map_api_data_to_hw
 * Purpose:
 *      Convert API space nh entry format to HW space nh entry.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      nh_entry  - (IN/OUT)Next hop entry data.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_map_api_data_to_hw(int unit, bcm_l3_egress_t *nh_entry)
{
    int port_out;               /* Calculated port value */
    int modid_out;              /* Calculated module id. */

    if (NULL == nh_entry)
        return (BCM_E_PARAM);

    /* convert API space numbers to HW space numbers */
    if ((nh_entry->flags & BCM_L3_TGID) == 0) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    nh_entry->module, nh_entry->port,
                                    &modid_out, &port_out));
        if (!SOC_MODID_ADDRESSABLE(unit, modid_out)) {
            return (BCM_E_BADID);
        }
        if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
            return (BCM_E_PORT);
        }
    } else {
#ifdef BCM_TRX_SUPPORT
        if (soc_feature(unit, soc_feature_trunk_group_overlay)) {
            modid_out = 0;
            port_out = nh_entry->trunk;
        } else
#endif /* BCM_TRX_SUPPORT */
        {
            /* Map trunk id to mod/port pair */
            modid_out = BCM_TRUNK_TO_MODIDf(unit, nh_entry->trunk);
            port_out = BCM_TRUNK_TO_TGIDf(unit, nh_entry->trunk);
        }
    }
    /* Preserve converted values in the structure. */
    /* NOTE: HW has a single field for port/trunk  */
    /*       hence port only used for hw calls.    */
    nh_entry->trunk  = 0;
    nh_entry->port   = port_out;  
    nh_entry->module = modid_out;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_enable
 * Purpose:
 *      Enable/disable L3 function.
 * Parameters:
 *      unit -   (IN)SOC PCI unit number.
 *      enable - (IN)TRUE: enable L3 support.
 *                   FALSE: disable L3 support.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_enable(int unit, int enable)
{
    int port;                   /* Port iterator.  */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Iterate over all the ports. */
    PBMP_PORT_ITER(unit, port) {
        /* Set l3 enable to the hw. */
        bcm_esw_port_control_set(unit, port, bcmPortControlIP4, enable);
        bcm_esw_port_control_set(unit, port, bcmPortControlIP6, enable);
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_tbl_dma
 * Purpose:
 *      
 * Parameters:
 *      unit         -(IN)SOC unit number. 
 *      tbl_mem      -(IN)Table memory. 
 *      tbl_entry_sz -(IN)Table entry size. 
 *      descr        -(IN)Table descriptor.    
 *      res_ptr      -(OUT) Allocated pointer filled with table info  
 *      entry_count  -(OUT) Table entry count. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_tbl_dma(int unit, soc_mem_t tbl_mem, uint16 tbl_entry_sz,
                     const char *descr, char **res_ptr, int *entry_count)
{
    int alloc_size;             /* Allocation size.          */
    char *buffer;               /* Buffer to read the table. */
    int tbl_size;               /* HW table size.            */

    /* Input parameters sanity check. */
    if ((NULL == res_ptr) || (NULL == descr)) {
        return (BCM_E_PARAM);
    }

    /* If entry size is not deterministic. reject. */
    if (tbl_entry_sz == BCM_XGS3_L3_INVALID_ENT_SZ) {
        return (BCM_E_UNAVAIL);
    }

    if (INVALIDm == tbl_mem) {
        return (BCM_E_NOT_FOUND);
    }

    /* Calculate table size. */
    tbl_size =  soc_mem_index_count(unit, tbl_mem);
    if (!tbl_size) {
        return (BCM_E_NOT_FOUND);
    } else if (NULL != entry_count) {
        *entry_count = tbl_size;
    }
    alloc_size = tbl_entry_sz * tbl_size;

    /* Allocate memory buffer. */
    buffer = soc_cm_salloc(unit, alloc_size, descr);
    if (buffer == NULL) {
        return (BCM_E_MEMORY);
    }

    /* Reset allocated buffer. */
    memset(buffer, 0, alloc_size);

    /* Read table to the buffer. */
    if (soc_mem_read_range(unit, tbl_mem, MEM_BLOCK_ANY,
                           soc_mem_index_min(unit, tbl_mem),
                           soc_mem_index_max(unit, tbl_mem), buffer) < 0) {
        soc_cm_sfree(unit, buffer);
        return (BCM_E_INTERNAL);
    }

    *res_ptr = buffer;
    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_xgs3_l3_intf_init
 * Purpose:
 *      Initialize L3 tables.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_init(int unit)
{
    int mem_sz;   /* Allocated memory size.     */
#if defined(BCM_EASYRIDER_SUPPORT)
    int idx;      /* Interface iteration index. */
#endif /* BCM_EASYRIDER_SUPPORT */

    BCM_XGS3_L3_IF_TBL_SIZE(unit) = 
        soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, intf));

    /* Keep track of L3 interfaces */
    mem_sz = SHR_BITALLOCSIZE(BCM_XGS3_L3_IF_TBL_SIZE(unit));
    BCM_XGS3_L3_ALLOC(BCM_XGS3_L3_IF_INUSE(unit), mem_sz, "l3_intf");
    if (NULL == BCM_XGS3_L3_IF_INUSE(unit)) {
        return (BCM_E_MEMORY);
    }

    /* Keep track of L3 interfaces */
    BCM_XGS3_L3_ALLOC(BCM_XGS3_L3_IF_ADD2ARL(unit), mem_sz, "l3_intf_arl");
    if (NULL == BCM_XGS3_L3_IF_ADD2ARL(unit)) {
        return (BCM_E_MEMORY);
    }

#if defined(BCM_EASYRIDER_SUPPORT)
    /*
     * For Easyrider, reserve 8 L3_INTF entries for MPLS incoming interfaces.
     */
    if (SOC_IS_EASYRIDER(unit)) {
        for (idx = BCM_XGS3_L3_MPLS_INTF_IDX_MIN(unit); 
             idx <= BCM_XGS3_L3_MPLS_INTF_IDX_MAX(unit); idx++) {
            BCM_L3_INTF_USED_SET(unit, idx);
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_ing_intf_init
 * Purpose:
 *      Initialize ingress l3 interface table .
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_ing_intf_init(int unit)
{
    int mem_sz;                  /* Allocated memory size.               */
    int idx;                     /* Reserved interfaces iteration index. */
#if defined(BCM_TRIUMPH_SUPPORT) 
    _bcm_l3_ingress_intf_t iif;  /* Ipmc interfaces identity map.        */
    int rv;                      /* Operation return status.             */
#endif /* BCM_TRIUMPH_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT) 
    sal_memset(&iif, 0, sizeof(_bcm_l3_ingress_intf_t));
#endif /* BCM_TRIUMPH_SUPPORT */

    if (INVALIDm != BCM_XGS3_L3_MEM(unit, ing_intf)) { 
        BCM_XGS3_L3_ING_IF_TBL_SIZE(unit) = 
            soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, ing_intf));
    } else {
        BCM_XGS3_L3_ING_IF_TBL_SIZE(unit) = 0;
        return (BCM_E_NONE);
    }

    /* Keep track of L3 interfaces */
    mem_sz = SHR_BITALLOCSIZE(BCM_XGS3_L3_ING_IF_TBL_SIZE(unit));
    BCM_XGS3_L3_ALLOC(BCM_XGS3_L3_ING_IF_INUSE(unit), mem_sz, "l3_iif");
    if (NULL == BCM_XGS3_L3_ING_IF_INUSE(unit)) {
        return (BCM_E_MEMORY);
    }

    /* First 4K interfaces are reserved for per vlan interface use. */
   if (!(BCM_XGS3_L3_INGRESS_INTF_MAP_MODE_ISSET(unit))) {
        for (idx = 0; idx < BCM_VLAN_INVALID; idx++) {
            SHR_BITSET(BCM_XGS3_L3_ING_IF_INUSE(unit), idx);
#if defined(BCM_TRIUMPH_SUPPORT) 
            if (SOC_IS_TR_VL(unit)) {
                iif.intf_id = idx;
                iif.ipmc_intf_id = idx;
                rv = _bcm_tr_l3_ingress_interface_set(unit, &iif);
                BCM_IF_ERROR_RETURN(rv);
            }
#endif /* BCM_TRIUMPH_SUPPORT */
        }
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_l3_nh_init
 * Purpose:
 *      Initialize L3next hop table.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      Number of L3 entries.
 */
STATIC int
_bcm_xgs3_l3_nh_init(int unit)
{
    int mem_sz;                 /* Allocation memory size. */
    _bcm_l3_tbl_t *tbl_ptr;     /* Next hop table pointer. */

    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);
    /* Get first next hop index.  */
    /* NOTE: First next hop index is reserved for LPM/HOST lookup failure. */ 
    tbl_ptr->idx_min = soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, nh)) + 1;
    /* Get last next hop index. */
    tbl_ptr->idx_max = soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, nh));
    /* Set next hop max used to first next hop entry.(table empty) */
    tbl_ptr->idx_maxused = tbl_ptr->idx_min;

    /* Return next hop table size. */
    BCM_XGS3_L3_NH_TBL_SIZE(unit) = 
        soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, nh));

    /*
     * Keep track of Next Hop table usage.
     */
    mem_sz = BCM_XGS3_L3_NH_TBL_SIZE(unit) * sizeof(_bcm_l3_tbl_ext_t);
    BCM_XGS3_L3_ALLOC(BCM_XGS3_L3_TBL(unit, next_hop).ext_arr, mem_sz, "l3_nh");
    if (NULL == BCM_XGS3_L3_TBL(unit, next_hop).ext_arr) {
        return (BCM_E_MEMORY);
    }
    return (BCM_E_NONE);
}

#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_2                SOC_SCACHE_VERSION(1,2)
#define BCM_WB_VERSION_1_1                SOC_SCACHE_VERSION(1,1)
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_2

/*
 * _bcm_xgs3_l3_l3table_reinit
 *
 * Updates l3_v4mc_added and l3_v6mc_added counts
 *
 */
STATIC int
_bcm_xgs3_l3_l3table_reinit(int unit)
{
    int i;                        /* Iteration index.              */
    int index_min;                /* First entry index.            */
    int index_max;                /* Last  entry index.            */
    int rv;                       /* Operation return value.       */
    _bcm_l3_cfg_t l3_cfg;         /* Extracted IPMC entry 	   */
#if defined(BCM_HAWKEYE_SUPPORT)
    if (SOC_IS_HAWKEYE(unit)) {
        return BCM_E_NONE;
    }
#endif
    /* Make sure hw call pointer was initialized. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_get_by_idx)) {
        return (BCM_E_UNAVAIL);
    }

    /* First do IPv4MC table */   
    index_max = soc_mem_index_max(unit,  BCM_XGS3_L3_MEM(unit, ipmc_v4));
    index_min = soc_mem_index_min(unit,  BCM_XGS3_L3_MEM(unit, ipmc_v4));

    for (i = index_min; i <= index_max; i++) {
        l3_cfg.l3c_flags = 0;
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_get_by_idx)(unit, NULL, i, 
                                                                &l3_cfg);
        if (rv == BCM_E_NOT_FOUND || rv == BCM_E_NONE) {
            continue;
        } else if (BCM_FAILURE(rv)) {
            return (rv);
        }
        BCM_XGS3_L3_IP4_IPMC_CNT(unit)++;
    }
 
    /* Now IPv6MC table */
    index_max = soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, ipmc_v6));
    index_min = soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, ipmc_v6));
 
    for (i = index_min; i <= index_max; i++) {
        l3_cfg.l3c_flags = BCM_L3_IP6;
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_get_by_idx)(unit, NULL, i,
                                                               &l3_cfg);
        if (rv == BCM_E_NOT_FOUND || rv == BCM_E_NONE) {
           continue;
        } else if (BCM_FAILURE(rv)) {
            return (rv);
        }
        BCM_XGS3_L3_IP6_IPMC_CNT(unit)++;
    }
    return BCM_E_NONE;
}

STATIC int
_bcm_xgs3_l3_nh_reinit(int unit, int egressMode)
{
    int idx;                      /* Iteration index.              */
    int index_min;                /* First interface entry index.  */
    int index_max;                /* Last interface entry index.   */
    int ret_val;                  /* Operation return value.       */
    int nh_idx;                   /* Next hop index from L3        */
    bcm_l3_egress_t nh_info;      /* Next hop (egress) info        */
    _bcm_l3_tbl_t *tbl_ptr;       /* Next hop table pointer.       */
    _bcm_l3_cfg_t l3_cfg;	  /* Extracted L3 entry            */
    bcm_mac_t     mac;
#if defined(BCM_HAWKEYE_SUPPORT)
    _bcm_rp_l3_data_t *rp_tbl_ptr;
#endif
    /* Make sure hw call pointer was initialized. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, nh_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Get next hop table start & end index. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);    
    tbl_ptr->idx_min = index_min =
        soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, nh)) + 1;
    tbl_ptr->idx_max = index_max =
        soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, nh));
    tbl_ptr->idx_maxused = 0;

/*    BCM_XGS3_L3_NH_TBL_SIZE(unit) = index_max - index_min + 1;*/
    BCM_XGS3_L3_NH_TBL_SIZE(unit) = 
        soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, nh));

   sal_memset (mac, 0, sizeof(bcm_mac_t));
    /* Loop over all valid next hop entries. */
    for (idx = index_min; idx <= index_max; idx++) {
        bcm_l3_egress_t_init(&nh_info);
        ret_val = BCM_XGS3_L3_HWCALL_EXEC(unit, nh_get) (unit, idx, &nh_info);
        BCM_IF_ERROR_RETURN(ret_val);
       if (!sal_memcmp(nh_info.mac_addr, mac, sizeof(bcm_mac_t))) {
            continue;
        }
        if (idx == index_min) {
            /* This corresponds to the CPU */
            BCM_XGS3_L3_ENT_HASH(tbl_ptr, idx) = 0;
        } else {
            /* Compute hash normally */
            _bcm_xgs3_nh_map_api_data_to_hw(unit, &nh_info);	
            _bcm_xgs3_nh_hash_calc(unit, &nh_info,
                                   &BCM_XGS3_L3_ENT_HASH(tbl_ptr, idx));

            /* Update hash & reference count info. */
            if (egressMode) {
                BCM_XGS3_L3_ENT_INIT(tbl_ptr, idx, _BCM_SINGLE_WIDE, BCM_XGS3_L3_ENT_HASH(tbl_ptr, idx));
            }
        }	    
        tbl_ptr->idx_maxused ++;
    }

    /*
     * NH table is referenced to from three different tables
     * DEFIP, L3_ENTRY_IPV4 and L3_ENTRY_IPV6; In this routine,
     * we update NH software state based on references from
     * L3_ENTRY_IPV4 and L3_ENTRY_IPV6; References from DEFIP
     * table will be accounted for in ecmp_reinit() (See below)
     * Recover nh usecount from IPv4 unicast L3 table
     */

#if defined(BCM_HAWKEYE_SUPPORT)
    /*
     * Update NH table reference count with software DEFIP table.
     */ 
    if (SOC_IS_HAWKEYE(unit)) {
        /* Get l3 prefixes table. */
        rp_tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

        /* Iterate over all host rules try to match passed address. */ 
        for (idx = 0; idx < BCM_XGS3_L3_RP_MAX_PREFIXES(unit); idx++) {
            if (!(rp_tbl_ptr->l3_arr[idx].flags & BCM_XGS3_L3_ENT_VALID)) {
                continue;
            }
            BCM_XGS3_L3_ENT_REF_CNT_INC(tbl_ptr, 
                                        rp_tbl_ptr->l3_arr[idx].nh_idx,
                                        _BCM_SINGLE_WIDE);
        }
        return BCM_E_NONE;
    }
#endif

    index_min = soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, v4));
    index_max = soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, v4)); 
    /* Loop over all valid IPV4 unicast entries to get nh use count. */
    for (idx = index_min; idx <= index_max; idx++) {
        l3_cfg.l3c_flags = 0;    
        ret_val = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get_by_idx)(unit, NULL, idx,
                                                               &l3_cfg, &nh_idx);
        if (ret_val == BCM_E_NOT_FOUND) {
            continue;
        } else if (BCM_FAILURE(ret_val)) {
            return (ret_val);
        }
        if ((l3_cfg.l3c_flags & BCM_L3_IP6) == BCM_L3_IP6) {
            continue;
        }
        if ((l3_cfg.l3c_flags & BCM_L3_IPMC) == BCM_L3_IPMC) {
            continue;
        }
        BCM_XGS3_L3_IP4_CNT(unit)++;
        BCM_XGS3_L3_ENT_REF_CNT_INC(tbl_ptr, nh_idx, _BCM_SINGLE_WIDE);
    }

    index_min = soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, v6));
    index_max = soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, v6));
    /*
     * Loop over all valid IPV6 unicast entries to get nh use count.
     */
    for (idx = index_min; idx <= index_max; idx++) {
        l3_cfg.l3c_flags = BCM_L3_IP6;    
        ret_val = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get_by_idx)(unit, NULL, idx,
                                                               &l3_cfg, &nh_idx);
        if (ret_val == BCM_E_NOT_FOUND) {
            continue;
        } else if (BCM_FAILURE(ret_val)) {
            return (ret_val);
        }
        if ((l3_cfg.l3c_flags & BCM_L3_IP6) != BCM_L3_IP6) {
            continue;
        }	
        if ((l3_cfg.l3c_flags & BCM_L3_IPMC) == BCM_L3_IPMC) {
            continue;
        }              
        BCM_XGS3_L3_IP6_CNT(unit)++;
        BCM_XGS3_L3_ENT_REF_CNT_INC(tbl_ptr, nh_idx, _BCM_SINGLE_WIDE);
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_xgs3_l3_intf_reinit(int unit)
{
    int idx;                      /* Iteration index.              */
    int index_min;                /* First interface entry index.  */
    int index_max;                /* Last interface entry index.   */
    _bcm_l3_intf_cfg_t intf_info; /* Interface configuration.      */ 
    bcm_tunnel_initiator_t tnl_info;/* Tunnel initiator struct. */
    bcm_l2_addr_t l2_addr;        /* Layer 2 address for interface.*/
    int ret_val;                  /* Operation return value.       */
    int entry_width;              /* Tunnel entry width.           */
    bcm_mac_t  mac;

    /* Make sure hw call pointer was initialized. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, if_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Get interface table start & end index. */
    index_min = soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, intf));
    index_max = soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, intf));
    BCM_XGS3_L3_IF_TBL_SIZE(unit) = index_max - index_min + 1;
    sal_memset (mac, 0, sizeof(bcm_mac_t));

    /* Loop over all valid interfaces. */
    for (idx = index_min; idx <= index_max; idx++) {

        /* Read interface from hardware. */
        intf_info.l3i_index = idx;
        ret_val = BCM_XGS3_L3_HWCALL_EXEC(unit, if_get) (unit, &intf_info);
        if (BCM_FAILURE(ret_val)) {
            return (ret_val);
        }

        if (!sal_memcmp(intf_info.l3i_mac_addr, mac, sizeof(bcm_mac_t))){
            continue;
        }

        if (!BCM_L3_INTF_USED_GET(unit, idx)) {
            /* Mark interface as used */
            BCM_L3_INTF_USED_SET(unit, idx); 
        }

        sal_memset(&l2_addr, 0, sizeof(bcm_l2_addr_t));
        /* Check if L2 entry is installed for interface. */ 
        if (bcm_esw_l2_addr_get(unit, intf_info.l3i_mac_addr, 
                            intf_info.l3i_vid, &l2_addr) >= 0) {
            if (l2_addr.flags & BCM_L2_L3LOOKUP) {
                BCM_L3_INTF_ARL_SET(unit, idx);
            }
        }

        /* Reference counts & signatures for tnl_init and adj_mac tables. */
        if (intf_info.l3i_tunnel_idx > 0) {
            sal_memset(&tnl_info, 0, sizeof(bcm_tunnel_initiator_t));	
            ret_val = bcm_xgs3_tunnel_initiator_get(unit, (bcm_l3_intf_t *)
                                                    &intf_info, &tnl_info);
            if (BCM_FAILURE(ret_val)) {
                return (ret_val);
            }
            _bcm_xgs3_tnl_init_hash_calc(unit, &tnl_info,
                          &BCM_XGS3_L3_ENT_HASH(BCM_XGS3_L3_TBL_PTR(unit, tnl_init),
                                                intf_info.l3i_tunnel_idx));

            entry_width = (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_info.type)) ? \
                           _BCM_DOUBLE_WIDE : _BCM_SINGLE_WIDE; 

            BCM_XGS3_L3_ENT_REF_CNT_INC (BCM_XGS3_L3_TBL_PTR(unit, tnl_init), 
                                         intf_info.l3i_tunnel_idx, entry_width); 
                                         
        }
    }   /* Loop over interface table. */

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_defip_reinit
 * Purpose:
 *      Recover basic DEFIP table info
 * Parameters:
 *      unit  - SOC unit number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_defip_table_reinit(int unit)
{
    int index_min = 0;                /* First entry index.        */
    int index_max = 0;                /* Last  entry index.        */
    int rv;                       /* Operation return status.  */     

#if defined(BCM_HAWKEYE_SUPPORT)
    if (SOC_IS_HAWKEYE(unit)) {
        BCM_XGS3_L3_DEFIP_TBL_SIZE(unit) = BCM_XGS3_L3_RP_MAX_PREFIXES(unit);
        return (BCM_E_NONE); 
    }
#endif

    if (soc_feature(unit, soc_feature_esm_support)) {
#ifdef BCM_TRIUMPH_SUPPORT
        soc_mem_t mem_v4;      /* IPv4 Route table memory.             */

        BCM_IF_ERROR_RETURN(_bcm_tr_l3_defip_mem_get(unit, 0, 0, &mem_v4));
        index_min = soc_mem_index_min(unit, mem_v4);
        index_max = soc_mem_index_max(unit, mem_v4);
#endif
    } else {
        index_min = soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, defip));
        index_max = soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, defip));
    }

    BCM_XGS3_L3_DEFIP_TBL_SIZE(unit) = index_max - index_min + 1;

    /* Traverse the defip table */ 
    rv = bcm_xgs3_defip_traverse(unit, 0, index_min, index_max, NULL, NULL);
    if (BCM_FAILURE(rv)) {
       return (rv);
    } 

    if (soc_feature(unit, soc_feature_esm_support)) {
#ifdef BCM_TRIUMPH_SUPPORT
        soc_mem_t mem_v6;      /* IPv6 Route table memory.             */

        BCM_IF_ERROR_RETURN
            (_bcm_tr_l3_defip_mem_get(unit, BCM_L3_IP6, 0, &mem_v6));
        index_min = soc_mem_index_min(unit, mem_v6);
        index_max = soc_mem_index_max(unit, mem_v6);
#endif
    } 

    rv = bcm_xgs3_defip_traverse(unit, BCM_L3_IP6, index_min, 
                                 index_max, NULL, NULL);
    if (BCM_FAILURE(rv)) {
       return (rv);
    }

    return (rv);
}

#if defined(BCM_TRIUMPH2_SUPPORT)
/*
 * Function:
 *      _bcm_xgs3_l3_flexstat_recover
 * Purpose:
 *      Recover VRF counter table info
 * Parameters:
 *      unit  - SOC unit number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_flexstat_recover(int unit)
{
    int index_min;                /* First entry index.        */
    int index_max;                /* Last  entry index.        */
    int vrf;                      /* VRF table index           */
    int fs_idx;                   /* Service counter index     */
    int rv = BCM_E_NONE;          /* Operation return status.  */
    vrf_entry_t vrf_entry;        /* VRF memory entry          */

    index_min = soc_mem_index_min(unit, VRFm);
    index_max = soc_mem_index_max(unit, VRFm);

    /* Traverse the VRF table */
    for (vrf = index_min; vrf <= index_max; vrf++) {
        rv = READ_VRFm(unit, MEM_BLOCK_ANY, vrf, &vrf_entry);
        if (BCM_FAILURE(rv)) {
            break;
        }
        fs_idx = soc_VRFm_field32_get(unit, &vrf_entry, SERVICE_CTR_IDXf);
        if (fs_idx) {
            _bcm_esw_flex_stat_reinit_add(unit,
                                _bcmFlexStatTypeVrf, fs_idx, vrf);
        }
    }

    return (rv);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      _bcm_esw_l3_mode_recover
 * Purpose:
 *      Retrieve L3 egress mode (also ingress mode if applicable)
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_l3_mode_recover(int unit, int32 *egrmode, int32 *ingrmode, int32 *hostmode, int32 *intfMapmode)
{
    /*
     * Match with _bcm_esw_l3_warm_boot_alloc/_bcm_esw_l3_sync
     */
    uint8 *l3_scache_ptr;
    uint16 recovered_ver;
    soc_scache_handle_t scache_handle;
    int alloc_sz = sizeof(int32) * 2, stable_size = 0, rv;

    *egrmode = 0; /* Default values */
    *ingrmode = 0;
    *hostmode = 0;
    *intfMapmode = 0;

    SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
    if (stable_size > 0) { /* Limited and Extended recovery only */

        if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
            alloc_sz  += sizeof(int32); /* Ingress mode */
            alloc_sz  += sizeof(int32); /* IntfMap mode */
        }
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_L3, 0);
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                     alloc_sz, &l3_scache_ptr, 
                                     BCM_WB_DEFAULT_VERSION, &recovered_ver);

        if (BCM_SUCCESS(rv)) {
            /* Common to versions 1.1 and 1.0 */
            sal_memcpy(egrmode, l3_scache_ptr, sizeof(*egrmode));
            l3_scache_ptr += sizeof(*egrmode);
            if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
                sal_memcpy(ingrmode, l3_scache_ptr, sizeof(*ingrmode));
                l3_scache_ptr += sizeof(*ingrmode);
            }

            if (recovered_ver >= BCM_WB_VERSION_1_1) {
                sal_memcpy(hostmode, l3_scache_ptr, sizeof(*hostmode));
                l3_scache_ptr += sizeof(*hostmode);
            }
      
            if (recovered_ver >= BCM_WB_VERSION_1_2) {
                if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
                   sal_memcpy(intfMapmode, l3_scache_ptr, sizeof(*intfMapmode));
                   l3_scache_ptr += sizeof(*intfMapmode);
                }
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs3_l3_reinit
 * Purpose:
 *      Recovers L3 sw state from hw tables
 * Parameters:
 *      unit  - SOC unit number
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_reinit(int unit)
{
    /*
     * Get L3 mode(s)
     */
    int32 l3EgressMode = 0, l3IngressMode = 0, l3HostAddMode=0, l3IntfMapMode=0;
    BCM_IF_ERROR_RETURN(_bcm_esw_l3_mode_recover(unit,
                                     &l3EgressMode, &l3IngressMode, &l3HostAddMode, &l3IntfMapMode));
    /*
     * Init L3 counters. 
     */     
    BCM_XGS3_L3_CNTRS_RESET(unit);    

    /*
     * Init have table sizes & hw callbacks. 
     */ 
    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_tables_init(unit));

#ifdef BCM_TRIUMPH_SUPPORT
   /*
    * Recover Ingress Mode
    */
    if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
        BCM_IF_ERROR_RETURN(bcm_xgs3_l3_ingress_mode_set(unit, l3IngressMode));
    } 
#endif /* BCM_TRIUMPH_SUPPORT */

    /*
     * Recover Egress Mode
     */
    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_egress_mode_set(unit, l3EgressMode));

    /*
     * Recover Host Add Mode
     */
    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_host_as_route_return_set(unit, l3HostAddMode));

#ifdef BCM_TRIUMPH_SUPPORT
   /*
    * Recover Intf Map mode
    */
    if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
        BCM_IF_ERROR_RETURN(bcm_xgs3_l3_ingress_intf_map_set(unit, l3IntfMapMode));
    } 
#endif /* BCM_TRIUMPH_SUPPORT */

    /*
     * Recover L3 Intf sw state
     */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_intf_reinit(unit));

    /* 
     * Recover next hop table. 
     */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_nh_reinit(unit, l3EgressMode));
  	 
    /* 
     * Recover ecmp groups. 
     */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_reinit(unit));

    /*
     * Recover l3 entries(host) table.
     */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_l3table_reinit(unit));

    /*
     * Recover defip table. 
     */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_defip_table_reinit(unit));

    /*
     * Recover VRF table. 
     */
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_gport_service_counters)) {
         BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_flexstat_recover(unit));
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    /*
     * Recover tunnel terminator table. 
     */
    BCM_IF_ERROR_RETURN(soc_tunnel_term_reinit(unit));  

    return (BCM_E_NONE);
}
#endif /* BCM_WARM_BOOT_SUPPORT */


/*
 * Function:
 *      _bcm_xgs3_l3_table_init
 * Purpose:
 *      Initialize L3 tables.
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      Number of L3 entries.
 */
STATIC int
_bcm_xgs3_l3_table_init(int unit)
{
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        BCM_XGS3_L3_TBL_SIZE(unit) = BCM_XGS3_L3_RP_MAX_PREFIXES(unit);
        return (BCM_E_NONE); 
    }
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */

    /* Get table L3 host table size. */
    BCM_XGS3_L3_TBL_SIZE(unit) = 
        soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, v4));

    return (BCM_E_NONE);
}

#ifdef BCM_FIREBOLT_SUPPORT  
/*
 * Function:
 *      bcm_xgs3_l3_fbx_defip_init
 * Purpose:
 *      Initialize L3 DEFIP table for fbx devices.
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_fbx_defip_init(int unit)
{
#if defined (BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) || \
    defined (BCM_RAVEN_SUPPORT)
    uint32 reg_val;    /* Buffer to store urpf register value.*/

    /* NOTE: URPF enable check  must be done before   */
    /*  specific route table initialization.          */

    /* URPF support check. */
    if (SOC_REG_IS_VALID(unit, L3_DEFIP_RPF_CONTROLr)) {
        BCM_IF_ERROR_RETURN(READ_L3_DEFIP_RPF_CONTROLr(unit, &reg_val));

        if (soc_reg_field_get(unit, L3_DEFIP_RPF_CONTROLr, reg_val,
                              DEFIP_RPF_ENABLEf)) {
            SOC_URPF_STATUS_SET(unit, TRUE); 
        } else {
            SOC_URPF_STATUS_SET(unit, FALSE); 
        }
    }
#else /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
    SOC_URPF_STATUS_SET(unit, FALSE); 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */

#if defined(BCM_SCORPION_SUPPORT) 
    if (SOC_IS_SCORPION(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_sc_defip_init(unit));
    } else
#endif /* BCM_SCORPION_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT) 
    if (SOC_IS_TR_VL(unit) && !SOC_IS_HURRICANE(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_tr_defip_init(unit));
    } else
#endif /* BCM_TRIUMPH_SUPPORT*/
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        return (BCM_E_NONE);
    } else
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
    {
        /* Init prefixes offsets, hash/avl - lookup engine. */
        BCM_IF_ERROR_RETURN(soc_fb_lpm_init(unit));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_fbx_defip_deinit
 * Purpose:
 *      De-initialize L3 DEFIP table for fbx devices.
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_fbx_defip_deinit(int unit)
{

#if defined(BCM_SCORPION_SUPPORT) 
    if (SOC_IS_SCORPION(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_sc_defip_deinit(unit));
    } else
#endif /* BCM_SCORPION_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT) 
    if (SOC_IS_TR_VL(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_tr_defip_deinit(unit));
    } else
#endif /* BCM_TRIUMPH_SUPPORT*/
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        return (BCM_E_NONE);
    } else
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
    {
        /* Init prefixes offsets, hash/avl - lookup engine. */
        BCM_IF_ERROR_RETURN(soc_fb_lpm_deinit(unit));
    }
    return (BCM_E_NONE);
}
#endif /* BCM_FIREBOLT_SUPPORT */

/*
 * Function:
 *      _bcm_xgs3_defip_table_init
 * Purpose:
 *      Initialize L3 DEFIP tables.
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      Number of DEFIP entries.
 */
STATIC int
_bcm_xgs3_defip_table_init(int unit)
{
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        BCM_XGS3_L3_DEFIP_TBL_SIZE(unit) = 
            BCM_XGS3_L3_RP_MAX_PREFIXES(unit);
        return (BCM_E_NONE); 
    }
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
    /* Get table size. */
    BCM_XGS3_L3_DEFIP_TBL_SIZE(unit) = 
        soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, defip));

#ifdef BCM_FIREBOLT_SUPPORT  
    if (SOC_IS_FBX(unit)) {
        BCM_IF_ERROR_RETURN(bcm_xgs3_l3_fbx_defip_init(unit));
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit) &&
       (L3_DEFIP_ALGm == BCM_XGS3_L3_MEM(unit, defip))) {
        _bcm_defip_cfg_t lpm_cfg;
        int vrf;

        /* Make sure hw call is defined. */
        if (!BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_del)) {
            return (BCM_E_UNAVAIL);
        }

        sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));
        /* Update ipv6  default route . */
        lpm_cfg.defip_flags = BCM_L3_IP6;
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_del) (unit, &lpm_cfg));

        /* Update ipv4  default route for every vrf. */
        lpm_cfg.defip_flags = 0;
        for (vrf = 0; vrf <= SOC_VRF_MAX(unit); vrf++) {
            lpm_cfg.defip_vrf = vrf;
            BCM_IF_ERROR_RETURN
                (BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_del) (unit, &lpm_cfg));
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_initiator_init
 * Purpose:
 *      Initiate the tunnel terminator table.
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_initiator_init(int unit)
{
    int mem_sz;                 /* Memory allocation size. */
    _bcm_l3_tbl_t *tbl_ptr;     /* Tunnel initiator table. */

    /* Get table pointer. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, tnl_init);

    /* Get L3 tunnel table first & last index. */
    tbl_ptr->idx_min = soc_mem_index_min(unit, 
                                         BCM_XGS3_L3_MEM(unit, tnl_init_v4));
    tbl_ptr->idx_max = soc_mem_index_max(unit, 
                                         BCM_XGS3_L3_MEM(unit, tnl_init_v4));

    /* Set max used tunnel to first one (table is empty). */
    tbl_ptr->idx_maxused = tbl_ptr->idx_min;

    BCM_XGS3_L3_TUNNEL_TBL_SIZE(unit) = tbl_ptr->idx_max - tbl_ptr->idx_min + 1;

    /*
     * Keep track of tunnel initiator table usage.
     * Table size doesn't include first reserved entry. 
     */
    mem_sz = (BCM_XGS3_L3_TUNNEL_TBL_SIZE(unit) + 1) *
        sizeof(_bcm_l3_tbl_ext_t);
    BCM_XGS3_L3_ALLOC(BCM_XGS3_L3_TBL(unit, tnl_init).ext_arr, 
                         mem_sz, "l3_tnl_init");
    if (NULL == BCM_XGS3_L3_TBL(unit, tnl_init).ext_arr) {
        return (BCM_E_MEMORY);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_dscp_init
 * Purpose:
 *      Initiate the tunnel dscp table.
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_dscp_init(int unit)
{
    int mem_sz;          /* Allocated memory size. */
    int tnl_dscp_tbl_sz; /* Dscp map entry count. */

    tnl_dscp_tbl_sz = soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, tnl_dscp));

    /* Keep track of L3 tunnel dscp map IDs based on 
     * the number of DSCP map table instances. Each
     * instance has (#prio * #color) entries. For Firebolt/Easyrider
     * there are 8 priorities and 4 colors.
     */
    BCM_XGS3_L3_TUNNEL_DSCP_MAP_TBL_SIZE(unit) = 
        SOC_IS_TRX(unit) ? (1) : tnl_dscp_tbl_sz / (8 * 4);
    mem_sz = SHR_BITALLOCSIZE(BCM_XGS3_L3_TUNNEL_DSCP_MAP_TBL_SIZE(unit));
    BCM_XGS3_L3_ALLOC(BCM_XGS3_L3_TUNNEL_DSCP_MAP_INUSE(unit), mem_sz,
                         "l3_dscp_map");
    if (NULL == BCM_XGS3_L3_TUNNEL_DSCP_MAP_INUSE(unit)) {
        return (BCM_E_MEMORY);
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_l3_adj_mac_init
 * Purpose:
 *      Initialize L3 tables.
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_adj_mac_init(int unit)
{
#ifdef BCM_EASYRIDER_SUPPORT
    int adj_mac_tbl_sz;    /* Adjacent mac table size.*/
    int mem_sz;            /* Memory allocation size. */

    if (SOC_IS_EASYRIDER(unit)) {
        _bcm_l3_tbl_t *tbl_ptr;     /* Adjacent mac table pointer. */

        /* Get table pointer. */
        tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, adj_mac);

        /* Get L3 adjacent mac table first index. */
        tbl_ptr->idx_min =
            soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, adj_mac));
        /* Get L3 adjacent mac table last index. */
        tbl_ptr->idx_max =
            soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, adj_mac));
        /* Set L3 adjacent mac table max used to a first entry(table empty). */
        tbl_ptr->idx_maxused = tbl_ptr->idx_min;

        /* Return total number of entries. */
        adj_mac_tbl_sz = soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, adj_mac));

        /* Keep track of Adjacent mac (next hop) table. */
        mem_sz = adj_mac_tbl_sz * sizeof(_bcm_l3_tbl_ext_t);
        BCM_XGS3_L3_ALLOC(BCM_XGS3_L3_TBL(unit, adj_mac).ext_arr,
                             mem_sz, "l3_adj_mac");
        if (NULL == BCM_XGS3_L3_TBL(unit, adj_mac).ext_arr) {
            return (BCM_E_MEMORY);
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_free_resource
 * Purpose:
 *      Free all allocated tables and memory
 * Parameters:
 *      unit - SOC unit number
 * Returns:
 *      Nothing
 */
STATIC void
_bcm_xgs3_l3_free_resource(int unit)
{
    int rv;        /* Operation return status. */

    /* If software tables were not allocated we are done. */ 
    if (NULL == l3_module_data[unit]) {
        return;
    }

    /* Free next hop ref counts & hashes. */
    if (BCM_XGS3_L3_TBL(unit, next_hop).ext_arr) {
        sal_free(BCM_XGS3_L3_TBL(unit, next_hop).ext_arr);
        BCM_XGS3_L3_TBL(unit, next_hop).ext_arr = NULL;
    }

    /* Free tunnel initiator ref counts & hashes. */
    if (BCM_XGS3_L3_TBL(unit, tnl_init).ext_arr) {
        sal_free(BCM_XGS3_L3_TBL(unit, tnl_init).ext_arr);
        BCM_XGS3_L3_TBL(unit, tnl_init).ext_arr = NULL;
    }

    /* Free route table. */ 
    if (SOC_IS_FBX(unit)) {
        rv = bcm_xgs3_l3_fbx_defip_deinit(unit);
        if (BCM_FAILURE(rv)) {
            L3_ERR(("Route table free error %d\n", rv));
        }
    }

    if(!SOC_IS_HURRICANE(unit)) {
        /* Tunnels terminator table  deinit. */
        rv = soc_tunnel_term_deinit(unit);
        if (BCM_FAILURE(rv)) {
            L3_ERR(("Tunnel terminator table free %d\n", rv));
        }
    }

    /* Free ecmp groups ref counts & hashes. */
    if (BCM_XGS3_L3_TBL(unit, ecmp_grp).ext_arr) {
        sal_free(BCM_XGS3_L3_TBL(unit, ecmp_grp).ext_arr);
        BCM_XGS3_L3_TBL(unit, ecmp_grp).ext_arr = NULL;
    }

    /* Free interface indexes usage bits. */
    if (BCM_XGS3_L3_IF_INUSE(unit)) {
        sal_free(BCM_XGS3_L3_IF_INUSE(unit));
        BCM_XGS3_L3_IF_INUSE(unit) = NULL;
    }

    /* Free interface indexes usage bits. */
    if (BCM_XGS3_L3_ING_IF_INUSE(unit)) {
        sal_free(BCM_XGS3_L3_ING_IF_INUSE(unit));
        BCM_XGS3_L3_ING_IF_INUSE(unit) = NULL;
    }


    /* Free interface address in ARL bits. */
    if (BCM_XGS3_L3_IF_ADD2ARL(unit)) {
        sal_free(BCM_XGS3_L3_IF_ADD2ARL(unit));
        BCM_XGS3_L3_IF_ADD2ARL(unit) = NULL;
    }
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Free adjacent mac address ref counters. */
        if (BCM_XGS3_L3_TBL(unit, adj_mac).ext_arr) {
            sal_free(BCM_XGS3_L3_TBL(unit, adj_mac).ext_arr);
            BCM_XGS3_L3_TBL(unit, adj_mac).ext_arr = NULL;
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Free dscp map indexes usage bits. */
    if (BCM_XGS3_L3_TUNNEL_DSCP_MAP_INUSE(unit)) {
        sal_free(BCM_XGS3_L3_TUNNEL_DSCP_MAP_INUSE(unit));
        BCM_XGS3_L3_TUNNEL_DSCP_MAP_INUSE(unit) = NULL;
    }

#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        _bcm_rp_l3_deinit(unit);
    }
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_gport_service_counters)) {
        _bcm_esw_flex_stat_release_handles(unit, _bcmFlexStatTypeVrf);
    }
#endif

    /* Free module data. */
    sal_free(l3_module_data[unit]);
    l3_module_data[unit] = NULL;

    return;
}

/*
 * Function:
 *      _bcm_xgs3_l3_get_tunnel_id
 * Purpose:
 *      Routine extracts tunnel id based on interface id.
 * Parameters:
 *      unit      -  (IN)StrataSwitch #.
 *      ifindex   -  (IN)Interface index.
 *      tunnel    -  (OUT) tunnel_id.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_get_tunnel_id(int unit, int ifindex, int *tunnel_id)
{
    _bcm_l3_intf_cfg_t l3_intf; /* Buffer to read interface info. */
    int rv = BCM_E_UNAVAIL;     /* Operation return status.       */

    /* Input parameters check. */
    if (NULL == tunnel_id) {
        return (BCM_E_PARAM);
    }

    /* Initialization. */
    sal_memset(&l3_intf, 0, sizeof(_bcm_l3_intf_cfg_t));
    *tunnel_id = 0;

    /* Set interface index. */
    BCM_XGS3_L3_INTF_IDX_SET(l3_intf, ifindex);

    /* Read interface information. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_get)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_get) (unit, &l3_intf);
    }
    /* Check hw operation status. */
    BCM_IF_ERROR_RETURN(rv);

    /* Save interface tunnel index. */
    *tunnel_id = l3_intf.l3i_tunnel_idx;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_nh_entry_init
 * Purpose:
 *      Routine fills next hop specific info
 *      based on l3 configuration OR defip entry. 
 * Parameters:
 *      unit      -  (IN) StrataSwitch #.
 *      nh_entry  -  (OUT)Next hop pointer to fill.
 *      l3cfg     -  (IN) L3 host entry. 
 *      lpm_cfg   -  (IN) Route entry. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_entry_init(int unit, bcm_l3_egress_t *nh_entry,
                        _bcm_l3_cfg_t *l3cfg, _bcm_defip_cfg_t *lpm_cfg)
{
    int port_tgid;           /* Port/trunk id.   */ 
#ifdef BCM_FIREBOLT_SUPPORT
    _bcm_l3_intf_cfg_t l3_intf;
#endif /* BCM_FIREBOLT_SUPPORT  */

    if ((NULL == nh_entry) || ((NULL == l3cfg) && (NULL == lpm_cfg)) || 
        ((NULL != l3cfg) && (NULL != lpm_cfg))) {
        return (BCM_E_PARAM);
    }

    /* Reset next hop entry. (We will use it for hash calc. */
    bcm_l3_egress_t_init(nh_entry);

    /* Set next hop  interface index. */
    nh_entry->intf = (l3cfg) ? l3cfg->l3c_intf : lpm_cfg->defip_intf;

    /* Set next hop flags.            */
    nh_entry->flags = (l3cfg) ? l3cfg->l3c_flags : lpm_cfg->defip_flags;

    /* Set  port/trunk information.  */
    port_tgid = (l3cfg) ? l3cfg->l3c_port_tgid : lpm_cfg->defip_port_tgid;
    if (nh_entry->flags & BCM_L3_TGID) {
        nh_entry->trunk = port_tgid;
    }  else {
        nh_entry->port = port_tgid;
    }

    /* Set module id.                */
    nh_entry->module = (l3cfg) ? l3cfg->l3c_modid : lpm_cfg->defip_modid;

    /* Set next hop mac address. */
    sal_memcpy(nh_entry->mac_addr, (l3cfg) ? l3cfg->l3c_mac_addr :
               lpm_cfg->defip_mac_addr, sizeof(bcm_mac_t));

#ifdef BCM_FIREBOLT_SUPPORT
    /* Firebolt devices need interface info to fill vid & tunnel next hop. */
    if (SOC_IS_FBX(unit)) {
        sal_memset(&l3_intf, 0, sizeof(_bcm_l3_intf_cfg_t));

        /* Read interface information. */
        BCM_XGS3_L3_INTF_IDX_SET(l3_intf, nh_entry->intf);
        if (BCM_XGS3_L3_HWCALL_EXEC(unit, if_get) (unit, &l3_intf) >= 0) {
            /* Set vid & istunnel to next hop entry. */
            nh_entry->vlan = l3_intf.l3i_vid;
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT  */

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        nh_entry->mpls_label = BCM_XGS3_L3_INVALID_MPLS_LABEL;
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    return (BCM_E_NONE);
}

/*
* Purpose:
*	   Entry-0 Nexthop Entry is reserved for BLACK_HOLE
* Parameters:
*	   unit 	 -	(IN) StrataSwitch #.
* Returns:
*	   BCM_E_XXX
*/
STATIC int
_bcm_xgs3_l3_black_hole_nh_setup(int unit)
{
    bcm_l3_egress_t nh_entry; /* next hop entry. */

    bcm_l3_egress_t_init(&nh_entry);
    nh_entry.port = CMIC_PORT(unit);
    nh_entry.intf = BCM_XGS3_L3_L2CPU_INTF_IDX(unit);
    nh_entry.flags = BCM_L3_DST_DISCARD;

    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, nh_add) (unit, BCM_XGS3_L3_BLACK_HOLE_NH_IDX,
                                                &nh_entry, 0));
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, nh_add) (unit, BCM_XGS3_L3_BLACK_HOLE_NH_IDX,
                                                &nh_entry, 0));

    /* Set entry reference count. */
    BCM_XGS3_L3_ENT_INIT((BCM_XGS3_L3_TBL_PTR(unit, next_hop)), 
                         BCM_XGS3_L3_BLACK_HOLE_NH_IDX, _BCM_SINGLE_WIDE, 0);

    BCM_XGS3_L3_TBL(unit, next_hop).idx_maxused = BCM_XGS3_L3_BLACK_HOLE_NH_IDX;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_l2cpu_nh_setup     
 * Purpose:
 *      The first Nexthop Entry is reserved for packet to be L2 switched
 *      to the local CPU.  This is used when the BCM_L3_L2TOCPU flag is used
 *      in host add; or when BCM_L3_DEFIP_LOCAL/BCM_L3_DEFIP_CPU flags
 *      are used when adding routes.
 * Parameters:
 *      unit      -  (IN) StrataSwitch #.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_l2cpu_nh_setup(int unit)
{
    bcm_l3_egress_t nh_entry; /* Trap to cpu next hop entry. */
    _bcm_l3_intf_cfg_t intf_info;       /* Bridge to cpu interface.    */
    /* Zero structures. */
    sal_memset(&intf_info, 0, sizeof(_bcm_l3_intf_cfg_t));
    bcm_l3_egress_t_init(&nh_entry);

    /* Set mac address to all ones. */
    sal_memset(nh_entry.mac_addr, 0xff, sizeof(bcm_mac_t));

    /* Set module id to a local module. */
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &nh_entry.module));

    /* Set port to be a cpu port. */
    nh_entry.port = CMIC_PORT(unit);

    /* Set interface to be cpu interface. */
    nh_entry.intf = BCM_XGS3_L3_L2CPU_INTF_IDX(unit);

    /* Add next hop entry to hw. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, nh_add)) {
        return (BCM_E_UNAVAIL);
    }
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, nh_add) (unit, BCM_XGS3_L3_L2CPU_NH_IDX,
                                                &nh_entry, 0));

    /* Set entry reference count. */
    BCM_XGS3_L3_ENT_INIT((BCM_XGS3_L3_TBL_PTR(unit, next_hop)), 
                         BCM_XGS3_L3_L2CPU_NH_IDX, _BCM_SINGLE_WIDE, 0);

    if (BCM_XGS3_L3_L2CPU_NH_IDX > BCM_XGS3_L3_TBL(unit, next_hop).idx_maxused) {
        BCM_XGS3_L3_TBL(unit, next_hop).idx_maxused = BCM_XGS3_L3_L2CPU_NH_IDX;
    }

    /* Create trap to cpu interface entry. */
    /* Set mac address to all ones. */
    sal_memset(intf_info.l3i_mac_addr, 0xff, sizeof(bcm_mac_t));

    /* Set L2_SWITCHf flag to true. */
    intf_info.l3i_flags = BCM_L3_L2ONLY;

    /* Set interface index. */
    intf_info.l3i_index = BCM_XGS3_L3_L2CPU_INTF_IDX(unit);

    /* Add next hop entry to hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_add)) {
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, if_add) (unit, &intf_info));
    } else {
        return (BCM_E_UNAVAIL);
    }

    /* Mark interface index "inuse" */
	
    if (!BCM_L3_INTF_USED_GET(unit, BCM_XGS3_L3_L2CPU_INTF_IDX(unit))) {
         BCM_L3_INTF_USED_SET(unit, BCM_XGS3_L3_L2CPU_INTF_IDX(unit));
    }
    return (BCM_E_NONE);
}



/*
 * Function:
 *      _bcm_xgs3_l3_l2cpu_nh_cb     
 * Purpose:
 *      To update the first Nexthop Entry if modid has been changed after init
 * Parameters:
 *      unit      -  (IN) StrataSwitch #.
 *      modid     -  (IN) Update modid
 *      userdata  -  (IN) Pointer to any user data
 * Returns:
 *      NONE
 */
STATIC void 
_bcm_xgs3_l3_l2cpu_nh_cb(int unit, soc_switch_event_t  event, uint32 arg1, 
                   uint32 arg2, uint32 arg3, void* userdata)
{

    switch (event) {
        case SOC_SWITCH_EVENT_MODID_CHANGE:
    _bcm_xgs3_l3_l2cpu_nh_setup(unit);
            break;
        default:
            break;
    }

    return;
}

/*
 * Function:
 *      bcm_xgs3_l3_ecmp_group_init
 * Purpose:
 *      Initialize internal L3 ecmp groups table.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_xgs3_l3_ecmp_group_init(int unit)
{
    int mem_sz;                 /* Ecmp group info memory size.          */
    int max_grp_sz;             /* Maximum number of paths in ecmp group. */
    _bcm_l3_tbl_t *tbl_ptr;     /* Ecmp table pointer.                   */
    int rv = BCM_E_UNAVAIL;     /* Operation return status.              */
    int idx;

    max_grp_sz = BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);

    /*
     *  Calculate number of ecmp groups. 
     */
    tbl_ptr->idx_min = 0;
    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {
         tbl_ptr->idx_max = soc_mem_index_count(unit, L3_ECMP_COUNTm) - 1;
    } else {
         tbl_ptr->idx_max = (BCM_XGS3_L3_ECMP_TBL_SIZE(unit) / max_grp_sz) - 1;
    }
    tbl_ptr->idx_maxused = tbl_ptr->idx_min;

    /*
     * Keep track of ECMP table usage
     */
    mem_sz = (tbl_ptr->idx_max + 1) * sizeof(_bcm_l3_tbl_ext_t);
    BCM_XGS3_L3_ALLOC(tbl_ptr->ext_arr, mem_sz, "l3_ecmp");
    if (NULL == tbl_ptr->ext_arr) {
        return (BCM_E_MEMORY);
    }

    for (idx=0; idx < tbl_ptr->idx_max+1; idx++) {
         BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, idx) = 0;
    }

    /* 
     *  In order to support warm reboot, we preserve maximum ecmp group size 
     *  in last ecmp entry, effectively making last group size (max - 1)
     *  If group_num * max_group_size < total_num_entries; 
     *      All groups are identical
     *  If group_num * max_group_size == total_num_entries;  
     *      Last group has 1 entry less.
     */
    /* Save maximum number of paths in hw.. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, max_grp_size_set)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, max_grp_size_set) (unit, max_grp_sz);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_l3_ecmp_table_init
 * Purpose:
 *      Initialize HW ECMP table
 * Parameters:
 *      unit    - (IN)  SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_ecmp_table_init(int unit)
{
    int idx;
    int mem_sz;                 /* Ecmp entries info memory size. */
    _bcm_l3_tbl_t *tbl_ptr;     /* Ecmp table pointer.                   */

    /*
     * Keep track of ECMP table usage.
     */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp);
    tbl_ptr->idx_min = 0;
    if (SOC_MEM_IS_VALID(unit, L3_ECMPm)) {
         tbl_ptr->idx_max = soc_mem_index_count(unit, L3_ECMPm) - 1;
    } else {
         tbl_ptr->idx_max = 0;
    }
    tbl_ptr->idx_maxused = tbl_ptr->idx_min;

    if (SOC_MEM_IS_VALID(unit, BCM_XGS3_L3_MEM(unit, ecmp)) &&
        soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, ecmp))) {
        BCM_XGS3_L3_ECMP_TBL_SIZE(unit) = 
            soc_mem_index_count(unit, BCM_XGS3_L3_MEM(unit, ecmp));
    } else {
        BCM_XGS3_L3_ECMP_TBL_SIZE(unit) = 0;
    }

    if (0 == BCM_XGS3_L3_ECMP_TBL_SIZE(unit)) {
        return (BCM_E_NONE);
    }

    /* Default max ECMP to max allowed by hw */
    BCM_XGS3_L3_TBL(unit, ecmp_info).ecmp_max_paths = 
        BCM_XGS3_L3_ECMP_MAX(unit);

    /* Ecmp is not in use until first ecmp route added. */
    BCM_XGS3_L3_TBL(unit, ecmp_info).ecmp_inuse = 0;

    /*
     * Keep track of ECMP table usage
     */
    mem_sz = (tbl_ptr->idx_max + 1) * sizeof(_bcm_l3_tbl_ext_t);
    BCM_XGS3_L3_ALLOC(tbl_ptr->ext_arr, mem_sz, "l3_ecmp");
    if (NULL == tbl_ptr->ext_arr) {
        return (BCM_E_MEMORY);
    }

    for (idx=0; idx < tbl_ptr->idx_max+1; idx++) {
         BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, idx) = 0;
    }

    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_init(unit));
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_tables_init
 * Purpose:
 *      Initialize internal L3 tables and enable L3.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs3_l3_tables_init(int unit)
{
    int rv;                     /* Operation status.              */

    /* Allocate/zero unit software tables. */
    if (NULL == l3_module_data[unit]) {
        BCM_XGS3_L3_ALLOC(l3_module_data[unit], sizeof(_bcm_l3_module_data_t),
                          "l3_module_data");
        if (NULL == l3_module_data[unit]) {
            return (BCM_E_MEMORY);
        }
    } else {
        l3_module_data[unit]->l3_op_flags = 0;
    }

    /*
     * Get hardware tables size. 
     */

    /* Init hw calls & memory addresses. */
    rv = _bcm_xgs3_l3_hw_op_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }


    /* Flush all the tables in hw. */
    if (!SAL_BOOT_QUICKTURN) {
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_clear_all)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_clear_all) (unit);
            if (BCM_FAILURE(rv)) {
                _bcm_xgs3_l3_free_resource(unit);
                return rv;
            }
        }
    }

    /* Egress L3 interface table init. */
    rv = _bcm_xgs3_l3_intf_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* Ingress L3 interface table init. */
    rv = _bcm_xgs3_l3_ing_intf_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* Next hop table init.  */
    rv = _bcm_xgs3_l3_nh_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* L3 host table init.   */
    rv = _bcm_xgs3_l3_table_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* ECMP groups table init. */
    rv = _bcm_xgs3_l3_ecmp_table_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* Defip table init. */
    rv = _bcm_xgs3_defip_table_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

if(!SOC_IS_HURRICANE(unit)) {
    /* Tunnels terminator table  init. */
    rv = soc_tunnel_term_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* Tunnels initiator table  init. */
    rv = _bcm_xgs3_l3_tnl_initiator_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }
} /* !SOC_IS_HURRICANE */
    /* Tunnels dscp table  init. */
    rv = _bcm_xgs3_l3_tnl_dscp_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* Adjacent mac table init. */
    rv = _bcm_xgs3_l3_adj_mac_init(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* Reset auxiliary counters. */
    BCM_XGS3_L3_CNTRS_RESET(unit);

#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        rv = _bcm_rp_l3_init(unit);  
        if (BCM_FAILURE(rv)) {
            _bcm_xgs3_l3_free_resource(unit);
            return rv;
        }
    }
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */

    /* Reserve BLACK_HOLE entry to Drop packets */
    rv = _bcm_xgs3_l3_black_hole_nh_setup(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /*
     * Reserve the entry in L3 NextHop table to L2 switch packet CPU
     */
    rv = _bcm_xgs3_l3_l2cpu_nh_setup(unit);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

    /* Register a call back function to update modid */
    rv = soc_event_register(unit, _bcm_xgs3_l3_l2cpu_nh_cb, NULL);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }

#if defined(BCM_TRX_SUPPORT)
    if (soc_feature(unit, soc_feature_virtual_switching)) {
#if defined(BCM_HURRICANE_SUPPORT)
        if (!SOC_IS_HURRICANE(unit))
#endif
        {
            rv = _bcm_virtual_init(unit, SOURCE_VPm, VFIm);
            if (BCM_FAILURE(rv)) {
                _bcm_xgs3_l3_free_resource(unit);
                return rv;
            }
        }
    } 
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
    if(SOC_IS_TRIDENT(unit)) {
       rv = _bcm_td_l3_routed_int_pri_init(unit);
       if (BCM_FAILURE(rv)) {
           _bcm_xgs3_l3_free_resource(unit);
           return rv;
       }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Set "resources allocated" flag. */
    BCM_XGS3_L3_INITIALIZED(unit) = TRUE;

    /* Enable routing on all ports. */
    rv = bcm_xgs3_l3_enable(unit, TRUE);
    if (BCM_FAILURE(rv)) {
        _bcm_xgs3_l3_free_resource(unit);
        return rv;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_tables_cleanup
 * Purpose:
 *      Disable L3 functionality and cleanup internal L3 tables.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs3_l3_tables_cleanup(int unit)
{
    /* Disable routing on all ports. */
    if (0 == SOC_HW_ACCESS_DISABLE(unit)) {
        bcm_xgs3_l3_enable(unit, FALSE);
    }


    /* Free allocated resources. */
    if (BCM_XGS3_L3_INITIALIZED(unit)) {
        _bcm_xgs3_l3_free_resource(unit);

        /* Reset auxiliary counters. */
        BCM_XGS3_L3_CNTRS_RESET(unit);

        BCM_XGS3_L3_INITIALIZED(unit) = FALSE;

        /* Unregister call back function */
         soc_event_unregister(unit, _bcm_xgs3_l3_l2cpu_nh_cb, NULL);
    }
    return (BCM_E_NONE);
}



/*
 * Function:
 *      bcm_xgs3_l3_intf_get
 * Purpose:
 *      Get an entry from L3 interface table.
 * Parameters:
 *      unit - SOC unit number.
 *      intf_info - Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_intf_get(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Make sure ifindex is valid & interface is used. */
    if (intf_info->l3i_index >= BCM_XGS3_L3_IF_TBL_SIZE(unit) ||
        !BCM_L3_INTF_USED_GET(unit, intf_info->l3i_index)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Make sure hw call pointer was initialized. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, if_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Read interface from hardware. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, if_get) (unit, intf_info));

    /* Read vrf info from hardware. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_vrf_get)) {
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, if_vrf_get) (unit, intf_info));
    }

    /* Get interface group id . */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_group_get)) {
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, if_group_get) (unit, intf_info));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_intf_lookup
 * Purpose:
 *      Get first entry from L3 interface table matching vlan id & mac.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      intf_info - (IN/OUT)Pointer to memory for interface information.
 *      vid       - (IN) Key vlan id.
 *      mac       - (IN) mac address. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_intf_lookup(int unit, _bcm_l3_intf_cfg_t *intf_info,
                         bcm_vlan_t vid, bcm_mac_t mac)
{
    int idx;                    /* Iteration index.             */
    int max_lkup_cnt;           /* Number of active interfaces, */
    int index_min;              /* First interface entry index. */
    int index_max;              /* Last interface entry index.  */


    /* Make sure hw call pointer was initialized. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, if_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Get interface table start & end index. */
    index_min = soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, intf));
    index_max = soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, intf));

    /* Don't iterate beyond number of active interfaces. */
    max_lkup_cnt = BCM_XGS3_L3_IF_COUNT(unit);
    if (max_lkup_cnt <= 0) {
        return (BCM_E_NOT_FOUND);
    }

    /* Loop over all valid interfaces. */
    for (idx = index_min; idx <= index_max; idx++) {
        if (BCM_L3_INTF_USED_GET(unit, idx)) {
            /* Read interface from hardware. */
            intf_info->l3i_index = idx;
            BCM_IF_ERROR_RETURN
                (BCM_XGS3_L3_HWCALL_EXEC(unit, if_get) (unit, intf_info));
            /* Check for match. */
            if (intf_info->l3i_vid == vid) {
                /* Search by VID only. */
                if (NULL == mac) {
                    break;
                }
                /* Search by MAC & VID */
                if (sal_memcmp(mac, intf_info->l3i_mac_addr,
                               sizeof(bcm_mac_t)) == 0) {
                    break;
                }               /* Mac match. */
            }
            /* Vid match. */
            /* Decrement number of interface to search. */
            max_lkup_cnt--;
            if (!max_lkup_cnt) {
                break;
            }
        }                       /* Interface is active. */
    }                           /* Loop over interface table. */

    if (!max_lkup_cnt || (idx == BCM_XGS3_L3_IF_TBL_SIZE(unit))) {
        return (BCM_E_NOT_FOUND);
    }

    /* Get vrf info if available. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_vrf_get)) {
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, if_vrf_get) (unit, intf_info));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_intf_get_by_vid
 * Purpose:
 *      Get first entry from L3 interface table matching vlan id.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      intf_info - (IN/OUT)Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_intf_get_by_vid(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }
    /*  Perform lookup by VID. */
    return _bcm_xgs3_l3_intf_lookup(unit, intf_info, intf_info->l3i_vid, NULL);
}


/*
 * Function:
 *      _bcm_xgs3_l3_intf_l2_addr_set
 * Purpose:
 *      Set interface mac address in l2 table.
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface information.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_l2_addr_set(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    bcm_l2_addr_t l2addr;       /* Layer 2 address for interface. */
    int rv;                     /* Operation return status.       */

    /* Set l2 address info. */
    bcm_l2_addr_t_init(&l2addr, intf_info->l3i_mac_addr, intf_info->l3i_vid);

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
    bcm_esw_l2_addr_delete(unit, intf_info->l3i_mac_addr, intf_info->l3i_vid);

    /* Add entry to l2 table. */
    rv = bcm_esw_l2_addr_add(unit, &l2addr);

    /* Set mac address installed indicator. */
    if (rv == BCM_E_NONE) {
        BCM_L3_INTF_ARL_SET(unit, intf_info->l3i_index);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_l3_intf_create
 * Purpose:
 *      Create l3 interface. 
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface information.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_xgs3_l3_intf_create(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    int rv;
#if defined(BCM_TRIUMPH_SUPPORT)
     bcm_vlan_control_vlan_t control;
#endif /* BCM_TRIUMPH_SUPPORT */

    /* Add interface to hardware. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_add)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_add) (unit, intf_info);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Write to interface group table. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_group_set)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_group_set) (unit, intf_info);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Check if L3 Ingress Interface mode not set */ 
    if (!(BCM_XGS3_L3_INGRESS_MODE_ISSET(unit))) {
        /* Set VRF_VFI table for primary interfaces. */
        if (!(intf_info->l3i_flags & BCM_L3_SECONDARY)) {
            if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_vrf_bind)) {
                rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_vrf_bind) (unit, intf_info);
                BCM_IF_ERROR_RETURN(rv);
            }
        }

#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_MEM_FIELD_VALID(unit, L3_IIFm, ALLOW_GLOBAL_ROUTEf) &&
            intf_info->l3i_vid != 0) {
            /* Check if L3 Ingress Interface Map mode not set */ 
            if (!(BCM_XGS3_L3_INGRESS_INTF_MAP_MODE_ISSET(unit))) {
                rv = bcm_esw_vlan_control_vlan_get(unit, intf_info->l3i_vid, &control);
                BCM_IF_ERROR_RETURN(rv);
                control.flags &= ~BCM_VLAN_L3_VRF_GLOBAL_DISABLE;
                rv = bcm_esw_vlan_control_vlan_set(unit, intf_info->l3i_vid, control);
                BCM_IF_ERROR_RETURN(rv);
            }
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

    /* Add interface layer 2 address if required. */
    if (intf_info->l3i_flags & BCM_L3_ADD_TO_ARL) {
        rv = _bcm_xgs3_l3_intf_l2_addr_set(unit, intf_info);
        BCM_IF_ERROR_RETURN(rv);
    }

    /* Mark interface "inuse" in bitmap. */
    if (!BCM_L3_INTF_USED_GET(unit, intf_info->l3i_index)) {
        BCM_L3_INTF_USED_SET(unit, intf_info->l3i_index);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_egress_l3_intf_id_free
 * Purpose:
 *      Free egress l3 interface id.
 * Parameters:
 *      unit   - (IN) SOC unit number.
 *      id     - (IN) Interface id
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_egress_l3_intf_id_free(int unit, int id)
{
    /* Input parameters check. */
    if ((id < 0) || (id > BCM_XGS3_L3_IF_TBL_SIZE(unit))) {
        return (BCM_E_PARAM);
    }

    /* Free interface index. */
    _bcm_esw_l3_lock(unit);
    BCM_L3_INTF_USED_CLR(unit, id); 
    _bcm_esw_l3_unlock(unit);

    return  (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_egress_l3_intf_id_alloc
 * Purpose:
 *      Allocate egress l3 interface id.
 * Parameters:
 *      unit   - (IN) SOC unit number.
 *      id     - (OUT) Allocated interface id
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_egress_l3_intf_id_alloc(int unit, int *id)
{
    int idx;                    /* Iteration index.           */

    /* Input parameters check. */
    if (NULL == id) {
        return (BCM_E_PARAM);
    }

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    _bcm_esw_l3_lock(unit);

    /* Search for unused index. */
    for (idx = 0; idx < BCM_XGS3_L3_IF_TBL_SIZE(unit); idx++) {
        /* Skip cpu interface. */
        if (idx == BCM_XGS3_L3_L2CPU_INTF_IDX(unit)) {
            continue;
        }
#ifdef BCM_EASYRIDER_SUPPORT
        if ((idx >= BCM_XGS3_L3_MPLS_INTF_IDX_MIN(unit)) &&
            (idx <= BCM_XGS3_L3_MPLS_INTF_IDX_MAX(unit))) {
            continue;
        }
#endif
        /* Check if interface index is used. */
        if (!BCM_L3_INTF_USED_GET(unit, idx)) {
            /* Free index found. */
            BCM_L3_INTF_USED_SET(unit, idx); 
            *id = idx;
            break;
        }
    }

    _bcm_esw_l3_unlock(unit);

    return  (idx < BCM_XGS3_L3_IF_TBL_SIZE(unit)) ? BCM_E_NONE : BCM_E_FULL;
}


/*
 * Function:
 *      bcm_xgs3_l3_intf_create
 * Purpose:
 *      Create L3 interface
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_intf_create(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    int rv;                     /* Interface creation status. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Make sure there are empty slots. */
    if (BCM_XGS3_L3_IF_COUNT(unit) == BCM_XGS3_L3_IF_TBL_SIZE(unit) - 1) {
        return (BCM_E_FULL);
    }

    /* Allocate interface id. */
    rv = _bcm_xgs3_egress_l3_intf_id_alloc(unit, &intf_info->l3i_index);
    BCM_IF_ERROR_RETURN(rv);

    /* Create interface */
    rv = _bcm_xgs3_l3_intf_create(unit, intf_info);
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_intf_id_create
 * Purpose:
 *      Create L3 interface with a specified index
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_intf_id_create(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Interface index sanity check. */
    if (intf_info->l3i_index < 0 ||
        intf_info->l3i_index >= BCM_XGS3_L3_IF_TBL_SIZE(unit) ||
        intf_info->l3i_index == BCM_XGS3_L3_L2CPU_INTF_IDX(unit)) {
        return (BCM_E_PARAM);
    }
#ifdef BCM_EASYRIDER_SUPPORT
    if ((intf_info->l3i_index >= BCM_XGS3_L3_MPLS_INTF_IDX_MIN(unit)) &&
        (intf_info->l3i_index <= BCM_XGS3_L3_MPLS_INTF_IDX_MAX(unit))) {
        return (BCM_E_PARAM);
    }
#endif

    /* Create interface */
    return _bcm_xgs3_l3_intf_create(unit, intf_info);
}

/*
 * Function:
 *      bcm_xgs3_l3_intf_lookup
 * Purpose:
 *      See if an L3 interface exists by VID and MAC.
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface information.
 * Returns:
 *      BCM_E_NONE if entry exists and is found;
 *      BCM_E_NOT_FOUND if the entry is not found.
 * Notes:
 *      Returns the L3 interface info if found
 */

int
bcm_xgs3_l3_intf_lookup(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    bcm_mac_t mac;              /* Key mac. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Set lookup key. */
    sal_memcpy(mac, intf_info->l3i_mac_addr, sizeof(bcm_mac_t));

    /* Perform search by MAC & VID. */
    return _bcm_xgs3_l3_intf_lookup(unit, intf_info, intf_info->l3i_vid, mac);

}

/*
 * Function:
 *      bcm_xgs3_l3_intf_del
 * Purpose:
 *      Delete L3 interface.
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_intf_del(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    int rv = BCM_E_UNAVAIL;     /* Operation status.   */
    int first_error = BCM_E_NONE;       /* First error occured. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    if (BCM_XGS3_L3_IF_COUNT(unit) == 0) {
        return (BCM_E_NOT_FOUND);
    }

    /* Check that ifindex corresponds to an active interface. */
    if (!BCM_L3_INTF_USED_GET(unit, intf_info->l3i_index)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Reserved interface entry, should not be touched. */
    if (intf_info->l3i_index == BCM_XGS3_L3_L2CPU_INTF_IDX(unit)) {
        return (BCM_E_NONE);
    }
#ifdef BCM_EASYRIDER_SUPPORT
    if ((intf_info->l3i_index >= BCM_XGS3_L3_MPLS_INTF_IDX_MIN(unit)) &&
        (intf_info->l3i_index <= BCM_XGS3_L3_MPLS_INTF_IDX_MAX(unit))) {
        return (BCM_E_NONE);
    }
#endif

    /* Verify interface index range. */
    if (intf_info->l3i_index >= BCM_XGS3_L3_IF_TBL_SIZE(unit)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Get interface info. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_get)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_get) (unit, intf_info);
    }
    BCM_IF_ERROR_RETURN(rv);

    /* Remove interface layer 2 address if required. */
    if (BCM_L3_INTF_ARL_GET(unit, intf_info->l3i_index)) {
        rv = bcm_esw_l2_addr_delete(unit, intf_info->l3i_mac_addr,
                                    intf_info->l3i_vid);
        if ((rv < 0) && (first_error == BCM_E_NONE) && (rv != BCM_E_NOT_FOUND) ) {
            first_error = rv;
        }
        BCM_L3_INTF_ARL_CLR(unit, intf_info->l3i_index);
    }

    /* Clear VRF_VFI table for primary interfaces. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_vrf_unbind)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_vrf_unbind) (unit, intf_info);
        if ((rv < 0) && (first_error == BCM_E_NONE)) {
            first_error = rv;
        }
    }

    /* Clear ingress & egress filter interface group table. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_group_set)) {
        intf_info->l3i_group = 0;
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_group_set) (unit, intf_info);
        if ((rv < 0) && (first_error == BCM_E_NONE)) {
            first_error = rv;
        }
    }

    /* Remove the interface. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, if_del)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_del) (unit, intf_info->l3i_index);
        if ((rv < 0) && (first_error == BCM_E_NONE)) {
            first_error = rv;
        }
    }

    /* Unreserve interface id. */
    rv = _bcm_xgs3_egress_l3_intf_id_free(unit, intf_info->l3i_index);
    return first_error;
}

/*
 * Function: bcm_xgs3_l3_intf_del_all
 * Purpose:
 *      Delete all L3 interfaces.
 * Parameters:
 *      unit - (IN) SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_intf_del_all(int unit)
{
    int idx;                    /* Interface index.             */
    _bcm_l3_intf_cfg_t intf;    /* Interface info structure.    */
    int max_lkup_cnt;           /* Number of active interfaces. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Don't iterate beyond number of active interfaces. */
    max_lkup_cnt = BCM_XGS3_L3_IF_COUNT(unit);
    if (max_lkup_cnt <= 0) {
        return (BCM_E_NONE);
    } 

    /* Iterate over all active interfaces. */
    for (idx = 0; idx < BCM_XGS3_L3_IF_TBL_SIZE(unit); idx++) {
        if (BCM_L3_INTF_USED_GET(unit, idx)) {
            max_lkup_cnt--;
            if (!max_lkup_cnt) {
                break;
            } 

            /* Remove interface. */
            intf.l3i_index = idx;
            BCM_IF_ERROR_RETURN(bcm_xgs3_l3_intf_del(unit, &intf));
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_flags_to_shr
 * Purpose:
 *      Translate l3 specific flags to shared (table management) flags. 
 * Parameters:
 *    l3_flags   - (IN)  l3 table flags. 
 *    shr_flags  - (OUT) Shared flags. 
 * Returns:
 *    BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_l3_flags_to_shr(uint32 l3_flags, uint32 *shr_flags)
{
    uint32 flag = 0;

    if (NULL == shr_flags) {
        return (BCM_E_PARAM);
    }

    if (l3_flags & BCM_L3_REPLACE) {
       flag |= _BCM_L3_SHR_UPDATE;
    }

    if (l3_flags & BCM_L3_WITH_ID) {
       flag |= _BCM_L3_SHR_WITH_ID;
    }

    *shr_flags = flag;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_flags_to_egr_obj
 * Purpose:
 *      Translate l3 specific flags to Egress Object flags. 
 * Parameters:
 *    l3_flags   - (IN)  l3 table flags. 
 *    egr_obj  - (OUT) Egress Object
 * Returns:
 *    BCM_E_XXX
 */

STATIC INLINE int
_bcm_xgs3_l3_flags_to_egr_obj (uint32 l3_flags, bcm_l3_egress_t *egr)
{
    /* Input parameters check. */
    if (NULL == egr) {
        return (BCM_E_PARAM);
    }

    if (l3_flags & BCM_L3_KEEP_SRCMAC) {
       egr->flags |= BCM_L3_KEEP_SRCMAC;
    }

    if (l3_flags & BCM_L3_KEEP_DSTMAC) {
       egr->flags |= BCM_L3_KEEP_DSTMAC;
    }

    if (l3_flags & BCM_L3_KEEP_VLAN) {
       egr->flags |= BCM_L3_KEEP_VLAN;
    }

    if (l3_flags & BCM_L3_KEEP_TTL) {
       egr->flags |= BCM_L3_KEEP_TTL;
    }

    return (BCM_E_NONE);

}

/*
 * Function:
 *      _bcm_xgs3_tunnel_flags_to_shr
 * Purpose:
 *      Translate tunnel specific flags to shared (table management) flags. 
 * Parameters:
 *    tnl_flags  - (IN)  Tunnel table flags. 
 *    shr_flags  - (OUT) Shared flags. 
 * Returns:
 *    BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_tunnel_flags_to_shr(uint32 tnl_flags, uint32 *shr_flags)
{
    /*
     * Reserve entry Zero in EGR_IP_TUNNEL memory. This reserved entry
     * will not be allocated. The purpose of this entry is that
     * any interface which points to this entry will be treated
     * as a tunnel with no encapsulation or 'NO-protocol" tunnel.
     */
    uint32 flag = _BCM_L3_SHR_SKIP_INDEX_ZERO;

    if (NULL == shr_flags) {
        return (BCM_E_PARAM);
    }

    if (tnl_flags & BCM_TUNNEL_REPLACE) {
       flag |= _BCM_L3_SHR_UPDATE;
    }

    *shr_flags = flag;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_tbl_free_idx_get
 * Purpose:
 *       Generic table get unused table entry index.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      data        - (IN/OUT) Software table operation entry .  
 * Returns:
 *    BCM_E_NONE - if free entry was found.
 *    BCM_E_FULL - if no free entry found.
 */
int
_bcm_xgs3_tbl_free_idx_get(int unit, _bcm_l3_tbl_op_t *data)
{
    _bcm_l3_tbl_t *tbl_ptr; /* Software table pointer. */ 
    int  width;            /* Entry width.            */ 
    int lkup_idx;           /* Iteration index.        */
    int idx;                /* Iteration index 2       */   
    
    tbl_ptr = data->tbl_ptr;
    width = data->width;

    if (!(data->oper_flags & _BCM_L3_SHR_TABLE_TRAVERSE_CONTROL)) {
        /* Try to identify matching/free entry */
        for (lkup_idx = tbl_ptr->idx_min;
             lkup_idx <= tbl_ptr->idx_max; lkup_idx += width) {
    
            if (!lkup_idx && (data->oper_flags & _BCM_L3_SHR_SKIP_INDEX_ZERO)) {
                continue;
            } 
    
            if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, lkup_idx)) {
                idx = width;
                while (--idx) {
                    /* Check if free slots can accomodate wide entry. */
                    if (BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, lkup_idx + idx)) {
                        break; 
                    }
                }
                if (idx) {
                    /* There is no room to accomodate wide entry. */
                    continue; 
                }
                data->entry_index = lkup_idx;
                return (BCM_E_NONE);
            }
        }
    } else {
        for (lkup_idx = tbl_ptr->idx_min;
             lkup_idx <= tbl_ptr->idx_max; lkup_idx++) {
    
            if (!lkup_idx && (data->oper_flags & _BCM_L3_SHR_SKIP_INDEX_ZERO)) {
                continue;
            } 
    
            if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, lkup_idx)) {
                /* Check if a contiguous chunk of 'data.width'
                 * entries are available starting from lkup_idx */
                for (idx = lkup_idx;
                    (idx < (lkup_idx + width)) && (idx <= tbl_ptr->idx_max);
                     idx++) {

                    if(BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, idx)) {
                         break;
                    }
                }

                if (idx == (lkup_idx + width)) {
                    /* Empty slot with 'data.width' entries available */
                    data->entry_index = lkup_idx;
                    return (BCM_E_NONE);
                }
            }
        }
    }
    return (BCM_E_FULL);
}

/*
 * Function:
 *      _bcm_xgs3_tbl_match
 * Purpose:
 *       Generic table match routine.  
 *       Identify identical entry in the table if exists or return 
 *       first free entry available for write.    
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      data        - (IN/OUT)Table entry operation info.
 * Returns:
 *    BCM_E_NONE      - if matching entry was found.
 *    BCM_E_NOT_FOUND - if no matching entry found.
 *    BCM_E_FULL      - if no unused entry found. 
 */
STATIC int
_bcm_xgs3_tbl_match(int unit, _bcm_l3_tbl_op_t *data)
{
    int lkup_idx = BCM_XGS3_L3_INVALID_INDEX;   /* Iteration index.      */
    int unused_idx =  BCM_XGS3_L3_INVALID_INDEX;/* Unused table index.   */
    _bcm_l3_tbl_t *tbl_ptr;                   /* Software table pointer. */ 
    uint16 entry_hash;                        /* Entry hash.             */
    int cmp_result;                           /* Comparison result.      */
    int width;                              /* Entry width.            */ 
    int idx;                                  /* Iteration index 2       */   
    int rv;                                   /* Return status.          */


    tbl_ptr = data->tbl_ptr;
    width = data->width;

    /* Calculate entry data hash. */
    (*(data->hash_func)) (unit, data->entry_buffer, &entry_hash);

    /* Try to identify matching/free entry */
    for (lkup_idx = tbl_ptr->idx_min;
         lkup_idx <= tbl_ptr->idx_maxused; lkup_idx += width) {

        if (!lkup_idx && (data->oper_flags & _BCM_L3_SHR_SKIP_INDEX_ZERO)) {
            continue;
        } 

        if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, lkup_idx)) {
            /* If entry is unused preserve free index. */
            if (unused_idx == BCM_XGS3_L3_INVALID_INDEX) {
                idx = width;
                while (--idx) {
                    /* Check if free slots can accomodate wide entry. */
                    if (BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, lkup_idx + idx)) {
                        break; 
                    }
                }
                if (idx) {
                    /* There is no room to accomodate wide entry. */
                    continue; 
                }
                unused_idx = lkup_idx;
            }
            continue;
        }

        /* Compare valid entry hash(signature). */
        if (!BCM_XGS3_L3_ENT_HASH_CMP(tbl_ptr, lkup_idx, entry_hash)) {
            continue;
        }

        /* If hash matches  compare the entry itself.  */
        rv = ((*(data->cmp_func))(unit, data->entry_buffer, 
                                  lkup_idx, &cmp_result));
        if (rv == BCM_E_NOT_FOUND) {
            continue;
        } else if (rv < 0) {
            return rv;
        }

        /* Check comparison result. */
        if (cmp_result == BCM_L3_CMP_EQUAL) {
            data->entry_index = lkup_idx;
            return (BCM_E_NONE);
        }  
    }  /* Loop over all the entries. */

    /* Return free entry if found. */
    if (BCM_XGS3_L3_INVALID_INDEX != unused_idx) {
        data->entry_index = unused_idx;
    } else if (lkup_idx <= tbl_ptr->idx_max) {
        data->entry_index = lkup_idx;
    } else {
        return (BCM_E_FULL);
    }
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *      _bcm_xgs3_tbl_add
 * Purpose:
 *       Generic table add routine.  
 *       Allocate table index & write an entry to into it.
 *       Routine trying to match new entry to an existing one, 
 *       if match found reference cound is incremented, otherwise
 *       new index allocated & entry added to hw. 
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      data    - (IN/OUT)Table entry operation info.
 *
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_xgs3_tbl_add(int unit, _bcm_l3_tbl_op_t *data)
{
    _bcm_l3_tbl_t *tbl_ptr;                   /* Software table pointer. */ 
    int l3_match_disable;                     /* L3 match disable flag.  */
    uint16 entry_hash;                        /* Entry hash.             */
    int rv;                                   /* Return status.          */

    /* Input parameters check. */
    if (NULL == data) {
        return (BCM_E_PARAM);
    }

    if ((NULL == data->entry_buffer) ||  (NULL == data->tbl_ptr) ||
        (NULL == data->hash_func) || (NULL == data->cmp_func) || 
        (NULL == data->add_func)) {
        return (BCM_E_PARAM);
    }

    tbl_ptr = data->tbl_ptr;
    /* Get l3 switch mode from module flags. */
    l3_match_disable =  data->oper_flags & _BCM_L3_SHR_MATCH_DISABLE;

    /* Calculate entry data hash. */
    (*(data->hash_func)) (unit, data->entry_buffer, &entry_hash);

    if(data->oper_flags & _BCM_L3_SHR_WITH_ID) {

        /* Check index range */
        if ((data->entry_index < tbl_ptr->idx_min) || 
            (data->entry_index > tbl_ptr->idx_max)) {
            return (BCM_E_PARAM);
        }

        /* Entry is currently used make sure replace flag is set. */
        if (BCM_XGS3_L3_ENT_REF_CNT(data->tbl_ptr, data->entry_index))  {
            if (!(data->oper_flags & _BCM_L3_SHR_UPDATE)) {
                return (BCM_E_EXISTS);
            }
        }
    } else {
        if (l3_match_disable) {
            /* Get free index to write the entry. */  
            BCM_IF_ERROR_RETURN(_bcm_xgs3_tbl_free_idx_get(unit, data));
        } else {
            /* Try to identify matching/free entry */
            rv = _bcm_xgs3_tbl_match(unit, data);
            if ((rv < 0) && (BCM_E_NOT_FOUND != rv)) {
                return rv;
            }

            /* If identical entry found -> just increment reference count */
            if (BCM_E_NONE == rv) {
                BCM_XGS3_L3_ENT_REF_CNT_INC(tbl_ptr, data->entry_index,
                                            _BCM_SINGLE_WIDE);
                return (BCM_E_NONE);
            }
        }
    }

    /* Update table maxused index. */
    if (tbl_ptr->idx_maxused < data->entry_index) {
        tbl_ptr->idx_maxused = data->entry_index; 
    }

    if (!(data->oper_flags & _BCM_L3_SHR_WRITE_DISABLE)) {
        /* Set the entry to hw. */
        rv = (*(data->add_func)) (unit, data->entry_index, data->entry_buffer, data->value);
        if (rv < 0) {
            while(!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, tbl_ptr->idx_maxused)) {
                (tbl_ptr->idx_maxused)--;
            }
            return rv;
        }
    }

    if ((data->oper_flags & _BCM_L3_SHR_WITH_ID) && (BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, data->entry_index) > 1)) {
        /* Update hash */
        BCM_XGS3_L3_ENT_HASH_UPDATE(tbl_ptr, data->entry_index, data->width, entry_hash);
    } else {
       /* Update hash & reference count info. */
       BCM_XGS3_L3_ENT_INIT(tbl_ptr, data->entry_index, data->width, entry_hash);
    }
    return (BCM_E_NONE);
}


void
_bcm_xgs3_nh_ref_cnt_incr(int unit, unsigned idx)
{
    BCM_XGS3_L3_ENT_REF_CNT_INC(BCM_XGS3_L3_TBL_PTR(unit, next_hop),
				idx, _BCM_SINGLE_WIDE);
}


/*
 * Function:
 *      _bcm_xgs3_tbl_del
 * Purpose:
 *       Generic table delete routine.  
 *       Entry kept if reference counter is not equal to 1 (other
 *       instances still reference the same entry).    
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      data        - (IN/OUT)Table entry operation info.
 *
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_xgs3_tbl_del(int unit,  _bcm_l3_tbl_op_t *data)
{
    int index;                    /* Temp index.             */
    _bcm_l3_tbl_t *tbl_ptr;       /* Software table pointer. */ 

    /* Input parameters check. */
    if (NULL == data) {
        return (BCM_E_PARAM);
    }

    if (NULL == data->delete_func) {
        return (BCM_E_PARAM);
    }

    tbl_ptr = data->tbl_ptr;

    /* Preserve next hop index */
    index = data->entry_index;

    /* Check index sanity. */
    if ((index < tbl_ptr->idx_min) || (index > tbl_ptr->idx_maxused)) {
        return (BCM_E_NONE);
    }

    /* If entry is not currently used return. */
    if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, index)) {
        return (BCM_E_NONE);
    }

    /* If entry used by other hosts/routes just decrement ref count. */
    if (BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, index) > 1) {
        BCM_XGS3_L3_ENT_REF_CNT_DEC(tbl_ptr, index, data->width);
        return (BCM_E_NONE);
    }

    /* Use count is exactly one, remove the entry. */

    /* Reset entry usecount & hash. */
    BCM_XGS3_L3_ENT_RESET(tbl_ptr, index, data->width);

    /* Back up idx_maxused if needed. */
    if (index == tbl_ptr->idx_maxused) {
        while ((!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, index)) &&
               index > tbl_ptr->idx_min) {
            index -= 1;
        }
        tbl_ptr->idx_maxused = index;
    }

    if (data->oper_flags & _BCM_L3_SHR_WRITE_DISABLE) {
        return (BCM_E_NONE);
    }

    return (*(data->delete_func)) (unit, data->entry_index, data->value);
}

/*
 * Function:
 *      bcm_xgs3_nh_get
 * Purpose:
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      nh_idx    - (IN)Next hop index.  
 *      nh_info   - (OUT)Buffer to fill next hop info. 
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_nh_get(int unit, int nh_idx, bcm_l3_egress_t *nh_info)
{
    int rv = BCM_E_UNAVAIL;     /* Operation return status.    */
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
    uint32  entry_type=0;              /* Next Hop Entry_type */
    uint32 label_action=-1;          /* MPLS Label Action */
#endif /* BCM_TRIUMPH_SUPPORT  && BCM_MPLS_SUPPORT */


    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == nh_info) {
        return (BCM_E_PARAM);
    } else {
        bcm_l3_egress_t_init (nh_info);
    }

    /* Make sure entry is inuse. */
    if (!BCM_XGS3_L3_ENT_REF_CNT(BCM_XGS3_L3_TBL_PTR(unit, next_hop), nh_idx)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Get next hop entry from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, nh_get)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, nh_get) (unit, nh_idx, nh_info);
        BCM_IF_ERROR_RETURN(rv);
    }
#if defined(BCM_TRIDENT_SUPPORT)
    if ( soc_feature(unit, soc_feature_trill)) {
         rv = bcm_td_trill_egress_get(unit, nh_info, nh_idx);
         BCM_IF_ERROR_RETURN(rv);
         if(nh_info->flags & BCM_L3_TRILL_ONLY) {
              return rv;
         }
    }
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
    if (SOC_IS_TR_VL(unit) && (soc_feature(unit, soc_feature_mpls))) {
        BCM_IF_ERROR_RETURN(
               bcm_tr_mpls_get_entry_type(unit, nh_idx, &entry_type));
        if (entry_type == 1) {
              BCM_IF_ERROR_RETURN(
                     bcm_tr_mpls_get_label_action(unit, nh_idx, &label_action));
              switch (label_action) {
                  case _BCM_MPLS_ACTION_NOOP:
                  case _BCM_MPLS_ACTION_PUSH:

                      BCM_IF_ERROR_RETURN(
                           bcm_tr_mpls_l3_label_get (unit, nh_idx, nh_info));
                      break;

                  case _BCM_MPLS_ACTION_SWAP:

                       BCM_IF_ERROR_RETURN(
                            bcm_tr_mpls_swap_nh_info_get (unit, nh_info, nh_idx));
                      break;

                  default:
                      break;
              } 
        }
    } 
#endif /* BCM_TRX_SUPPORT  && BCM_MPLS_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_failover))  {
         int prot_nh_index=0;
         rv = bcm_tr2_failover_prot_nhi_get  (unit, nh_idx, &nh_info->failover_id, &prot_nh_index);
         if ( rv != BCM_E_NONE ) {
              return rv;
         } else {
              nh_info->failover_if_id = prot_nh_index + BCM_XGS3_EGRESS_IDX_MIN;
         }
    }
#endif /* BCM_TRIUMPH2_SUPPORT  */
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_nh_ent_cmp
 * Purpose:
 *      Routine compares next hop entry with entry in the chip
 *      with specified index. 
 *      NOTE: It is safer to compare entries in hw format after
 *            proper validation & mapping of api data.  
 *            Hence next hop entry expected to be in HW format!
 *            Please see  _bcm_xgs3_nh_map_api_data_to_hw.
 *
 * Parameters:
 *      unit       - (IN) SOC unit number.
 *      buf        - (IN) First nh entry to compare.
 *      index      - (IN) Entry index in the chip to compare.
 *      cmp_result - (OUT)Compare result. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_ent_cmp(int unit, void *buf, int index, int *cmp_result)
{
    bcm_l3_egress_t nh_hw;    /* Next hop entry from hw.  */
    bcm_l3_egress_t *nh_entry;        /* Next hop entry from api. */

    if ((NULL == buf) || (NULL == cmp_result))
        return (BCM_E_PARAM);

    /* Get api entry info. */
    nh_entry = (bcm_l3_egress_t *) buf;

    /* Get next hop entry from hw. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_nh_get(unit, index, &nh_hw));

    /*
     *  Compare entries in hw format. Should never fail, we just read it.
     *  NOTE  bcm_xgs3_nh_get returns entry in API format. 
     */
    _bcm_xgs3_nh_map_api_data_to_hw(unit, &nh_hw);

    /* Compare port/trunk id. */
    if (nh_hw.port != nh_entry->port) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    /* Module Id. */
    if (nh_hw.module != nh_entry->module) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    /* Interface compare. */
    if (nh_hw.intf != nh_entry->intf) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    /* Mac compare */
    if (sal_memcmp(nh_hw.mac_addr, nh_entry->mac_addr, sizeof(bcm_mac_t))) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }
#ifdef BCM_FIREBOLT_SUPPORT
    /* Tunnel type compare. */
    if (SOC_IS_FBX(unit)) {
        /* Vlan id compare. */
        if (nh_hw.vlan != nh_entry->vlan) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            return (BCM_E_NONE);
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Only match label if nh_mpls_label is legal. */
        if (BCM_XGS3_L3_MPLS_LBL_VALID(nh_entry->mpls_label)) {
            if (nh_hw.mpls_label != nh_entry->mpls_label) {
                *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                return (BCM_E_NONE);
            }
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    *cmp_result = BCM_L3_CMP_EQUAL;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_nh_add
 * Purpose:
 *       Allocate NextHop table index & Write NextHop entry.
 *       Routine trying to match new entry to an existing one, 
 *       if match found reference cound is incremented, otherwise
 *       new index allocated & entry added to hw. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      flags     - (IN)Egress object flags. 
 *      nh_info   - (IN)Next hop  entry info. 
 *      index     - (OUT)Next hop index.  
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_nh_add(int unit, uint32 flags, bcm_l3_egress_t *nh_info, int *index)
{
    _bcm_l3_tbl_op_t data;        /* Operation data. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == nh_info) {
        return (BCM_E_PARAM);
    }

    if (!(flags & _BCM_L3_SHR_WRITE_DISABLE)) {
        /* Make sure hw call is defined. */
        if (!BCM_XGS3_L3_HWCALL_CHECK(unit, nh_add)) {
            return (BCM_E_UNAVAIL);
        }

        /* Convert API next hop entry to HW space format. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_nh_map_api_data_to_hw(unit, nh_info));
    }

    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, next_hop);
    data.width = _BCM_SINGLE_WIDE;
    data.oper_flags = flags;
    data.entry_buffer = (void *)nh_info;
    data.hash_func = _bcm_xgs3_nh_hash_calc;
    data.cmp_func  = _bcm_xgs3_nh_ent_cmp;
    data.add_func  = BCM_XGS3_L3_HWCALL(unit, nh_add);
    if (flags & _BCM_L3_SHR_WITH_ID) {
        data.entry_index = *index;
    }

    /* Add next hop table entry. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_tbl_add(unit, &data)); 
    *index = data.entry_index;
    
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_nh_del
 * Purpose:
 *      Delete NextHop entry.  
 *      Entry kept if reference counter not equals to 1(Other hosts/routes
 *      reference the same entry).   
 *       
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      flags     - (IN)Egress object flags.
 *      nh_index  - (IN)next hop index.  
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_nh_del(int unit, uint32 flags, int nh_index)
{
    _bcm_l3_tbl_op_t data;        /* Operation data. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    if (!(flags & _BCM_L3_SHR_WRITE_DISABLE)) {
        /*  Make sure hw call is registred. */
        if (!BCM_XGS3_L3_HWCALL_CHECK(unit, nh_del)) {
            return (BCM_E_UNAVAIL);
        }
    }

    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, next_hop);
    data.entry_index = nh_index;
    data.width = _BCM_SINGLE_WIDE;
    data.oper_flags = flags;
    data.delete_func = BCM_XGS3_L3_HWCALL(unit, nh_del);

    /* Don't touch reserved trap to cpu entry. */
    if (nh_index == BCM_XGS3_L3_L2CPU_NH_IDX) {
        if (BCM_XGS3_L3_ENT_REF_CNT(data.tbl_ptr, nh_index) > 1) {
            BCM_XGS3_L3_ENT_REF_CNT_DEC(data.tbl_ptr, nh_index, 
                                        _BCM_SINGLE_WIDE);
        }
        return (BCM_E_NONE);
    }

    /* Delete next hop table entry. */
    return _bcm_xgs3_tbl_del(unit, &data) ;
}

/*
 * Function:
 *      bcm_xgs3_get_nh_from_egress_object
 * Purpose:
 *       Extract next hop id/Ecmp group id from egress object. 
 *       Increment next hop/ecmp group  reference count
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      intf      - (IN)L3 egress object id.
 *      mpath_flag     - (OUT) multipath indicator
 *      ref_count - (IN) Flag to increment reference_count.
 *      index     - (OUT)Next hop index.
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_get_nh_from_egress_object(int unit, bcm_if_t intf,
                                    uint32 *mpath_flag, int ref_count, int *index)
{
    if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf) || 
        BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, intf)) {

        /* Extract next hop index from unipath egress object. */
        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf)) {
            *index = intf - BCM_XGS3_EGRESS_IDX_MIN;
        } else {
            *index = intf - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        }
        *mpath_flag = 0;

        /* Make sure next hop is in use. */
        if (!BCM_XGS3_L3_ENT_REF_CNT
                 (BCM_XGS3_L3_TBL_PTR(unit, next_hop), *index)) {
            return (BCM_E_PARAM);
        }

        /* Increment next hop reference count. */
        if (ref_count) {
            BCM_XGS3_L3_ENT_REF_CNT_INC
                (BCM_XGS3_L3_TBL_PTR(unit, next_hop), *index, _BCM_SINGLE_WIDE);
        }
    } else if (BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, intf)) {

        /* Extract ecmp group index from multipath egress object. */
        *index = intf - BCM_XGS3_MPATH_EGRESS_IDX_MIN;

        *mpath_flag = BCM_L3_MULTIPATH;

        /* Make sure ecmp group is in use. */
        if (!BCM_XGS3_L3_ENT_REF_CNT
                 (BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp), *index)) {
            return (BCM_E_PARAM);
        }

        /* Increment ecmp group reference count. */
        if (ref_count) {
            BCM_XGS3_L3_ENT_REF_CNT_INC
                (BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp), *index, _BCM_SINGLE_WIDE);
        }
    } else {
        /* Invalid egress object id specified. */
        return (BCM_E_PARAM);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_get_ref_count_from_nhi
 * Purpose:
 *       Extract  egress/multipath object from next hop id/Ecmp group
 *       Decrement next hop/ecmp group  reference count
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      flags     - (IN)Caller flags (eg Multipath route).
 *      ref_count - (IN) pointer to reference_count.
 *      index     - (OUT)Next hop index.
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_get_ref_count_from_nhi(int unit, uint32 flags, int *ref_count, int nh_ecmp_index)
{
   int count=0;

    if ((flags & BCM_L3_MULTIPATH)) {

        /* Make sure ecmp group is in use. */
        count = BCM_XGS3_L3_ENT_REF_CNT
                 (BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp), nh_ecmp_index);
 
        if (count <= 1) {
            *ref_count = count;
            return (BCM_E_PARAM);
        }

        /* Decrement ecmp group reference count. */
        BCM_XGS3_L3_ENT_REF_CNT_DEC
                (BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp), nh_ecmp_index, _BCM_SINGLE_WIDE);
        *ref_count = BCM_XGS3_L3_ENT_REF_CNT
                 (BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp), nh_ecmp_index);

    } else {

        /* Make sure next hop is in use. */
        count = BCM_XGS3_L3_ENT_REF_CNT
                 (BCM_XGS3_L3_TBL_PTR(unit, next_hop), nh_ecmp_index);

        if (count <= 1) {
            *ref_count = count;
            return (BCM_E_NONE);
        }

        /* Decrement next hop reference count. */
        BCM_XGS3_L3_ENT_REF_CNT_DEC
                (BCM_XGS3_L3_TBL_PTR(unit, next_hop), nh_ecmp_index, _BCM_SINGLE_WIDE);

        *ref_count = BCM_XGS3_L3_ENT_REF_CNT
                 (BCM_XGS3_L3_TBL_PTR(unit, next_hop), nh_ecmp_index);

    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_nh_init_add
 * Purpose:
 *       Initialize next hop entry. 
 *       Allocate NextHop table index & Write NextHop entry.
 *       Routine trying to match new entry to an existing one, 
 *       if match found reference cound is incremented, otherwise
 *       new index allocated & entry added to hw. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3cfg     - (IN)L3 host entry. 
 *  OR  <- We always get next hop index either for host OR for a route.       
 *      lpm_cfg   - (IN)Route entry. 
 *      index     - (OUT)Next hop index.  
 * Returns:
 *    BCM_E_XXX
 */

STATIC int
_bcm_xgs3_nh_init_add(int unit, _bcm_l3_cfg_t *l3cfg,
                      _bcm_defip_cfg_t *lpm_cfg, int *index)
{
    bcm_if_t intf;            /* Egress interface index. */
    uint32 flags=0;             /* Next hop entry flags.   */ 
    bcm_l3_egress_t nh_entry; /* Next hop entry.         */

    /* Input parameters check. */
    if ((NULL == index) || ((NULL == l3cfg) && (NULL == lpm_cfg)) || 
        ((NULL != l3cfg) && (NULL != lpm_cfg))) {
        return (BCM_E_PARAM);
    }



    /* Check if egress switching mode is enabled. */
    if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
        /* Get egress object id. */
        intf = (NULL != l3cfg) ? l3cfg->l3c_intf : lpm_cfg->defip_intf;
        flags = (NULL == lpm_cfg) ? 0 : 
            (lpm_cfg->defip_flags & BCM_L3_MULTIPATH);
        if (l3cfg != NULL) {
            if (l3cfg->l3c_flags & BCM_L3_L2TOCPU) {
                *index = BCM_XGS3_L3_L2CPU_NH_IDX;
            } else {
                return bcm_xgs3_get_nh_from_egress_object(unit, intf, &flags, 1, index);
            }
        } else {
            return bcm_xgs3_get_nh_from_egress_object(unit, intf, &flags, 1, index);
        }
    }

    *index = BCM_XGS3_L3_INVALID_INDEX;
    /* Check if traffic should be trapped to cpu. */
    if (l3cfg) {
        if (l3cfg->l3c_flags & BCM_L3_L2TOCPU) {
            *index = BCM_XGS3_L3_L2CPU_NH_IDX;
        }
    } else if (lpm_cfg->defip_flags & BCM_L3_DEFIP_LOCAL) {
        *index = BCM_XGS3_L3_L2CPU_NH_IDX;
    }
    /* If packet should be trapped increment reference count and return. */
    if (*index == BCM_XGS3_L3_L2CPU_NH_IDX) {
        BCM_XGS3_L3_ENT_REF_CNT_INC((BCM_XGS3_L3_TBL_PTR(unit, next_hop)),
                                    BCM_XGS3_L3_L2CPU_NH_IDX, _BCM_SINGLE_WIDE);
        *index = BCM_XGS3_L3_L2CPU_NH_IDX;
        return (BCM_E_NONE);
    }

    /* Real entry required -> init next hop entry. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_nh_entry_init(unit, &nh_entry, l3cfg, lpm_cfg));

    /* Add next hop entry. */
    /*    coverity[negative_returns : FALSE]    */
    return bcm_xgs3_nh_add(unit, 0, &nh_entry, index);
}

/*
 * Function:
 *      _bcm_xgs3_l3_get_nh_info
 * Purpose:
 *      Fill next hop info for l3 entry.
 * Parameters:
 *      unit   - SOC unit number.
 *      l3cfg  - L3 entry to fill.
 *      nh_idx - Next hop index. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_get_nh_info(int unit, _bcm_l3_cfg_t *l3cfg, int nh_idx)
{
    bcm_l3_egress_t nh_info;  /* Next hop info buffer.    */
    /* In egress switching mode - set egress object id only. */
    /* Depends on whether it is a regular L3 next hop or a WLAN DVP */ 
    if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
#ifdef BCM_TRIUMPH2_SUPPORT
        egr_l3_next_hop_entry_t next_hop;
        int entry_type;
        if (soc_feature(unit, soc_feature_wlan)) {
            BCM_IF_ERROR_RETURN
                (READ_EGR_L3_NEXT_HOPm(unit, MEM_BLOCK_ANY, nh_idx, &next_hop));
            entry_type = soc_EGR_L3_NEXT_HOPm_field32_get(unit, &next_hop, 
                                                          ENTRY_TYPEf);
            if (entry_type == 4) {
                l3cfg->l3c_intf = nh_idx + BCM_XGS3_DVP_EGRESS_IDX_MIN;
            } else {
                l3cfg->l3c_intf = nh_idx + BCM_XGS3_EGRESS_IDX_MIN;
            }
        } else        
#endif
        {
            l3cfg->l3c_intf = nh_idx + BCM_XGS3_EGRESS_IDX_MIN;
        }
        return (BCM_E_NONE);
    } 

    /* If next hop is trap to cpu - don't read nh table. */
    if (nh_idx == BCM_XGS3_L3_L2CPU_NH_IDX) {
        /* Set flags to bridge to cpu. */
        l3cfg->l3c_flags |= BCM_L3_L2TOCPU;
        /* Set interface index to last one in the table. */
        l3cfg->l3c_intf = BCM_XGS3_L3_L2CPU_INTF_IDX(unit);
        /* Set module id to a local module. */
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &l3cfg->l3c_modid));
        /* No vlan/interface info. */
        l3cfg->l3c_vid = 0;
        l3cfg->l3c_tunnel = 0;
        /* Set port to be a cpu port. */
        l3cfg->l3c_port_tgid = CMIC_PORT(unit);
        /* Reset mac address. */
        sal_memset(l3cfg->l3c_mac_addr, 0, sizeof(bcm_mac_t));
        /* Set host flags to trap to cpu. */
        l3cfg->l3c_flags |= BCM_L3_L2TOCPU;
        return (BCM_E_NONE);
    }

    /* Get next hop entry from hw. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_nh_get(unit, nh_idx, &nh_info));

    /* Fill next hop info to l3 entry. */
    if (nh_info.flags & BCM_L3_TGID) {       /* Trunk */
        l3cfg->l3c_flags |= BCM_L3_TGID;
    }
    /* Set module id. */
    l3cfg->l3c_modid = nh_info.module;
    /* Set trunk/port info. */
    l3cfg->l3c_port_tgid =
        (nh_info.flags & BCM_L3_TGID) ? nh_info.trunk : nh_info.port;
    /* Set interface index. */
    l3cfg->l3c_intf = nh_info.intf;
    /* Set physical address. */
    sal_memcpy(l3cfg->l3c_mac_addr, nh_info.mac_addr, sizeof(bcm_mac_t));
    /* Get tunnel id info. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_l3_get_tunnel_id(unit, nh_info.intf, &l3cfg->l3c_tunnel));
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        l3cfg->l3c_vid = nh_info.vlan;
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_max_grp_size_get
 * Purpose:
 *      Get maximum number of next hop entries in ecmp group
 * Parameters:
 *      unit           - (IN)SOC unit number.
 *      ecmp_group_id  - (IN)Ecmp group id. 
 *      *grp_sz        - (OUT)Ecmp group size. 
 * Returns:
 *      void
 */
STATIC INLINE int
_bcm_xgs3_ecmp_max_grp_size_get(int unit, int ecmp_group_id, int *grp_sz)
{
    int max_group_size;           /* Maximum ecmp group size. */
#if defined(BCM_TRX_SUPPORT)
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry. 	 */
    int group_size=0;
#endif /* BCM_TRX_SUPPORT */
    int rv = BCM_E_NONE;

#if defined(BCM_TRX_SUPPORT)
    if ((SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) && (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))){
        sal_memset(hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));
        /* Get ECMP group size. */
        rv = soc_mem_read(unit, L3_ECMP_COUNTm, MEM_BLOCK_ANY, (ecmp_group_id+1), hw_buf);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
             group_size = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, COUNTf);
        } else  if (SOC_IS_TRIDENT(unit) || SOC_IS_KATANA(unit)) {
             group_size = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, COUNT_0f);
        } else {
             group_size = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, COUNTf);
        }
        *grp_sz = group_size+1;
    } else
#endif /* BCM_TRX_SUPPORT */
    {

    /* Set number of next hop entries in ecmp group. */
    max_group_size = BCM_XGS3_L3_ECMP_MAX_PATHS(unit);

    /*
     *  If we are writing a last group, it can accomodate 
     *  only  MAX_PATHs -1 entries. Since last entry used to keep
     *  max paths for warm boot.
     */
    if (ecmp_group_id == BCM_XGS3_L3_TBL(unit, ecmp_grp).idx_max) {
        /* 
         * Check if ecmp table uses all the entries.
         * NOTE: Ecmp group id is zero based.
         */
        if ((ecmp_group_id + 1) * max_group_size == 
            BCM_XGS3_L3_ECMP_TBL_SIZE(unit)) {
            /* Last group has 1 entry less. */
            max_group_size--;
        }
    }   

    /* Return max group size to the caller. */
    *grp_sz = max_group_size;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_grp_size_get
 * Purpose:
 *      Get number of next hop entries in ecmp group
 * Parameters:
 *      unit          - (IN)SOC unit number.
 *      ecmp_group_id - (IN)Ecmp group id.
 *      nh_arr        - (IN)Ecmp group memeber next hop entries.
 *      *grp_sz       - (OUT)Ecmp group size. 
 * Returns:
 *      void
 */
STATIC INLINE void
_bcm_xgs3_ecmp_grp_size_get(int unit, int ecmp_group_id, 
                            int *nh_arr, int *grp_sz)
{
    int ecmp_max_paths=0;                 /* Maximum number of paths for group. */
    int ecmp_count;                     /* Ecmp group next hop count.         */
    int idx;                            /* Iteration index.                   */

    /* Calculate maximum group size. */
    (void) _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_id, &ecmp_max_paths);

    /* Check real group size. */
    ecmp_count = 0;
    for (idx = 0; idx < ecmp_max_paths; idx++) {
        if (nh_arr[idx] != 0) {
            ecmp_count++;
        } else {
            break;
        }
    }
    *grp_sz = ecmp_count;
    return;
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_tbl_read
 * Purpose:
 *      Read up to max_cnt values from ecmp table. 
 * Parameters:
 *      unit           - (IN)SOC unit number.
 *      ecmp_group_idx - (IN)First ecmp entry index to read. 
 *      ecmp_grp       - (OUT)Buffer to fill group info.
 *      ecmp_count     - (OUT)Actual ecmp group size.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC INLINE int
_bcm_xgs3_ecmp_tbl_read(int unit, int ecmp_group_idx, 
                        int *ecmp_grp, int *ecmp_count)
{
    int rv = BCM_E_UNAVAIL;     /* Operation return value. */
    int max_group_size = 0;

    rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_idx, &max_group_size);
    if (rv < 0) {
        return rv;
    }

    /* Make sure entry is inuse. */
    if (!BCM_XGS3_L3_ENT_REF_CNT(BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp),
                                 ecmp_group_idx)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Read ecmp group members from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, ecmp_grp_get) && max_group_size) {
        rv = BCM_XGS3_L3_HWCALL_EXEC (unit, ecmp_grp_get) (unit, ecmp_group_idx,
                                                           max_group_size, ecmp_grp);
    }

    /* Get group size. */
    if (ecmp_count) {
        _bcm_xgs3_ecmp_grp_size_get(unit, ecmp_group_idx, ecmp_grp, ecmp_count);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_grp_hash_calc
 * Purpose:
 *      Calculate ecmp group next hops hash(signature). 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      buf      -  (IN)Next hop entry information.
 *      hash     -  (OUT) Hash(signature) calculated value.  
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_ecmp_grp_hash_calc(int unit, void *buf, uint16 *hash)
{

    if ((NULL == buf) || (NULL == hash)) {
        return (BCM_E_PARAM);
    }

    /* Calculate hash of next hop indexes, which are members in the group. */
    *hash = _shr_crc16(0, (uint8 *) buf,
                       BCM_XGS3_L3_ECMP_MAX_PATHS(unit) * sizeof(int));

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_grp_cmp
 * Purpose:
 *      Compare ecmp group next hop members with group in hw.
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      buf        - (IN)Next hop indexes.
 *      grp_idx    - (IN)ECMP group index in hw. 
 *      cmp_result - (OUT)Comparison result. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ecmp_grp_cmp(int unit, void *buf, int grp_idx, int *cmp_result)
{
    int ecmp_max_paths;         /* Maximum number of ecmp paths. */
    int ecmp_count;             /* Count indexes in the group.   */
    int *ecmp_grp;              /* Ecmp group from hw.           */
    int *nh_idx;                /* Ecmp group buffer pointer.    */
    int idx;                    /* Iteration index.              */
    int rv;                     /* Operation return status.      */

    /* Input parameters sanity check. */
    if ((NULL == cmp_result) || (NULL == buf)) {
        return (BCM_E_PARAM);
    }

    /* Get maximum ecmp group size for group index. */
    rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, grp_idx, &ecmp_max_paths);
    if (rv < 0) {
        return rv;
    }

    /* Get next hops array. */
    nh_idx = (int *)buf;

    /* Get ecmp group size. */
    _bcm_xgs3_ecmp_grp_size_get(unit, grp_idx, nh_idx, &ecmp_count);

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

    /* Read ecmp group from hw. */
    rv = _bcm_xgs3_ecmp_tbl_read(unit, grp_idx, ecmp_grp, NULL);
    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_grp);
        return (rv);
    }

    /* Compare ecmp groups. */
    if (sal_memcmp(nh_idx, ecmp_grp, (ecmp_count * sizeof(int)))) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        sal_free(ecmp_grp);
        return (BCM_E_NONE);
    }

    /* Make sure group doesn't contain any additional indexes. */
    for (idx = ecmp_count; idx < ecmp_max_paths; idx++) {
        if (ecmp_grp[idx]) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            sal_free(ecmp_grp);
            return (BCM_E_NONE);
        }
    }
    sal_free(ecmp_grp);
    *cmp_result = BCM_L3_CMP_EQUAL;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_ecmp_group_del
 * Purpose:
 *      Delete ecmp group/update a reference count of existing group 
 *      if it is used by other prefixes.
 * Parameters:
 *      unit           - (IN)SOC unit number.
 *      ecmp_group_id  - (IN)Ecmp group id.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_ecmp_group_del(int unit, int ecmp_group_id)
{
    _bcm_l3_tbl_op_t data;
#ifdef BCM_TRX_SUPPORT
    int max_paths = 0;
#endif

    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);

    /* Check hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, ecmp_grp_del)) {
        return (BCM_E_UNAVAIL);
    }

    /* Calculate ecmp group id. */
    if ((ecmp_group_id > data.tbl_ptr->idx_max) || 
        (ecmp_group_id < data.tbl_ptr->idx_min)) {
        return (BCM_E_PARAM);
    }

    data.entry_index = ecmp_group_id;

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
        if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
              BCM_IF_ERROR_RETURN
                   (_bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_id,
                                              &max_paths));
              if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
                   if (max_paths > 1024) {
                        max_paths = 1024;
                   }                 
                   data.width = _BCM_DOUBLE_WIDE;
              } else if (SOC_IS_TRIDENT(unit)) {
                   if (max_paths > 256) {
                        max_paths = 256;
                   }
                   data.width = _BCM_DOUBLE_WIDE;
              } else if (SOC_IS_SCORPION(unit)) {
                   if (max_paths > 256) {
                        max_paths = 256;
                   }
                   data.width = max_paths;
                   data.oper_flags |= _BCM_L3_SHR_TABLE_TRAVERSE_CONTROL;
              } else {
                   if (max_paths > 32) {
                        max_paths = 32;
                   }
                   data.width = max_paths;
                   if (SOC_IS_TRIUMPH2(unit) ||
                       SOC_IS_APOLLO(unit) || 
                       SOC_IS_VALKYRIE2(unit)) {
                       data.width = _BCM_DOUBLE_WIDE;
                   }
              }
              data.value = max_paths;
         } else {
              return (BCM_E_PARAM);
         }
    } else
#endif
    {
         data.width = _BCM_SINGLE_WIDE;
         data.value = _BCM_SINGLE_WIDE;
    }
    data.delete_func = BCM_XGS3_L3_HWCALL(unit, ecmp_grp_del);

    /* Delete ecmp group table entry. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_tbl_del(unit, &data));

    /* If last ecmp group is gone reset ecmp_inuse flag. */
    if ((0 == data.tbl_ptr->idx_maxused) &&
        (BCM_XGS3_L3_ENT_REF_CNT(data.tbl_ptr, 0) == 0)) {
        BCM_XGS3_L3_ECMP_IN_USE(unit) = 0;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_group_add
 * Purpose:
 *      Write ecmp group to the hw.
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      flags        - (IN)BCM_L3_REPLACE && BCM_L3_WITH_ID
 *      ecmp_count   - (IN)Ecmp group next hops count. 
 *      ecmp_group   - (IN)Ecmp group.
 *      group_index  - (OUT)Ecmp group index
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ecmp_group_add(int unit, uint32 flags, int ecmp_count, int max_paths,
                        int *ecmp_group, int *group_index)
{
    _bcm_l3_tbl_op_t data;        /* Operation data. */
    /* Make sure ecmp group write call is defined for the device. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, ecmp_grp_add)) {
        return (BCM_E_UNAVAIL);
    }

    /*
     * Next hop indexes must be sorted in the group 
     * in order to produce identical hash every time. 
     */
    _shr_sort(ecmp_group, ecmp_count, sizeof(int), _bcm_xgs3_cmp_int);


    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);
    data.oper_flags = flags;

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
        if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
              /* Limit Max_paths */
              if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
                   if (max_paths > 1024) {
                        max_paths = 1024;
                   }                 
                   data.width = _BCM_DOUBLE_WIDE;
              } else if (SOC_IS_TRIDENT(unit)) {
                   if (max_paths > 256) {
                        max_paths = 256;
                   }
                   data.width = _BCM_DOUBLE_WIDE;
              } else if (SOC_IS_SCORPION(unit)) {
                   if (max_paths > 256) {
                        max_paths = 256;
                   }
                   data.width = max_paths;
              } else {
                   if (max_paths > 32) {
                        max_paths = 32;
                   }
                   data.width = max_paths;
                   if (SOC_IS_TRIUMPH2(unit) ||
                       SOC_IS_APOLLO(unit) ||
                       SOC_IS_VALKYRIE2(unit)) {
                       data.width = _BCM_DOUBLE_WIDE;
                   }
              }
              data.value = max_paths;
         } else {
              return (BCM_E_PARAM);
         }
    } else
#endif  /* BCM_TRX_SUPPORT */
   {
         data.width = _BCM_SINGLE_WIDE;
         data.value = _BCM_SINGLE_WIDE;
   }
   if (flags & _BCM_L3_SHR_WITH_ID) {
         data.entry_index = *group_index;
    } 
    data.entry_buffer = (void *)ecmp_group;
    data.hash_func = _bcm_xgs3_ecmp_grp_hash_calc;
    data.cmp_func  = _bcm_xgs3_ecmp_grp_cmp;
    data.add_func  = BCM_XGS3_L3_HWCALL(unit, ecmp_grp_add);
    /* Add ecmp group. */
    BCM_IF_ERROR_RETURN (_bcm_xgs3_tbl_add(unit, &data));
    *group_index = data.entry_index;
    /* Indicate that at least one ecmp path is present. */
    if (!BCM_XGS3_L3_ECMP_IN_USE(unit)) {
        BCM_XGS3_L3_ECMP_IN_USE(unit) = 1;
    }

    return (BCM_E_NONE);
}

/*
 * Functions:
 *      _bcm_xgs3_l3_multipathCountUpdate
 * Description:
 *      Helper function to update L3-DEFIP.ECMP_Count via DMA 
 * Parameters:
 *     unit         device number
 * Return:
 *     BCM_E_XXX
 */

int
_bcm_xgs3_l3_multipathCountUpdate (int unit, int set)
{
    int idx;                      /* Iteration index. */
    char *lpm_tbl_ptr;    /* Dma table pointer.*/
    defip_entry_t *lpm_entry;    /* Hw entry buffer. */
    int defip_table_size;  /* Defip table size. */
    int ecmp_count=0;             /* Next hop count in the group.        */
    int *ecmp_grp;              /* Ecmp group from hw.                 */
    int ecmp_group_idx;
    int rv=BCM_E_NONE, update_flag=0;
    soc_mem_t mem;         /* IPv4 in IPv6 table memory.    */

    mem = BCM_XGS3_L3_MEM(unit, defip);

    if (!set) {
       return BCM_E_NONE;
    }

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

    /* Table DMA the LPM table to software copy */
    rv = bcm_xgs3_l3_tbl_dma(unit, mem,
              BCM_XGS3_L3_ENT_SZ(unit, defip), "lpm_tbl",
                   &lpm_tbl_ptr, &defip_table_size);
    if (BCM_FAILURE(rv)) {
         sal_free(ecmp_grp);
         return (rv);
    }

    _bcm_esw_l3_lock(unit);

    for (idx = 0; idx < defip_table_size; idx++) {
         /* Calculate entry ofset. */
         lpm_entry =
              soc_mem_table_idx_to_pointer(unit, mem,
                             defip_entry_t *, lpm_tbl_ptr, idx);
         if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, VALID0f)) {
              if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, ECMP0f)) {
                   ecmp_group_idx = soc_L3_DEFIPm_field32_get(unit, lpm_entry, ECMP_PTR0f);
                   rv = _bcm_xgs3_ecmp_tbl_read(unit, ecmp_group_idx, ecmp_grp, &ecmp_count);
                   if (rv != BCM_E_NOT_FOUND) {
                        if (BCM_FAILURE(rv)) {
                            sal_free(ecmp_grp);
                            _bcm_esw_l3_unlock(unit);
                            return (rv);
                        }
                        soc_L3_DEFIPm_field32_set(unit, lpm_entry, ECMP_COUNT0f, ecmp_count);
                        update_flag = 1;
                   }
              }
         }

         if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, VALID1f)) {
              if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, ECMP1f)) {
                   ecmp_group_idx = soc_L3_DEFIPm_field32_get(unit, lpm_entry, ECMP_PTR1f);
                   rv = _bcm_xgs3_ecmp_tbl_read(unit, ecmp_group_idx, ecmp_grp, &ecmp_count);
                   if (rv != BCM_E_NOT_FOUND) {
                        if (BCM_FAILURE(rv)) {
                             sal_free(ecmp_grp);
                             _bcm_esw_l3_unlock(unit);
                             return (rv);
                        }
                        soc_L3_DEFIPm_field32_set(unit, lpm_entry, ECMP_COUNT1f, ecmp_count);
                        update_flag = 1;
                   }
              }
         }
#ifdef BCM_TRIDENT_SUPPORT
         if (SOC_IS_TRIDENT(unit) && update_flag) {
             defip_hit_only_y_entry_t hit_entry_y;
             uint32 hit;
             BCM_IF_ERROR_RETURN
                 (BCM_XGS3_MEM_READ(unit, L3_DEFIP_HIT_ONLY_Ym, idx,
                                    &hit_entry_y));
             hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym,
                                       &hit_entry_y, HIT0f);
             hit |= soc_mem_field32_get(unit, L3_DEFIPm, lpm_entry, HIT0f);
             soc_mem_field32_set(unit, L3_DEFIPm, lpm_entry, HIT0f, hit);
             hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym,
                                       &hit_entry_y, HIT1f);
             hit |= soc_mem_field32_get(unit, L3_DEFIPm, lpm_entry, HIT1f);
             soc_mem_field32_set(unit, L3_DEFIPm, lpm_entry, HIT1f, hit);
         }
#endif /* BCM_TRIDENT_SUPPORT */
    }
    /* Write the modified  buffer back. */
    if (update_flag) {
        rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL,
                                 soc_mem_index_min(unit, mem),
                                 soc_mem_index_max(unit, mem), lpm_tbl_ptr); 
    }
    sal_free(ecmp_grp);
    soc_cm_sfree(unit, lpm_tbl_ptr);
    _bcm_esw_l3_unlock(unit);
    return (rv);
}


#if defined (BCM_TRIUMPH_SUPPORT)
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
bcm_xgs3_l3_ingress_create(int unit, bcm_l3_ingress_t *ing_intf, bcm_if_t *intf_id)
{
    int rv = BCM_E_UNAVAIL;
    _bcm_l3_ingress_intf_t  iif;         /* Ingress interface. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check L3 Ingress Interface mode. */ 
    if (!(BCM_XGS3_L3_INGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ((NULL == ing_intf) || (NULL == intf_id)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&iif, 0, sizeof(_bcm_l3_ingress_intf_t)); 
    if ((ing_intf->flags & BCM_L3_INGRESS_WITH_ID) || (ing_intf->flags & BCM_L3_INGRESS_REPLACE)) {
         if (ing_intf->flags & BCM_L3_INGRESS_WITH_ID){
             if ( (*intf_id < (BCM_VLAN_MAX+1)) ||  (*intf_id > BCM_XGS3_L3_ING_IF_TBL_SIZE(unit))) {
                 return (BCM_E_PARAM);
             }
             if (0 == SHR_BITGET(BCM_XGS3_L3_ING_IF_INUSE(unit), *intf_id)) {
                 iif.intf_id = *intf_id;
                 iif.flags |= BCM_L3_INGRESS_WITH_ID;
             } else {
                return BCM_E_EXISTS;
             }
         } else if (ing_intf->flags & BCM_L3_INGRESS_REPLACE){
             if (0 != SHR_BITGET(BCM_XGS3_L3_ING_IF_INUSE(unit), *intf_id)) {
                 iif.intf_id = *intf_id;
                 iif.flags |= BCM_L3_INGRESS_REPLACE;
             } else {
                 return BCM_E_PARAM;
             }
         }
    }

    /* Disable global route */
    if (!(ing_intf->flags & BCM_L3_INGRESS_GLOBAL_ROUTE)) {
        iif.flags |= BCM_VLAN_L3_VRF_GLOBAL_DISABLE;
    }

    /* Disable URPF Default Route Check */
    if (!(ing_intf->flags & BCM_L3_INGRESS_URPF_DEFAULT_ROUTE_CHECK)) {
        iif.flags |= BCM_VLAN_L3_URPF_DEFAULT_ROUTE_CHECK_DISABLE;
    }

    if (ing_intf->flags & BCM_L3_INGRESS_DSCP_TRUST) {
        iif.qos_map_id = ing_intf->qos_map_id;
        iif.flags |= BCM_L3_INGRESS_DSCP_TRUST;
    }

    iif.vrf = ing_intf->vrf;
    iif.if_class = ing_intf->intf_class;
    iif.ipmc_intf_id = ing_intf->ipmc_intf_id;
    iif.urpf_mode = ing_intf->urpf_mode;
    rv =_bcm_xgs3_l3_ingress_interface_add (unit, &iif);
    if (BCM_SUCCESS(rv)) {
        *intf_id = iif.intf_id;
    }
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_ingress_destroy
 * Purpose:
 *      Destroy an Ingress Interface object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf_id    - (IN) L3 Ingress interface id pointing to Ingress object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_xgs3_l3_ingress_destroy(int unit, bcm_if_t intf_id)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check L3 Ingress Interface mode. */ 
    if (!(BCM_XGS3_L3_INGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ( (intf_id < 0) ||  (intf_id > BCM_XGS3_L3_ING_IF_TBL_SIZE(unit))) {
         return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN 
          (_bcm_xgs3_l3_ingress_interface_delete(unit, intf_id));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_xgs3_l3_ingress_get
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
bcm_xgs3_l3_ingress_get(int unit, bcm_if_t intf_id, bcm_l3_ingress_t *ing_intf)
{
    int rv = BCM_E_UNAVAIL;
    _bcm_l3_ingress_intf_t  iif;         /* Ingress interface. */

    /*	Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
       return (BCM_E_INIT);
    }

    /* Check L3 Ingress Interface mode. */ 
    if (!(BCM_XGS3_L3_INGRESS_MODE_ISSET(unit))) {
       return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ( (intf_id < 0) ||  (intf_id > BCM_XGS3_L3_ING_IF_TBL_SIZE(unit))) {
       return (BCM_E_PARAM);
    }

    iif.intf_id = intf_id;

    rv = _bcm_xgs3_l3_ingress_interface_get(unit, &iif);
    if (BCM_SUCCESS(rv)) {
         ing_intf->vrf = iif.vrf;
         ing_intf->intf_class = iif.if_class;
         ing_intf->ipmc_intf_id = iif.ipmc_intf_id;
         ing_intf->urpf_mode = iif.urpf_mode;
         if ( !(iif.flags & BCM_VLAN_L3_VRF_GLOBAL_DISABLE)) {
              ing_intf->flags |= BCM_L3_INGRESS_GLOBAL_ROUTE;
         }
         if ( !(iif.flags & BCM_VLAN_L3_URPF_DEFAULT_ROUTE_CHECK_DISABLE)) {
              ing_intf->flags |= BCM_L3_INGRESS_URPF_DEFAULT_ROUTE_CHECK;
         }
         if (iif.flags & BCM_L3_INGRESS_DSCP_TRUST) {
              ing_intf->qos_map_id = iif.qos_map_id;
              ing_intf->flags |= BCM_L3_INGRESS_DSCP_TRUST;
         }
    }
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_ingress_find
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
bcm_xgs3_l3_ingress_find(int unit, bcm_l3_ingress_t *ing_intf, 
                        bcm_if_t *intf_id)
{
    _bcm_l3_tbl_op_t data;         /* Operation data.      */
    _bcm_l3_ingress_intf_t  iif;         /* Ingress interface. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_INGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ((NULL == ing_intf) || (NULL == intf_id)) {
        return (BCM_E_PARAM);
    }

    sal_memset(&iif, 0, sizeof(_bcm_l3_ingress_intf_t)); 
    /* Verify Ingress interface object range if BCM_L3_INGRESS_WITH_ID flag is set. */ 
    if (ing_intf->flags & BCM_L3_INGRESS_WITH_ID) {
        if ( (*intf_id < (BCM_VLAN_MAX+1)) ||  (*intf_id > BCM_XGS3_L3_ING_IF_TBL_SIZE(unit))) {
             return (BCM_E_PARAM);
        } else {
            iif.intf_id = *intf_id;
            iif.flags |= BCM_L3_WITH_ID;
        }
    }

    /* Disable global route */
    if (!(ing_intf->flags & BCM_L3_INGRESS_GLOBAL_ROUTE)) {
         iif.flags |= BCM_VLAN_L3_VRF_GLOBAL_DISABLE;
    }
    
    /* Disable URPF Default Route Check */
    if ( !(ing_intf->flags & BCM_L3_INGRESS_URPF_DEFAULT_ROUTE_CHECK)) {
         iif.flags  |= BCM_VLAN_L3_URPF_DEFAULT_ROUTE_CHECK_DISABLE;
    }

    iif.vrf = ing_intf->vrf;
    iif.if_class = ing_intf->intf_class;
    iif.urpf_mode = ing_intf->urpf_mode;

    if (ing_intf->flags & BCM_L3_INGRESS_DSCP_TRUST) {
        iif.qos_map_id = ing_intf->qos_map_id;
        iif.flags |= BCM_L3_INGRESS_DSCP_TRUST;
    }

    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, ing_intf);
    data.width = _BCM_SINGLE_WIDE;
    data.entry_buffer = (void *)&iif;
    data.hash_func = _bcm_xgs3_nh_hash_calc;
    data.cmp_func  = _bcm_xgs3_nh_ent_cmp;

    /* Find next hop entry. */
    BCM_IF_ERROR_RETURN (_bcm_xgs3_tbl_match(unit, &data)); 
    *intf_id = data.entry_index; 

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_ing_intf_traverse_cb
 * Purpose:
 *      Call user callback for valid L3_IIF entries.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern (interface id & negate).
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_ing_intf_traverse_cb(int unit, void *pattern, void *data1, 
                         void *data2, int *cmp_result)
{
    _bcm_l3_trvrs_data_t *trv_data;     /* Travers data.    */
    bcm_l3_ingress_t  *ing_intf;              /* Ingress object.   */
    int intf_id;                         /* L3 IIF index.  */


    /* Cast input parameters. */
    trv_data = (_bcm_l3_trvrs_data_t *) pattern;
    ing_intf      = (bcm_l3_ingress_t *) data1;
    intf_id   = *(int *)data2;

    /* Call user callback. */  
    if (NULL != trv_data->ingress_cb) {
        (*trv_data->ingress_cb) (unit, intf_id, ing_intf, trv_data->cookie);
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_ingress_traverse
 * Purpose:
 *      Goes through ingress interface objects table and runs the user callback
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
bcm_xgs3_l3_ingress_traverse(int unit, bcm_l3_ingress_traverse_cb trav_fn,
                            void *user_data)
{
    _bcm_l3_trvrs_data_t trv_data;     /* Travers initiator function input. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_INGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if (NULL == trav_fn) {
        return (BCM_E_PARAM);
    }

    /* Zero traverse info. */
    sal_memset(&trv_data, 0, sizeof(_bcm_l3_trvrs_data_t));

    /* Fill traverse data struct. */
    trv_data.op_cb = _bcm_xgs3_ing_intf_traverse_cb; 
    trv_data.ingress_cb = trav_fn;
    trv_data.cookie = user_data;

    /*  Update_match */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, ing_intf_update_match)) {
        return BCM_XGS3_L3_HWCALL_EXEC(unit, ing_intf_update_match) (unit, &trv_data);
    }

    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *      _bcm_xgs3_ing_intf_entry_parse
 * Purpose:
 *      Parse the L3 Ingress Interface  table entry info.
 * Parameters:
 *      unit          - (IN) SOC unit number.
 *      ing_entry_ptr - (IN) Ingress Interface table entry. 
 *      bcm_l3_ingress_t      - (OUT) Ingress Interface table entry info. 
 * Returns:
 *      BCM_E_XXX
 */
 
STATIC int
_bcm_xgs3_ing_intf_entry_parse(int unit, uint32 *ing_entry_ptr, 
                         bcm_l3_ingress_t *ing_intf_entry)
{
    soc_mem_t mem;                      /* Table location memory.           */
    int hw_map_idx=0;

    /* Zero buffers. */
    sal_memset(ing_intf_entry, 0, sizeof(bcm_l3_ingress_t));

    /* Get next hop table memory location. */
    mem = BCM_XGS3_L3_MEM(unit, ing_intf);

    /* Get class id value. */
    ing_intf_entry->intf_class = soc_mem_field32_get(unit, mem, ing_entry_ptr, CLASS_IDf);

    /* Get vrf id value. */
    ing_intf_entry->vrf = soc_mem_field32_get(unit, mem, ing_entry_ptr, VRFf);

    /* Get Flags value. */
    if (0 == soc_mem_field32_get(unit, mem, ing_entry_ptr,
                                 ALLOW_GLOBAL_ROUTEf)) {
        ing_intf_entry->flags |= BCM_VLAN_L3_VRF_GLOBAL_DISABLE; 
    }

    /* Get Trust QoS Map Id Value. */
    hw_map_idx = soc_mem_field32_get(unit, mem, ing_entry_ptr,
                                TRUST_DSCP_PTRf);
    if (hw_map_idx != 63) {
         if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
              BCM_IF_ERROR_RETURN
                   (_bcm_tr_qos_idx2id(unit, hw_map_idx, 0x3,
                        &ing_intf_entry->qos_map_id));
         } else {
#ifdef BCM_TRIUMPH2_SUPPORT
              BCM_IF_ERROR_RETURN
                   (_bcm_tr2_qos_idx2id(unit, hw_map_idx, 0x3,
                        &ing_intf_entry->qos_map_id));
#endif /*BCM_TRIUMPH2_SUPPORT*/
         }
         ing_intf_entry->flags |= BCM_L3_INGRESS_DSCP_TRUST; 
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_mem_field_valid(unit, mem, IPMC_L3_IIFf)) {
         ing_intf_entry->ipmc_intf_id = soc_mem_field32_get(unit, mem, 
                                                                     ing_entry_ptr, IPMC_L3_IIFf);
    }
#endif    

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_mem_field_valid(unit, L3_IIFm, URPF_MODEf)) {
         ing_intf_entry->urpf_mode = soc_mem_field32_get(unit, mem, 
                                                                ing_entry_ptr, URPF_MODEf);
    }
    
    if (soc_mem_field_valid(unit, L3_IIFm, URPF_DEFAULTROUTECHECKf)) {
         if (0 == soc_mem_field32_get(unit, mem, ing_entry_ptr,
                                       URPF_DEFAULTROUTECHECKf)) {
              ing_intf_entry->flags |= BCM_VLAN_L3_URPF_DEFAULT_ROUTE_CHECK_DISABLE; 
         }
    }
#endif
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_xgs3_ing_intf_update_match
 * Purpose:
 *      Update/Show/Delete all entries in L3_IIF table matching a certain rule.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      trv_data - (IN)Delete pattern + compare,act,notify routines.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ing_intf_update_match(int unit, _bcm_l3_trvrs_data_t *trv_data)
{
    bcm_l3_ingress_t  ing_intf_entry;     /* Next hop entry info.           */
    uint32 *ing_entry_ptr;        /* Ingress L3-IIF Table entry pointer.*/ 
    char *ing_tbl_ptr;            /* Dma table pointer.             */
    int cmp_result;               /* Test routine result.           */
    soc_mem_t mem;                /* Next hop memory.               */
    int idx,start_iif;                      /* Iteration index.               */
    int rv = BCM_E_NONE;          /* Operation return status.       */

    /* Get L3-IIF Table memory. */
    mem = BCM_XGS3_L3_MEM(unit, ing_intf);
    start_iif = BCM_VLAN_MAX+1;

    /* Table DMA the L3-IIF table to software copy */
    BCM_IF_ERROR_RETURN 
        (bcm_xgs3_l3_tbl_dma(unit, mem, BCM_XGS3_L3_ENT_SZ(unit, ing_intf), "ing_intf_tbl",
                             &ing_tbl_ptr, NULL));

    for (idx = start_iif; idx < BCM_XGS3_L3_ING_IF_TBL_SIZE(unit); idx++) {
        /* Skip unused entries. */
        if (0 == SHR_BITGET(BCM_XGS3_L3_ING_IF_INUSE(unit), idx)) {
            continue;
        }

        /* Calculate entry ofset. */
        ing_entry_ptr =
            soc_mem_table_idx_to_pointer(unit, mem, uint32 *, ing_tbl_ptr, idx);

        (void) _bcm_xgs3_ing_intf_entry_parse(unit, ing_entry_ptr, &ing_intf_entry);

        /* Execute operation routine if any. */
        if (trv_data->op_cb) {
            rv = (*trv_data->op_cb)(unit, (void *)trv_data,
                                    (void *)&ing_intf_entry,(void *)&idx, &cmp_result);
            if (rv < 0) {
                break;
            }
        }
    }
    soc_cm_sfree(unit, ing_tbl_ptr);
    return (rv);
}

#endif /*BCM_TRIUMPH_SUPPORT*/

/*
 * Function:
 *      bcm_xgs3_l3_egress_id_parse
 * Purpose:
 *      Parse Egress object, to return flag and next hop / ecmp group id.
 * Parameters:
 *      unit       - (IN)  bcm device.
 *      intf       - (IN)  Interface id pointing to egress object or
 *                         multipath egress object.      
 *      flags      - (OUT) BCM_L3_MULTIPATH if interface points to multipath
 *                         egress object.
 *      nh_ecmp_id - (OUT) Next hop /ecmp group id.  
 * Returns:
 *      BCM_E_DISABLED - if advanced egress switching disabled.
 *      BCM_E_PARAM    - if invalid interface id passed. 
 *      BCM_E_NONE     - otherwise.   
 */
int
bcm_xgs3_l3_egress_id_parse(int unit, bcm_if_t intf, 
                            uint32 *flags, int *nh_ecmp_id)
{
    /* Check if L3 feature is supported on the device */
    if (!soc_feature(unit, soc_feature_l3)) {
        return (BCM_E_UNAVAIL);
    }

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ((NULL == flags) || (NULL == nh_ecmp_id)) {
        return (BCM_E_PARAM);
    }
   
    if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf)) {
        *nh_ecmp_id = intf - BCM_XGS3_EGRESS_IDX_MIN;
        /* Make sure next hop is in use. */
        if (!BCM_XGS3_L3_ENT_REF_CNT
              (BCM_XGS3_L3_TBL_PTR(unit, next_hop), *nh_ecmp_id)) {
                   return (BCM_E_PARAM);
        }
        *flags = 0;
    } else if (BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, intf)) {
        *nh_ecmp_id = intf - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        /* Make sure next hop is in use. */
        if (!BCM_XGS3_L3_ENT_REF_CNT
              (BCM_XGS3_L3_TBL_PTR(unit, next_hop), *nh_ecmp_id)) {
                   return (BCM_E_PARAM);
        }
        *flags = 0;
    } else if (BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, intf)) {
        *nh_ecmp_id = intf - BCM_XGS3_MPATH_EGRESS_IDX_MIN;
        /* Make sure ecmp grp is in use. */
        if (!BCM_XGS3_L3_ENT_REF_CNT
              (BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp), *nh_ecmp_id)) {
                   return (BCM_E_PARAM);
        }
        *flags = BCM_L3_MULTIPATH;
    } else {
        return (BCM_E_PARAM);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_egress_to_nh_info
 * Purpose:
 *      Map API egress object data to hw used next hop format.
 * Parameters:
 *      unit       - (IN)     bcm device.
 *      egr        - (IN)     Egress object properties to match.  
 *      nh_info    - (IN/OUT) L3 interface id pointing to egress object
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_xgs3_l3_egress_to_nh_info(int unit, bcm_l3_egress_t *egr,
                               bcm_l3_egress_t *nh_info)
{
#ifdef BCM_FIREBOLT_SUPPORT
    _bcm_l3_intf_cfg_t l3_intf;     /* Outgoing interface info. */
    int interface_map_mode=0;
#endif /* BCM_FIREBOLT_SUPPORT  */

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_l3_ingress_interface)) {
         /* Check L3 Ingress Interface Map mode. */ 
        BCM_IF_ERROR_RETURN (bcm_xgs3_l3_ingress_intf_map_get(unit, &interface_map_mode));
    }
#endif /* BCM_TRIUMPH_SUPPORT */


    /* Copy user info to local structure to preserve user structure. */
    *nh_info = *egr;   

    /* If trap to cpu required overwrite mac/module/intf/port. */
    if (nh_info->flags & BCM_L3_L2TOCPU) {
        /* Set module id to a local module. */
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &nh_info->module));

        /* Set port to be a cpu port. */
        nh_info->port = CMIC_PORT(unit);

        /* Set interface to be cpu interface. */
        nh_info->intf = BCM_XGS3_L3_L2CPU_INTF_IDX(unit);
    }
#ifdef BCM_FIREBOLT_SUPPORT
    /* Firebolt devices need interface info to fill vid & tunnel next hop. */
    else if (SOC_IS_FBX(unit)) { 
        /* Read interface information. */
        /* Verify interface index range. */
        if (nh_info->intf >= BCM_XGS3_L3_IF_TBL_SIZE(unit)) {
            return (BCM_E_PARAM);
        }
        sal_memset(&l3_intf, 0, sizeof(_bcm_l3_intf_cfg_t));
        BCM_XGS3_L3_INTF_IDX_SET(l3_intf, nh_info->intf);
        if (BCM_XGS3_L3_HWCALL_EXEC(unit, if_get) (unit, &l3_intf) >= 0) {
            if (interface_map_mode) {
                nh_info->vlan = egr->vlan;
            } else {
                /* Set vid to next hop entry. */
                nh_info->vlan = l3_intf.l3i_vid;
                if (!BCM_VLAN_VALID(nh_info->vlan)) {
                     return (BCM_E_CONFIG);
                }
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT  */
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_egress_create
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
bcm_xgs3_l3_egress_create(int unit, uint32 flags, bcm_l3_egress_t *egr, 
                          bcm_if_t *intf)
{
    int index;                      /* Next hop index.     */ 
    bcm_l3_egress_t   nh_info;      /* Next hop entry.     */   
    uint32  shr_flags;              /* Table shared flags. */
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
    uint32  entry_type=0;              /* Next Hop Entry_type */
#endif /* BCM_TRIUMPH_SUPPORT  && BCM_MPLS_SUPPORT */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ((NULL == egr) || (NULL == intf)) {
        return (BCM_E_PARAM);
    }

    /* Verify egress object range if BCM_L3_WITH_ID flag is set. */ 
    if (flags & BCM_L3_WITH_ID) {
        if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, *intf) && 
            !BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, *intf)) {
             return (BCM_E_PARAM);
        }
        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, *intf)) {
            index = *intf - BCM_XGS3_EGRESS_IDX_MIN;
        } else {
            index = *intf - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        }
    }

    /* Convert api egress object to hw next hop entry format. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_egress_to_nh_info(unit, egr, &nh_info));

    /* Convert api flags to shared table management flags. */ 
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_flags_to_shr(flags, &shr_flags));

   /* Store api flags within egr object flags */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_flags_to_egr_obj(flags, &nh_info));

    /* Disable matching entry lookup. */
    shr_flags |= _BCM_L3_SHR_MATCH_DISABLE;

    /* Create next hop entry. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_nh_add(unit, shr_flags, &nh_info, &index));

    /* Return Egress object id 100K + nh_index for regular L3 egress objects */
    /* Return BCM_XGS3_DVP_EGRESS_IDX_MIN + nh_index for L3 egress WLAN DVP */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_wlan) && egr->encap_id > 0) {
        *intf = BCM_XGS3_DVP_EGRESS_IDX_MIN + index;
    } else
#endif
    {
        *intf = BCM_XGS3_EGRESS_IDX_MIN + index; 
    }

#if defined(BCM_TRIDENT_SUPPORT)
    if ( soc_feature(unit, soc_feature_trill) && (egr->flags & BCM_L3_TRILL_ONLY)){
         return (bcm_td_trill_egress_set(unit, index));
    }
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_mpls_failover)) {
         if (egr->failover_id > 0 && egr->failover_id < 1024) {
                 BCM_IF_ERROR_RETURN(bcm_tr2_failover_prot_nhi_create( unit, index ));
                 BCM_IF_ERROR_RETURN(bcm_tr2_failover_prot_nhi_set(
                                         unit, index, (uint32) (egr->failover_if_id - BCM_XGS3_EGRESS_IDX_MIN), egr->failover_id ));
         }
    }
#endif /* BCM_TRIUMPH2_SUPPORT  */

#if defined (BCM_EASYRIDER_SUPPORT) && defined (BCM_MPLS_SUPPORT)
    if (SOC_IS_EASYRIDER (unit)) {
         if (!(flags &  BCM_L3_ROUTE_LABEL)) { 
              if (egr->mpls_label != BCM_MPLS_LABEL_INVALID) {
                   return BCM_E_UNAVAIL;
              }
         }
    }
#endif /* BCM_EASYRIDER_SUPPORT && BCM_MPLS_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
    if ((SOC_IS_TR_VL(unit)) && (soc_feature(unit, soc_feature_mpls))) {
        if (flags &  BCM_L3_REPLACE) {
            /* Update the VP NextHops -- For L2 MPLS and L3 SWAP */
            BCM_IF_ERROR_RETURN(bcm_tr_mpls_update_vp_nh (unit, *intf));
        }
        if (flags &  BCM_L3_ROUTE_LABEL) {
             if (egr->mpls_label != BCM_MPLS_LABEL_INVALID) {
                  /* Case of PUSH Inner-Label */
                  BCM_IF_ERROR_RETURN(
                       bcm_tr_mpls_get_entry_type(unit, index, &entry_type));
                  if (entry_type == 0) {
                       BCM_IF_ERROR_RETURN(
                            bcm_tr_mpls_egress_entry_modify(unit, index, 1));
                  }
                  BCM_IF_ERROR_RETURN(
                       bcm_tr_mpls_l3_label_add (unit, egr, index, flags));
              }
        } else  if (!(flags &  BCM_L3_ROUTE_LABEL)) { 
              if (egr->mpls_label != BCM_MPLS_LABEL_INVALID ) {
                  /* Case of SWAP-OUT Label */
                  BCM_IF_ERROR_RETURN(
                       bcm_tr_mpls_get_entry_type(unit, index, &entry_type));
                  if (entry_type == 0) {
                       BCM_IF_ERROR_RETURN(
                           bcm_tr_mpls_egress_entry_modify(unit, index, 1));
                  }
                  BCM_IF_ERROR_RETURN(
                           bcm_tr_mpls_swap_nh_info_add (unit, egr, index, flags));
              } else if (egr->mpls_label == BCM_MPLS_LABEL_INVALID ) {
                  /* Case of Egress into MPLS Tunnel */
                  if (BCM_SUCCESS(bcm_tr_mpls_tunnel_intf_valid (unit, index))) {
                       BCM_IF_ERROR_RETURN(
                            bcm_tr_mpls_get_entry_type(unit, index, &entry_type));
                       if (entry_type == 0) {
                            BCM_IF_ERROR_RETURN(
                                 bcm_tr_mpls_egress_entry_modify(unit, index, 1));
                       }
                  }
              }
        }
    } 
#endif /* BCM_TRX_SUPPORT  && BCM_MPLS_SUPPORT */

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_destroy
 * Purpose:
 *      Destroy an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN) L3 interface id pointing to Egress object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_xgs3_l3_egress_destroy(int unit, bcm_if_t intf)
{
    bcm_l3_egress_t  egr;     /* Next hop info.           */ 
    int nh_idx;               /* Next hop index.          */
    int rv;                   /* Operation return status. */
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
    uint32  entry_type=0;              /* Next Hop Entry_type */
#endif /* BCM_TRIUMPH_SUPPORT  && BCM_MPLS_SUPPORT */


    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf) && 
        !BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, intf)) {
        return (BCM_E_PARAM);
    }

    /* Calculate next hop index. */
    if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf)) {
        nh_idx = intf - BCM_XGS3_EGRESS_IDX_MIN;
    } else {
        nh_idx = intf - BCM_XGS3_DVP_EGRESS_IDX_MIN;
    }

    /* If entry is used by hosts/routes or ecmp objects - reject deletion */
    if(1 < BCM_XGS3_L3_ENT_REF_CNT
       (BCM_XGS3_L3_TBL_PTR(unit, next_hop), nh_idx)) {
        return (BCM_E_BUSY);
    }

    rv = bcm_xgs3_l3_egress_get(unit, intf, &egr);
    BCM_IF_ERROR_RETURN(rv);

#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT)
    if ((SOC_IS_TR_VL(unit)) && (soc_feature(unit, soc_feature_mpls))) {
        /* coverity[uninit_use] */
        BCM_IF_ERROR_RETURN(bcm_tr_mpls_get_entry_type(unit, nh_idx, &entry_type));
        if (entry_type == 1) {
            if (egr.flags &  BCM_L3_ROUTE_LABEL) { 
                if (egr.mpls_label != BCM_MPLS_LABEL_INVALID) {
                    rv = bcm_tr_mpls_l3_label_delete (unit, nh_idx);
                    BCM_IF_ERROR_RETURN(rv);
                }
            } else if (!(egr.flags &  BCM_L3_ROUTE_LABEL)) {
                if (egr.mpls_label != BCM_MPLS_LABEL_INVALID) {
                    rv = bcm_tr_mpls_swap_nh_info_delete (unit, nh_idx);
                    BCM_IF_ERROR_RETURN(rv);
                 }
            }
        }
    } 
#endif /* BCM_TRIUMPH_SUPPORT && BCM_MPLS_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT) 
    if (soc_feature(unit, soc_feature_mpls_failover))  {
        if (egr.failover_id > 0 && egr.failover_id < 1024) {
            rv = bcm_tr2_failover_prot_nhi_cleanup  (unit, nh_idx);
            if ( (rv != BCM_E_NOT_FOUND) && (rv != BCM_E_NONE) ) {
                return rv;
            }
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT  */

    /* Delete next hop entry. */
    rv = bcm_xgs3_nh_del(unit, 0, nh_idx);
    return (rv);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_get
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
bcm_xgs3_l3_egress_get (int unit, bcm_if_t intf, bcm_l3_egress_t *egr) 
{
    int offset;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ((NULL == egr) || (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf) && 
         !BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, intf))) {
        return (BCM_E_PARAM);
    }
    if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf)) {
        offset = BCM_XGS3_EGRESS_IDX_MIN;
    } else {
        offset = BCM_XGS3_DVP_EGRESS_IDX_MIN;
    }
    /* Fill next hop entry info. */
    return bcm_xgs3_nh_get(unit, (intf - offset), egr);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_find
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
bcm_xgs3_l3_egress_find(int unit, bcm_l3_egress_t *egr, 
                        bcm_if_t *intf)
{
    _bcm_l3_tbl_op_t data;         /* Operation data.      */
    bcm_l3_egress_t nh_info;       /* Next hop structure.  */
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if ((NULL == egr) || (NULL == intf)) {
        return (BCM_E_PARAM);
    }

    /* Convert api egress object to hw next hop entry format. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_egress_to_nh_info(unit, egr, &nh_info));

    /* Convert API next hop entry to HW space format. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_nh_map_api_data_to_hw(unit, &nh_info));


    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, next_hop);
    data.width = _BCM_SINGLE_WIDE;
    data.entry_buffer = (void *)&nh_info;
    data.hash_func = _bcm_xgs3_nh_hash_calc;
    data.cmp_func  = _bcm_xgs3_nh_ent_cmp;


    /* Find next hop entry. */
    BCM_IF_ERROR_RETURN (_bcm_xgs3_tbl_match(unit, &data)); 

    /* Return Egress object id offset + nh_index */
    /* Offset depends on whether it is a regular next hop or a WLAN DVP */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_wlan)) {
        egr_l3_next_hop_entry_t next_hop;
        int entry_type, nh_idx = data.entry_index;
        BCM_IF_ERROR_RETURN
            (READ_EGR_L3_NEXT_HOPm(unit, MEM_BLOCK_ANY, nh_idx, &next_hop));
        entry_type = soc_EGR_L3_NEXT_HOPm_field32_get(unit, &next_hop, 
                                                      ENTRY_TYPEf);
        if (entry_type == 4) {
            *intf = nh_idx + BCM_XGS3_DVP_EGRESS_IDX_MIN;
        } else {
            *intf = nh_idx + BCM_XGS3_EGRESS_IDX_MIN;
        }
    } else        
#endif
    {
        *intf = BCM_XGS3_EGRESS_IDX_MIN + data.entry_index; 
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_nh_traverse_cb
 * Purpose:
 *      Call user callback for valid next hop entries.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern (interface id & negate).
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_nh_traverse_cb(int unit, void *pattern, void *data1, 
                         void *data2, int *cmp_result)
{
    _bcm_l3_trvrs_data_t *trv_data;     /* Travers data.    */
    bcm_l3_egress_t  *egr;              /* Egress object.   */
    int nh_idx;                         /* Next hop index.  */

    /* Cast input parameters. */
    trv_data = (_bcm_l3_trvrs_data_t *) pattern;
    egr      = (bcm_l3_egress_t *) data1;
    nh_idx   = *(int *)data2;
    nh_idx  += BCM_XGS3_EGRESS_IDX_MIN; 

    /* Call user callback. */  
    if (NULL != trv_data->egress_cb) {
        (*trv_data->egress_cb) (unit, nh_idx, egr, trv_data->cookie);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_group_nh_delete
 * Purpose:
 *      Service routine used to decrement ecmp group member next hop 
 *      reference counter. 
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      ecmp_group   - (IN)Ecmp group member array.
 *      ecmp_count   - (IN)Number of entries in ecmp group erray.
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_ecmp_group_nh_delete(int unit, int *ecmp_group, int ecmp_count)
{
    int idx;         /* Iteration index. */

    /* Decrease reference count for all group member nexthops. */
    for (idx = 0; idx < ecmp_count; idx++) {
        bcm_xgs3_nh_del(unit, 0, ecmp_group[idx]);
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_ecmp_group_remove
 * Purpose:
 *      Service routine used to decrement ecmp group member next hop 
 *      reference counter & destroy ecmp group.
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      ecmp_idx     - (IN)Ecmp group start index.
 *      remove_nh    - (IN)Remove group's member next hops.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ecmp_group_remove(int unit, int ecmp_idx, int remove_nh)
{
    int ecmp_count;             /* Next hop count in the group.  */
    int *ecmp_grp;              /* Ecmp group from hw.           */
    int rv=BCM_E_NONE;                     /* Operation return status.      */

    if (remove_nh) {
        /* Allocate ecmp group buffer. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

        /* Read ecmp group members from the hw. */
        rv = _bcm_xgs3_ecmp_tbl_read(unit, ecmp_idx,
                                     ecmp_grp, &ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_grp);
            return (rv);
        }

        /* Decrease reference count for all group member nexthops. */
        rv = _bcm_xgs3_ecmp_group_nh_delete(unit, ecmp_grp, ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_grp);
            return (rv);
        }
        sal_free (ecmp_grp);
    }

    /* Decrement reference count of existing group */

	return bcm_xgs3_ecmp_group_del(unit, ecmp_idx);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_traverse
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
bcm_xgs3_l3_egress_traverse(int unit, bcm_l3_egress_traverse_cb trav_fn,
                            void *user_data)
{
    _bcm_l3_trvrs_data_t trv_data;     /* Travers initiator function input. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check l3 switching mode. */ 
    if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        return (BCM_E_DISABLED);
    }

    /* Input parameters check. */
    if (NULL == trav_fn) {
        return (BCM_E_PARAM);
    }

    /* Zero traverse info. */
    sal_memset(&trv_data, 0, sizeof(_bcm_l3_trvrs_data_t));

    /* Fill traverse data struct. */
    trv_data.op_cb = _bcm_xgs3_nh_traverse_cb;
    trv_data.egress_cb = trav_fn;
    trv_data.cookie = user_data;

    /*  Delete routes matching the pattern from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, nh_update_match)) {
        return BCM_XGS3_L3_HWCALL_EXEC(unit, nh_update_match) (unit, &trv_data);
    }
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      _bcm_xgs3_l3_egress_intf_validate
 * Purpose:
 *       Validate L3 egress objects array for multipath egress object
 *       create / destroy / lookup
 * Parameters:
 *      unit        - (IN) bcm device.
 *      intf_count  - (IN) Number of elements in intf_array.
 *      intf_array  - (IN) Array of Egress forwarding objects.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_egress_intf_validate(int unit, int intf_count,
                                  bcm_if_t *intf_array)
{
    _bcm_l3_tbl_t *tbl_ptr;      /* Unit next hop table pointer. */ 
    int idx;                     /* Iteration index.             */
    int nh_id;                   /* Next hop id.                 */

    /* Get next hop table pointer. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);

    /* Initialize ecmp group next hop indexes. */
    for (idx = 0; idx < intf_count; idx ++) {
        if (NULL == intf_array + idx) {
            return (BCM_E_PARAM);
        }

        /* Egress object range sanity. */
        if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf_array[idx]) && 
            !BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, intf_array[idx])) {
            return (BCM_E_PARAM);
        }

        /* Calculate next hop index. */
        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf_array[idx])) {
            nh_id = intf_array[idx] - BCM_XGS3_EGRESS_IDX_MIN;
        } else {
            nh_id = intf_array[idx] - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        }

        /* Validate that next hop is valid and in use */
        if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, nh_id)) {
            return (BCM_E_PARAM);
        }
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_l3_egress_intf_ref_count_update
 * Purpose:
 *      Update L3 egress object reference counts.
 * Parameters:
 *      unit        - (IN) bcm device.
 *      intf_count  - (IN) Number of elements in intf_array.
 *      intf_array  - (IN) Array of Egress forwarding objects.
 *      inc_dec     - (IN) +/-1 Increment/Decrement.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_egress_intf_ref_count_update(int unit, int intf_count,
                                          bcm_if_t *intf_array, int inc_dec)
{
    _bcm_l3_tbl_t *tbl_ptr;      /* Unit next hop table pointer. */ 
    int idx;                     /* Iteration index.             */
    int nh_id;                   /* Next hop id.                 */

    /* Get next hop table pointer. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);

    /* Initialize ecmp group next hop indexes. */
    for (idx = 0; idx < intf_count; idx ++) {
        /* Calculate next hop index. */
        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf_array[idx])) {
            nh_id = intf_array[idx] - BCM_XGS3_EGRESS_IDX_MIN;
        } else {
            nh_id = intf_array[idx] - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        }

        /* Increment next hops reference counter. */ 
        if (1 == inc_dec) {
            BCM_XGS3_L3_ENT_REF_CNT_INC(tbl_ptr, nh_id, _BCM_SINGLE_WIDE);
        } else {
            BCM_XGS3_L3_ENT_REF_CNT_DEC(tbl_ptr, nh_id, _BCM_SINGLE_WIDE);
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_egress_multipath_to_ecmp_grp
 * Purpose:
 *      Map an Egress Multipath forwarding object to hw ecmp group.
 * Parameters:
 *      unit        - (IN) bcm device.
 *      intf_count  - (IN) Number of elements in intf_array.
 *      intf_array  - (IN) Array of Egress forwarding objects.
 *      ref_cnt_inc - (IN) Increment next hops reference counters. 
 *      ecmp_group  - (OUT)Array of next hop indexes.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_egress_multipath_to_ecmp_grp(int unit, int intf_count,
                                          bcm_if_t *intf_array,
                                          int ref_cnt_inc, int *ecmp_group)
{
    int idx;                              /* Iteration index.             */

    /* Enforce maximum ecmp path's */
    if (intf_count > BCM_XGS3_L3_ECMP_MAX_PATHS(unit)) {
        return (BCM_E_RESOURCE);
    }


    /* Validate egress object interfaces array. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_l3_egress_intf_validate(unit, intf_count, intf_array));

    /* Increment each individual next hop reference count. */
    if (ref_cnt_inc) {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_l3_egress_intf_ref_count_update(unit, intf_count,
                                                       intf_array, 1));
    }

    /* Initialize ecmp group next hop indexes. */
    for (idx = 0; idx < intf_count; idx ++) {
        /* Calculate next hop index. */
        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf_array[idx])) {
             ecmp_group[idx] = intf_array[idx] - BCM_XGS3_EGRESS_IDX_MIN;
        } else if (BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, intf_array[idx])){
             ecmp_group[idx] = intf_array[idx] - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_egress_multipath_read
 * Purpose:
 *      Read an Egress Multipath forwarding object from hw.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      grp_idx    - (IN) Ecmp group index.
 *      intf_size  - (IN) Maximum forwarding entries to read.
 *      intf_array - (OUT) Array of Egress forwarding objects.
 *      intf_count - (OUT) Number of entries of intf_count actually filled in.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_xgs3_l3_egress_multipath_read(int unit, int grp_idx, int intf_size,
                                    bcm_if_t *intf_array, int *intf_count)
{
    int *ecmp_grp;                      /* Ecmp group from hw.            */
    int idx;                            /* Iteration index.               */
    int rv;                             /* Operation return status.       */

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

    /* Read ecmp group from hw. */
    rv = _bcm_xgs3_ecmp_tbl_read(unit, grp_idx,
                                 ecmp_grp, intf_count);
    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_grp);
        return (rv);
    }

    if (intf_size == 0) {
        sal_free(ecmp_grp);
        return  (BCM_E_NONE);
    }

    /* Adjust number of read entries . */
    if (*intf_count > intf_size) {
        *intf_count = intf_size; 
    }

    /* Fill ecmp group info. */
    for (idx = 0; idx < *intf_count; idx++) { 
        if (NULL == intf_array + idx) {
            sal_free(ecmp_grp);
            return (BCM_E_PARAM);
        }
        intf_array[idx] = ecmp_grp[idx] + BCM_XGS3_EGRESS_IDX_MIN;
    }
    sal_free(ecmp_grp);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_max_create 
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
bcm_xgs3_l3_egress_multipath_max_create(int unit, uint32 flags, int max_paths, 
                                           int intf_count, bcm_if_t *intf_array, bcm_if_t *mpintf)
{
    int *ecmp_group;                     /* Next hop indexes array.      */
    int *old_ecmp_group;                 /* Original nh indexes array.   */
    int old_group_size;                  /* Original ecmp group size.    */
    int ecmp_group_idx;                  /* Ecmp group index.            */
    uint32  shr_flags;                   /* Table management flags.      */
    int     rv=0;                          /* Operation return status.     */
    int    max_grp_paths = 0;    /* Configured Max ECMP Group paths */

    /* Make sure unit supports multipath egress mode. */ 
    BCM_IF_ERROR_RETURN(BCM_XGS3_L3_MPATH_EGRESS_UNSUPPORTED(unit));

    /* Input parameters check. */
    if (NULL == mpintf) {
        return (BCM_E_PARAM);
    }

    /* Make sure device supports ecmp forwarding. */
    if (0 == BCM_XGS3_L3_ECMP_MAX_PATHS(unit)) {
        return (BCM_E_UNAVAIL); 
    }

    /* Check for max_paths range */
    if (max_paths == 0) {
        max_paths = BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
    } else if ((max_paths < 2) || (max_paths > 1024)) {
        return (BCM_E_PARAM);
    }

    old_group_size = 0;
    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_group));
    rv = _bcm_xgs3_l3_ecmp_group_alloc(unit, &old_ecmp_group);
    if (BCM_FAILURE(rv)) {
        sal_free (ecmp_group);
        return rv;
    }
        

    /* Verify egress object range if BCM_L3_WITH_ID flag is set. */
    if (flags & BCM_L3_WITH_ID) {
        /* Get ecmp group index. */
        ecmp_group_idx = *mpintf - BCM_XGS3_MPATH_EGRESS_IDX_MIN;
        rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_idx, &max_grp_paths);
        if (rv < 0) {
            sal_free(ecmp_group);
            sal_free(old_ecmp_group);
            return rv;
        }

        if (!BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, *mpintf)) {
            sal_free(ecmp_group);
            sal_free(old_ecmp_group);
            return (BCM_E_PARAM);
        }

        /* Check if operation updates existing multipath object. */
        if (BCM_XGS3_L3_ENT_REF_CNT(BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp),
                                    ecmp_group_idx)) {
            rv = _bcm_xgs3_l3_egress_multipath_read(unit, ecmp_group_idx, 
                                                 max_grp_paths,
                                                 old_ecmp_group, 
                                                 &old_group_size);

        if (BCM_FAILURE(rv)) {
                sal_free(ecmp_group);
                sal_free(old_ecmp_group);
                return (rv );
            }
        }
    }

    /* Convert api flags to shared table management flags. */ 
    rv = _bcm_xgs3_l3_flags_to_shr(flags, &shr_flags);
    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_group);
        sal_free(old_ecmp_group);
        return (rv );
    }

    /* Disable matching entry lookup. */
    shr_flags |= _BCM_L3_SHR_MATCH_DISABLE;

    /* Convert egress multipath object to ecmp group. */
    rv = _bcm_xgs3_l3_egress_multipath_to_ecmp_grp(unit, intf_count, intf_array,
                                                   TRUE, ecmp_group);

    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_group);
        sal_free(old_ecmp_group);
        return (rv );
    }

    /* Create ecmp group. */
    rv  = _bcm_xgs3_ecmp_group_add(unit, shr_flags, intf_count, max_paths, 
                                   ecmp_group, &ecmp_group_idx);
    if (BCM_FAILURE(rv)) {
        /* Decrement nh reference count - if ecmp group creation failed. */
        _bcm_xgs3_ecmp_group_nh_delete(unit, ecmp_group, intf_count);
        sal_free(ecmp_group);
        sal_free(old_ecmp_group);
        return (rv);
    } else if (old_group_size) {
        /* Decrement nh reference count - for old ecmp group members. */
        rv = _bcm_xgs3_l3_egress_intf_ref_count_update(unit, old_group_size,
                                                       old_ecmp_group, -1);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_group);
            sal_free(old_ecmp_group);
            return (rv);
        }
    }

    sal_free(ecmp_group);
    sal_free(old_ecmp_group);

    /* Return Egress object id 100K + nh_index */
    *mpintf = BCM_XGS3_MPATH_EGRESS_IDX_MIN + ecmp_group_idx;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_create 
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
bcm_xgs3_l3_egress_multipath_create(int unit, uint32 flags, int intf_count,
                                    bcm_if_t *intf_array, bcm_if_t *mpintf)
{
    int     rv = BCM_E_UNAVAIL;                 /* Operation return status.     */

    rv = bcm_xgs3_l3_egress_multipath_max_create(unit, flags, 
                                            BCM_XGS3_L3_ECMP_MAX_PATHS(unit),
                                            intf_count, intf_array, mpintf);
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_get
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
bcm_xgs3_l3_egress_multipath_get(int unit, bcm_if_t mpintf, int intf_size,
                                 bcm_if_t *intf_array, int *intf_count)
{
    int grp_idx;                        /* Ecmp group index.              */

    /* Make sure unit supports multipath egress mode. */ 
    BCM_IF_ERROR_RETURN(BCM_XGS3_L3_MPATH_EGRESS_UNSUPPORTED(unit));

    /* Input parameters sanity check. */
    if ((NULL == intf_count) || ((NULL == intf_array) && (0 != intf_size))) {
        return (BCM_E_PARAM);
    }
    if (!BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, mpintf)) {
        return (BCM_E_PARAM);
    }

    /* Get ecmp group index. */
    grp_idx = mpintf - BCM_XGS3_MPATH_EGRESS_IDX_MIN; 

    /* Read ecmp group from hardware. */
    return _bcm_xgs3_l3_egress_multipath_read(unit, grp_idx, intf_size,
                                              intf_array, intf_count);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_destroy
 * Purpose:
 *      Destroy an Egress Multipath forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) L3 interface id pointing to Egress multipath object.
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_xgs3_l3_egress_multipath_destroy(int unit, bcm_if_t mpintf) 
{
    int ecmp_group_id;        /* Ecmp group id.             */ 
    _bcm_l3_tbl_t *tbl_ptr;   /* Unit ecmp table pointer.   */ 

    /* Make sure unit supports multipath egress mode. */ 
    BCM_IF_ERROR_RETURN(BCM_XGS3_L3_MPATH_EGRESS_UNSUPPORTED(unit));
    if (!BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, mpintf)) {
        return (BCM_E_PARAM);
    }

    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);

    /* Calculate ecmp group id. */
    ecmp_group_id = mpintf - BCM_XGS3_MPATH_EGRESS_IDX_MIN;

    /* If entry is used by routes - reject deletion */
    if(1 < BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, ecmp_group_id)) {
         return (BCM_E_BUSY);
    }

    /* Delete ecmp group. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_ecmp_group_remove(unit, ecmp_group_id, TRUE));

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_add
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
bcm_xgs3_l3_egress_multipath_add(int unit, bcm_if_t mpintf, bcm_if_t intf)
{
    int ecmp_grp_sz;                    /* Ecmp group size.        */
    int *ecmp_grp;                      /* Ecmp group from hw.     */
    int rv;                             /* Operation return status.*/
    int ecmp_group_idx;                  /* Ecmp group index.            */
    int max_grp_paths = 0;    /* Configured Max ECMP Group paths */

#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_IS_HURRICANE(unit)) {
        return (BCM_E_UNAVAIL); 	
    }
#endif	
    if (!BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, mpintf)) {
        return (BCM_E_PARAM);
    }
    /* Validate added interface. */
    if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf)) {
        return (BCM_E_PARAM);
    }

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));
    ecmp_group_idx = mpintf - BCM_XGS3_MPATH_EGRESS_IDX_MIN;
    rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_idx, &max_grp_paths);
    if (rv < 0) {
        sal_free (ecmp_grp);
        return rv;
    }

    /* Get multipath egress object from hw. */
    rv = bcm_xgs3_l3_egress_multipath_get (unit, mpintf, 
                                           max_grp_paths,
                                           ecmp_grp, &ecmp_grp_sz);
    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_grp);
        return (rv);
    }


    /* Check if there is room in ecmp grp buffer */ 
    if (ecmp_grp_sz == BCM_XGS3_L3_ECMP_MAX(unit) - 1) {
        sal_free(ecmp_grp);
        return (BCM_E_FULL);
    }

    ecmp_grp[ecmp_grp_sz] = intf;

    /* Update multipath egress object. */
    rv = bcm_xgs3_l3_egress_multipath_create(unit,
                                             BCM_L3_REPLACE | BCM_L3_WITH_ID,
                                             ecmp_grp_sz + 1, ecmp_grp,
                                             &mpintf);
    sal_free(ecmp_grp);
    return (rv);
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_delete
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
bcm_xgs3_l3_egress_multipath_delete(int unit, bcm_if_t mpintf, bcm_if_t intf)
{
    int ecmp_grp_sz;                    /* Ecmp group size.        */
    int *ecmp_grp;                      /* Ecmp group from hw.     */
    int idx;                            /* Iteration index.        */
    int rv;                             /* Operation return status.*/
    int ecmp_group_idx;                  /* Ecmp group index.            */
    int max_grp_paths = 0;    /* Configured Max ECMP Group paths */

#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_IS_HURRICANE(unit)) {
        return (BCM_E_UNAVAIL);
    }

#endif
    if (!BCM_XGS3_L3_MPATH_EGRESS_IDX_VALID(unit, mpintf)) {
        return (BCM_E_PARAM);
    }
    /* Validate deleted interface. */
    if (!BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf)) {
        return (BCM_E_PARAM);
    }

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));
    ecmp_group_idx = mpintf - BCM_XGS3_MPATH_EGRESS_IDX_MIN;
    rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_idx, &max_grp_paths);
    if (rv < 0) {
        sal_free(ecmp_grp);
        return rv;
    }

    /* Get multipath egress object from hw. */
    rv = bcm_xgs3_l3_egress_multipath_get(unit, mpintf,
                                          max_grp_paths,
                                          ecmp_grp, &ecmp_grp_sz);
    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_grp);
        return (rv);
    }

    /* Remove interface from egress object. */
    for (idx = 0; idx < ecmp_grp_sz; idx++) {
        if (ecmp_grp[idx] == intf) {
            ecmp_grp[idx] = 0; 
            break;
        }
    }

    /* Check if deleted interface was found. */
    if (idx == ecmp_grp_sz) {
        sal_free(ecmp_grp);
        return (BCM_E_NOT_FOUND);
    } 

    /* Copy last object interface to the deleted one spot. */ 
    if (idx != (--ecmp_grp_sz)) {
        ecmp_grp[idx] = ecmp_grp[ecmp_grp_sz];
    }

    /* Update multipath egress object. */
    rv = bcm_xgs3_l3_egress_multipath_create(unit, 
                                             BCM_L3_REPLACE | BCM_L3_WITH_ID,
                                             ecmp_grp_sz, ecmp_grp,
                                             &mpintf);
    sal_free(ecmp_grp);
    return (rv); 
}

/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_find
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
bcm_xgs3_l3_egress_multipath_find(int unit, int intf_count, bcm_if_t
                                 *intf_array, bcm_if_t *mpintf)
{
    _bcm_l3_tbl_op_t data;         /* Operation data.          */
    int *ecmp_group;               /* Next hop indexes array.  */
    int rv;                        /* Operation return status. */

    /* Make sure unit supports multipath egress mode. */ 
    BCM_IF_ERROR_RETURN(BCM_XGS3_L3_MPATH_EGRESS_UNSUPPORTED(unit));

    /* Input parameters check. */
    if (NULL == mpintf) {
        return (BCM_E_PARAM);
    }

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_group));

    /* Convert egress multipath object to ecmp group. */
    rv = _bcm_xgs3_l3_egress_multipath_to_ecmp_grp(unit, intf_count, intf_array,
                                                   FALSE, ecmp_group);
    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_group);
        return (rv);
    }

    /*
     * Next hop indexes must be sorted in the group 
     * in order to produce identical hash every time. 
     */
    _shr_sort(ecmp_group, intf_count, sizeof(int), _bcm_xgs3_cmp_int);
 
    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);
    data.width = _BCM_SINGLE_WIDE;
    data.entry_buffer = (void *)ecmp_group;
    data.hash_func = _bcm_xgs3_ecmp_grp_hash_calc;
    data.cmp_func  = _bcm_xgs3_ecmp_grp_cmp;

    /* Find next hop entry. */
    rv = _bcm_xgs3_tbl_match(unit, &data);
    sal_free(ecmp_group);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    /* Return Egress object id 100K + nh_index */
    *mpintf = BCM_XGS3_MPATH_EGRESS_IDX_MIN + data.entry_index;
    return (BCM_E_NONE);
} 
/*
 * Function:
 *      bcm_xgs3_l3_egress_multipath_traverse
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
bcm_xgs3_l3_egress_multipath_traverse(int unit, 
                          bcm_l3_egress_multipath_traverse_cb trav_fn,
                          void *user_data)
{
    bcm_if_t *intf_array;                      /* Ecmp group from hw.      */
    _bcm_l3_tbl_t  *tbl_ptr;                   /* Unit ecmp table pointer. */
    int intf_count;                            /* Egress interfaces count. */
    int idx;                                   /* Iteration index.         */
    int rv;                                    /* Operation return status. */
    int max_grp_paths = 0;    /* Configured Max ECMP Group paths */

    /* Make sure unit supports multipath egress mode. */ 
    BCM_IF_ERROR_RETURN(BCM_XGS3_L3_MPATH_EGRESS_UNSUPPORTED(unit));

    /* Input parameters check. */
    if (NULL == trav_fn) {
        return (BCM_E_PARAM);
    }

    /* Check if unit supports ecmp. */
    if (!BCM_XGS3_L3_ECMP_MAX_PATHS(unit)) {
        return (BCM_E_NONE);
    }

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &intf_array));

    tbl_ptr  = BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);
    for (idx = tbl_ptr->idx_min; idx <= tbl_ptr->idx_maxused; idx++) {
         rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, idx, &max_grp_paths);
         if (rv < 0) {
              sal_free(intf_array);
              return rv;
         }

         /* If group is not in use continue to the next one. */
         if (!BCM_XGS3_L3_ENT_REF_CNT (tbl_ptr, idx)) {
              continue;
         }

         /* Read group nexthops. */
         rv = _bcm_xgs3_l3_egress_multipath_read(unit, idx, 
                                        max_grp_paths,
                                        intf_array, &intf_count);
         if (BCM_FAILURE(rv)) {
               sal_free(intf_array);
              return (rv);
         }
         rv = (*trav_fn)(unit, (idx + BCM_XGS3_MPATH_EGRESS_IDX_MIN), intf_count,
                         intf_array, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
         if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
             sal_free(intf_array);
             return (rv);
         }
#endif
    }
    sal_free(intf_array);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_host_to_route
 * Purpose:
 *      Service routine used to copy host data in a route format
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      l3cfg   - (IN) Host information.
 *      lpm_cfg - (OUT) Corresponding route info. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
bcm_xgs3_host_to_route(int unit, _bcm_l3_cfg_t *l3cfg, 
                       _bcm_defip_cfg_t *lpm_cfg)
{
    /* Input parameters check. */
    if ((NULL == l3cfg) || (NULL == lpm_cfg)) {
        return (BCM_E_PARAM);
    }
    /* Reset destination buffer. */
    sal_memset(lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));

    /* Prepare route entry. */
    lpm_cfg->defip_flags = (l3cfg->l3c_flags | BCM_L3_HOST_AS_ROUTE);
    lpm_cfg->defip_vrf = l3cfg->l3c_vrf;

    if (l3cfg->l3c_flags & BCM_L3_IP6)  {
        sal_memcpy(lpm_cfg->defip_ip6_addr, l3cfg->l3c_ip6, sizeof(bcm_ip6_t));
        lpm_cfg->defip_sub_len = BCM_XGS3_L3_IPV6_PREFIX_LEN;
    } else {
        lpm_cfg->defip_ip_addr =  l3cfg->l3c_ip_addr;
        lpm_cfg->defip_sub_len = BCM_XGS3_L3_IPV4_PREFIX_LEN;
    }
    sal_memcpy(lpm_cfg->defip_mac_addr, l3cfg->l3c_mac_addr, 
               sizeof(bcm_mac_t));
    lpm_cfg->defip_intf = l3cfg->l3c_intf;
    lpm_cfg->defip_port_tgid = l3cfg->l3c_port_tgid;
    lpm_cfg->defip_modid = l3cfg->l3c_modid;
    lpm_cfg->defip_vid = l3cfg->l3c_vid;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_route_to_host
 * Purpose:
 *      Service routine used to copy route data in a host format
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN) Route info. 
 *      l3cfg   - (OUT) Host information.
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
bcm_xgs3_route_to_host(int unit, _bcm_defip_cfg_t *lpm_cfg, 
                       _bcm_l3_cfg_t *l3cfg)
{
    /* Input parameters check. */
    if ((NULL == l3cfg) || (NULL == lpm_cfg)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination buffer. */
    sal_memset(l3cfg, 0, sizeof(_bcm_l3_cfg_t));

    /* Prepare host entry. */
    if (lpm_cfg->defip_flags & BCM_L3_IP6)  {
        if (lpm_cfg->defip_sub_len != BCM_XGS3_L3_IPV6_PREFIX_LEN) {
            return (BCM_E_NOT_FOUND);
        }
        sal_memcpy(l3cfg->l3c_ip6, lpm_cfg->defip_ip6_addr, sizeof(bcm_ip6_t));
    } else {
        if (lpm_cfg->defip_sub_len != BCM_XGS3_L3_IPV4_PREFIX_LEN) {
            return (BCM_E_NOT_FOUND);
        }
        l3cfg->l3c_ip_addr = lpm_cfg->defip_ip_addr;
    }

    l3cfg->l3c_flags = (lpm_cfg->defip_flags | BCM_L3_HOST_AS_ROUTE);
    l3cfg->l3c_vrf = lpm_cfg->defip_vrf;

    sal_memcpy(l3cfg->l3c_mac_addr, lpm_cfg->defip_mac_addr, 
               sizeof(bcm_mac_t));
    l3cfg->l3c_intf = lpm_cfg->defip_intf;
    l3cfg->l3c_port_tgid = lpm_cfg->defip_port_tgid;
    l3cfg->l3c_modid = lpm_cfg->defip_modid;
    l3cfg->l3c_vid = lpm_cfg->defip_vid;
    return (BCM_E_NONE);
}



/*
 * Function:
 *      bcm_xgs3_host_as_route
 * Purpose:
 *      Service routine used to operate host entry as route.
 *      Host inserted as route in case host table is full. 
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      l3cfg     - (IN) Deleted host information.
 *      operation - (IN) Host table operation.  
 *      orig_rv     (IN) Host as route is not supported on 
 *                       some devices/configurations. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
bcm_xgs3_host_as_route(int unit, _bcm_l3_cfg_t *l3cfg,
                       int operation, int orig_rv)
{
    _bcm_defip_cfg_t lpm_cfg; /* Route info.             */
    int retval;               /* Operation return value. */                 

    /* Input parameters check. */
    if (NULL == l3cfg) {
        return (BCM_E_PARAM);
    }

    /* IPv6 not supported. */
    if (l3cfg->l3c_flags & BCM_L3_IP6) {
        return (orig_rv);
    }

    BCM_IF_ERROR_RETURN(bcm_xgs3_host_to_route(unit, l3cfg, &lpm_cfg));

    switch (operation){
      case BCM_XGS3_L3_OP_GET:
         retval = bcm_xgs3_defip_get(unit, &lpm_cfg);
         if (BCM_SUCCESS(retval)) {
            BCM_IF_ERROR_RETURN(bcm_xgs3_route_to_host(unit, &lpm_cfg, l3cfg));
         }
         break;
      case BCM_XGS3_L3_OP_ADD:
         retval = bcm_xgs3_defip_add(unit, &lpm_cfg);
         break;
      case BCM_XGS3_L3_OP_DEL:
         retval = bcm_xgs3_defip_del(unit, &lpm_cfg);
         break;
      default:
         retval = BCM_E_INTERNAL;
    }
    return (retval);
}


/*
 * Function:
 *      bcm_xgs3_l3_get
 * Purpose:
 *      Get an entry from L3 table.
 * Parameters:
 *      unit  - (IN)SOC unit number.
 *      l3cfg - (IN/OUT)Structure to fill l3 entry information. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_get(int unit, _bcm_l3_cfg_t *l3cfg)
{
    int nh_idx;                 /* Next hop index.          */
    int rv = BCM_E_UNAVAIL;     /* Operation return status. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == l3cfg) {
        return (BCM_E_PARAM);
    }
    BCM_ER_VRF_INVALID_RETURN(unit, l3cfg->l3c_flags & BCM_L3_IP6, 
                              l3cfg->l3c_vrf);

    /* Check if address is multicast. */
    if (BCM_XGS3_L3_MCAST_ENTRY(l3cfg)) {
        /* Read multicast entry. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_get) (unit, l3cfg);
        }
    } else {
        /* Unicast handling. */
        /* Read l3 host entry. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get) (unit, l3cfg, &nh_idx);
        }

        if (BCM_SUCCESS(rv)) {
            BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_get_nh_info(unit, l3cfg, nh_idx));
        } else if ((BCM_E_NOT_FOUND == rv) || (BCM_E_DISABLED == rv)) {
            rv = bcm_xgs3_host_as_route(unit, l3cfg, BCM_XGS3_L3_OP_GET, rv);
            if (BCM_SUCCESS(rv)) {
                /* Special return value - as entry retrieved from ROUTE table */
                (void) bcm_xgs3_l3_host_as_route_return_get(unit, &rv);
            }
        }
    }
    return rv;
}
/*
 * Function:
 *      bcm_xgs3_l3_add
 * Purpose:
 *      Add an entry in L3 table.
 * Parameters:
 *      unit  - (IN)SOC unit number.
 *      l3cfg - (IN)Structure with l3 entry information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_add(int unit, _bcm_l3_cfg_t *l3cfg)
{
    int nh_idx;                 /* Next hop index.        */
    int rv = BCM_E_UNAVAIL;     /* Operation return value */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == l3cfg) {
        return (BCM_E_PARAM);
    }
    BCM_ER_VRF_INVALID_RETURN(unit, l3cfg->l3c_flags & BCM_L3_IP6, 
                              l3cfg->l3c_vrf);

    if (BCM_XGS3_L3_MCAST_ENTRY(l3cfg)) {
        /* Check if identical entry exits. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_get) (unit, l3cfg);
        }
        BCM_XGS3_IF_ERROR_OR_ENTRY_EXISTS_RETURN(rv);

        /* Write mcast entry to hw. */
        l3cfg->l3c_hw_index = BCM_XGS3_L3_INVALID_INDEX;/* Add /No overrite. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_add)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_add) (unit, l3cfg);
        } else {
            return (BCM_E_UNAVAIL);
        }
    } else {
        if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
            /* Trunk id validation. */
            BCM_XGS3_L3_IF_INVALID_TGID_RETURN(unit, l3cfg->l3c_flags,
                                               l3cfg->l3c_port_tgid);
        }

        /* Check if identical entry exits. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get) (unit, l3cfg, NULL);
        }
        BCM_XGS3_IF_ERROR_OR_ENTRY_EXISTS_RETURN(rv);

        if (BCM_GPORT_IS_BLACK_HOLE(l3cfg->l3c_port_tgid)) {
             nh_idx = 0;
        } else {
             /* Get next hop index. */
             BCM_IF_ERROR_RETURN(_bcm_xgs3_nh_init_add(unit, l3cfg, NULL, &nh_idx));
        }
        /* Write entry to hw. */
        l3cfg->l3c_hw_index = BCM_XGS3_L3_INVALID_INDEX;        /* Add /No overrite. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_add)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_add) (unit, l3cfg, nh_idx);
        } else {
            rv = BCM_E_UNAVAIL;
        }

        /* Do best effort clean_up if needed. */
        if (BCM_FAILURE(rv)) {
            bcm_xgs3_nh_del(unit, 0, nh_idx);
        }
        /* If l3 table full or disabled customer allowed to write */
        /* the entry to lpm table.                                */
        if (((BCM_E_FULL == rv) || (BCM_E_DISABLED == rv)) && 
            (l3cfg->l3c_flags & BCM_L3_HOST_AS_ROUTE)) {
            rv = bcm_xgs3_host_as_route(unit, l3cfg, BCM_XGS3_L3_OP_ADD, rv);
            if (BCM_SUCCESS(rv)) {
                /* Special return value - as entry added to ROUTE table */
                (void) bcm_xgs3_l3_host_as_route_return_get(unit, &rv);
            }
        }
    }

    return rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_del
 * Purpose:
 *      Delete an entry in L3 table.
 * Parameters:
 *      unit - SOC unit number.
 *      l3cfg - Pointer to memory for l3 table related information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_del(int unit, _bcm_l3_cfg_t *l3cfg)
{
    int nh_idx;                 /* Next hop index.          */
    int rv = BCM_E_UNAVAIL;     /* Operation return status. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == l3cfg) {
        return (BCM_E_PARAM);
    }
    BCM_ER_VRF_INVALID_RETURN(unit, l3cfg->l3c_flags & BCM_L3_IP6, 
                              l3cfg->l3c_vrf);

    if (BCM_XGS3_L3_MCAST_ENTRY(l3cfg)) {
        /* Delete entry from  hw. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_del)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_del) (unit, l3cfg);
        } 
        BCM_IF_ERROR_RETURN(rv);
    } else {
        /* Read l3 host entry to get next hop index. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get) (unit, l3cfg, &nh_idx);
        }
        if ((BCM_E_NOT_FOUND == rv) || (BCM_E_DISABLED == rv)) {
            rv =  bcm_xgs3_host_as_route(unit, l3cfg, BCM_XGS3_L3_OP_DEL, rv);
            if (BCM_SUCCESS(rv)) {
                /* Special return value - as entry deleted from ROUTE table */
                (void) bcm_xgs3_l3_host_as_route_return_get(unit, &rv);                
            }
            return rv;
        }
        BCM_IF_ERROR_RETURN(rv);

        /* Delete entry from  hw. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_del)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_del) (unit, l3cfg);
        } else {
            return (BCM_E_UNAVAIL);
        }

        /* Release next hop index. */
        if(BCM_SUCCESS(rv)) {
            BCM_IF_ERROR_RETURN(bcm_xgs3_nh_del(unit, 0, nh_idx));
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_l3_nh_intf_cmp
 * Purpose:
 *      Compare interface of next hop with a given interface index. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      ifindex   - (IN)Interface index.
 *      nh_idx    - (IN)Next hop index.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_nh_intf_cmp(int unit, int ifindex, int nh_idx, int *cmp_result)
{
    bcm_l3_egress_t nh_info;  /* Next hop info buffer.    */

    /* If next hop is trap to cpu - don't read nh table. */
    if (nh_idx == BCM_XGS3_L3_L2CPU_NH_IDX) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    /* Get next hop entry from hw. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_nh_get(unit, nh_idx, &nh_info));

    if (ifindex != nh_info.intf) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
    } else {
        *cmp_result = BCM_L3_CMP_EQUAL;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_prefix_cmp
 * Purpose:
 *      Compare prefix & vrf of l3 entry.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern.
 *      data1     - (IN)L3 entry. 
 *      data2     - UNUSED.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_l3_prefix_cmp(int unit, void *pattern,
                        void *data1, void *data2, int *cmp_result)
{
    int idx;                    /* Iteration index.              */
    int ipv6;                   /* ipv6 entry flag.              */

    _bcm_l3_cfg_t *l3_entry = (_bcm_l3_cfg_t *) data1;
    _bcm_l3_cfg_t *l3cfg = (_bcm_l3_cfg_t *) pattern;

    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    if (l3_entry->l3c_vrf != l3cfg->l3c_vrf) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    if (ipv6) {
        /* Compare ipv6 ip address. */
        for (idx = 0; idx < BCM_IP6_ADDRLEN; idx++) {
            if ((l3_entry->l3c_ip6[idx] & l3cfg->l3c_ip6_mask[idx]) !=
                (l3cfg->l3c_ip6[idx] & l3cfg->l3c_ip6_mask[idx])) {
                *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                return (BCM_E_NONE);
            }
        }
    } else {
        /* Compare ipv4 ip address. */
        if ((l3_entry->l3c_ip_addr & l3cfg->l3c_ip_mask) !=
            (l3cfg->l3c_ip_addr & l3cfg->l3c_ip_addr)) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            return (BCM_E_NONE);
        }
    }
    *cmp_result = BCM_L3_CMP_EQUAL;
    return (BCM_E_NONE);

}

/*
 * Function:
 *      _bcm_xgs3_l3_intf_cmp
 * Purpose:
 *      Compare l3 entry interface with a given interface id.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern (interface id & negate).
 *      data1     - UNUSED. 
 *      data2     - (IN)Next hop index.
 *      cmp_result- (OUT)Comparison result.
 *      
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_l3_intf_cmp(int unit, void *pattern, void *data1,
                      void *data2, int *cmp_result)
{
    _bcm_if_del_pattern_t *trv_data;    /* Interface & negate combination. */
    int if_cmp_result;                  /* Interface comparison return.   */
    int nh_idx = *(int *)data2;         /* Next hop index.                */

    trv_data = (_bcm_if_del_pattern_t *) pattern;

    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_nh_intf_cmp(unit, trv_data->l3_intf,
                                                 nh_idx, &if_cmp_result));
    /* Calculate final result based on negate and interface index comparison. */
    if (BCM_XGS3_L3_IS_EQUAL(if_cmp_result, trv_data->negate)) {
        *cmp_result = BCM_L3_CMP_EQUAL;
    } else {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_cmp_hit
 * Purpose:
 *      Check l3 entry hit bit. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern (l3cfg hit flag)
 *      data1     - (IN)L3 entry. 
 *      data2     - UNUSED.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX 
 */
STATIC INLINE int
_bcm_xgs3_l3_cmp_hit(int unit, void *pattern, void *data1,
                     void *data2, int *cmp_result)
{
    uint32 flags = *(uint32 *)pattern;
    _bcm_l3_cfg_t *l3cfg = (_bcm_l3_cfg_t *) data1;

    /* If compare function provided match the entry against pattern */
    if ((flags & BCM_L3_HIT_CLEAR) && (l3cfg->l3c_flags & BCM_L3_HIT)) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    /* Unused entry -> matches "not hit" pattern. */
    *cmp_result = BCM_L3_CMP_EQUAL;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_host_ent_init 
 * Purpose:
 *      Fill host entry for callbacks.  
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3cfg    -  (IN)L3 entry info.
 *      nh_valid -  (IN)Flag indicates passed next hop info is valid.
 *      host_info - (OUT)Buffer to be filled.
 * Returns:
 *      void
 */
void
_bcm_xgs3_host_ent_init(int unit, _bcm_l3_cfg_t *l3cfg, int nh_valid,
                        bcm_l3_host_t *host_info)
{

    /* Input parameters sanity. */
    if ((NULL == host_info) || (NULL == l3cfg)) {
        return;
    }

    /* Init host entry. */
    bcm_l3_host_t_init(host_info);

    /* Set vrf. */
    host_info->l3a_vrf = l3cfg->l3c_vrf;

    /* Set ip address. */
    if (l3cfg->l3c_flags & BCM_L3_IP6) {
        sal_memcpy(host_info->l3a_ip6_addr, l3cfg->l3c_ip6, sizeof(bcm_ip6_t));
    } else {
        host_info->l3a_ip_addr = l3cfg->l3c_ip_addr;
    }
    /* Set flags. */
    host_info->l3a_flags = l3cfg->l3c_flags;

    /* Set classification group id. */
    host_info->l3a_lookup_class = l3cfg->l3c_lookup_class;

    if (nh_valid) {
        /* Set mac address. */
        sal_memcpy(host_info->l3a_nexthop_mac, l3cfg->l3c_mac_addr,
                   sizeof(bcm_mac_t));
        /* Set module id. */
        host_info->l3a_modid = l3cfg->l3c_modid;
        /* Set port/trunk id. */
        host_info->l3a_port_tgid = l3cfg->l3c_port_tgid;
        /* Set interface index. */
        host_info->l3a_intf = l3cfg->l3c_intf;
    }
}

/*
 * Function:
 *      _bcm_xgs3_trvrs_flags_cmp
 * Purpose:
 *      Service routine used to compare l3 entry flags with 
 *      travers input parameters and update iteration index accordingly.  
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      req_flags - (IN)Travers request flags. 
 *      ent_flags - (IN)Entry flags.
 *      idx       - (OUT)Traverse index.
 * Returns:
 *      BCM_L3_CMP_EQUAL if flags match 
 *      BCM_L3_CMP_NOT_EQUAL otherwise.
 */
int
_bcm_xgs3_trvrs_flags_cmp(int unit, int req_flags, int ent_flags, int *idx)
{
    /* If protocol doesn't match skip the entry. */
    if (req_flags & BCM_L3_IP6) {
        if (!(ent_flags & BCM_L3_IP6)) {
            return (BCM_L3_CMP_NOT_EQUAL);
        }
    } else if (ent_flags & BCM_L3_IP6) {
#ifdef BCM_FIREBOLT_SUPPORT
        BCM_XGS3_INC_IF_FIREBOLT(unit, (*idx));
#endif /* BCM_FIREBOLT_SUPPORT */
        return (BCM_L3_CMP_NOT_EQUAL);
    }

    /* Check ipmc flag. */
    if ((!(req_flags & BCM_L3_IPMC)) && (ent_flags & BCM_L3_IPMC)) {
#ifdef BCM_FIREBOLT_SUPPORT
        if (req_flags & BCM_L3_IP6) {
            BCM_XGS3_INC_IF_FIREBOLT(unit, (*idx));
        }
#endif /* BCM_FIREBOLT_SUPPORT */
        return (BCM_L3_CMP_NOT_EQUAL);
    }
    return (BCM_L3_CMP_EQUAL);
}

/*
 * Function:
 *      _bcm_xgs3_l3_del_match
 * Purpose:
 *      Delete an entry in L3 table matches a certain rule.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      flags.    - (IN)Generic l3 flags. 
 *      pattern   - (IN)Comparison match argurment.
 *      cmp_func  - (IN)Comparison function.
 *      notify_cb - (IN)Delete notification callback. 
 *      user_data - (IN)User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_del_match(int unit, int flags, void *pattern,
                       bcm_xgs3_ent_op_cb cmp_func,
                       bcm_l3_host_traverse_cb notify_cb, void *user_data)
{
    _bcm_l3_cfg_t l3cfg;        /* Hw entry info.                  */
    bcm_l3_host_t info;         /* Host cb buffer.                 */
    int cmp_result;             /* Compare against pattern result. */
    soc_mem_t mem;              /* Table memory.                   */
    int idx_max;                /* Maximum iteration index.        */
    int idx_min;                /* Minimum iteration index.        */
    int nh_idx;                 /* Next hop index.                 */
    int ipv6;                   /* IPv6/IPv4 lookup flag.          */
    int idx;                    /* Iteration index.                */
    int rv;                     /* Operation return value.         */


    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get_by_idx)) {
        return (BCM_E_UNAVAIL);
    }
    /* Init iteration parameters. */

    /* Check Protocol. */
    ipv6 = (flags & BCM_L3_IP6) ? TRUE : FALSE;

    /* Get table boundaries. */
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        /* Table is not present on this device. */
        idx_min = 1;
        idx_max = BCM_XGS3_L3_RP_MAX_PREFIXES(unit);
    } else 
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
    {
        /* Get table memory. */
        mem = (ipv6) ? BCM_XGS3_L3_MEM(unit, v6) : BCM_XGS3_L3_MEM(unit, v4);
        idx_max = soc_mem_index_max(unit, mem);
        idx_min =  soc_mem_index_min(unit, mem);
    }

    /* Iterate over all the entries - delete matching ones. */
    for (idx = idx_min; idx <= idx_max; idx++) {

        /* Set protocol. */
        l3cfg.l3c_flags = flags;

        /* Get entry from  hw. */
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get_by_idx) (unit, NULL, idx,
                                                           &l3cfg, &nh_idx);

        /* Check for read errors & invalid entries. */
        if (rv < 0) {
            if (rv != BCM_E_NOT_FOUND) {
                return rv;
            }
            continue;
        }

        /* Check read entry flags & update index accordingly. */
        if (BCM_L3_CMP_EQUAL !=
            _bcm_xgs3_trvrs_flags_cmp(unit, flags, l3cfg.l3c_flags, &idx)) {
            continue;
        }

        /* If compare function provided match the entry against pattern */
        if (cmp_func) {
            BCM_IF_ERROR_RETURN((*cmp_func) (unit, pattern, (void *)&l3cfg,
                                             (void *)&nh_idx, &cmp_result));
            /* If entry doesn't match the pattern don't delete it. */
            if (BCM_L3_CMP_EQUAL != cmp_result) {
                continue;
            }
        }

        /* Entry matches the rule -> delete it. */
        BCM_IF_ERROR_RETURN(bcm_xgs3_l3_del(unit, &l3cfg));

        /* Check if notification required. */
        if (notify_cb) {
            /* Fill host info. */
            _bcm_xgs3_host_ent_init(unit, &l3cfg, FALSE, &info);
            /* Call notification callback. */
            (*notify_cb) (unit, idx, &info, user_data);
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_del_prefix
 * Purpose:
 *      Delete an entry in L3 table.
 * Parameters:
 *      unit  - (IN)SOC unit number.
 *      l3cfg - (IN)Pointer to memory for l3 table related information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_del_prefix(int unit, _bcm_l3_cfg_t *l3cfg)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == l3cfg) {
        return (BCM_E_PARAM);
    }

    return BCM_XGS3_L3_HWCALL_EXEC(unit, l3_del_match)
                                   (unit, l3cfg->l3c_flags,
                                    (void *)l3cfg,
                                    _bcm_xgs3_l3_prefix_cmp,
                                    NULL, NULL);
}

/*
 * Function:
 *      bcm_xgs3_l3_del_intf
 * Purpose:
 *      Delete all the entries in L3 table with nh matching a certain
 *      interface.
 * Parameters:
 *      unit   - (IN)SOC unit number.
 *      l3cfg  - (IN)Pointer to memory for l3 table related information.
 *      negate - (IN)0 means interface match; 1 means not match
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_del_intf(int unit, _bcm_l3_cfg_t *l3cfg, int negate)
{
    _bcm_if_del_pattern_t pattern;      /* Interface & negate combination 
                                           to be used for every l3 entry
                                           comparision.                  */
    bcm_l3_egress_t egr;                /* Egress object.                */
    int nh_index;                       /* Next hop index.               */
    bcm_if_t intf;                      /* Deleted interface id.         */
    int tmp_rv;                         /* First error occured.          */
    int rv;                             /* Operation return status.      */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == l3cfg) {
        return (BCM_E_PARAM);
    }

    intf = l3cfg->l3c_intf;

    /* Check l3 switching mode. */ 
    if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf) || 
            BCM_XGS3_DVP_EGRESS_IDX_VALID(unit, intf)) {
            if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, intf)) {
                nh_index = intf - BCM_XGS3_EGRESS_IDX_MIN; 
            } else {
                nh_index = intf - BCM_XGS3_DVP_EGRESS_IDX_MIN;
            }
            /* Get egress object information. */
            BCM_IF_ERROR_RETURN(bcm_xgs3_nh_get(unit, nh_index, &egr));
            /* Use egress object interface for iteration */
            intf = egr.intf;
        }
    }

    pattern.l3_intf = intf;
    pattern.negate = negate;

    /* Delete all ipv4 entries matching the interface. */
    tmp_rv = _bcm_xgs3_l3_del_match(unit, 0, (void *)&pattern,
                                    _bcm_xgs3_l3_intf_cmp, NULL, NULL);

    /* Delete all ipv6 entries matching the interface. */
    rv = _bcm_xgs3_l3_del_match(unit, BCM_L3_IP6, (void *)&pattern,
                                _bcm_xgs3_l3_intf_cmp, NULL, NULL);
    return (tmp_rv < 0) ? tmp_rv : rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_del_all
 * Purpose:
 *      Delete all entries in L3 table.
 * Parameters:
 *      unit - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_del_all(int unit)
{
    int tmp_rv;         /* First error occured.          */
    int rv;             /* Operation return status.      */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Delete all ipv4 entries. */
    tmp_rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_del_match)
                       (unit, 0, NULL, NULL, NULL, NULL);

    /* Delete all ipv6 entries. */
    rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_del_match)
                   (unit, BCM_L3_IP6, NULL, NULL, NULL, NULL);

    return (tmp_rv < 0) ? tmp_rv : rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_age
 * Purpose:
 *      Age out the l3 entry based on L3SH, L3DH or both.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      flags     - (IN)The criteria used to age or reset entry hit .
 *      age_out   - (IN)Call back routine.
 *      user_data - (IN)User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs3_l3_age(int unit, uint32 flags, bcm_l3_host_traverse_cb age_out, 
                void *user_data)
{
    int tmp_rv;         /* First error occured.          */
    int rv;             /* Operation return status.      */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Reset set flags to reset hit if required.  */
    /* SEEMS LIKE API INCONSISTENCY. */
    if (flags & BCM_L3_HIT) {
        flags = BCM_L3_HIT_CLEAR;
    }

    /* Age all ipv4 entries. */
    tmp_rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_del_match)
                                     (unit, flags, (void *)&flags,
                                      _bcm_xgs3_l3_cmp_hit, age_out, user_data);

    /* Age all ipv6 entries. */
    flags |= BCM_L3_IP6;
    rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_del_match)
                                 (unit, flags, (void *)&flags,
                                  _bcm_xgs3_l3_cmp_hit, age_out, user_data);

    return (tmp_rv < 0) ? tmp_rv : rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_replace
 * Purpose:
 *      Replace an entry in L3 table.
 * Parameters:
 *      unit -  (IN)SOC unit number.
 *      l3cfg - (IN)L3 entry information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_replace(int unit, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_l3_cfg_t entry;        /* Original l3 entry.             */
    int nh_idx_old;             /* Original entry next hop index. */
    int nh_idx_new;             /* New allocated next hop index   */
    int rv = BCM_E_UNAVAIL;     /* Operation return status.       */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == l3cfg) {
        return (BCM_E_PARAM); 
    }

    /* Set lookup key. */
    entry = *l3cfg;

    if (BCM_XGS3_L3_MCAST_ENTRY(l3cfg)) {
        /* Check if identical entry exits. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_get) (unit, &entry);
        }
        if (rv < 0) {
            return rv;
        }

        /* Write mcast entry to hw. */
        l3cfg->l3c_hw_index = entry.l3c_hw_index;       /*Replacement index & flag. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_add)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_add) (unit, l3cfg);
        } else {
            return (BCM_E_UNAVAIL);
        }
    } else {
        if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
            /* Trunk id validation. */
            BCM_XGS3_L3_IF_INVALID_TGID_RETURN(unit, l3cfg->l3c_flags,
                                               l3cfg->l3c_port_tgid);
        }

        /* Check if identical entry exits. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get) (unit, &entry,
                                                        &nh_idx_old);
        }
        if ((BCM_E_NOT_FOUND == rv) || (BCM_E_DISABLED == rv)) {
            rv =  bcm_xgs3_host_as_route(unit, l3cfg, BCM_XGS3_L3_OP_ADD, rv);
            if (BCM_SUCCESS(rv)) {
                /* Special return value - as entry Replaced within ROUTE table */
                (void) bcm_xgs3_l3_host_as_route_return_get(unit, &rv);                
            }
            return rv;
        } else if (BCM_FAILURE(rv)) {
            return rv;
        }

        if (BCM_GPORT_IS_BLACK_HOLE(l3cfg->l3c_port_tgid)) {
             nh_idx_new = 0;
        } else {
              /* Get next hop index. */
              BCM_IF_ERROR_RETURN
                   (_bcm_xgs3_nh_init_add(unit, l3cfg, NULL, &nh_idx_new));
        }
        /* Write entry to hw. */
        l3cfg->l3c_hw_index = entry.l3c_hw_index; /*Replacement index & flag. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_add)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_add)(unit, l3cfg, nh_idx_new);
        } else {
            rv = BCM_E_UNAVAIL;
        }

        /* Do best effort clean_up if needed. */
        if (BCM_FAILURE(rv)) {
            bcm_xgs3_nh_del(unit, 0, nh_idx_new);
        }

        /* Free original next hop index. */
        BCM_IF_ERROR_RETURN(bcm_xgs3_nh_del(unit, 0, nh_idx_old));
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_l3_traverse
 * Purpose:
 *      Walk over range of entries in l3 in the table.
 * Parameters:
 *      unit - SOC unit number.
 *      flags - L3 entry flags to match during traverse.  
 *      start - First index to read. 
 *      end - Last index to read.
 *      cb  - Caller notification callback.
 *      user_data - User cookie, which should be returned in cb. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_traverse(int unit, int flags, uint32 start, uint32 end,
                      bcm_l3_host_traverse_cb cb, void *user_data)
{
    _bcm_l3_cfg_t l3cfg;        /* HW entry info.                  */
    bcm_l3_host_t info;         /* Callback info buffer.           */
    int table_ent_size;         /* Table entry size. */
    int table_size;          /* Number of entries in the table. */
    soc_mem_t mem;              /* Table memory.                   */
    int idx_max;                /* Maximum iteration index.        */
    int idx_min;                /* Minimum iteration index.        */
    int nh_idx;                 /* Next hop index.                 */
    int ipv6;                   /* Protocol IPv6/IPv4.             */
    int idx;                    /* Iterator index.                 */
    char *l3_tbl_ptr = NULL;    /* Table dma pointer.              */
    int total = 0;              /* Total number of entries walked. */
    int rv = BCM_E_NONE;        /* Operation return value.         */

    /* If no callback provided, we are done.  */
    if (NULL == cb) {
        return (BCM_E_NONE);
    }

    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get_by_idx)) {
        return (BCM_E_UNAVAIL);
    }

    ipv6 = (flags & BCM_L3_IP6) ? TRUE : FALSE;
    /* If table is empty -> there is nothing to read. */
    if (ipv6 && (!BCM_XGS3_L3_IP6_CNT(unit))) {
        return (BCM_E_NONE);
    }
    if (!ipv6 && (!BCM_XGS3_L3_IP4_CNT(unit))) {
        return (BCM_E_NONE);
    }

    /* Get table boundaries. */
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
        /* Table is not present on this device. */
        idx_min = 1;
        idx_max = BCM_XGS3_L3_RP_MAX_PREFIXES(unit);
        table_size = idx_max;
    } else 
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
    {
        /* Get table memory. */
        mem = (ipv6) ? BCM_XGS3_L3_MEM(unit, v6) : BCM_XGS3_L3_MEM(unit, v4);
        idx_max = soc_mem_index_max(unit, mem);
        idx_min =  soc_mem_index_min(unit, mem);
        /* Get single entry size. */
        table_ent_size =
           (ipv6) ? BCM_XGS3_L3_ENT_SZ(unit, v6) : BCM_XGS3_L3_ENT_SZ(unit, v4);

        /* Dma the table - to speed up operation. */
        BCM_IF_ERROR_RETURN
             (bcm_xgs3_l3_tbl_dma(unit, mem, table_ent_size, 
                                  "l3_tbl", &l3_tbl_ptr, &table_size));
    }

    /* Input indexes sanity. */
    if (start > (uint32)table_size || end > (uint32)table_size || start > end) {
        return (BCM_E_NOT_FOUND);
    }


    /* Iterate over all the entries - show matching ones. */
    for (idx = idx_min; idx <= idx_max; idx++) {
        /* Reset buffer before read. */
        sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
        l3cfg.l3c_flags = flags;

        /* Get entry from  hw. */
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get_by_idx) (unit, l3_tbl_ptr,
                                                           idx, &l3cfg,
                                                           &nh_idx);

        /* Check for read errors & invalid entries. */
        if (rv < 0) {
            if (rv != BCM_E_NOT_FOUND) {
                break;
            }
            continue;
        }

        /* Check read entry flags & update index accordingly. */
        if (BCM_L3_CMP_EQUAL !=
            _bcm_xgs3_trvrs_flags_cmp(unit, flags, l3cfg.l3c_flags, &idx)) {
            continue;
        }

        /* Valid entry found -> increment total table_sizeer */
        total++;
        if ((uint32)total < start) {
            continue;
        }

        /* Don't read beyond last required index. */
        if ((uint32)total > end) {
            break;
        }

        /* Get next hop info. */
        rv = _bcm_xgs3_l3_get_nh_info(unit, &l3cfg, nh_idx);
        if (rv < 0) {
            break;
        }

        /* Fill host info. */
        _bcm_xgs3_host_ent_init(unit, &l3cfg, TRUE, &info);
        rv = (*cb) (unit, total, &info, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
        if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
            break;
        }
#endif
    }

    if(l3_tbl_ptr) {
        soc_cm_sfree(unit, l3_tbl_ptr);
    }

    /* Reset last read status. */
    if (BCM_E_NOT_FOUND == rv) {
        rv = BCM_E_NONE;
    }

    return (rv);
}

/*
 * Function:
 *      bcm_xgs3_l3_ip4_traverse
 * Purpose:
 *      Walk over range of entries in l3 in the table.
 * Parameters:
 *      unit - SOC unit number.
 *      start - First index to read. 
 *      end - Last index to read.
 *      cb  - Caller notification callback.
 *      user_data - User cookie, which should be returned in cb. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_ip4_traverse(int unit, uint32 start, uint32 end,
                         bcm_l3_host_traverse_cb cb, void *user_data)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    return (BCM_XGS3_L3_HWCALL_EXEC(unit, l3_traverse)
                         (unit, 0, start, end, cb, user_data));
}

/*
 * Function:
 *      bcm_xgs3_l3_ip6_traverse
 * Purpose:
 *      Walk over range of entries in l3 in the table.
 * Parameters:
 *      unit - SOC unit number.
 *      start - First index to read. 
 *      end - Last index to read.
 *      cb  - Caller notification callback.
 *      user_data - User cookie, which should be returned in cb. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_ip6_traverse(int unit, uint32 start, uint32 end,
                         bcm_l3_host_traverse_cb cb, void *user_data)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    return (BCM_XGS3_L3_HWCALL_EXEC(unit, l3_traverse)
                         (unit, BCM_L3_IP6, start, end, cb, user_data));
}

/*
 * Function:
 *      _bcm_xgs3_ip_key_to_l3cfg
 * Purpose:
 *      Translate l3 key to l3_cfg structure, which can be submitted for 
 *      hw lookup.   
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      ipkey    - (IN)IP address key.
 *      l3cfg    - (IN)Lookup structure to fill.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_ip_key_to_l3cfg(int unit, bcm_l3_key_t *ipkey, _bcm_l3_cfg_t *l3cfg)
{
    int ipv6;           /* IPV6 key.                     */

    /* Input parameters check */
    if ((NULL == ipkey) || (NULL == l3cfg)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination buffer first. */
    sal_memset(l3cfg, 0, sizeof(_bcm_l3_cfg_t));

    ipv6 = (ipkey->l3k_flags & BCM_L3_IP6);

    /* Set vrf id. */
    BCM_ER_VRF_INVALID_RETURN(unit, ipv6, ipkey->l3k_vrf); 
    l3cfg->l3c_vrf = ipkey->l3k_vrf;
    l3cfg->l3c_vid = ipkey->l3k_vid;

    if (ipv6) {
        if (BCM_IP6_MULTICAST(ipkey->l3k_ip6_addr)) {
            /* Copy ipv6 group, source & vid. */
            sal_memcpy(l3cfg->l3c_ip6, ipkey->l3k_ip6_addr, sizeof(bcm_ip6_t));
            sal_memcpy(l3cfg->l3c_sip6, ipkey->l3k_sip6_addr,
                       sizeof(bcm_ip6_t));
            l3cfg->l3c_vid = ipkey->l3k_vid;
            l3cfg->l3c_flags = (BCM_L3_IP6 | BCM_L3_IPMC);
        } else {
            /* Copy ipv6 address. */
            sal_memcpy(l3cfg->l3c_ip6, ipkey->l3k_ip6_addr, sizeof(bcm_ip6_t));
            l3cfg->l3c_flags = BCM_L3_IP6;
        }
    } else {
        if (BCM_IP4_MULTICAST(ipkey->l3k_ip_addr)) {
            /* Copy ipv4 mcast group, source & vid. */
            l3cfg->l3c_ip_addr = ipkey->l3k_ip_addr;
            l3cfg->l3c_src_ip_addr = ipkey->l3k_sip_addr;
            l3cfg->l3c_vid = ipkey->l3k_vid;
            l3cfg->l3c_flags = BCM_L3_IPMC;
        } else {
            /* Copy ipv4 address . */
            l3cfg->l3c_ip_addr = ipkey->l3k_ip_addr;
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3cfg_to_ipkey
 * Purpose:
 *      Translate ipkey to l3_cfg structure, which can be submitted for 
 *      hw lookup.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      ipkey    - (IN)IP address key.
 *      l3cfg    - (IN)Lookup structure to fill.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_l3cfg_to_ipkey(int unit, bcm_l3_key_t *ipkey, _bcm_l3_cfg_t *l3cfg)
{
    /* Input parameters check */
    if ((NULL == ipkey) || (NULL == l3cfg)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination buffer first. */
    sal_memset(ipkey, 0, sizeof(bcm_l3_key_t));

    ipkey->l3k_vrf = l3cfg->l3c_vrf;
    ipkey->l3k_vid = l3cfg->l3c_vid;

    if (l3cfg->l3c_flags & BCM_L3_IP6) {
        if (l3cfg->l3c_flags & BCM_L3_IPMC) {
            /* Copy ipv6 group, source & vid. */
            sal_memcpy(ipkey->l3k_ip6_addr, l3cfg->l3c_ip6, sizeof(bcm_ip6_t));
            sal_memcpy(ipkey->l3k_sip6_addr, l3cfg->l3c_sip6,
                       sizeof(bcm_ip6_t));
            ipkey->l3k_vid = l3cfg->l3c_vid;
        } else {
            /* Copy ipv6 address. */
            sal_memcpy(ipkey->l3k_ip6_addr, l3cfg->l3c_ip6, sizeof(bcm_ip6_t));
        }
    } else {
        if (l3cfg->l3c_flags & BCM_L3_IPMC) {
            /* Copy ipv4 mcast group, source & vid. */
            ipkey->l3k_ip_addr = l3cfg->l3c_ip_addr;
            ipkey->l3k_sip_addr = l3cfg->l3c_src_ip_addr;
            ipkey->l3k_vid = l3cfg->l3c_vid;
        } else {
            /* Copy ipv4 address . */
            ipkey->l3k_ip_addr = l3cfg->l3c_ip_addr;
        }
    }
    /* Store entry flags. */
    ipkey->l3k_flags = l3cfg->l3c_flags;
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_l3_get_by_index
 * Purpose:
 *      Get L3 entry at index
 * Parameters:
 *      unit  - (IN)SOC unit number.
 *      idx   - (IN)Entry index to read. 
 *      l3cfg - (OUT) decoded L3 entry
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_get_by_index(int unit, int idx, _bcm_l3_cfg_t *l3cfg)
{
    int rv = BCM_E_UNAVAIL;     /* Operation return code. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    if (l3cfg->l3c_flags & BCM_L3_IPMC) {
        /* Get multicast entry. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, ipmc_get_by_idx)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, ipmc_get_by_idx) (unit, NULL,
                                                                 idx, l3cfg);
        }
    } else {
        /* Get unicast entry. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get_by_idx)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get_by_idx) (unit, NULL, idx,
                                                               l3cfg, NULL);
        }
    }
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_l3_del_by_index
 * Purpose:
 *      Delete L3 entry at index. 
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      idx        - (IN)Index to read.   
 *      flags      - (IN)l3cfg entry flags. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_del_by_index(int unit, int idx, int flags)
{
    _bcm_l3_cfg_t l3cfg;        /* L3 entry info structure. */
    int rv;                     /* Return value.            */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    l3cfg.l3c_flags = flags;
    /* Read entry from hw. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_l3_get_by_index(unit, idx, &l3cfg));

    /* If read successfully delete the entry. */
    rv = bcm_xgs3_l3_del(unit, &l3cfg);
    return (rv);
}

/*
 * Function:
 *      bcm_xgs3_l3_conflict_get
 * Purpose:
 *      Given a IP address, return conflicts in the L3 table.
 * Parameters:
 *      unit     - SOC unit number.
 *      ipkey    - IP address to test conflict condition
 *      cf_array - (OUT) arrary of conflicting addresses(at most 8)
 *      cf_max   - max number of conflicts wanted
 *      count    - (OUT) actual # of conflicting addresses
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_conflict_get(int unit, bcm_l3_key_t *ipkey,
                         bcm_l3_key_t *cf_array, int cf_max, int *cf_count)
{
    _bcm_l3_cfg_t l3cfg;        /* Lookup entry info.         */
    int l3_index;               /* Adjusted table index.      */ 
    int bucket_primary;         /* Primary bucket in bank 0.  */
    int bucket_secondary;       /* Secondary bucket in bank 1.*/
    int bucket_iterator;        /* Bucket iterator.           */
    int idx;                    /* Iteration index.           */
    int idx_max;                /* Iteration end index.       */
    int rv = BCM_E_UNAVAIL;     /* Operation return value.    */


    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if ((NULL == ipkey) || (NULL == cf_count) || 
        (NULL == cf_array) || (cf_max <= 0)) {
        return (BCM_E_PARAM);
    }

    /* Make sure hw call to read ip4 entries is initialized. */
    if ((!BCM_XGS3_L3_HWCALL_CHECK(unit, l3_get_by_idx)) || 
        (!BCM_XGS3_L3_HWCALL_CHECK(unit, l3_bucket_get))) {
        return (BCM_E_UNAVAIL);
    }

    *cf_count = 0;

    /* Translate lookup key to l3cfg format. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_ip_key_to_l3cfg(unit, ipkey, &l3cfg));

    /* Ge. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, l3_bucket_get)(unit, &l3cfg,
                                                      &bucket_primary,
                                                      &bucket_secondary));

    /*
     * Loop through all L3_BUCKET_SIZE entries in this bucket
     */
    for (bucket_iterator = 0; bucket_iterator < 2; bucket_iterator++) {
        idx = (bucket_iterator) ? bucket_secondary : bucket_primary;
        idx_max = idx + SOC_L3X_BUCKET_SIZE(unit)/2; 

        for (;idx < idx_max; idx++) {
            /* Reset entry flags so we alwayes read ipv4 entry. */
            l3cfg.l3c_flags = 0;

            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, l3_get_by_idx) (unit, NULL, idx,
                                                               &l3cfg, NULL);
            if (rv == BCM_E_NOT_FOUND) {
                continue;
            }
            if (BCM_FAILURE(rv)) {
                return rv;
            }

            /* Reread entry if it is not ipv4 unicast one. */
            if (l3cfg.l3c_flags & (BCM_L3_IP6 | BCM_L3_IPMC)) {
                l3_index = idx;
#ifdef BCM_FIREBOLT_SUPPORT
                BCM_XGS3_FB_IPMC_IP6_IDX_CALC(unit, idx, l3_index, l3cfg.l3c_flags);
#endif /* BCM_FIREBOLT_SUPPORT */
                BCM_IF_ERROR_RETURN
                    (bcm_xgs3_l3_get_by_index(unit, l3_index, &l3cfg));
            }

            /* Fill conflicting entry in key format. */
            _bcm_xgs3_l3cfg_to_ipkey(unit, cf_array + (*cf_count), &l3cfg);
            if ((++(*cf_count)) >= cf_max) {
                break;
            }
        }
    }
    return (BCM_E_NONE);
}

#ifdef BCM_EASYRIDER_SUPPORT
/*
 * Function:
 *      _bcm_er_l3_ent_count
 * Purpose:
 *      Return the number of used entries in the L3 table 
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      entry_count  - (OUT)Number of entries in the table. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_l3_ent_count(int unit, int *entry_count)
{
    l3_entry_v4_entry_t *v4_tp; /* L3 v4 table pointer. */
    l3_entry_v6_entry_t *v6_tp; /* L3 v6 table pointer. */
    char *l3_tbl_ptr;           /* DMA memory pointer.  */
    int idx_max;                /* Maximum table index. */
    int idx;                    /* Iteration index.     */
    int rv;                     /* Return value.        */
    int total = 0;              /* Counter.             */

    if (NULL == entry_count) {
        return (BCM_E_PARAM);
    }

    /* Count number of v4 entries. */
    /* Dma the v4 table. */
    rv = bcm_xgs3_l3_tbl_dma(unit, BCM_XGS3_L3_MEM(unit, v4),
                             BCM_XGS3_L3_ENT_SZ(unit, v4), "l3_v4_tbl",
                             &l3_tbl_ptr, &idx_max);
    if (BCM_E_NOT_FOUND != rv) {
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        /* Loop over all the table & increment counter for every valid entry. */
        for (idx = 0; idx < idx_max; idx++) {
            v4_tp = soc_mem_table_idx_to_pointer(unit, BCM_XGS3_L3_MEM(unit, v4),
                                                 l3_entry_v4_entry_t *,
                                                 l3_tbl_ptr, idx);
            if (soc_L3_ENTRY_V4m_field32_get(unit, v4_tp, VALIDf)) {
                total++;
            }
        }
        /* Free v4 image. */
        soc_cm_sfree(unit, l3_tbl_ptr);
    }

    /* Dma the v6 table. */
    rv = bcm_xgs3_l3_tbl_dma(unit, BCM_XGS3_L3_MEM(unit, v6),
                             BCM_XGS3_L3_ENT_SZ(unit, v6), "l3_v6_tbl",
                             &l3_tbl_ptr, &idx_max);
    if (BCM_E_NOT_FOUND != rv) {
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        /* Loop over all the table & increment counter for every valid entry. */
        for (idx = 0; idx < idx_max; idx++) {
            v6_tp = soc_mem_table_idx_to_pointer(unit, BCM_XGS3_L3_MEM(unit, v6),
                                                 l3_entry_v6_entry_t *,
                                                 l3_tbl_ptr, idx);
            if (soc_L3_ENTRY_V6m_field32_get(unit, v6_tp, VALIDf)) {
                total++;
            }
        }

        /* Free v6 image. */
        soc_cm_sfree(unit, l3_tbl_ptr);
    }
    *entry_count = total;
    return (BCM_E_NONE);
}
#endif /* BCM_EASYRIDER_SUPPORT */

/*
 * Function:
 *      bcm_xgs3_l3_info_dump
 * Purpose:
 *      Dump the status of hardware.
 * Parameters:
 *      unit   - (IN)SOC unit number
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs3_l3_info_dump(int unit)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32 ipv6_lpm_128b_enable;
    uint32 ipv6_64b_free = 0;
    uint32 ipv6_64b_used = 0;
    uint32 ipv6_128b_free = 0;
    uint32 ipv6_128b_used = 0;
    uint32 ipv6_128b_total = 0;
    uint32 ipv4_free = 0;
    uint32 ipv4_used = 0;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    if (SOC_IS_KATANA(unit)) {
        ipv6_lpm_128b_enable = soc_property_get(unit,spn_IPV6_LPM_128B_ENABLE, 0);
        soc_cm_print("\n*************************************************");
        soc_cm_print("\nURPF check \t\t\t\t= %s",
                     SOC_URPF_STATUS_GET(unit) ? "ENABLED" : "DISABLED");
        soc_cm_print("\nconfig ipv6_lpm_128b_enable \t\t= %d", ipv6_lpm_128b_enable);
        soc_cm_print("\n*************************************************");
        soc_cm_print("\nMaximum LPM entries types");
        soc_cm_print("\n     IPv4 * IPv6 64 bit *IPv6 128 bit");
        if (SOC_URPF_STATUS_GET(unit)) {
            if (ipv6_lpm_128b_enable == 0 ) {
                soc_cm_print("\nupto 6144   3072         128");
            } else {
                soc_cm_print("\nupto 2048   1024         1152");
            }
        } else {
            if (ipv6_lpm_128b_enable == 0) {
                soc_cm_print("\nupto 12288  6144         256");
            } else {
                soc_cm_print("\nupto 4096   2048         2304");
            }
        }

        /* IPv4, IPv6 64b and IPv6 pair128b entries share L3_DEFIPm 
         * Ipv6 64b and IPv6 pair128b are stored in mutually exclusive entries */
        if (kt_depth128 > 0) {
            ipv6_128b_total = kt_depth128 + BCM_TRX_DEFIP128_IDX_MAX(unit) + 1;
            ipv6_128b_used = BCM_KT_DEFIP_PAIR128_USED_COUNT(unit) + 
                                 BCM_TRX_DEFIP128_USED_COUNT(unit);
            ipv6_128b_free = ipv6_128b_total - ipv6_128b_used;
        } else {
            ipv6_128b_total = BCM_TRX_DEFIP128_IDX_MAX(unit) + 1;
            ipv6_128b_used = BCM_TRX_DEFIP128_USED_COUNT(unit);
            ipv6_128b_free = ipv6_128b_total - ipv6_128b_used;
        }
        ipv6_64b_used = BCM_XGS3_L3_DEFIP_IP6_CNT(unit) - ipv6_128b_used;
        ipv4_used = BCM_XGS3_L3_DEFIP_IP4_CNT(unit);        
        ipv6_64b_free = kt_depth64 - ((ipv4_used+1)/2 + ipv6_64b_used);
        ipv4_free = ipv6_64b_free * 2;

        soc_cm_print("\n*************************************************");
        soc_cm_print("\nIPv4 LPM entries in use \t\t= %5d", ipv4_used);
        soc_cm_print("\nIPv4 LPM entries free \t\t\t= %5d", ipv4_free);
        soc_cm_print("\nIPv4 LPM entries total \t\t\t= %5d", ipv4_free + ipv4_used);
        soc_cm_print("\n**********");
        soc_cm_print("\nIPv6 64 bit LPM entries in use \t\t= %5d", ipv6_64b_used);
        soc_cm_print("\nIPv6 64 bit LPM entries free \t\t= %5d", ipv6_64b_free);
        soc_cm_print("\n64 bit LPM DEFIP entries total \t\t= %5d",
                     ipv6_64b_used + ipv6_64b_free);
        soc_cm_print("\n**********");
        soc_cm_print("\nIPv6 128 bit LPM entries in use \t= %5d", ipv6_128b_used);
        soc_cm_print("\nIPv6 128 bit LPM entries free \t\t= %5d", ipv6_128b_free);
        soc_cm_print("\nIPv6 128 bit LPM entries total \t\t= %5d",
                     (ipv6_128b_used + ipv6_128b_free));
        soc_cm_print("\n**********");
        soc_cm_print("\nTotal IPv6 LPM entries in use \t\t= %5d",
                     BCM_XGS3_L3_DEFIP_IP6_CNT(unit));
        soc_cm_print("\nTotal IPv6 LPM entries free \t\t= %5d", 
                     (ipv6_64b_free + ipv6_128b_free));
        soc_cm_print("\nTotal IPv6 LPM entries \t\t\t= %5d\n",
                     BCM_XGS3_L3_DEFIP_IP6_CNT(unit) +
                     (ipv6_64b_free + ipv6_128b_free));
    }
#endif
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_info
 * Purpose:
 *      Get the status of hardware.
 * Parameters:
 *      unit   - (IN)SOC unit number
 *      l3info - (OUT)Structure to fill with L3 related information.
 * Returns:
 *      BCM_E_XXX.
 */
int
bcm_xgs3_l3_info(int unit, bcm_l3_info_t *l3info)
{
#ifdef BCM_TRIUMPH_SUPPORT 
    soc_mem_t mem;
    BCM_IF_ERROR_RETURN(_bcm_tr_l3_defip_mem_get(unit, 0, 0, &mem));
#endif /* BCM_TRIUMPH_SUPPORT */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == l3info) {
        return (BCM_E_PARAM);
    }

    if (SOC_DEBUG_CHECK(DK_L3)) {
        bcm_xgs3_l3_info_dump(unit);
    }

    l3info->l3info_max_vrf = SOC_VRF_MAX(unit);
    l3info->l3info_used_vrf = -1;
    l3info->l3info_max_intf = BCM_XGS3_L3_IF_TBL_SIZE(unit);
    l3info->l3info_max_intf_group = SOC_INTF_CLASS_MAX(unit);
    l3info->l3info_max_l3 = BCM_XGS3_L3_TBL_SIZE(unit);
    l3info->l3info_max_defip = BCM_XGS3_L3_DEFIP_TBL_SIZE(unit);
    l3info->l3info_max_ecmp = BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
    l3info->l3info_occupied_intf = BCM_XGS3_L3_IF_COUNT(unit);
    l3info->l3info_max_host = l3info->l3info_max_l3;
    l3info->l3info_max_lpm_block = 0;
    l3info->l3info_used_lpm_block = 0;

#ifdef BCM_TRIUMPH_SUPPORT
    if ((soc_feature(unit, soc_feature_esm_support)) && (L3_DEFIPm != mem)) { 
        /* ESM - One route per entry */
        l3info->l3info_max_route = l3info->l3info_max_defip;
    } else 
#endif /* BCM_TRIUMPH_SUPPORT */
    {  /* Two routes per entry */
        l3info->l3info_max_route = l3info->l3info_max_defip << 1;
    }

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        l3info->l3info_occupied_defip = 
            BCM_XGS3_L3_DEFIP_IP4_CNT(unit) + BCM_XGS3_L3_DEFIP_IP6_CNT(unit);
        BCM_IF_ERROR_RETURN
            (_bcm_er_l3_ent_count(unit, &l3info->l3info_occupied_l3));
    }
#endif /* BCM_EASYRIDER_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
        if (soc_feature(unit, soc_feature_fp_based_routing)) {
            l3info->l3info_max_route = l3info->l3info_max_defip;
            l3info->l3info_occupied_l3 = 
                BCM_XGS3_L3_IP4_CNT(unit) + BCM_XGS3_L3_IP6_CNT(unit);
            l3info->l3info_occupied_defip = 
                BCM_XGS3_L3_DEFIP_IP4_CNT(unit) + BCM_XGS3_L3_DEFIP_IP6_CNT(unit);
        } else 
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
        {
            l3info->l3info_occupied_l3 = BCM_XGS3_L3_IP4_CNT(unit) +
                BCM_XGS3_L3_IP4_IPMC_CNT(unit) + 2 * BCM_XGS3_L3_IP6_CNT(unit) \
                + 4 * BCM_XGS3_L3_IP6_IPMC_CNT(unit);
            /* Entries occupied in L3_DEFIPm excluding L3_DEFIP_128m */
#if defined(BCM_KATANA_SUPPORT)
            if (SOC_IS_KATANA(unit)) {
                uint32 ipv6_128b_used = 0;
                if (kt_depth128 > 0) {
                    ipv6_128b_used = BCM_KT_DEFIP_PAIR128_USED_COUNT(unit);
                }
                l3info->l3info_occupied_defip = 
                    BCM_XGS3_L3_DEFIP_IP4_CNT(unit)
                    + ((BCM_XGS3_L3_DEFIP_IP6_CNT(unit) - ipv6_128b_used) * 2);
            } else
#endif
            {
                l3info->l3info_occupied_defip = BCM_XGS3_L3_DEFIP_IP4_CNT(unit) +
                    (BCM_XGS3_L3_DEFIP_IP6_CNT(unit) * 2);
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    l3info->l3info_occupied_host = l3info->l3info_occupied_l3;
    l3info->l3info_occupied_route = l3info->l3info_occupied_defip;
    l3info->l3info_max_nexthop = BCM_XGS3_L3_NH_TBL_SIZE(unit);
    l3info->l3info_used_nexthop = BCM_XGS3_L3_TBL(unit, next_hop).idx_maxused;
    
    if(!SOC_IS_HURRICANE(unit)) {
        l3info->l3info_max_tunnel_init = BCM_XGS3_L3_TUNNEL_TBL_SIZE(unit);
        l3info->l3info_used_tunnel_init = BCM_XGS3_L3_TBL(unit, tnl_init).idx_maxused;
        l3info->l3info_max_tunnel_term = soc_mem_index_count(unit, L3_TUNNELm);
        l3info->l3info_used_tunnel_term = soc_tunnel_term_used_get(unit);
    } else {
        l3info->l3info_max_tunnel_init = 0;
        l3info->l3info_used_tunnel_init = 0;
        l3info->l3info_max_tunnel_term = 0; 
        l3info->l3info_used_tunnel_term = 0;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_lpm_get_nh_info
 * Purpose:
 *      Fill next hop info to lpm entry. 
 * Parameters:
 *      unit   - (IN)SOC unit number.
 *      lpm_cfg- (IN)Buffer to fill defip information. 
 *      nh_idx - (IN)Next hop index. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_lpm_get_nh_info(int unit, _bcm_defip_cfg_t *lpm_cfg, int nh_idx)
{
    bcm_l3_egress_t nh_info;  /* Next hop info buffer.    */

    /* In egress switching mode - set egress object id only. */ 
    if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
        if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
            lpm_cfg->defip_intf = nh_idx + BCM_XGS3_MPATH_EGRESS_IDX_MIN; 
        } else {
            lpm_cfg->defip_intf = nh_idx + BCM_XGS3_EGRESS_IDX_MIN;
        }
        return (BCM_E_NONE);
    }

    /* If next hop is trap to cpu - don't read nh table. */
    if (nh_idx == BCM_XGS3_L3_L2CPU_NH_IDX) {
        /* Set flags to bridge to cpu. */
        lpm_cfg->defip_flags |= BCM_L3_DEFIP_LOCAL;
        /* Set interface index to last one in the table. */
        lpm_cfg->defip_intf = BCM_XGS3_L3_L2CPU_INTF_IDX(unit);
        /* Set module id to a local module. */
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &lpm_cfg->defip_modid));
        /* Set port to be a cpu port. */
        lpm_cfg->defip_port_tgid = CMIC_PORT(unit);
        /* No vlan/interface info. */
        lpm_cfg->defip_vid = 0;
        lpm_cfg->defip_tunnel = 0;
        /* Reset mac address. */
        sal_memset(lpm_cfg->defip_mac_addr, 0, sizeof(bcm_mac_t));
        return (BCM_E_NONE);
    }

    /* Get next hop entry from hw. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_nh_get(unit, nh_idx, &nh_info));

    /* Fill next hop info to l3 entry. */
    if (nh_info.flags & BCM_L3_TGID) {       /* Trunk */
        lpm_cfg->defip_flags |= BCM_L3_TGID;
    }
    /* Set module id. */
    lpm_cfg->defip_modid = nh_info.module;
    /* Set trunk/port info. */
    lpm_cfg->defip_port_tgid = 
        (nh_info.flags & BCM_L3_TGID) ? nh_info.trunk : nh_info.port;
    /* Set interface index. */
    lpm_cfg->defip_intf = nh_info.intf;
    /* Set physical address. */
    sal_memcpy(lpm_cfg->defip_mac_addr, nh_info.mac_addr, sizeof(bcm_mac_t));

    /* Get tunnel id info. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_l3_get_tunnel_id(unit, nh_info.intf, 
                                    &lpm_cfg->defip_tunnel));
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        lpm_cfg->defip_vid = nh_info.vlan;
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_defip_set_route_info
 * Purpose:
 *      Fill user callbacks route info.
 * Parameters:
 *      unit       - (IN)SOC unit number. 
 *      lpm_cfg    - (IN)Defip information. 
 *      route_info - (OUT)Buffer to fill route info. 
 *      nh_idx     - (IN)Next hop index. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_defip_set_route_info(int unit, _bcm_defip_cfg_t *lpm_cfg,
                               bcm_l3_route_t *route_info, int nh_idx)
{
    _bcm_defip_cfg_t lpm_temp;  /* Avoid modifying original lpm entry.*/ 
    int ipv6;                   /* IPv6 entry indicator.              */

    /* Input parameters check. */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    lpm_temp =  *lpm_cfg;
    ipv6 = (lpm_temp.defip_flags & BCM_L3_IP6);

    /* Init route entry. */
    bcm_l3_route_t_init(route_info);

    /* Set vrf id. */
    route_info->l3a_vrf = lpm_temp.defip_vrf;

    if (ipv6) {
        /* Set ipv6 address. */
        sal_memcpy(route_info->l3a_ip6_net, lpm_temp.defip_ip6_addr,
                   sizeof(bcm_ip6_t));

        /* Set subnet mask. */
        bcm_ip6_mask_create(route_info->l3a_ip6_mask, lpm_temp.defip_sub_len);

        /* Set entry flags to ipv6 */
        route_info->l3a_flags = BCM_L3_IP6;
    } else {
        /* Set ipv4 address. */
        route_info->l3a_subnet = lpm_temp.defip_ip_addr;

        /* Set subnet mask. */
        route_info->l3a_ip_mask =
            BCM_IP4_MASKLEN_TO_ADDR(lpm_temp.defip_sub_len);

        /* Reset entry flags */
        route_info->l3a_flags = 0;
    }

    /* Fill next hop info if needed. */
    if (nh_idx != BCM_XGS3_L3_INVALID_INDEX) {
        /* Get next hop info. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_lpm_get_nh_info(unit, &lpm_temp, nh_idx));

        /* Entry flags. */
        route_info->l3a_flags |= lpm_temp.defip_flags;

        /* Interface index/Egress object id. */
        route_info->l3a_intf = lpm_temp.defip_intf;

        /* Don't fill additional info in egress switching mode. */
        if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
           return (BCM_E_NONE);
        }

        /* Module id. */
        route_info->l3a_modid = lpm_temp.defip_modid;
        /* Port/Trunk info. */
        route_info->l3a_port_tgid = lpm_temp.defip_port_tgid;
        /* Vlan id. */
        route_info->l3a_vid = lpm_temp.defip_vid;
        /* Mac address. */
        sal_memcpy(route_info->l3a_nexthop_mac, lpm_temp.defip_mac_addr,
                   sizeof(bcm_mac_t));
        /* Priority */
        route_info->l3a_pri = lpm_temp.defip_prio;
        /* Class ID */
        route_info->l3a_lookup_class = lpm_temp.defip_lookup_class;
    }

#if defined (BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)) {
        if (lpm_temp.defip_flags & BCM_L3_ROUTE_LABEL) {
            route_info->l3a_flags |= BCM_L3_ROUTE_LABEL;
            route_info->l3a_mpls_label = lpm_temp.defip_mpls_label;
        } else {
            route_info->l3a_tunnel_option = lpm_temp.defip_tunnel_option;
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_lpm_call_user_cb
 * Purpose:
 *      Fill user callbacks route info and call user callback. 
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      trv_data   - (IN)User cookie & callback routine. 
 *      lpm_cfg    - (IN)Defip information. 
 *      nh_idx     - next hop index. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_lpm_call_user_cb(int unit, _bcm_l3_trvrs_data_t *trv_data,
                           _bcm_defip_cfg_t *lpm_cfg, int nh_idx)
{
    bcm_l3_route_t route_info;

    /* Fill route info. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_defip_set_route_info(unit, lpm_cfg, &route_info, nh_idx));

    /* Call user notification routine. */
    if (NULL != trv_data->defip_cb) {
        (*trv_data->defip_cb) (unit, lpm_cfg->defip_index,
                                &route_info, trv_data->cookie);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_max_ecmp_set
 * Purpose:
 *      Set the max allowable number of ECMP paths that is less
 *      than the hardware allowed value
 * Parameters:
 *      unit       - (IN) SOC device unit number
 *      max_paths  - (IN) Maximum number of ecmp paths. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_max_ecmp_set(int unit, int max_paths)
{
    int rv;                     /* Operation return value. */

    if (!SOC_MEM_IS_VALID(unit, BCM_XGS3_L3_MEM(unit, ecmp)) || 
        !soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, ecmp))) {
        return (BCM_E_UNAVAIL);
    }

    if (BCM_XGS3_L3_ECMP_IN_USE(unit)) {
        BCM_XGS3_L3_ERR(("ECMP already in use, max path can't be reset\n"));
        return (BCM_E_BUSY);
    }

    /* Max paths sanity. */
    if (max_paths < 2 || max_paths > BCM_XGS3_L3_ECMP_MAX(unit)) {
        return (BCM_E_PARAM);
    }

    /* Free original groups array. */
    sal_free(BCM_XGS3_L3_TBL(unit, ecmp_grp).ext_arr);
    BCM_XGS3_L3_TBL(unit, ecmp_grp).ext_arr = NULL;

    /* Set maximum number of paths. */
    BCM_XGS3_L3_ECMP_MAX_PATHS(unit) = max_paths;

    /* Reinitialize ecmp group table. */
    rv = _bcm_xgs3_l3_ecmp_group_init(unit);

    return rv;
}

/*
 * Function:
 *      bcm_xgs3_max_ecmp_get
 * Purpose:
 *      Get the max allowable number of ECMP paths.
 * Parameters:
 *      unit       - (IN) SOC device unit number
 *      max_paths  - (IN) Maximum number of ecmp paths. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_max_ecmp_get(int unit, int *max_paths)
{
#if defined(BCM_HAWKEYE_SUPPORT) || defined(BCM_HURRICANE_SUPPORT)
    if ( SOC_IS_HAWKEYE(unit) || SOC_IS_HURRICANE(unit)) {
        return (BCM_E_UNAVAIL);
    }	
#endif	

    if (NULL == max_paths) {
        return (BCM_E_PARAM);
    }

    /* Set maximum number of paths. */
    *max_paths = BCM_XGS3_L3_ECMP_MAX_PATHS(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_defip_mpath_add
 * Purpose:
 *      Update multipath route in the hw. 
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      lpm_cfg      - (IN)Route data.
 *      ecmp_group_id  - (IN)New Ecmp group id.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_defip_mpath_add(int unit, _bcm_defip_cfg_t *lpm_cfg, int ecmp_group_id)
{
    int ecmp_index;             /* Index in ecmp table.       */
    int rv = BCM_E_UNAVAIL;     /* Operation return status.   */

    /* Ecmp table index for the allocated group. */
    ecmp_index = ecmp_group_id;
    if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
        ecmp_index = ecmp_group_id * BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
    }

    /* Set zero based number of paths to lpm entry. */
    rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_id, 
                                    &lpm_cfg->defip_ecmp_count);
    if (rv < 0) {
        return rv;
    }
    lpm_cfg->defip_ecmp_count--;

    /*  Add route to the hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_add)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_add) (unit, lpm_cfg, ecmp_index);
    }
    /* Decrement group reference count if operation failed.  */
    if (rv < 0) {
        bcm_xgs3_ecmp_group_del(unit, ecmp_group_id);
        return rv;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_defip_ecmp_write
 * Purpose:
 *      Write route with multipaths to the hw. 
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      lpm_cfg      - (IN)Route data.
 *      ecmp_group   - (IN)Ecmp group.
 *      ecmp_count   - (IN)Count of next hop indexes in ecmp group.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_defip_ecmp_write(int unit, _bcm_defip_cfg_t *lpm_cfg,
                           int *ecmp_group, int ecmp_count)
{
    int ecmp_group_id;          /* New ecmp group index.      */
    int ecmp_index;             /* Index in ecmp table.       */
    int rv = BCM_E_UNAVAIL;     /* Operation return status.   */

    /* Add ecmp group to the hw. */
    rv = _bcm_xgs3_ecmp_group_add(unit, 0, ecmp_count, 
                                  0,
                                  ecmp_group, 
                                  &ecmp_group_id);
    BCM_IF_ERROR_RETURN(rv);

    /* Ecmp table index for the allocated group. */
    ecmp_index = ecmp_group_id;
    if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
        ecmp_index = ecmp_group_id * BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
    }

    /* Set zero based number of paths to lpm entry. */
    rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_id,
                                    &lpm_cfg->defip_ecmp_count);
    if (rv < 0) {
        return rv;
    }
    lpm_cfg->defip_ecmp_count--;

    /*  Add route to the hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_add)) {
          rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_add) (unit, lpm_cfg, ecmp_index);
    }

    if (BCM_FAILURE(rv)) {
        bcm_xgs3_ecmp_group_del(unit, ecmp_group_id);
        return rv;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_defip_ecmp_add
 * Purpose:
 *      IP route with multipaths to DEFIP table
 * Parameters:
 *      unit           - (IN)SOC unit number.
 *      lpm_cfg        - (IN)Route data.
 *      ecmp_group_idx - (IN)Ecmp group index.
 *      nh_index       - (IN)Next hop index. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_defip_ecmp_add(int unit, _bcm_defip_cfg_t *lpm_cfg,
                         int ecmp_group_idx, int nh_index)
{
    int ecmp_max_count;         /* Maximum next hop count in the group.*/
    int ecmp_count;             /* Next hop count in the group.        */
    int *ecmp_grp;              /* Ecmp group from hw.                 */
    int rv;                     /* Operation return status.            */

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

    /*If ecmp group exists just update next hops set. */
    if (BCM_XGS3_L3_INVALID_INDEX != ecmp_group_idx) {

        /* Extract maximum ecmp group size. */
        rv = _bcm_xgs3_ecmp_max_grp_size_get(unit, ecmp_group_idx, &ecmp_max_count);
        if (rv < 0) {
           sal_free(ecmp_grp);
           return rv;
        }

        /* Read ecmp group members from hw. */
        rv = _bcm_xgs3_ecmp_tbl_read(unit, ecmp_group_idx,
                                     ecmp_grp, &ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_grp);
            return (rv);
        }


        /* Enforce max number of multipaths. */
        if (ecmp_count == ecmp_max_count) {
            sal_free(ecmp_grp);
            BCM_XGS3_L3_ERR(("Maximum number of ECMP paths reached\n"));
            return (BCM_E_RESOURCE);
        }
    } else {            /* New route. */
        ecmp_count = 0;
    }

    /* Store new ecmp index in the group. */
    ecmp_grp[ecmp_count] = nh_index;

    /* Write group & route to hardware. */
    rv = _bcm_xgs3_defip_ecmp_write(unit, lpm_cfg, ecmp_grp, (ecmp_count + 1));

    sal_free(ecmp_grp);
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_defip_del
 * Purpose:
 *      Delete IP route with multipaths from DEFIP table
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      lpm_cfg      - (IN)Route data.
 *      nh_idx       - (IN)Next hop/Ecmp group index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_defip_del(int unit, _bcm_defip_cfg_t *lpm_cfg, int nh_idx)
{
    int rv = BCM_E_UNAVAIL;

    /*  Delete route from the hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_del)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_del) (unit, lpm_cfg);
    }
    /* If route deletion successfull decrement next hop reference count. */
    if (rv >= 0) {
        bcm_xgs3_nh_del(unit, 0, nh_idx);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_defip_ecmp_del
 * Purpose:
 *      Delete IP route with multipaths from DEFIP table
 * Parameters:
 *      unit          - (IN)SOC unit number.
 *      lpm_cfg       - (IN)Route data.
 *      ecmp_group_id - (IN)Ecmp group id.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_defip_ecmp_del(int unit, _bcm_defip_cfg_t *lpm_cfg, 
                         int ecmp_group_id)
{
    bcm_l3_egress_t nh_info;    /* Deleted next hop info.             */
    int cmp_result;             /* Next hop comparison result.        */
    int ecmp_count;             /* Next hop count in the group.       */
    int *ecmp_grp;              /* Ecmp group from hw.                */
    uint16 hash;                /* Hash used to find next hop entry.  */
    int idx;                    /* Iteration index.                   */
    int del_nh_index;           /* Matching next hop index.           */
    int rv;                     /* Internal operations status.        */


    /* Input parameters check. */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

    /* Read ecmp group members from the hw. */
    rv = _bcm_xgs3_ecmp_tbl_read(unit, ecmp_group_id,
                                 ecmp_grp, &ecmp_count);
    if (BCM_FAILURE(rv)) {
        sal_free(ecmp_grp);
        return (rv);
    }

    /*
     * If caller didn't specify multipath or group contains a single entry.
     * just delete a whole ecmp group.
     */
    if ((ecmp_count == 1) || (!(lpm_cfg->defip_flags & BCM_L3_MULTIPATH)) ||
        (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        /*  Delete route from the hw. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_del)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_del) (unit, lpm_cfg);
        } else {
            sal_free(ecmp_grp);
            return  (BCM_E_UNAVAIL);
        }

        /* Decrease reference count for all group member nexthops. */
        if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
            _bcm_xgs3_ecmp_group_nh_delete(unit, ecmp_grp, ecmp_count);
        }
    } else {
        /*
         * 1) BCM_L3_MULTIPATH flag is set   &&
         * 2) Next hop information is  present  &&
         * 3)  number of entries > 1. 
         * We need to find next hop index marked for deletion. 
         */
        /* Prepare next hop info. */
        rv = _bcm_xgs3_nh_entry_init(unit, &nh_info, NULL, lpm_cfg);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_grp);
            return (rv);
        }

        /* If no interface information present use customer provided vlan id. */
        if(!nh_info.vlan)  {
            nh_info.vlan = lpm_cfg->defip_vid;
        }

        /* Convert API next hop entry to HW space format. */
        rv = _bcm_xgs3_nh_map_api_data_to_hw(unit, &nh_info);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_grp);
            return (rv);
        }

        /* Calculate entry hash. */
        _bcm_xgs3_nh_hash_calc(unit, (void *)&nh_info, &hash);

        /* Find next hop entry from ecmp group. */
        for (idx = 0; idx < ecmp_count; idx++) {

            /* Compare next hop hash. */
            if (!BCM_XGS3_L3_ENT_HASH_CMP(BCM_XGS3_L3_TBL_PTR(unit, next_hop),
                                          ecmp_grp[idx], hash)) {
                continue;
            }

            /* Compare entire next hop entry. */
            rv = _bcm_xgs3_nh_ent_cmp(unit, (void *)&nh_info,
                                      ecmp_grp[idx], &cmp_result);
            if (BCM_FAILURE(rv)) {
                sal_free(ecmp_grp);
                return (rv);
            }
            if (BCM_L3_CMP_EQUAL == cmp_result) {
                break;
            }                   /* Next hops are identical check. */
        }                       /* Loop over all next hop indexes in the group. */

        /* If we didn't find deleted next hop return an error. */
        if (idx >= ecmp_count) {
            sal_free(ecmp_grp);
            return (BCM_E_PARAM);
        }

        /* Preserve matching next hop. */
        del_nh_index = ecmp_grp[idx];

        /* Decrement ecmp count. */
        ecmp_count--;

        /* Copy last next hop index into the deleted index spot. */
        if (idx < ecmp_count) {
            ecmp_grp[idx] = ecmp_grp[ecmp_count];
        }

        /* Reset last next hop index in the group. */
        ecmp_grp[ecmp_count] = 0;

        /* Write group & route to hardware. */
        rv = _bcm_xgs3_defip_ecmp_write(unit, lpm_cfg, ecmp_grp, ecmp_count);

        /* Release next hop index. */
        bcm_xgs3_nh_del(unit, 0, del_nh_index);
    }
    /* 
     * Decrement original ecmp group reference count, 
     * and possibly delete the group. 
     */
    rv = bcm_xgs3_ecmp_group_del(unit, ecmp_group_id);
    sal_free(ecmp_grp);
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_defip_get
 * Purpose:
 *      Get an entry from DEFIP table.
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Buffer to fill defip information. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_get(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    int nh_ecmp_idx;        /* Next hop/Ecmp group index.          */
    int *ecmp_grp;          /* Ecmp group from hw.                 */
    int nh_idx;             /* Next hop index.                     */
    int rv = BCM_E_UNAVAIL; /* Operation return status.            */

    /* Input parameters check. */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Input parameters checks. */
    BCM_XGS3_L3_DEFIP_INVALID_PARAMS_RETURN(unit, lpm_cfg);

    /* Get lpm entry. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_get)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_get) (unit, lpm_cfg,
                                                     &nh_ecmp_idx);
    } 

    BCM_XGS3_LKUP_IF_ERROR_RETURN(rv, BCM_E_NOT_FOUND);
    /* Translate ecmp index to ecmp group id. */
    if (lpm_cfg->defip_ecmp) {
        if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
            nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        }
    }

    /* If route is multipath get nh info from the first path. */
    if (lpm_cfg->defip_ecmp && (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)))) {
        /* Allocate ecmp group buffer. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

        rv = _bcm_xgs3_ecmp_tbl_read(unit, nh_ecmp_idx,
                                     ecmp_grp, &lpm_cfg->defip_ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_grp);
            return (rv);
        }
        nh_idx = ecmp_grp[0];
        sal_free(ecmp_grp);
    } else {
        nh_idx = nh_ecmp_idx;
    }

    /* Fill next hop info to the prefix. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_lpm_get_nh_info(unit, lpm_cfg, nh_idx));

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_defip_path_validate
 * Purpose:
 *      Service routine used to validate route path for non-egress mode.
 * Parameters:
 *      unit        - (IN) Bcm device number.
 *      new_route    - (IN) Added route information.
 *      old_route   - (IN) Previously existing route. 
 * Returns:
 *      BCM_E_NONE - Route path is valid.
 *      BCM_E_XXX  - Otherwise.
 */
int
bcm_xgs3_defip_path_validate(int unit, _bcm_defip_cfg_t *new_route,
                             _bcm_defip_cfg_t *old_route)
{
    /* Validate trunk id. */
    BCM_XGS3_L3_IF_INVALID_TGID_RETURN(unit, new_route->defip_flags,
                                       new_route->defip_port_tgid);

    if (new_route->defip_flags & BCM_L3_MULTIPATH) {
        /* Make sure device supports ecmp forwarding. */
        if (0 == BCM_XGS3_L3_ECMP_MAX_PATHS(unit)) {
            return (BCM_E_UNAVAIL); 
        }

        /* Trap to cpu is not allowed as member of ecmp group. */
        if (new_route->defip_flags & BCM_L3_DEFIP_LOCAL) {
            return (BCM_E_PARAM);
        }
    }

    /* If it is not addition of new path to a multipath route. */
    /* BCM_L3_REPLACE flag must be set.                        */
    if (NULL != old_route) {
        if ((!(old_route->defip_flags & BCM_L3_MULTIPATH)) || 
            (!(new_route->defip_flags & BCM_L3_MULTIPATH))) {
            if (!(new_route->defip_flags & BCM_L3_REPLACE)) {
                return (BCM_E_EXISTS);
            }
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_defip_add
 * Purpose:
 *      Add IP route to DEFIP table
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Added route information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_add(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_defip_cfg_t entry;                      /* Lookup buffer.            */
    _bcm_defip_cfg_t *old_route;                 /* Lookup result pointer.    */
    int nh_idx;                                  /* Next hop index.           */
    int mpath;                                   /* New route is multipath.   */
    int nh_remove;                               /* Flag to remove mpath nhops*/
    int rv = BCM_E_UNAVAIL;                      /* Operation return status.  */
    int lkup_result = BCM_E_UNAVAIL;             /* Lookup result.            */
    int nh_ecmp_idx = BCM_XGS3_L3_INVALID_INDEX; /* Next hop/Ecmp group index.*/
    int old_ecmp_grp = BCM_XGS3_L3_INVALID_INDEX;/* Original route ecmp group.*/

    /* Route lookup key Subnet sanity check. */
    BCM_XGS3_L3_DEFIP_INVALID_PARAMS_RETURN(unit, lpm_cfg);

    /* Set lookup key. */
    entry = *lpm_cfg;
    mpath = lpm_cfg->defip_flags & BCM_L3_MULTIPATH;

    /* Check if route exists. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_get)) {
        lkup_result = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_get) (unit, &entry,
                                                              &nh_ecmp_idx);
    }

    if (BCM_SUCCESS(lkup_result)) {
        /* Save route entry index in hw to indicate  */
        /* existing entry replacement.               */
        lpm_cfg->defip_index = entry.defip_index;
        old_route = &entry;
        /* eXTRACT ecmp group group only if original route was multipath.*/
        if (old_route->defip_flags & BCM_L3_MULTIPATH) {
            /* Trunslate next hop index to ecmp group id. */
            if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
                nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
            }
            old_ecmp_grp = nh_ecmp_idx;
        }
    } else if (lkup_result == BCM_E_NOT_FOUND){
        /* In case of host replacement - make sure host was present. */ 
        if ((lpm_cfg->defip_flags & BCM_L3_REPLACE) && 
            (lpm_cfg->defip_flags & BCM_L3_HOST_AS_ROUTE)) {
            return (lkup_result);
        }

        /* New route. */
        lpm_cfg->defip_index = BCM_XGS3_L3_INVALID_INDEX;   
        old_route = NULL;
    } else {
        return lkup_result;
    }

    if (BCM_GPORT_IS_BLACK_HOLE(lpm_cfg->defip_port_tgid)) {
         nh_idx = 0;
    } else {
         /* Validate forwarding path. */
         if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
              /* No path sanity checks for egress mode,         */
              /* Path sanity done during egress object addition.*/
              BCM_IF_ERROR_RETURN
                   (bcm_xgs3_defip_path_validate(unit, lpm_cfg, old_route));
         } else if (BCM_SUCCESS(lkup_result)){
              if (!(lpm_cfg->defip_flags & BCM_L3_REPLACE)) {
                   return (BCM_E_EXISTS);
              }
         }
         /* Get next hop index. */
         BCM_IF_ERROR_RETURN(_bcm_xgs3_nh_init_add(unit, NULL, lpm_cfg, &nh_idx));
    }

    /*
     * If adding new paths to ECMP route, add new paths
     * and update ecmp count in LPM table.
     */
    if (mpath) {
        /* For egress switching mode just set the route entry. */
        if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
           /* In egress mode increment path reference & update the route.*/ 
           rv = _bcm_xgs3_defip_mpath_add(unit, lpm_cfg, nh_idx);
        } else {
           if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
               /* For non egress switching mode create ecmp group and route.*/ 
               rv = _bcm_xgs3_defip_ecmp_add(unit, lpm_cfg, old_ecmp_grp, nh_idx);
           } else {
               return BCM_E_UNAVAIL;
           }
        }
    } else {
        /*  Add route to the hw. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_add)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_add) (unit, lpm_cfg, nh_idx);
        }
    }

    /* In case of route update decrement original path/group reference count. */
    if ((BCM_SUCCESS(lkup_result)) && (BCM_SUCCESS(rv))) {
        if(old_route->defip_flags & BCM_L3_MULTIPATH) {
            /* Destroy ecmp group. Keep member next hops if new route is */
            /* multipath as well, otherwise remove them.                 */
            /* Never touch next hops attched to an egress object.        */   
            nh_remove = (!(mpath || BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)));
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_ecmp_group_remove(unit, nh_ecmp_idx, nh_remove)); 
        } else {
            BCM_IF_ERROR_RETURN(bcm_xgs3_nh_del(unit, 0, nh_ecmp_idx));
        }
    }

    /* Regardless of status don't touch egress object multipath routes. */
    if (mpath && BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
        return (rv);
    }
    
    /* If route addition failed decrement next hop reference count. */
    if (BCM_FAILURE(rv)) {
        /* Free next hop entry. */
        bcm_xgs3_nh_del(unit, 0, nh_idx);
    }
    return (rv);
}

/*
 * Function:
 *      bcm_xgs3_defip_del
 * Purpose:
 *      Delete IP route from DEFIP table
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Deleted route information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_defip_cfg_t entry;     /* Lookup buffer.            */
    int rv;                     /* Operation return status.  */
    int nh_ecmp_idx = BCM_XGS3_L3_INVALID_INDEX;        /* Next hop/Ecmp group index. */
    int lkup_result = BCM_E_UNAVAIL;    /* Lookup result.            */

    /* Input parameters checks. */
    BCM_XGS3_L3_DEFIP_INVALID_PARAMS_RETURN(unit, lpm_cfg);

    /* Port trunk verification. */
    if (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) { 
        BCM_XGS3_L3_IF_INVALID_TGID_RETURN(unit, lpm_cfg->defip_flags,
                                           lpm_cfg->defip_port_tgid);
    }

    /* Set lookup key. */
    entry = *lpm_cfg;

    /* Check if route exists. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_get)) {
        lkup_result = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_get) (unit, &entry,
                                                              &nh_ecmp_idx);
    }
    if (lkup_result < 0) {
        return lkup_result;
    } 

    /* Remove path from hw. */
    if (entry.defip_flags & BCM_L3_MULTIPATH) {
        /* Trunslate next hop index to ecmp group id. */
        if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
            nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        }
        rv = _bcm_xgs3_defip_ecmp_del(unit, lpm_cfg, nh_ecmp_idx);
    } else {
        rv = _bcm_xgs3_defip_del(unit, lpm_cfg, nh_ecmp_idx);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_defip_intf_del_test_cb
 * Purpose:
 *      Test entries in DEFIP table by intf match.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern (interface id & negate).
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_intf_del_test_cb(int unit, void *pattern,
                                 void *data1, void *data2, int *cmp_result)
{
    _bcm_l3_trvrs_data_t *trv_data;     /* Travers data.              */
    _bcm_if_del_pattern_t *if_info;     /* Interface id/negate info.  */
    _bcm_defip_cfg_t *lpm_cfg;          /* Route information.         */
    int if_cmp_result;                  /* Interface compare result.  */
    int nh_ecmp_idx;                    /* Next hop/ecmp group index. */
    int ecmp_count;                     /* Ecmp group size.           */
    int *ecmp_grp;                      /* Ecmp group from hw.        */
    int idx;                            /* Iteration index.           */
    int rv;                             /* Operation return status.   */

    /* Cast input parameters. */
    trv_data = (_bcm_l3_trvrs_data_t *) pattern;
    if_info = (_bcm_if_del_pattern_t *) trv_data->pattern;
    lpm_cfg = (_bcm_defip_cfg_t *) data1;
    nh_ecmp_idx = *(int *)data2;


    if (lpm_cfg->defip_ecmp) {
        /* Translate next hop index to ecmp group id. */
        if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
            nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        }

        /* Allocate ecmp group buffer. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

        /* If route is multipath get next hops from ecmp group. */
         rv = _bcm_xgs3_ecmp_tbl_read(unit, nh_ecmp_idx,
                                     ecmp_grp, &ecmp_count);
         if (BCM_FAILURE(rv)) {
             sal_free(ecmp_grp);
             return (rv);
         }

        /* Loop over all ecmp group members and check outgoing interface. */
        for (idx = 0; idx < ecmp_count; idx++) {
            rv = _bcm_xgs3_l3_nh_intf_cmp(unit, if_info->l3_intf,
                                          ecmp_grp[idx], &if_cmp_result);
            if (BCM_FAILURE(rv)) {
                sal_free(ecmp_grp);
                return (rv);
            }
            if (BCM_L3_CMP_EQUAL == *cmp_result) {
                break;
            }
        }                       /* Loop over ecmp group next hops. */
        sal_free(ecmp_grp);
    } else {
        /* Check next hop outgoing interface. */
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_l3_nh_intf_cmp(unit, if_info->l3_intf,
                                      nh_ecmp_idx, &if_cmp_result));
    }

    /* Calculate final result based on negate and interface index comparison. */
    if (BCM_XGS3_L3_IS_EQUAL(if_cmp_result, if_info->negate)) {
        *cmp_result = BCM_L3_CMP_EQUAL;
    } else {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_defip_intf_del_op_cb
 * Purpose:
 *      Delete entries in DEFIP table by intf match.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern (interface id & negate).
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_intf_del_op_cb(int unit, void *pattern,
                               void *data1, void *data2, int *cmp_result)
{
    _bcm_l3_trvrs_data_t *trv_data;     /* Travers data.              */
    _bcm_if_del_pattern_t *if_info;     /* Interface id/negate info.  */
    _bcm_defip_cfg_t *lpm_cfg;          /* Route information.         */
    int if_cmp_result;                  /* Interface compare result.  */
    int nh_ecmp_idx;                    /* Next hop/ecmp group index. */
    int ecmp_count;                     /* Ecmp group next hops count.*/
    int *ecmp_grp;                      /* Ecmp group from hw.        */
    int idx;                            /* Iteration index.           */
    int rv;                             /* Operation return status.   */
    int new_ecmp_count = 0;             /* New ecmp group nh count.   */

    /* Cast input parameters. */
    trv_data = (_bcm_l3_trvrs_data_t *) pattern;
    if_info = (_bcm_if_del_pattern_t *) trv_data->pattern;
    lpm_cfg = (_bcm_defip_cfg_t *) data1;
    nh_ecmp_idx = *(int *)data2;


    if (lpm_cfg->defip_ecmp) {  /* Multipath. */
        /* Translate next hop index to ecmp group id. */
        if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
            nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        }

        /* Allocate ecmp group buffer. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_grp));

        /* If route is multipath get next hops from ecmp group. */
        rv = _bcm_xgs3_ecmp_tbl_read(unit, nh_ecmp_idx,
                                     ecmp_grp, &ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_grp);
            return (rv);
        }

        /* Loop over all ecmp group members and check outgoing interface. */
        for (idx = 0; idx < ecmp_count; idx++) {
            rv = _bcm_xgs3_l3_nh_intf_cmp(unit, if_info->l3_intf,
                                          ecmp_grp[idx], &if_cmp_result);
            if (BCM_FAILURE(rv)) {
                sal_free(ecmp_grp);
                return (rv);
            }

            /* Update ecmp group based on comparison result. */
            if (BCM_XGS3_L3_IS_EQUAL(if_cmp_result, if_info->negate)) {
                /* Overrite the entry with last group member. */
                ecmp_grp[idx] = ecmp_grp[ecmp_count - 1];
                BCM_XGS3_L3_DEC_IF_POSITIVE(ecmp_count);
                BCM_XGS3_L3_DEC_IF_POSITIVE(idx);
            } else {
                new_ecmp_count++;
            }
        }

        /* If no paths left or at least one interface matches in 
           l3 egress switching mode, delete the route. */
        if ((!new_ecmp_count) ||
            ((new_ecmp_count != ecmp_count) && 
             (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)))){
            /* Unset multipath flag to indicate complete route deletion. */
            lpm_cfg->defip_flags &= ~BCM_L3_MULTIPATH;
            sal_free(ecmp_grp);
            return _bcm_xgs3_defip_ecmp_del(unit, lpm_cfg, nh_ecmp_idx);
        } else if (new_ecmp_count != ecmp_count) {
            /* Next hops array changed rewrite ecmp group. */
            rv = _bcm_xgs3_defip_ecmp_write(unit, lpm_cfg, ecmp_grp,
                                            new_ecmp_count);
            sal_free(ecmp_grp);
            BCM_IF_ERROR_RETURN(rv);

            /* Free original ecmp group. */
            BCM_IF_ERROR_RETURN(bcm_xgs3_ecmp_group_del(unit, nh_ecmp_idx));
        } else {
            sal_free(ecmp_grp);
            /* Else nothing changed don't touch the entry. */
        }
    } else {                    /* Single path. */
        /* Check next hop outgoing interface. */
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_l3_nh_intf_cmp(unit, if_info->l3_intf,
                                      nh_ecmp_idx, &if_cmp_result));
        /* Based on comparison & negate decide to delete an entry or not. */
        if (BCM_XGS3_L3_IS_EQUAL(if_cmp_result, if_info->negate)) {
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_defip_del(unit, lpm_cfg, nh_ecmp_idx));
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_ip4_defip_del_intf
 * Purpose:
 *      Delete entries in DEFIP table by intf match.
 * Parameters:
 *      unit - SOC unit number.
 *      defip - Pointer to memory for DEFIP table related information.
 *      negate - 0 means interface match; 1 means not match
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_del_intf(int unit, _bcm_defip_cfg_t *lpm_cfg, int negate)
{
    _bcm_l3_trvrs_data_t trv_data;     /* Travers initiator function input. */
    _bcm_if_del_pattern_t pattern;      /* Interface & negate combination 
                                           to be used for comparision.      */
    int rv = BCM_E_UNAVAIL;             /* Operation return value.          */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* If table is empty -> there is nothing to delete. */
    BCM_XGS3_L3_IF_DEFIP_CNT_ZERO_RETURN
        (unit, (lpm_cfg->defip_flags & BCM_L3_IP6));

    /* Zero traverse info. */
    sal_memset(&trv_data, 0, sizeof(_bcm_l3_trvrs_data_t));

    /* Set interface match pattern (interface index + negate) */
    pattern.l3_intf = lpm_cfg->defip_intf;
    pattern.negate = negate;

    /* Fill traverse data struct. */
    trv_data.flags = lpm_cfg->defip_flags;
    trv_data.pattern = (void *)&pattern;
    trv_data.cmp_cb = _bcm_xgs3_defip_intf_del_test_cb;
    trv_data.op_cb = _bcm_xgs3_defip_intf_del_op_cb;

    /*  Delete routes matching the pattern from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_update_match)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_update_match) (unit, &trv_data);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_defip_del_all_op_cb
 * Purpose:
 *      Delete all entries in DEFIP table. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern.
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_del_all_op_cb(int unit, void *pattern,
                              void *data1, void *data2, int *cmp_result)
{
    _bcm_defip_cfg_t *lpm_cfg;  /* Route information.         */
    int nh_ecmp_idx;            /* Next hop/ecmp group index. */
    int rv;                     /* Operation return status.   */

    /* Cast input parameters. */
    lpm_cfg = (_bcm_defip_cfg_t *) data1;
    nh_ecmp_idx = *(int *)data2;

    if (lpm_cfg->defip_ecmp) {
        /* Translate next hop index to ecmp group id. */
        if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
            nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        }
        /*
         * Reset multipath flag so the whole entry and not a specific path
         * will be deleted. 
         */
        lpm_cfg->defip_flags &= ~BCM_L3_MULTIPATH;
        rv = _bcm_xgs3_defip_ecmp_del(unit, lpm_cfg, nh_ecmp_idx);
    } else {
        rv = _bcm_xgs3_defip_del(unit, lpm_cfg, nh_ecmp_idx);
    }
    return rv;
}

#if defined(BCM_TRIUMPH_SUPPORT)
/*
 * Function:
 *      bcm_xgs3_defip_verify_internal_mem_usage
 * Purpose:
 *      Verify if entries are present within INT-DEFIP table
 * Parameters:
 *      unit  - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_xgs3_defip_verify_internal_mem_usage(int unit)
{

  if (soc_feature(unit, soc_feature_esm_support)) {
    if (soc_mem_index_count(unit, L3_DEFIPm)) {
      if (BCM_XGS3_L3_DEFIP_IP4_CNT(unit)) {
        return BCM_E_DISABLED;
      }

      if (BCM_XGS3_L3_DEFIP_IP6_CNT(unit)) {
        return BCM_E_DISABLED;
      }

    }
    BCM_IF_ERROR_RETURN(_bcm_trx_l3_defip_verify_internal_mem_usage(unit));
  }
  return BCM_E_NONE;
}
#endif

/*
 * Function:
 *      bcm_xgs3_defip_del_all
 * Purpose:
 *      Delete all entries in DEFIP table
 * Parameters:
 *      unit  - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_del_all(int unit)
{
    _bcm_l3_trvrs_data_t trv_data;  /* Travers initiator parameters. */
    int tmp_rv;                     /* First error occured.          */
    int rv;                         /* Operation return status.      */

    tmp_rv = rv = BCM_E_NONE;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_update_match)) {
        return (BCM_E_UNAVAIL);
    }

    /* Zero traverse info. */
    sal_memset(&trv_data, 0, sizeof(_bcm_l3_trvrs_data_t));

    /* Fill traverse data struct. */
    trv_data.op_cb = _bcm_xgs3_defip_del_all_op_cb;

    /*  Delete ipv4 routes matching the pattern from hw. */
    if (BCM_XGS3_L3_DEFIP_IP4_CNT(unit)) {
        trv_data.flags = 0;
        tmp_rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_update_match) (unit,
                                                                  &trv_data);
    } 

    /*  Delete ipv6 routes matching the pattern from hw. */
    if (BCM_XGS3_L3_DEFIP_IP6_CNT(unit)) {
        trv_data.flags = BCM_L3_IP6;
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_update_match) (unit,
                                                              &trv_data);
    }
    return (tmp_rv < 0) ? tmp_rv : rv;
}

/*
 * Function:
 *      _bcm_xgs3_defip_age_test_cb
 * Purpose:
 *      Test hit bit in DEFIP entries.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern.
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_age_test_cb(int unit, void *pattern, void *data1,
                            void *data2, int *cmp_result)
{
    _bcm_defip_cfg_t *lpm_cfg;          /* Route information.        */

    /* Cast input parameters. */
    lpm_cfg = (_bcm_defip_cfg_t *) data1;

    if (lpm_cfg->defip_flags & BCM_L3_HIT) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
    } else {
        *cmp_result = BCM_L3_CMP_EQUAL;
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_defip_age_delete_cb
 * Purpose:
 *      Delete unused entries in DEFIP table 
 *      and notify the user.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern.
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_age_delete_cb(int unit, void *pattern, void *data1,
                       void *data2, int *cmp_result)
{
    _bcm_defip_cfg_t *lpm_cfg;          /* Route information.        */
    _bcm_l3_trvrs_data_t *trv_data;    /* Travers data.             */
    int nh_ecmp_idx;                    /* Next hop/Ecmp group index.*/
    int rv = BCM_E_NONE;                /* Operation return status.  */

    /* Cast input parameters. */
    trv_data = (_bcm_l3_trvrs_data_t *) pattern;
    lpm_cfg = (_bcm_defip_cfg_t *) data1;
    nh_ecmp_idx = *(int *)data2;

    if (!(lpm_cfg->defip_flags & BCM_L3_HIT)) {
        /* Remove path from hw. */
        if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
            /* Unset multipath flag -> so all route path's can be deleted. */
            lpm_cfg->defip_flags &= ~BCM_L3_MULTIPATH;
            rv = _bcm_xgs3_defip_ecmp_del(unit, lpm_cfg, nh_ecmp_idx);
        } else {
            rv = _bcm_xgs3_defip_del(unit, lpm_cfg, nh_ecmp_idx);
        }
        /* Notify caller regarding unused entry. */
        _bcm_xgs3_lpm_call_user_cb(unit, trv_data, lpm_cfg,
                                   BCM_XGS3_L3_INVALID_INDEX);
    }
    return (rv);
}


/*
 * Function:
 *      _bcm_xgs3_defip_age_clear_hit_cb
 * Purpose:
 *      Age entries in DEFIP table and notify the user.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern.
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result.
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_age_clear_hit_cb(int unit, void *pattern, void *data1,
                                 void *data2, int *cmp_result)
{
    _bcm_defip_cfg_t *lpm_cfg;          /* Route information.        */
    int rv;                             /* Operation return status.  */

    /* Comparison is always false - since no action required. */
    *cmp_result = BCM_L3_CMP_NOT_EQUAL;

    /* Cast input parameters. */
    lpm_cfg =  (_bcm_defip_cfg_t *) data1;

    if (lpm_cfg->defip_flags & BCM_L3_HIT) {
        lpm_cfg->defip_flags |= BCM_L3_HIT_CLEAR;

        /* Clear hit bit. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_get)) {
            rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_get) (unit, lpm_cfg, NULL);
            if (rv < 0) {
                return rv;
            }
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *    bcm_xgs3_defip_age
 * Purpose:
 *    Age out the DEFIP table
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      flags     - (IN)IPv4/IPv6.     
 *      age_out   - (IN)age_out Notification callback. 
 *      user_data - (IN)User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_defip_age(int unit, int flags, bcm_l3_route_traverse_cb age_out,
                    void *user_data)
{
    _bcm_l3_trvrs_data_t trv_data;     /* Travers initiator parameters. */
    int rv = BCM_E_UNAVAIL;             /* Operation return status.      */

    /* If table is empty -> there is nothing to delete. */
    BCM_XGS3_L3_IF_DEFIP_CNT_ZERO_RETURN(unit, flags & BCM_L3_IP6);

    /* Zero traverse info. */
    sal_memset(&trv_data, 0, sizeof(_bcm_l3_trvrs_data_t));

    /* Fill traverse data struct. */
    trv_data.flags = flags;
    /* We need to set both operational & test callback since firebolt 
       never executes test routine. */
    trv_data.cmp_cb = _bcm_xgs3_defip_age_test_cb;
    trv_data.op_cb = _bcm_xgs3_defip_age_delete_cb;
    trv_data.defip_cb = age_out;
    trv_data.cookie   = user_data;

    /*  Age routes matching the pattern from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_update_match)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_update_match) (unit, &trv_data);
    }

    /*  Clear hit bit for the rest of the routes. */
    trv_data.cmp_cb = _bcm_xgs3_defip_age_clear_hit_cb;
    trv_data.op_cb = _bcm_xgs3_defip_age_clear_hit_cb;
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_update_match)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_update_match) (unit, &trv_data);
    }
    return rv;
}

/*
 * Function:
 *    bcm_xgs3_defip_age
 * Purpose:
 *    Age out DEFIP table
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      age_out  - (IN)age_out Notification callback. 
 *      user_data - (IN)User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_age(int unit, bcm_l3_route_traverse_cb age_out, void *user_data)
{
    int rv;                  /* Operation return status. */
    int tmp_rv;              /* First error occured.     */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Age all ipv4 entries. */
    tmp_rv = _bcm_xgs3_defip_age(unit, 0, age_out, user_data);

    /* Age all ipv6 entries. */
    rv = _bcm_xgs3_defip_age(unit, BCM_L3_IP6, age_out, user_data);

    return (tmp_rv < 0) ? tmp_rv : rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_xgs3_defip_state_recover
 * Purpose:
 *      Recover route table references & usage counters 
 *      after warm reboot. 
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Route info.
 *      nh_ecmp_idx - (IN)Next hop /Ecmp group index. 
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_state_recover(int unit, _bcm_defip_cfg_t *lpm_cfg, int
                              nh_ecmp_idx)
                           
{
    int *ecmp_group;                        /* Ecmp group from hw.      */
    int ecmp_count;                         /* Ecmp group size.         */ 
    _bcm_l3_tbl_t *nh_tbl_ptr;              /* Next hop table ptr.      */    
    _bcm_l3_tbl_t *ecmp_tbl_ptr;            /* ECMP table ptr.          */
    int rv;                                 /* Operation return status. */

    /* Get table pointers */
    nh_tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);
    ecmp_tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);

    /* Allocate ecmp group buffer. */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_group));

    /* Update defip book-keeping info */
    if ((lpm_cfg->defip_flags & BCM_L3_IP6) == BCM_L3_IP6) {
        BCM_XGS3_L3_DEFIP_IP6_CNT(unit)++;	
    } else {
        BCM_XGS3_L3_DEFIP_IP4_CNT(unit)++;
    }

    if (lpm_cfg->defip_ecmp) {
         /* Translate ecmp index to ecmp group id. */
         if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
              nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
         }

        /* Increment ecmp group reference count. */
        BCM_XGS3_L3_ENT_REF_CNT_INC(ecmp_tbl_ptr, nh_ecmp_idx, 
                                    _BCM_SINGLE_WIDE);

        /* If route is multipath get next hops from ecmp group. */
        rv = _bcm_xgs3_ecmp_tbl_read(unit, nh_ecmp_idx,
                                     ecmp_group, &ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_group);
            return (rv);
        }

        /* Set ecmp in use flag if needed. */
        if (!BCM_XGS3_L3_ECMP_IN_USE(unit)) {
            BCM_XGS3_L3_ECMP_IN_USE(unit) = TRUE;
        }

        /* Compute signature and reference count for ECMP group */
        _bcm_xgs3_ecmp_grp_hash_calc(unit, (void *)ecmp_group, 
                             &BCM_XGS3_L3_ENT_HASH(ecmp_tbl_ptr, nh_ecmp_idx));

    } else {  /* Unipath route. */
        /* Set reference count to nh_hop table entry */
        BCM_XGS3_L3_ENT_REF_CNT_INC(nh_tbl_ptr, nh_ecmp_idx, 
                                    _BCM_SINGLE_WIDE);
    }
    sal_free(ecmp_group);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_ecmp_reinit
 * Purpose:
 *      Recover basic ECMP groups table info
 * Parameters:
 *      unit  - SOC unit number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_ecmp_reinit(int unit)
{
    int index_min;                /* First entry index.            */
    int index_max;                /* Last  entry index.            */
    int entry;			  /* Buffer.                       */
    _bcm_l3_tbl_t *nh_tbl_ptr;              /* Next hop table ptr.      */    
    _bcm_l3_tbl_t *ecmp_tbl_ptr;            /* ECMP table ptr.          */
    int *ecmp_group;
    int rv=0, ecmp_count=0;
    int nh_idx=0, grp_idx=0, grp_max_idx=0;
    int ecmp_max_paths=0;
#if defined(BCM_HAWKEYE_SUPPORT)
    if (SOC_IS_HAWKEYE(unit)) {
        BCM_XGS3_L3_ECMP_TBL_SIZE(unit) = 0;
        BCM_XGS3_L3_ECMP_MAX_PATHS(unit) = 0;
        return BCM_E_NONE;
    }
#endif
    /* Get table pointers */
    nh_tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);
    ecmp_tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);

    /* Get ECMP table start & end index. */
    ecmp_tbl_ptr->idx_min = index_min =
	   soc_mem_index_min(unit, BCM_XGS3_L3_MEM(unit, ecmp));
    ecmp_tbl_ptr->idx_max = index_max =
	   soc_mem_index_max(unit, BCM_XGS3_L3_MEM(unit, ecmp));
    BCM_XGS3_L3_ECMP_TBL_SIZE(unit) = index_max - index_min + 1;

    /* Get maximum number of ECMP paths (stored in last entry) */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, 
			BCM_XGS3_L3_MEM(unit, ecmp), index_max, &entry));
    if ( (ecmp_max_paths = soc_L3_ECMPm_field32_get(unit, &entry, NEXT_HOPf)) 
         != 0) {
         BCM_XGS3_L3_ECMP_MAX_PATHS(unit) = ecmp_max_paths;
    }

    /* Initialize the number of ECMP groups */	
    ecmp_tbl_ptr->idx_max = BCM_XGS3_L3_ECMP_TBL_SIZE(unit) / 
	    	       BCM_XGS3_L3_ECMP_MAX_PATHS(unit) - 1;
    
    grp_max_idx = ecmp_tbl_ptr->idx_max;

    if (soc_feature(unit, soc_feature_l3_dynamic_ecmp_group) && 
        (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
        /* Here, the max possible groups (and max group index) is limited by 
         * the size of L3_ECMP_COUNTm table. To support dynamic
         * per-ecmp-group size, two entries are added to the L3_ECMP_COUNTm
         * table per group:
         *
         * L3_ECMP_COUNTm Table:
         *        Group # Entry idx   COUNTf
         *          0 ------> 0     Current ecmp path count for group 0.
         *                    1     Max size for group 0.
         *          2 ------> 2     Current ecmp path count for group 2.
         *                    3     Max size for group 2.
         *         ...so on...
         *  (Tbl size - 2)-->(Tbl size-2)  Current path count for this group.
         *                   (Tbl size-1)  Max size for this group.
         * */
        grp_max_idx = soc_mem_index_count(unit, L3_ECMP_COUNTm) - 2;
    }

    ecmp_tbl_ptr->idx_maxused = ecmp_tbl_ptr->idx_min;

    /* Allocate ecmp group buffer */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_group));

    for (grp_idx = ecmp_tbl_ptr->idx_min; grp_idx <= grp_max_idx; grp_idx++) {

         /* Increment ECMP group reference count. */
         BCM_XGS3_L3_ENT_REF_CNT_INC(ecmp_tbl_ptr, grp_idx, _BCM_SINGLE_WIDE);

         /* Get next hops from ECMP group */
         rv = _bcm_xgs3_ecmp_tbl_read(unit, grp_idx,
                                                                ecmp_group, &ecmp_count);
         if (BCM_FAILURE(rv)) {
              sal_free(ecmp_group);
             return (rv);
         }

         if (ecmp_count < 1) {
              BCM_XGS3_L3_ENT_REF_CNT_DEC(ecmp_tbl_ptr, grp_idx, _BCM_SINGLE_WIDE);
              continue;
         }
         /* Set ecmp in use flag if needed. */
         if (!BCM_XGS3_L3_ECMP_IN_USE(unit)) {
              BCM_XGS3_L3_ECMP_IN_USE(unit) = TRUE;
         }

         /* Compute signature and reference count for ECMP group */
         _bcm_xgs3_ecmp_grp_hash_calc(unit, (void *)ecmp_group, 
                                      &BCM_XGS3_L3_ENT_HASH(ecmp_tbl_ptr, grp_idx));

         /* Loop over all ecmp group members and increment nh reference count.*/
         for (nh_idx = 0; nh_idx < ecmp_count; nh_idx++) {
              BCM_XGS3_L3_ENT_REF_CNT_INC(nh_tbl_ptr, ecmp_group[nh_idx],
                                                                                     _BCM_SINGLE_WIDE);
         }
         ecmp_tbl_ptr->idx_maxused ++;
    }		 
    sal_free(ecmp_group);
    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */   

/*
 * Function:
 *      _bcm_xgs3_defip_traverse_cb
 * Purpose:
 *      Walk entries in DEFIP table.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      pattern   - (IN)Match pattern. 
 *      data1     - (IN)Route info.
 *      data2     - (IN)Next hop info.
 *      cmp_result- (OUT)Comparison result (Always negative).
 * Returns:
 *      BCM_E_XXX  
 */
STATIC int
_bcm_xgs3_defip_traverse_cb(int unit, void *pattern,
                            void *data1, void *data2, int *cmp_result)
{
    _bcm_l3_trvrs_data_t *trv_data;         /* Traverse data.               */
    _bcm_trvs_range_t *range_info;          /* Travers start/end/count info.*/
    _bcm_defip_cfg_t *lpm_cfg;              /* Route information.           */
    int nh_ecmp_idx;                        /* Next hop/ecmp group index.   */
    int ecmp_count;                         /* Ecmp group size.             */ 
    int *ecmp_group;                        /* Ecmp group from hw.          */
    int idx;                                /* Iteration index.             */
    int rv;                                 /* Operation return status.     */

    /* Cast input parameters. */
    trv_data = (_bcm_l3_trvrs_data_t *) pattern;
    range_info = (_bcm_trvs_range_t *) trv_data->pattern;
    lpm_cfg = (_bcm_defip_cfg_t *) data1;
    nh_ecmp_idx = *(int *)data2;

    /* We never need to call operational callback. */
    *cmp_result = BCM_L3_CMP_NOT_EQUAL;

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        return _bcm_xgs3_defip_state_recover(unit, lpm_cfg, nh_ecmp_idx);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */


    if ((lpm_cfg->defip_ecmp) && 
        (!(BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)))) {
        /* Allocate ecmp group buffer. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_group));

        /* If route is multipath get next hops from ecmp group. */
        rv = _bcm_xgs3_ecmp_tbl_read(unit, nh_ecmp_idx, 
                                     ecmp_group, &ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_group);
            return (rv);
        }

        /* Set number of ecmp paths. */
        lpm_cfg->defip_ecmp_count = ecmp_count; 

        /* Loop over all ecmp group members and check outgoing interface. */
        for (idx = 0; idx < ecmp_count; idx++) {
            /* Check number of routes reported. */
            if (range_info->start > range_info->current_count) {
                continue;
            }
            if (range_info->end < range_info->current_count) {
                sal_free(ecmp_group);
                return (BCM_E_FULL);
            }
            range_info->current_count++;

            /* Notify caller regarding entry path. */
            _bcm_xgs3_lpm_call_user_cb(unit, trv_data,
                                       lpm_cfg, ecmp_group[idx]);

        } /* Loop over ecmp group next hops. */
        sal_free(ecmp_group);
    } else {
        if (range_info->start > range_info->current_count) {
            return (BCM_E_NONE);
        }

        if (range_info->end < range_info->current_count) {
            return (BCM_E_FULL);
        }
        range_info->current_count++;

        /* Notify caller regarding unused entry. */
        _bcm_xgs3_lpm_call_user_cb(unit, trv_data, lpm_cfg, nh_ecmp_idx);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_defip_traverse
 * Purpose:
 *      Traverse entries in the DEF_IP table
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      flags     - (IN)IPv4/IPv6.     
 *      start     - (IN)Start index of interest.
 *      end       - (IN)End index of interest.
 *      trav_fn   - (IN)Callback function for every entry.
 *      user_data - (IN)User callback data.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_traverse(int unit, int flags, uint32 start, uint32 end,
                        bcm_l3_route_traverse_cb trav_fn, void *user_data)
{
    _bcm_l3_trvrs_data_t trv_data;     /* Traverse information.        */
    _bcm_trvs_range_t range_info;       /* Start/End/Count information. */
    int rv = BCM_E_UNAVAIL;             /* Operation return status.     */

    if (!SOC_WARM_BOOT(unit)) {
	/* If table is empty -> there is nothing to delete. */
	BCM_XGS3_L3_IF_DEFIP_CNT_ZERO_RETURN(unit, flags & BCM_L3_IP6);
    }

    /* Zero traverse info. */
    sal_memset(&trv_data, 0, sizeof(_bcm_l3_trvrs_data_t));
    sal_memset(&range_info, 0, sizeof(_bcm_trvs_range_t));

    range_info.start = start;
    range_info.end = end;

    /* Fill traverse data struct. */
    trv_data.flags = flags;
    trv_data.pattern = &range_info;
    /* We need to set both operational & test callback since firebolt 
       never executes test routine. */
    trv_data.cmp_cb = _bcm_xgs3_defip_traverse_cb;
    trv_data.op_cb = _bcm_xgs3_defip_traverse_cb;
    trv_data.defip_cb = trav_fn;
    trv_data.cookie = user_data;

    /*  Update/show routes matching the pattern from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_update_match)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_update_match) (unit, &trv_data);
    }
    /* Handle case we got to the last requested index (end) callback 
       will return table full, no error in this case.
     */
    rv = (rv == BCM_E_FULL) ? BCM_E_NONE : rv;
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_defip_ip4_traverse
 * Purpose:
 *      Traverse IPv4 entries in the DEF_IP table
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      start     - (IN)Start index of interest.
 *      end       - (IN)End index of interest.
 *      trav_fn   - (IN)Callback function for every entry.
 *      user_data - (IN)User callback data.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_ip4_traverse(int unit, uint32 start, uint32 end,
                            bcm_l3_route_traverse_cb trav_fn, void *user_data)
{
    return bcm_xgs3_defip_traverse(unit, 0, start, end, trav_fn, user_data);
}

/*
 * Function:
 *      bcm_xgs3_defip_ip6_traverse
 * Purpose:
 *      Traverse IPv6 entries in the DEF_IP table
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      start     - (IN)Start index of interest.
 *      end       - (IN)End index of interest.
 *      trav_fn   - (IN)Callback function for every entry.
 *      user_data - (IN)User callback data.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_ip6_traverse(int unit, uint32 start, uint32 end,
                            bcm_l3_route_traverse_cb trav_fn, void *user_data)
{
    return bcm_xgs3_defip_traverse(unit, BCM_L3_IP6,
                                   start, end, trav_fn, user_data);
}

/*
 * Function:
 *      bcm_xgs3_defip_ecmp_get_all
 * Purpose:
 *      Get all paths for a route, useful for ECMP route
 * Parameters:
 *      unit       - (IN) SOC device unit number
 *      the_route  - (IN) route's net/mask
 *      path_array - (OUT) Array of all ECMP paths
 *      max_path   - (IN) Max number of ECMP paths
 *      path_count - (OUT) Actual number of ECMP paths
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_defip_ecmp_get_all(int unit, _bcm_defip_cfg_t *lpm_cfg,
                            bcm_l3_route_t *path_array, int max_path,
                            int *path_count)
{
    int *ecmp_group;                     /* Next hop index.            */
    int nh_ecmp_idx;                     /* Next hop/Ecmp group index. */
    int ecmp_count;                      /* Ecmp group size.           */
    int idx;                             /* Iteration index.           */
    int rv = BCM_E_UNAVAIL;              /* Operation return status.   */

    /* Input parameters checks. */
    BCM_XGS3_L3_DEFIP_INVALID_PARAMS_RETURN(unit, lpm_cfg);


    /* Get lpm entry. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, lpm_get)) {
        rv = BCM_XGS3_L3_HWCALL_EXEC(unit, lpm_get) (unit, lpm_cfg,
                                                     &nh_ecmp_idx);
    }
    BCM_IF_ERROR_RETURN(rv);

    if (lpm_cfg->defip_ecmp) {
        /* Trunslate next hop index to ecmp group id. */
        if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
            nh_ecmp_idx /= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        }
        /* Allocate ecmp group buffer. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_ecmp_group_alloc(unit, &ecmp_group));

        /* Get ecmp group next hop indexes. */
        rv = _bcm_xgs3_ecmp_tbl_read(unit, nh_ecmp_idx, 
                                     ecmp_group, &ecmp_count);
        if (BCM_FAILURE(rv)) {
            sal_free(ecmp_group);
            return (rv);
        }

        /* Set route information. */
        for (idx = 0; ((idx < ecmp_count) && (idx < max_path)); idx++) {
            rv = _bcm_xgs3_defip_set_route_info(unit, lpm_cfg, path_array + idx, 
                                                ecmp_group[idx]);
            if (BCM_FAILURE(rv)) {
                sal_free(ecmp_group);
                return (rv);
            }
        }
        sal_free(ecmp_group);
        /* Fill number of ecmp paths. */
        *path_count = idx;
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_defip_set_route_info(unit, lpm_cfg, path_array, 
                                            nh_ecmp_idx));
        *path_count =  1;
    }
    return (BCM_E_NONE);
} 
/*
 * Function:
 *      bcm_xgs3_l3_ip6_prefix_map_get
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
bcm_xgs3_l3_ip6_prefix_map_get(int unit, int map_size, 
                              bcm_ip6_t *ip6_array, int *ip6_count)
{ 
#if defined(BCM_TRX_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT)
    soc_mem_t mem;         /* IPv4 in IPv6 table memory.    */
    int  tbl_sz;           /* IPv4 in IPv6 table size.      */
    char *tbl_ptr;         /* Dma table pointer.            */
    uint32 *buf_p;         /* Table entry pointer.          */
    int  idx;              /* Table iterator.               */
    int  valid_entry_count;/* Table valid entry count.      */

    mem = BCM_XGS3_L3_MEM(unit, v6_prefix_map);

    /* Check feature support. */
    if (INVALIDm == mem) {
        return (BCM_E_UNAVAIL);
    }

    /* Input parameters sanity check. */
    if ((NULL == ip6_count) || ((NULL == ip6_array) && (0 != map_size))) {
        return (BCM_E_PARAM);
    }

    /* Table DMA the ipv4 in ipv6 table to software copy */
    BCM_IF_ERROR_RETURN 
        (bcm_xgs3_l3_tbl_dma(unit, mem, BCM_XGS3_L3_ENT_SZ(unit, v6_prefix_map),
                             "v6_prefix_tbl", &tbl_ptr, &tbl_sz));

    /* Reset destination buffer. */
    valid_entry_count = 0;
    if (NULL != ip6_array) {
        sal_memset (ip6_array, 0, map_size * sizeof(bcm_ip6_t));
    }

    for (idx = 0; idx < tbl_sz; idx++) {
        buf_p = soc_mem_table_idx_to_pointer(unit, mem, uint32 *, tbl_ptr, idx);
        /* Skip if entry is invalid. */
        if (!soc_mem_field32_get(unit, mem, buf_p, ENABLEf)) {
            continue;
        } 

        if (NULL != ip6_array) {
            soc_mem_ip6_addr_get(unit, mem, buf_p, IPV6_PREFIXf,
                                 ip6_array[valid_entry_count],
                                 SOC_MEM_IP6_UPPER_96BIT);
            /* Update number of read entries . */
            if (++valid_entry_count >= map_size) {
                break;
            }
        } else {
           valid_entry_count++;
        }
    }

    /* Assign number of valid entries read. */
    *ip6_count = valid_entry_count;
    /* Free resources. */
    soc_cm_sfree(unit,tbl_ptr);

    return (BCM_E_NONE);
#endif /* BCM_TRX_SUPPORT || BCM_FIREBOLT2_SUPPORT */
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_xgs3_l3_ip6_prefix_map_add
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
bcm_xgs3_l3_ip6_prefix_map_add(int unit, bcm_ip6_t ip6_addr) 
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT)
    soc_mem_t mem;         /* IPv4 in IPv6 table memory.    */
    int  tbl_sz;           /* IPv4 in IPv6 table size.      */
    char *tbl_ptr;         /* Dma table pointer.            */
    uint32 *buf_p;         /* Table entry pointer.          */
    bcm_ip6_t addr;        /* IPv6 prefix map.              */
    int  rv;               /* Operation status.             */
    int  idx;              /* Table iterator.               */

    mem = BCM_XGS3_L3_MEM(unit, v6_prefix_map);

    /* Check feature support. */
    if (INVALIDm == mem) {
        return (BCM_E_UNAVAIL);
    }

    /* Reset lower 32 bit of ip6 address. */
    sal_memset(((char *)ip6_addr) + 12,0, 4);

    /* Table DMA the ipv4 in ipv6 table to software copy */
    rv = bcm_xgs3_l3_tbl_dma(unit, mem, BCM_XGS3_L3_ENT_SZ(unit, v6_prefix_map),
                             "v6_prefix_tbl", &tbl_ptr, &tbl_sz);
    BCM_IF_ERROR_RETURN(rv);

    rv = BCM_E_RESOURCE;  /* Assuming there is no room for new entry. */
    for (idx = 0; idx < tbl_sz; idx++) {
        buf_p = soc_mem_table_idx_to_pointer (unit, mem, uint32 *, tbl_ptr, idx);

        /* Skip if entry is disabled. */
        if (!soc_mem_field32_get(unit, mem, buf_p, ENABLEf)) {
            if (BCM_E_RESOURCE == rv) {
                soc_mem_field32_set(unit, mem, buf_p, ENABLEf, 1);

                soc_mem_ip6_addr_set(unit, mem, buf_p, IPV6_PREFIXf, 
                                     ip6_addr, SOC_MEM_IP6_UPPER_96BIT);
                rv = BCM_E_NONE;
            }
            continue;
        }

        /* Extract entry prefix. */
        soc_mem_ip6_addr_get(unit, mem, buf_p, IPV6_PREFIXf, addr,
                             SOC_MEM_IP6_UPPER_96BIT);

        /* Compare 12 most significant bytes of address. */
        if (0 == sal_memcmp(addr, ip6_addr, 12)) {
           rv = BCM_E_EXISTS;
           break; 
        }
    }

    /* Write the modified  buffer back. */
    if (BCM_SUCCESS(rv)) {
        rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL,
                                 soc_mem_index_min(unit, mem),
                                 soc_mem_index_max(unit, mem), tbl_ptr); 
    }

    /* Free resources. */
    soc_cm_sfree(unit,tbl_ptr);
    return (rv);
#endif /* BCM_TRX_SUPPORT || BCM_FIREBOLT2_SUPPORT */
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_xgs3_l3_ip6_prefix_map_delete
 * Purpose:
 *      Destroy an IPv6 prefix map entry.
 * Parameters:
 *      unit     - (IN)  bcm device.
 *      ip6_addr - (IN)  IPv6 translation address.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_ip6_prefix_map_delete(int unit, bcm_ip6_t ip6_addr)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT)
    soc_mem_t mem;         /* IPv4 in IPv6 table memory.    */
    int  tbl_sz;           /* IPv4 in IPv6 table size.      */
    char *tbl_ptr;         /* Dma table pointer.            */
    uint32 *buf_p;         /* Table entry pointer.          */
    bcm_ip6_t addr;        /* IPv6 prefix map.              */
    int  rv;                /* Operation status.             */
    int  idx;              /* Table iterator.               */

    mem = BCM_XGS3_L3_MEM(unit, v6_prefix_map);

    /* Check feature support. */
    if (INVALIDm == mem) {
        return (BCM_E_UNAVAIL);
    }

    /* Table DMA the ipv4 in ipv6 table to software copy */
    rv = bcm_xgs3_l3_tbl_dma(unit, mem, BCM_XGS3_L3_ENT_SZ(unit, v6_prefix_map),
                             "v6_prefix_tbl", &tbl_ptr, &tbl_sz);
    BCM_IF_ERROR_RETURN(rv);

    rv = BCM_E_NOT_FOUND;
    for (idx = 0; idx < tbl_sz; idx++) {
        buf_p = soc_mem_table_idx_to_pointer (unit, mem, uint32 *, tbl_ptr, idx);

        /* Skip if entry is disabled. */
        if (!soc_mem_field32_get(unit, mem, buf_p, ENABLEf)) {
            continue;
        } 

        /* Extract entry prefix. */
        soc_mem_ip6_addr_get(unit, mem, buf_p, IPV6_PREFIXf, addr,
                             SOC_MEM_IP6_UPPER_96BIT);

        /* Compare 12 most significant bytes of address. */
        if (0 == sal_memcmp(addr, ip6_addr, 12)) {
           soc_mem_field32_set(unit, mem, buf_p, ENABLEf, 0);
           rv = BCM_E_NONE;
           break; 
        }
    }

    if (BCM_SUCCESS(rv)) { 
        /* Write the buffer back. */
        rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL,
                                 soc_mem_index_min(unit, mem),
                                 soc_mem_index_max(unit, mem), tbl_ptr); 
    }
    /* Free resources. */
    soc_cm_sfree(unit,tbl_ptr);

    return (rv);
#endif /* BCM_TRX_SUPPORT || BCM_FIREBOLT2_SUPPORT */
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_xgs3_l3_ip6_prefix_map_delete_all
 * Purpose:
 *      Flush all IPv6 prefix maps.
 * Parameters:
 *      unit     - (IN)  bcm device.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_l3_ip6_prefix_map_delete_all(int unit)
{
#if defined(BCM_TRX_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT)
    /* Check feature support. */
    if (INVALIDm == BCM_XGS3_L3_MEM(unit, v6_prefix_map)) {
        return (BCM_E_UNAVAIL);
    }

    /* Clear IPv6 prefix map table. */
    BCM_IF_ERROR_RETURN 
        (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, v6_prefix_map)));

    return (BCM_E_NONE);
#endif /* BCM_TRX_SUPPORT || BCM_FIREBOLT2_SUPPORT */
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      _bcm_xgs3_l3_ingress_interface_add
 * Purpose:
 *      Add l3 ingress interface. 
 * Parameters:
 *      unit  - (IN) Bcm device number.
 *      iif   - (IN/OUT) Ingress interface descriptor.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_ingress_interface_add(int unit, _bcm_l3_ingress_intf_t *iif) 
{
    static int last_allocated_iif = BCM_VLAN_MAX; /* Last allocated intf id. */
    int idx;                                      /* Allocation index        */
    int loop_max;                                 /* Max number of tries.    */

    /* Input parameters check. */
    if (NULL == iif) {
        return (BCM_E_PARAM);
    }
    /* Vrf id range check. */
    if ((iif->vrf > SOC_VRF_MAX(unit)) ||
        (iif->vrf <  BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM);
    }

    /* Qualification class range check. */
    if ((iif->if_class > SOC_INTF_CLASS_MAX(unit)) || (iif->if_class < 0)) {
        return (BCM_E_PARAM);
    }

    if ((0 == BCM_XGS3_L3_HWCALL_CHECK(unit, ing_intf_add))) {
        return (BCM_E_UNAVAIL);
    }

    /* Allocate interface class id. */
    if ((iif->flags & BCM_L3_INGRESS_WITH_ID) || (iif->flags & BCM_L3_INGRESS_REPLACE)){
        if ((iif->intf_id >= (BCM_XGS3_L3_ING_IF_TBL_SIZE(unit))) ||
            (iif->intf_id < 0)) {
            return (BCM_E_PARAM);
        }
        /* Does entry exist? */
        if (iif->flags & BCM_L3_INGRESS_WITH_ID) {
             if (0 != SHR_BITGET(BCM_XGS3_L3_ING_IF_INUSE(unit), iif->intf_id)) {
                 return (BCM_E_EXISTS);
             }
        }
    } else {
        idx = last_allocated_iif + 1; 
        loop_max = BCM_XGS3_L3_ING_IF_TBL_SIZE(unit);
        while (loop_max) {
            if (0 == SHR_BITGET(BCM_XGS3_L3_ING_IF_INUSE(unit), idx)) {
                SHR_BITSET(BCM_XGS3_L3_ING_IF_INUSE(unit), idx); 
                iif->intf_id = idx;
                break;
            }
            if (idx == BCM_XGS3_L3_ING_IF_TBL_SIZE(unit) - 1) {
                idx = BCM_VLAN_MAX; 
            }
            idx++;
            loop_max--;
        }
        if (0 == loop_max) { 
            return (BCM_E_FULL);
        }
    }
    return  BCM_XGS3_L3_HWCALL_EXEC(unit, ing_intf_add) (unit, iif);
}

/*
 * Function:
 *      _bcm_xgs3_l3_ingress_interface_delete
 * Purpose:
 *      Delete l3 ingress interface. 
 * Parameters:
 *      unit  - (IN) Bcm device number.
 *      if_id - (IN) Ingress interface id.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_ingress_interface_delete(int unit, int intf_id) 
{
    /* Input parameters check. */
    if ((intf_id >= (BCM_XGS3_L3_ING_IF_TBL_SIZE(unit))) || (intf_id < 0)) {
        return (BCM_E_PARAM);
    } 

    if (BCM_XGS3_L3_HWCALL_CHECK(unit, ing_intf_del)) {
        SHR_BITCLR(BCM_XGS3_L3_ING_IF_INUSE(unit), intf_id); 
        return  BCM_XGS3_L3_HWCALL_EXEC(unit, ing_intf_del) (unit, intf_id);
    }
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      _bcm_xgs3_l3_ingress_interface_get
 * Purpose:
 *      Get l3 ingress interface. 
 * Parameters:
 *      unit  - (IN) Bcm device number.
 *      iif   - (IN/OUT) Ingress interface descriptor.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_ingress_interface_get(int unit, _bcm_l3_ingress_intf_t *iif) 
{
    /* Input parameters check. */
    if (NULL == iif) {
        return (BCM_E_PARAM);
    }

    if ((iif->intf_id >= (BCM_XGS3_L3_ING_IF_TBL_SIZE(unit))) || 
        (iif->intf_id < 0)) {
        return (BCM_E_PARAM);
    } 

    if (BCM_XGS3_L3_HWCALL_CHECK(unit, ing_intf_get)) {
        return  BCM_XGS3_L3_HWCALL_EXEC(unit, ing_intf_get) (unit, iif);
    }
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      _bcm_xgs3_tunnel_type_support_check
 * Purpose:
 *      Check if tunnel type is supported on a device. 
 * Parameters:
 *      unit        - (IN) Bcm device number.
 *      tnl_type    - (IN) Tunnel type to check.   
 *      tunnel_term - (OUT) Type supported as tunnel terminator.   
 *      tunnel_init - (OUT) Type supported as tunnel initiator.   
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_tunnel_type_support_check(int unit, bcm_tunnel_type_t tnl_type, 
                                    int *tunnel_term, int *tunnel_init)
{
    int term;      /* Tunnel Terminator supported. */
    int init;      /* Tunnel Initiator supported.  */

    switch (tnl_type) {
      case bcmTunnelTypeNone:
      case bcmTunnelTypeIp4In4:
      case bcmTunnelTypeIp6In4:
      case bcmTunnelType6In4Uncheck:
      case bcmTunnelTypeIsatap:
      case bcmTunnelTypePimSmDr1:
      case bcmTunnelTypePimSmDr2:
          term = init = TRUE;
          break;
      case bcmTunnelTypeIpAnyIn4:
          init = TRUE;
          if (soc_feature(unit, soc_feature_tunnel_protocol_match)) {
              term = FALSE;
          } else {
              term = TRUE;
          }
          break;
      case bcmTunnelTypeIp4In6:
      case bcmTunnelTypeIp6In6:
          if (!soc_feature(unit, soc_feature_tunnel_any_in_6)) {
              term = init = FALSE;
          } else {
              term = init = TRUE;
          }
          break;
      case bcmTunnelTypeIpAnyIn6:
          if (!soc_feature(unit, soc_feature_tunnel_any_in_6)) {
              term = init = FALSE;
          } else {
              init = TRUE;
              if (soc_feature(unit, soc_feature_tunnel_protocol_match)) {
                  term = FALSE;
              } else {
                  term = TRUE;
              }
          }
          break;
      case bcmTunnelType6In4:
          if (!soc_feature(unit, soc_feature_tunnel_6to4_secure)) {
              term = init = FALSE;
          }  else {
              term = init = TRUE;
          }
          break;
      case bcmTunnelTypePim6SmDr1:
      case bcmTunnelTypePim6SmDr2:
          if (!soc_feature(unit, soc_feature_tunnel_any_in_6)) {
              term = init = FALSE;
          } else {
              init = TRUE;
              term = FALSE;
          }
          break;
      case bcmTunnelTypeGre4In4:
      case bcmTunnelTypeGre6In4:
      case bcmTunnelTypeGreAnyIn4:
          if (!soc_feature(unit, soc_feature_tunnel_gre)) {
              term = init = FALSE;
          } else {
              term = init = TRUE;
          }
          break;
      case bcmTunnelTypeGre4In6:
      case bcmTunnelTypeGre6In6:
      case bcmTunnelTypeGreAnyIn6:
          if ((!soc_feature(unit, soc_feature_tunnel_gre)) ||
              (!soc_feature(unit, soc_feature_tunnel_any_in_6))) {
              term = init = FALSE;
          } else {
              term = init = TRUE;
          }
          break;
      case bcmTunnelTypeUdp:
      case bcmTunnelTypeMpls:
          if (!SOC_IS_EASYRIDER(unit)) {
              term = init = FALSE;
          } else {
              term = init = TRUE;
          }
          break;
      case bcmTunnelTypeWlanWtpToAc:
      case bcmTunnelTypeWlanWtpToAc6:
      case bcmTunnelTypeWlanAcToAc:
      case bcmTunnelTypeWlanAcToAc6:
          if (soc_feature(unit, soc_feature_wlan)) {
              term = init = TRUE;
          } else {
              term = init = FALSE;
          }
          break;
      case bcmTunnelTypeAutoMulticast:
      case bcmTunnelTypeAutoMulticast6:
          if (soc_feature(unit, soc_feature_auto_multicast)) {
              term = init = TRUE;
          } else {
              term = init = FALSE;
          }
          break;
      default:
          term = init = FALSE;
    }

    if (NULL != tunnel_term) {
        *tunnel_term = term;     
    }
    if (NULL != tunnel_init) {
        *tunnel_init = init;     
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_set_tnl_term_type
 * Purpose:
 *      Set tunnel terminator HW type & sub type.
 * Parameters:
 *      unit          - (IN)SOC unit number 
 *      tnl_info      - (IN)tnl_info Tunnel parameters. 
 *      tnl_type      - (OUT)Tunnel type.         
 *      tnl_sub_type  - (OUT)Tunnel sub_type
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_set_tnl_term_type(int unit, bcm_tunnel_terminator_t *tnl_info,
                               _bcm_tnl_term_type_t *tnl_type)
{
    /* Input parameters check. */
    if ((NULL == tnl_info) || (NULL == tnl_type)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination buffer first. */
    sal_memset(tnl_type, 0, sizeof(_bcm_tnl_term_type_t));

    /* Set tunnel outer header IP version. */
    tnl_type->tnl_outer_hdr_ipv6 =_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_info->type);

    switch (tnl_info->type) {
      case bcmTunnelType6In4Uncheck:
          tnl_type->tnl_auto = 1;           /* automatic tunnel. */
          tnl_type->tnl_sub_type = 0;
          tnl_type->tnl_protocol = 0x29;    /* 6over4 OR 6to4 */
          break;
      case bcmTunnelTypeIsatap:
          tnl_type->tnl_auto = 1;           /* automatic tunnel. */
          tnl_type->tnl_sub_type = 0x1;
          tnl_type->tnl_protocol = 0x29;    /* 6over4 OR 6to4 */
          break;
      case bcmTunnelType6In4:
          tnl_type->tnl_auto = 1;           /* automatic tunnel. */
          if (soc_feature(unit, soc_feature_tunnel_6to4_secure)) {
              tnl_type->tnl_sub_type = 0x2;
          } else {
              tnl_type->tnl_sub_type = 0;
          }
          tnl_type->tnl_protocol = 0x29;   /* 6over4 OR 6to4 */
          break;
      case bcmTunnelTypeIp4In4: 
      case bcmTunnelTypeIp4In6: 
          tnl_type->tnl_auto = 0;          /* configured tunnel */
          tnl_type->tnl_sub_type = 0x2;    /* IPv4 payload only.*/
          tnl_type->tnl_protocol = 0x04;   /* IP in IP */
          break;
      case bcmTunnelTypeIp6In4: 
      case bcmTunnelTypeIp6In6: 
          tnl_type->tnl_auto = 0;          /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x1;    /* IPv6 payload only.   */
          tnl_type->tnl_protocol = 0x29;   /* IPv6 */
          break;
      case bcmTunnelTypeIpAnyIn4: 
      case bcmTunnelTypeIpAnyIn6: 
          tnl_type->tnl_auto = 0;          /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;    /* IPv4 & IPv6 payload. */
          tnl_type->tnl_protocol = 0x04;   /* IP in IP */
          break;
      case bcmTunnelTypeGre4In4:
      case bcmTunnelTypeGre4In6:
          tnl_type->tnl_auto = 0;            /* configured tunnel */
          tnl_type->tnl_sub_type = 0x2;      /* IPv4 payload only.*/
          tnl_type->tnl_gre = 0x1;           /* GRE tunnel.       */
          tnl_type->tnl_gre_v4_payload = 0x1;/* IPv4 payload only.*/
          tnl_type->tnl_protocol = 0x2F;     /* Protocol GRE      */
          break;
      case bcmTunnelTypeGre6In4:
      case bcmTunnelTypeGre6In6:
          tnl_type->tnl_auto = 0;            /* configured tunnel */
          tnl_type->tnl_sub_type = 0x1;      /* IPv6 payload only.*/
          tnl_type->tnl_gre = 0x1;           /* GRE tunnel.       */
          tnl_type->tnl_gre_v6_payload = 0x1;/* IPv4 payload only.*/
          tnl_type->tnl_protocol = 0x2F;     /* Protocol GRE      */
          break;
      case bcmTunnelTypeGreAnyIn4:
      case bcmTunnelTypeGreAnyIn6:
          tnl_type->tnl_auto = 0;            /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;      /* IPv4 & IPv6 payload. */
          tnl_type->tnl_gre = 0x1;           /* GRE tunnel.          */
          tnl_type->tnl_gre_v4_payload = 0x1;/* IPv4 payload only.   */
          tnl_type->tnl_gre_v6_payload = 0x1;/* IPv4 payload only.   */
          tnl_type->tnl_protocol = 0x2F;     /* Protocol GRE      */
          break;
      case bcmTunnelTypePimSmDr1:
      case bcmTunnelTypePimSmDr2:
      case bcmTunnelTypePim6SmDr1:
      case bcmTunnelTypePim6SmDr2:
          tnl_type->tnl_auto = 0;            /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;      /* IPv4 & IPv6 payload. */
          tnl_type->tnl_pim_sm = 0x1;        /* PIM SM tunnel.       */
          tnl_type->tnl_protocol = 0x67;     /* Protocol PIM         */
          break;
      case bcmTunnelTypeUdp:
          tnl_type->tnl_auto = 0;            /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;      /* IPv4 & IPv6 payload. */
          tnl_type->tnl_udp = 0x1;           /* UDP tunnel.          */
          tnl_type->tnl_protocol = 0x11;     /* Protocol UDP         */
          break;
      case bcmTunnelTypeWlanWtpToAc:
          tnl_type->tnl_auto = 0;            /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;      /* IPv4 & IPv6 payload. */
          tnl_type->tnl_udp_type = 0x1;      /* UDP tunnel.          */
          tnl_type->tnl_protocol = 0x11;     /* Protocol UDP         */
          break;
      case bcmTunnelTypeWlanAcToAc:
          tnl_type->tnl_auto = 0;            /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;      /* IPv4 & IPv6 payload. */
          tnl_type->tnl_udp_type = 0x0;      /* UDP tunnel.          */
          tnl_type->tnl_protocol = 0x11;     /* Protocol UDP         */
          break;
      case bcmTunnelTypeWlanWtpToAc6:
          tnl_type->tnl_auto = 0;            /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;      /* IPv4 & IPv6 payload. */
          tnl_type->tnl_udp_type = 0x1;      /* UDP tunnel.          */
          tnl_type->tnl_protocol = 0x88;     /* Protocol UDP lite    */
          break;
      case bcmTunnelTypeWlanAcToAc6:
          tnl_type->tnl_auto = 0;            /* configured tunnel    */
          tnl_type->tnl_sub_type = 0x3;      /* IPv4 & IPv6 payload. */
          tnl_type->tnl_udp_type = 0x0;      /* UDP tunnel.          */
          tnl_type->tnl_protocol = 0x88;     /* Protocol UDP lite    */
          break;
      case bcmTunnelTypeAutoMulticast:
      case bcmTunnelTypeAutoMulticast6:
          tnl_type->tnl_auto = 0;			 /* Configured tunnel	 */
          tnl_type->tnl_sub_type = 0x2; 	
          tnl_type->tnl_udp_type = 0x2; 	 /* AMT tunnel. 		 */
          tnl_type->tnl_protocol = 0x11;       /* Protocol UDP         */
          break;	  
      default:
          return (BCM_E_PARAM);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_l3_get_tnl_term_type
 * Purpose:
 *      Get tunnel terminator API type 
 * Parameters:
 *      unit          - (IN)SOC unit number 
 *      tnl_info      - (IN/OUT)tnl_info Tunnel parameters. 
 *      tnl_type      - (IN)Tunnel type.         
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_xgs3_l3_get_tnl_term_type(int unit, bcm_tunnel_terminator_t *tnl_info,
                               _bcm_tnl_term_type_t *tnl_type)
                               
{
    int v6;                 /* Outer header is ipv6. */

    /* Input parameters check. */
    if ((NULL == tnl_type) || (NULL == tnl_info)) {
        return (BCM_E_PARAM);
    }

    if (tnl_type->tnl_auto) {             /* Automatic tunnel */
        switch (tnl_type->tnl_sub_type) {
          case 0x0:
              tnl_info->type = bcmTunnelType6In4Uncheck;
              break;
          case 0x1:
              tnl_info->type = bcmTunnelTypeIsatap;
              break;
          case 0x2:
              tnl_info->type = bcmTunnelType6In4;
              break;
          default:
              return (BCM_E_PARAM);
        }
        return (BCM_E_NONE); 
    } 

    /* Configured tunnel */

    /* Read outer header IP version. */
    v6 = tnl_type->tnl_outer_hdr_ipv6;

    if (tnl_type->tnl_gre) {

        if ((tnl_type->tnl_gre_v4_payload) &&
            (tnl_type->tnl_gre_v6_payload)) {
            tnl_info->type = (v6) ? bcmTunnelTypeGreAnyIn6 : 
                bcmTunnelTypeGreAnyIn4;
        } else {
            if (tnl_type->tnl_gre_v6_payload) {
                tnl_info->type = (v6) ? bcmTunnelTypeGre6In6 : 
                    bcmTunnelTypeGre6In4;
            } else {
                tnl_info->type = (v6) ? bcmTunnelTypeGre4In6 : 
                    bcmTunnelTypeGre4In4;
            }
        }
    } else if (tnl_type->tnl_udp) {
        tnl_info->type = bcmTunnelTypeUdp;
    } else if (tnl_type->tnl_pim_sm) {
        tnl_info->type = (v6) ? bcmTunnelTypePim6SmDr1 : bcmTunnelTypePimSmDr1;
    } else if (tnl_type->tnl_protocol == 0x11) {
        if (tnl_type->tnl_udp_type == 1) {
            tnl_info->type = bcmTunnelTypeWlanWtpToAc;
        } else if (tnl_type->tnl_udp_type == 2) {
            tnl_info->type = (v6) ? bcmTunnelTypeAutoMulticast6 : bcmTunnelTypeAutoMulticast;
        } else {
            tnl_info->type = bcmTunnelTypeWlanAcToAc;
        } 
    } else if (tnl_type->tnl_protocol == 0x88) {
        if (tnl_type->tnl_udp_type == 1) {
            tnl_info->type = bcmTunnelTypeWlanWtpToAc6;
        } else {
            tnl_info->type = bcmTunnelTypeWlanAcToAc6;
        } 
    } else {
        switch (tnl_type->tnl_sub_type) {
          case 0x3: 
              tnl_info->type = (v6) ? bcmTunnelTypeIpAnyIn6 : 
                 bcmTunnelTypeIpAnyIn4;
              break;
          case 0x2: 
              tnl_info->type = (v6) ? bcmTunnelTypeIp4In6 : 
                  bcmTunnelTypeIp4In4;
              break;
          case 0x1: 
              tnl_info->type = (v6) ? bcmTunnelTypeIp6In6 : 
                  bcmTunnelTypeIp6In4;
              break;
          default:
              return (BCM_E_PARAM);
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_terminator_validate
 * Purpose:
 *     Tunnel terminator structure sanity check routine. 
 * Parameters:
 *      unit     - (IN) SOC unit number. 
 *      tnl_info - (IN) Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_NONE  - If data in the structure is valid. 
 *      BCM_E_PARAMS - otherwise. 
 */
STATIC int
_bcm_xgs3_tunnel_terminator_validate(int unit, 
                                     bcm_tunnel_terminator_t *tnl_info)
{
    int support;         /* Tunnel type support by device. */
    int masklen;         /* IPv6 sip/dip mask length.      */
    bcm_ip6_t zero_addr; /* IPv6 sip address.              */
    int rv;              /* Operation return status.       */

    /* Input parameters check. */
    if (NULL == tnl_info) {
        return (SOC_E_PARAM);
    }

    /* Vrf range sanity. */
    if ((tnl_info->vrf > SOC_VRF_MAX(unit)) ||
        (tnl_info->vrf <  BCM_L3_VRF_DEFAULT)) {
        return (BCM_E_PARAM); 
    }

    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_info->type)) {
        sal_memset(zero_addr, 0, sizeof(bcm_ip6_t)); 

        /* Destination ip mask must be a full mask. */
        masklen = bcm_ip6_mask_length(tnl_info->dip6_mask);
        if (masklen != BCM_XGS3_L3_IPV6_PREFIX_LEN) {
            return (BCM_E_PARAM);
        }

        /* Apply source ip subnet mask. */
        rv = bcm_xgs3_l3_mask6_apply(tnl_info->sip6_mask, tnl_info->sip6);
        BCM_IF_ERROR_RETURN(rv);
    } else {
        /* Destination ip mask must be a full mask. */
        if (tnl_info->dip_mask != BCM_XGS3_L3_IP4_FULL_MASK)  {
            return (BCM_E_PARAM);
        }

        /* Apply source ip subnet mask. */
        tnl_info->sip &= tnl_info->sip_mask; 
    }

    rv = _bcm_xgs3_tunnel_type_support_check(unit, tnl_info->type,
                                             &support, NULL);
    BCM_IF_ERROR_RETURN(rv);

    if (FALSE == support) {
        return (BCM_E_UNAVAIL);
    }

    return (BCM_E_NONE);
}



/*
 * Function:
 *      bcm_xgs3_tunnel_terminator_add
 * Purpose:
 *      Add a tunnel terminator for DIP-SIP  
 * Parameters:
 *      unit     - (IN) SOC unit number. 
 *      tnl_info - (IN) Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_terminator_add(int unit, bcm_tunnel_terminator_t *tnl_info)
{
    bcm_tunnel_terminator_t tmp_info;    /* Entry lookup buffer.       */
    _bcm_l3_ingress_intf_t  iif;         /* Ingress interface.         */
    int old_iif = -1;                    /* Original ingress interface.*/
    int rv;                              /* Operation result.          */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Make sure hw call is initialized.*/
    if ((!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_term_add)) ||
        (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_term_get))) {
        return (BCM_E_UNAVAIL);
    }

    /* Check input parameters */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_tunnel_terminator_validate(unit, tnl_info));

    /* Fill lkup buf. */
    tmp_info = *tnl_info;

    /* Check if entry already exists. */
    rv = BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_term_get) (unit, &tmp_info);
    if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
        return (rv);
    }

    /* Make sure replace flag is set if replacing existing entry. */
    if (BCM_SUCCESS(rv)) {
        if (0 == (tnl_info->flags & BCM_TUNNEL_REPLACE)) {
            return (BCM_E_EXISTS);
        }

        if (BCM_XGS3_L3_ING_IF_TBL_SIZE(unit) && (tmp_info.vlan > BCM_VLAN_MAX)) {
            old_iif = tmp_info.vlan;
        }
    }

   /* If SC L3IngressMode Set, then tnl_info->vlan contains the L3_IIF Idx */
    if (BCM_XGS3_L3_ING_IF_TBL_SIZE(unit) && (0 == tnl_info->vlan)) {
        /* Check L3 Ingress Interface mode. */ 
        if (!(BCM_XGS3_L3_INGRESS_MODE_ISSET(unit))) {
             sal_memset(&iif, 0, sizeof(_bcm_l3_ingress_intf_t)); 
             iif.vrf      = tnl_info->vrf;
             iif.if_class = tnl_info->if_class; 
             rv =_bcm_xgs3_l3_ingress_interface_add (unit, &iif);
             if (BCM_FAILURE(rv)) {
                 if (-1 != old_iif) {
                     (void)_bcm_xgs3_l3_ingress_interface_delete(unit, old_iif);
                 }
                 return (rv);
             }
             tnl_info->vlan = iif.intf_id;
        }
    }

    /* Insert new/updated entry. */
    rv = BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_term_add) (unit, tnl_info);
    if (BCM_FAILURE(rv)) {
        if (BCM_XGS3_L3_ING_IF_TBL_SIZE(unit) && 
            (tnl_info->vlan > BCM_VLAN_MAX)) {
            (void)(_bcm_xgs3_l3_ingress_interface_delete(unit, iif.intf_id));
        }
    }

    /* Remove original ingress interface. */
    if (-1 != old_iif) {
        (void)_bcm_xgs3_l3_ingress_interface_delete(unit, old_iif);
    }

    return (rv);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_terminator_delete
 * Purpose:
 *      Delete a tunnel terminator for DIP-SIP + 
 * Parameters:
 *      unit     - (IN) SOC unit number. 
 *      tnl_info - (IN) Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_xgs3_tunnel_terminator_delete(int unit, bcm_tunnel_terminator_t *tnl_info)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Make sure hw call is initialized.*/
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_term_del)) {
        return (BCM_E_UNAVAIL);
    }

    /* Check input parameters */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_tunnel_terminator_validate(unit, tnl_info));

    /* Do entry lookup. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_term_get) (unit, tnl_info));

    /* Delete ingress interface. */ 
    if (BCM_XGS3_L3_ING_IF_TBL_SIZE(unit) && 
        (tnl_info->vlan > BCM_VLAN_MAX)) {
        BCM_IF_ERROR_RETURN 
            (_bcm_xgs3_l3_ingress_interface_delete(unit, tnl_info->vlan));
    }

    /* Delete tunnel entry. */
    return  BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_term_del) (unit, tnl_info);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_terminator_update
 * Purpose:
 *      Update a tunnel terminator for DIP-SIP + 
 *             on ER (DST_UDP_PORT/SRC_UDP_PORT)
 * Parameters:
 *      unit     - (IN) SOC unit number.
 *      tnl_info - (IN) Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_terminator_update(int unit, bcm_tunnel_terminator_t *tnl_info)
{
    if (NULL == tnl_info) {
        return (BCM_E_PARAM);
    }
    tnl_info->flags |= BCM_TUNNEL_REPLACE;

    return bcm_xgs3_tunnel_terminator_add(unit,tnl_info);
}

/*
 * Function:
 *     bcm_xgs3_tunnel_terminator_get
 * Purpose:
 *      Get a tunnel terminator for DIP-SIP + 
 *             on ER (DST_UDP_PORT/SRC_UDP_PORT)
 * Parameters:
 *      unit     - (IN) SOC unit number.
 *      tnl_info - (IN) Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_terminator_get(int unit, bcm_tunnel_terminator_t *tnl_info)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Make sure hw call is initialized.*/
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_term_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Check input parameters */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_tunnel_terminator_validate(unit, tnl_info));


    /* Do entry lookup. */
    return BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_term_get) (unit, tnl_info);
}
/*
 * Function:
 *      _bcm_xgs3_tnl_type_to_hw_code
 * Purpose:
 * Translate tunnel type into value used in hardware
 *    0: None
 *    1: Config
 *    2: Automatic 6to4
 *    3: Automatic ISATAP
 *    4: PIM_DR1
 *    5: PIM_DR2
 *    6: Firebolt - RESERVED/ER - GRE
 *    7: Firebolt - RESERVED/ER - UDP
 *    8: Firebolt - RESERVED/ER - MPLS
 * Parameters:
 *      unit        - (IN) SOC unit number.
 *      tnl_type    - (IN) Tunnel type.
 *      hw_code     - (OUT) HW code for tunnel type. 
 *      entry_type  - (OUT) Entry type.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_tnl_type_to_hw_code(int unit, bcm_tunnel_type_t tnl_type, 
                              int *hw_code, int *entry_type)
{
    /* Input parameters check. */
    if ((NULL == hw_code) || (NULL == entry_type)) {
        return (BCM_E_PARAM);
    }

#if defined(BCM_TRX_SUPPORT)
    if (soc_feature(unit, soc_feature_tunnel_any_in_6)) {
        return _bcm_trx_tnl_type_to_hw_code(unit, tnl_type, hw_code, entry_type);
    } else 
#endif /* BCM_TRX_SUPPORT */ 
    {
        *entry_type = BCM_XGS3_TUNNEL_INIT_V4;
    }

    switch (tnl_type) {
      case bcmTunnelTypeNone:
           *hw_code = 0;
           break;

      case bcmTunnelTypeIp4In4:
      case bcmTunnelTypeIp6In4:
      case bcmTunnelTypeIpAnyIn4:
           *hw_code = 0x1;
           break;

      case bcmTunnelTypeIp4In6:
      case bcmTunnelTypeIp6In6:
      case bcmTunnelTypeIpAnyIn6:
           *hw_code = 0x1;
           break;

      case bcmTunnelType6In4Uncheck:
           *hw_code = 0x2;
           break;

      case bcmTunnelType6In4:
           *hw_code = 0x6;
           break;

      case bcmTunnelTypeIsatap:
           *hw_code = 0x3;
           break;

      case bcmTunnelTypePimSmDr1:
           *hw_code = 0x4;
           break;

      case bcmTunnelTypePimSmDr2:
           *hw_code = 0x5;
           break;

      case bcmTunnelTypePim6SmDr1:
           *hw_code = 0x4;
           break;

      case bcmTunnelTypePim6SmDr2:
           *hw_code = 0x5;
           break;


      case bcmTunnelTypeGre4In4:
      case bcmTunnelTypeGre6In4:
      case bcmTunnelTypeGreAnyIn4:
           *hw_code = 0x6;
           break;
      case bcmTunnelTypeUdp:
           *hw_code = 0x7;
           break;

      case bcmTunnelTypeMpls:
           *hw_code = 0x8;
           break;

      default:
           break;
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_tnl_hw_code_to_type
 * Purpose:
 * Translate hw code for tunnel type into api value 
 *    0: None
 *    1: Config
 *    2: Automatic 6to4
 *    3: Automatic ISATAP
 *    4: PIM_DR1
 *    5: PIM_DR2
 *    6: Firebolt - RESERVED/ER - GRE
 *    7: Firebolt - RESERVED/ER - UDP
 *    8: Firebolt - RESERVED/ER - MPLS
 * Parameters:
 *      unit        - (IN) SOC unit number.
 *      hw_tnl_type - (IN) HW tunnel code.
 *      entry_type  - (IN) Tunnel entry type. 
 *      tunnel_type - (OUT) Tunnel type. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_xgs3_tnl_hw_code_to_type(int unit, int hw_tnl_type, 
                              int entry_type, bcm_tunnel_type_t *tunnel_type)
{
    /* Input parameters check. */
    if (NULL == tunnel_type) {
       return (BCM_E_PARAM);
    }
#if defined(BCM_TRX_SUPPORT)
    if (soc_feature(unit, soc_feature_tunnel_any_in_6)) {
        return _bcm_trx_tnl_hw_code_to_type(unit, hw_tnl_type, entry_type, tunnel_type);
    } 
#endif /* BCM_TRX_SUPPORT*/

    switch (hw_tnl_type) {
      case 0:
          *tunnel_type = bcmTunnelTypeNone;
          break;
      case 1:
          *tunnel_type = bcmTunnelTypeIpAnyIn4;
          break;
      case 2:
          *tunnel_type = bcmTunnelType6In4Uncheck;
          break;
      case 3:
          *tunnel_type = bcmTunnelTypeIsatap;
          break;
      case 4:
          *tunnel_type = (BCM_XGS3_TUNNEL_INIT_V4 == entry_type) ? \
                         bcmTunnelTypePimSmDr1 : bcmTunnelTypePim6SmDr1;
          break;
      case 5:
          *tunnel_type = (BCM_XGS3_TUNNEL_INIT_V4 == entry_type) ? \
                         bcmTunnelTypePimSmDr2 : bcmTunnelTypePim6SmDr2;
          break;
      case 6:
          *tunnel_type = bcmTunnelTypeGreAnyIn4;
          break;
      case 7:
          *tunnel_type = bcmTunnelTypeUdp;
          break;
      case 8:
          *tunnel_type = bcmTunnelTypeMpls;
          break;
      default: 
          return (BCM_E_PARAM);
    }
    return (BCM_E_NONE);
}

#ifdef BCM_EASYRIDER_SUPPORT
/*
 * Function:
 *      _bcm_xgs3_adj_mac_hash_calc
 * Purpose:
 *      Calculate adjacent mac entry hash(signature). 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      buf      -  (IN)Next hop entry information.
 *      hash     -  (OUT) Hash(signature) calculated value.  
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_adj_mac_hash_calc(int unit, void *buf, uint16 *hash)
{

    if ((NULL == buf) || (NULL == hash)) {
        return (BCM_E_PARAM);
    }

    /* Calculate mac address hash. */
    *hash = _shr_crc16(0, (uint8 *) buf, sizeof(bcm_mac_t));

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_adj_mac_cmp
 * Purpose:
 *      Routine compares adjacent mac entry with entry in the chip
 *      with specified index. 
 * Parameters:
 *      unit       - (IN) SOC unit number.
 *      buf        - (IN) Adjacent mac entry to compare.
 *      index      - (IN) Entry index in the chip to compare.
 *      cmp_result - (OUT)Compare result. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_adj_mac_cmp(int unit, void *buf, int index, int *cmp_result)
{
    bcm_mac_t mac;              /* Mac entry from hw.  */

    if ((NULL == buf) || (NULL == cmp_result))
        return (BCM_E_PARAM);


    /* Get next hop entry from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, adj_mac_get)) {
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, adj_mac_get) (unit, index, mac));
    } else {
        return (BCM_E_UNAVAIL);
    }

    /* Mac compare */
    if (sal_memcmp(mac, buf, sizeof(bcm_mac_t))) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }
    *cmp_result = BCM_L3_CMP_EQUAL;
    return (BCM_E_NONE);
}

/*
 * Function:
 *       bcm_xgs3_adj_mac_add
 * Purpose:
 *       Allocate Adj MAC table index & Write Adj MAC entry.
 *       Routine trying to match new entry to an existing one, 
 *       if match found reference cound is incremented, otherwise
 *       new index allocated & entry added to hw. 
 * Parameters:
 *      unit       - (IN)StrataSwitch unit number.
 *      dmac       - (IN)Mac address.. 
 *      index      - (OUT)next hop index.  
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_adj_mac_add(int unit, bcm_mac_t dmac, int *index)
{
    _bcm_l3_tbl_op_t data;        /* Operation data. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Make sure hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, adj_mac_add)) {
        return (BCM_E_UNAVAIL);
    }

    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, adj_mac);
    data.width = _BCM_SINGLE_WIDE;
    data.entry_buffer = (void *)dmac;
    data.hash_func = _bcm_xgs3_adj_mac_hash_calc;
    data.cmp_func  = _bcm_xgs3_adj_mac_cmp;
    data.add_func  = BCM_XGS3_L3_HWCALL(unit, adj_mac_add);

    /* Add ecmp group . */
    BCM_IF_ERROR_RETURN (_bcm_xgs3_tbl_add(unit, &data)); 
    *index = data.entry_index;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_adj_mac_del
 * Purpose:
 *      Delete adjacent mac entry.  
 *      Entry kept if reference counter not equals to 1(Other tunnels  
 *      reference the same entry).   
 *       
 * Parameters:
 *      unit      - (IN)StrataSwitch unit number.
 *      ent_index - (IN)Adjacent mac entry index.  
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_adj_mac_del(int unit, int ent_index)
{
    _bcm_l3_tbl_op_t data;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Check hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, adj_mac_del)) {
        return (BCM_E_UNAVAIL);
    }

    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, adj_mac);
    data.entry_index = ent_index;
    data.width = _BCM_SINGLE_WIDE;
    data.delete_func = BCM_XGS3_L3_HWCALL(unit, adj_mac_del);

    /* Delete adjacent mac table entry. */
    return _bcm_xgs3_tbl_del(unit, &data);
}

#endif /* BCM_EASYRIDER_SUPPORT */
/*
 * Function:
 *      _bcm_xgs3_tnl_init_hash_calc
 * Purpose:
 *      Calculate tunnel initiator entry hash(signature). 
 * Parameters:
 *      unit     -  (IN)SOC unit number. 
 *      buf      -  (IN)Tunnel initiator entry information.
 *      hash     -  (OUT) Hash(signature) calculated value.  
 * Returns:
 *      BCM_X_XXX
 */
STATIC int
_bcm_xgs3_tnl_init_hash_calc(int unit, void *buf, uint16 *hash)
{

    bcm_tunnel_initiator_t tnl_entry;

    if ((NULL == buf) || (NULL == hash)) {
        return (BCM_E_PARAM);
    }

    /* Copy buffer to structure. */
    sal_memcpy(&tnl_entry, buf, sizeof(bcm_tunnel_initiator_t));

    /* Ignore flow label for not V6 tunnels. */
    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_entry.type))  {
       tnl_entry.flow_label = 0;
    }

    /* Mask fields we don't want to include in hash. */
    tnl_entry.flags = 0;

    if (SOC_IS_EASYRIDER(unit)) {
        tnl_entry.ttl = 0;
        tnl_entry.type = 0;
        sal_memset(tnl_entry.dmac, 0, sizeof(bcm_mac_t));
    } else if (SOC_IS_FBX(unit)) {
        switch (tnl_entry.type) {
          case bcmTunnelTypeIp4In4:
          case bcmTunnelTypeIp6In4:
              tnl_entry.type = bcmTunnelTypeIpAnyIn4;
              break;
          case bcmTunnelTypeIp4In6:
          case bcmTunnelTypeIp6In6:
              tnl_entry.type = bcmTunnelTypeIpAnyIn6;
              break;
          case  bcmTunnelTypeGre4In4:
          case  bcmTunnelTypeGre6In4:
              tnl_entry.type = bcmTunnelTypeGreAnyIn4;
              break;
          case    bcmTunnelTypeGre4In6:
          case    bcmTunnelTypeGre6In6:
              tnl_entry.type = bcmTunnelTypeGreAnyIn6;
              break;
          default:
              break;
        }
    }

    /* Calculate mac address hash. */
    *hash = _shr_crc16(0, (uint8 *)&tnl_entry,
                       sizeof(bcm_tunnel_initiator_t));
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_tnl_init_cmp
 * Purpose:
 *      Routine compares tunnel initiator entry with entry in the chip
 *      with specified index. 
 * Parameters:
 *      unit       - (IN) SOC unit number.
 *      buf        - (IN) Tunnel initiator entry to compare.
 *      index      - (IN) Entry index in the chip to compare.
 *      cmp_result - (OUT)Compare result. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_tnl_init_cmp(int unit, void *buf, int index, int *cmp_result)
{

    bcm_tunnel_initiator_t hw_entry; /* Entry read from hw. */
    bcm_tunnel_initiator_t *tnl_entry;       /* Api passed buffer.  */

    if ((NULL == buf) || (NULL == cmp_result)) {
        return (BCM_E_PARAM);
    }

    /* Initialization. */
    bcm_tunnel_initiator_t_init(&hw_entry);

    /* Copy buffer to structure. */
    tnl_entry = (bcm_tunnel_initiator_t *) buf;

    /* Get tunnel initiator entry from hw. */
    if (BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_init_get)) {
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_init_get) (unit, index,
                                                          &hw_entry));
    } else {
        return (BCM_E_UNAVAIL);
    }

    /* Compare source & destination ip. */
    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_entry->type)) {
        if((sal_memcmp(tnl_entry->dip6, hw_entry.dip6, sizeof(bcm_ip6_t))) || 
           (sal_memcmp(tnl_entry->sip6, hw_entry.sip6, sizeof(bcm_ip6_t))) ||
           (tnl_entry->flow_label != hw_entry.flow_label)) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            return (BCM_E_NONE);
        }
    } else {
        if ((tnl_entry->dip != hw_entry.dip) ||
            (tnl_entry->sip != hw_entry.sip)) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            return (BCM_E_NONE);
        }

        /* For all boards except FB A0 compare DSCP DF SEL v4/v6 */
        if (SOC_MEM_FIELD_VALID(unit, BCM_XGS3_L3_MEM(unit, tnl_init_v4), 
                                IPV4_DF_SELf)) {
            if ((tnl_entry->flags & BCM_TUNNEL_INIT_SET_DF) != 
                (hw_entry.flags & BCM_TUNNEL_INIT_SET_DF)) {
                *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                return (BCM_E_NONE);
            }
        }
    }

    /* Compare hw entry with passed one. */
    if ((tnl_entry->dscp_sel != hw_entry.dscp_sel) ||
        (tnl_entry->dscp != hw_entry.dscp)) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    /* Compare tunnel type / ttl */
    if (SOC_IS_FBX(unit)) {
        /* Tunnel type comparison.                                  */
        /* Note: Only outer header matters in tunnel initialization */
        if (tnl_entry->type != hw_entry.type) {
            switch (tnl_entry->type) {
              case bcmTunnelTypeIp4In4:
              case bcmTunnelTypeIp6In4:
                  if (bcmTunnelTypeIpAnyIn4 != hw_entry.type) {
                      *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                      return (BCM_E_NONE);
                  }
                  break;
              case bcmTunnelTypeIp4In6:
              case bcmTunnelTypeIp6In6:
                  if (bcmTunnelTypeIpAnyIn6 != hw_entry.type) {
                      *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                      return (BCM_E_NONE);
                  }
                  break;
              case  bcmTunnelTypeGre4In4:
              case  bcmTunnelTypeGre6In4:
                  if (bcmTunnelTypeGreAnyIn4 != hw_entry.type) {
                      *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                      return (BCM_E_NONE);
                  }
                  break;
              case    bcmTunnelTypeGre4In6:
              case    bcmTunnelTypeGre6In6:
                  if (bcmTunnelTypeGreAnyIn6 != hw_entry.type) {
                      *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                      return (BCM_E_NONE);
                  }
                  break;
              default:
                  *cmp_result = BCM_L3_CMP_NOT_EQUAL;
                  return (BCM_E_NONE);
            }
        }
        if (tnl_entry->ttl != hw_entry.ttl) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            return (BCM_E_NONE);
        }
        /* Compare destination mac. */
        if (sal_memcmp(tnl_entry->dmac, hw_entry.dmac,
                       sizeof(bcm_mac_t))) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            return (BCM_E_NONE);
        }
    }
    *cmp_result = BCM_L3_CMP_EQUAL;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_tnl_init_add
 * Purpose:
 *       Allocate tunnel initiator table index & Write tunnel initiator entry.
 *       Routine trying to match new entry to an existing one, 
 *       if match found reference cound is incremented, otherwise
 *       new index allocated & entry added to hw. 
 * Parameters:
 *      unit       - (IN)StrataSwitch unit number.
 *      flags      - (IN)Table shared flags.
 *      tnl_entry  - (IN)Tunnel initiator entry. 
 *      index      - (OUT)next hop index.  
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_tnl_init_add(int unit, uint32 flags, bcm_tunnel_initiator_t *tnl_entry,
                      int *index)
{
    _bcm_l3_tbl_op_t data;        /* Operation data.               */

    if (!(flags & _BCM_L3_SHR_WRITE_DISABLE)) {
        /* Make sure hw call is defined. */
        if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_init_add)) {
            return (BCM_E_UNAVAIL);
        }
    }

    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));
    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, tnl_init);
    data.width = _BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_entry->type) ? \
                  _BCM_DOUBLE_WIDE: _BCM_SINGLE_WIDE;
    data.oper_flags = flags;
    data.entry_buffer = (void *)tnl_entry;
    data.hash_func = _bcm_xgs3_tnl_init_hash_calc;
    data.cmp_func  = _bcm_xgs3_tnl_init_cmp;
    data.add_func  = BCM_XGS3_L3_HWCALL(unit, tnl_init_add);
    if (flags & _BCM_L3_SHR_WITH_ID) {
        data.entry_index = *index;
    }

    /* Add next hop table entry. */
    BCM_IF_ERROR_RETURN (_bcm_xgs3_tbl_add(unit, &data));
    *index = data.entry_index;

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_xgs3_tnl_init_del
 * Purpose:
 *      Delete tunnels initiator entry.  
 *      Entry kept if reference counter not equals to 1(Other tunnels  
 *      reference the same entry).   
 *       
 * Parameters:
 *      unit      - (IN)StrataSwitch unit number.
 *      flags     - (IN)Table shared flags.
 *      ent_index - (IN)Entry index.  
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_xgs3_tnl_init_del(int unit, uint32 flags, int ent_index)
{
    bcm_tunnel_initiator_t hw_entry;     /* Entry read from hw.    */
    _bcm_l3_tbl_op_t data;               /* Delete operation info. */
    
    /* Initialization */
    sal_memset(&data, 0, sizeof(_bcm_l3_tbl_op_t));

    /* Default is single table entry operation.*/
    data.width = _BCM_SINGLE_WIDE;


    /* Check hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_init_del)) {
        return (BCM_E_UNAVAIL);
    }

    if (!(flags & _BCM_L3_SHR_WRITE_DISABLE)) {
        /* Get tunnel initiator entry from hw. */
        if (BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_init_get)) {
            BCM_IF_ERROR_RETURN
                (BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_init_get) (unit, ent_index,
                                                              &hw_entry));
            /* IPv6 tunnels occupy 2 slots. */
            if(_BCM_TUNNEL_OUTER_HEADER_IPV6(hw_entry.type)) {
                data.width = _BCM_DOUBLE_WIDE;
            }
        } else {
            return (BCM_E_UNAVAIL);
        }
    }

    data.tbl_ptr =  BCM_XGS3_L3_TBL_PTR(unit, tnl_init);
    data.entry_index = ent_index;
    data.oper_flags = flags;
    data.delete_func = BCM_XGS3_L3_HWCALL(unit, tnl_init_del);

    /* Delete tunnel initiator table entry. */
    return  _bcm_xgs3_tbl_del(unit, &data); 
}

/*
 * Function:
 *      bcm_xgs3_tunnel_initiator_set
 * Purpose:
 *      Set the tunnel initiator property for the given L3 interface
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      intf     - (IN)L3 interface.
 *      tnl_info - (IN)Tunnel initiator information.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_initiator_set(int unit, bcm_l3_intf_t *intf,
                                 bcm_tunnel_initiator_t *tnl_info)
{
    _bcm_tnl_init_data_t cfg_ent; /* Tunnel configuration entry.   */
    int ifindex;                  /* Interface index.              */
    int support;                  /* Tunnel type is supported flag.*/
    int rv;                       /* Operation return status.      */
    uint32 shr_flags;             /* Table shared flags.           */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if ((NULL == intf) || (NULL == tnl_info)) {
        return (BCM_E_PARAM);
    }

    ifindex = intf->l3a_intf_id;
    /* Validate interface index. */
    if (ifindex >= BCM_XGS3_L3_IF_TBL_SIZE(unit)) {
        return (BCM_E_PARAM);
    }

    if (!BCM_L3_INTF_USED_GET(unit, ifindex)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Validate ttl for firebolt based boards. */
    if (!BCM_TTL_VALID(tnl_info->ttl)) {
        return (BCM_E_PARAM);
    }

    /* Validate IP tunnel DSCP SEL. */
    if (tnl_info->dscp_sel > 2 || tnl_info->dscp_sel < 0) {
        return (BCM_E_PARAM);
    }

    /* Validate IP tunnel DSCP value. */
    if (tnl_info->dscp > 63 || tnl_info->dscp < 0) {
        return (BCM_E_PARAM);
    }

    /* Validate flow label value. */
    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_info->type)) {
        if (tnl_info->flow_label > (1 << 20)) {
            return (BCM_E_PARAM);
        }
    }

    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_tunnel_type_support_check(unit, tnl_info->type,
                                             NULL, &support));
    if (FALSE == support) {
        return (BCM_E_UNAVAIL);
    }


    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, if_tnl_init_set) ||
        !BCM_XGS3_L3_HWCALL_CHECK(unit, if_tnl_init_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Reset tunnel configuration buffer. */
    sal_memset(&cfg_ent, 0, sizeof(_bcm_tnl_init_data_t));

    /* Check if interface already has tunnel attached. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, if_tnl_init_get) (unit, ifindex,
                                                         &cfg_ent));

    /* Check for tunnel replacement. */
    if (cfg_ent.tnl_idx) {
        if (!(tnl_info->flags & BCM_TUNNEL_REPLACE)) {
            return (BCM_E_EXISTS);
        } 
        /* Destroy original tunnel initiator. */
        BCM_IF_ERROR_RETURN (bcm_xgs3_tunnel_initiator_clear(unit, intf));
    }

    /* Convert tunnel specific flags to table management flags. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_tunnel_flags_to_shr(tnl_info->flags, &shr_flags));

    /* Add tunnel initiator entry to hw. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_tnl_init_add(unit, shr_flags, tnl_info,
                                              &cfg_ent.tnl_idx));

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Find the index into ADJ_MAC table for tunnel DA */
        BCM_IF_ERROR_RETURN(bcm_xgs3_adj_mac_add(unit, tnl_info->dmac,
                                                   &cfg_ent.adj_mac_idx));
        cfg_ent.tnl_ttl = tnl_info->ttl;
        cfg_ent.tnl_type = tnl_info->type;
        cfg_ent.dscp_map = tnl_info->dscp_map;
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    rv = BCM_XGS3_L3_HWCALL_EXEC(unit, if_tnl_init_set) (unit, ifindex,
                                                         &cfg_ent);
    /* Do best effort clean up if needed. */
    if (rv < 0) {
#ifdef BCM_EASYRIDER_SUPPORT
        if (SOC_IS_EASYRIDER(unit)) {
            bcm_xgs3_adj_mac_del(unit, cfg_ent.adj_mac_idx);
        }
#endif /* BCM_EASYRIDER_SUPPORT */
        bcm_xgs3_tnl_init_del(unit, 0, cfg_ent.tnl_idx);
    }
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_tunnel_initiator_clear
 * Purpose:
 *      Delete the tunnel association for the given L3 interface
 * Parameters:
 *      unit - SOC unit number
 *      intf - the given L3 interface
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_xgs3_tunnel_initiator_clear(int unit, bcm_l3_intf_t *intf)
{
    _bcm_tnl_init_data_t cfg_ent;       /* Tunnel configuration entry. */
    int ifindex;                /* Interface index.            */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == intf) {
        return (BCM_E_PARAM);
    }

    ifindex = intf->l3a_intf_id;
    /* Validate interface index. */
    if (ifindex >= BCM_XGS3_L3_IF_TBL_SIZE(unit)) {
        return (BCM_E_PARAM);
    }

    if (!BCM_L3_INTF_USED_GET(unit, ifindex)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, if_tnl_init_reset) ||
        !BCM_XGS3_L3_HWCALL_CHECK(unit, if_tnl_init_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Reset tunnel configuration buffer. */
    sal_memset(&cfg_ent, 0, sizeof(_bcm_tnl_init_data_t));

    /* Read tunnel info from hw. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, if_tnl_init_get) (unit, ifindex,
                                                         &cfg_ent));

    /* Make sure tunnel index is valid. */
    if (!cfg_ent.tnl_idx) {
        return (BCM_E_NOT_FOUND);
    }

    /* Remove tunnel initiator entry to hw. */
    BCM_IF_ERROR_RETURN(bcm_xgs3_tnl_init_del(unit, 0, cfg_ent.tnl_idx));

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Free index into ADJ_MAC table for tunnel DA */
        BCM_IF_ERROR_RETURN(bcm_xgs3_adj_mac_del(unit, cfg_ent.adj_mac_idx));
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    return BCM_XGS3_L3_HWCALL_EXEC(unit, if_tnl_init_reset) (unit, ifindex);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_initiator_get
 * Purpose:
 *      Get the tunnel initiator property for the given L3 interface
 * Parameters:
 *      unit - SOC unit number
 *      intf - the given L3 interface
 *      tunnel - (OUT) the tunnel header information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_initiator_get(int unit, bcm_l3_intf_t *intf,
                                 bcm_tunnel_initiator_t *tnl_info)
{
    _bcm_tnl_init_data_t cfg_ent;       /* Tunnel configuration entry. */
    int ifindex;                /* Interface index.            */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if ((NULL == intf) || (NULL == tnl_info)) {
        return (BCM_E_PARAM);
    }

    ifindex = intf->l3a_intf_id;
    /* Validate interface index. */
    if (ifindex >= BCM_XGS3_L3_IF_TBL_SIZE(unit)) {
        return (BCM_E_PARAM);
    }

    if (!BCM_L3_INTF_USED_GET(unit, ifindex)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, if_tnl_init_get) ||
        !BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_init_get)) {
        return (BCM_E_UNAVAIL);
    }

    /* Reset tunnel configuration buffer. */
    sal_memset(&cfg_ent, 0, sizeof(_bcm_tnl_init_data_t));

    /* Read tunnel index/ (ER adj_mac_idx, ttl,type) from interface table. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, if_tnl_init_get) (unit, ifindex,
                                                         &cfg_ent));

    /* Make sure tunnel index is valid. */
    if (!cfg_ent.tnl_idx) {
        return (BCM_E_NOT_FOUND);
    }

    /* Get tunnel initiator entry info from hw. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_init_get) (unit, cfg_ent.tnl_idx,
                                                      tnl_info));

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        if (!BCM_XGS3_L3_HWCALL_CHECK(unit, adj_mac_get)) {
            return (BCM_E_UNAVAIL);
        }

        BCM_IF_ERROR_RETURN
            (BCM_XGS3_L3_HWCALL_EXEC(unit, adj_mac_get)(unit, cfg_ent.
                                                        adj_mac_idx,
                                                        tnl_info->dmac));

        if (SOC_WARM_BOOT(unit)) {
            /* Compute adj mac signature and update reference count. */
            _bcm_xgs3_adj_mac_hash_calc(unit, &tnl_info->dmac,
                   &BCM_XGS3_L3_ENT_HASH(BCM_XGS3_L3_TBL_PTR(unit, adj_mac),
                   cfg_ent.adj_mac_idx));
            BCM_XGS3_L3_ENT_REF_CNT_INC(BCM_XGS3_L3_TBL_PTR(unit, adj_mac),
                                        cfg_ent.adj_mac_idx, _BCM_SINGLE_WIDE);
        }

        /* Get tunnel TTL. */
        tnl_info->ttl = cfg_ent.tnl_ttl;

        /* Get tunnel type. */
        tnl_info->type = cfg_ent.tnl_type;

        /* Get DSCP map. */
        tnl_info->dscp_map = cfg_ent.dscp_map;
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_dscp_map_create
 * Purpose:
 *      Create a tunnel DSCP map instance.
 * Parameters:
 *      unit         - (IN)  SOC unit #
 *      flags        - (IN)  BCM_L3_XXX
 *      dscp_map_id  - (OUT) Allocated DSCP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_dscp_map_create(int unit, uint32 flags, int *dscp_map_id)
{
    int idx;              /* Iteration index.           */
    int idx_max;          /* Iteration index max value. */
 
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Input parameters check. */
    if (NULL == dscp_map_id) {
        return (BCM_E_PARAM);
    }

    if (flags & BCM_L3_WITH_ID) {
        if (BCM_L3_DSCP_MAP_USED_GET(unit, *dscp_map_id) && 
            !(flags & BCM_L3_REPLACE)) {
            return (BCM_E_EXISTS);
        }
        BCM_L3_DSCP_MAP_USED_SET(unit, *dscp_map_id);
        return (BCM_E_NONE);
    }

    idx_max = BCM_XGS3_L3_TUNNEL_DSCP_MAP_TBL_SIZE(unit);

    for (idx = 0; idx < idx_max; idx++) {
        if (!BCM_L3_DSCP_MAP_USED_GET(unit, idx)) {
            BCM_L3_DSCP_MAP_USED_SET(unit, idx);
            *dscp_map_id = idx;
            return (BCM_E_NONE);
        }
    }   
    return (BCM_E_FULL);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_dscp_map_destroy
 * Purpose:
 *      Destroy an existing tunnel DSCP map instance.
 * Parameters:
 *      unit        - (IN) SOC unit #
 *      dscp_map_id - (IN) DSCP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_dscp_map_destroy(int unit, int dscp_map_id)
{   
    int idx_max;  /* Maximum dscp map index. */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    idx_max = SOC_IS_TRX(unit) ? 1 : BCM_XGS3_L3_TUNNEL_DSCP_MAP_TBL_SIZE(unit);
    if ((dscp_map_id < 0) || (dscp_map_id >= idx_max)) {
        return (BCM_E_PARAM);
    }

    BCM_L3_DSCP_MAP_USED_CLR(unit, dscp_map_id);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_dscp_map_set
 * Purpose:
 *      Set the mapping of { internal priority, color }
 *      to a DSCP value for outer tunnel headers
 *      in the specified DSCP map instance.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      dscp_map_id  - (IN) DSCP map ID
 *      dscp_map     - (IN) DSCP map structure. 
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_tunnel_dscp_map_set(int unit, int dscp_map_id,
                             bcm_tunnel_dscp_map_t *dscp_map)
{
    int cng, index;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_dscp_set)) {
        return (BCM_E_UNAVAIL);
    }

    cng = _BCM_COLOR_ENCODING(unit, dscp_map->color);

    /* Get the base index for this DSCP map */
    index = (dscp_map_id * 32);

    /* Add the offset based on priority and color values */
    index += ((dscp_map->priority << 2) | (cng & 3));

    /* Set tunnel dscp entry info in hw. */
    return (BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_dscp_set)(unit, index, dscp_map));
}
    
/*
 * Function:
 *      bcm_xgs3_tunnel_dscp_map_get
 * Purpose:
 *      Get the mapping of { internal priority, color }
 *      to a DSCP value for outer tunnel headers
 *      in the specified DSCP map instance.
 * Parameters:
 *      unit         - (IN)  BCM device number.#
 *      dscp_map_id  - (IN)  DSCP map ID
 *      dscp_map     - (OUT) DSCP map structure.
 * Returns:
 *      BCM_E_XXX
 */     
int     
bcm_xgs3_tunnel_dscp_map_get(int unit, int dscp_map_id,
                             bcm_tunnel_dscp_map_t *dscp_map)
{
    int cng, index;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_dscp_get)) {
        return (BCM_E_UNAVAIL);
    }

    cng = _BCM_COLOR_ENCODING(unit, dscp_map->color);

    /* Get the base index for this DSCP map in EGR_DSCP_TABLEm */
    index = (dscp_map_id * 32);

    /* Add the offset based on priority and color values */
    index += ((dscp_map->priority << 2) | (cng & 3));

    /* Get tunnel dscp entry info from hw. */
    return(
        BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_dscp_get) (unit, index, dscp_map));
}

#if defined(BCM_TRX_SUPPORT)
/*
 * Function:
 *      _bcm_trx_tunnel_dscp_map_port_set
 * Purpose:
 *      Set the mapping of { internal priority, color }
 *      to a DSCP value for outer tunnel headers
 *      in the specified egress port/ports.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      port         - (IN) Egress port number. 
 *      dscp_map     - (IN) DSCP map structure. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_trx_tunnel_dscp_map_port_set(int unit, bcm_port_t port,
                             bcm_tunnel_dscp_map_t *dscp_map)
{
    int cng, index;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    if ((dscp_map == NULL) ||
        (dscp_map->priority < BCM_PRIO_MIN) ||
        (dscp_map->priority > BCM_PRIO_MAX) ||
        (dscp_map->dscp < 0) ||
        (dscp_map->dscp > 63)) {
        return (BCM_E_PARAM);
    }
   
    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_dscp_set)) {
        return (BCM_E_UNAVAIL);
    }

    cng = _BCM_COLOR_ENCODING(unit, dscp_map->color);

    /* Add the offset based on priority and color values */
    index = ((port & 0x3F) << 6) | \
            ((dscp_map->priority & 0xF)  << 2) | \
            (cng & 0x3);

    /* Set tunnel dscp entry info in hw. */
    return (BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_dscp_set)(unit, index, dscp_map));
}
    
/*
 * Function:
 *      _bcm_trx_tunnel_dscp_map_port_get
 * Purpose:
 *      Get the mapping of { internal priority, color }
 *      to a DSCP value for outer tunnel headers
 *      in the specified egress port/ports.
 * Parameters:
 *      unit         - (IN)  BCM device number.#
 *      port         - (IN) Egress port number. 
 *      dscp_map     - (OUT) DSCP map structure.
 * Returns:
 *      BCM_E_XXX
 */     
int     
_bcm_trx_tunnel_dscp_map_port_get(int unit, bcm_port_t port,
                                 bcm_tunnel_dscp_map_t *dscp_map)
{
    int cng, index;

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

    if ((dscp_map == NULL) ||
        (!SOC_PORT_ADDRESSABLE(unit, port)) ||
        (dscp_map->priority < BCM_PRIO_MIN) ||
        (dscp_map->priority > BCM_PRIO_MAX)) {
        return (BCM_E_PARAM);
    }

    /* Make sure required hw call is defined. */
    if (!BCM_XGS3_L3_HWCALL_CHECK(unit, tnl_dscp_get)) {
        return (BCM_E_UNAVAIL);
    }

    cng = _BCM_COLOR_ENCODING(unit, dscp_map->color);

    /* Add the offset based on port, priority and color values */
    index = ((port & 0x3F) << 6) | \
            ((dscp_map->priority & 0xF)  << 2) | \
            (cng & 0x3);


    /* Get tunnel dscp entry info from hw. */
    return(
        BCM_XGS3_L3_HWCALL_EXEC(unit, tnl_dscp_get) (unit, index, dscp_map));
}   
#endif /* BCM_TRX_SUPPORT */

/*
 * Function:
 *      bcm_xgs3_tunnel_config_set
 * Purpose:
 *      Set the global tunnel initiator property
 * Parameters:
 *      unit    - (IN)SOC unit number. 
 *      tconfig - (IN)L3 tunneling configuration
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_xgs3_tunnel_config_set(int unit, bcm_tunnel_config_t *tconfig)
{
    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return (BCM_E_UNAVAIL);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Input parameters check.*/
    if ((NULL == tconfig) || (tconfig->ip4_id > 0xffff) || 
        (tconfig->ip4_id < 0)) {
        return (BCM_E_PARAM);
    }

    /* Check if IPv4 ID space is global */
#if defined BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        int val;
        egr_fragment_id_table_entry_t entry;
        BCM_IF_ERROR_RETURN
            (bcm_esw_switch_control_get(unit, bcmSwitchTunnelIp4IdShared, &val));
        if (val) {
            sal_memset(&entry, 0, sizeof(egr_fragment_id_table_entry_t));
            soc_EGR_FRAGMENT_ID_TABLEm_field32_set(unit, &entry, FRAGMENT_IDf, 
                                                   tconfig->ip4_id);
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_FRAGMENT_ID_TABLEm, SOC_BLOCK_ALL, 
                               0, &entry));
        } 
    }
#endif 

    /*
     * IPv4 ID for non-triumph2 devices
     */
    if (SOC_REG_IS_VALID(unit, EGR_TUNNEL_CONTROLr)) {
        BCM_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, EGR_TUNNEL_CONTROLr, REG_PORT_ANY, 
                                    IPV4_IDf, tconfig->ip4_id));   
    }

    /*
     * PIM Sparse Mode DR1 Header
     */
    BCM_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, EGR_TUNNEL_PIMDR1_CFG0r, REG_PORT_ANY, 
                                MS_PIMSM_HDRf, tconfig->ms_pimsm_hdr1));   


    BCM_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, EGR_TUNNEL_PIMDR1_CFG1r, REG_PORT_ANY, 
                                LS_PIMSM_HDRf, tconfig->ls_pimsm_hdr1));   

    /*
     * PIM Sparse Mode DR2 Header
     */
    BCM_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, EGR_TUNNEL_PIMDR2_CFG0r, REG_PORT_ANY, 
                                MS_PIMSM_HDRf, tconfig->ms_pimsm_hdr2));   


    BCM_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, EGR_TUNNEL_PIMDR2_CFG1r, REG_PORT_ANY, 
                                LS_PIMSM_HDRf, tconfig->ls_pimsm_hdr2));   

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_xgs3_tunnel_config_get
 * Purpose:
 *      Get the global tunnel initiator property
 * Parameters:
 *      unit - SOC unit number
 *      tconfig - Global information about the L3 tunneling configuration
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_xgs3_tunnel_config_get(int unit, bcm_tunnel_config_t *tconfig)
{
    uint32 reg_val;         /* Register value */

    /*  Make sure module was initialized. */
    if (!BCM_XGS3_L3_INITIALIZED(unit)) {
        return (BCM_E_INIT);
    }

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return (BCM_E_UNAVAIL);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Input parameters check.*/
    if (NULL == tconfig) { 
        return (BCM_E_PARAM);
    }

    /* Check if IPv4 ID space is global */
#if defined BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        int val;
        egr_fragment_id_table_entry_t entry;
        BCM_IF_ERROR_RETURN
            (bcm_esw_switch_control_get(unit, bcmSwitchTunnelIp4IdShared, &val));
        if (val) {
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, EGR_FRAGMENT_ID_TABLEm, SOC_BLOCK_ANY, 
                               0, &entry));
            tconfig->ip4_id = soc_EGR_FRAGMENT_ID_TABLEm_field32_get
                                  (unit, &entry, FRAGMENT_IDf); 
        } 
    }
#endif 

    /*
     * IPv4 ID for non-triumph2 devices
     */
    if (SOC_REG_IS_VALID(unit, EGR_TUNNEL_CONTROLr)) {
        BCM_IF_ERROR_RETURN(READ_EGR_TUNNEL_CONTROLr(unit, &reg_val));
        tconfig->ip4_id =
            soc_reg_field_get(unit, EGR_TUNNEL_CONTROLr, reg_val, IPV4_IDf);
    }

    /*
     * PIM Sparse Mode DR1 Header
     */
    BCM_IF_ERROR_RETURN(READ_EGR_TUNNEL_PIMDR1_CFG0r(unit, &reg_val));
    tconfig->ms_pimsm_hdr1 = soc_reg_field_get(unit, EGR_TUNNEL_PIMDR1_CFG0r,
                                               reg_val, MS_PIMSM_HDRf);

    BCM_IF_ERROR_RETURN(READ_EGR_TUNNEL_PIMDR1_CFG1r(unit, &reg_val));
    tconfig->ls_pimsm_hdr1 = soc_reg_field_get(unit, EGR_TUNNEL_PIMDR1_CFG1r,
                                               reg_val, LS_PIMSM_HDRf);

    /*
     * PIM Sparse Mode DR2 Header
     */
    BCM_IF_ERROR_RETURN(READ_EGR_TUNNEL_PIMDR2_CFG0r(unit, &reg_val));
    tconfig->ms_pimsm_hdr2 = soc_reg_field_get(unit, EGR_TUNNEL_PIMDR2_CFG0r,
                                               reg_val, MS_PIMSM_HDRf);

    BCM_IF_ERROR_RETURN(READ_EGR_TUNNEL_PIMDR2_CFG1r(unit, &reg_val));
    tconfig->ls_pimsm_hdr2 = soc_reg_field_get(unit, EGR_TUNNEL_PIMDR2_CFG1r,
                                               reg_val, LS_PIMSM_HDRf);

    return (BCM_E_NONE);
}


/*
 * The following are not implemented for Firebolt
 */

int
bcm_xgs3_lpm_defip_sw_tbl_empty(int unit, int *count)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_xgs3_lpm_defip_hw_tbl_empty(int unit, int *count)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_xgs3_lpm_check_table(int unit)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_xgs3_lpm_delete(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_xgs3_lpm_insert(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_xgs3_lpm_lookup(int unit, ip_addr_t iproute, _bcm_defip_cfg_t *lpm_cfg)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_xgs3_ip6_defip_find_index(int unit, int index, bcm_l3_route_t *lpm_cfg)
{
    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *      _bcm_xgs3_ecmp_grp_get
 * Purpose:
 *      Get ecmp group next hop members by index.
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_index - (IN)Ecmp group id to read. 
 *      ecmp_count - (IN)Maximum number of entries to read.
 *      nh_idx     - (OUT)Next hop indexes. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ecmp_grp_get(int unit, int ecmp_idx, int ecmp_count, int *nh_idx)
{
    int idx;                                /* Iteration index.              */
    int max_ent_count;                      /* Number of entries to read.    */
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry.      */
    int one_entry_grp = TRUE;               /* Single next hop entry group.  */ 
    int rv = BCM_E_UNAVAIL;                 /* Operation return status.      */

    /* Input parameters sanity check. */
    if ((NULL == nh_idx) || (ecmp_count < 1)) {
        return (BCM_E_PARAM);
    }

    /* Zero all next hop indexes first. */
    sal_memset(nh_idx, 0, ecmp_count * sizeof(int));
    sal_memset(hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {
        /* Get group base pointer. */
        if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
             idx = ecmp_idx;
        } else {
              if (soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
                     idx = ecmp_idx;
              } else {
                     idx = ecmp_idx * BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
               }
       }

        /* Read zero based ecmp count. */
        rv = soc_mem_read(unit, L3_ECMP_COUNTm, MEM_BLOCK_ANY, idx, hw_buf);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMP_COUNTm)) {
            rv = soc_mem_read(unit, INITIAL_L3_ECMP_COUNTm, MEM_BLOCK_ANY, idx, hw_buf);
            BCM_IF_ERROR_RETURN(rv);
        }

        max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, COUNTf);
        if (!BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
            ecmp_idx *= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        }
        max_ent_count++; /* Count is zero based. */ 
    } else 
#endif  /* BCM_TRX_SUPPORT */
    {
        ecmp_idx *= BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
        /* Check number of entries to read. */ 
        max_ent_count = (ecmp_count < BCM_XGS3_L3_ECMP_MAX_PATHS(unit)) ?  \
                        ecmp_count : BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
    }

    /* Read all the indexes from hw. */
    for (idx = 0; idx < max_ent_count; idx++) {

        /* Read next hop index. */
        rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, 
                          (ecmp_idx + idx), hw_buf);
        if (rv < 0) {
            break;
        }
        nh_idx[idx] = soc_mem_field32_get(unit, L3_ECMPm, 
                                          hw_buf, NEXT_HOP_INDEXf);
        /* Check if group contains . */ 
        if (idx && (nh_idx[idx] != nh_idx[0])) { 
            one_entry_grp = FALSE;
        }

        if (0 == soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
             /* Next hops popuplated in cycle,stop once you read first entry again */
             if (idx && (FALSE == one_entry_grp) && (nh_idx[idx] == nh_idx[0])) {
                 nh_idx[idx] = 0;
                 break;
             }
        } else {
             one_entry_grp = FALSE;
        }
    }
    /* Reset rest of the group if only 1 next hop is present. */
    if (one_entry_grp) {
       sal_memset(nh_idx + 1, 0, (ecmp_count - 1) * sizeof(int)); 
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_ecmp_grp_add
 * Purpose:
 *      Add ecmp group next hop members, or reset ecmp group entry.  
 *      NOTE: Function always writes all the entries in ecmp group.
 *            If there is not enough nh indexes - next hops written
 *            in cycle. 
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp   - (IN)ecmp group id to write.
 *      buf        - (IN)Next hop indexes or NULL for entry reset.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ecmp_grp_add(int unit, int ecmp_grp, void *buf, int max_paths)
{
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry.*/
    int max_grp_size=0;                       /* Maximum ecmp group size.*/
    int ecmp_idx;                           /* Ecmp table entry index. */
    int *nh_idx;                            /* Ecmp group nh indexes.  */
    int nh_cycle_idx;                       /* NH cycle index.         */
    int idx=0;                                /* Iteration index.        */
    int rv = BCM_E_NONE;                 /* Operation return value. */

    /* Input parameters check. */
    if (NULL == buf) {
        return (BCM_E_PARAM);
    }

    /* Cast input buffer. */
    nh_idx = (int *) buf;
#ifdef BCM_TRX_SUPPORT
    if ((SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) && (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))) {
         max_grp_size = max_paths;
         ecmp_idx = ecmp_grp;
    } else
#endif  /* BCM_TRX_SUPPORT */
   {
       max_grp_size = BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
       /* Calculate table index. */
       ecmp_idx = ecmp_grp * BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
   }

    /* Write all the indexes to hw. */
    for (idx = 0, nh_cycle_idx = 0; idx < max_grp_size; idx++, nh_cycle_idx++) {

        /* Set next hop index. */
        sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));


        /* If this is the last nhop then program black-hole. */
        if ( (!idx) && (!nh_idx[nh_cycle_idx]) ) {
              nh_cycle_idx = 0;
        } else  if (!nh_idx[nh_cycle_idx]) {
        /* If next hop index is not set start from the beginning. */
#ifdef BCM_TRX_SUPPORT
            if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {
                /* No entry replication is required. */
                break;
            }
#endif  /* BCM_TRX_SUPPORT */
            nh_cycle_idx = 0;
        }
        soc_mem_field32_set(unit, L3_ECMPm, hw_buf, NEXT_HOP_INDEXf,
                            nh_idx[nh_cycle_idx]);
        /* Write buffer to hw. */
        rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL, 
                           (ecmp_idx + idx), hw_buf);
        if (BCM_FAILURE(rv)) {
            break;
        }
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
        defined(BCM_TRX_SUPPORT)
        /* Write initial ecmp table. */
        if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMPm)) {
            /* Write buffer to hw. */
            rv = soc_mem_write(unit, INITIAL_L3_ECMPm, MEM_BLOCK_ALL, 
                                        (ecmp_idx + idx), hw_buf);
            if (BCM_FAILURE(rv)) {
                break;
            }
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    }
#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm) && BCM_SUCCESS(rv)) {
      if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
        /* Set Max Group Size. */
        sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));
        soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNTf, max_grp_size - 1);

        rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                           (ecmp_idx+1), hw_buf);
        BCM_IF_ERROR_RETURN(rv);
      }

        /* Set zero based ecmp count. */
        sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));
        if (!idx) {
            soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNTf, idx);
        } else {
            soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNTf, idx - 1);
        }

        /* Set group base pointer. */
        if (SOC_MEM_FIELD_VALID(unit, L3_ECMP_COUNTm, BASE_PTRf)) {
            soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, 
                                BASE_PTRf, ecmp_idx);
        }

        rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                           ecmp_idx, hw_buf);
        BCM_IF_ERROR_RETURN(rv);

        if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMP_COUNTm)) {
            rv = soc_mem_write(unit, INITIAL_L3_ECMP_COUNTm,
                               MEM_BLOCK_ALL, ecmp_idx, hw_buf);
        }
    }
#endif  /* BCM_TRX_SUPPORT */
    return rv;
}


/*
 * Function:
 *      _bcm_xgs3_ecmp_grp_del
 * Purpose:
 *      Reset ecmp group next hop members
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp   - (IN)ecmp group id to write.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ecmp_grp_del(int unit, int ecmp_grp, int max_paths)
{
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry.*/
    int ecmp_idx;               /* Ecmp table entry index. */
    int idx;                    /* Iteration index.        */
    int rv = BCM_E_UNAVAIL;     /* Operation return value. */
    int max_grp_size=0;          /* Maximum ecmp group size.*/


    /* Initialize ecmp entry. */
        sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

       /* Calculate table index. */
#ifdef BCM_TRX_SUPPORT
       if ((SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) && (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit))){
            max_grp_size = max_paths;
            ecmp_idx = ecmp_grp;
       } else
#endif
      {
          max_grp_size = BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
          ecmp_idx = ecmp_grp * BCM_XGS3_L3_ECMP_MAX_PATHS(unit);
       }

    /* Write all the indexes to hw. */
    for (idx = 0; idx < max_grp_size; idx++) {
        /* Write buffer to hw. */
        rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL, 
                           (ecmp_idx + idx), hw_buf);

        if (BCM_FAILURE(rv)) {
            return rv;
        }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
        defined(BCM_TRX_SUPPORT)
        /* Write initial ecmp table. */
        if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMPm)) {
            /* Write buffer to hw. */
            rv = soc_mem_write(unit, INITIAL_L3_ECMPm, MEM_BLOCK_ALL, 
                           (ecmp_idx + idx), hw_buf);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    }
#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {
        /* Set group base pointer. */
        if (soc_feature(unit, soc_feature_l3_dynamic_ecmp_group)) {
            ecmp_idx = ecmp_grp;
        }

        rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                           ecmp_idx, hw_buf);
        BCM_IF_ERROR_RETURN(rv);

        rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                           (ecmp_idx+1), hw_buf);
        BCM_IF_ERROR_RETURN(rv);

        if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMP_COUNTm)) {
            rv = soc_mem_write(unit, INITIAL_L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                           ecmp_idx, hw_buf);
        }

    }
#endif  /* BCM_TRX_SUPPORT */

    return rv;
}


/*
 * Function:
 *      _bcm_xgs3_ecmp_max_grp_size_set
 * Purpose:
 *      Set size of maximum ecmp group for sw reload. 
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      grp_size   - (IN)Maximum Number of entries in the group.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_ecmp_max_grp_size_set(int unit, int grp_size)
{
    ecmp_entry_t ecmp_entry;
    int rv;

    /* Reset write buffer. */
    sal_memset(&ecmp_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, ecmp));

    /* Set maximum group size. */
    soc_L3_ECMPm_field32_set(unit, &ecmp_entry, NEXT_HOP_INDEXf, grp_size);

    /* Write buffer to last entry in the table. */
    rv = WRITE_L3_ECMPm(unit, MEM_BLOCK_ALL,
                        (BCM_XGS3_L3_ECMP_TBL_SIZE(unit) - 1), &ecmp_entry);
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_term_entry_init
 * Purpose:
 *      Initialize soc tunnel terminator entry key portion.
 * Parameters:
 *      unit     - (IN)  BCM device number. 
 *      tnl_info - (IN)  BCM buffer with tunnel info.
 *      entry    - (OUT) SOC buffer with key filled in.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_term_entry_init(int unit, bcm_tunnel_terminator_t *tnl_info,
                                 soc_tunnel_term_t *entry)
{
    int       idx;                /* Entry iteration index.     */
    int       idx_max;            /* Entry widht.               */
    uint32    *entry_ptr;         /* Filled entry pointer.      */
    _bcm_tnl_term_type_t tnl_type;/* Tunnel type.               */
    soc_field_t vrf_fld;          /* VRF id field name.         */
    int         rv;               /* Operation return status.   */

    /* Input parameters check. */
    if ((NULL == tnl_info) || (NULL == entry)) {
        return (BCM_E_PARAM);
    }

    /* Get tunnel type & sub_type. */
    rv = _bcm_xgs3_l3_set_tnl_term_type(unit, tnl_info, &tnl_type);
    BCM_IF_ERROR_RETURN(rv);

    /* Reset destination structure. */
    sal_memset(entry, 0, sizeof(soc_tunnel_term_t));

    /* Set Destination/Source pair. */
    entry_ptr = (uint32 *)&entry->entry_arr[0];
    if (tnl_type.tnl_outer_hdr_ipv6) {
#if defined(BCM_TRX_SUPPORT)
        /* Apply mask on source address. */
        rv = bcm_xgs3_l3_mask6_apply(tnl_info->sip6_mask, tnl_info->sip6);
        BCM_IF_ERROR_RETURN(rv);

        /* SIP [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[0], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[1], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[2], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[3], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_UPPER_ONLY);

        /* SIP MASK [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm,
                             (uint32 *)&entry->entry_arr[0], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP MASK [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm,
                             (uint32 *)&entry->entry_arr[1], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP MASK [0-63] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm,
                             (uint32 *)&entry->entry_arr[2], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP MASK [64-127] */
        soc_mem_ip6_addr_set(unit, L3_TUNNELm,
                             (uint32 *)&entry->entry_arr[3], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_UPPER_ONLY);
#endif /* BCM_TRX_SUPPORT */
    }  else {
        tnl_info->sip &= tnl_info->sip_mask;

        /* Set destination ip. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, DIPf, tnl_info->dip);

        /* Set source ip. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, SIPf, tnl_info->sip);
#if defined(BCM_FIREBOLT_SUPPORT)
        if (SOC_IS_FBX(unit)) {
            /* Set destination subnet mask. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, DIP_MASKf,
                                       tnl_info->dip_mask);

            /* Set source subnet mask. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, SIP_MASKf,
                                       tnl_info->sip_mask);
        }
#endif /* BCM_FIREBOLT_SUPPORT */
    }

    /* Resolve vrf field name. */
#if defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, VRFf)) {
        vrf_fld = VRFf;
    } else
#endif /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) ||\
        defined(BCM_SCORPION_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, TUNNEL_VRF_IDf)) {
        vrf_fld = TUNNEL_VRF_IDf;
        if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, TUNNEL_VRF_OVERRIDEf)) {
             soc_L3_TUNNELm_field32_set(unit, entry_ptr, TUNNEL_VRF_OVERRIDEf, 1);
        }
    } else
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_SCORPION_SUPPORT */
    {
        vrf_fld = INVALIDf;
    }

    /* Resolve number of entries hw entry occupies. */
    idx_max = (tnl_type.tnl_outer_hdr_ipv6) ? SOC_TNL_TERM_IPV6_ENTRY_WIDTH : \
              SOC_TNL_TERM_IPV4_ENTRY_WIDTH;

    for (idx = 0; idx < idx_max; idx++) {
        entry_ptr = (uint32 *) &entry->entry_arr[idx];

        /* Set valid bit. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, VALIDf, 1);

        /* Set tunnel subtype. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, SUB_TUNNEL_TYPEf,
                                   tnl_type.tnl_sub_type);

        /* Set tunnel type. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, TUNNEL_TYPEf,
                                   tnl_type.tnl_auto);

#if defined(BCM_EASYRIDER_SUPPORT) 
        if (SOC_IS_EASYRIDER(unit)) {
            /* Set tunnel type is GRE bit. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, GRE_TUNNELf,
                                       tnl_type.tnl_gre);
            /* UDP destination port. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, UDP_DST_PORTf,
                                       tnl_info->udp_dst_port);

            /* UDP source port. */
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, UDP_SRC_PORTf, 
                                       tnl_info->udp_src_port);
        }
#endif /* BCM_EASYRIDER_SUPPORT */

#if defined(BCM_TRX_SUPPORT)
        if (SOC_IS_FBX(unit)) {
            /* Set entry mode. */
            if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, MODEf)) {
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, MODEf,
                                           tnl_type.tnl_outer_hdr_ipv6);
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, MODE_MASKf, 1);
            }

            /* Set tunnel type is GRE bit. */
            if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, GRE_TUNNELf)) {
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, GRE_TUNNELf,
                                           tnl_type.tnl_gre);
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, GRE_TUNNEL_MASKf, 1);

                if (tnl_type.tnl_gre) {
                    /* GRE IPv6 payload is allowed. */
                    soc_L3_TUNNELm_field32_set(unit, entry_ptr, GRE_PAYLOAD_IPV6f,
                                               tnl_type.tnl_gre_v6_payload);

                    /* GRE IPv6 payload is allowed. */
                    soc_L3_TUNNELm_field32_set(unit, entry_ptr, GRE_PAYLOAD_IPV4f,
                                               tnl_type.tnl_gre_v4_payload);
                }
            }
        }
#endif  /* BCM_TRX_SUPPORT */

        /* Set entry vrf id. */
        if (INVALIDf != vrf_fld) {
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, vrf_fld, tnl_info->vrf);
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_term_entry_parse
 * Purpose:
 *      Parse tunnel terminator entry portion.
 * Parameters:
 *      unit     - (IN)  BCM device number. 
 *      entry    - (IN)  SOC buffer with tunne information.  
 *      tnl_info - (OUT) BCM buffer with tunnel info.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_term_entry_parse(int unit, soc_tunnel_term_t *entry,
                                  bcm_tunnel_terminator_t *tnl_info)
{
    _bcm_tnl_term_type_t tnl_type;  /* Tunnel type information.   */
    uint32 *entry_ptr;              /* Filled entry pointer.      */
    soc_field_t vrf_fld = INVALIDf; /* Vrf id field if supported. */

    /* Input parameters check. */
    if ((NULL == tnl_info) || (NULL == entry)) {
        return (BCM_E_PARAM);
    }

    /* Reset destination structure. */
    sal_memset(tnl_info, 0, sizeof(bcm_tunnel_terminator_t));
    sal_memset(&tnl_type, 0, sizeof(_bcm_tnl_term_type_t));

    entry_ptr = (uint32 *)&entry->entry_arr[0];

    /* Get valid bit. */
    if (!soc_L3_TUNNELm_field32_get(unit, entry_ptr, VALIDf)) {
        return (BCM_E_NOT_FOUND);
    }

    if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, MODEf)) {
        tnl_type.tnl_outer_hdr_ipv6 =
            soc_L3_TUNNELm_field32_get(unit, entry_ptr, MODEf);
    }
     

    /* Get Destination/Source pair. */
    if (tnl_type.tnl_outer_hdr_ipv6) {
#if defined(BCM_TRX_SUPPORT)
        /* SIP [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[0], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[1], IP_ADDRf,
                             tnl_info->sip6, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[2], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[3], IP_ADDRf,
                             tnl_info->dip6, SOC_MEM_IP6_UPPER_ONLY);

        /* SIP MASK [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[0], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* SIP MASK [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[1], IP_ADDR_MASKf,
                             tnl_info->sip6_mask, SOC_MEM_IP6_UPPER_ONLY);
        /* DIP MASK [0-63] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[2], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_LOWER_ONLY);
        /* DIP MASK [64-127] */
        soc_mem_ip6_addr_get(unit, L3_TUNNELm, 
                             (uint32 *)&entry->entry_arr[3], IP_ADDR_MASKf,
                             tnl_info->dip6_mask, SOC_MEM_IP6_UPPER_ONLY);
#endif /* BCM_TRX_SUPPORT */

    }  else {
        /* Get destination ip. */
        tnl_info->dip = soc_L3_TUNNELm_field32_get(unit, entry_ptr, DIPf);

        /* Get source ip. */
        tnl_info->sip = soc_L3_TUNNELm_field32_get(unit, entry_ptr, SIPf);

        /* Destination subnet mask. */
        tnl_info->dip_mask = BCM_XGS3_L3_IP4_FULL_MASK;

        /* Source subnet mask. */
        if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, SIP_MASKf)) {
            tnl_info->sip_mask = soc_L3_TUNNELm_field32_get(unit, entry_ptr,
                                                            SIP_MASKf);
        } else {
            tnl_info->sip_mask = (tnl_info->sip == 0) ? 0x0 : 
                BCM_XGS3_L3_IP4_FULL_MASK;
        }
    }

    /* Get tunnel subtype. */
    tnl_type.tnl_sub_type = 
        soc_L3_TUNNELm_field32_get(unit, entry_ptr, SUB_TUNNEL_TYPEf);

    /* Get tunnel type. */
    tnl_type.tnl_auto = 
        soc_L3_TUNNELm_field32_get(unit, entry_ptr, TUNNEL_TYPEf);

    /* Copy DSCP from outer header flag. */
    if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, USE_OUTER_HDR_DSCPf)) {
        tnl_info->flags |= BCM_TUNNEL_TERM_USE_OUTER_DSCP;
    }
    /* Copy TTL from outer header flag. */
    if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, USE_OUTER_HDR_TTLf)) {
        tnl_info->flags |= BCM_TUNNEL_TERM_USE_OUTER_TTL;
    }
    /* Keep inner header DSCP flag. */
    if (soc_L3_TUNNELm_field32_get(unit, entry_ptr,
                                   DONOT_CHANGE_INNER_HDR_DSCPf)) {
        tnl_info->flags |= BCM_TUNNEL_TERM_KEEP_INNER_DSCP;
    }

    soc_mem_pbmp_field_get(unit, L3_TUNNELm, entry_ptr, ALLOWED_PORT_BITMAPf,
                           &tnl_info->pbmp);

    /* IPMC lokup  vlan id.. */
    if(SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, IINTFf)) {
        tnl_info->vlan = soc_L3_TUNNELm_field32_get(unit, entry_ptr, IINTFf);
    }

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
#if defined (BCM_RAVEN_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) ||  \
    defined (BCM_SCORPION_SUPPORT)
        /* Set vrf id field. */
        vrf_fld = TUNNEL_VRF_IDf;
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_SCORPION_SUPPORT */

        /*  Get trust dscp per tunnel */ 
        if (soc_feature(unit, soc_feature_tunnel_dscp_trust)) {
            if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, USE_TUNNEL_DSCPf)) {
                tnl_info->flags |= BCM_TUNNEL_TERM_DSCP_TRUST;
            }
        }
#ifdef BCM_TRX_SUPPORT
        /* Get gre tunnel indicator. */
        if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, GRE_TUNNELf)){
            tnl_type.tnl_gre = 
                soc_L3_TUNNELm_field32_get(unit, entry_ptr, GRE_TUNNELf);
        }

        /* Get gre IPv4 payload allowed. */
        if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, GRE_PAYLOAD_IPV4f)){
            tnl_type.tnl_gre_v4_payload = 
                soc_L3_TUNNELm_field32_get(unit, entry_ptr, GRE_PAYLOAD_IPV4f); 
        }

        /* Get gre IPv6 payload allowed. */
        if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, GRE_PAYLOAD_IPV6f)){
            tnl_type.tnl_gre_v6_payload = 
                soc_L3_TUNNELm_field32_get(unit, entry_ptr, GRE_PAYLOAD_IPV6f);
        }
#endif  /* BCM_TRX_SUPPORT */
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Set vrf id field. */
        vrf_fld = VRFf;

        /* UDP destination port. */
        tnl_info->udp_dst_port =
            soc_L3_TUNNELm_field32_get(unit, entry_ptr, UDP_DST_PORTf);

        /* UDP source port. */
        tnl_info->udp_src_port =
            soc_L3_TUNNELm_field32_get(unit, entry_ptr, UDP_SRC_PORTf);

        /* GRE tunnel flag. */
        if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, GRE_TUNNELf)) {
            tnl_type.tnl_gre = TRUE; 
            if (tnl_type.tnl_sub_type & 0x2) {
                tnl_type.tnl_gre_v4_payload = 0x1;
            } 
            if (tnl_type.tnl_sub_type & 0x1) {
                tnl_type.tnl_gre_v6_payload = 0x1;
            } 
        }

        /* UDP tunnel flag. */
        if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, UDP_TUNNELf)) {
            tnl_type.tnl_udp = TRUE; 
        }

        /* PIM RP tunnel flag. */
        if (soc_L3_TUNNELm_field32_get(unit, entry_ptr, PIM_RPf)) {
            tnl_type.tnl_pim_sm = TRUE; 
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    /* Get tunnel type & sub_type. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_l3_get_tnl_term_type(unit, tnl_info, &tnl_type));

    /* Get vrf id. */
    if (SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, vrf_fld)){
        tnl_info->vrf = soc_L3_TUNNELm_field32_get(unit, entry_ptr, vrf_fld);
    } else {
        tnl_info->vrf = BCM_L3_VRF_DEFAULT;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_term_get
 * Purpose:
 *      Get a tunnel terminator for DIP-SIP/ for ER UDP dst/src port.
 * Parameters:
 *      unit     -(IN)SOC unit number. 
 *      tnl_info -(IN)Buffer to fill tunnel info.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_term_get(int unit, bcm_tunnel_terminator_t *tnl_info)
{
    soc_tunnel_term_t key;      /* Buffer to fill lookup key.    */
    soc_tunnel_term_t entry;    /* Buffer to read entry from hw. */

    /* Initialize lookup key. */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_tr2_l3_tnl_term_entry_init(unit, tnl_info, &key));
    } else
#endif
    {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_l3_tnl_term_entry_init(unit, tnl_info, &key));
    }

    /* Find tunnel terminator in TCAM. */
    BCM_IF_ERROR_RETURN(soc_tunnel_term_match(unit,&key, &entry));

    /* Parse lookup result. */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        BCM_IF_ERROR_RETURN
          (_bcm_tr2_l3_tnl_term_entry_parse(unit, &entry, tnl_info));
    } else
#endif
        BCM_IF_ERROR_RETURN
          (_bcm_xgs3_l3_tnl_term_entry_parse(unit, &entry, tnl_info));

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_l3_tnl_term_add
 * Purpose:
 *      Add tunnel termination entry to the hw.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      tnl_info - (IN)Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_term_add(int unit, bcm_tunnel_terminator_t *tnl_info)
{
    soc_tunnel_term_t entry;        /* Buffer to writh to hw.     */
    uint32 *entry_ptr;              /* Filled entry pointer.      */
    int tmp_val;                    /* Temp value to check flags. */
    uint32 index;                   /* HW index.                  */
    int rv;                         /* Operation return status.   */

    /* Initialize lookup key. */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        rv = _bcm_tr2_l3_tnl_term_entry_init(unit, tnl_info, &entry);
    } else
#endif
    {
        rv =  _bcm_xgs3_l3_tnl_term_entry_init(unit, tnl_info, &entry);
    }
    BCM_IF_ERROR_RETURN(rv);

    entry_ptr = (uint32 *)&entry.entry_arr[0];

    /* Set flag to use outer header DSCP value or not. */
    tmp_val = (tnl_info->flags & BCM_TUNNEL_TERM_USE_OUTER_DSCP) ? 1 : 0;
    soc_L3_TUNNELm_field32_set(unit, entry_ptr, USE_OUTER_HDR_DSCPf, tmp_val);

    /* Set flag to use outer header TTL value or not. */
    tmp_val = (tnl_info->flags & BCM_TUNNEL_TERM_USE_OUTER_TTL) ? 1 : 0;
    soc_L3_TUNNELm_field32_set(unit, entry_ptr, USE_OUTER_HDR_TTLf, tmp_val);

    /* Set flag to preserve inner header DSCP value or not. */
    tmp_val = (tnl_info->flags & BCM_TUNNEL_TERM_KEEP_INNER_DSCP) ? 1 : 0;
    soc_L3_TUNNELm_field32_set(unit, entry_ptr,
                               DONOT_CHANGE_INNER_HDR_DSCPf, tmp_val);

    /* Set l3 port bitmap. */
    soc_mem_pbmp_field_set(unit, L3_TUNNELm, entry_ptr, ALLOWED_PORT_BITMAPf,
                           &tnl_info->pbmp);

    /* Sev vlan id for ipmc lookup.*/
    if(SOC_MEM_FIELD_VALID(unit, L3_TUNNELm, IINTFf)) {
        soc_L3_TUNNELm_field32_set(unit, entry_ptr, IINTFf, tnl_info->vlan);
    }

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        if (soc_feature(unit, soc_feature_tunnel_dscp_trust)) {
            if (tnl_info->flags & BCM_TUNNEL_TERM_DSCP_TRUST) {
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, USE_TUNNEL_DSCPf, 1);
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Set udp destination port. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr,
                                   UDP_DST_PORTf, tnl_info->udp_dst_port);

        /* Set udp source port. */
        soc_L3_TUNNELm_field32_set(unit, entry_ptr,
                                   UDP_SRC_PORTf, tnl_info->udp_src_port);

        if (bcmTunnelTypeUdp == tnl_info->type) {
            soc_L3_TUNNELm_field32_set(unit, entry_ptr, UDP_TUNNELf, 1);
            if ((bcmTunnelTypePimSmDr1 == tnl_info->type) ||
                (bcmTunnelTypePimSmDr2 == tnl_info->type))
                soc_L3_TUNNELm_field32_set(unit, entry_ptr, PIM_RPf, 1);
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        rv = _bcm_tr2_l3_tnl_term_add(unit, entry_ptr, tnl_info);
        BCM_IF_ERROR_RETURN(rv);
    }
#endif

    rv = soc_tunnel_term_insert(unit, &entry, &index);
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_term_del
 * Purpose:
 *      Write a singe tunnel termination entry to the hw.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      tnl_info - (IN)Tunnel terminator parameters. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_term_del(int unit, bcm_tunnel_terminator_t *tnl_info)
{
    soc_tunnel_term_t key;      /* Buffer to read entry from hw. */
    int rv;                     /* Operation return status.      */

    /* Initialize lookup key. */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        rv = _bcm_tr2_l3_tnl_term_entry_init(unit, tnl_info, &key);
    } else
#endif
    {
        rv =  _bcm_xgs3_l3_tnl_term_entry_init(unit, tnl_info, &key);
    }
    BCM_IF_ERROR_RETURN(rv);

    /* Find tunnel terminator in TCAM. */
    return soc_tunnel_term_delete(unit, &key);
}


/*
 * Function:
 *      _bcm_xgs3_l3_tnl_init_get
 * Purpose:
 *      Get a tunnel initiator entry. 
 * Parameters:
 *      unit     -(IN)SOC unit number. 
 *      idx      -(IN)Entry index to read.
 *      tnl_info -(IN)Buffer to fill tunnel info.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_init_get(int unit, int idx, 
                          bcm_tunnel_initiator_t *tnl_info)
{
    uint32 tnl_entry[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry.  */
    soc_mem_t mem;                         /* Tunnel initiator table memory.*/  
    int df_val;                            /* Don't fragment encoded value. */
    int rv;                                /* Operation return status.      */
#ifdef BCM_FIREBOLT_SUPPORT
    int tnl_type;                          /* Tunnel type.                  */
#endif /* BCM_FIREBOLT_SUPPORT */  
    uint32 entry_type = BCM_XGS3_TUNNEL_INIT_V4;/* Entry type (IPv4/IPv6/MPLS)*/

    /* Get table memory. */
    mem = BCM_XGS3_L3_MEM(unit, tnl_init_v4); 

    rv = BCM_XGS3_MEM_READ(unit, mem, idx, tnl_entry); 
    BCM_IF_ERROR_RETURN(rv);

#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, mem, ENTRY_TYPEf)) {
        /* Get entry_type. */
        entry_type = soc_mem_field32_get(unit, mem, tnl_entry, ENTRY_TYPEf); 

        if (BCM_XGS3_TUNNEL_INIT_V6 == entry_type) {
            mem = BCM_XGS3_L3_MEM(unit, tnl_init_v6); 
            idx >>= 1;    /* Each record takes two slots. */

            rv = BCM_XGS3_MEM_READ(unit, mem, idx, tnl_entry); 
            BCM_IF_ERROR_RETURN(rv);
        } else if (BCM_XGS3_TUNNEL_INIT_MPLS == entry_type) {
             mem = BCM_XGS3_L3_MEM(unit, tnl_init_mpls);
             rv = BCM_XGS3_MEM_READ(unit, mem, idx, tnl_entry);
             BCM_IF_ERROR_RETURN(rv); 
        }
    }
#endif /* BCM_TRX_SUPPORT */

    /* Parse hw buffer. */
    if (BCM_XGS3_TUNNEL_INIT_V4 == entry_type) {
        /* Get destination ip. */
        tnl_info->dip = soc_mem_field32_get(unit, mem, tnl_entry, DIPf); 

        /* Get source ip. */
        tnl_info->sip = soc_mem_field32_get(unit, mem, tnl_entry, SIPf);
    } else if (BCM_XGS3_TUNNEL_INIT_V6 == entry_type) {
        /* Get destination ip. */
        soc_mem_ip6_addr_get(unit, mem, tnl_entry, DIPf, tnl_info->dip6,
                             SOC_MEM_IP6_FULL_ADDR);

        /* Get source ip. */
        soc_mem_ip6_addr_get(unit, mem, tnl_entry, SIPf, tnl_info->sip6,
                             SOC_MEM_IP6_FULL_ADDR);
    }

    if (entry_type != BCM_XGS3_TUNNEL_INIT_MPLS) {
        /* Tunnel header DSCP select. */
        tnl_info->dscp_sel = soc_mem_field32_get(unit, mem, tnl_entry, DSCP_SELf);

        /* Tunnel header DSCP value. */
        tnl_info->dscp = soc_mem_field32_get(unit, mem, tnl_entry, DSCPf);
    }

    if (SOC_MEM_FIELD_VALID(unit, mem, IPV4_DF_SELf)) {
        /* IP tunnel hdr DF bit for IPv4 */
        df_val = soc_mem_field32_get(unit, mem, tnl_entry, IPV4_DF_SELf);
        if (0x2 <= df_val) {
            tnl_info->flags |= BCM_TUNNEL_INIT_USE_INNER_DF;  
        } else if (0x1 == df_val) {
            tnl_info->flags |= BCM_TUNNEL_INIT_IPV4_SET_DF;  
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, mem, IPV6_DF_SELf)) {
        /* IP tunnel hdr DF bit for IPv6 */
        if (soc_mem_field32_get(unit, mem, tnl_entry, IPV6_DF_SELf)) {
            tnl_info->flags |= BCM_TUNNEL_INIT_IPV6_SET_DF;  
        }
    }

#ifdef BCM_FIREBOLT_SUPPORT
    if(SOC_IS_FBX(unit)) {
       if (entry_type == BCM_XGS3_TUNNEL_INIT_MPLS) {
           /* Get TTL. */
           tnl_info->ttl = soc_mem_field32_get(unit, mem, tnl_entry, MPLS_TTL_0f);
           /* Get tunnel type. */
           tnl_type = soc_mem_field32_get(unit, mem, tnl_entry, ENTRY_TYPEf);
       } else {
           /* Get TTL. */
           tnl_info->ttl = soc_mem_field32_get(unit, mem, tnl_entry, TTLf);

           /* Get tunnel type. */
           tnl_type = soc_mem_field32_get(unit, mem, tnl_entry, TUNNEL_TYPEf);
       }

        /* Translate hw tunnel type into API encoding. */
        rv = _bcm_xgs3_tnl_hw_code_to_type(unit, tnl_type, entry_type, 
                                           &tnl_info->type);
        BCM_IF_ERROR_RETURN(rv);
#if defined(BCM_TRX_SUPPORT)
        if (SOC_MEM_FIELD_VALID(unit, mem, FLOW_LABELf)) {
            tnl_info->flow_label = soc_mem_field32_get(unit, mem, tnl_entry, 
                                                       FLOW_LABELf);
        }
#endif /* BCM_TRX_SUPPORT*/

       if (entry_type != BCM_XGS3_TUNNEL_INIT_MPLS) {
        /* Get mac address. */
        soc_mem_mac_addr_get(unit, mem, tnl_entry, DEST_ADDRf, tnl_info->dmac);
       } 
    }
#if defined BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        int val;
        egr_fragment_id_table_entry_t entry;

        rv = bcm_esw_switch_control_get(unit, bcmSwitchTunnelIp4IdShared, &val);
        BCM_IF_ERROR_RETURN(rv);
        if (!val) {
            rv = soc_mem_read(unit, EGR_FRAGMENT_ID_TABLEm, SOC_BLOCK_ANY, 
                              idx, &entry);
            BCM_IF_ERROR_RETURN(rv);
            tnl_info->ip4_id = soc_EGR_FRAGMENT_ID_TABLEm_field32_get
                                   (unit, &entry, FRAGMENT_IDf); 
        } else {
            return BCM_E_CONFIG;
        }
    }
#endif 

#endif /* BCM_FIREBOLT_SUPPORT */

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_init_add
 * Purpose:
 *      Add tunnel initiator entry to hw. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      idx      - (IN)Index to insert tunnel initiator index.
 *      buf      - (IN)Tunnel initiator entry info.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_init_add(int unit, int idx, void *buf, int max_paths)
{
    uint32 tnl_entry[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to write hw entry.*/
    bcm_tunnel_initiator_t *tnl_info;     /* Tunnel initiator structure.   */
    soc_mem_t mem;                        /* Tunnel initiator table memory.*/  
    int df_val;                           /* Don't fragment encoding.      */
    int ipv6;                             /* IPv6 tunnel initiator.        */
#ifdef BCM_FIREBOLT_SUPPORT
    int entry_type = 0;                   /* HW entry type IPv4/IPv6/MPLS  */
    int hw_type_code = 0;                 /* Tunnel type.                  */
#endif /* BCM_FIREBOLT_SUPPORT */  

    /* Input parameters check. */
    if (NULL == buf) {
        return (BCM_E_PARAM);
    }

    /* Read buffer from hw. */
    tnl_info = (bcm_tunnel_initiator_t *) buf;

    ipv6 = _BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_info->type);

    
    /* Get table memory. */
    if (ipv6) {
        idx >>= 1;
        mem = BCM_XGS3_L3_MEM(unit, tnl_init_v6);
    } else {
        mem =  BCM_XGS3_L3_MEM(unit, tnl_init_v4); 
    }

    if (!SOC_MEM_IS_VALID(unit, mem)) {
        return (BCM_E_UNAVAIL);   
    }

    /* Zero write buffer. */
    sal_memset(tnl_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, tnl_init_v6));

    if (ipv6) {
        /* Set destination ipv6 address. */
        soc_mem_ip6_addr_set(unit, mem, tnl_entry, DIPf, tnl_info->dip6, 
                             SOC_MEM_IP6_FULL_ADDR);

        /* Set source ipv6 address. */
        soc_mem_ip6_addr_set(unit, mem, tnl_entry, SIPf, tnl_info->sip6, 
                             SOC_MEM_IP6_FULL_ADDR);
    } else {
        /* Set destination address. */
        soc_mem_field_set(unit, mem, tnl_entry, DIPf, (uint32 *)&tnl_info->dip);

        /* Set source address. */
        soc_mem_field_set(unit, mem, tnl_entry, SIPf, (uint32 *)&tnl_info->sip);
    }

    if ((!ipv6) && SOC_MEM_FIELD_VALID(unit, mem, IPV4_DF_SELf)) {
        /* IP tunnel hdr DF bit for IPv4. */
        df_val = 0;
        if (tnl_info->flags & BCM_TUNNEL_INIT_USE_INNER_DF) {
            df_val |= 0x2;
        } else if (tnl_info->flags & BCM_TUNNEL_INIT_IPV4_SET_DF) {
            df_val |= 0x1;
        }
        soc_mem_field32_set(unit, mem, tnl_entry, IPV4_DF_SELf, df_val);
    }

    if (ipv6 && SOC_MEM_FIELD_VALID(unit, mem, IPV6_DF_SELf)) {
        /* IP tunnel hdr DF bit for IPv6. */
        df_val = (tnl_info->flags & BCM_TUNNEL_INIT_IPV6_SET_DF) ? 0x1 : 0;
        soc_mem_field32_set(unit, mem, tnl_entry, IPV6_DF_SELf, df_val);
    }

    /* Set DSCP value.  */
    soc_mem_field32_set(unit, mem, tnl_entry, DSCPf, tnl_info->dscp);

    /* Tunnel header DSCP select. */
    soc_mem_field32_set(unit, mem, tnl_entry, DSCP_SELf, tnl_info->dscp_sel);

#if defined(BCM_FIREBOLT_SUPPORT)
    if(SOC_IS_FBX(unit)) {
        /* Set TTL. */
        soc_mem_field32_set(unit, mem, tnl_entry, TTLf, tnl_info->ttl);

        /* Set tunnel type. */
        BCM_IF_ERROR_RETURN (_bcm_xgs3_tnl_type_to_hw_code(unit, tnl_info->type,
                                                           &hw_type_code,
                                                           &entry_type));

        soc_mem_field32_set(unit, mem, tnl_entry, TUNNEL_TYPEf, hw_type_code);

#if defined(BCM_TRX_SUPPORT)
        /* Set flow label. */
        if (ipv6 && SOC_MEM_FIELD_VALID(unit, mem, FLOW_LABELf)) {
            soc_mem_field32_set(unit, mem, tnl_entry, FLOW_LABELf,
                                tnl_info->flow_label);
        }

        /* Set entry type. */
        if (SOC_MEM_FIELD_VALID(unit, mem, ENTRY_TYPEf)) {
            soc_mem_field32_set(unit, mem, tnl_entry, ENTRY_TYPEf, entry_type);
        }
#endif /* BCM_TRX_SUPPORT */

        /* Set destination mac address. */
        if (SOC_MEM_FIELD_VALID(unit, mem, DEST_ADDRf)) {
            soc_mem_mac_addr_set(unit, mem, &tnl_entry, DEST_ADDRf, tnl_info->dmac);
        }
#if defined (BCM_SCORPION_SUPPORT)
        /* Set lower portion of destination mac address. */
        if (SOC_MEM_FIELD_VALID(unit, mem, DEST_ADDR_LOWERf)) {
            soc_mem_mac_address_set(unit, mem, &tnl_entry, DEST_ADDR_LOWERf,
                                    tnl_info->dmac, SOC_MEM_MAC_LOWER_ONLY);
        }

        /* Set upper portion of destination mac address. */
        if (SOC_MEM_FIELD_VALID(unit, mem, DEST_ADDR_UPPERf)) {
            soc_mem_mac_address_set(unit, mem, &tnl_entry, DEST_ADDR_UPPERf,
                                    tnl_info->dmac, SOC_MEM_MAC_UPPER_ONLY);
        } 
#endif /* BCM_SCORPION_SUPPORT */
    }
#if defined(BCM_TRIUMPH2_SUPPORT)
    /* Program the IPv4 ID if not using global ID space */
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        int val;
        uint16 random;
        egr_fragment_id_table_entry_t entry;
        BCM_IF_ERROR_RETURN
            (bcm_esw_switch_control_get(unit, bcmSwitchTunnelIp4IdShared, &val));
        if (!val) {
            sal_memset(&entry, 0, sizeof(egr_fragment_id_table_entry_t));
            if (tnl_info->flags & BCM_TUNNEL_INIT_IP4_ID_SET_FIXED) {
                soc_EGR_FRAGMENT_ID_TABLEm_field32_set(unit, &entry,  
                                                       FRAGMENT_IDf, 
                                                       tnl_info->ip4_id);
            } else if (tnl_info->flags & BCM_TUNNEL_INIT_IP4_ID_SET_RANDOM) {
                random = (uint16) (sal_time_usecs() & 0xFFFF);
                soc_EGR_FRAGMENT_ID_TABLEm_field32_set(unit, &entry,  
                                                       FRAGMENT_IDf, 
                                                       random);               
            } else {
                /* Default is random */
                random = (uint16) (sal_time_usecs() & 0xFFFF);
                soc_EGR_FRAGMENT_ID_TABLEm_field32_set(unit, &entry,  
                                                       FRAGMENT_IDf, 
                                                       random);  
            }
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, EGR_FRAGMENT_ID_TABLEm, SOC_BLOCK_ALL, 
                               idx, &entry));
        } else {
            return BCM_E_CONFIG;
        }
    }
#endif
#endif /* BCM_FIREBOLT_SUPPORT */

    /* Write buffer to hw. */
    return BCM_XGS3_MEM_WRITE(unit, mem, idx, tnl_entry);
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_init_del
 * Purpose:
 *      Delete tunnel initiator entry from hw. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      idx      - (IN)Index to insert tunnel initiator index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_init_del(int unit, int idx, int max_paths)
{
    uint32 tnl_entry[SOC_MAX_MEM_FIELD_WORDS];  /* Buffer to write hw entry.  */
    soc_mem_t mem;                              /* Tunnel initiator memory.   */  
#if defined(BCM_TRX_SUPPORT)
    uint32 entry_type = BCM_XGS3_TUNNEL_INIT_V4;/* Entry type (IPv4/IPv6/MPLS)*/
#endif /* BCM_TRX_SUPPORT*/


    /* Get table memory. */
    mem = BCM_XGS3_L3_MEM(unit, tnl_init_v4); 

#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, mem, ENTRY_TYPEf)) {
        /* Parse hw buffer. */
        BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, tnl_entry)); 

        /* Get entry_type. */
        entry_type = soc_mem_field32_get(unit, mem, tnl_entry, ENTRY_TYPEf); 

        if (BCM_XGS3_TUNNEL_INIT_V6 == entry_type) {
            mem = BCM_XGS3_L3_MEM(unit, tnl_init_v6); 
            idx >>= 1;    /* Each record takes two slots. */
        }
    }
#endif /* BCM_TRX_SUPPORT */
    /* Zero write buffer. */
    sal_memset(&tnl_entry, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

    /* Write buffer to hw. */
    return BCM_XGS3_MEM_WRITE(unit, mem, idx, &tnl_entry);
}


/*
 * Function:
 *      _bcm_xgs3_l3_tnl_dscp_get
 * Purpose:
 *      Get a tunnel dscp entry. 
 * Parameters:
 *      unit     -(IN)SOC unit number. 
 *      idx      -(IN)Entry index to read.
 *      tnl_info -(IN)Buffer to fill tunnel info.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_dscp_get(int unit, int idx, 
                          bcm_tunnel_dscp_map_t *tnl_info)
{
   soc_mem_t mem;                             /* Tunnel dscp table memory.*/  
   uint32 tnl_entry[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry. */
   soc_field_t fld;                           /* DSCP field index.        */

    /* Get table memory. */
    mem = BCM_XGS3_L3_MEM(unit, tnl_dscp); 

    /* Read buffer from hw. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, tnl_entry)); 

    /* Get dscp field id based on unit type. */ 
#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_FBX(unit)) {
        fld = DSCPf;
    } else 
#endif /* BCM_FIREBOLT_SUPPORT */
#if defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)){
        fld = EGRESS_DSCPf;
    } else 
#endif /* BCM_EASYRIDER_SUPPORT */
    {
        return (BCM_E_UNAVAIL);
    }

    /* Parse hw buffer. */
    tnl_info->dscp = soc_mem_field32_get(unit, mem, tnl_entry, fld); 

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_tnl_dscp_add
 * Purpose:
 *      Add tunnel dscp entry to hw. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      idx      - (IN)Index to insert tunnel initiator index.
 *      buf      - (IN)Tunnel dscp entry info.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_tnl_dscp_set(int unit, int idx, void *buf)
{
    soc_mem_t mem;                      /* Tunnel dscp table memory.*/  
    bcm_tunnel_dscp_map_t *tnl_info;    /* Tunnel dscp information. */

    /* Input parameters check. */
    if (NULL == buf) {
        return (BCM_E_PARAM);
    }

    /* Get table memory. */
    mem = BCM_XGS3_L3_MEM(unit, tnl_dscp); 

    tnl_info = (bcm_tunnel_dscp_map_t *) buf;

#ifdef BCM_FIREBOLT_SUPPORT
    if(SOC_IS_FBX(unit)) {
        egr_dscp_table_entry_t tnl_entry; /* Buffer to read hw entry. */
        uint32 *buf_p;                    /* HW buffer address.       */

        /* Read buffer from hw. */
        buf_p = (uint32 *)&tnl_entry;

        /* Zero write buffer. */
        sal_memset(buf_p, 0, BCM_XGS3_L3_ENT_SZ(unit, tnl_dscp));

        soc_mem_field32_set(unit, mem, buf_p, DSCPf, tnl_info->dscp);

        /* Write buffer to hw. */
        return BCM_XGS3_MEM_WRITE(unit, mem, idx, buf_p);
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
    if(SOC_IS_EASYRIDER(unit)) {
        egress_dscp_exp_entry_t tnl_entry; /* Buffer to read hw entry. */
        uint32 *buf_p;                     /* HW buffer address.       */

        /* Read buffer from hw. */
        buf_p = (uint32 *)&tnl_entry;

        /* Since this table is shared with MPLS EXP, need to read,
         * modify, and write to preserve the EXP field value.
         */
        BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, buf_p)); 

        soc_mem_field32_set(unit, mem, buf_p, EGRESS_DSCPf, tnl_info->dscp);

        /* Write buffer to hw. */
        return BCM_XGS3_MEM_WRITE(unit, mem, idx, buf_p);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_clear_all
 * Purpose:
 *      Flush all l3 tables on the chip
 * Parameters:
 *      unit      - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_clear_all(int unit)
{

#if defined(BCM_TRIUMPH_SUPPORT) 
    if (SOC_MEM_IS_VALID(unit, L3_MTU_VALUESm)) {
        /* Clear L3 mtu values. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, L3_MTU_VALUESm));
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    if (SAL_BOOT_BCMSIM || SAL_BOOT_QUICKTURN) {
        return BCM_E_NONE;
    }

    /* Clear next hop table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, intf)));

    /* Clear next hop table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, nh)));

    /* Clear L3 ipv4 table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, v4)));

    /* Clear L3 ipv6 table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, v6)));

    /* Clear tunnel terminator table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit,
                                                                    tnl_term)));

    /* Clear IPv4 tunnel initiator table. */
    BCM_IF_ERROR_RETURN 
        (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, tnl_init_v4)));

    /* Clear IPv6 tunnel initiator table. */
    BCM_IF_ERROR_RETURN 
        (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, tnl_init_v6)));
    /* Clear tunnel dscp table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit,
                                                                    tnl_dscp)));

    /* Clear ECMP groups table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit,
                                                                    ecmp)));

    /* Clear route table. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit,
                                                                    defip)));

    /* Clear IPv6 prefix map table. */
    BCM_IF_ERROR_RETURN 
        (BCM_XGS3_MEM_CLEAR (unit, BCM_XGS3_L3_MEM (unit, v6_prefix_map)));

#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_FBX(unit)) {
        /* Clear egress next hop table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, EGR_L3_NEXT_HOPm));

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
        defined(BCM_TRX_SUPPORT)
        /* Clear initial ecmp groups table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, INITIAL_L3_ECMPm));

        /* Clear initial ingress next hop table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, 
                                                 INITIAL_ING_L3_NEXT_HOPm));
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRX_SUPPORT) 
    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {
        /* Clear ecmp groups count table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, L3_ECMP_COUNTm));
    }

    if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMP_COUNTm)) {
        /* Clear initial ecmp groups count table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, INITIAL_L3_ECMP_COUNTm));
    }
#endif  /* BCM_TRX_SUPPORT */  

#if defined(BCM_TRIUMPH_SUPPORT) 
    if (SOC_MEM_IS_VALID(unit, L3_IIFm)) {
        /* Clear initial ecmp groups count table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, L3_IIFm));
    }

    if (SOC_MEM_IS_VALID(unit, EXT_IPV4_DEFIPm)) {
        /* Clear External V4 table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, EXT_IPV4_DEFIPm));
    }

    if (SOC_MEM_IS_VALID(unit, EXT_IPV6_64_DEFIPm)) {
        /* Clear External V6 upper 64 table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, EXT_IPV6_64_DEFIPm));
    }

    if (SOC_MEM_IS_VALID(unit, EXT_IPV6_128_DEFIPm)) {
        /* Clear External V6 full prefix length table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, EXT_IPV6_128_DEFIPm));
    }
#endif /* BCM_TRIUMPH_SUPPORT */  


#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Clear ECMP groups table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, VRF_VFI_INTFm));

        /* Clear L3 interface group table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, 
                                                 L3INTF_EGR_FILTER_LISTm));

        /* Clear L3 interface group table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, 
                                                 L3INTF_IGR_FILTER_LISTm));

        /* Clear adjacent mac/next hop table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, EGRESS_ADJACENT_MACm));

        /* Note : since IPMC_GROUP_V4 is shared with IPMC_GROUP_V6, only 
         * clear IPMC_GROUP_V4 table here.
         */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, IPMC_GROUP_V4m));
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
           SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
           SOC_IS_KATANA(unit)) {
        /* Clear VRF counter index table. */
        BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_CLEAR (unit, VRFm));
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    return (BCM_E_NONE);
} 


/*
 * Function:
 *      _bcm_xgs3_l3_intf_get
 * Purpose:
 *      Get an entry from L3 interface table.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      intf_info - (IN/OUT)Pointer to memory for interface information.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_get(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    l3intf_entry_t l3_if_entry;    /* Buffer to read interface info. */
    _bcm_l3_intf_fields_t *fld;    /* Interface table common fields. */
    uint32  *l3_if_entry_p;        /* Interface read buffer address. */
    soc_mem_t mem;                 /* Interface table memory.        */

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Get interface table memory. */
    mem = BCM_XGS3_L3_MEM(unit,  intf);

    /* Zero read buffer. */
    l3_if_entry_p = (uint32 *)&l3_if_entry;
    sal_memset(l3_if_entry_p, 0, sizeof(l3intf_entry_t));
    
    /* Read interface table entry by index. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_MEM_READ (unit, mem, intf_info->l3i_index, l3_if_entry_p));

    /* Extract interface information. */
    fld = (_bcm_l3_intf_fields_t *)BCM_XGS3_L3_MEM_FIELDS(unit, intf);

    /* Get mac address. */
    soc_mem_mac_addr_get(unit, mem, l3_if_entry_p,
                         fld->mac_addr, intf_info->l3i_mac_addr);

    /* Get vlan id. */
    intf_info->l3i_vid = 
        soc_mem_field32_get(unit, mem, l3_if_entry_p, fld->vlan_id);

    /* Get Time To Live. */ 
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->ttl)) {
        intf_info->l3i_ttl =
            soc_mem_field32_get(unit, mem, l3_if_entry_p, fld->ttl);
    }

    /* Get tunnel index. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->tnl_id)) {
        intf_info->l3i_tunnel_idx =
            soc_mem_field32_get(unit, mem, l3_if_entry_p, fld->tnl_id);
    }

    /* Set L2 bridging only flag. */
    if (soc_mem_field32_get(unit, mem, l3_if_entry_p, fld->l2_switch)) {
        intf_info->l3i_flags |= BCM_L3_L2ONLY;
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, OPRI_OCFI_SELf)) {
         BCM_IF_ERROR_RETURN
            (_bcm_td_l3_intf_qos_get(unit, l3_if_entry_p, intf_info));
    }
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, IVID_VALIDf)) {
        if (soc_mem_field32_get(unit, mem, l3_if_entry_p, IVID_VALIDf)) {
            intf_info->l3i_inner_vlan = 
                soc_mem_field32_get(unit, mem, l3_if_entry_p, IVIDf);
        }
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
   if (SOC_IS_EASYRIDER(unit)) {
       /* Get RPF flag. */
       if (soc_mem_field32_get(unit, mem, l3_if_entry_p, URPFf)) {
           intf_info->l3i_flags |= BCM_L3_RPF;
       }
   }
#endif /* BCM_EASYRIDER_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT)
    /* Read L3 mtu value for the interface. */
    if (SOC_MEM_IS_VALID(unit, L3_MTU_VALUESm)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            BCM_IF_ERROR_RETURN(_bcm_hu_l3_intf_mtu_get(unit, intf_info));
        } else
#endif /* BCM_HURRICANE_SUPPORT*/
        {
            BCM_IF_ERROR_RETURN(_bcm_tr_l3_intf_mtu_get(unit, intf_info));
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_xgs3_l3_intf_add
 * Purpose:
 *      Add an entry to L3 interface table.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      intf_info - (IN)Interface information.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_add(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    l3intf_entry_t l3_if_entry;    /* Buffer to write interface info. */
    _bcm_l3_intf_fields_t *fld;    /* Interface table common fields.  */
    uint32  *l3_if_entry_p;        /* Write buffer address.           */
    soc_mem_t mem;                 /* Interface table memory.         */

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Get interface table memory. */
    mem = BCM_XGS3_L3_MEM(unit,  intf);

    /* Get memory fields information. */
    fld = (_bcm_l3_intf_fields_t *)BCM_XGS3_L3_MEM_FIELDS(unit, intf);

    /* Zero the buffer. */
    l3_if_entry_p = (uint32 *)&l3_if_entry;
    sal_memset(l3_if_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, intf));

    /* Set mac address. */
    soc_mem_mac_addr_set(unit, mem, l3_if_entry_p, 
                         fld->mac_addr, intf_info->l3i_mac_addr);

    /* Set vlan id. */
    soc_mem_field32_set(unit, mem, l3_if_entry_p, 
                        fld->vlan_id, intf_info->l3i_vid);

    /* Set Time To Live. */ 
    if(SOC_MEM_FIELD_VALID(unit, mem, fld->ttl)) {
        soc_mem_field32_set(unit, mem, l3_if_entry_p, 
                            fld->ttl, intf_info->l3i_ttl);
    }

    /* Set tunnel index. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->tnl_id)) {
        soc_mem_field32_set(unit, mem, l3_if_entry_p, 
                            fld->tnl_id, intf_info->l3i_tunnel_idx);
    }

    /* Set L2 bridging only flag. */
    if (intf_info->l3i_flags & BCM_L3_L2ONLY) {
        soc_mem_field32_set(unit, mem, l3_if_entry_p,
                            fld->l2_switch, 1);
    }
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Set Enable RPF check flag. */
        if (intf_info->l3i_flags & BCM_L3_RPF) {
            soc_mem_field32_set(unit, mem, l3_if_entry_p, URPFf, 1);
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, IVIDf)) {
        if (intf_info->l3i_inner_vlan) {
            soc_mem_field32_set(unit, mem, l3_if_entry_p, IVIDf, 
                                intf_info->l3i_inner_vlan);
            soc_mem_field32_set(unit, mem, l3_if_entry_p, IVID_VALIDf, 0x1);
        }
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, OPRI_OCFI_SELf)) {
         /* Check if L3 Ingress Interface Map mode not set */ 
         if (!(BCM_XGS3_L3_INGRESS_INTF_MAP_MODE_ISSET(unit))) {
            BCM_IF_ERROR_RETURN
               (_bcm_td_l3_intf_qos_set(unit, l3_if_entry_p, intf_info));
         }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Write entry to the chip. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_MEM_WRITE(unit, mem, intf_info->l3i_index, l3_if_entry_p));

    if (BCM_XGS3_L3_INGRESS_INTF_MAP_MODE_ISSET(unit)) {
         /* mark L3_ING_Interface with interface_id as Used */
         SHR_BITSET(BCM_XGS3_L3_ING_IF_INUSE(unit), intf_info->l3i_index);
    }

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_MEM_IS_VALID(unit, L3_MTU_VALUESm)) {
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            BCM_IF_ERROR_RETURN(_bcm_hu_l3_intf_mtu_set(unit, intf_info));
        } else
#endif /* BCM_HURRICANE_SUPPORT*/
        {
            BCM_IF_ERROR_RETURN(_bcm_tr_l3_intf_mtu_set(unit, intf_info));
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */
    return (BCM_E_NONE);

}

/*
 * Function:
 *      _bcm_xgs3_l3_intf_del
 * Purpose:
 *      Delete an entry from L3 interface table.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      ifindex   - (IN)Interface index. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_del(int unit, int ifindex)
{
    l3intf_entry_t l3_if_entry;    /* Buffer to write interface info. */
    soc_mem_t              mem;    /* Interface table memory.        */

    /* Get interface table memory. */
    mem = BCM_XGS3_L3_MEM(unit,  intf);

    /* Zero the buffer. */
    sal_memset(&l3_if_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, intf));

    if (BCM_XGS3_L3_INGRESS_INTF_MAP_MODE_ISSET(unit)) {
         /* mark L3_ING_Interface with interface_id as Un-used */
         SHR_BITCLR(BCM_XGS3_L3_ING_IF_INUSE(unit), ifindex);
    }

    /* Write entry to the chip. */
    return BCM_XGS3_MEM_WRITE(unit, mem, ifindex, &l3_if_entry);
}

/*
 * Function:
 *      _bcm_xgs3_l3_intf_tnl_init_get
 * Purpose:
 *      Get interface tunnel initiator info
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      ifindex      - (IN)Interface index. 
 *      tnl_info     - (OUT)Tunnel initiator buffer to fill.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_tnl_init_get(int unit, int ifindex,
                               _bcm_tnl_init_data_t *tnl_info)
{
    l3intf_entry_t l3_if_entry;    /* Buffer to read interface info. */
    _bcm_l3_intf_fields_t *fld;    /* Interface table common fields. */
    uint32  *l3_if_entry_p;        /* Write buffer address.          */
    soc_mem_t mem;                 /* Interface table memory.        */
#ifdef BCM_EASYRIDER_SUPPORT
    int tnl_type;                  /* Tunnel type hw code.           */
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Input parameters check */
    if (NULL == tnl_info) {
        return (BCM_E_PARAM);
    }

    /* Get interface table memory. */
    mem = BCM_XGS3_L3_MEM(unit,  intf);

    /* Zero the buffer. */
    l3_if_entry_p = (uint32 *)&l3_if_entry;
    sal_memset(l3_if_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, intf));

    /* Read interface table entry by index. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ (unit, mem, ifindex, l3_if_entry_p));

    /* Extract interface information. */
    fld = (_bcm_l3_intf_fields_t *)BCM_XGS3_L3_MEM_FIELDS(unit, intf);

    /* Get tunnel index. */
    tnl_info->tnl_idx =
        soc_mem_field32_get(unit, mem, l3_if_entry_p, fld->tnl_id);


#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Get tunnel type.  */
        tnl_type = soc_mem_field32_get(unit, mem, l3_if_entry_p, TUNNEL_TYPEf);

        /* Translate tunnel type to API format. */
        BCM_IF_ERROR_RETURN(_bcm_xgs3_tnl_hw_code_to_type(unit, tnl_type,
                                                        BCM_XGS3_TUNNEL_INIT_V4,
                                                        &tnl_info->tnl_type));

        /* Get tunnel TTL.  */
        tnl_info->tnl_ttl =
            soc_mem_field32_get(unit, mem, l3_if_entry_p, TTLf);

        /* Adjacent mac entry index. */
        tnl_info->adj_mac_idx =
            soc_mem_field32_get(unit, mem, l3_if_entry_p, ADJ_MAC_INDEXf);

        /* Get DSCP map.  */
        tnl_info->dscp_map =
            soc_mem_field32_get(unit, mem, l3_if_entry_p, QOS_INDEXf);
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_intf_tnl_init_set
 * Purpose:
 *      Mark interface as tunnel initiator
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      ifindex      - (IN)Interface index. 
 *      tnl_info     - (IN)Tunnel initiator entry params.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_tnl_init_set(int unit, int ifindex,
                               _bcm_tnl_init_data_t *tnl_info)
{
    l3intf_entry_t l3_if_entry;    /* Buffer to read interface info. */
    _bcm_l3_intf_fields_t *fld;    /* Interface table common fields. */
    uint32  *l3_if_entry_p;        /* Write buffer address.          */
    soc_mem_t mem;                 /* Interface table memory.        */
#ifdef BCM_EASYRIDER_SUPPORT
    int entry_type;                /* Unused.                        */
    int tnl_type;                  /* Tunnel type hw code.            */
    bcm_tunnel_type_t old_tnl_type = 0;/* Mpls conflicts test variable.   */
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Input parameters check */
    if (NULL == tnl_info) {
        return (BCM_E_PARAM);
    }

    /* Get interface table memory. */
    mem = BCM_XGS3_L3_MEM(unit,  intf);

    /* Zero the buffer. */
    l3_if_entry_p = (uint32 *)&l3_if_entry;
    sal_memset(l3_if_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, intf));

    /* Read interface table entry by index. */
    BCM_IF_ERROR_RETURN (BCM_XGS3_MEM_READ (unit, mem, ifindex, l3_if_entry_p));

    /* Extract interface information. */
    fld = (_bcm_l3_intf_fields_t *)BCM_XGS3_L3_MEM_FIELDS(unit, intf);

    /* Get tunnel index. */
    soc_mem_field32_set(unit, mem, l3_if_entry_p, fld->tnl_id,
                        tnl_info->tnl_idx);

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Check for MPLS tunnel conflicts */
        tnl_type  = soc_mem_field32_get(unit, mem, l3_if_entry_p, TUNNEL_TYPEf);
        _bcm_xgs3_tnl_hw_code_to_type (unit, tnl_type, 
                                       BCM_XGS3_TUNNEL_INIT_V4, &old_tnl_type);
        if (old_tnl_type == bcmTunnelTypeMpls) {
            BCM_XGS3_L3_ERR(("L3 interface already used for MPLS tunnel\n"));
            return (BCM_E_PARAM);
        }

        /* Translate API tunnel type to hw code. */
        BCM_IF_ERROR_RETURN (_bcm_xgs3_tnl_type_to_hw_code(unit, tnl_info->tnl_type,
                                                           &tnl_type,
                                                           &entry_type));

        /* Set tunnel type.  */
        soc_mem_field32_set(unit, mem, l3_if_entry_p, TUNNEL_TYPEf, tnl_type);

        /* Set tunnel TTL.  */
        soc_mem_field32_set(unit, mem, l3_if_entry_p, TTLf,
                            tnl_info->tnl_ttl);

        /* Set Adjacent mac entry index. */
        soc_mem_field32_set(unit, mem, l3_if_entry_p, ADJ_MAC_INDEXf,
                            tnl_info->adj_mac_idx);

        /* Set DSCP map index. */
        soc_mem_field32_set(unit, mem, l3_if_entry_p, QOS_INDEXf,
                            tnl_info->dscp_map);
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    /* Write entry back to hw. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_MEM_WRITE(unit, mem, ifindex, l3_if_entry_p));

#ifdef BCM_FIREBOLT_SUPPORT 
    if (SOC_MEM_FIELD_VALID(unit, ING_L3_NEXT_HOPm, L3UC_TUNNEL_TYPEf)) {
        BCM_IF_ERROR_RETURN
            (_bcm_fb_nh_intf_is_tnl_update(unit, ifindex, tnl_info->tnl_idx));
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_intf_tnl_init_reset
 * Purpose:
 *      Unmark interface as tunnel initiator
 * Parameters:
 *      unit         - (IN)SOC unit number.
 *      ifindex      - (IN)Interface index. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_intf_tnl_init_reset(int unit, int ifindex)
{
    l3intf_entry_t l3_if_entry;    /* Buffer to read interface info. */
    _bcm_l3_intf_fields_t *fld;    /* Interface table common fields. */
    uint32  *l3_if_entry_p;        /* Write buffer address.          */
    soc_mem_t mem;                 /* Interface table memory.        */

    /* Get interface table memory. */
    mem = BCM_XGS3_L3_MEM(unit,  intf);

    /* Zero the buffer. */
    l3_if_entry_p = (uint32 *)&l3_if_entry;
    sal_memset(l3_if_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, intf));

    /* Read interface table entry by index. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ (unit, mem, ifindex, l3_if_entry_p));

    /* Extract interface information. */
    fld = (_bcm_l3_intf_fields_t *)BCM_XGS3_L3_MEM_FIELDS(unit, intf);

    /* Get tunnel index. */
    soc_mem_field32_set(unit, mem, l3_if_entry_p, fld->tnl_id, 0);

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Set tunnel type.  */
        soc_mem_field32_set(unit, mem, l3_if_entry_p, TUNNEL_TYPEf, 0);

        /* Set tunnel TTL.  */
        soc_mem_field32_set(unit, mem, l3_if_entry_p, TUNNEL_TTLf, 0);

        /* Set Adjacent mac entry index. */
        soc_mem_field32_set(unit, mem, l3_if_entry_p, ADJ_MAC_INDEXf, 0);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Write entry back to hw. */
    BCM_IF_ERROR_RETURN
        (BCM_XGS3_MEM_WRITE(unit, mem, ifindex, l3_if_entry_p));

#ifdef BCM_FIREBOLT_SUPPORT 
    if (SOC_IS_FBX(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_fb_nh_intf_is_tnl_update(unit, ifindex, 0));
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return (BCM_E_NONE);

}


/*
 * Function:
 *      _bcm_xgs3_nh_entry_parse
 * Purpose:
 *      Parse the NextHop table entry info.
 * Parameters:
 *      unit          - (IN) SOC unit number.
 *      ing_entry_ptr - (IN) Ingress next hop table entry. 
 *      egr_entry_ptr - (IN) Egress next hop table entry. 
 *      nh_entry      - (OUT) Next hop entry info. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_entry_parse(int unit, uint32 *ing_entry_ptr, 
                         uint32 *egr_entry_ptr, bcm_l3_egress_t *nh_entry)
{
    _bcm_l3_nh_fields_t *fld;           /* Next hop table common fields.    */
    uint32 *hw_entry_p;                 /* HW entry buffer pointer.         */
    soc_mem_t mem;                      /* Table location memory.           */
    int mod;                            /* Module id value in hw.           */
    int port;                           /* Port value in hw.                */

    hw_entry_p = ing_entry_ptr;

    /* Zero buffers. */
    bcm_l3_egress_t_init(nh_entry);

    /* Get next hop table memory location. */
    mem = BCM_XGS3_L3_MEM(unit, nh);

    /* Extract next hop fields information. */       
    fld = (_bcm_l3_nh_fields_t *)BCM_XGS3_L3_MEM_FIELDS(unit, nh);

    /* Extract next hop info. */

    /* Get port trunk information. */
    mod = soc_mem_field32_get(unit, mem, hw_entry_p, fld->module);
    port = soc_mem_field32_get(unit, mem, hw_entry_p,  fld->port_tgid);

#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, COPY_TO_CPUf)) {
        if (soc_mem_field32_get(unit, mem, hw_entry_p, COPY_TO_CPUf)) {
            nh_entry->flags |= BCM_L3_COPY_TO_CPU;
        } 
    }

    if (SOC_MEM_FIELD_VALID(unit, mem, DROPf)) {
        if (soc_mem_field32_get(unit, mem, hw_entry_p, DROPf)) {
            nh_entry->flags |= BCM_L3_DST_DISCARD;
        }
    }

    if (soc_feature(unit, soc_feature_trunk_group_overlay)) {
        if (soc_mem_field32_get(unit, mem, hw_entry_p, Tf)) {
            nh_entry->flags |= BCM_L3_TGID;
            port = soc_mem_field32_get(unit, mem, hw_entry_p, TGIDf);
        }
    } else 
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_VLAN_DISABLEf)) {
         if (soc_mem_field32_get(unit, mem, hw_entry_p, L3__L3_UC_VLAN_DISABLEf)) {
              nh_entry->flags |= BCM_L3_KEEP_VLAN;
         }
    }

    if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_TTL_DISABLEf)) {
         if (soc_mem_field32_get(unit, mem, hw_entry_p, L3__L3_UC_TTL_DISABLEf)) {
              nh_entry->flags |= BCM_L3_KEEP_TTL;
         }
    }

    if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_DA_DISABLEf)) {
         if (soc_mem_field32_get(unit, mem, hw_entry_p, L3__L3_UC_DA_DISABLEf)) {
              nh_entry->flags |= BCM_L3_KEEP_DSTMAC;
         }
    }

    if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_SA_DISABLEf)) {
         if (soc_mem_field32_get(unit, mem, hw_entry_p, L3__L3_UC_SA_DISABLEf)) {
              nh_entry->flags |= BCM_L3_KEEP_SRCMAC;
         }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */


    if (port & BCM_TGID_TRUNK_INDICATOR(unit)) {
        nh_entry->flags |= BCM_L3_TGID;
    }

    /* Map module/port to next hop entry format. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_nh_map_hw_data_to_api(unit, port, mod, nh_entry));

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Get mpls label. */
        nh_entry->mpls_label =
            soc_mem_field32_get(unit, mem, hw_entry_p, FASTREROUTE_LABELf);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        /* Get vlan id. */
        nh_entry->vlan =
            soc_mem_field32_get(unit, mem, hw_entry_p, VLAN_IDf);

        hw_entry_p = egr_entry_ptr;
        mem = EGR_L3_NEXT_HOPm;
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    /* Get interface index. */
    nh_entry->intf =
       soc_mem_field32_get(unit, mem, hw_entry_p, fld->ifindex); 

    /* Get mac address. */
    soc_mem_mac_addr_get(unit, mem, hw_entry_p,
                         fld->mac_addr, nh_entry->mac_addr);

#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, ENTRY_TYPEf)) {
         int macda_idx;
         egr_mac_da_profile_entry_t macda;
         int entry_type;
		
		
         entry_type =	soc_mem_field32_get(unit, mem, hw_entry_p,  ENTRY_TYPEf);
         if (entry_type == 0x1) { /* == MPLS_MACDA_PROFILE */
		
              /* Get Index to MAC-DA-PROFILEm */
             macda_idx = soc_mem_field32_get(unit, mem, hw_entry_p, 
                                  MAC_DA_PROFILE_INDEXf);
		
              /* Read entry from MAC-DA-PROFILEm */
             BCM_IF_ERROR_RETURN (soc_mem_read(unit, EGR_MAC_DA_PROFILEm, 
                                  MEM_BLOCK_ANY, macda_idx, &macda));
		
              /* Obtain Mac-Address */
              soc_mem_mac_addr_get(unit, EGR_MAC_DA_PROFILEm,
                                  &macda, MAC_ADDRESSf, nh_entry->mac_addr);
         }
    }
#endif

    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_xgs3_nh_get
 * Purpose:
 *        Read the NextHop table info given index.
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      idx       - (IN) Table index to read. 
 *      nh_entry  - (OUT) Next hop entry info. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_get(int unit, int idx, bcm_l3_egress_t *nh_entry)
{
    uint32 hw_entry[SOC_MAX_MEM_FIELD_WORDS];  /* Buffer to fill nh entry.   */
    egr_l3_next_hop_entry_t  egr_entry;        /* Egress next hop entry.     */

    /* Input parameters check */
    if (NULL == nh_entry) {
        return (BCM_E_PARAM);
    }

    /* Zero buffers. */
    sal_memset(&hw_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, nh));

    /* Read ingress next hop entry. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, BCM_XGS3_L3_MEM(unit, nh),
                                          idx, &hw_entry));

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        sal_memset(&egr_entry, 0, sizeof(egr_l3_next_hop_entry_t));

        /*  Read egress next hop entry. */
        BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, EGR_L3_NEXT_HOPm,
                                              idx, &egr_entry));
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    _bcm_xgs3_nh_entry_parse(unit, (uint32 *)&hw_entry,
                             (uint32 *)&egr_entry, nh_entry);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_nh_del
 * Purpose:
 *      Delete next hop entry from the chip.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      idx       - (IN)Deleted entry index.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_del(int unit, int idx, int max_paths)
{
    uint32 hw_entry[SOC_MAX_MEM_FIELD_WORDS];/* Buffer to fill nh entry.   */
    soc_mem_t mem;                           /* Table memory.              */
    int rv;                                  /* Operation return status.   */
    int first_error = BCM_E_NONE;            /* First error occured.       */

    /* Get table memory. */
    mem = BCM_XGS3_L3_MEM(unit, nh);

    /* Zero write buffer. */
    sal_memset(&hw_entry, 0, SOC_MAX_MEM_FIELD_WORDS *sizeof(uint32));

    /* Write next hop entry. */
    rv = BCM_XGS3_MEM_WRITE(unit, mem, idx, &hw_entry);
    if (rv < 0) {
        first_error = rv;
    }

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        /* Write egress next hop entry. */
        mem = EGR_L3_NEXT_HOPm; 
        rv = BCM_XGS3_MEM_WRITE(unit, mem, idx, &hw_entry);
        if ((rv < 0) && (first_error == BCM_E_NONE)) {
            first_error = rv;
        }
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
        defined(BCM_TRX_SUPPORT)
        /* Write initial next hop table. */
        mem = INITIAL_ING_L3_NEXT_HOPm;
        if (SOC_MEM_IS_VALID(unit, mem)) {
            /* Write buffer to hw. */
            rv = BCM_XGS3_MEM_WRITE(unit, mem, idx, &hw_entry);
            if ((rv < 0) && (first_error == BCM_E_NONE)) {
                first_error = rv;
            }
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return first_error;
}

/*
 * Function:
 *      _bcm_xgs3_nh_update_match
 * Purpose:
 *      Update/Show/Delete all entries in nh table matching a certain rule.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      trv_data - (IN)Delete pattern + compare,act,notify routines.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_nh_update_match(int unit, _bcm_l3_trvrs_data_t *trv_data)
{
    bcm_l3_egress_t nh_entry;     /* Next hop entry info.           */
    uint32 *ing_entry_ptr;        /* Ingress next hop entry pointer.*/ 
    char *ing_tbl_ptr;            /* Dma table pointer.             */
    int cmp_result;               /* Test routine result.           */
    soc_mem_t mem;                /* Next hop memory.               */
    int idx;                      /* Iteration index.               */
#ifdef BCM_FIREBOLT_SUPPORT
    char *egr_tbl_ptr;            /* Dma egress nh table pointer.   */
#endif /* BCM_FIREBOLT_SUPPORT */
    uint32 *egr_entry_ptr = NULL; /* Egress next hop entry pinter.  */ 
    int rv = BCM_E_NONE;          /* Operation return status.       */

    /* Get next table memory. */
    mem = BCM_XGS3_L3_MEM(unit, nh);

    /* Table DMA the nhtable to software copy */
    BCM_IF_ERROR_RETURN 
        (bcm_xgs3_l3_tbl_dma(unit, mem, BCM_XGS3_L3_ENT_SZ(unit, nh), "nh_tbl",
                             &ing_tbl_ptr, NULL));

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        /*  Read egress next hop entry. */
        rv = bcm_xgs3_l3_tbl_dma(unit, EGR_L3_NEXT_HOPm,
                                 sizeof(egr_l3_next_hop_entry_t), "egr_nh_tbl",
                                 &egr_tbl_ptr, NULL);
        if (rv < 0) {
            soc_cm_sfree(unit, ing_tbl_ptr);
            return rv;
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    for (idx = 0; idx < BCM_XGS3_L3_NH_TBL_SIZE(unit); idx++) {
        /* Skip unused entries. */
        if (!BCM_XGS3_L3_ENT_REF_CNT (BCM_XGS3_L3_TBL_PTR(unit, next_hop),
                                      idx)) {
            continue;
        }

        /* Skip trap to CPU entry internally installed entry. */
        if (BCM_XGS3_L3_L2CPU_NH_IDX == idx) {
            continue;
        }

        /* Calculate entry ofset. */
        ing_entry_ptr =
            soc_mem_table_idx_to_pointer(unit, mem, uint32 *, ing_tbl_ptr, idx);

#ifdef BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(unit)) {
            /*  Read egress next hop entry. */
            egr_entry_ptr = 
                soc_mem_table_idx_to_pointer(unit, EGR_L3_NEXT_HOPm,
                                             uint32 *, egr_tbl_ptr, idx);
        }
#endif /* BCM_FIREBOLT_SUPPORT */
        /* coverity[var_deref_model : FALSE] */
        _bcm_xgs3_nh_entry_parse(unit, ing_entry_ptr, egr_entry_ptr, &nh_entry);

        /* Execute operation routine if any. */
        if (trv_data->op_cb) {
            rv = (*trv_data->op_cb)(unit, (void *)trv_data,
                                    (void *)&nh_entry,(void *)&idx, &cmp_result);
            if (rv < 0) {
                break;
            }
        }
    }
    soc_cm_sfree(unit, ing_tbl_ptr);
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        soc_cm_sfree(unit, egr_tbl_ptr);
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    return (rv);
}

/*
 * Function:
 *      _bcm_xgs3_l3_ent_init
 * Purpose:
 *      Service routine used to init hw l3 entry 
 * Parameters:
 *      unit      - (IN)SOC unit number. 
 *      mem       - (IN)L3 table memory.
 *      l3cfg     - (IN/OUT)l3 entry  lookup key & search result.
 *      l3x_entry - (IN)hw buffer.
 * Returns:
 *      void
 */
STATIC void
_bcm_xgs3_l3_ent_init(int unit, soc_mem_t mem, 
                      _bcm_l3_cfg_t *l3cfg, void *l3x_entry)
{
    _bcm_l3_fields_t *fld;        /* L3 table common fields.     */
    uint32 *buf_p;                /* HW buffer address.          */ 
    int ipv6;                     /* Entry is IPv6 flag.         */

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Zero destination buffer. */
    buf_p = (uint32 *)l3x_entry;
    sal_memset(buf_p, 0, (ipv6) ?  BCM_XGS3_L3_ENT_SZ(unit, v6) : \
                                   BCM_XGS3_L3_ENT_SZ(unit, v4));

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, v6) : \
                                        BCM_XGS3_L3_MEM_FIELDS(unit, v4));

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_l3_entry_key_type) && ipv6) {
        /* Set address lower part (64-127). */
        soc_mem_ip6_addr_set(unit, mem, buf_p, IP_ADDR_LWR_64f,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_LOWER_ONLY);

        /* Set address upper part (0-63). */
        soc_mem_ip6_addr_set(unit, mem, buf_p, IP_ADDR_UPR_64f,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_UPPER_ONLY);

        /* Mark entry as ipv6. */
        soc_mem_field32_set(unit, mem, buf_p, KEY_TYPE_0f, TR_L3_HASH_KEY_TYPE_V6UC);
        soc_mem_field32_set(unit, mem, buf_p, KEY_TYPE_1f, TR_L3_HASH_KEY_TYPE_V6UC);

        /* Set valid bit. */
        soc_mem_field32_set(unit, mem, buf_p, VALID_1f, 1);
    } else
#endif /* BCM_TRIUMPH_SUPPORT*/
    if (SOC_IS_FBX(unit) && ipv6) {
        /* Set address lower part (64-127). */
        soc_mem_ip6_addr_set(unit, mem, buf_p, IP_ADDR_LWR_64f, 
                             l3cfg->l3c_ip6, SOC_MEM_IP6_LOWER_ONLY);

        /* Set address upper part (0-63). */
        soc_mem_ip6_addr_set(unit, mem, buf_p, IP_ADDR_UPR_64f, 
                             l3cfg->l3c_ip6, SOC_MEM_IP6_UPPER_ONLY);

        /* Mark entry as ipv6. */
        soc_mem_field32_set(unit, mem, buf_p, V6_0f, 1);
        soc_mem_field32_set(unit, mem, buf_p, V6_1f, 1);

        /* Set valid bit. */
        soc_mem_field32_set(unit, mem, buf_p, VALID_1f, 1);
    } else if(SOC_IS_EASYRIDER(unit) && ipv6) {
        /* Set ip address. */
        soc_mem_ip6_addr_set(unit, mem, buf_p, IP_ADDRf,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_FULL_ADDR);
    } else { 
#ifdef BCM_TRIUMPH_SUPPORT
        if (soc_feature(unit, soc_feature_l3_entry_key_type)) { 
            /* Mark entry as ipv4. */
            soc_mem_field32_set(unit, mem, buf_p, KEY_TYPEf, 
                                TR_L3_HASH_KEY_TYPE_V4UC);
        }
#endif /* BCM_TRIUMPH_SUPPORT */

        /* Set ip address. */
        soc_mem_field32_set(unit, mem, buf_p, IP_ADDRf, l3cfg->l3c_ip_addr);
    }

    /* Set virtual router id. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->vrf)) {
        soc_mem_field32_set(unit, mem, buf_p, fld->vrf, l3cfg->l3c_vrf);
    }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_SCORPION_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, mem, VRF_ID_1f) && ipv6) {
        soc_mem_field32_set(unit, mem, buf_p, VRF_ID_1f, l3cfg->l3c_vrf);
    } 
#endif /* BCM_FIREBOLT2_SUPPORT  || BCM_RAVEN_SUPPORT || BCM_SCORPION_SUPPORT */

    /* Set entry valid bit. */
    soc_mem_field32_set(unit, mem, buf_p, fld->valid, 1);
}


/*
 * Function:
 *      _bcm_xgs3_l3_ent_parse
 * Purpose:
 *      Service routine used to parse hw l3 entry to api format.
 * Parameters:
 *      unit      - (IN)SOC unit number. 
 *      mem       - (IN)L3 table memory.
 *      l3cfg     - (IN/OUT)l3 entry key & parse result.
 *      nh_idx    - (IN/OUT)Next hop index. 
 *      l3x_entry - (IN)hw buffer.
 * Returns:
 *      void
 */
STATIC void
_bcm_xgs3_l3_ent_parse(int unit, soc_mem_t mem, 
                       _bcm_l3_cfg_t *l3cfg, int *nh_idx, void *l3x_entry)
{
    _bcm_l3_fields_t *fld;        /* L3 table common fields.     */
    uint32 *buf_p;                /* HW buffer address.          */ 
    int ipv6;                     /* Entry is IPv6 flag.         */

    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);
    buf_p = (uint32 *)l3x_entry;

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, v6) : \
                               BCM_XGS3_L3_MEM_FIELDS(unit, v4));

    /* Reset entry flags first. */
    l3cfg->l3c_flags = (ipv6) ? BCM_L3_IP6 : 0;


    /* Get info from L3 and next hop table */
    if (soc_mem_field32_get(unit, mem, buf_p, fld->hit) ||
        BCM_XGS3_V6_FLD32_GET_IF_FBX(unit, ipv6, mem, buf_p, HIT_1f)) {
        l3cfg->l3c_flags |= BCM_L3_HIT;
    }

    /* Get priority override flag. */
    if (soc_mem_field32_get(unit, mem, buf_p, fld->rpe)) {
        l3cfg->l3c_flags |= BCM_L3_RPE;
    }

    /* Get destination discard flag. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->dst_discard)) {
       if (soc_mem_field32_get(unit, mem, buf_p, fld->dst_discard)) {
           l3cfg->l3c_flags |= BCM_L3_DST_DISCARD;
       }
    }

#if defined(BCM_TRX_SUPPORT)
    /* Get classification group id. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->class_id)) {
        l3cfg->l3c_lookup_class = 
            soc_mem_field32_get(unit, mem, buf_p, fld->class_id);
    }
#endif /* BCM_TRX_SUPPORT */

    /* Get priority. */
    l3cfg->l3c_prio = soc_mem_field32_get(unit, mem, buf_p, fld->priority);

    /* Get virtual router id. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->vrf)) {
        l3cfg->l3c_vrf = soc_mem_field32_get(unit, mem, buf_p, fld->vrf);
    } else {
        l3cfg->l3c_vrf = BCM_L3_VRF_DEFAULT;
    }

    /* Get next hop info. */
    if (nh_idx) {
        *nh_idx = soc_mem_field32_get(unit, mem, buf_p, fld->nh_idx);
    }

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* Get source discard flag. */
        if (soc_mem_field32_get(unit, mem, buf_p, SRC_DISCARDf)) {
            l3cfg->l3c_flags |= BCM_L3_SRC_DISCARD;
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    return;
}

/*
 * Function:
 *      _bcm_xgs3_l3_clear_hit
 * Purpose:
 *      Clear hit bit on l3 entry
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      mem       - (IN)L3 table memory.
 *      l3cfg     - (IN)l3 entry info. 
 *      l3x_entry - (IN)l3 entry filled hw buffer.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_clear_hit(int unit, soc_mem_t mem,
                       _bcm_l3_cfg_t *l3cfg, void *l3x_entry)
{
    _bcm_l3_fields_t *fld;        /* L3 table common fields.  */
    uint32 *buf_p;                /* HW buffer address.       */ 
    int mcast;                    /* Entry is multicast flag. */ 
    int ipv6;                     /* Entry is IPv6 flag.      */
#ifdef BCM_FIREBOLT_SUPPORT
    int idx;                      /* Iterator index.          */
    soc_field_t hitf[] = { HIT_0f, HIT_1f, HIT_2f, HIT_3f };
#endif /* BCM_FIREBOLT_SUPPORT */

    /* Input parameters check */
    if ((NULL == l3cfg) || (NULL == l3x_entry)) {
        return (BCM_E_PARAM);
    }

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);
    mcast = (l3cfg->l3c_flags & BCM_L3_IPMC);

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, v6) : \
                               BCM_XGS3_L3_MEM_FIELDS(unit, v4));

    /* Init memory pointers. */ 
    buf_p = (uint32 *)l3x_entry;

    /* If entry was not hit  there is nothing to clear */
    if (!(l3cfg->l3c_flags & BCM_L3_HIT)) {
        return (BCM_E_NONE);
    }

#ifdef BCM_FIREBOLT_SUPPORT
    /* Reset entry hit bit in hw. */
    if (ipv6 && mcast) {
        /* IPV6 multicast entry hit reset. */
        if (SOC_IS_FBX(unit)) {
            for (idx = 1; idx < 4; idx++) {
                soc_mem_field32_set(unit, mem, buf_p, hitf[idx], 0);
            }
        }
    } else if (ipv6) {
        /* Reset IPV6 unicast  hit bit. */
        if (SOC_IS_FBX(unit)) {
            for (idx = 1; idx < 2; idx++) {
                soc_mem_field32_set(unit, mem, buf_p, hitf[idx], 0);
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    /* Reset hit bit. */
    soc_mem_field32_set(unit, mem, buf_p, fld->hit, 0);

    /* Write entry back to hw. */
    return BCM_XGS3_MEM_WRITE(unit, mem, l3cfg->l3c_hw_index, buf_p);
}

/*
 * Function:
 *      _bcm_xgs3_l3_get
 * Purpose:
 *      Get an entry from L3 table.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      l3cfg    - (IN/OUT)l3 entry  lookup key & search result.
 *      nh_index - (IN/OUT)Next hop index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_get(int unit, _bcm_l3_cfg_t *l3cfg, int *nh_idx)
{
    l3_entry_ipv6_unicast_entry_t l3x_key;      /* Lookup entry buffer.     */
    l3_entry_ipv6_unicast_entry_t l3x_entry;    /* Search result buffer.    */
    soc_mem_t mem;                              /* L3 table memory.         */  
    int clear_hit;                              /* Clear hit bit.           */
    int ipv6;                                   /* IPv6 entry indicator.    */
    int rv = BCM_E_NONE;                        /* Operation return status. */

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Preserve clear_hit value. */
    clear_hit = l3cfg->l3c_flags & BCM_L3_HIT_CLEAR;

    /* Get table memory. */
    mem = (ipv6) ? BCM_XGS3_L3_MEM(unit, v6) : BCM_XGS3_L3_MEM(unit, v4);
    if (INVALIDm == mem) {
        return (BCM_E_NOT_FOUND);
    }

    /* Prepare lookup key. */
    /* coverity[overrun-buffer-val : FALSE]    */
    _bcm_xgs3_l3_ent_init(unit, mem, l3cfg, &l3x_key);

    /* Perform lookup hw. */
#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        /* coverity[overrun-buffer-val : FALSE] */
        rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &l3cfg->l3c_hw_index,
                            &l3x_key, &l3x_entry, 0);
    } else
#endif /*  BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        rv = soc_fb_l3x_lookup(unit, (void *)&l3x_key, 
                               (void *)&l3x_entry, &l3cfg->l3c_hw_index);
    }
#endif /*  BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        if (ipv6) {
            rv = soc_er_l3v6_lookup(unit, (void *)&l3x_key, 
                                    (void *)&l3x_entry, &l3cfg->l3c_hw_index);
        } else {
            rv = soc_er_l3v4_lookup(unit, (void *)&l3x_key, 
                                    (void *)&l3x_entry, &l3cfg->l3c_hw_index);
        }
    }
#endif /*  BCM_FIREBOLT_SUPPORT */
    BCM_XGS3_LKUP_IF_ERROR_RETURN(rv, BCM_E_NOT_FOUND);

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TRIDENT(unit)) {
        soc_mem_t mem_y;
        l3_entry_ipv6_unicast_entry_t l3x_entry_y;
        uint32 hit;
        mem_y = ipv6 ? L3_ENTRY_IPV6_UNICAST_Ym : L3_ENTRY_IPV4_UNICAST_Ym;
        BCM_IF_ERROR_RETURN
            (BCM_XGS3_MEM_READ(unit, mem_y, l3cfg->l3c_hw_index,
                               &l3x_entry_y));
        if (ipv6) {
            hit = soc_mem_field32_get(unit, mem, &l3x_entry, HIT_0f);
            hit |= soc_mem_field32_get(unit, mem_y, &l3x_entry_y, HIT_0f);
            soc_mem_field32_set(unit, mem, &l3x_entry, HIT_0f, hit);
            hit = soc_mem_field32_get(unit, mem, &l3x_entry, HIT_1f);
            hit |= soc_mem_field32_get(unit, mem_y, &l3x_entry_y, HIT_1f);
            soc_mem_field32_set(unit, mem, &l3x_entry, HIT_1f, hit);
        } else {
            hit = soc_mem_field32_get(unit, mem, &l3x_entry, HITf);
            hit |= soc_mem_field32_get(unit, mem_y, &l3x_entry_y, HITf);
            soc_mem_field32_set(unit, mem, &l3x_entry, HITf, hit);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Extract entry info. */
    _bcm_xgs3_l3_ent_parse(unit, mem, l3cfg, nh_idx, &l3x_entry);

    /* Clear the HIT bit */
    if (clear_hit) {
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_clear_hit(unit, mem,
                                                   l3cfg, &l3x_entry));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_add
 * Purpose:
 *      Add an entry to L3 table.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      l3cfg    - (IN)l3 entry information.
 *      nh_idx   - (IN)Next hop index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_add(int unit, _bcm_l3_cfg_t *l3cfg, int nh_idx)
{
    l3_entry_ipv6_unicast_entry_t l3x_entry;    /* Buffer for write        */
    _bcm_l3_fields_t *fld;                      /* L3 table common fields. */
    uint32 *buf_p;                              /* Hardware buffer address.*/
    soc_mem_t mem;                              /* L3 table memory.        */  
    int ipv6;                                   /* IPv6 entry indicator.   */
    int rv = BCM_E_NONE;                        /* Operation status.       */

    buf_p = (uint32 *)&l3x_entry;

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, v6) : \
                               BCM_XGS3_L3_MEM_FIELDS(unit, v4));

    /* Get table memory. */
    mem = (ipv6) ? BCM_XGS3_L3_MEM(unit, v6) : BCM_XGS3_L3_MEM(unit, v4);
    if (INVALIDm == mem) {
        return (BCM_E_DISABLED);
    }

    /* Prepare hw entry for addition. */
    _bcm_xgs3_l3_ent_init(unit, mem, l3cfg, buf_p);

    /* Set hit bit. */
    if (l3cfg->l3c_flags & BCM_L3_HIT) {
        soc_mem_field32_set(unit, mem, buf_p, fld->hit, 1);
        BCM_XGS3_V6_FLD32_SET_IF_FBX(unit, ipv6, mem, buf_p, HIT_1f, 1);
     }

    /* Set priority override bit. */
    if (l3cfg->l3c_flags & BCM_L3_RPE) {
        soc_mem_field32_set(unit, mem, buf_p, fld->rpe, 1);
        BCM_XGS3_V6_FLD32_SET_IF_FBX(unit, ipv6, mem, buf_p, RPE_1f, 1);
    }

    /* Set destination discard bit. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->dst_discard)) {
        if (l3cfg->l3c_flags & BCM_L3_DST_DISCARD) {
            soc_mem_field32_set(unit, mem, buf_p, fld->dst_discard, 1);
            BCM_XGS3_V6_FLD32_SET_IF_FBX(unit, ipv6, mem, buf_p,
                                         DST_DISCARD_1f, 1);
        }
    }

#if defined(BCM_TRX_SUPPORT)
    /* Get classification group id. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->class_id)) {
        soc_mem_field32_set(unit, mem, buf_p, fld->class_id, 
                            l3cfg->l3c_lookup_class);
        BCM_XGS3_V6_FLD32_SET_IF_FBX(unit, ipv6, mem, buf_p,
                                     CLASS_ID_1f, l3cfg->l3c_lookup_class);
    }
#endif /* BCM_TRX_SUPPORT */

    /*  Set priority. */
    soc_mem_field32_set(unit, mem, buf_p, fld->priority, l3cfg->l3c_prio);
    BCM_XGS3_V6_FLD32_SET_IF_FBX(unit, ipv6, mem, buf_p,
                                         PRI_1f, l3cfg->l3c_prio);
    /* Set next hop index. */
    soc_mem_field32_set(unit, mem, buf_p, fld->nh_idx, nh_idx);
    BCM_XGS3_V6_FLD32_SET_IF_FBX(unit, ipv6, mem, buf_p,
                                         NEXT_HOP_INDEX_1f, nh_idx);

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)){
        /* Set source discard bit. */
        if (l3cfg->l3c_flags & BCM_L3_SRC_DISCARD) {
            soc_mem_field32_set(unit, mem, buf_p, SRC_DISCARDf, 1);
        }
    } 
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Write entry to hw. */
    /* Handle replacement if hw index already known, write directly. */
    if (BCM_XGS3_L3_INVALID_INDEX != l3cfg->l3c_hw_index) {
        rv = BCM_XGS3_MEM_WRITE(unit, mem, l3cfg->l3c_hw_index, buf_p);
    } else {
#ifdef BCM_TRX_SUPPORT
        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            rv = soc_mem_insert(unit, mem, MEM_BLOCK_ANY, (void *)&l3x_entry);
        } else
#endif /*  BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(unit)) {
            rv = soc_fb_l3x_insert(unit, (void *)&l3x_entry);
        } 
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
        if (SOC_IS_EASYRIDER(unit)){
            rv = soc_mem_insert(unit, mem, COPYNO_ALL, buf_p);
        }
#endif /* BCM_EASYRIDER_SUPPORT */
    }

    /* Write status check. */
    if ((rv >= 0) && (BCM_XGS3_L3_INVALID_INDEX == l3cfg->l3c_hw_index)) {
        (ipv6) ?  BCM_XGS3_L3_IP6_CNT(unit)++ : BCM_XGS3_L3_IP4_CNT(unit)++;
    }
    return rv;
}


/*
 * Function:
 *      _bcm_xgs3_l3_bucket_get
 * Purpose:
 *      Get l3 entry bucket start index.
 * Parameters:
 *      unit          - (IN)SOC unit number.
 *      l3cfg         - (IN)l3 entry information.
 *      idx_primary   - (OUT)Primary bank bucket start index.
 *      idx_secondary - (OUT)Secondary bank bucket start index.
 * Returns:
 *      BCM_E_XXX
 * NOTE: 
 *     idx_primary = points to the first half of the bucket.
 *     idx_secondary = points to the second half of the bucket.
 *     If dual hash is not supported idx_primary & idx_secondary 
 *     point to the same bucket.
 */

STATIC int
_bcm_xgs3_l3_bucket_get (int unit, _bcm_l3_cfg_t *l3cfg,
                         int *idx_primary, int *idx_secondary)
{
    l3_entry_ipv6_multicast_entry_t l3x_entry;  /* Buffer for hw  write    */
    uint32 *buf_p;                              /* Hardware buffer address.*/
    soc_mem_t mem;                              /* L3 table memory.        */  
    uint8 ipv6;                                 /* IPv6 entry indicator.   */
    uint8 ipmc;                                 /* IPMC entry indicator.   */
    int prim_idx;                               /* Primary bucket.         */
    int sec_idx;                                /* Secondary bucket.       */

    if ((NULL == idx_primary) || (NULL == idx_secondary)) {
        return (BCM_E_PARAM);
    }

    buf_p = (uint32 *)&l3x_entry;

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6) ? 1 : 0;
    ipmc = (l3cfg->l3c_flags & BCM_L3_IPMC) ? 1 : 0;
    
    /* Get table memory. */
    mem = (ipv6) ? BCM_XGS3_L3_MEM(unit, v6) : BCM_XGS3_L3_MEM(unit, v4);
    if (INVALIDm == mem) {
        return (BCM_E_DISABLED);
    }

    /* Prepare hw entry for addition. */

    if (ipmc) {
#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_IS_TR_VL(unit)) {
            _bcm_tr_l3_ipmc_ent_init(unit, buf_p, l3cfg);
        } else 
#endif /* BCM_TRIUMPH_SUPPORT */
        {
#ifdef BCM_FIREBOLT_SUPPORT
            _bcm_fb_l3_ipmc_ent_init(unit, buf_p, l3cfg);
#endif /* BCM_FIREBOLT_SUPPORT */
        }
    } else {
        _bcm_xgs3_l3_ent_init(unit, mem, l3cfg, buf_p);
    }
 
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) ||\
    defined(BCM_TRX_SUPPORT)
        if (soc_feature(unit, soc_feature_dual_hash)) {

            prim_idx = 
                soc_fb_l3x_bank_entry_hash(unit, 0, (uint32 *)&l3x_entry);
                if (prim_idx < 0) {
                    return prim_idx;
                }
            sec_idx = 
                soc_fb_l3x_bank_entry_hash(unit, 1, (uint32 *)&l3x_entry);
                if (sec_idx < 0) {
                    return sec_idx;
                }
        } else 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
        {
           sec_idx = prim_idx = soc_fb_l3x2_entry_hash(unit, (uint32 *)&l3x_entry);
        } 
    } else  
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        sec_idx = prim_idx = soc_er_l3x_hash(unit, (uint32 *)&l3x_entry, ipv6);
    }  else 
#endif /* BCM_EASYRIDER_SUPPORT */
    {
        *idx_primary = *idx_secondary = -1;
        return (BCM_E_UNAVAIL);
    } 

    /* First half of primary bucket. */
    *idx_primary = prim_idx * SOC_L3X_BUCKET_SIZE(unit);

    /* Second half of secondary bucket */
    *idx_secondary = sec_idx * SOC_L3X_BUCKET_SIZE(unit) +
        SOC_L3X_BUCKET_SIZE(unit)/2;

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_xgs3_l3_del
 * Purpose:
 *      Delete an entry to L3 table.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      l3cfg    - (IN/OUT)l3 entry deletion key.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_del(int unit, _bcm_l3_cfg_t *l3cfg)
{
    l3_entry_ipv6_unicast_entry_t l3x_entry;    /* Buffer for write        */
    soc_mem_t mem;                              /* L3 table memory.        */  
    int ipv6;                                   /* IPv6 entry indicator.   */
    int rv = BCM_E_NONE;                        /* Operation status.       */

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Get table memory. */
    mem = (ipv6) ? BCM_XGS3_L3_MEM(unit, v6) : BCM_XGS3_L3_MEM(unit, v4);
    if (INVALIDm == mem) {
        return (BCM_E_DISABLED);
    }

    /* Prepare hw entry for addition. */
    _bcm_xgs3_l3_ent_init(unit, mem, l3cfg, &l3x_entry);

    /* Write entry to hw. */
#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        rv = soc_mem_delete(unit, mem, MEM_BLOCK_ANY, (void *)&l3x_entry);
    } else
#endif /*  BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        rv = soc_fb_l3x_delete(unit, (void *)&l3x_entry);
    } 
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        rv = soc_mem_delete(unit, mem, COPYNO_ALL, &l3x_entry);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Write status check. */
    if (rv >= 0) {
        (ipv6) ?  BCM_XGS3_L3_IP6_CNT(unit)-- : BCM_XGS3_L3_IP4_CNT(unit)--;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_xgs3_l3_get_by_idx
 * Purpose:
 *      Get an entry from L3 table by index.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      dma_ptr  - (IN)Table pointer in dma. 
 *      idx      - (IN)Index to read.
 *      l3cfg    - (OUT)l3 entry search result.
 *      nh_index - (OUT)Next hop index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_get_by_idx(int unit, void *dma_ptr, int idx,
                        _bcm_l3_cfg_t *l3cfg, int *nh_idx)
{
    l3_entry_ipv6_unicast_entry_t *l3x_entry_p; /* Read buffer address.    */
    l3_entry_ipv6_unicast_entry_t l3x_entry;    /* Read buffer.            */
    _bcm_l3_fields_t *fld;                      /* L3 table common fields. */
    soc_mem_t mem;                              /* L3 table memory.        */  
    int clear_hit;                              /* Clear hit bit flag.     */
    uint32 ipv6;                                /* IPv6 entry indicator.   */

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Get table memory. */
    mem = (ipv6) ? BCM_XGS3_L3_MEM(unit, v6) : BCM_XGS3_L3_MEM(unit, v4);
    
    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, v6) : \
                               BCM_XGS3_L3_MEM_FIELDS(unit, v4));

    /* Get clear hit flag. */
    clear_hit = l3cfg->l3c_flags & BCM_L3_HIT_CLEAR;

    if (NULL == dma_ptr) {             /* Read from hardware. */
        l3x_entry_p = &l3x_entry;
        /* Zero buffers. */
        sal_memset(l3x_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, v6));

        /* Read entry from hw. */
        BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, l3x_entry_p)); 

    } else {                    /* Read from dma. */
        l3x_entry_p =
            soc_mem_table_idx_to_pointer(unit, mem,
                                         l3_entry_ipv6_unicast_entry_t *,
                                         dma_ptr, idx);
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TRIDENT(unit)) {
        soc_mem_t mem_y;
        l3_entry_ipv6_unicast_entry_t l3x_entry_y;
        uint32 hit;
        mem_y = ipv6 ? L3_ENTRY_IPV6_UNICAST_Ym : L3_ENTRY_IPV4_UNICAST_Ym;
        BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem_y, idx, &l3x_entry_y));
        if (ipv6) {
            hit = soc_mem_field32_get(unit, mem, l3x_entry_p, HIT_0f);
            hit |= soc_mem_field32_get(unit, mem_y, &l3x_entry_y, HIT_0f);
            soc_mem_field32_set(unit, mem, l3x_entry_p, HIT_0f, hit);
            hit = soc_mem_field32_get(unit, mem, l3x_entry_p, HIT_1f);
            hit |= soc_mem_field32_get(unit, mem_y, &l3x_entry_y, HIT_1f);
            soc_mem_field32_set(unit, mem, l3x_entry_p, HIT_1f, hit);
        } else {
            hit = soc_mem_field32_get(unit, mem, l3x_entry_p, HITf);
            hit |= soc_mem_field32_get(unit, mem_y, &l3x_entry_y, HITf);
            soc_mem_field32_set(unit, mem, l3x_entry_p, HITf, hit);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Ignore invalid entries. */
    if (!soc_mem_field32_get(unit, mem, l3x_entry_p, fld->valid)) {
        return (BCM_E_NOT_FOUND);
    }
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_l3_entry_key_type)) { 
        /* Get protocol. */
        int       key_type;

        key_type = soc_mem_field32_get(unit, L3_ENTRY_ONLYm,
                                       l3x_entry_p, KEY_TYPEf);

        switch (key_type) {
            case TR_L3_HASH_KEY_TYPE_V4UC:
                l3cfg->l3c_flags = 0;
                break;
            case TR_L3_HASH_KEY_TYPE_V4MC:
                l3cfg->l3c_flags = BCM_L3_IPMC;
                break;
            case TR_L3_HASH_KEY_TYPE_V6UC:
                l3cfg->l3c_flags = BCM_L3_IP6;
                break;
            case TR_L3_HASH_KEY_TYPE_V6MC:
                l3cfg->l3c_flags = BCM_L3_IP6 | BCM_L3_IPMC;
                break;
            default:
                break;
        }

        /* Ignore protocol mismatch & multicast entries. */
        if ((ipv6  != (l3cfg->l3c_flags & BCM_L3_IP6)) ||
            (l3cfg->l3c_flags & BCM_L3_IPMC)) {
            return (BCM_E_NONE);
        }

        /* Get host ip address. */
        if (ipv6) {
            /* Extract host info from the entry. */
            soc_mem_ip6_addr_get(unit, mem, l3x_entry_p,
                                 IP_ADDR_LWR_64f, l3cfg->l3c_ip6,
                                 SOC_MEM_IP6_LOWER_ONLY);

            soc_mem_ip6_addr_get(unit, mem, l3x_entry_p,
                                 IP_ADDR_UPR_64f, l3cfg->l3c_ip6,
                                 SOC_MEM_IP6_UPPER_ONLY);
        }
    } else
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        /* Get protocol. */
        if (soc_mem_field32_get(unit, mem, l3x_entry_p, fld->v6_entry)){
            l3cfg->l3c_flags = BCM_L3_IP6;
        } else {
            l3cfg->l3c_flags = 0;
        }

        /* Get ipmc flag . */
        if (soc_mem_field32_get(unit, mem, l3x_entry_p, fld->ipmc_entry)) {
            l3cfg->l3c_flags |= BCM_L3_IPMC;
        }

        /* Ignore protocol mismatch & multicast entries. */
        if ((ipv6  != (l3cfg->l3c_flags & BCM_L3_IP6)) ||
            (l3cfg->l3c_flags & BCM_L3_IPMC)) {
            return (BCM_E_NONE); 
        }
            

        /* Get host ip address. */
        if (ipv6) {
            /* Extract host info from the entry. */
            soc_mem_ip6_addr_get(unit, mem, l3x_entry_p,
                                 IP_ADDR_LWR_64f, l3cfg->l3c_ip6,
                                 SOC_MEM_IP6_LOWER_ONLY);

            soc_mem_ip6_addr_get(unit, mem, l3x_entry_p,
                                 IP_ADDR_UPR_64f, l3cfg->l3c_ip6,
                                 SOC_MEM_IP6_UPPER_ONLY);
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit) && ipv6) {
        soc_mem_ip6_addr_get(unit, mem, l3x_entry_p, IP_ADDRf,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_FULL_ADDR);
    } 
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Set index to l3cfg. */
    l3cfg->l3c_hw_index = idx;

    if (!ipv6) {
        l3cfg->l3c_ip_addr = 
            soc_mem_field32_get(unit, mem, l3x_entry_p, IP_ADDRf);
    }

    /* Parse entry data. */
    _bcm_xgs3_l3_ent_parse(unit, mem, l3cfg, nh_idx, l3x_entry_p);

    /* Clear the HIT bit */
    if (clear_hit) {
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_clear_hit(unit, mem,
                                                   l3cfg, (void *)l3x_entry_p));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_nh_inc_ref_count
 * Purpose:
 *      Increment Next Hop Reference count
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      nh_index - (IN)Next hop index.
 */

void
bcm_xgs3_nh_inc_ref_count(int unit, int nh_index)
{
    _bcm_l3_tbl_t *nh_tbl_ptr; 			 /* Next hop table ptr. 	 */

    /* Get table pointers */
    nh_tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);

    BCM_XGS3_L3_ENT_REF_CNT_INC(nh_tbl_ptr, nh_index,
                             _BCM_SINGLE_WIDE);
}

/*
 * Function:
 *      _bcm_xgs3_nh_dec_ref_count
 * Purpose:
 *      Decrement Next Hop Reference count
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      nh_index - (IN)Next hop index.
 */

void
bcm_xgs3_nh_dec_ref_count(int unit, int nh_index)
{
    _bcm_l3_tbl_t *nh_tbl_ptr; 			 /* Next hop table ptr. 	 */

    /* Get table pointers */
    nh_tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);

    BCM_XGS3_L3_ENT_REF_CNT_DEC(nh_tbl_ptr, nh_index,
                             _BCM_SINGLE_WIDE);

}

#ifdef BCM_FIREBOLT_SUPPORT
/*
 * Function:
 *      _bcm_fb_l3_intf_vrf_get
 * Purpose:
 *      Get the VRF,flags info for the specified L3 interface
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      intf_info - (IN/OUT)Interface configuration info.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_fb_l3_intf_vrf_get(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    int ret_val;                           /* Operation return value.       */
    bcm_vlan_control_vlan_t vlan_control;  /* Vlan configuration structure. */
       
    ret_val = bcm_esw_vlan_control_vlan_get(unit, intf_info->l3i_vid, 
                                            &vlan_control);
    if (BCM_SUCCESS(ret_val)) {
        /* Get Virtual Router id */
        intf_info->l3i_vrf = vlan_control.vrf;
    } else {
        intf_info->l3i_vrf = BCM_L3_VRF_DEFAULT;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_fb_l3_intf_vrf_bind
 * Purpose:
 *      Bind interface to the VRF & update if info.
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface configuration info.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_fb_l3_intf_vrf_bind(int unit, _bcm_l3_intf_cfg_t *intf_info)
{

    int ret_val;                           /* Operation return value.       */
    bcm_vlan_control_vlan_t vlan_control;  /* Vlan configuration structure. */

    sal_memset(&vlan_control, 0, sizeof(vlan_control));       
    ret_val = bcm_esw_vlan_control_vlan_get(unit, intf_info->l3i_vid, &vlan_control);
    if ((BCM_E_NOT_FOUND == ret_val) || (BCM_E_UNAVAIL == ret_val)) {
        /* Assuming application will set vrf id later on through vlan commands. */
        return (BCM_E_NONE);
    }

    /* Return any other error. */
    if (BCM_FAILURE(ret_val)){
        return (ret_val);
    }

    if (intf_info->l3i_vrf >= 0) {
        vlan_control.vrf = intf_info->l3i_vrf;
        ret_val = bcm_esw_vlan_control_vlan_set(unit, intf_info->l3i_vid, vlan_control);
    }
    return (ret_val);
}

/*
 * Function:
 *      _bcm_fb_nh_add
 * Purpose:
 *      Routine set next hop entry in the chip.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      idx       - (IN)Allocated entry index.
 *      buf       - (IN)Next hop entry data.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_fb_nh_add(int unit, int idx, void *buf, int max_paths)
{
    ing_l3_next_hop_entry_t in_entry;   /* Buffer to read ingress nh entry. */
    egr_l3_next_hop_entry_t eg_entry;   /* Buffer to read egress nh entry.  */
    bcm_l3_egress_t *nh_entry;          /* Next hop entry passed buffer.    */
    int tunnel_id;                      /* Tunnel id attached to interface. */
    soc_mem_t mem;                      /* Next hop table memory.           */
    uint32 entry_type=0;

    /* Input parameters check */
    if (NULL == buf) {
        return (BCM_E_PARAM);
    }
    nh_entry = (bcm_l3_egress_t *) buf;
    /* Zero buffers. */
    sal_memset(&in_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, nh));
    sal_memset(&eg_entry, 0, sizeof(egr_l3_next_hop_entry_t));

    mem = ING_L3_NEXT_HOPm;

    /* Read ingress next hop entry. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, &in_entry));

    /* Set entry vlan id. */
    soc_mem_field32_set(unit, mem, &in_entry, VLAN_IDf, nh_entry->vlan);

    /* Set module id. */
    soc_mem_field32_set(unit, mem, &in_entry, MODULE_IDf, nh_entry->module);

#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, mem, INTF_NUMf)) {
        soc_mem_field32_set(unit, mem, &in_entry, INTF_NUMf, nh_entry->intf);
    }
#endif /* BCM_HURRICANE_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_trunk_group_overlay)) {
        if (nh_entry->flags & BCM_L3_TGID) {
            soc_mem_field32_set(unit, mem, &in_entry, Tf, 1);
            soc_mem_field32_set(unit, mem, &in_entry, TGIDf, nh_entry->port);
        } else {
            soc_mem_field32_set(unit, mem, &in_entry, PORT_NUMf, nh_entry->port);
        }
    } else 
#endif /* BCM_TRX_SUPPORT */
    {
        /* Set port/trunk id. */
        soc_mem_field32_set(unit, mem, &in_entry, PORT_TGIDf, nh_entry->port);
    }

    if (nh_entry->flags & BCM_L3_COPY_TO_CPU)  {
        if (SOC_MEM_FIELD_VALID(unit, mem, COPY_TO_CPUf)) {
            soc_mem_field32_set(unit, mem, &in_entry, COPY_TO_CPUf, 1);
        } else {
            return (BCM_E_UNAVAIL);
        }
    }

    if (nh_entry->flags & BCM_L3_DST_DISCARD) {
        if (SOC_MEM_FIELD_VALID(unit, mem, DROPf)) {
            soc_mem_field32_set(unit, mem, &in_entry, DROPf, 1);
        }
        /*
         * Host or route entry will handle drop
         * condition no else required.
         */
    }

    /* Set interface is tunnel bit. */
    BCM_IF_ERROR_RETURN
        (_bcm_xgs3_l3_get_tunnel_id(unit, nh_entry->intf, &tunnel_id));

    /* Set flag that packet going to be tunneled. */ 
    if (tunnel_id) {
        if (SOC_MEM_FIELD_VALID(unit, mem, L3UC_TUNNEL_TYPEf)) {
            soc_mem_field32_set(unit, mem, &in_entry, L3UC_TUNNEL_TYPEf, 1);
        }
#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_MEM_FIELD_VALID(unit, mem, ENTRY_TYPEf)) {
            soc_mem_field32_set(unit, mem, &in_entry, ENTRY_TYPEf, 1);
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

    /* Write ingress next hop entry. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_WRITE(unit, mem, idx, &in_entry));

    mem = EGR_L3_NEXT_HOPm;
    /* Read egress next hop entry. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, &eg_entry));
#if defined(BCM_TRIUMPH_SUPPORT)
         if (SOC_MEM_FIELD_VALID(unit, mem, ENTRY_TYPEf)) {
              entry_type = soc_mem_field32_get (unit, mem, &eg_entry,
                                                   ENTRY_TYPEf);
         }
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (nh_entry->flags & BCM_L3_KEEP_VLAN) {
         if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_VLAN_DISABLEf)) {
              soc_mem_field32_set(unit, mem, &eg_entry, L3__L3_UC_VLAN_DISABLEf, 1);
         }
    }
    if (nh_entry->flags & BCM_L3_KEEP_TTL) {
         if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_TTL_DISABLEf)) {
              soc_mem_field32_set(unit, mem, &eg_entry, L3__L3_UC_TTL_DISABLEf, 1);
         }
    }
    if (nh_entry->flags & BCM_L3_KEEP_DSTMAC) {
         if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_DA_DISABLEf)) {
              soc_mem_field32_set(unit, mem, &eg_entry, L3__L3_UC_DA_DISABLEf, 1);
         }
    }
    if (nh_entry->flags & BCM_L3_KEEP_SRCMAC) {
         if (SOC_MEM_FIELD_VALID(unit, mem, L3__L3_UC_SA_DISABLEf)) {
              soc_mem_field32_set(unit, mem, &eg_entry, L3__L3_UC_SA_DISABLEf, 1);
         }
    }

    if (soc_feature(unit, soc_feature_wlan) && nh_entry->encap_id > 0) {
        /* Set ENTRY_TYPE to WLAN DVP */
        soc_mem_field32_set(unit, mem, &eg_entry, ENTRY_TYPEf, 4);
        /* Set next hop mac address. */
        soc_mem_mac_addr_set(unit, mem, &eg_entry, WLAN__MAC_ADDRESSf, 
                             nh_entry->mac_addr);
        /* Set interface id. */
        soc_mem_field32_set(unit, mem, &eg_entry, WLAN__INTF_NUMf, 
                            nh_entry->intf);
        /* Set the DVP */
        soc_mem_field32_set(unit, mem, &eg_entry, WLAN__DVPf, 
                            nh_entry->encap_id);
    } else
#endif
    {
        if (entry_type == 0) {
             /* Set next hop mac address. */
             soc_mem_mac_addr_set(unit, mem, &eg_entry, MAC_ADDRESSf, 
                             nh_entry->mac_addr);
        }
        /* Set interface id. */
        soc_mem_field32_set(unit, mem, &eg_entry, INTF_NUMf, nh_entry->intf);
    }
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_WRITE(unit, mem, idx, &eg_entry));
    
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
        defined(BCM_TRX_SUPPORT)
    /* Write initial ingress next hop entry. */ 
    mem = INITIAL_ING_L3_NEXT_HOPm;

    if (!SOC_MEM_IS_VALID(unit, mem)) {
        return (BCM_E_NONE);
    }
    /* Reset ingress buffer. */
    sal_memset(&in_entry, 0, sizeof(ing_l3_next_hop_entry_t));
    /* Read initial ingress next hop entry. */
    BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, &in_entry));

    /* Set module id to initial ingress l3 next hop entry. */
    soc_mem_field32_set(unit, mem, &in_entry, MODULE_IDf, nh_entry->module);

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_trunk_group_overlay)) {
        if (nh_entry->flags & BCM_L3_TGID) {
            soc_mem_field32_set(unit, mem, &in_entry, Tf, 1);
            soc_mem_field32_set(unit, mem, &in_entry, TGIDf, nh_entry->port);
        } else {
            soc_mem_field32_set(unit, mem, &in_entry, PORT_NUMf,
                                nh_entry->port);
        }
    } else 
#endif /* BCM_TRX_SUPPORT */
    {
        /* Set port/trunk id. */
        soc_mem_field32_set(unit, mem, &in_entry, PORT_TGIDf, nh_entry->port);
    }

    /* Write buffer to hw. */
    return  BCM_XGS3_MEM_WRITE(unit, mem, idx, &in_entry);
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_fb_nh_intf_is_tnl_update
 * Purpose:
 *      Routine updates next hop interface is tunnel bit. 
 *      Routine updates all next hop entries using a certain 
 *      interface tunnel bit.  
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      intf       - (IN)Interface index to update.
 *      tunnel_id  - (IN)Tunnel index.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_fb_nh_intf_is_tnl_update(int unit, bcm_if_t intf, int tunnel_id)
{
    uint32 *ing_entry_ptr;        /* Ingress next hop entry pointer.*/ 
    char *ing_tbl_ptr;            /* Dma table pointer.             */
    int ifindex;                  /* Interface index.               */
    soc_mem_t mem;                /* Next hop memory.               */
    int idx;                      /* Iteration index.               */
    char *egr_tbl_ptr;            /* Dma egress nh table pointer.   */
    uint32 *egr_entry_ptr = NULL; /* Egress next hop entry pointer. */ 
    int rv = BCM_E_NONE;          /* Operation return status.       */

    /* Get next table memory. */
    mem = BCM_XGS3_L3_MEM(unit, nh);

    /* Lock ingress next hop table. */

    /* Table DMA the nhtable to software copy */
    rv = bcm_xgs3_l3_tbl_dma(unit, mem, BCM_XGS3_L3_ENT_SZ(unit, nh), "nh_tbl",
                             &ing_tbl_ptr, NULL);
    BCM_IF_ERROR_RETURN(rv);

    /*  Read egress next hop entry. */
    rv = bcm_xgs3_l3_tbl_dma(unit, EGR_L3_NEXT_HOPm,
                             sizeof(egr_l3_next_hop_entry_t), "egr_nh_tbl",
                             &egr_tbl_ptr, NULL);
    if (rv < 0) {
        soc_cm_sfree(unit, ing_tbl_ptr);
        return rv;
    }

    for (idx = 0; idx < BCM_XGS3_L3_NH_TBL_SIZE(unit); idx++) {
        /* Skip unused entries. */
        if (!BCM_XGS3_L3_ENT_REF_CNT 
                (BCM_XGS3_L3_TBL_PTR(unit, next_hop), idx)) {
            continue;
        }

        /* Skip trap to CPU entry internally installed entry. */
        if (BCM_XGS3_L3_L2CPU_NH_IDX == idx) {
            continue;
        }

        /*  Read egress next hop entry. */
        egr_entry_ptr = 
            soc_mem_table_idx_to_pointer(unit, EGR_L3_NEXT_HOPm,
                                         uint32 *, egr_tbl_ptr, idx);

        /* Get interface index. */
        ifindex =  soc_mem_field32_get(unit, EGR_L3_NEXT_HOPm,
                                       egr_entry_ptr, INTF_NUMf);
        if (ifindex != intf) {
            continue;
        }

        /* Calculate entry ofset. */
        ing_entry_ptr =
            soc_mem_table_idx_to_pointer(unit, mem, uint32 *, ing_tbl_ptr, idx);
        if (SOC_MEM_FIELD_VALID(unit, mem, L3UC_TUNNEL_TYPEf)) {
            soc_mem_field32_set(unit, mem, ing_entry_ptr, L3UC_TUNNEL_TYPEf, 
                                (tunnel_id > 0) ? 1 : 0);
        }
#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_MEM_FIELD_VALID(unit, mem, ENTRY_TYPEf)) {
            soc_mem_field32_set(unit, mem, ing_tbl_ptr, ENTRY_TYPEf,
                                (tunnel_id > 0) ? 1 : 0);
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

    /* Write the buffer back. */
    rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL,
                             soc_mem_index_min(unit, mem),
                             soc_mem_index_max(unit, mem), ing_tbl_ptr); 

    soc_cm_sfree(unit, ing_tbl_ptr);
    soc_cm_sfree(unit, egr_tbl_ptr);
    return (rv);
}

/*
 * Function:
 *      _bcm_fb_l3_ipmc_ent_init
 * Purpose:
 *      Set GROUP/SOURCE/VID/IMPC flag in the entry.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3x_entry - (IN/OUT) IPMC entry to fill. 
 *      l3cfg     - (IN) Api IPMC data. 
 * Returns:
 *    BCM_E_XXX
 */
STATIC void
_bcm_fb_l3_ipmc_ent_init(int unit, uint32 *buf_p, _bcm_l3_cfg_t *l3cfg)
{
    soc_mem_t mem;                     /* IPMC table memory.    */
    int ipv6;                          /* IPv6 entry indicator. */
    soc_field_t v6f[] = { V6_0f, V6_1f, V6_2f, V6_3f };
    soc_field_t mcf[] = { IPMC_0f, IPMC_1f, IPMC_2f, IPMC_3f };
    soc_field_t vidf[] = { VLAN_ID_0f, VLAN_ID_1f, VLAN_ID_2f, VLAN_ID_3f };
    soc_field_t validf[] = { VALID_0f, VALID_1f, VALID_2f, VALID_3f };
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
    soc_field_t vrff[] = { VRF_ID_0f, VRF_ID_1f, VRF_ID_2f, VRF_ID_3f };
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    int idx;

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Get table memory. */
    mem =  (ipv6) ? L3_ENTRY_IPV6_MULTICASTm : L3_ENTRY_IPV4_MULTICASTm;

    if (ipv6) {
        /* Set group address. */
        soc_mem_ip6_addr_set(unit, mem, buf_p, GROUP_IP_ADDR_LWR_64f,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_LOWER_ONLY);
        l3cfg->l3c_ip6[0] = 0x0;    /* Don't write ff entry already mcast. */
        soc_mem_ip6_addr_set(unit, mem, buf_p, GROUP_IP_ADDR_UPR_56f, 
                             l3cfg->l3c_ip6, SOC_MEM_IP6_UPPER_ONLY);
        l3cfg->l3c_ip6[0] = 0xff;    /* Restore The entry  */

        /* Set source  address. */
        soc_mem_ip6_addr_set(unit, mem, buf_p, SOURCE_IP_ADDR_LWR_64f, 
                             l3cfg->l3c_sip6, SOC_MEM_IP6_LOWER_ONLY);
        soc_mem_ip6_addr_set(unit, mem, buf_p, SOURCE_IP_ADDR_UPR_64f, 
                             l3cfg->l3c_sip6, SOC_MEM_IP6_UPPER_ONLY);

        for (idx = 0; idx < 4; idx++) {
            /* Set address v6 bit. */
            soc_mem_field32_set(unit, mem, buf_p, v6f[idx], 1);

            /* Set address is multicast. */
            soc_mem_field32_set(unit, mem, buf_p, mcf[idx], 1);

            /* Set vlan id. */
            soc_mem_field32_set(unit, mem, buf_p, vidf[idx], l3cfg->l3c_vid);

            /* Set entry valid flag. */
            soc_mem_field32_set(unit, mem, buf_p, validf[idx], 1);

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
            /* Set virtual router id. */
           /*vrf id feild is reserved in hurricane*/
            if ((!(SOC_IS_HURRICANE(unit))) &&
		  (SOC_MEM_FIELD_VALID(unit, mem, vrff[idx]))) {
                soc_mem_field32_set(unit, mem, buf_p, 
                                    vrff[idx], l3cfg->l3c_vrf);
            }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
        }
    } else {
        /* Set group id. */
        soc_mem_field32_set(unit, mem, buf_p, GROUP_IP_ADDRf,
                            l3cfg->l3c_ip_addr);

        /* Set source address. */
        soc_mem_field32_set(unit, mem, buf_p, SOURCE_IP_ADDRf,
                            l3cfg->l3c_src_ip_addr);

        /* Set vlan id. */
        soc_mem_field32_set(unit, mem, buf_p, VLAN_IDf, l3cfg->l3c_vid);

        /* Set multicast flag. */
        soc_mem_field32_set(unit, mem, buf_p, IPMCf, 1);

        /* Set entry valid flag. */
        soc_mem_field32_set(unit, mem, buf_p, VALIDf, 1);

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT) 
        /* Set virtual router id. */
       /* vrf id is reserved in hurricane*/
        if ((!(SOC_IS_HURRICANE(unit))) &&
	     (SOC_MEM_FIELD_VALID(unit, mem, VRF_IDf))) {
            soc_mem_field32_set(unit, mem, buf_p, VRF_IDf, l3cfg->l3c_vrf);
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    }

    return;
	
}

/*
 * Function:
 *      _bcm_fb_l3_ipmc_ent_parse
 * Purpose:
 *      Service routine used to parse hw l3 ipmc entry to api format.
 * Parameters:
 *      unit      - (IN)SOC unit number. 
 *      l3cfg     - (IN/OUT)l3 entry parsing destination buf.
 *      l3x_entry - (IN/OUT)hw buffer.
 * Returns:
 *      void
 */
STATIC INLINE void
_bcm_fb_l3_ipmc_ent_parse(int unit, _bcm_l3_cfg_t *l3cfg,
                          l3_entry_ipv6_multicast_entry_t *l3x_entry)
{
    _bcm_l3_fields_t *fld;             /* IPMC table fields.    */   
    soc_mem_t mem;                     /* IPMC table memory.    */
    uint32 *buf_p;                     /* HW buffer address.    */
    int ipv6;                          /* IPv6 entry indicator. */
    int idx;                           /* Iteration index.      */
    soc_field_t hitf[] = {HIT_1f, HIT_2f, HIT_3f };

    buf_p = (uint32 *)l3x_entry;

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6) :\
                               BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4));

    /* Get table memory. */
    mem =  (ipv6) ? L3_ENTRY_IPV6_MULTICASTm : L3_ENTRY_IPV4_MULTICASTm;

    /* Mark entry as multicast & clear rest of the flags. */
    l3cfg->l3c_flags = BCM_L3_IPMC;
    if (ipv6) {
       l3cfg->l3c_flags |= BCM_L3_IP6;
    }

    /* Read hit value. */
    if(soc_mem_field32_get(unit, mem, buf_p, fld->hit)) { 
        l3cfg->l3c_flags |= BCM_L3_HIT;
    } else if (ipv6) {
        /* Get hit bit. */
        for (idx = 0; idx < 3; idx++) {
            if(soc_mem_field32_get(unit, mem, buf_p, hitf[idx])) { 
                l3cfg->l3c_flags |= BCM_L3_HIT;
                break;
            }
        }
    }

    /* Set ipv6 group address to multicast. */
    if (ipv6) {
        l3cfg->l3c_ip6[0] = 0xff;   /* Set entry ip to mcast address. */
    }

    /* Read priority override */
    if(soc_mem_field32_get(unit, mem, buf_p, fld->rpe)) { 
        l3cfg->l3c_flags |= BCM_L3_RPE;
    }

    /* Read destination discard bit. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->dst_discard)) {
        if(soc_mem_field32_get(unit, mem, buf_p, fld->dst_discard)) { 
            l3cfg->l3c_flags |= BCM_L3_DST_DISCARD;
        }
    }

    /* Read Virtual Router Id. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->vrf)) {
        l3cfg->l3c_vrf = soc_mem_field32_get(unit, mem, buf_p, fld->vrf);
    } else {
        l3cfg->l3c_vrf = BCM_L3_VRF_DEFAULT;
    }

    /*  */
    l3cfg->l3c_ipmc_ptr =
        soc_mem_field32_get(unit, mem, buf_p, fld->l3mc_index);

    /* Read priority value. */
    l3cfg->l3c_prio = soc_mem_field32_get(unit, mem, buf_p, fld->priority);

    return;
}

/*
 * Function:
 *      _bcm_fb_l3_ipmc_get
 * Purpose:
 *      Get l3 multicast entry.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3cfg     - (IN/OUT)Group/Source key & Get result buffer.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_fb_l3_ipmc_get(int unit, _bcm_l3_cfg_t *l3cfg)
{
    l3_entry_ipv6_multicast_entry_t l3x_key;    /* Lookup key buffer.    */
    l3_entry_ipv6_multicast_entry_t l3x_entry;  /* Search result buffer. */
    int clear_hit;                              /* Clear hit flag.       */
    soc_mem_t mem;                              /* IPMC table memory.    */
    int ipv6;                                   /* IPv6 entry indicator. */
    int rv;                                     /* Return value.         */

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /*  Zero buffers. */
    sal_memset(&l3x_key, 0, BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6));
    sal_memset(&l3x_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6));

    /* Get table memory. */
    mem =  (ipv6) ? L3_ENTRY_IPV6_MULTICASTm : L3_ENTRY_IPV4_MULTICASTm;

    /* Check if clear hit bit is required. */
    clear_hit = l3cfg->l3c_flags & BCM_L3_HIT_CLEAR;

    /* Lookup Key preparation. */
    _bcm_fb_l3_ipmc_ent_init(unit, (uint32 *)&l3x_key, l3cfg);

    /* Perform hw lookup. */
#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &l3cfg->l3c_hw_index,
                            &l3x_key, &l3x_entry, 0);
    } else
#endif /*  BCM_TRX_SUPPORT */
    {
        rv = soc_fb_l3x_lookup(unit, (void *)&l3x_key, (void *)&l3x_entry,
                               &l3cfg->l3c_hw_index);
    }
    BCM_XGS3_LKUP_IF_ERROR_RETURN(rv, BCM_E_NOT_FOUND);

    /* Extract buffer information. */
    _bcm_fb_l3_ipmc_ent_parse(unit, l3cfg, &l3x_entry);

    /* Clear the HIT bit */
    if (clear_hit) {
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_clear_hit(unit, mem,
                                                   l3cfg, &l3x_entry));
    }
    return rv;
}

/*
 * Function:
 *      _bcm_fb_l3_ipmc_add
 * Purpose:
 *      Add l3 multicast entry.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3cfg     - (IN/OUT)Group/Source key.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_fb_l3_ipmc_add(int unit, _bcm_l3_cfg_t *l3cfg)
{
    l3_entry_ipv6_multicast_entry_t l3x_entry;  /* Write entry buffer.   */
    _bcm_l3_fields_t *fld;                      /* IPMC table fields.    */   
    soc_mem_t mem;                              /* IPMC table memory.    */
    uint32 *buf_p;                              /* HW buffer address.    */
    int ipv6;                                   /* IPv6 entry indicator. */
    int idx;                                    /* Iteration index.      */
    int rv;                                     /* Return value.         */
    soc_field_t priorityf[] = { PRI_0f, PRI_1f, PRI_2f, PRI_3f };
    soc_field_t idxf[] = { L3MC_INDEX_0f, L3MC_INDEX_1f, \
                           L3MC_INDEX_2f, L3MC_INDEX_3f };
    soc_field_t hitf[] = { HIT_0f, HIT_1f, HIT_2f, HIT_3f };
    soc_field_t rpef[] = { RPE_0f, RPE_1f, RPE_2f, RPE_3f };
    soc_field_t dst_discf[] = { DST_DISCARD_0f, DST_DISCARD_1f,\
                               DST_DISCARD_2f, DST_DISCARD_3f };
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
    soc_field_t vrf_idf[] = { VRF_ID_0f, VRF_ID_1f, VRF_ID_2f, VRF_ID_3f};
#else /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    soc_field_t vrf_idf[] = { INVALIDf, INVALIDf, INVALIDf, INVALIDf};
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /*  Zero buffers. */
    buf_p = (uint32 *)&l3x_entry; 
    sal_memset(buf_p, 0, BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6));

    /* Get table memory. */
    mem =  (ipv6) ? L3_ENTRY_IPV6_MULTICASTm : L3_ENTRY_IPV4_MULTICASTm;

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6) :\
                               BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4));

    /* Overwrite field values if entry is ipv4 */
    if (!ipv6) {  
        priorityf[0] = fld->priority;
        idxf[0] = fld->l3mc_index; 
        hitf[0] = fld->hit;
        rpef[0] = fld->rpe;
        dst_discf[0] = fld->dst_discard;
        vrf_idf[0] = fld->vrf;
    }

    /* Prepare entry to write. */
    _bcm_fb_l3_ipmc_ent_init(unit, (uint32 *)&l3x_entry, l3cfg);

    for (idx = 0; idx < 4; idx++) {
        /* Set hit bit. */
        if (l3cfg->l3c_flags & BCM_L3_HIT) {
            soc_mem_field32_set(unit, mem, buf_p, hitf[idx], 1);
        }

        /* Set priority override bit. */
        if (l3cfg->l3c_flags & BCM_L3_RPE) {
            soc_mem_field32_set(unit, mem, buf_p, rpef[idx], 1);
        }

        /* Set destination discard. */
        if (SOC_MEM_FIELD_VALID(unit, mem, dst_discf[idx])) {
            if (l3cfg->l3c_flags & BCM_L3_DST_DISCARD) {
                soc_mem_field32_set(unit, mem, buf_p, dst_discf[idx], 1);
            }
        }

        /* Virtual router id. */
        if (SOC_MEM_FIELD_VALID(unit, mem, vrf_idf[idx])) {
            soc_mem_field32_set(unit, mem, buf_p, vrf_idf[idx], l3cfg->l3c_vrf);
        }

        /* Set priority. */
        soc_mem_field32_set(unit, mem, buf_p, priorityf[idx], l3cfg->l3c_prio);

        /* . */
        soc_mem_field32_set(unit, mem, buf_p, idxf[idx], l3cfg->l3c_ipmc_ptr);

        /* IPv4 entry needs only first field. */
        if (!ipv6) {
            break;
        }
    }

    /* Write entry to the hw. */
    /* Handle replacement. */
    if (BCM_XGS3_L3_INVALID_INDEX != l3cfg->l3c_hw_index) {
        rv = BCM_XGS3_MEM_WRITE(unit, mem, l3cfg->l3c_hw_index, buf_p);
    } else {
#ifdef BCM_TRX_SUPPORT
        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            rv = soc_mem_insert(unit, mem, MEM_BLOCK_ANY, (void *)buf_p);
        } else
#endif /*  BCM_TRX_SUPPORT */
        {
            rv = soc_fb_l3x_insert(unit, (void *)buf_p);
        }
    }

    /* Increment number of ipmc routes. */
    if ((rv >= 0) && (BCM_XGS3_L3_INVALID_INDEX == l3cfg->l3c_hw_index)) {
        (ipv6) ? BCM_XGS3_L3_IP6_IPMC_CNT(unit)++ : \
                 BCM_XGS3_L3_IP4_IPMC_CNT(unit)++;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_fb_l3_ipmc_del
 * Purpose:
 *      Delete l3 multicast entry.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3cfg     - (IN)Group/Source deletion key.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_fb_l3_ipmc_del(int unit, _bcm_l3_cfg_t *l3cfg)
{
    l3_entry_ipv6_multicast_entry_t l3x_entry;  /* Delete buffer.          */
    soc_mem_t mem;                              /* IPMC table memory.      */
    int ipv6;                                   /* IPv6 entry indicator.   */
    int rv;                                     /* Operation return value. */

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /*  Zero entry buffer. */
    sal_memset(&l3x_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6));

    /* Get table memory. */
    mem =  (ipv6) ? L3_ENTRY_IPV6_MULTICASTm : L3_ENTRY_IPV4_MULTICASTm;

    /* Key preparation. */
    _bcm_fb_l3_ipmc_ent_init(unit, (uint32 *)&l3x_entry, l3cfg);

    /* Delete the entry from hw. */
#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        rv = soc_mem_delete(unit, mem, MEM_BLOCK_ANY, (void *)&l3x_entry);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
       rv = soc_fb_l3x_delete(unit, (void *)&l3x_entry);
    }

    /* Decrement number of ipmc routes. */
    if (rv >= 0) {
        (ipv6) ? BCM_XGS3_L3_IP6_IPMC_CNT(unit)-- : \
                 BCM_XGS3_L3_IP4_IPMC_CNT(unit)--;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_fb_l3_ipmc_get_by_idx
 * Purpose:
 *      Get l3 multicast entry by entry index.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      dma_ptr  - (IN)Table pointer in dma. 
 *      idx       - (IN)Index to read. 
 *      l3cfg     - (IN/OUT)Entry data.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_fb_l3_ipmc_get_by_idx(int unit, void *dma_ptr,
                               int idx, _bcm_l3_cfg_t *l3cfg)
{
    l3_entry_ipv6_multicast_entry_t *l3x_entry_p; /* Read buffer address.    */
    l3_entry_ipv6_multicast_entry_t l3x_entry;    /* Read buffer.            */
    _bcm_l3_fields_t *fld;                        /* IPMC table fields.      */
    soc_mem_t mem;                                /* IPMC table memory.      */
    uint32 ipv6;                                  /* IPv6 entry indicator.   */
    int clear_hit;                                /* Clear hit bit indicator.*/

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Get table memory. */
    mem =  (ipv6) ? L3_ENTRY_IPV6_MULTICASTm : L3_ENTRY_IPV4_MULTICASTm;

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6) :\
                               BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4));

    /* Check if clear hit is required. */
    clear_hit = l3cfg->l3c_flags & BCM_L3_HIT_CLEAR;

    if (NULL == dma_ptr) {             /* Read from hardware. */
        /* Zero buffers. */
        l3x_entry_p = &l3x_entry;
        sal_memset(l3x_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6));
        /* Read entry from hw. */
        BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, l3x_entry_p));
    } else {                    /* Read from dma. */
        l3x_entry_p =
            soc_mem_table_idx_to_pointer(unit, mem,
                                         l3_entry_ipv6_multicast_entry_t *,
                                         dma_ptr, idx);
    }

    /* Ignore invalid entries. */
    if (!soc_mem_field32_get(unit, mem, l3x_entry_p, fld->valid)) {
        return (BCM_E_NOT_FOUND);
    }

    /* Get protocol. */
    if (soc_mem_field32_get(unit, mem, l3x_entry_p, fld->v6_entry)){
        l3cfg->l3c_flags = BCM_L3_IP6;
    } else {
        l3cfg->l3c_flags = 0;
    }

    /* Get ipmc flag . */
    if (soc_mem_field32_get(unit, mem, l3x_entry_p, fld->ipmc_entry)) {
        l3cfg->l3c_flags |= BCM_L3_IPMC;
    }

    /* Ignore protocol mismatch & multicast entries. */
    if ((ipv6  != (l3cfg->l3c_flags & BCM_L3_IP6)) ||
        (!(l3cfg->l3c_flags & BCM_L3_IPMC))) {
        return (BCM_E_NONE); 
    }

    /* Set index to l3cfg. */
    l3cfg->l3c_hw_index = idx;

    if (ipv6) {
        /* Get group address. */
        soc_mem_ip6_addr_get(unit, mem, l3x_entry_p, GROUP_IP_ADDR_LWR_64f,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_LOWER_ONLY);

        soc_mem_ip6_addr_get(unit, mem, l3x_entry_p, GROUP_IP_ADDR_UPR_56f,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_UPPER_ONLY);

        /* Get source  address. */
        soc_mem_ip6_addr_get(unit, mem, l3x_entry_p, SOURCE_IP_ADDR_LWR_64f, 
                             l3cfg->l3c_sip6, SOC_MEM_IP6_LOWER_ONLY);
        soc_mem_ip6_addr_get(unit, mem, l3x_entry_p, SOURCE_IP_ADDR_UPR_64f, 
                             l3cfg->l3c_sip6, SOC_MEM_IP6_UPPER_ONLY);

        l3cfg->l3c_ip6[0] = 0xff;    /* Set entry to multicast*/
    } else {
        /* Get group id. */
        l3cfg->l3c_ip_addr =
            soc_mem_field32_get(unit, mem, l3x_entry_p, GROUP_IP_ADDRf);

        /* Get source address. */
        l3cfg->l3c_src_ip_addr =
            soc_mem_field32_get(unit, mem,  l3x_entry_p, SOURCE_IP_ADDRf);
    }
    /* Get vlan id. */
    l3cfg->l3c_vid = soc_mem_field32_get(unit, mem,  l3x_entry_p, fld->vlan_id);

    /* Parse entry data. */
    _bcm_fb_l3_ipmc_ent_parse(unit, l3cfg, l3x_entry_p);

    /* Clear the HIT bit */
    if (clear_hit) {
        BCM_IF_ERROR_RETURN(_bcm_xgs3_l3_clear_hit(unit, mem,
                                                   l3cfg, l3x_entry_p));
    }
    return (BCM_E_NONE);
}

/*
 * Function:	
 *     _bcm_fb_mem_ip6_defip_set
 * Purpose:
 *    Set an IP6 address field in L3_DEFIPm
 * Parameters: 
 *    unit    - (IN) SOC unit number; 
 *    lpm_key - (OUT) Buffer to fill. 
 *    lpm_cfg - (IN) Route information. 
 * Note:
 *    See soc_mem_ip6_addr_set()
 */
STATIC void
_bcm_fb_mem_ip6_defip_set(int unit, void *lpm_key, _bcm_defip_cfg_t *lpm_cfg)
{
    uint8 *ip6;                 /* Ip6 address.       */
    uint8 mask[BCM_IP6_ADDRLEN];        /* Subnet mask.       */
    uint32 ip6_word;            /* Temp storage.      */
    int idx;                    /* Iteration index .  */

    /* Just to keep variable name short. */
    ip6 = lpm_cfg->defip_ip6_addr;

    /* Create mask from prefix length. */
    bcm_ip6_mask_create(mask, lpm_cfg->defip_sub_len);

    /* Apply subnet mask */
    idx = lpm_cfg->defip_sub_len / 8;   /* Unchanged byte count.    */
    ip6[idx] &= mask[idx];      /* Apply mask on next byte. */
    for (idx++; idx < BCM_IP6_ADDRLEN; idx++) {
        ip6[idx] = 0;           /* Reset rest of bytes.     */
    }


    ip6_word = ((ip6[0] << 24) | (ip6[1] << 16) | (ip6[2] << 8) | (ip6[3]));
    soc_L3_DEFIPm_field_set(unit, lpm_key, IP_ADDR1f, (void *)&ip6_word);

    ip6_word = ((ip6[4] << 24) | (ip6[5] << 16) | (ip6[6] << 8) | (ip6[7]));
    soc_L3_DEFIPm_field_set(unit, lpm_key, IP_ADDR0f, (void *)&ip6_word);

    ip6_word = ((mask[0] << 24) | (mask[1] << 16) | (mask[2] << 8) | (mask[3]));
    soc_L3_DEFIPm_field_set(unit, lpm_key, IP_ADDR_MASK1f,
                            (void *)&ip6_word);

    ip6_word = ((mask[4] << 24) | (mask[5] << 16) | (mask[6] << 8) | (mask[7]));
    soc_L3_DEFIPm_field_set(unit, lpm_key, IP_ADDR_MASK0f,
                            (void *)&ip6_word);
}

/*
 * Function:
 *     _bcm_fb_mem_ip6_defip_get
 * Purpose:
 *    Set an IP6 address field in L3_DEFIPm
 * Note:
 *    See soc_mem_ip6_addr_set()
 */
STATIC void
_bcm_fb_mem_ip6_defip_get(int unit, const void *lpm_key,
                          _bcm_defip_cfg_t *lpm_cfg)
{
    uint8 *ip6;                 /* Ip6 address.       */
    uint8 mask[BCM_IP6_ADDRLEN];        /* Subnet mask.       */
    uint32 ip6_word;            /* Temp storage.      */

    sal_memset(mask, 0, sizeof (bcm_ip6_t));

    /* Just to keep variable name short. */
    ip6 = lpm_cfg->defip_ip6_addr;
    sal_memset(ip6, 0, sizeof (bcm_ip6_t));

    soc_L3_DEFIPm_field_get(unit, lpm_key, IP_ADDR1f, &ip6_word);
    ip6[0] = (uint8) ((ip6_word >> 24) & 0xff);
    ip6[1] = (uint8) ((ip6_word >> 16) & 0xff);
    ip6[2] = (uint8) ((ip6_word >> 8) & 0xff);
    ip6[3] = (uint8) (ip6_word & 0xff);

    soc_L3_DEFIPm_field_get(unit, lpm_key, IP_ADDR0f, &ip6_word);
    ip6[4] = (uint8) ((ip6_word >> 24) & 0xff);
    ip6[5] = (uint8) ((ip6_word >> 16) & 0xff);
    ip6[6] = (uint8) ((ip6_word >> 8) & 0xff);
    ip6[7] = (uint8) (ip6_word & 0xff);

    soc_L3_DEFIPm_field_get(unit, lpm_key, IP_ADDR_MASK1f, &ip6_word);
    mask[0] = (uint8) ((ip6_word >> 24) & 0xff);
    mask[1] = (uint8) ((ip6_word >> 16) & 0xff);
    mask[2] = (uint8) ((ip6_word >> 8) & 0xff);
    mask[3] = (uint8) (ip6_word & 0xff);

    soc_L3_DEFIPm_field_get(unit, lpm_key, IP_ADDR_MASK0f, &ip6_word);
    mask[4] = (uint8) ((ip6_word >> 24) & 0xff);
    mask[5] = (uint8) ((ip6_word >> 16) & 0xff);
    mask[6] = (uint8) ((ip6_word >> 8) & 0xff);
    mask[7] = (uint8) (ip6_word & 0xff);

    lpm_cfg->defip_sub_len = bcm_ip6_mask_length(mask);
}

/*
 * Function:
 *      _bcm_fb_lpm_clear_hit
 * Purpose:
 *      Clear prefix entry hit bit.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      lpm_cfg   - (IN)Route info.  
 *      lpm_entry - (IN)Buffer to write to hw. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fb_lpm_clear_hit(int unit, _bcm_defip_cfg_t *lpm_cfg,
                      defip_entry_t *lpm_entry)
{
    int rv;                        /* Operation return status. */
    int tbl_idx;                   /* Defip table index.       */
    soc_field_t hit_field = HIT0f; /* HIT bit field id.        */

    /* Input parameters check */
    if ((NULL == lpm_cfg) || (NULL == lpm_entry)) {
        return (BCM_E_PARAM);
    }

    /* If entry was not hit  clear "clear hit" and return */
    if (!(lpm_cfg->defip_flags & BCM_L3_HIT)) {
        return (BCM_E_NONE);
    }

    /* Reset entry hit bit in buffer. */
    if (lpm_cfg->defip_flags & BCM_L3_IP6) {
        tbl_idx = lpm_cfg->defip_index;
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, HIT1f, 0);
    } else {
        tbl_idx = lpm_cfg->defip_index >> 1;
        rv = READ_L3_DEFIPm(unit, MEM_BLOCK_ALL, tbl_idx, lpm_entry);
        if (lpm_cfg->defip_index &= 0x1) {
            hit_field = HIT1f;
        }
    }
    soc_L3_DEFIPm_field32_set(unit, lpm_entry, hit_field, 0);

    /* Write lpm buffer to hardware. */
    rv = BCM_XGS3_MEM_WRITE(unit, L3_DEFIPm, tbl_idx, lpm_entry);
    return rv;
}

/*
 * Function:
 *      bcm_xgs3_internal_lpm_vrf_calc
 * Purpose:
 *      Service routine used to translate API vrf id to hw specific.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      lpm_cfg   - (IN)Prefix info.
 *      vrf_id    - (OUT)Internal vrf id.
 *      vrf_mask  - (OUT)Internal vrf mask.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_xgs3_internal_lpm_vrf_calc(int unit, _bcm_defip_cfg_t *lpm_cfg, 
                              int *vrf_id, int *vrf_mask)
{
    /* Handle special vrf id cases. */
    switch (lpm_cfg->defip_vrf) {
      case BCM_L3_VRF_OVERRIDE:
          *vrf_id = 0;
          *vrf_mask = 0;
          break;
      case BCM_L3_VRF_GLOBAL:
          *vrf_id = SOC_VRF_MAX(unit);
          *vrf_mask = 0;
          break;
      default:   
          *vrf_id = lpm_cfg->defip_vrf;
          *vrf_mask = SOC_VRF_MAX(unit);
    }

    /* In any case vrf id shouldn't exceed max field mask. */
    if ((*vrf_id < 0) || (*vrf_id > SOC_VRF_MAX(unit))) {
        return (BCM_E_PARAM);
    } 
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_fb_lpm_ent_init
 * Purpose:
 *      Service routine used to initialize lkup key for lpm entry.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      lpm_cfg   - (IN)Prefix info.
 *      lpm_entry - (OUT)Hw buffer to fill.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fb_lpm_ent_init(int unit, _bcm_defip_cfg_t *lpm_cfg,
                     defip_entry_t *lpm_entry)
{
    bcm_ip_t ip4_mask;
    int vrf_id;
    int vrf_mask;
    int ipv6 = lpm_cfg->defip_flags & BCM_L3_IP6;


    /* Extract entry  vrf id  & vrf mask. */
    /*for Hurricane it should return default value
    for vrf id*/	
    BCM_IF_ERROR_RETURN
        (bcm_xgs3_internal_lpm_vrf_calc(unit, lpm_cfg, &vrf_id, &vrf_mask));

    /* Set prefix ip address & mask. */
    if (ipv6) {
        _bcm_fb_mem_ip6_defip_set(unit, lpm_entry, lpm_cfg);
    } else {
        ip4_mask = BCM_IP4_MASKLEN_TO_ADDR(lpm_cfg->defip_sub_len);
        /* Apply subnet mask. */
        lpm_cfg->defip_ip_addr &= ip4_mask;

        /* Set address to the buffer. */
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, IP_ADDR0f,
                                  lpm_cfg->defip_ip_addr);
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, IP_ADDR_MASK0f, ip4_mask);
    }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
    /* Set Virtual Router id if supported. */
    if (!SOC_IS_HURRICANE(unit) && SOC_MEM_FIELD_VALID(unit, BCM_XGS3_L3_MEM(unit, defip), VRF_ID_0f)) {
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, VRF_ID_0f, vrf_id);
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, VRF_ID_MASK0f, vrf_mask);
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    /* Set valid bit. */
    soc_L3_DEFIPm_field32_set(unit, lpm_entry, VALID0f, 1);

    if (ipv6) {
        /* Set second part valid bit. */
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, VALID1f, 1);

        /* Set mode to ipv6 */
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, MODE0f, 1);
        soc_L3_DEFIPm_field32_set(unit, lpm_entry, MODE1f, 1);

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        /* Set Virtual Router id if supported. */
        if (SOC_MEM_FIELD_VALID(unit, BCM_XGS3_L3_MEM(unit, defip), VRF_ID_1f)){
            soc_L3_DEFIPm_field32_set(unit, lpm_entry, VRF_ID_1f, vrf_id);
            soc_L3_DEFIPm_field32_set(unit, lpm_entry, VRF_ID_MASK1f, vrf_mask);
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    }
#if defined(BCM_TRX_SUPPORT) || defined(BCM_RAVEN_SUPPORT)
    if (soc_mem_field_valid(unit, BCM_XGS3_L3_MEM(unit, defip), MODE_MASK0f)) {
        soc_mem_field32_set(unit, BCM_XGS3_L3_MEM(unit, defip), lpm_entry, MODE_MASK0f, 
            (1 << soc_mem_field_length(unit, BCM_XGS3_L3_MEM(unit, defip), MODE_MASK0f)) - 1);
    }
    if (soc_mem_field_valid(unit, BCM_XGS3_L3_MEM(unit, defip), MODE_MASK1f)) {
        soc_mem_field32_set(unit, BCM_XGS3_L3_MEM(unit, defip), lpm_entry, MODE_MASK1f, 
            (1 << soc_mem_field_length(unit, BCM_XGS3_L3_MEM(unit, defip), MODE_MASK1f)) - 1);
    }
#endif /* BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
#if defined(BCM_RAVEN_SUPPORT)
    if (soc_mem_field_valid(unit, BCM_XGS3_L3_MEM(unit, defip), RESERVED_MASK0f)) {
        soc_mem_field32_set(unit, BCM_XGS3_L3_MEM(unit, defip), 
                            lpm_entry, RESERVED_MASK0f, 0);
    }
    if (soc_mem_field_valid(unit, BCM_XGS3_L3_MEM(unit, defip), RESERVED_KEY0f)) {
        soc_mem_field32_set(unit, BCM_XGS3_L3_MEM(unit, defip), lpm_entry, RESERVED_KEY0f, 0);
    }
    if (soc_mem_field_valid(unit, BCM_XGS3_L3_MEM(unit, defip), RESERVED_MASK1f)) {
        soc_mem_field32_set(unit, BCM_XGS3_L3_MEM(unit, defip), 
                            lpm_entry, RESERVED_MASK1f, 0);
    }
    if (soc_mem_field_valid(unit, BCM_XGS3_L3_MEM(unit, defip), RESERVED_KEY1f)) {
        soc_mem_field32_set(unit, BCM_XGS3_L3_MEM(unit, defip), lpm_entry, RESERVED_KEY1f, 0);
    }
#endif /* BCM_RAVEN_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_fb_lpm_ent_get_key
 * Purpose:
 *      Parse entry key from DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (OUT)Buffer to fill defip information. 
 *      lpm_entry   - (IN) Buffer read from hw. 
 * Returns:
 *      void
 */
STATIC void
_bcm_fb_lpm_ent_get_key(int unit, _bcm_defip_cfg_t *lpm_cfg,
                        defip_entry_t *lpm_entry)
{
    bcm_ip_t v4_mask;
    int ipv6 = lpm_cfg->defip_flags & BCM_L3_IP6;

    /* Set prefix ip address & mask. */
    if (ipv6) {
        _bcm_fb_mem_ip6_defip_get(unit, lpm_entry, lpm_cfg);
    } else {
        /* Get ipv4 address. */
        lpm_cfg->defip_ip_addr =
            soc_L3_DEFIPm_field32_get(unit, lpm_entry, IP_ADDR0f);

        /* Get subnet mask. */
        v4_mask = soc_L3_DEFIPm_field32_get(unit, lpm_entry, IP_ADDR_MASK0f);

        /* Fill mask length. */
        lpm_cfg->defip_sub_len = bcm_ip_mask_length(v4_mask);
    }

    /* Get Virtual Router id */
    soc_fb_lpm_vrf_get(unit, lpm_entry, &lpm_cfg->defip_vrf);

    return;
}

/*
 * Function:
 *      _bcm_fb_lpm_ent_parse
 * Purpose:
 *      Parse an entry from DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (OUT)Buffer to fill defip information. 
 *      nh_ecmp_idx - (OUT)Next hop index or ecmp group id.  
 *      lpm_entry   - (IN) Buffer read from hw. 
 * Returns:
 *      void
 */
STATIC INLINE void
_bcm_fb_lpm_ent_parse(int unit, _bcm_defip_cfg_t *lpm_cfg, int *nh_ecmp_idx,
                      defip_entry_t *lpm_entry)
{
    int ipv6 = soc_L3_DEFIPm_field32_get(unit, lpm_entry, MODE0f);

    /* Reset entry flags first. */
    lpm_cfg->defip_flags = 0;

    /* Check if entry points to ecmp group. */
   /*hurricane does not have ecmp flag*/	
    if ((!(SOC_IS_HURRICANE(unit))) &&
	(soc_L3_DEFIPm_field32_get(unit, lpm_entry, ECMP0f))) {
        /* Mark entry as ecmp */
        lpm_cfg->defip_ecmp = 1;
        lpm_cfg->defip_flags |= BCM_L3_MULTIPATH;

        /* Get ecmp group id. */
        if (nh_ecmp_idx) {
            *nh_ecmp_idx =
                soc_L3_DEFIPm_field32_get(unit, lpm_entry, ECMP_PTR0f);
        }
    } else {
        /* Mark entry as non-ecmp. */
        lpm_cfg->defip_ecmp = 0;

        /* Reset ecmp group next hop count. */
        lpm_cfg->defip_ecmp_count = 0;

        /* Get next hop index. */
        if (nh_ecmp_idx) {
            *nh_ecmp_idx =
                soc_L3_DEFIPm_field32_get(unit, lpm_entry, NEXT_HOP_INDEX0f);
        }
    }
    /* Get entry priority. */
    lpm_cfg->defip_prio = soc_L3_DEFIPm_field32_get(unit, lpm_entry, PRI0f);

    /* Get hit bit. */
    if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, HIT0f)) {
        lpm_cfg->defip_flags |= BCM_L3_HIT;
    }

    /* Get priority override bit. */
    if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, RPE0f)) {
        lpm_cfg->defip_flags |= BCM_L3_RPE;
    }

    /* Get destination discard flag. */
    if (SOC_MEM_FIELD_VALID(unit, L3_DEFIPm, DST_DISCARD0f)) {
        if(soc_L3_DEFIPm_field32_get(unit, lpm_entry, DST_DISCARD0f)) {
            lpm_cfg->defip_flags |= BCM_L3_DST_DISCARD;
        }
    }

#if defined(BCM_TRX_SUPPORT)
    /* Set classification group id. */
    if (SOC_MEM_FIELD_VALID(unit, L3_DEFIPm, CLASS_ID0f)) {
        lpm_cfg->defip_lookup_class = 
            soc_L3_DEFIPm_field32_get(unit, lpm_entry, CLASS_ID0f);
    }
#endif /* BCM_TRX_SUPPORT */


    if (ipv6) {
        lpm_cfg->defip_flags |= BCM_L3_IP6;
        /* Get hit bit from the second part of the entry. */
        if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, HIT1f)) {
            lpm_cfg->defip_flags |= BCM_L3_HIT;
        }

        /* Get priority override bit from the second part of the entry. */
        if (soc_L3_DEFIPm_field32_get(unit, lpm_entry, RPE1f)) {
            lpm_cfg->defip_flags |= BCM_L3_RPE;
        }
    }
    return;
}

/*
 * Function:
 *      _bcm_fb_lpm_get
 * Purpose:
 *      Get an entry from DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fb_lpm_get(int unit, _bcm_defip_cfg_t *lpm_cfg, int *nh_ecmp_idx)
{
    defip_entry_t lpm_key;      /* Route lookup key.        */
    defip_entry_t lpm_entry;    /* Search result buffer.    */
    int clear_hit;              /* Clear hit indicator.     */
    int rv;                     /* Operation return status. */

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Zero buffers. */
    sal_memset(&lpm_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));
    sal_memset(&lpm_key, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));

    /* Check if clear hit bit required. */
    clear_hit = lpm_cfg->defip_flags & BCM_L3_HIT_CLEAR;

    /* Initialize lkup key. */
    BCM_IF_ERROR_RETURN(_bcm_fb_lpm_ent_init(unit, lpm_cfg, &lpm_key));

    /* Perform hw lookup. */
    rv = soc_fb_lpm_match(unit, &lpm_key, &lpm_entry, &lpm_cfg->defip_index);
    BCM_IF_ERROR_RETURN(rv);

    /*
     * If entry is ipv4 we always operate on entry in "zero" half of the, 
     * buffer "zero" half of lpm_entry if the  original entry
     * is in the "one" half.
     */
    if ((!(lpm_cfg->defip_flags & BCM_L3_IP6)) && (lpm_cfg->defip_index & 0x1)) {
        soc_fb_lpm_ip4entry1_to_0(unit, &lpm_entry, &lpm_entry, TRUE);
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TRIDENT(unit)) {
        defip_hit_only_y_entry_t hit_entry_y;
        uint32 hit;
        if (lpm_cfg->defip_flags & BCM_L3_IP6) {
            BCM_IF_ERROR_RETURN
                (BCM_XGS3_MEM_READ(unit, L3_DEFIP_HIT_ONLY_Ym,
                                   lpm_cfg->defip_index, &hit_entry_y));
            hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym,
                                      &hit_entry_y, HIT0f);
            hit |= soc_mem_field32_get(unit, L3_DEFIPm, &lpm_entry, HIT0f);
            soc_mem_field32_set(unit, L3_DEFIPm, &lpm_entry, HIT0f, hit);
            hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym,
                                      &hit_entry_y, HIT1f);
            hit |= soc_mem_field32_get(unit, L3_DEFIPm, &lpm_entry, HIT1f);
            soc_mem_field32_set(unit, L3_DEFIPm, &lpm_entry, HIT1f, hit);
        } else {
            BCM_IF_ERROR_RETURN
                (BCM_XGS3_MEM_READ(unit, L3_DEFIP_HIT_ONLY_Ym,
                                   lpm_cfg->defip_index >> 1, &hit_entry_y));
            hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym, &hit_entry_y,
                                      lpm_cfg->defip_index & 0x1 ?
                                      HIT1f : HIT0f);
            hit |= soc_mem_field32_get(unit, L3_DEFIPm, &lpm_entry, HIT0f);
            soc_mem_field32_set(unit, L3_DEFIPm, &lpm_entry, HIT0f, hit);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* Parse hw buffer to defip entry. */
    _bcm_fb_lpm_ent_parse(unit, lpm_cfg, nh_ecmp_idx, &lpm_entry);

    /* Clear the HIT bit */
    if (clear_hit) {
        BCM_IF_ERROR_RETURN(_bcm_fb_lpm_clear_hit(unit, lpm_cfg, &lpm_entry));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_fb_lpm_add
 * Purpose:
 *      Add an entry to DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fb_lpm_add(int unit, _bcm_defip_cfg_t *lpm_cfg, int nh_ecmp_idx)
{
    defip_entry_t lpm_entry;    /* Search result buffer.    */
    int rv;                     /* Operation return status. */

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Zero buffers. */
    sal_memset(&lpm_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));

    /* Set hit bit. */
    if (lpm_cfg->defip_flags & BCM_L3_HIT) {
        soc_L3_DEFIPm_field32_set(unit, &lpm_entry, HIT0f, 1);
    }

    /* Set priority override bit. */
    if (lpm_cfg->defip_flags & BCM_L3_RPE) {
        soc_L3_DEFIPm_field32_set(unit, &lpm_entry, RPE0f, 1);
    }

    /* Write priority field. */
    soc_L3_DEFIPm_field32_set(unit, &lpm_entry, PRI0f, lpm_cfg->defip_prio);

    /* Fill next hop information. */
    if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
        soc_L3_DEFIPm_field32_set(unit, &lpm_entry, ECMP0f, 1);
        /* NOTE: Order is important set next hop index before ECMP_COUNT.
         * If device doesn't have ECMP_COUNT:  ECMP_PTR field is shorter than 
         * next hop -> we must ensure that write resets high bits of next hop
         * field.
         */
        soc_L3_DEFIPm_field32_set(unit, &lpm_entry, NEXT_HOP_INDEX0f, nh_ecmp_idx);
        if (SOC_MEM_FIELD_VALID(unit, L3_DEFIPm, ECMP_COUNT0f)) {
            soc_L3_DEFIPm_field32_set(unit, &lpm_entry, ECMP_COUNT0f,
                                      lpm_cfg->defip_ecmp_count);
        }
    } else {
        soc_L3_DEFIPm_field32_set(unit, &lpm_entry, NEXT_HOP_INDEX0f,
                                  nh_ecmp_idx);
    }

    /* Set destination discard flag. */
    if (lpm_cfg->defip_flags & BCM_L3_DST_DISCARD) {
        if (SOC_MEM_FIELD_VALID(unit, L3_DEFIPm, DST_DISCARD0f)) {
            soc_L3_DEFIPm_field32_set(unit, &lpm_entry, DST_DISCARD0f, 1);
        } else {
            return (BCM_E_UNAVAIL);
        }
    }

#if defined(BCM_TRX_SUPPORT)
    /* Set classification group id. */
    if (SOC_MEM_FIELD_VALID(unit, L3_DEFIPm, CLASS_ID0f)) {
        soc_L3_DEFIPm_field32_set(unit, &lpm_entry, CLASS_ID0f, 
                                  lpm_cfg->defip_lookup_class);
    }

    /* Set Global route flag. */
    if (SOC_MEM_FIELD_VALID(unit, L3_DEFIPm, GLOBAL_ROUTE0f)) {
        if (BCM_L3_VRF_GLOBAL == lpm_cfg->defip_vrf) {
            soc_mem_field32_set(unit, L3_DEFIPm, &lpm_entry, 
                                GLOBAL_ROUTE0f, 0x1);
        }
    }
#endif /* BCM_TRX_SUPPORT */

    /* 
     * Note ipv4 entries are expected to reside in part 0 of the entry.
     *      ipv6 entries both parts should be filled.  
     *      Hence if entry is ipv6 copy part 0 to part 1
     */
    if (lpm_cfg->defip_flags & BCM_L3_IP6) {
        soc_fb_lpm_ip4entry0_to_1(unit, &lpm_entry, &lpm_entry, TRUE);
    }

    /* 
     * Initialize hw buffer from lpm configuration. 
     * NOTE: DON'T MOVE _bcm_fb_defip_ent_init CALL UP, 
     * We want to copy flags & nh info, avoid  ipv6 mask & address corruption. 
     */
    BCM_IF_ERROR_RETURN(_bcm_fb_lpm_ent_init(unit, lpm_cfg, &lpm_entry));

    /* Write buffer to hw. */
    rv = soc_fb_lpm_insert(unit, &lpm_entry);

    /* If new route added increment total number of routes.  */
    /* Lack of index indicates a new route. */
    if ((rv >= 0) && (BCM_XGS3_L3_INVALID_INDEX == lpm_cfg->defip_index)) {
        BCM_XGS3_L3_DEFIP_CNT_INC(unit, (lpm_cfg->defip_flags & BCM_L3_IP6));
    }
    return rv;
}

/*
 * Function:
 *      _bcm_fb_lpm_del
 * Purpose:
 *      Delete an entry from DEFIP table.
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Buffer to fill defip information. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fb_lpm_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    int rv;                     /* Operation return status. */
    defip_entry_t lpm_entry;    /* Search result buffer.    */

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Zero buffers. */
    sal_memset(&lpm_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));

    /* Initialize hw buffer deletion key. */
    BCM_IF_ERROR_RETURN(_bcm_fb_lpm_ent_init(unit, lpm_cfg, &lpm_entry));

    /* Write buffer to hw. */
    rv = soc_fb_lpm_delete(unit, &lpm_entry);

    /* If new route added increment total number of routes.  */
    if (rv >= 0) {
        BCM_XGS3_L3_DEFIP_CNT_DEC(unit, lpm_cfg->defip_flags & BCM_L3_IP6);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_fb_lpm_update_match
 * Purpose:
 *      Update/Delete all entries in defip table matching a certain rule.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      trv_data - (IN)Delete pattern + compare,act,notify routines.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fb_lpm_update_match(int unit, _bcm_l3_trvrs_data_t *trv_data)
{
    uint32 ipv6;                /* Iterate over ipv6 only flag. */
    int idx;                    /* Iteration index.             */
    int tmp_idx;                /* ipv4 entries iterator.       */
    char *lpm_tbl_ptr;          /* Dma table pointer.           */
    int nh_ecmp_idx;            /* Next hop/Ecmp group index.   */
    int cmp_result;             /* Test routine result.         */
    defip_entry_t *lpm_entry;   /* Hw entry buffer.             */
    _bcm_defip_cfg_t lpm_cfg;   /* Buffer to fill route info.   */
    int defip_table_size;       /* Defip table size.            */
    int rv;                     /* Operation return status.     */
    int idx_start = 0;
    int idx_end = 0;
#if defined(BCM_KATANA_SUPPORT)
    extern uint32 kt_depth64;
    extern uint32 kt_dip64_start_offset;
#endif

    ipv6 = (trv_data->flags & BCM_L3_IP6);

    /* Table DMA the LPM table to software copy */
    BCM_IF_ERROR_RETURN
        (bcm_xgs3_l3_tbl_dma(unit, BCM_XGS3_L3_MEM(unit, defip),
                             BCM_XGS3_L3_ENT_SZ(unit, defip), "lpm_tbl",
                             &lpm_tbl_ptr, &defip_table_size));

    if (SOC_URPF_STATUS_GET(unit)) {
        if (soc_feature(unit, soc_feature_l3_defip_hole)) {
              defip_table_size = SOC_APOLLO_B0_L3_DEFIP_URPF_SIZE;
        } else if (SOC_IS_APOLLO(unit)) {
            defip_table_size = SOC_APOLLO_L3_DEFIP_URPF_SIZE;
        } else {
            defip_table_size >>= 1;
        }
    }

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        idx_start = kt_dip64_start_offset;
        idx_end = idx_start + kt_depth64;
    } else
#endif
    {
        idx_start = 0;
        idx_end = defip_table_size;
    }

    for (idx = idx_start; idx < idx_end; idx++) {
        /* Calculate entry ofset. */
        lpm_entry =
            soc_mem_table_idx_to_pointer(unit, BCM_XGS3_L3_MEM(unit, defip),
                                         defip_entry_t *, lpm_tbl_ptr, idx);

        /* Each lpm entry contains 2 ipv4 entries ->  Check both. */
        for (tmp_idx = 0; tmp_idx < ((ipv6) ? 1 : 2); tmp_idx++) {

            if (tmp_idx) {
                /* Check second part of the entry. */
                soc_fb_lpm_ip4entry1_to_0(unit, lpm_entry, lpm_entry, TRUE);
            }

            /* Make sure entry is valid. */
            if (!soc_L3_DEFIPm_field32_get(unit, lpm_entry, VALID0f)) {
                continue;
            }

#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TRIDENT(unit)) {
                defip_hit_only_y_entry_t hit_entry_y;
                uint32 hit;
                if (ipv6) {
                    BCM_IF_ERROR_RETURN
                        (BCM_XGS3_MEM_READ(unit, L3_DEFIP_HIT_ONLY_Ym, idx,
                                           &hit_entry_y));
                    hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym,
                                              &hit_entry_y, HIT0f);
                    hit |= soc_mem_field32_get(unit, L3_DEFIPm, lpm_entry,
                                               HIT0f);
                    soc_mem_field32_set(unit, L3_DEFIPm, lpm_entry, HIT0f,
                                        hit);
                    hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym,
                                              &hit_entry_y, HIT1f);
                    hit |= soc_mem_field32_get(unit, L3_DEFIPm, lpm_entry,
                                               HIT1f);
                    soc_mem_field32_set(unit, L3_DEFIPm, lpm_entry, HIT1f,
                                        hit);
                } else {
                    BCM_IF_ERROR_RETURN
                        (BCM_XGS3_MEM_READ(unit, L3_DEFIP_HIT_ONLY_Ym,
                                           idx >> 1, &hit_entry_y));
                    hit = soc_mem_field32_get(unit, L3_DEFIP_HIT_ONLY_Ym,
                                              &hit_entry_y,
                                              tmp_idx ? HIT1f : HIT0f);
                    hit |= soc_mem_field32_get(unit, L3_DEFIPm, lpm_entry,
                                               HIT0f);
                    soc_mem_field32_set(unit, L3_DEFIPm, lpm_entry, HIT0f,
                                        hit);
                }
            }
#endif /* BCM_TRIDENT_SUPPORT */

            /* Zero destination buffer first. */
            sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));

            /* Parse  the entry. */
            _bcm_fb_lpm_ent_parse(unit, &lpm_cfg, &nh_ecmp_idx, lpm_entry);

            /* If protocol doesn't match skip the entry. */
            if ((lpm_cfg.defip_flags & BCM_L3_IP6) != ipv6) {
                continue;
            }

            /* Fill entry ip address &  subnet mask. */
            _bcm_fb_lpm_ent_get_key(unit, &lpm_cfg, lpm_entry);

            /* Execute operation routine if any. */
            if (trv_data->op_cb) {
                rv = (*trv_data->op_cb) (unit, (void *)trv_data,
                                         (void *)&lpm_cfg,
                                         (void *)&nh_ecmp_idx, &cmp_result);
                if (rv < 0) {
                    soc_cm_sfree(unit, lpm_tbl_ptr);
                    return rv;
                }
            }
#ifdef BCM_WARM_BOOT_SUPPORT
            if (SOC_WARM_BOOT(unit) && (!tmp_idx)) {
                rv = soc_fb_lpm_reinit(unit, idx, lpm_entry);
                if (rv < 0) {
                    soc_cm_sfree(unit, lpm_tbl_ptr);
                    return rv;
                }
            }
#endif /* BCM_WARM_BOOT_SUPPORT */
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_IF_ERROR_RETURN(soc_fb_lpm_reinit_done(unit));
#endif /* BCM_WARM_BOOT_SUPPORT */
    soc_cm_sfree(unit, lpm_tbl_ptr);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_fbx_lpm_get
 * Purpose:
 *      Get an entry from DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fbx_lpm_get(int unit, _bcm_defip_cfg_t *lpm_cfg, int *nh_ecmp_idx)
{
    soc_mem_t mem = L3_DEFIPm;

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_trx_l3_defip_mem_get(unit, lpm_cfg->defip_flags,
                                      lpm_cfg->defip_sub_len, &mem));
    }
#endif /* BCM_TRX_SUPPORT */


    switch (mem) {
#if defined(BCM_TRIUMPH_SUPPORT)
        case EXT_IPV6_128_DEFIPm:
        case EXT_IPV6_64_DEFIPm:
        case EXT_IPV4_DEFIPm:
            return _bcm_tr_ext_lpm_match(unit, lpm_cfg, nh_ecmp_idx);
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRX_SUPPORT) 
        case L3_DEFIP_128m:
#if defined(BCM_KATANA_SUPPORT)
            if (SOC_IS_KATANA(unit) && (kt_depth128 > 0)) {
                return _bcm_kt_defip_pair128_get(unit, lpm_cfg, nh_ecmp_idx);
            } else
#endif
            {
                return _bcm_trx_defip_128_get(unit, lpm_cfg, nh_ecmp_idx);
            }
#endif /* BCM_TRX_SUPPORT */
      default: 
            return _bcm_fb_lpm_get(unit, lpm_cfg, nh_ecmp_idx);
    }
    return (BCM_E_INTERNAL); /* Never reached. */
}

/*
 * Function:
 *      _bcm_fbx_lpm_add
 * Purpose:
 *      Add an entry to DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fbx_lpm_add(int unit, _bcm_defip_cfg_t *lpm_cfg, int nh_ecmp_idx)
{
    soc_mem_t mem = L3_DEFIPm;

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_trx_l3_defip_mem_get(unit, lpm_cfg->defip_flags,
                                      lpm_cfg->defip_sub_len, &mem));
    }
#endif /* BCM_TRX_SUPPORT */

    switch (mem) {
#if defined(BCM_TRIUMPH_SUPPORT)
        case EXT_IPV6_128_DEFIPm:
        case EXT_IPV6_64_DEFIPm:
        case EXT_IPV4_DEFIPm:
            return _bcm_tr_ext_lpm_add(unit, lpm_cfg, nh_ecmp_idx);
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
        case L3_DEFIP_128m:
#if defined(BCM_KATANA_SUPPORT)
            if (SOC_IS_KATANA(unit) && (kt_depth128 > 0)) {
                return _bcm_kt_defip_pair128_add(unit, lpm_cfg, nh_ecmp_idx);
            } else
#endif
            {
                return _bcm_trx_defip_128_add(unit, lpm_cfg, nh_ecmp_idx);
            }
#endif /* BCM_TRX_SUPPORT */
        default: 
            return _bcm_fb_lpm_add(unit, lpm_cfg, nh_ecmp_idx);
    }
    return (BCM_E_INTERNAL); /* Never reached. */
}

/*
 * Function:
 *      _bcm_fbx_lpm_del
 * Purpose:
 *      Delete an entry from DEFIP table.
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Buffer to fill defip information. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fbx_lpm_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    soc_mem_t mem = L3_DEFIPm;

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_trx_l3_defip_mem_get(unit, lpm_cfg->defip_flags,
                                      lpm_cfg->defip_sub_len, &mem));
    }
#endif /* BCM_TRX_SUPPORT */

    switch (mem) {
#if defined(BCM_TRIUMPH_SUPPORT)
        case EXT_IPV6_128_DEFIPm:
        case EXT_IPV6_64_DEFIPm:
        case EXT_IPV4_DEFIPm:
            return _bcm_tr_ext_lpm_delete(unit, lpm_cfg);
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
        case L3_DEFIP_128m:
#if defined(BCM_KATANA_SUPPORT)
            if (SOC_IS_KATANA(unit) && (kt_depth128 > 0)) {
                return _bcm_kt_defip_pair128_delete(unit, lpm_cfg);
            } else
#endif
            {
                return _bcm_trx_defip_128_delete(unit, lpm_cfg);
            }
#endif /* BCM_TRX_SUPPORT */
      default: 
            return _bcm_fb_lpm_del(unit, lpm_cfg);
    }
    return (BCM_E_INTERNAL); /* Never reached. */
}

/*
 * Function:
 *      _bcm_fbx_lpm_update_match
 * Purpose:
 *      Update/Delete all entries in defip table matching a certain rule.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      trv_data - (IN)Delete pattern + compare,act,notify routines.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_fbx_lpm_update_match(int unit, _bcm_l3_trvrs_data_t *trv_data)
{
    soc_mem_t mem = L3_DEFIPm;  /* Route table memory.      */
    int rv = BCM_E_NONE;        /* Operation return status. */

#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TR_VL(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_tr_l3_defip_mem_get(unit, trv_data->flags, 0, &mem));
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    switch (mem) {
#if defined(BCM_TRIUMPH_SUPPORT)
      case EXT_IPV6_128_DEFIPm:
      case EXT_IPV6_64_DEFIPm:
      case EXT_IPV4_DEFIPm:
          return  _bcm_tr_defip_traverse(unit, trv_data);
#endif /* BCM_TRIUMPH_SUPPORT */
      default: 
          rv = _bcm_fb_lpm_update_match(unit, trv_data);
    }
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit) && (kt_depth128 > 0)) {
        if ((trv_data->flags & BCM_L3_IP6)) {
            rv = _bcm_trx_defip_128_update_match(unit, trv_data);
            rv = _bcm_kt_defip_pair128_update_match(unit, trv_data);
        }
    } else
#endif /* BCM_KATANA_SUPPORT */
    {
#if defined(BCM_TRX_SUPPORT)
        if ((trv_data->flags & BCM_L3_IP6) && 
            SOC_MEM_IS_VALID(unit, L3_DEFIP_128m)) {
            rv = _bcm_trx_defip_128_update_match(unit, trv_data);
        }
#endif /* BCM_TRX_SUPPORT */
    }
    return (rv);
}


#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
/* Forward declaration for rule shift. */
STATIC int _bcm_rp_lpm_entry_remove(int unit, int rule_idx);

/*
 * Function:
 *      _bcm_rp_grp_priority_cmp
 * Purpose:
 *      Compare two FP groups priorities.
 * Parameters:
 *      b - (IN)first compared group. 
 *      a - (IN)second compared group. 
 * Returns:
 *      a<=>b
 */
STATIC INLINE int
_bcm_rp_grp_priority_cmp(void *a, void *b)
{
    _bcm_rp_l3_fp_group_t first;       /* First compared group.  */
    _bcm_rp_l3_fp_group_t second;      /* Second compared group. */

    /* Cast group info pointers. */
    first = *(_bcm_rp_l3_fp_group_t *)a;
    second = *(_bcm_rp_l3_fp_group_t *)b;

    /* Compare groups priorities. */
    return _bcm_xgs3_cmp_int(&first.grp_hdlr, &second.grp_hdlr);
}

/*
 * Function:
 *      _bcm_rp_l3_rule_create
 * Purpose:
 *      Create L3 field processor rule & meter.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      rule_idx - (IN)Rule index.   
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_rule_create(int unit, int rule_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;      /* L3 sw table address. */
    bcm_field_entry_t rule_hdlr;     /* FP l3 rule handler.  */
    bcm_field_group_t grp_hdlr;      /* FP l3 group handler. */
    int grp_idx;                     /* FP l3 group index.   */ 

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);   

    grp_idx = rule_idx/(tbl_ptr->rules_per_group);
    grp_hdlr = (tbl_ptr->l3_grp[grp_idx]).grp_hdlr;

    /* Create FP rule. */
    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_create(unit, grp_hdlr, &rule_hdlr));

    /* Create a counter.
     * NOTE: Counters are shared between odd & even entries. 
     *       We assume even entry is created befor odd one.          
     */
    if (BCM_XGS3_L3_EVEN_IDX(rule_idx) ||
        (0 == (rule_idx % (tbl_ptr->rules_per_group)))) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_counter_create(unit, rule_hdlr));
    } else {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_counter_share(unit, 
                                         (tbl_ptr->l3_arr[rule_idx - 1]).rule_hdlr,
                                         rule_hdlr));
    }

    /* Preserve rule handler. */
    (tbl_ptr->l3_arr[rule_idx]).rule_hdlr = rule_hdlr;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_rp_l3_rule_destroy
 * Purpose:
 *      Destroy L3 field processor rule.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      rule_idx - (IN)Rule index.   
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_rule_destroy(int unit, int rule_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;      /* L3 sw table address.     */
    bcm_field_entry_t rule_hdlr;     /* FP l3 rule handler. */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);   

    /* Get Rule Handler. */ 
    rule_hdlr = (tbl_ptr->l3_arr[rule_idx]).rule_hdlr;

    /* If entry is valid -> remove it from HW. */
    if (tbl_ptr->l3_arr[rule_idx].flags & BCM_XGS3_L3_ENT_VALID) {
        /* Uinstall fp entry from hw. */
        BCM_IF_ERROR_RETURN(bcm_esw_field_entry_remove(unit, rule_hdlr));
    }

    /* Delete match counter. */
    if ((BCM_XGS3_L3_EVEN_IDX(rule_idx)) ||
        (0 == (rule_idx % (tbl_ptr->rules_per_group)))) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_counter_destroy(unit, rule_hdlr));
    }

    /* Uinstall fp entry from sw. */
    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_destroy(unit, rule_hdlr));

    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_rp_l3_usage_cntr_set
 * Purpose:
 *      Copy usage counter from one l3 entry rule to another.
 *      or reset usage counter if second entry is invalid. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      dst_idx  - (IN)Destination counter entry index.
 *      src_idx  - (IN)Counter entry index.
 *      flags    - (IN)BCM_L3_HIT.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_usage_cntr_set(int unit, int dst_idx, int src_idx, uint32 flags)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.   */
    uint64 usage_cntr;            /* Entry usage counter.   */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    if(BCM_XGS3_L3_INVALID_INDEX != src_idx) 
    {
        /* Read counter value. */
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_counter_get(unit, tbl_ptr->l3_arr[src_idx].rule_hdlr,
                                       BCM_XGS3_L3_EVEN_IDX(src_idx), &usage_cntr));
    } else {
        sal_memset(&usage_cntr, 0, sizeof usage_cntr);
        if (flags & BCM_L3_HIT) {
             COMPILER_64_ADD_32(usage_cntr, 1);
        }
    }

    /* Set Counter value.*/
    return bcm_esw_field_counter_set(unit, tbl_ptr->l3_arr[dst_idx].rule_hdlr,
                                     BCM_XGS3_L3_EVEN_IDX(dst_idx), usage_cntr);
}

/*
 * Function:
 *      _bcm_rp_l3_entry_add
 * Purpose:
 *      Add L3 rule to field processor.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      rule_idx  - (IN)FP entry rule index.
 *      l3_entry  - (IN)Added L3 entry information.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_entry_add(int unit, int rule_idx, _bcm_rp_l3_entry_t *l3_entry)
{
    _bcm_rp_l3_data_t *tbl_ptr;  /* L3 sw table address.     */
    bcm_ip6_t ip6_mask;          /* IPv6 destination mask.   */
    bcm_ip_t  ip4_mask;          /* IPv4 destination mask.   */ 
    bcm_field_entry_t rule_hdlr; /* L3 rule handler.         */
    bcm_ip6_t ip6;               /* IPv6 address.            */
    int ip_type;                 /* QualifyIpType value.     */
    int rule_priority;           /* Rule prioiry.            */ 
    int nh_idx;                  /* Next hop index.          */ 


    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);   

    /* Preserve rule handler. */
    rule_hdlr =  tbl_ptr->l3_arr[rule_idx].rule_hdlr;

    /* Copy FP rule info. */
    tbl_ptr->l3_arr[rule_idx] = *l3_entry;

    /* Restore rule handler. */
    tbl_ptr->l3_arr[rule_idx].rule_hdlr = rule_hdlr;

    /* Get next hop index for the rule. */
    nh_idx = l3_entry->nh_idx; 
    if (BCM_XGS3_L3_EGRESS_MODE_ISSET(unit)) {
        if (l3_entry->flags & BCM_L3_MULTIPATH) {
            nh_idx += BCM_XGS3_MPATH_EGRESS_IDX_MIN;
        } else {
            nh_idx += BCM_XGS3_EGRESS_IDX_MIN;
        }
    }

    /* Set rule qualification criterias. */
#if defined(BCM_MIRAGE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_routing_mirage)) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_qualify_DstPort(unit, rule_hdlr,
                                      0, 0, BCM_MIRAGE_L3_PORT, BCM_MIRAGE_L3_PORT));
    } else
#endif /* BCM_MIRAGE_SUPPORT */
    {
        /* Add unknown unicast. */
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_L3Routable(unit, rule_hdlr,
                                                0x1, 0x1));

    }
    if (l3_entry->flags & BCM_L3_IP6) {
        /* Set to protocol ipv6. */
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpType(unit, rule_hdlr,
                                          bcmFieldIpTypeIpv6));

        /* Calculate subnet mask. */
        bcm_ip6_mask_create(ip6_mask, l3_entry->prefixlen);

        /* Rule priority. */
        rule_priority = l3_entry->prefixlen;

        /* Copy address. */
        sal_memcpy(ip6, l3_entry->addr.ip6, sizeof(bcm_ip6_t));
    } else {

#if defined(BCM_XGS3_L3_FORWARD_IPV4_WITH_OPTIONS)
        ip_type = bcmFieldIpTypeIpv4Any; 
#else  /* BCM_XGS3_L3_FORWARD_IPV4_WITH_OPTIONS */
        ip_type = bcmFieldIpTypeIpv4NoOpts; 
#endif /* BCM_XGS3_L3_FORWARD_IPV4_WITH_OPTIONS */
#if defined(BCM_MIRAGE_SUPPORT) 
        if (soc_feature(unit, soc_feature_fp_routing_mirage)) {
            ip_type = bcmFieldIpTypeIpv4Any; 
        }
#endif /* BCM_MIRAGE_SUPPORT */

        /* Set to protocol ipv4. */
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpType(unit, rule_hdlr, ip_type));

        /* Calculate subnet mask. */
        ip4_mask = BCM_IP4_MASKLEN_TO_ADDR(l3_entry->prefixlen);
        BCM_XGS3_L3_CP_V4_ADDR_TO_V6(ip6_mask, ip4_mask);

        /* Rule priority. */
        if(l3_entry->prefixlen) { 
            /* This code comes to avoid extra shifts for host v4 entries. */
            rule_priority = l3_entry->prefixlen + 
                (BCM_XGS3_L3_IPV6_PREFIX_LEN - BCM_XGS3_L3_IPV4_PREFIX_LEN);
        } else {
            rule_priority = l3_entry->prefixlen; 
        }

        /* Set address. */
        BCM_XGS3_L3_CP_V4_ADDR_TO_V6(ip6, l3_entry->addr.ip4);
    }

    /* Add destination ip. */
    BCM_IF_ERROR_RETURN
        (bcm_esw_field_qualify_DstIp6(unit, rule_hdlr, ip6, ip6_mask));

    /* Set entry priority. */
    BCM_IF_ERROR_RETURN
        (bcm_esw_field_entry_prio_set(unit, rule_hdlr, rule_priority));


    /* Add  route  action  */
    BCM_IF_ERROR_RETURN(bcm_esw_field_action_add(unit, rule_hdlr,
                                                 bcmFieldActionL3Switch,
                                                 nh_idx, 0));

    /* Add  counter increment action  */
    BCM_IF_ERROR_RETURN
        (bcm_esw_field_action_add(unit, rule_hdlr,
                                  bcmFieldActionUpdateCounter, 
                                  BCM_XGS3_L3_CNTR_MODE(rule_idx), 0));

    /* Install entry to hw. */
    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_install(unit, rule_hdlr));

    /* Mark entry as valid. */
    tbl_ptr->l3_arr[rule_idx].flags |= BCM_XGS3_L3_ENT_VALID;

    /* Reset entry counter. */
    BCM_IF_ERROR_RETURN
        (_bcm_rp_l3_usage_cntr_set(unit, rule_idx, 
                                   BCM_XGS3_L3_INVALID_INDEX, l3_entry->flags));

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_rp_l3_entry_delete
 * Purpose:
 *      Remove L3 rule from  field processor.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      rule_idx  - (IN)FP entry rule index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_entry_delete(int unit, int rule_idx)
{
    _bcm_rp_l3_entry_t *l3_entry; /* L3 entry to be removed. */

    /* Get l3 entry. */
    l3_entry = &(BCM_XGS3_L3_TBL(unit, rp_prefix)).l3_arr[rule_idx];

    /* Uinstall entry from hw. */
    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_remove(unit, l3_entry->rule_hdlr));

    /* Remove all actions from the entry */
    BCM_IF_ERROR_RETURN
        (bcm_esw_field_action_remove_all(unit, l3_entry->rule_hdlr));

    /* Mark entry as invalid. */
    l3_entry->flags &= ~BCM_XGS3_L3_ENT_VALID;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_rp_l3_sw_entry_swap
 * Purpose:
 *      Swap to entries in host/route sw image. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      dst_idx  - (IN)Destination entry rule index.
 *      src_idx  - (IN)Original entry rule index. 
 * Returns:
 *      void
 */
STATIC INLINE void
_bcm_rp_l3_entry_swap(int unit, int dst_idx, int src_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.  */
    _bcm_rp_l3_entry_t l3_temp;  /* Entry swap buffer.    */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Preserve source entry. */
    l3_temp = tbl_ptr->l3_arr[src_idx];  

    /* Write destination to the source. */
    tbl_ptr->l3_arr[src_idx] = tbl_ptr->l3_arr[dst_idx];

    /* Write souce to the destination. */
    tbl_ptr->l3_arr[dst_idx] = l3_temp;
    return;
}

/*
 * Function:
 *      _bcm_rp_l3_entry_shift
 * Purpose:
 *      Copy l3 entry rule from one index to another.
 *      NOTE: Should be called with protection mutex locked. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      dst_idx  - (IN)Destination entry rule index.
 *      src_idx  - (IN)Original entry rule index. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_entry_shift(int unit, int dst_idx, int src_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /*
     * If destination & source sitting in the same FP group just swap
     * software images. (This saves  extra hw updates).
     */ 
    if ((dst_idx/tbl_ptr->rules_per_group) ==
        (src_idx/tbl_ptr->rules_per_group)) {
        _bcm_rp_l3_entry_swap(unit, dst_idx, src_idx);
        return (BCM_E_NONE);
    }

    /*
     * Rules are in the different groups we have to move/pack them. 
     * Add shifted rule new group and remove it from original one. 
     */
    BCM_IF_ERROR_RETURN
        (_bcm_rp_l3_entry_add(unit, dst_idx, &tbl_ptr->l3_arr[src_idx]));

    /* Copy rule usage counter. */
    BCM_IF_ERROR_RETURN(_bcm_rp_l3_usage_cntr_set(unit, dst_idx, src_idx, 0)); 

    /* Uinstall original entry from hw. */
    if (BCM_XGS3_L3_RP_HOST_ENTRY((&tbl_ptr->l3_arr[src_idx]))) {
        /* Remove host entry. */
        return  _bcm_rp_l3_entry_delete(unit, src_idx);
    }

    /* Remove route entry. */
    return _bcm_rp_lpm_entry_remove(unit, src_idx);
}

/*
 * Function:
 *      _bcm_rp_grp_rules_num_get
 * Purpose:
 *      Get number rules in group.  
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      rules    - (OUT)Interface table size.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_grp_rules_num_get(int unit, int *rules)
{
    int tcam_sz;         /* FP tcam size.        */
    int tcam_slices;     /* Number of FP slices. */

    /* Get tcam size. */
    tcam_sz = soc_mem_index_count(unit, FP_TCAMm);

    /* Note tcam_slices only counts internal slices. */
    if (soc_feature(unit, soc_feature_field_slices2)) {
        tcam_slices = 2;
    } else if (soc_feature(unit, soc_feature_field_slices4)) {
        tcam_slices = 4;
    } else {
        tcam_slices = soc_feature(unit, soc_feature_field_slices8) ? 8 : 16;
    }

    /* Get number of rules  */
    *rules = (tcam_sz / tcam_slices) - 1;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_rp_lpm_group_update
 * Purpose:
 *      Update FP group properties (min/max) prefix length location.      
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      grp_id     - (IN)FP group id. 
 * Returns:
 *      void
 */
STATIC void
_bcm_rp_lpm_group_update(int unit, int grp_id)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.                 */
    int max_pfx_len_idx;          /* Max prefix length rule idx in group  */
    int min_pfx_len_idx;          /* Max prefix length rule idx in group  */
    int idx;                      /* Iteration index.                     */

    /* Initialization. */
    max_pfx_len_idx = BCM_XGS3_L3_INVALID_INDEX;
    min_pfx_len_idx = BCM_XGS3_L3_INVALID_INDEX;

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Get l3 prefixes table. */
    /* Set min & max to first entry. */
    for (idx = grp_id * tbl_ptr->rules_per_group;
         ((idx < ((grp_id + 1) * tbl_ptr->rules_per_group)) && 
          (idx <= tbl_ptr->route_maxused_rule_id)); idx ++) {

        /* Check  only valid entries. */
        if (!(tbl_ptr->l3_arr[idx].flags & BCM_XGS3_L3_ENT_VALID)) {
            continue;    
        }

        /* Set minimum & maximum to the first valid entry. */ 
        if(max_pfx_len_idx == BCM_XGS3_L3_INVALID_INDEX) {
            max_pfx_len_idx =  min_pfx_len_idx = idx;
            continue;
        }

        /* Check if entry prefix length is less than minimum. */
        if (tbl_ptr->l3_arr[idx].prefixlen <
            tbl_ptr->l3_arr[min_pfx_len_idx].prefixlen){
            min_pfx_len_idx = idx;
        }

        /* Check if entry prefix length is greater than maximum. */
        if (tbl_ptr->l3_arr[idx].prefixlen > 
            tbl_ptr->l3_arr[max_pfx_len_idx].prefixlen){
            max_pfx_len_idx = idx;
        }
    }
    /* Preserve group max/min prefix length indexes. */
    tbl_ptr->l3_grp[grp_id].max_prefix_len_idx = max_pfx_len_idx; 
    tbl_ptr->l3_grp[grp_id].min_prefix_len_idx = min_pfx_len_idx; 
    return;
}

/*
 * Function:
 *      _bcm_rp_lpm_group_add_del
 * Purpose:
 *      Insert prefixlen to the group and 
 *      update (min/max) prefix length location.      
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      rule_idx   - (IN)FP prefix rule idx. 
 *      prefixlen  - (IN)Prefix length. 
 * Returns:
 *      void
 */
STATIC void
_bcm_rp_lpm_group_add_del(int unit, int rule_idx, int prefixlen)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.                 */
    int *max_pfx_len_idx;         /* Max prefix length rule idx in group  */
    int *min_pfx_len_idx;         /* Min prefix length rule idx in group  */
    int grp_id;                   /* FP l3 group index.                   */ 

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Get group id. */
    grp_id = rule_idx/(tbl_ptr->rules_per_group);

    /* Get group min & max prefix length indexes. */
    max_pfx_len_idx = &tbl_ptr->l3_grp[grp_id].max_prefix_len_idx;
    min_pfx_len_idx = &tbl_ptr->l3_grp[grp_id].min_prefix_len_idx;

    /* If first group entry added set min & max idx to it. */
    if (BCM_XGS3_L3_INVALID_INDEX != prefixlen) {
        if(0 == tbl_ptr->l3_grp[grp_id].rule_count) {
            *max_pfx_len_idx = *min_pfx_len_idx  = rule_idx;
        } else {
            /* Update group max & min indexes if necessary. */
            if (prefixlen < tbl_ptr->l3_arr[*min_pfx_len_idx].prefixlen) {
                *min_pfx_len_idx  = rule_idx;
            }
            if (prefixlen > tbl_ptr->l3_arr[*max_pfx_len_idx].prefixlen) {
                *max_pfx_len_idx  = rule_idx;
            }
        }
        /* Increment number of valid rules. */
        tbl_ptr->l3_grp[grp_id].rule_count++;
    } else {
        /* Prefix deleted from the group. */
        /* If first group entry added set min & max idx to it. */ 
        if(1 == tbl_ptr->l3_grp[grp_id].rule_count) {
            *max_pfx_len_idx = *min_pfx_len_idx  = BCM_XGS3_L3_INVALID_INDEX;
        } else {
            if ((rule_idx == *max_pfx_len_idx) || 
                (rule_idx == *min_pfx_len_idx)) {
                /* Update group max & min indexes. */
                _bcm_rp_lpm_group_update(unit, grp_id);
            }
        }
        /* Decrement number of valid rules. */
        tbl_ptr->l3_grp[grp_id].rule_count--;
    }
}

/*
 * Function:
 *      _bcm_rp_l3_deinit
 * Purpose:
 *      Deinitialize L3 functionality on Raptor 6530x devices. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_deinit(int unit)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.         */
    int idx;                      /* Iteration index.             */
    int rv = BCM_E_NONE;          /* Operation return status.     */
    int first_error = BCM_E_NONE; /* First error occured.         */ 

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);   

    /* If table was not initialized just return. */
    if ((NULL == tbl_ptr->l3_arr) || (NULL == tbl_ptr->l3_grp)) {
        return (BCM_E_NONE);
    }

    /* Iterate over all rules & delete them. */
    if (0 == SOC_HW_ACCESS_DISABLE(unit)) {
        for (idx = 0; idx < BCM_XGS3_L3_RP_MAX_PREFIXES(unit); idx++) {
            if (!tbl_ptr->l3_arr[idx].rule_hdlr) {
                continue;
            }
            rv = _bcm_rp_l3_rule_destroy(unit, idx);
            if ((rv < 0) && (first_error == BCM_E_NONE)) {
                first_error = rv;
            }
        }

        /* Delete allocated FP groups. */ 
        for (idx = 0; idx < tbl_ptr->l3_group_num; idx ++) { 
            if (!tbl_ptr->l3_grp[idx].grp_hdlr) {
                continue;
            }
            rv = bcm_esw_field_group_destroy(unit, 
                                             (tbl_ptr->l3_grp[idx]).grp_hdlr);
            if ((rv < 0) && (first_error == BCM_E_NONE)) {
                first_error = rv;
            }
        }
    }

    /* Free allocated memory. */
    if (tbl_ptr->l3_grp) {
        sal_free(tbl_ptr->l3_grp);
    }
    if (tbl_ptr->l3_arr) {
        sal_free(tbl_ptr->l3_arr);
    }

    /* Reset l3 control structure. */ 
    sal_memset(tbl_ptr, 0, sizeof(_bcm_rp_l3_data_t));
    return (rv);
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_hk_l3_field_init
 * Purpose:
 *      Allocate FP group and entries on Hakweye device. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_hk_l3_field_init(int unit)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    bcm_field_qset_t qset;        /* FP qualifiers set.        */
    int idx;                      /* Iteration index.          */
    int rv = BCM_E_NONE;          /* Operation return status.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Create qualifiers set. */
    BCM_FIELD_QSET_INIT(qset);

    /* Add qualifier fields. */
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyPacketFormat);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp6);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyL3Routable);

    /* Add FP groups. */
    for (idx = 0; idx < tbl_ptr->l3_group_num; idx++) {
        /* Create FP group. */
        if (tbl_ptr->l3_grp[idx].grp_hdlr) {
            /* This group has been created. */
            continue;
        }

        rv = bcm_esw_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY,
                                        &tbl_ptr->l3_grp[idx].grp_hdlr);
        if (rv < 0) {
            return rv;
        }
        /* Set max & min prefix length indexes. */
        tbl_ptr->l3_grp[idx].max_prefix_len_idx = BCM_XGS3_L3_INVALID_INDEX;
        tbl_ptr->l3_grp[idx].min_prefix_len_idx = BCM_XGS3_L3_INVALID_INDEX;
    }

    /* Sort groups by priority. */
    _shr_sort(tbl_ptr->l3_grp, tbl_ptr->l3_group_num,
              sizeof(_bcm_rp_l3_fp_group_t), _bcm_rp_grp_priority_cmp);

    /* Allocate rule handlers. */
    for (idx = 0; idx < BCM_XGS3_L3_RP_MAX_PREFIXES(unit); idx++) {
        if (tbl_ptr->l3_arr[idx].rule_hdlr) {
            /* This entry has been created. */
            continue;
        }
        rv = _bcm_rp_l3_rule_create(unit, idx);
        if (rv < 0) {
            return rv;
        }
    }
    return (BCM_E_NONE);
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      _bcm_rp_l3_init
 * Purpose:
 *      Initialize l3 functionality on Raptor 6530x device. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_init(int unit)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    bcm_field_qset_t qset;        /* FP qualifiers set.        */
    int mem_sz;                   /* Allocated memory size.    */
    int idx;                      /* Iteration index.          */
    int rv = BCM_E_NONE;          /* Operation return status.  */
#ifdef BCM_WARM_BOOT_SUPPORT
    int group_idx = 0;
#endif
    /* If number of prefixes is 0 - L3 functionality is disabled.*/ 
    if(!BCM_XGS3_L3_RP_MAX_PREFIXES(unit)) {
        return (BCM_E_NONE);
    }

    /* If fp rules & groups already present remove them first. */
    if(BCM_XGS3_L3_INITIALIZED(unit)) {
        _bcm_rp_l3_deinit(unit);
    }

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Set min used host index & max used route index. */
    tbl_ptr->route_maxused_rule_id = -1;
    tbl_ptr->host_minused_rule_id = BCM_XGS3_L3_RP_MAX_PREFIXES(unit);

    /* Get number of rules per group. */
    _bcm_rp_grp_rules_num_get(unit, &tbl_ptr->rules_per_group);

    /* Get number of groups. */
    tbl_ptr->l3_group_num = 
        BCM_XGS3_L3_RP_MAX_PREFIXES(unit)/tbl_ptr->rules_per_group;
    if (BCM_XGS3_L3_RP_MAX_PREFIXES(unit) % tbl_ptr->rules_per_group) { 
        tbl_ptr->l3_group_num++; 
    }

    /* FP group handlers. */
    mem_sz = tbl_ptr->l3_group_num * sizeof(_bcm_rp_l3_fp_group_t);
    BCM_XGS3_L3_ALLOC(tbl_ptr->l3_grp, mem_sz, "l3_fp_groups");
    if (NULL == tbl_ptr->l3_grp) {
        _bcm_rp_l3_deinit(unit); 
        return (BCM_E_MEMORY);
    }

    /* FP rule handlers. */
    mem_sz = BCM_XGS3_L3_RP_MAX_PREFIXES(unit) * sizeof(_bcm_rp_l3_entry_t);
    BCM_XGS3_L3_ALLOC(tbl_ptr->l3_arr, mem_sz, "l3_fp_rules");
    if (NULL == tbl_ptr->l3_arr) {
        _bcm_rp_l3_deinit(unit); 
        return (BCM_E_MEMORY);
    }

    /* Create qualifiers set. */
    BCM_FIELD_QSET_INIT(qset);

    /* Add qualifier fields. */
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyPacketFormat);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp6);
#if defined(BCM_MIRAGE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_routing_mirage)) {
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstPort);
    } else
#endif /* BCM_MIRAGE_SUPPORT */
    {
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyL3Routable);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_IS_HAWKEYE(unit)) {
        if (SOC_WARM_BOOT(unit)) {
            rv = bcm_esw_field_group_traverse(unit,
                                          _bcm_rp_l3_group_reload,
                                          &group_idx);
            /* Init is done if nothing recovered. */
            if (tbl_ptr->route_maxused_rule_id == -1 &&
                tbl_ptr->host_minused_rule_id == BCM_XGS3_L3_RP_MAX_PREFIXES(unit)) {
                return BCM_E_NONE;
            }
            rv = _bcm_hk_l3_field_init(unit);
            return (rv);
        }
        return BCM_E_NONE;
    }
#endif

    /* Add FP groups. */
    for (idx = 0; idx < tbl_ptr->l3_group_num; idx++) {
        /* Create FP group. */
        rv = bcm_esw_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY,
                                        &tbl_ptr->l3_grp[idx].grp_hdlr);
        if (rv < 0) {
            goto RP_INIT_ERR;
        }
        /* Set max & min prefix length indexes. */
        tbl_ptr->l3_grp[idx].max_prefix_len_idx = BCM_XGS3_L3_INVALID_INDEX;
        tbl_ptr->l3_grp[idx].min_prefix_len_idx = BCM_XGS3_L3_INVALID_INDEX;
    }

    /* Sort groups by priority. */
    _shr_sort(tbl_ptr->l3_grp, tbl_ptr->l3_group_num,
              sizeof(_bcm_rp_l3_fp_group_t), _bcm_rp_grp_priority_cmp);

    /* Allocate rule handlers. */
    for (idx = 0; idx < BCM_XGS3_L3_RP_MAX_PREFIXES(unit); idx++) {
        rv = _bcm_rp_l3_rule_create(unit, idx);
        if (rv < 0) {
            goto RP_INIT_ERR;
        }
    }
    return (BCM_E_NONE);

RP_INIT_ERR:
    _bcm_rp_l3_deinit(unit); 
    return rv;
}

/*
 * Function:
 *      _bcm_rp_l3_entry_idx_get
 * Purpose:
 *      Get host entry index.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3cfg     - (IN)Host lookup key.
 *      rule_idx  - (OUT) Entry index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_entry_idx_get(int unit, _bcm_l3_cfg_t *l3cfg, int *rule_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    _bcm_rp_l3_entry_t *l3_entry; /* L3 host entry.            */                      
    int ipv6;                     /* Entry family.             */
    int idx;                      /* Iteration index.          */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Get entry type. */
    ipv6 = l3cfg->l3c_flags & BCM_L3_IP6;

    /* Iterate over all host rules try to match passed address. */ 
    for (idx = tbl_ptr->host_minused_rule_id;
         idx < BCM_XGS3_L3_RP_MAX_PREFIXES(unit); idx++) {

        l3_entry = &tbl_ptr->l3_arr[idx];
        /* Skip family mismatch and invalid entries. */
        if (!(l3_entry->flags & BCM_XGS3_L3_ENT_VALID) ||
            ((l3_entry->flags & BCM_L3_IP6) != ipv6)) {
            continue;
        }

        /* Compare requested address with entry address. */  
        if (ipv6) {
            if(sal_memcmp(l3_entry->addr.ip6, 
                          l3cfg->l3c_ip6, sizeof(bcm_ip6_t))) { 
                continue;
            }
        } else {
            if (l3_entry->addr.ip4 != l3cfg->l3c_ip_addr) {
                continue;
            }
        }
        /* Entry Found. */
        *rule_idx = idx;
        return (BCM_E_NONE);
    } 
    /* Passed over all the entries & didn't find the match. */ 
    return (BCM_E_NOT_FOUND); 
}

/*
 * Function:
 *      _bcm_rp_l3_ent_parse
 * Purpose:
 *      Service routine used to parse  l3 entry to api format.
 * Parameters:
 *      unit      - (IN)SOC unit number. 
 *      l3cfg     - (OUT)l3 entry  lookup key & search result.
 *      nh_idx    - (OUT)Next hop index. 
 *      rule_idx  - (IN)Rule index.
 * Returns:
 *      void
 */
STATIC void
_bcm_rp_l3_ent_parse(int unit, _bcm_l3_cfg_t *l3cfg, int *nh_idx, int rule_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.     */
    uint64 usage_cntr;            /* Entry usage counter.     */ 
    int ipv6;                     /* Entry is IPv6 flag.      */
    int rv;                       /* Operation return status. */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /*  Get lookup type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Reset entry flags first. */
    l3cfg->l3c_flags = (ipv6) ? BCM_L3_IP6 : 0;

    /* Set entry hw index. */
    l3cfg->l3c_hw_index = rule_idx;

    /* Set vrf id to default. */
    l3cfg->l3c_vrf = BCM_L3_VRF_DEFAULT;

    /* Get entry usage counter. */ 
    rv = bcm_esw_field_counter_get(unit, tbl_ptr->l3_arr[rule_idx].rule_hdlr,
                                   BCM_XGS3_L3_EVEN_IDX(rule_idx), &usage_cntr);
    if ((rv >= 0) && (!COMPILER_64_IS_ZERO(usage_cntr))) {
        l3cfg->l3c_flags |= BCM_L3_HIT;
    }

    /* Get next hop index. */
    if(nh_idx) {
        *nh_idx =  tbl_ptr->l3_arr[rule_idx].nh_idx;
    }
    return;
}

/*
 * Function:
 *      _bcm_rp_l3_pack
 * Purpose:
 *      Pack l3 entries & free space for routes.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      BCM_E_NONE - If pack succeeded.
 *      BCM_E_FULL - Otherwise.
 */
STATIC int
_bcm_rp_l3_pack(int unit)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    int rule_idx;                 /* Entry index to write.     */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Try to find unused index. */
    for(rule_idx = BCM_XGS3_L3_RP_MAX_PREFIXES(unit) - 1; 
        rule_idx > tbl_ptr->route_maxused_rule_id; rule_idx--) {
        if(!(tbl_ptr->l3_arr[rule_idx].flags & BCM_XGS3_L3_ENT_VALID)) {
            break;
        }
    }

    /* If failed to allocated a new index -> table is full. */
    if (rule_idx == tbl_ptr->route_maxused_rule_id) {
        return (BCM_E_FULL); 
    }

    /* Shift last entry into an empty slot. */
    _bcm_rp_l3_entry_shift(unit, rule_idx, tbl_ptr->host_minused_rule_id);

    /* Update host minused rule id. */
    rule_idx = tbl_ptr->host_minused_rule_id;
    while (!(tbl_ptr->l3_arr[rule_idx].flags & BCM_XGS3_L3_ENT_VALID) &&
           rule_idx < BCM_XGS3_L3_RP_MAX_PREFIXES(unit)) {
        rule_idx++;
    }
    tbl_ptr->host_minused_rule_id = rule_idx;  

    return (BCM_E_NONE); 
}

/*
 * Function:
 *      _bcm_rp_l3_get
 * Purpose:
 *      Get an entry from L3 table.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      l3cfg    - (IN/OUT)l3 entry  lookup key & search result.
 *      nh_index - (OUT)Next hop index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_get(int unit, _bcm_l3_cfg_t *l3cfg, int *nh_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    int clear_hit;                /* Clear hit bit indicator.  */
    int rule_idx;                 /* Array entry index.        */
    int rv;                       /* Operation return status.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Preserve clear_hit value. */
    clear_hit = l3cfg->l3c_flags & BCM_L3_HIT_CLEAR;

    /* Lookup hosts table for match. */
    rv = _bcm_rp_l3_entry_idx_get(unit, l3cfg, &rule_idx);
    if(rv >= 0) {
        /* Entry Found - parse it. */
        _bcm_rp_l3_ent_parse(unit, l3cfg, nh_idx, rule_idx);

        /* Clear the HIT bit */
        if (clear_hit) {
            rv = _bcm_rp_l3_usage_cntr_set(unit, rule_idx,
                                           BCM_XGS3_L3_INVALID_INDEX, 0);
        }
    }

    return (rv); 
}

/*
 * Function:
 *      _bcm_rp_l3_add
 * Purpose:
 *      Add an entry to L3 table.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      l3cfg    - (IN)l3 entry information.
 *      nh_idx   - (IN)Next hop index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_add(int unit, _bcm_l3_cfg_t *l3cfg, int nh_idx)
{
    _bcm_rp_l3_entry_t l3_entry;  /* Host entry information.   */
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    int rule_idx;                 /* Entry index to write.     */
    int rv;                       /* Operation return status.  */

    /* Initialization. */
    sal_memset(&l3_entry, 0, sizeof(_bcm_rp_l3_entry_t));

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);
#ifdef BCM_WARM_BOOT_SUPPORT
    /* Initialize field if necessary. */
    if (SOC_IS_HAWKEYE(unit)) {
        if ( tbl_ptr->route_maxused_rule_id == -1 &&
             tbl_ptr->host_minused_rule_id == BCM_XGS3_L3_RP_MAX_PREFIXES(unit)) {
            rv = _bcm_hk_l3_field_init(unit);
            if (BCM_FAILURE(rv)) {
                return (rv);
            }
        }
    }
#endif
    /* Fill the entry info. */

    /* Set family . */
    l3_entry.flags = l3cfg->l3c_flags & BCM_L3_IP6;

    /* Set hit bit. */
    l3_entry.flags |= l3cfg->l3c_flags & BCM_L3_HIT;

    /* Set next hop index. */
    l3_entry.nh_idx = nh_idx;

    /* Set address and prefix length. */ 
    if (l3_entry.flags & BCM_L3_IP6) {
        l3_entry.prefixlen = BCM_XGS3_L3_IPV6_PREFIX_LEN;
        sal_memcpy(l3_entry.addr.ip6, l3cfg->l3c_ip6, sizeof(bcm_ip6_t));
    } else {
        l3_entry.prefixlen = BCM_XGS3_L3_IPV4_PREFIX_LEN;
        l3_entry.addr.ip4 = l3cfg->l3c_ip_addr;
    }

    /* Handle replacement if hw index already known, write directly. */
    if (BCM_XGS3_L3_INVALID_INDEX != l3cfg->l3c_hw_index) {
        rule_idx = l3cfg->l3c_hw_index;

        /* Remove original entry & actions */
        _bcm_rp_l3_entry_delete(unit, rule_idx);
    } else {
        /* Allocate a new index otherwise. */
        for(rule_idx = BCM_XGS3_L3_RP_MAX_PREFIXES(unit) - 1; 
            rule_idx > tbl_ptr->route_maxused_rule_id; rule_idx--) {
            if(!(tbl_ptr->l3_arr[rule_idx].flags & BCM_XGS3_L3_ENT_VALID)) {
                break;
            }
        }
        /* If failed to allocated a new index -> table is full. */
        if (rule_idx == tbl_ptr->route_maxused_rule_id) {
            return (BCM_E_FULL); 
        }
    }

    /* Add rule to FP. */
    rv = _bcm_rp_l3_entry_add(unit, rule_idx, &l3_entry);

    /* Write status check. */
    if ((rv >= 0) && (BCM_XGS3_L3_INVALID_INDEX == l3cfg->l3c_hw_index)) {
        /* Update host entry count. */
        (l3_entry.flags & BCM_L3_IP6) ?  \
            BCM_XGS3_L3_IP6_CNT(unit)++ : BCM_XGS3_L3_IP4_CNT(unit)++;

        /* Update host min used index. */
        if (rule_idx < tbl_ptr->host_minused_rule_id) {
            tbl_ptr->host_minused_rule_id = rule_idx;
        }
    }

    return (rv); 
}

/*
 * Function:
 *      _bcm_rp_l3_del
 * Purpose:
 *      Delete an entry from L3 table.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      l3cfg    - (IN)l3 entry deletion key.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_del(int unit, _bcm_l3_cfg_t *l3cfg)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    int rule_idx;                 /* L3 entry rule index.      */
    int rv;                       /* Operation return status.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Lookup hosts table for match. */
    rv =  _bcm_rp_l3_entry_idx_get(unit, l3cfg, &rule_idx);
    BCM_IF_ERROR_RETURN(rv);

    /* Delete the entry.*/  
    rv = _bcm_rp_l3_entry_delete(unit, rule_idx);

    /* Write status check. */
    if (rv >= 0) {
        /* Update host entry count. */
        (l3cfg->l3c_flags & BCM_L3_IP6) ? \
            BCM_XGS3_L3_IP6_CNT(unit)-- : BCM_XGS3_L3_IP4_CNT(unit)--;

        /* Update host min used index. */
        if (rule_idx == tbl_ptr->host_minused_rule_id) {
            while (!(tbl_ptr->l3_arr[rule_idx].flags & BCM_XGS3_L3_ENT_VALID) &&
                   rule_idx < BCM_XGS3_L3_RP_MAX_PREFIXES(unit)) {
                rule_idx++;
            }
            tbl_ptr->host_minused_rule_id = rule_idx;  
        }
    }

    return (rv);
}

/*
 * Function:
 *      _bcm_rp_l3_get_by_idx
 * Purpose:
 *      Get an entry from L3 table by index.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      dma_ptr  - (IN)NOT USED
 *      idx      - (IN)Index to read.
 *      l3cfg    - (OUT)l3 entry search result.
 *      nh_index - (OUT)Next hop index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_l3_get_by_idx(int unit, void *dma_ptr, int idx,
                      _bcm_l3_cfg_t *l3cfg, int *nh_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    _bcm_rp_l3_entry_t *l3_entry; /* L3 host entry.            */
    int clear_hit;                /* Clear hit bit indicator.  */
    int rule_idx;                 /* Array entry index.        */
    int ipv6;                     /* Entry family to read.     */
    int rv = BCM_E_NONE;          /* Operation return status.  */

    /* Get entry type. */
    ipv6 = l3cfg->l3c_flags & BCM_L3_IP6;

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Preserve clear_hit value. */
    clear_hit = l3cfg->l3c_flags & BCM_L3_HIT_CLEAR;

    /* Calculate rule index. */
    rule_idx = BCM_XGS3_L3_RP_MAX_PREFIXES(unit) - idx;
    if (rule_idx < 0) {
        return (BCM_E_NOT_FOUND);
    }

    l3_entry = &tbl_ptr->l3_arr[rule_idx];

    /* Make sure entry is a host & valid. */
    if ((rule_idx < tbl_ptr->host_minused_rule_id) ||
        (!(l3_entry->flags & BCM_XGS3_L3_ENT_VALID))) { 
        return (BCM_E_NOT_FOUND);
    }

    /* Get entry protocol. */
    l3cfg->l3c_flags = l3_entry->flags & BCM_L3_IP6;

    /* If protocol doesn't match the request return. */ 
    if ((l3_entry->flags  & BCM_L3_IP6) != ipv6) {
        return (BCM_E_NONE);
    }

    /* Get entry  address. */
    if (ipv6) {
        sal_memcpy(l3cfg->l3c_ip6, l3_entry->addr.ip6, sizeof(bcm_ip6_t));
    } else {
        l3cfg->l3c_ip_addr = l3_entry->addr.ip4;
    }

    /* Parset entry flags. */
    _bcm_rp_l3_ent_parse(unit, l3cfg, nh_idx, rule_idx);

    /* Clear the HIT bit */
    if (clear_hit) {
        rv = _bcm_rp_l3_usage_cntr_set(unit, rule_idx,
                                       BCM_XGS3_L3_INVALID_INDEX, 0);
    }

    /* Set index to l3cfg. */
    l3cfg->l3c_hw_index = idx;

    return (rv);
}

/*
 * Function:
 *      _bcm_rp_lpm_ent_parse
 * Purpose:
 *      Service routine used to parse  l3 entry to api format.
 * Parameters:
 *      unit      - (IN)SOC unit number. 
 *      lpm_cfg   - (IN/OUT)l3 entry  lookup key & search result.
 *      nh_idx    - (OUT)Next hop index. 
 *      rule_idx  - (IN)Rule index.
 * Returns:
 *      void
 */
STATIC void
_bcm_rp_lpm_ent_parse(int unit, _bcm_defip_cfg_t *lpm_cfg,
                      int *nh_idx, int rule_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.     */
    uint64 usage_cntr;            /* Entry usage counter.     */ 
    int ipv6;                     /* Entry is IPv6 flag.      */
    int rv;                       /* Operation return status. */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /*  Get lookup type. */
    ipv6 = (lpm_cfg->defip_flags & BCM_L3_IP6);

    /* Reset entry flags first. */
    lpm_cfg->defip_flags = (ipv6) ? BCM_L3_IP6 : 0;

    /* Set entry ecmp flag. */
    lpm_cfg->defip_flags |= 
        (tbl_ptr->l3_arr[rule_idx].flags & BCM_L3_MULTIPATH);

    /* Set vrf id to default. */
    lpm_cfg->defip_vrf = BCM_L3_VRF_DEFAULT;

    /* Set index of the defip table entry */
    lpm_cfg->defip_index = rule_idx;

    /* Get entry usage counter. */ 
    rv = bcm_esw_field_counter_get(unit, tbl_ptr->l3_arr[rule_idx].rule_hdlr,
                                   BCM_XGS3_L3_EVEN_IDX(rule_idx), &usage_cntr);
    if ((rv >= 0) && (!COMPILER_64_IS_ZERO(usage_cntr))) {
        lpm_cfg->defip_flags  |= BCM_L3_HIT;
    }

    /* Get next hop index. */
    if(nh_idx) {
        *nh_idx =  tbl_ptr->l3_arr[rule_idx].nh_idx;
    }
    return;
}

/*
 * Function:
 *      _bcm_rp_lpm_entry_idx_get
 * Purpose:
 *      Get route entry rule index.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      lpm_cfg   - (IN)Route lookup key.
 *      rule_idx  - (OUT) Entry index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_lpm_entry_idx_get(int unit, _bcm_defip_cfg_t *lpm_cfg, int *rule_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    _bcm_rp_l3_entry_t *l3_entry; /* L3 route entry.           */      
    int ipv6;                     /* Entry family.             */
    int idx;                      /* Iteration index.          */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Get entry type. */
    ipv6 = lpm_cfg->defip_flags & BCM_L3_IP6;

    /* Iterate over all host rules try to match passed address. */ 
    for (idx = 0; idx <= tbl_ptr->route_maxused_rule_id; idx++) {

        l3_entry = &tbl_ptr->l3_arr[idx];
        /* Skip family mismatch/prefix length mismatch  and invalid entries. */
        if (!(l3_entry->flags & BCM_XGS3_L3_ENT_VALID) ||
            ((l3_entry->flags & BCM_L3_IP6) != ipv6) ||
            (l3_entry->prefixlen != lpm_cfg->defip_sub_len)) {
            continue;
        }

        /* Compare requested address with entry address. */  
        if (ipv6) {
            if(sal_memcmp(l3_entry->addr.ip6, 
                          lpm_cfg->defip_ip6_addr, sizeof(bcm_ip6_t))) { 
                continue;
            }
        } else {
            if (l3_entry->addr.ip4 != lpm_cfg->defip_ip_addr) {
                continue;
            }
        }
        /* Entry Found. */
        *rule_idx = idx;
        return (BCM_E_NONE);
    }
    /* Passed over all the entries & didn't find the match. */ 
    return (BCM_E_NOT_FOUND); 
}

/*
 * Function:
 *      _bcm_rp_lpm_entry_insert
 * Purpose:
 *      Allocate rule index and write route to FP. 
 *      Function invoked recursively if there is no room in destination 
 *      group & rules shift is required. Max prefix length rule in each 
 *      group shifted one group ahead.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_lpm_entry_insert(int unit, _bcm_rp_l3_entry_t *l3_entry, int shift_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.                 */
    int max_prefix_len_idx;       /* Max prefix length rule idx in group  */
    int *max_rt_idx;              /* Last route rule index.               */ 
    int grp_idx;                  /* FP group index.                      */
    int insert_idx = BCM_XGS3_L3_INVALID_INDEX;  /* Rule insertion index. */ 

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Get max used route rule index. */
    max_rt_idx = &tbl_ptr->route_maxused_rule_id;

    if(BCM_XGS3_L3_INVALID_INDEX == shift_idx) {
        /* Attempt to insert into the first group. */         
        grp_idx = 0; 
    } else  {
        /* 
         * Group is full shift required - move maximum prefix 
         * length rule to the next group (group + 1). 
         */         
        grp_idx = (shift_idx/tbl_ptr->rules_per_group) + 1;
    }

    /* Iterate over all the group & try to find free rule idx. */
    for(; grp_idx < tbl_ptr->l3_group_num; grp_idx++) {
        /* Get group longet prefix rule index. */
        max_prefix_len_idx = tbl_ptr->l3_grp[grp_idx].max_prefix_len_idx;

        /* Check if group has an empty slot. NOTE: max_rt_idx is 0 based.*/
        if((grp_idx + 1) * tbl_ptr->rules_per_group  > (*max_rt_idx + 1)) {

            /* Make sure we  are not overwriting host entries. */
            if((*max_rt_idx) + 1 == tbl_ptr->host_minused_rule_id) {
                /* Attempt to pack host entries. */
                BCM_IF_ERROR_RETURN(_bcm_rp_l3_pack(unit));
            }

            /* Set write index to max used + 1 */
            insert_idx = ++(*max_rt_idx);
            break;

            /* Check if prefix length belongs to the group & there */ 
        } else if(tbl_ptr->l3_arr[max_prefix_len_idx].prefixlen >
                  l3_entry->prefixlen) {

            /* If this is last group - we can't shift rules anymore. */  
            if (grp_idx == tbl_ptr->l3_group_num - 1) {
                break;
            }

            /*
             *  Shift max prefix length entry to the next group.
             *  Write new entry into max prefix length entry place. 
             */
            insert_idx = max_prefix_len_idx;
            BCM_IF_ERROR_RETURN
                (_bcm_rp_lpm_entry_insert(unit,&tbl_ptr->l3_arr[insert_idx],
                                          insert_idx));
            break;
        }
    }

    /* If failed to allocate an index - table is full. */
    if (insert_idx == BCM_XGS3_L3_INVALID_INDEX) {
        return (BCM_E_FULL); 
    }

    /* Write entry to FP & update group properties. */
    BCM_IF_ERROR_RETURN(_bcm_rp_l3_entry_add(unit, insert_idx, l3_entry));

    /* Update group max/min prefix location. */ 
    _bcm_rp_lpm_group_add_del(unit, insert_idx, l3_entry->prefixlen);

    /* If entry was shifted -> delete original entry. */
    if(BCM_XGS3_L3_INVALID_INDEX != shift_idx) {
        /* Remove shifted rule. */
        BCM_IF_ERROR_RETURN(_bcm_rp_l3_entry_delete(unit, shift_idx));

        /* Decrement number of rules in original group */
        _bcm_rp_lpm_group_add_del(unit, shift_idx, BCM_XGS3_L3_INVALID_INDEX);
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_rp_lpm_entry_remove
 * Purpose:
 *      Free rule index and delete route rule from FP. 
 *      Function fills gaps rule array. And invoked recursively 
 *      if there are rules in higher groups - which need to be
 *      shifted to to lower group which currently has an empty space. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      rule_idx - (IN)Route entry rule for deletion.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_lpm_entry_remove(int unit, int rule_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    int *max_rt_idx;              /* Last route rule index.    */ 
    int shift_idx;                /* Shifted entry rule index. */
    int grp_idx;                  /* FP group index.           */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Get max used route rule index. */             
    max_rt_idx = &tbl_ptr->route_maxused_rule_id; 

    /* Get FP group index. */
    grp_idx = (rule_idx/tbl_ptr->rules_per_group);

    /* Remove the rule from FP. */
    BCM_IF_ERROR_RETURN(_bcm_rp_l3_entry_delete(unit, rule_idx));

    /* Update group prefix count & min/max locations. */
    _bcm_rp_lpm_group_add_del(unit, rule_idx, BCM_XGS3_L3_INVALID_INDEX);

    /* If last entry was removed just decrement max used route index. */
    if (rule_idx == (*max_rt_idx)) {
        /* Last entry was removed update max route index. */
        (*max_rt_idx)--;
        return (BCM_E_NONE);
    }

    /*
     *  If all the routes are in the same group, take the last one
     *  and write to the location of the deleted one. 
     *  If routes are spread across the groups, shift min prefixlen 
     *  route in each group one group down. 
     */
    if((grp_idx + 1) * tbl_ptr->rules_per_group > (*max_rt_idx)) {

        /* Last group. */
        shift_idx = (*max_rt_idx);

        /* Swap last entry with deleted one. */
        _bcm_rp_l3_entry_swap(unit, rule_idx, shift_idx);

        /* If maximum prefix length entry shifted update index. */ 
        if (shift_idx == tbl_ptr->l3_grp[grp_idx].max_prefix_len_idx) {
            tbl_ptr->l3_grp[grp_idx].max_prefix_len_idx = rule_idx;
        }

        /* If minimum prefix length entry shifted update index. */ 
        if (shift_idx == tbl_ptr->l3_grp[grp_idx].min_prefix_len_idx) {
            tbl_ptr->l3_grp[grp_idx].min_prefix_len_idx = rule_idx;
        }

        /* Since we are shifting last entry decrement max route index. */ 
        (*max_rt_idx)--;
    } else {

        /* Shift min prefix length entry from the next group if any. */
        shift_idx = tbl_ptr->l3_grp[grp_idx + 1].min_prefix_len_idx;

        /* Perform rule shift operation. */
        BCM_IF_ERROR_RETURN(_bcm_rp_l3_entry_shift(unit, rule_idx, shift_idx));

        /* Add shifted rule prefix length to the destination group. */
        _bcm_rp_lpm_group_add_del(unit, rule_idx,
                                  tbl_ptr->l3_arr[shift_idx].prefixlen);
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_rp_lpm_get
 * Purpose:
 *      Get an entry from DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_lpm_get(int unit, _bcm_defip_cfg_t *lpm_cfg, int *nh_ecmp_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    int clear_hit;                /* Clear hit bit indicator.  */
    int rule_idx;                 /* Array entry index.        */
    int rv;                       /* Operation return status.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Preserve clear_hit value. */
    clear_hit = lpm_cfg->defip_flags & BCM_L3_HIT_CLEAR;

    /* Lookup hosts table for match. */
    rv = _bcm_rp_lpm_entry_idx_get(unit, lpm_cfg, &rule_idx);
    if(rv >= 0) {
        /* Entry Found - parse it. */
        _bcm_rp_lpm_ent_parse(unit, lpm_cfg, nh_ecmp_idx, rule_idx);

        /* Clear the HIT bit */
        if (clear_hit) {
            rv = _bcm_rp_l3_usage_cntr_set(unit, rule_idx,
                                           BCM_XGS3_L3_INVALID_INDEX, 0);
        }
    }

    return (rv); 
}

/*
 * Function:
 *      _bcm_rp_lpm_add
 * Purpose:
 *      Add an entry to route table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_lpm_add(int unit, _bcm_defip_cfg_t *lpm_cfg, int nh_ecmp_idx)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    _bcm_rp_l3_entry_t l3_entry;  /* Route entry.              */
    int rule_idx;                 /* Entry index to write.     */
    int rv;                       /* Operation return status.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

#ifdef BCM_WARM_BOOT_SUPPORT
    /* Initialize field if necessary. */
    if (SOC_IS_HAWKEYE(unit)) {
        if ( tbl_ptr->route_maxused_rule_id == -1 &&
             tbl_ptr->host_minused_rule_id == BCM_XGS3_L3_RP_MAX_PREFIXES(unit)) {
            rv = _bcm_hk_l3_field_init(unit);
            if (BCM_FAILURE(rv)) {
                return (rv);
            }
        }
    }
#endif
    /* Fill the entry info. */
    /* Set family and prefix length. */
    l3_entry.flags = lpm_cfg->defip_flags & BCM_L3_IP6;

    /* Set hit bit. */
    l3_entry.flags |= lpm_cfg->defip_flags & BCM_L3_HIT;

    /* Set ecmp bit. */
    l3_entry.flags |= lpm_cfg->defip_flags & BCM_L3_MULTIPATH;

    /* Set prefix length. */
    l3_entry.prefixlen = lpm_cfg->defip_sub_len;

    /* Set subnet address */
    if (l3_entry.flags & BCM_L3_IP6) {
        sal_memcpy(l3_entry.addr.ip6, lpm_cfg->defip_ip6_addr, sizeof(bcm_ip6_t));
    } else {
        l3_entry.addr.ip4 = lpm_cfg->defip_ip_addr;
    }

    /* Set next hop index. */
    l3_entry.nh_idx = nh_ecmp_idx;

    /* Handle replacement if hw index already known, write directly. */
    if (BCM_XGS3_L3_INVALID_INDEX != lpm_cfg->defip_index) {

        rule_idx = lpm_cfg->defip_index;

        /* Remove original entry & actions */
        _bcm_rp_l3_entry_delete(unit, rule_idx);

        /* Only next hop update is supported. */
        tbl_ptr->l3_arr[rule_idx].nh_idx = nh_ecmp_idx;

        /* Write rule to FP. */
        rv = _bcm_rp_l3_entry_add(unit, rule_idx, &tbl_ptr->l3_arr[rule_idx]);

        return (rv);
    }

    /* Add rule to FP. */
    rv = _bcm_rp_lpm_entry_insert(unit, &l3_entry, BCM_XGS3_L3_INVALID_INDEX);

    /* Increment number of routes. */
    if (rv >= 0) {
        BCM_XGS3_L3_DEFIP_CNT_INC(unit, (lpm_cfg->defip_flags & BCM_L3_IP6));
    }

    return (rv); 
}

/*
 * Function:
 *      _bcm_rp_lpm_del
 * Purpose:
 *      Delete an entry from route table.
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Buffer to fill defip information. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_lpm_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    int rule_idx;                 /* Array entry index.        */
    int rv;                       /* Operation return status.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Lookup hosts table for match. */
    rv = _bcm_rp_lpm_entry_idx_get(unit, lpm_cfg, &rule_idx);
    BCM_IF_ERROR_RETURN(rv);

    /* Remove the route rule from FP. */
    rv = _bcm_rp_lpm_entry_remove(unit, rule_idx);

    /* Decrement number of routes. */
    if (rv >= 0) {
        BCM_XGS3_L3_DEFIP_CNT_DEC(unit, (lpm_cfg->defip_flags & BCM_L3_IP6));
    }

    return (rv);
}

/*
 * Function:
 *      _bcm_rp_lpm_update_match
 * Purpose:
 *      Update/Delete all entries in defip table matching a certain rule.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      trv_data - (IN)Delete pattern + compare,act,notify routines.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_rp_lpm_update_match(int unit, _bcm_l3_trvrs_data_t *trv_data)
{
    _bcm_rp_l3_data_t *tbl_ptr;   /* L3 sw table address.      */
    _bcm_rp_l3_entry_t *l3_entry; /* L3 route entry.           */      
    _bcm_rp_l3_entry_t *rt_tbl;   /* L3 route table shadow.    */      
    _bcm_defip_cfg_t lpm_cfg;     /* Buffer to fill route info.*/
    int nh_ecmp_idx;              /* Next hop/Ecmp group index.*/
    int cmp_result;               /* Test routine result.      */
    int tbl_sz;                   /* L3 route table size.      */
    int ipv6;                     /* Entry family.             */
    int idx;                      /* Iteration index.          */
    int rv = BCM_E_NOT_FOUND;     /* Operation return status.  */

    /* Get l3 prefixes table. */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);

    /* Create shaddow image. */

    tbl_sz= (tbl_ptr->route_maxused_rule_id + 1) * sizeof(_bcm_rp_l3_entry_t);

    /* Allocate memory for shadow image. */
    rt_tbl = sal_alloc(tbl_sz, "rp_routes_shadow");   
    if (NULL == rt_tbl) {
        return (BCM_E_MEMORY); 
    }

    /* Copy all  the routes. */
    sal_memcpy(rt_tbl, tbl_ptr->l3_arr, tbl_sz);

    /* Preserve route table size. */
    tbl_sz = tbl_ptr->route_maxused_rule_id + 1;

    /* Get requested entry type. */
    ipv6 = trv_data->flags & BCM_L3_IP6;

    /* Iterate over all host rules try to match passed address. */ 
    for (idx = 0; idx < tbl_sz; idx++) {

        l3_entry = rt_tbl + idx;


        /* Skip family mismatch/prefix length mismatch  and invalid entries. */
        if (!(l3_entry->flags & BCM_XGS3_L3_ENT_VALID) ||
            ((l3_entry->flags & BCM_L3_IP6) != ipv6)) {
            continue;
        }

        /* Zero destination buffer first. */
        sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));

        /* Set entry flags. */
        lpm_cfg.defip_flags = l3_entry->flags & BCM_L3_IP6;

        /* Fill entry ip address &  prefix length. */
        if (ipv6) {
            sal_memcpy(lpm_cfg.defip_ip6_addr, l3_entry->addr.ip6,
                       sizeof(bcm_ip6_t));
        } else {
            lpm_cfg.defip_ip_addr = l3_entry->addr.ip4;
        }
        lpm_cfg.defip_sub_len = l3_entry->prefixlen;

        /* Get entry flags  & info . */
        _bcm_rp_lpm_get(unit, &lpm_cfg, &nh_ecmp_idx);

        /* Execute operation routine if any. */
        if (trv_data->op_cb) {
            rv = (*trv_data->op_cb) (unit, (void *)trv_data,
                                     (void *)&lpm_cfg,
                                     (void *)&nh_ecmp_idx, &cmp_result);
            if (rv < 0) {
                sal_free(rt_tbl);
                return rv;
            }
        }
    }
    /* Free shadow image. */
    sal_free(rt_tbl);
    return (BCM_E_NONE);
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_rp_l3_group_reload
 * Purpose:
 *      Restore l3 route entries from FP. 
 * Parameters:
 *      unit     - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
static int _bcm_rp_l3_group_reload(int unit, bcm_field_group_t group, 
    void *user_data)
{
    int idx, pfx_len, rule_idx, rv, nh_index, update_grp_idx = 0;
    int *grp_idx;
    int entry_count, entry_num;
    bcm_field_IpType_t iptype;
    bcm_ip6_t data; 
    bcm_ip6_t mask;
    bcm_ip_t ip_mask;
    _bcm_rp_l3_data_t *tbl_ptr;
    int alloc_sz;
    bcm_field_entry_t *entry_array, entry_id;
    uint32 param0, param1;

    BCM_IF_ERROR_RETURN
        (bcm_esw_field_entry_multi_get(unit, group, 0, NULL, &entry_num));

    if (entry_num == 0) {
        return BCM_E_PARAM;
    }

    alloc_sz = sizeof(bcm_field_entry_t) * entry_num;
    entry_array = sal_alloc(alloc_sz, "Field IDs");
    if (NULL == entry_array) {
        return BCM_E_MEMORY;
    }
    sal_memset(entry_array, 0, alloc_sz);
        
    rv = bcm_esw_field_entry_multi_get(unit, group, entry_num,
                                       entry_array, &entry_count);
    
    if (BCM_FAILURE(rv)) {
        sal_free(entry_array);
        return rv;
    }

    if (entry_count != entry_num) {
        /* Why didn't we get the number of ID's we were told existed? */
        sal_free(entry_array);
        return BCM_E_INTERNAL;
    }

    grp_idx = (int *)user_data;
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, rp_prefix);
    
    for (idx = 0; idx < entry_num; idx++) {
        entry_id = entry_array[idx];
        rv = bcm_esw_field_action_get(unit, entry_id,
                                      bcmFieldActionL3Switch,
                                      &param0, &param1);
        if (BCM_SUCCESS(rv)) {
            if (*grp_idx >= tbl_ptr->l3_group_num) {
                sal_free(entry_array);
                return (BCM_E_NONE);
            }
            tbl_ptr->l3_grp[*grp_idx].grp_hdlr = group;
            nh_index = param0;
        } else {
            continue;
        }
        
        rv = bcm_esw_field_qualify_IpType_get(unit, entry_id, &iptype);
        if (BCM_FAILURE(rv)) {
            continue;
        }
        
        rv = bcm_esw_field_qualify_DstIp6_get(unit, entry_id, &data, &mask);
        if (BCM_FAILURE(rv)) {
            continue;
        }

        update_grp_idx = 1;

        if (iptype == bcmFieldIpTypeIpv6) {
            pfx_len = bcm_ip6_mask_length(mask);
            if (pfx_len == BCM_XGS3_L3_IPV6_PREFIX_LEN) {
                /* Assume this is host entry. */
                tbl_ptr->host_minused_rule_id--;
                rule_idx = tbl_ptr->host_minused_rule_id;
                BCM_XGS3_L3_IP6_CNT(unit)++;
            } else {
                /* Assume this is route entry. */
                tbl_ptr->route_maxused_rule_id++;
                rule_idx = tbl_ptr->route_maxused_rule_id;
                BCM_XGS3_L3_DEFIP_CNT_INC(unit, TRUE);
                tbl_ptr->l3_grp[*grp_idx].rule_count++;
            }
            sal_memcpy(tbl_ptr->l3_arr[rule_idx].addr.ip6, data, sizeof(bcm_ip6_t));
            tbl_ptr->l3_arr[rule_idx].prefixlen = pfx_len;
            tbl_ptr->l3_arr[rule_idx].flags |= BCM_L3_IP6;
        } else {
            sal_memcpy(&ip_mask, 
                        (uint8 *)mask + BCM_XGS3_V4_IN_V6_OFFSET,
                        sizeof(bcm_ip_t));
            pfx_len = bcm_ip_mask_length(ip_mask);
            if (pfx_len == BCM_XGS3_L3_IPV4_PREFIX_LEN) {
                /* Assume this is host entry. */
                tbl_ptr->host_minused_rule_id--;
                rule_idx = tbl_ptr->host_minused_rule_id;
                BCM_XGS3_L3_IP4_CNT(unit)++;
                
            } else {
                /* Assume this is route entry. */
                tbl_ptr->route_maxused_rule_id++;
                rule_idx = tbl_ptr->route_maxused_rule_id;
                BCM_XGS3_L3_DEFIP_CNT_INC(unit, FALSE);
                tbl_ptr->l3_grp[*grp_idx].rule_count++;
            }
            sal_memcpy(&tbl_ptr->l3_arr[rule_idx].addr.ip4, 
                        (uint8 *)data + BCM_XGS3_V4_IN_V6_OFFSET,
                        sizeof(bcm_ip_t));
            tbl_ptr->l3_arr[rule_idx].prefixlen = pfx_len;
        }
        tbl_ptr->l3_arr[rule_idx].rule_hdlr = entry_id;
        tbl_ptr->l3_arr[rule_idx].nh_idx = nh_index;
        tbl_ptr->l3_arr[rule_idx].flags |= BCM_XGS3_L3_ENT_VALID;
    }
    
    if (update_grp_idx) {
        if (tbl_ptr->route_maxused_rule_id != -1 &&
            *grp_idx < tbl_ptr->l3_group_num) {
            _bcm_rp_lpm_group_update(unit, *grp_idx);
        }
        *grp_idx = *grp_idx+1;
    }

    if (rv == BCM_E_NOT_FOUND) {
        rv = BCM_E_NONE; /* Do not propagate this error */
    }

    sal_free(entry_array);
    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#endif /* BCM_RAPTOR_SUPPORT  || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */

#endif /* BCM_FIREBOLT_SUPPORT  */
#ifdef BCM_EASYRIDER_SUPPORT

/*
 * Function:
 *      _bcm_er_l3_intf_vrf_get
 * Purpose:
 *      Get the VRF,flags info for the specified L3 interface
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      intf_info - (IN/OUT)Interface configuration info.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_er_l3_intf_vrf_get(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    vrf_vfi_intf_entry_t vrf_entry;     /* Buffer to read vrf entry. */
    vrf_vfi_intf_entry_t vrf_key;       /* VRF lookup key.           */
    int vfi;                    /* VFI/VRF interface flag.   */
    int ifindex;                /* Interface index.          */
    int hw_index;               /* Search result index.      */
    int rv;                     /* Operation status.         */

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Initialize vrf id to zero. */
    intf_info->l3i_vrf = BCM_L3_VRF_DEFAULT;

    /* Reset lookup key. */
    sal_memset(&vrf_key, 0, sizeof(vrf_vfi_intf_entry_t));

    /* Set SRC_MODID,TGID to don't care, 0xfff */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, SRC_MODIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, TGIDf, 0x3f);

    /* Set interface vlan id. */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, VLANf, intf_info->l3i_vid);

    /* Perform memory search. */
    rv = soc_mem_search(unit, VRF_VFI_INTFm,
                        MEM_BLOCK_ANY, &hw_index, &vrf_key, &vrf_entry, 0);

    if (rv == BCM_E_NONE) {
        /* Check if interface is vrf. */
        vfi = soc_VRF_VFI_INTFm_field32_get(unit, &vrf_entry, VFIf);

        if (vfi == 0) {         /* VRF  */
            /* Get interface index. */
            ifindex = soc_VRF_VFI_INTFm_field32_get(unit, &vrf_entry, L3_INTFf);
            /* Get VRF id. */
            intf_info->l3i_vrf = soc_VRF_VFI_INTFm_field32_get(unit, &vrf_entry,
                                                               VRF_VFIf);
            /*
             *  Set interface secondary flag if ifindex doesn't 
             *  match original interface.
             */
            if (ifindex != intf_info->l3i_index) {
                intf_info->l3i_flags |= BCM_L3_SECONDARY;
            }
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_er_l3_intf_vrf_bind
 * Purpose:
 *      Bind interface to the VRF & update interface info.
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface configuration info.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_er_l3_intf_vrf_bind(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    vrf_vfi_intf_entry_t vrf_entry;     /* Buffer to be written to vrf table. */

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Zero the buffer. */
    sal_memset(&vrf_entry, 0, sizeof(vrf_vfi_intf_entry_t));

    /* Set interface index. */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_entry, L3_INTFf,
                                  intf_info->l3i_index);

    /* Set vrf id. */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_entry, VRF_VFIf,
                                  intf_info->l3i_vrf);

    /* Set vfi flag to indicate vrf interface. */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_entry, VFIf, 0);

    /* Set SRC_MODID,TGID to don't care, 0xfff */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_entry, SRC_MODIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_entry, TGIDf, 0x3f);

    /* Set interface vlan id. */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_entry, VLANf, intf_info->l3i_vid);

    if (SOC_MEM_FIELD_VALID(unit, VRF_VFI_INTFm, MPLS_ENABLEf)) {
        soc_VRF_VFI_INTFm_field32_set(unit, &vrf_entry, MPLS_ENABLEf, 1);
    }

    /* Write entry to the hw. */
    return soc_mem_insert(unit, VRF_VFI_INTFm, COPYNO_ALL, &vrf_entry);
}

/*
 * Function:
 *      _bcm_er_l3_intf_vrf_unbind
 * Purpose:
 *      Unbind interface to the VRF & update interface info.
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface configuration info.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_er_l3_intf_vrf_unbind(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    vrf_vfi_intf_entry_t vrf_key;     /* Buffer to search vrf table.        */
    vrf_vfi_intf_entry_t vrf_entry;   /* Buffer to be written to vrf table. */
    int hw_index;                     /* Vrf table index.                   */
    int ifindex;                      /* Inteface index from the hw entry.  */
    int vfi;                          /* Flag to indicate vrf.              */
    int rv;                           /* Operation return status.           */

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Zero the buffer. */
    sal_memset(&vrf_key, 0, sizeof(vrf_vfi_intf_entry_t));

    /* Set SRC_MODID,TGID to don't care, 0xfff */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, SRC_MODIDf, 0x3f);
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, TGIDf, 0x3f);

    /* Set vlan id. */
    soc_VRF_VFI_INTFm_field32_set(unit, &vrf_key, VLANf, intf_info->l3i_vid);

    /* Try to locate record in vrf table. */
    rv = soc_mem_search(unit, VRF_VFI_INTFm, MEM_BLOCK_ANY,
                        &hw_index, &vrf_key, &vrf_entry, 0);


    if (rv == BCM_E_NONE) {
        vfi = soc_VRF_VFI_INTFm_field32_get(unit, &vrf_entry, VFIf);
        ifindex = soc_VRF_VFI_INTFm_field32_get(unit, &vrf_entry, L3_INTFf);

        /* On vrf bound interfaces match interface index & delete. */
        if ((vfi == 0) && (ifindex == intf_info->l3i_index)) {
            rv = soc_mem_delete(unit, VRF_VFI_INTFm, MEM_BLOCK_ALL, &vrf_entry);
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_er_l3_intf_group_set
 * Purpose:
 *      Add interface to the group
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN) Interface configuration info.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_er_l3_intf_group_set(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    l3intf_igr_filter_list_entry_t igr_list;    /* Buffer for ingress tbl. */
    l3intf_egr_filter_list_entry_t egr_list;    /* Buffer for egress tbl.  */
    int rv;                     /* Operation return status. */

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    /* Reset buffers to zero. */
    sal_memset(&igr_list, 0, sizeof(l3intf_igr_filter_list_entry_t));
    sal_memset(&egr_list, 0, sizeof(l3intf_egr_filter_list_entry_t));

    /* Set interface group id to ingress filter table. */
    soc_L3INTF_IGR_FILTER_LISTm_field32_set(unit, &igr_list,
                                            INGRESS_FILTER_LISTf,
                                            intf_info->l3i_group);

    /* Set interface group id to egress filter table. */
    soc_L3INTF_EGR_FILTER_LISTm_field32_set(unit, &egr_list,
                                            EGRESS_FILTER_LISTf,
                                            intf_info->l3i_group);

    /* Write ingress filter table. */
    rv = WRITE_L3INTF_IGR_FILTER_LISTm(unit, MEM_BLOCK_ALL,
                                       intf_info->l3i_index, &igr_list);
    BCM_IF_ERROR_RETURN(rv);

    /* Write egress filter table. */
    rv = WRITE_L3INTF_EGR_FILTER_LISTm(unit, MEM_BLOCK_ALL,
                                       intf_info->l3i_index, &egr_list);
    if (rv < 0) {
        /* Perform the best effort clean up. */
        sal_memset(&igr_list, 0, sizeof(l3intf_igr_filter_list_entry_t));

        WRITE_L3INTF_IGR_FILTER_LISTm(unit, MEM_BLOCK_ALL,
                                      intf_info->l3i_index, &igr_list);
    }
    return rv;
}


/*
 * Function:
 *      _bcm_er_l3_intf_group_get
 * Purpose:
 *      Get interface group id. 
 * Parameters:
 *      unit      - (IN) SOC unit number.
 *      intf_info - (IN/OUT) Interface configuration info.
 * Returns:
 *      BCM_E_XXX.
 */
STATIC int
_bcm_er_l3_intf_group_get(int unit, _bcm_l3_intf_cfg_t *intf_info)
{
    l3intf_igr_filter_list_entry_t entry;           /* HW entry buffer.   */
    int idx;                                        /* Interface index.   */

    /* Input parameters sanity check. */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }
  
    idx = intf_info->l3i_index;

    if ((idx > soc_mem_index_max(unit, L3INTF_IGR_FILTER_LISTm)) || 
        (idx < soc_mem_index_min(unit, L3INTF_IGR_FILTER_LISTm))) {
        return (BCM_E_PARAM);
    }

    /* Read buffer from hw. */
    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, L3INTF_IGR_FILTER_LISTm, MEM_BLOCK_ANY,
                      idx, (uint32 *)&entry));

    intf_info->l3i_group = soc_mem_field32_get(unit, L3INTF_IGR_FILTER_LISTm, 
                                               (uint32 *)&entry,
                                               INGRESS_FILTER_LISTf);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_er_nh_add
 * Purpose:
 *      Routine set next hop entry in the chip.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      idx       - (IN)Allocated entry index .
 *      buf       - (IN)Next hop entry data.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_er_nh_add(int unit, int idx, void *buf, int max_paths)
{
    uint32 nh_buf[SOC_MAX_MEM_FIELD_WORDS];/* Buffer to fill nh entry.      */
    bcm_l3_egress_t *nh_entry;             /* Next hop entry passed buffer. */
    soc_mem_t mem;                         /* Table location memory.        */
    uint32 *buf_p;                         /* HW address buffer.            */

    /* Input parameters check */
    if (NULL == buf) {
        return (BCM_E_PARAM);
    }
    nh_entry = (bcm_l3_egress_t *) buf;
    buf_p = (uint32 *)&nh_buf;

    if (nh_entry->flags & BCM_L3_COPY_TO_CPU) {
        return (BCM_E_UNAVAIL);
    }

    /* Reset new entry info. */
    sal_memset(buf_p, 0, BCM_XGS3_L3_ENT_SZ(unit, nh));

    /* Get next hop table memory address. */
    mem = BCM_XGS3_L3_MEM(unit, nh);

    /* Set mac address. */
    soc_mem_mac_addr_set(unit, mem, buf_p, MAC_ADDRf, nh_entry->mac_addr);

    /* Set module id. */
    soc_mem_field32_set(unit, mem, buf_p, MODIDf, nh_entry->module);

    /* Set port/trunk id. */
    soc_mem_field32_set(unit, mem, buf_p, PORT_TGIDf, nh_entry->port);

    /* Set interface index. */
    soc_mem_field32_set(unit, mem, buf_p, L3_INTF_NUMf, nh_entry->intf);

    /* Inser mpls label only if it is valid.  */
    if (BCM_XGS3_L3_MPLS_LBL_VALID(nh_entry->mpls_label)) {
        soc_mem_field32_set(unit, mem, buf_p,
                            FASTREROUTE_LABELf, nh_entry->mpls_label);
    }

    return BCM_XGS3_MEM_WRITE(unit, mem, idx, buf_p);
}



/*
 * Function:
 *      _bcm_er_lpm_hit_set
 * Purpose:
 *      Clear lpm entry hit bit.
 * Parameters:
 *      unit    -(IN)SOC unit number. 
 *      lpm_cfg -(IN)LPM entry information. 
 *      value   -(IN)Hit bit value 0/1.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_hit_set(int unit, _bcm_defip_cfg_t *lpm_cfg, int value)
{
    lpm_hitbit_entry_t h_entry; /* Hardware read buffer. */
    uint32 bmval;               /* Hit value.            */
    int index;                  /* LPM hit table index.  */
    int rv;                     /* Return value.         */

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Calculate table index. */
    index = (lpm_cfg->defip_index >> 5);
    rv = BCM_XGS3_MEM_READ(unit, L3_LPM_HITBITm, index, &h_entry);
    BCM_IF_ERROR_RETURN(rv);

    /* Read entry from hw. */
    soc_L3_LPM_HITBITm_field_get(unit, &h_entry, HITBITSf, &bmval);

    /* Set hit bit. */
    if (value) {  
       bmval |= (1 << (lpm_cfg->defip_index & 0x1f));
    } else {
       bmval &= ~(1 << (lpm_cfg->defip_index & 0x1f));
    }

    /* Set value to the buffer. */
    soc_L3_LPM_HITBITm_field_set(unit, &h_entry, HITBITSf, &bmval);

    /* Write entry back to hw. */
    rv = BCM_XGS3_MEM_WRITE(unit, L3_LPM_HITBITm, index, &h_entry);
    return (rv);
}

/*
 * Function:
 *      _bcm_er_lpm_hit_get
 * Purpose:
 *      Determine if hitbit at the specified index is set.
 * Parameters:
 *      unit  -  (IN)SOC unit number.
 *      index -  (IN)Index into the hitbit memory.
 *      hit   -  (OUT)TRUE hitbit is set/not set.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_hit_get(int unit, int index, int *hit)
{
    lpm_hitbit_entry_t h_entry; /* Buffer to read from hw.  */
    uint32 bmval;

    /* Read buffer from hw. */
    BCM_IF_ERROR_RETURN(soc_mem_read(unit, L3_LPM_HITBITm, MEM_BLOCK_ANY,
                         (index >> 5), &h_entry));

    soc_L3_LPM_HITBITm_field_get(unit, &h_entry, HITBITSf, &bmval);
    if (bmval & (1 << (index & 0x1f))) {
        *hit = TRUE;
    } else {
        *hit = FALSE;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_er_lpm_ent_init
 * Purpose:
 *      Service routine used to initialize lkup key for lpm entry.
 * Parameters:
 *      unit      - (IN)SOC unit number. 
 *      lpm_cfg   - (IN)Prefix info.
 *      lpm_entry - (OUT) Hw buffer to fill.
 * Returns:
 *      BCM_E_XXX
 */
STATIC INLINE int
_bcm_er_lpm_ent_init(int unit, _bcm_defip_cfg_t *lpm_cfg,
                     defip_alg_entry_t *lpm_entry)
{
    if (lpm_cfg->defip_vrf < 0) {
        return (BCM_E_PARAM);
    }
    /* Reset destination buffer. */
    sal_memset(lpm_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));

    /* Set vrf id. */
    soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, VRFf, lpm_cfg->defip_vrf);

    /* Set prefix ip address. */
    if (lpm_cfg->defip_flags & BCM_L3_IP6) {
        /* Set upper 64 bit of address. */
        soc_mem_ip6_addr_set(unit, L3_DEFIP_ALGm, lpm_entry,
                             IP_ADDR_V6f, lpm_cfg->defip_ip6_addr,
                             SOC_MEM_IP6_UPPER_ONLY);
    } else {
        /* Apply subnet mask. */
        lpm_cfg->defip_ip_addr &=
            BCM_IP4_MASKLEN_TO_ADDR(lpm_cfg->defip_sub_len);

        /* Set ipv4 address. */
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, IP_ADDR_V4f,
                                      lpm_cfg->defip_ip_addr);
        /* Set key type to ipv4 */
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, KEY_TYPEf,
                                      SOC_ER_DEFIP_KEY_TYPE_V4);
        /* ... */
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, FFf,
                                      SOC_ER_DEFIP_FF_FIELD);

        /* ... */
        soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, PADDING_IPV4f, 0);
    }

    /* Set prefix mask. */
    soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, PREFIX_LENGTHf,
                                  lpm_cfg->defip_sub_len);

    /* Set valid bit. */
    soc_L3_DEFIP_ALGm_field32_set(unit, lpm_entry, VALIDf, 1);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_er_lpm_ent_get_key
 * Purpose:
 *      Parse entry key from DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (OUT)Buffer to fill defip information. 
 *      lpm_entry   - (IN) Buffer read from hw. 
 * Returns:
 *      void
 */
STATIC INLINE void
_bcm_er_lpm_ent_get_key(int unit, _bcm_defip_cfg_t *lpm_cfg,
                        defip_alg_entry_t *lpm_entry)
{
    if (lpm_cfg->defip_flags & BCM_L3_IP6) {
        /* Get ipv6 address. */
        soc_mem_ip6_addr_get(unit, L3_DEFIP_ALGm, lpm_entry, IP_ADDR_V6f,
                             lpm_cfg->defip_ip6_addr, SOC_MEM_IP6_UPPER_ONLY);

        /* Get prefix length. */
        lpm_cfg->defip_sub_len =
            soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, PREFIX_LENGTHf);
    } else {
        /* Get ipv4 address. */
        lpm_cfg->defip_ip_addr =
            soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, IP_ADDR_V4f);

        /* Fill mask length. */
        lpm_cfg->defip_sub_len =
            soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, PREFIX_LENGTHf);
    }
    return;
}

/*
 * Function:
 *      _bcm_er_lpm_ent_parse
 * Purpose:
 *      Service routine used to parse hw lpm entry to api format.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (OUT)Buffer to fill defip information. 
 *      nh_ecmp_idx - (OUT)Next hop index or ecmp group id.  
 *      lpm_entry   - (IN) Buffer read from hw. 
 * Returns:
 *      void
 */
STATIC INLINE void
_bcm_er_lpm_ent_parse(int unit, _bcm_defip_cfg_t *lpm_cfg, int *nh_ecmp_idx,
                      defip_alg_entry_t *lpm_entry)
{
    int hitbit;                 /* Entry usage bit.    */
    int ipv6;                   /* Entry protocol.     */
    int tunnel_opt;             /* Tunnel option value */

    ipv6 = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, FFf) != 0xff;

    /* Reset entry flags first. */
    lpm_cfg->defip_flags = 0;

    /* Check if entry points to ecmp group. */
    if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, ECMPf)) {

        /* Mark entry as ecmp */
        lpm_cfg->defip_ecmp = 1;
        lpm_cfg->defip_flags |= BCM_L3_MULTIPATH;

        /* Get ecmp group index. */
        if (nh_ecmp_idx) {
            *nh_ecmp_idx =
                soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, ECMP_INDEXf);
        }
    } else {
        /* Mark entry as non-ecmp. */
        lpm_cfg->defip_ecmp = 0;

        /* Reset ecmp group next hop count. */
        lpm_cfg->defip_ecmp_count = 0;

        /* Get next hop index. */
        if (nh_ecmp_idx) {
            *nh_ecmp_idx =
                soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, NHOP_INDEXf);
        }
    }

    /* Get vrf id. */
    if (!ipv6) {
        lpm_cfg->defip_vrf =
            soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, VRFf);
    } else {
        lpm_cfg->defip_flags |= BCM_L3_IP6;
    }

    /* Get entry priority. */
    lpm_cfg->defip_prio = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, PRIf);

    /*
     * NOTE: When LPM Algorithm table is used, only the hitbits
     * corresponding to the TOP entries are checked (not BOTTOM).
     * During traverse operational cb table index is not available. 
     */
    /* Get hit bit value. */
    if (lpm_cfg->defip_index != BCM_XGS3_L3_INVALID_INDEX) {
        _bcm_er_lpm_hit_get(unit, lpm_cfg->defip_index, &hitbit);
        if (hitbit) {
            lpm_cfg->defip_flags |= BCM_L3_HIT;
        }
    }

    /* Get priority override bit. */
    if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, RPEf)) {
        lpm_cfg->defip_flags |= BCM_L3_RPE;
    }

    /* Get source discard bit. */
    if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, SRC_DISCARDf)) {
        lpm_cfg->defip_flags |= BCM_L3_SRC_DISCARD;
    }

    /* Get destination discard bit. */
    if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, DST_DISCARDf)) {
        lpm_cfg->defip_flags |= BCM_L3_DST_DISCARD;
    }

    /* Label valid bit. */
    tunnel_opt = soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, UDPf);
    if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, VC_LABEL_VALIDf)) {
        lpm_cfg->defip_mpls_label = tunnel_opt;
        lpm_cfg->defip_tunnel_option = 0;
        lpm_cfg->defip_flags |= BCM_L3_ROUTE_LABEL;
    } else {
        lpm_cfg->defip_mpls_label = 0;
        lpm_cfg->defip_tunnel_option = tunnel_opt;
    }
}

/*
 * Function:
 *      _bcm_er_l3_ipmc_ent_parse
 * Purpose:
 *      Service routine used to parse hw l3 ipmc entry to api format.
 * Parameters:
 *      unit      - (IN)SOC unit number. 
 *      l3cfg     - (IN/OUT)l3 entry parsing destination buf.
 *      l3x_entry - (IN/OUT)hw buffer.
 * Returns:
 *      void
 */
STATIC INLINE void
_bcm_er_l3_ipmc_ent_parse(int unit, _bcm_l3_cfg_t *l3cfg,
                          l3_entry_ipv6_multicast_entry_t *l3x_entry)
{
    _bcm_l3_fields_t *fld;             /* IPMC table fields.    */   
    soc_mem_t mem;                     /* IPMC table memory.    */
    uint32 *buf_p;                     /* HW buffer address.    */
    int ipv6;                          /* IPv6 entry indicator. */

    buf_p = (uint32 *)l3x_entry;

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6) :\
                               BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4));

    /* Get table memory. */
    mem =  (ipv6) ?IPMC_GROUP_V6m : IPMC_GROUP_V4m;

    /* Mark entry as multicast & clear rest of the flags. */
    l3cfg->l3c_flags = BCM_L3_IPMC;
    if (ipv6) {
       l3cfg->l3c_flags |= BCM_L3_IP6;
    }

    /* Set ipv6 group address to multicast. */
    if (ipv6) {
        l3cfg->l3c_ip6[0] = 0xff;   /* Set entry ip to mcast address. */
    }

    /* Read priority override */
    if(soc_mem_field32_get(unit, mem, buf_p, fld->rpe)) { 
        l3cfg->l3c_flags |= BCM_L3_RPE;
    }

    /* Read destination discard bit. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->dst_discard)) {
        if(soc_mem_field32_get(unit, mem, buf_p, fld->dst_discard)) {
            l3cfg->l3c_flags |= BCM_L3_DST_DISCARD;
        }
    }

    /* Read Virtual Router Id. */
    if (SOC_MEM_FIELD_VALID(unit, mem, fld->vrf)) {
        l3cfg->l3c_vrf = soc_mem_field32_get(unit, mem, buf_p, fld->vrf);
    } else {
        l3cfg->l3c_vrf = BCM_L3_VRF_DEFAULT;
    }

    /*  */
    l3cfg->l3c_ipmc_ptr =
        soc_mem_field32_get(unit, mem, buf_p, L3_IPMC_INDEXf);

    /* Read priority value. */
    l3cfg->l3c_prio = soc_mem_field32_get(unit, mem, buf_p, fld->priority);

    return;
}

/*
 * Function:
 *      _bcm_er_l3_ipmc_get_by_idx
 * Purpose:
 *      Get l3 multicast entry by entry index.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      dma_ptr  - (IN)Table pointer in dma. 
 *      idx       - (IN)Index to read. 
 *      l3cfg     - (IN/OUT)Entry data.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_er_l3_ipmc_get_by_idx(int unit, void *dma_ptr,
                               int idx, _bcm_l3_cfg_t *l3cfg)
{
    l3_entry_ipv6_multicast_entry_t *l3x_entry_p; /* Read buffer address.    */
    l3_entry_ipv6_multicast_entry_t l3x_entry;    /* Read buffer.            */
    _bcm_l3_fields_t *fld;                        /* IPMC table fields.      */
    soc_mem_t mem;                                /* IPMC table memory.      */
    int ipv6;                                     /* IPv6 entry indicator.   */

    /* Get entry type. */
    ipv6 = (l3cfg->l3c_flags & BCM_L3_IP6);

    /* Get table memory. */
    mem =  (ipv6) ? IPMC_GROUP_V6m : IPMC_GROUP_V4m;

    /* Extract l3 ipv6/4 fields information. */
    fld = (_bcm_l3_fields_t *)((ipv6) ? BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6) :\
                               BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4));

    if (NULL == dma_ptr) {             /* Read from hardware. */
        /* Zero buffers. */
        l3x_entry_p = &l3x_entry;
        sal_memset(l3x_entry_p, 0, BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6));
        /* Read entry from hw. */
        BCM_IF_ERROR_RETURN(BCM_XGS3_MEM_READ(unit, mem, idx, l3x_entry_p));
    } else {                    /* Read from dma. */
        l3x_entry_p =
            soc_mem_table_idx_to_pointer(unit, mem,
                                         l3_entry_ipv6_multicast_entry_t *,
                                         dma_ptr, idx);
    }

    /* Get protocol. */
    if (ipv6){
        l3cfg->l3c_flags = BCM_L3_IP6;
    } else {
        l3cfg->l3c_flags = 0;
    }

    /* Set ipmc flag . */
    l3cfg->l3c_flags |= BCM_L3_IPMC;


    /* Set index to l3cfg. */
    l3cfg->l3c_hw_index = idx;

    if (ipv6) {
        /* Get group address. */
        soc_mem_ip6_addr_get(unit, mem, l3x_entry_p, DIPf,
                             l3cfg->l3c_ip6, SOC_MEM_IP6_FULL_ADDR);

        /* Get source  address. */
        soc_mem_ip6_addr_get(unit, mem, l3x_entry_p, SIPf, 
                             l3cfg->l3c_sip6, SOC_MEM_IP6_FULL_ADDR);

        l3cfg->l3c_ip6[0] = 0xff;    /* Set entry to multicast*/
    } else {
        /* Get group id. */
        l3cfg->l3c_ip_addr =
            soc_mem_field32_get(unit, mem, l3x_entry_p, DIPf);

        /* Get source address. */
        l3cfg->l3c_src_ip_addr =
            soc_mem_field32_get(unit, mem,  l3x_entry_p, SIPf);
    }
    /* Get vlan id. */
    l3cfg->l3c_vid = soc_mem_field32_get(unit, mem,  l3x_entry_p, fld->vlan_id);

    /* Parse entry data. */
    _bcm_er_l3_ipmc_ent_parse(unit, l3cfg, l3x_entry_p);

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_er_lpm_get
 * Purpose:
 *      Get an entry from DEFIP table.
 * Parameters:
 *      unit        - (IN)SOC unit number.
 *      lpm_cfg     - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_get(int unit, _bcm_defip_cfg_t *lpm_cfg, int *nh_ecmp_idx)
{
    defip_alg_entry_t lpm_entry;        /* Search result buffer. */
    defip_alg_entry_t lpm_key;  /* Route lookup key.     */
    int clear_hit;              /* Clear hit indicator.  */
    int valid;

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Zero buffers. */
    sal_memset(&lpm_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));

    /* Check if clear hit bit is required. */
    clear_hit = lpm_cfg->defip_flags & BCM_L3_HIT_CLEAR;

    /* Initialized lookup key. */
    BCM_IF_ERROR_RETURN(_bcm_er_lpm_ent_init(unit, lpm_cfg, &lpm_key));

    /* Perform hw lookup. */
    BCM_IF_ERROR_RETURN
        (soc_er_defip_lookup(unit, &lpm_cfg->defip_index,
                             &lpm_key, &lpm_entry));

    /* Make sure entry is valid one. */
    valid = soc_L3_DEFIP_ALGm_field32_get(unit, &lpm_entry, VALIDf);
    if (!valid) {
        return (BCM_E_NOT_FOUND);
    }

    /* Parse hw buffer to defip entry. */
    _bcm_er_lpm_ent_parse(unit, lpm_cfg, nh_ecmp_idx, &lpm_entry);

    /* Clear the HIT bit */
    if (clear_hit) {
        BCM_IF_ERROR_RETURN(_bcm_er_lpm_hit_set(unit, lpm_cfg, 0));
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_er_lpm_add
 * Purpose:
 *      Add an entry to DEFIP table.
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Buffer to fill defip information. 
 *      nh_ecmp_idx - (IN)Next hop or ecmp group index.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_add(int unit, _bcm_defip_cfg_t *lpm_cfg, int nh_ecmp_idx)
{
    defip_alg_entry_t lpm_entry;        /* Write buffer.            */
    int rv;                     /* Operation return status. */

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Zero buffers. */
    sal_memset(&lpm_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));

    /* Initialize entry. */
    BCM_IF_ERROR_RETURN(_bcm_er_lpm_ent_init(unit, lpm_cfg, &lpm_entry));

    /* Set entry priority. */
    soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, PRIf, lpm_cfg->defip_prio);

    /* Set priority override bit. */
    if (lpm_cfg->defip_flags & BCM_L3_RPE) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, RPEf, 1);
    }

    /* Set source discard flag. */
    if (lpm_cfg->defip_flags & BCM_L3_SRC_DISCARD) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, SRC_DISCARDf, 1);
    }

    /* Set destination discard flag. */
    if (lpm_cfg->defip_flags & BCM_L3_DST_DISCARD) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, DST_DISCARDf, 1);
    }

    /* VC LABEL. */
    if (lpm_cfg->defip_flags & BCM_L3_ROUTE_LABEL) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, VC_LABEL_VALIDf, 1);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry,
                                      UDPf, lpm_cfg->defip_mpls_label);
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry,
                                      UDPf, lpm_cfg->defip_tunnel_option);
    }

    /* Fill next hop information. */
    if (lpm_cfg->defip_flags & BCM_L3_MULTIPATH) {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, ECMP_INDEXf,
                                      nh_ecmp_idx);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, ECMPf, 1);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, ECMP_COUNTf,
                                          lpm_cfg->defip_ecmp_count);
    } else {
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, NHOP_INDEXf,
                                      nh_ecmp_idx);
    }

    /* Write buffer to hw. */
    rv = soc_mem_insert(unit, L3_DEFIP_ALGm, COPYNO_ALL, &lpm_entry);

    /* If new route added increment total number of routes.  */
    if ((rv >= 0) && (BCM_XGS3_L3_INVALID_INDEX == lpm_cfg->defip_index)) {
        BCM_XGS3_L3_DEFIP_CNT_INC(unit, lpm_cfg->defip_flags & BCM_L3_IP6);
    }

    /* Set entry hit bit if required. */
    if (lpm_cfg->defip_flags & BCM_L3_HIT) {
        /* Perform hw lookup. */
        BCM_IF_ERROR_RETURN (soc_er_defip_lookup(unit, &lpm_cfg->defip_index,
                             &lpm_entry, &lpm_entry));

        BCM_IF_ERROR_RETURN(_bcm_er_lpm_hit_set(unit, lpm_cfg, 1));
    }
    return rv;
}

/*
 * Function:
 *      _bcm_er_lpm_del
 * Purpose:
 *      Delete an entry from DEFIP table.
 * Parameters:
 *      unit    - (IN)SOC unit number.
 *      lpm_cfg - (IN)Buffer to fill defip information. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_del(int unit, _bcm_defip_cfg_t *lpm_cfg)
{
    int rv;                     /* Operation return status. */
    defip_alg_entry_t lpm_entry;        /* Search result buffer.    */

    /* Input parameters check */
    if (NULL == lpm_cfg) {
        return (BCM_E_PARAM);
    }

    /* Zero buffers. */
    sal_memset(&lpm_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, defip));

    /* Initialize hw buffer deletion key. */
    BCM_IF_ERROR_RETURN(_bcm_er_lpm_ent_init(unit, lpm_cfg, &lpm_entry));

    /* Write buffer to hw. */
    if (!lpm_cfg->defip_sub_len && 
        (L3_DEFIP_ALGm == BCM_XGS3_L3_MEM(unit, defip))) {
        /* This is a default route, set the DEFAULT_MISS bit */
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, DEFAULT_MISSf, 1);
        soc_L3_DEFIP_ALGm_field32_set(unit, &lpm_entry, NHOP_INDEXf, 0);
        rv = soc_mem_insert(unit, L3_DEFIP_ALGm, COPYNO_ALL, &lpm_entry);
    } else {
        rv = soc_mem_delete(unit, L3_DEFIP_ALGm, COPYNO_ALL, &lpm_entry);
        /* If route deleted decrement total number of routes. */
        if (rv >= 0) {
            BCM_XGS3_L3_DEFIP_CNT_DEC(unit, lpm_cfg->defip_flags & BCM_L3_IP6);
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_er_lpm_protocol_match
 * Purpose:
 *      Verify entry read from HW matches a certain L3 protocol.
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      flags      - (IN)Protocol flags.
 *      lpm_entry  - (IN)HW buffer with route entry.
 * Returns:
 *      BCM_L3_CMP_EQUAL - protocol match. 
 *      BCM_L3_CMP_NOT_EQUAL - No protocol match 
 */
STATIC INLINE int
_bcm_er_lpm_protocol_match(int unit, int flags, defip_alg_entry_t *lpm_entry)
{
    /* Make sure entry is valid. */
    if (!soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, VALIDf) ||
        soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, DEFAULT_MISSf)) {
        return BCM_L3_CMP_NOT_EQUAL;
    }

    if (flags & BCM_L3_IP6) {
        /* Check ipv6 entry. */
        if (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, FFf) == 0xff) {
            return BCM_L3_CMP_NOT_EQUAL;
        }
    } else {
        /* Check ipv4 entry. */
        if ((soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, FFf) != 0xff) ||
            (soc_L3_DEFIP_ALGm_field32_get(unit, lpm_entry, KEY_TYPEf) !=
             SOC_ER_DEFIP_KEY_TYPE_V4)) {
            return BCM_L3_CMP_NOT_EQUAL;
        }
    }
    return BCM_L3_CMP_EQUAL;
}

/*
 * Function:
 *      _bcm_er_lpm_update_opcb
 * Purpose:
 *      Operational callback defip table match a certain rule.
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      lpm_entry  - (IN)HW buffer with route entry.
 *      tbl_idx    - (UNUSED)
 *      cmp_result - (OUT)Comparison result.
 *      cookie     - (IN)Delete pattern + compare,act,notify routines.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_update_opcb(int unit, defip_alg_entry_t *lpm_entry,
                        int tbl_idx, int *cmp_result, void *cookie)
{
    int rv;                            /* Operation return status.     */
    int nh_ecmp_idx;                   /* Next hop/Ecmp group index.   */
    _bcm_defip_cfg_t lpm_cfg;          /* Buffer to fill route info.   */
    _bcm_l3_trvrs_data_t *trv_data;    /* Pattern & callbacks.         */

    /* Get traverse pattern. */
    trv_data = (_bcm_l3_trvrs_data_t *) cookie;

    /* Match the protocol. */
    if (BCM_L3_CMP_EQUAL !=
        _bcm_er_lpm_protocol_match(unit, trv_data->flags, lpm_entry)) {
        return (BCM_E_NONE);
    }

    /* Zero destination buffer first. */
    sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));

    /* Reset table index in the entry. */
    lpm_cfg.defip_index = BCM_XGS3_L3_INVALID_INDEX;

    /* Parse  the entry. */
    _bcm_er_lpm_ent_parse(unit, &lpm_cfg, &nh_ecmp_idx, lpm_entry);

    /* Fill entry ip address &  subnet mask. */
    _bcm_er_lpm_ent_get_key(unit, &lpm_cfg, lpm_entry);

    /* Execute operation routine if any. */
    if (trv_data->op_cb) {
        rv = (*trv_data->op_cb) (unit, trv_data, (void *)&lpm_cfg,
                                 (void *)&nh_ecmp_idx, cmp_result);
        if (rv < 0) {
            return rv;
        }
    }
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_er_lpm_update_testcb
 * Purpose:
 *      Test entry in defip table match a certain rule.
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      lpm_entry  - (IN)HW buffer with route entry.
 *      tbl_idx    - (IN)Entry index in hw table. 
 *      cmp_result - (IN/OUT)Comparison result.
 *      cookie     - (IN)Delete pattern + compare,act,notify routines.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_update_testcb(int unit, defip_alg_entry_t *lpm_entry,
                          int tbl_idx, int *cmp_result, void *cookie)
{
    int rv;                            /* Operation return status.     */
    int nh_ecmp_idx;                   /* Next hop/Ecmp group index.   */
    _bcm_defip_cfg_t lpm_cfg;          /* Buffer to fill route info.   */
    _bcm_l3_trvrs_data_t *trv_data;    /* Pattern & callbacks.         */

    /* Get traverse pattern. */
    trv_data = (_bcm_l3_trvrs_data_t *) cookie;

    /* Match the protocol. */
    if (BCM_L3_CMP_EQUAL !=
        _bcm_er_lpm_protocol_match(unit, trv_data->flags, lpm_entry)) {
        *cmp_result = BCM_L3_CMP_NOT_EQUAL;
        return (BCM_E_NONE);
    }

    /* Zero destination buffer first. */
    sal_memset(&lpm_cfg, 0, sizeof(_bcm_defip_cfg_t));

    /* Set table index to the entry. */
    lpm_cfg.defip_index = tbl_idx;

    /* Parse  the entry. */
    _bcm_er_lpm_ent_parse(unit, &lpm_cfg, &nh_ecmp_idx, lpm_entry);

    /* Fill entry ip address &  subnet mask. */
    _bcm_er_lpm_ent_get_key(unit, &lpm_cfg, lpm_entry);

    /* Execute test routine if any. */
    if (trv_data->cmp_cb) {
        rv = (*trv_data->cmp_cb) (unit, trv_data, (void *)&lpm_cfg,
                                  (void *)&nh_ecmp_idx, cmp_result);
        if ((rv < 0) || (*cmp_result == BCM_L3_CMP_NOT_EQUAL)) {
            *cmp_result = BCM_L3_CMP_NOT_EQUAL;
            return rv;
        }
    }
    /* No comparison function  assume match. */
    *cmp_result = BCM_L3_CMP_EQUAL;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_er_lpm_update_match
 * Purpose:
 *      Update/Delete all entries in defip table matching a certain rule.
 * Parameters:
 *      unit     - (IN)SOC unit number.
 *      trv_data - (IN)Delete pattern + compare,act,notify routines.  
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_er_lpm_update_match(int unit, _bcm_l3_trvrs_data_t *trv_data)
{
    /* We don't need to lock the table since soc_er_defip_traverse does it. */
    return soc_er_defip_traverse(unit, _bcm_er_lpm_update_testcb,
                                 _bcm_er_lpm_update_opcb, trv_data, trv_data);
}

/*
 * Function:
 *      _bcm_er_adj_mac_get
 * Purpose:
 *      Read adjacent mac entry from hw. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      idx       - (IN)Adjacent mac entry index .
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_er_adj_mac_get(int unit, int idx, bcm_mac_t mac)
{
    egress_adjacent_mac_entry_t amac_entry;     /* Buffer to write to hw.  */
    int rv;                     /* Operation return value. */

    /* Zero buffer. */
    sal_memset(&amac_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, adj_mac));

    rv = READ_EGRESS_ADJACENT_MACm(unit, MEM_BLOCK_ALL, idx, &amac_entry);
    BCM_IF_ERROR_RETURN(rv);

    soc_EGRESS_ADJACENT_MACm_mac_addr_get(unit, &amac_entry, MAC_ADDRf, mac);
    return rv;
}

/*
 * Function:
 *      _bcm_er_adj_mac_add
 * Purpose:
 *      Add adjacent mac entry in hw. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      idx       - (IN)Allocated entry index .
 *      buf       - (IN)Adjacent mac address.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_er_adj_mac_add(int unit, int idx, void *buf, int max_paths)
{

    egress_adjacent_mac_entry_t amac_entry;     /* Buffer to write to hw.  */
    int rv;                     /* Operation return value. */

    /* Zero buffer. */
    sal_memset(&amac_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, adj_mac));

    /* Set mac address. */
    soc_EGRESS_ADJACENT_MACm_mac_addr_set(unit, &amac_entry, MAC_ADDRf, buf);

    /* Write mac address buffer to hw. */
    rv = WRITE_EGRESS_ADJACENT_MACm(unit, MEM_BLOCK_ALL, idx, &amac_entry);
    return rv;
}

/*
 * Function:
 *      _bcm_er_adj_mac_del
 * Purpose:
 *      Delete adjacent mac entry from hw. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      idx       - (IN)Adjacent mac entry index .
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_er_adj_mac_del(int unit, int idx, int max_paths)
{
    egress_adjacent_mac_entry_t amac_entry;     /* Buffer to write to hw.  */
    int rv;                     /* Operation return value. */

    /* Zero buffer. */
    sal_memset(&amac_entry, 0, BCM_XGS3_L3_ENT_SZ(unit, adj_mac));

    /* Write empty buffer to hw. */
    rv = WRITE_EGRESS_ADJACENT_MACm(unit, MEM_BLOCK_ALL, idx, &amac_entry);
    return rv;
}

#endif /* BCM_EASYRIDER_SUPPORT */
/*
 * Function:
 *      _bcm_xgs3_l3_hw_call_init
 * Purpose:
 *      Initialize L3 hardware operations memories & callbacks.
 * Parameters:
 *      unit      - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_hw_call_init(int unit)
{
#ifdef BCM_EASYRIDER_SUPPORT
    static _bcm_l3_hw_calls_t     er_hw_call;        /* Easyrider hw calls.   */
    static _bcm_l3_intf_fields_t  er_l3_intf_fields; /* L3 intf fields.       */
    static _bcm_l3_nh_fields_t    er_nh_fields;      /* NH table fields.      */
    static _bcm_l3_fields_t       er_l3_fields;      /* L3 table fields. */
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    static _bcm_l3_hw_calls_t     fb_hw_call;        /* FBX hw calls .        */
    static _bcm_l3_intf_fields_t  fb_l3_intf_fields; /* L3 intf fields.       */
    static _bcm_l3_nh_fields_t    fb_nh_fields;      /* NH table fields.      */
    static _bcm_l3_fields_t       fb_l3_v4_fields;   /* L3 ipv4 table fields. */
    static _bcm_l3_fields_t       fb_l3_v6_fields;   /* L3 ipv6 table fields. */
#endif /* BCM_FIREBOLT_SUPPORT */      
#if defined (BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    static _bcm_l3_hw_calls_t    rp_hw_call;        /* Raptor/Raven hw calls. */
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
    static _bcm_l3_hw_calls_t     tr_hw_call;       /* Triumph hw calls. */
    static _bcm_l3_fields_t       tr_l3_v6_fields;  /* Truimph l3 ipv6 fields.*/
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
    static _bcm_l3_nh_fields_t    trx_nh_fields;     /* NH table fields.      */
#endif /* BCM_TRX_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
    static _bcm_l3_hw_calls_t     td_hw_call;       /* Trident hw calls. */
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
    static _bcm_l3_hw_calls_t     tr2_hw_call;      /* Triumph2 hw calls. */
#endif /* BCM_TRIUMPH2_SUPPORT */
    static int hw_call_initialized = FALSE;
    _bcm_l3_hw_calls_t     tmp_hw_call;              /* Common hw calls.      */

    if (!hw_call_initialized) {
        /* Hw calls pointers. */
        sal_memset(&tmp_hw_call, 0, sizeof(_bcm_l3_hw_calls_t));
        tmp_hw_call.l3_clear_all      = _bcm_xgs3_l3_clear_all;
        tmp_hw_call.if_get            = _bcm_xgs3_l3_intf_get;
        tmp_hw_call.if_add            = _bcm_xgs3_l3_intf_add;
        tmp_hw_call.if_del            = _bcm_xgs3_l3_intf_del;
        tmp_hw_call.if_tnl_init_get   = _bcm_xgs3_l3_intf_tnl_init_get;
        tmp_hw_call.if_tnl_init_set   = _bcm_xgs3_l3_intf_tnl_init_set;
        tmp_hw_call.if_tnl_init_reset = _bcm_xgs3_l3_intf_tnl_init_reset;
        tmp_hw_call.nh_get            = _bcm_xgs3_nh_get;
        tmp_hw_call.nh_del            = _bcm_xgs3_nh_del;
        tmp_hw_call.nh_update_match   = _bcm_xgs3_nh_update_match;
        tmp_hw_call.tnl_term_get      = _bcm_xgs3_l3_tnl_term_get;
        tmp_hw_call.tnl_term_add      = _bcm_xgs3_l3_tnl_term_add;
        tmp_hw_call.tnl_term_del      = _bcm_xgs3_l3_tnl_term_del;
        tmp_hw_call.tnl_init_get      = _bcm_xgs3_l3_tnl_init_get;
        tmp_hw_call.tnl_init_add      = _bcm_xgs3_l3_tnl_init_add;
        tmp_hw_call.tnl_init_del      = _bcm_xgs3_l3_tnl_init_del;
        tmp_hw_call.tnl_dscp_get      = _bcm_xgs3_l3_tnl_dscp_get;
        tmp_hw_call.tnl_dscp_set      = _bcm_xgs3_l3_tnl_dscp_set;

#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
        /* Hw calls pointers. */
        sal_memcpy(&rp_hw_call, &tmp_hw_call, sizeof(_bcm_l3_hw_calls_t));
        rp_hw_call.nh_add            = _bcm_fb_nh_add;
        rp_hw_call.l3_get            = _bcm_rp_l3_get;
        rp_hw_call.l3_add            = _bcm_rp_l3_add;
        rp_hw_call.l3_del            = _bcm_rp_l3_del;
        rp_hw_call.l3_get_by_idx     = _bcm_rp_l3_get_by_idx;
        rp_hw_call.l3_traverse       = _bcm_xgs3_l3_traverse;
        rp_hw_call.l3_del_match      = _bcm_xgs3_l3_del_match;
        rp_hw_call.lpm_get           = _bcm_rp_lpm_get;
        rp_hw_call.lpm_add           = _bcm_rp_lpm_add;
        rp_hw_call.lpm_del           = _bcm_rp_lpm_del;
        rp_hw_call.lpm_update_match  = _bcm_rp_lpm_update_match;
        if (soc_feature(unit, soc_feature_fp_routing_mirage)) {
            rp_hw_call.ecmp_grp_get      = _bcm_xgs3_ecmp_grp_get;
            rp_hw_call.ecmp_grp_add      = _bcm_xgs3_ecmp_grp_add;
            rp_hw_call.ecmp_grp_del      = _bcm_xgs3_ecmp_grp_del;
            rp_hw_call.max_grp_size_set  = _bcm_xgs3_ecmp_max_grp_size_set;
        }
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
        /* Hw calls pointers. */
        sal_memcpy(&fb_hw_call, &tmp_hw_call, sizeof(_bcm_l3_hw_calls_t));
        fb_hw_call.nh_add            = _bcm_fb_nh_add;
        fb_hw_call.ecmp_grp_get      = _bcm_xgs3_ecmp_grp_get;
        fb_hw_call.ecmp_grp_add      = _bcm_xgs3_ecmp_grp_add;
        fb_hw_call.ecmp_grp_del      = _bcm_xgs3_ecmp_grp_del;
        fb_hw_call.max_grp_size_set  = _bcm_xgs3_ecmp_max_grp_size_set;
        fb_hw_call.l3_get            = _bcm_xgs3_l3_get;
        fb_hw_call.l3_add            = _bcm_xgs3_l3_add;
        fb_hw_call.l3_del            = _bcm_xgs3_l3_del;
        fb_hw_call.l3_get_by_idx     = _bcm_xgs3_l3_get_by_idx;
        fb_hw_call.l3_traverse       = _bcm_xgs3_l3_traverse;
        fb_hw_call.l3_del_match      = _bcm_xgs3_l3_del_match;
        fb_hw_call.l3_bucket_get     = _bcm_xgs3_l3_bucket_get;
        fb_hw_call.ipmc_get          = _bcm_fb_l3_ipmc_get;
        fb_hw_call.ipmc_add          = _bcm_fb_l3_ipmc_add;
        fb_hw_call.ipmc_del          = _bcm_fb_l3_ipmc_del;
        fb_hw_call.ipmc_get_by_idx   = _bcm_fb_l3_ipmc_get_by_idx;
        fb_hw_call.lpm_get           = _bcm_fbx_lpm_get;
        fb_hw_call.lpm_add           = _bcm_fbx_lpm_add;
        fb_hw_call.lpm_del           = _bcm_fbx_lpm_del;
        fb_hw_call.lpm_update_match  = _bcm_fbx_lpm_update_match;
        if(!SOC_IS_HURRICANE(unit)) {
           fb_hw_call.if_vrf_get        = _bcm_fb_l3_intf_vrf_get;
           fb_hw_call.if_vrf_bind       = _bcm_fb_l3_intf_vrf_bind;
        }
#if defined(BCM_TRIUMPH_SUPPORT)
        sal_memcpy(&tr_hw_call, &fb_hw_call, sizeof(_bcm_l3_hw_calls_t));
        tr_hw_call.ipmc_get          = _bcm_tr_l3_ipmc_get;
        tr_hw_call.ipmc_add          = _bcm_tr_l3_ipmc_add;
        tr_hw_call.ipmc_del          = _bcm_tr_l3_ipmc_del;
        tr_hw_call.ipmc_get_by_idx   = _bcm_tr_l3_ipmc_get_by_idx;
        tr_hw_call.ing_intf_add      = _bcm_tr_l3_ingress_interface_set;
        tr_hw_call.ing_intf_del      = _bcm_tr_l3_ingress_interface_clr;
        tr_hw_call.ing_intf_get      = _bcm_tr_l3_ingress_interface_get;
        tr_hw_call.ing_intf_update_match = _bcm_xgs3_ing_intf_update_match;
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
        sal_memcpy(&tr2_hw_call, &tr_hw_call, sizeof(_bcm_l3_hw_calls_t));
        tr2_hw_call.ecmp_grp_get  = _bcm_tr2_l3_ecmp_grp_get;
        tr2_hw_call.ecmp_grp_add  = _bcm_tr2_l3_ecmp_grp_add;
        tr2_hw_call.ecmp_grp_del  = _bcm_tr2_l3_ecmp_grp_del;
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
        sal_memcpy(&td_hw_call, &tr_hw_call, sizeof(_bcm_l3_hw_calls_t));
        td_hw_call.ecmp_grp_get  = _bcm_td_l3_ecmp_grp_get;
        td_hw_call.ecmp_grp_add  = _bcm_td_l3_ecmp_grp_add;
        td_hw_call.ecmp_grp_del  = _bcm_td_l3_ecmp_grp_del;
#endif /* BCM_TRIDENT_SUPPORT */
        /* Interface table common fields. */
        fb_l3_intf_fields.vlan_id    = VIDf;
        fb_l3_intf_fields.mac_addr   = MAC_ADDRESSf;
        fb_l3_intf_fields.ttl        = TTL_THRESHOLDf;
        fb_l3_intf_fields.tnl_id     = TUNNEL_INDEXf;
        fb_l3_intf_fields.l2_switch  = L2_SWITCHf;

        /* Next hop table common fields. */
        fb_nh_fields.mac_addr        = MAC_ADDRESSf;  
        fb_nh_fields.module          = MODULE_IDf;  
        fb_nh_fields.port_tgid       = PORT_TGIDf;  
        fb_nh_fields.ifindex         = INTF_NUMf;  

        /* L3 ipv4 table common fields. */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        fb_l3_v4_fields.vrf          = VRF_IDf;
#else /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
        fb_l3_v4_fields.vrf          = INVALIDf;
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
        fb_l3_v4_fields.hit          = HITf; 
        fb_l3_v4_fields.rpe          = RPEf;
        fb_l3_v4_fields.dst_discard  = DST_DISCARDf;
        fb_l3_v4_fields.priority     = PRIf;
        fb_l3_v4_fields.nh_idx       = NEXT_HOP_INDEXf;
        fb_l3_v4_fields.valid        = VALIDf;
        fb_l3_v4_fields.l3mc_index   = L3MC_INDEXf;
        fb_l3_v4_fields.v6_entry     = V6f;
        fb_l3_v4_fields.ipmc_entry   = IPMCf;
        fb_l3_v4_fields.vlan_id      = VLAN_IDf;
#if defined(BCM_TRX_SUPPORT)
        fb_l3_v4_fields.class_id     = CLASS_IDf;
#endif /* BCM_TRX_SUPPORT */

        /* L3 ipv6 table common fields. */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_SCORPION_SUPPORT)
        fb_l3_v6_fields.vrf          = VRF_ID_0f;
#else /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_SCORPION_SUPPORT*/
        fb_l3_v6_fields.vrf          = INVALIDf;
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_SCORPION_SUPPORT*/
        fb_l3_v6_fields.hit          = HIT_0f; 
        fb_l3_v6_fields.rpe          = RPE_0f;
        fb_l3_v6_fields.dst_discard  = DST_DISCARD_0f;
        fb_l3_v6_fields.priority     = PRI_0f;
        fb_l3_v6_fields.nh_idx       = NEXT_HOP_INDEX_0f;
        fb_l3_v6_fields.valid        = VALID_0f;
        fb_l3_v6_fields.l3mc_index   = L3MC_INDEX_0f;
        fb_l3_v6_fields.v6_entry     = V6_0f;
        fb_l3_v6_fields.ipmc_entry   = IPMC_0f;
        fb_l3_v6_fields.vlan_id      = VLAN_ID_0f;
#if defined(BCM_SCORPION_SUPPORT)
        fb_l3_v6_fields.class_id     = CLASS_ID_0f;
#endif /* BCM_SCORPION_SUPPORT */

#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
        /* Hw calls pointers. */
        sal_memcpy(&er_hw_call, &tmp_hw_call, sizeof(_bcm_l3_hw_calls_t));
        er_hw_call.nh_add            = _bcm_er_nh_add;
        er_hw_call.ecmp_grp_get      = _bcm_xgs3_ecmp_grp_get;
        er_hw_call.ecmp_grp_add      = _bcm_xgs3_ecmp_grp_add;
        er_hw_call.ecmp_grp_del      = _bcm_xgs3_ecmp_grp_del;
        er_hw_call.max_grp_size_set  = _bcm_xgs3_ecmp_max_grp_size_set;
        er_hw_call.l3_get            = _bcm_xgs3_l3_get;
        er_hw_call.l3_add            = _bcm_xgs3_l3_add;
        er_hw_call.l3_del            = _bcm_xgs3_l3_del;
        er_hw_call.l3_get_by_idx     = _bcm_xgs3_l3_get_by_idx;
        er_hw_call.l3_traverse       = _bcm_xgs3_l3_traverse;
        er_hw_call.l3_del_match      = _bcm_xgs3_l3_del_match;
        er_hw_call.l3_bucket_get     = _bcm_xgs3_l3_bucket_get;
        er_hw_call.ipmc_get_by_idx   = _bcm_er_l3_ipmc_get_by_idx;	
        er_hw_call.lpm_get           = _bcm_er_lpm_get;
        er_hw_call.lpm_add           = _bcm_er_lpm_add;
        er_hw_call.lpm_del           = _bcm_er_lpm_del;
        er_hw_call.lpm_update_match  = _bcm_er_lpm_update_match;
        er_hw_call.if_vrf_get        = _bcm_er_l3_intf_vrf_get;
        er_hw_call.if_vrf_bind       = _bcm_er_l3_intf_vrf_bind;
        er_hw_call.if_vrf_unbind     = _bcm_er_l3_intf_vrf_unbind;
        er_hw_call.if_group_set      = _bcm_er_l3_intf_group_set;
        er_hw_call.if_group_get      = _bcm_er_l3_intf_group_get;
        er_hw_call.adj_mac_get       = _bcm_er_adj_mac_get;
        er_hw_call.adj_mac_add       = _bcm_er_adj_mac_add;
        er_hw_call.adj_mac_del       = _bcm_er_adj_mac_del;

        /* Interface table common fields. */
        er_l3_intf_fields.vlan_id    = VLAN_IDf;
        er_l3_intf_fields.mac_addr   = MAC_ADDRf;
        er_l3_intf_fields.ttl        = TTL_THRESHOLDf;
        er_l3_intf_fields.tnl_id     = TUNNEL_INDEXf;
        er_l3_intf_fields.l2_switch  = L2_SWITCHf;

        /* Next hop table common fields. */
        er_nh_fields.mac_addr        = MAC_ADDRf;  
        er_nh_fields.module          = MODIDf;
        er_nh_fields.port_tgid       = PORT_TGIDf;  
        er_nh_fields.ifindex         = L3_INTF_NUMf;  

        /* L3 ipv4 table common fields. */
        er_l3_fields.vrf          = VRFf;
        er_l3_fields.hit          = HITf;
        er_l3_fields.rpe          = RPEf;
        er_l3_fields.dst_discard  = DST_DISCARDf;
        er_l3_fields.priority     = PRIf;
        er_l3_fields.nh_idx       = NHOP_INDXf;
        er_l3_fields.valid        = VALIDf;
        er_l3_fields.vlan_id	  = VLANf;
#endif /* BCM_EASYRIDER_SUPPORT */
        /* Next hop table common fields. */
#ifdef BCM_TRX_SUPPORT
        trx_nh_fields.mac_addr        = MAC_ADDRESSf;  
        trx_nh_fields.module          = MODULE_IDf;  
        trx_nh_fields.port_tgid       = PORT_NUMf;  
        trx_nh_fields.ifindex         = INTF_NUMf;  
#endif /* BCM_TRX_SUPPORT */ 

#if defined(BCM_TRIUMPH_SUPPORT) 
        /* L3 ipv6 table fields. */
        tr_l3_v6_fields.vrf          = VRF_IDf;
        tr_l3_v6_fields.hit          = HIT_0f; 
        tr_l3_v6_fields.rpe          = RPEf;
        tr_l3_v6_fields.dst_discard  = DST_DISCARDf;
        tr_l3_v6_fields.priority     = PRIf;
        tr_l3_v6_fields.nh_idx       = NEXT_HOP_INDEXf;
        tr_l3_v6_fields.valid        = VALID_0f;
        tr_l3_v6_fields.l3mc_index   = INVALIDf;
        tr_l3_v6_fields.v6_entry     = V6_0f;
        tr_l3_v6_fields.ipmc_entry   = IPMC_0f;
        tr_l3_v6_fields.vlan_id      = INVALIDf;
        tr_l3_v6_fields.class_id     = CLASS_IDf;
#endif /* BCM_TRIUMPH_SUPPORT */


        hw_call_initialized = TRUE;
    }
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        BCM_XGS3_L3_HW_CALL(unit)          = &er_hw_call;
        BCM_XGS3_L3_MEM_FIELDS(unit, intf) = (void *)&er_l3_intf_fields;
        BCM_XGS3_L3_MEM_FIELDS(unit, nh)   = (void *)&er_nh_fields;
        BCM_XGS3_L3_MEM_FIELDS(unit, v4)   = (void *)&er_l3_fields;
        BCM_XGS3_L3_MEM_FIELDS(unit, v6)   = (void *)&er_l3_fields;
        BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4)   = (void *)&er_l3_fields;
        BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6)   = (void *)&er_l3_fields;	
    }
#endif /* BCM_EASYRIDER_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_MIRAGE_SUPPORT) || \
    defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_fp_based_routing)) {
            BCM_XGS3_L3_HW_CALL(unit)          = &rp_hw_call;
            BCM_XGS3_L3_MEM_FIELDS(unit, intf) = (void *)&fb_l3_intf_fields;
            BCM_XGS3_L3_MEM_FIELDS(unit, nh)   = (void *)&fb_nh_fields;
        } else
#endif /* BCM_RAPTOR_SUPPORT || BCM_MIRAGE_SUPPORT || BCM_HAWKEYE_SUPPORT */

#ifdef BCM_TRX_SUPPORT
            if (SOC_IS_TRX(unit)) {
#ifdef BCM_TRIUMPH2_SUPPORT
                if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                    SOC_IS_VALKYRIE2(unit)){
                      BCM_XGS3_L3_HW_CALL(unit)          = &tr2_hw_call;
                      BCM_XGS3_L3_MEM_FIELDS(unit, v6)   = (void *)&tr_l3_v6_fields;
                } else
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_TRIDENT_SUPPORT
                if (SOC_IS_TRIDENT(unit) || SOC_IS_KATANA(unit)) {
                      BCM_XGS3_L3_HW_CALL(unit)          = &td_hw_call;
                      BCM_XGS3_L3_MEM_FIELDS(unit, v6)   = (void *)&tr_l3_v6_fields;
                } else
#endif
#ifdef BCM_TRIUMPH_SUPPORT
                if (SOC_IS_TR_VL(unit)) {
                    BCM_XGS3_L3_HW_CALL(unit)          = &tr_hw_call;
                    BCM_XGS3_L3_MEM_FIELDS(unit, v6)   = (void *)&tr_l3_v6_fields;
                } else 
#endif /* BCM_TRIUMPH_SUPPORT */
                {
                    BCM_XGS3_L3_HW_CALL(unit)          = &fb_hw_call;
                    BCM_XGS3_L3_MEM_FIELDS(unit, v6)   = (void *)&fb_l3_v6_fields;
                }
                BCM_XGS3_L3_MEM_FIELDS(unit, intf) = (void *)&fb_l3_intf_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, v4)   = (void *)&fb_l3_v4_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4) = (void *)&fb_l3_v4_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6) = (void *)&fb_l3_v6_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, nh)   = (void *)&trx_nh_fields;
            }  else 
#endif /* BCM_TRX_SUPPORT */
            {
                BCM_XGS3_L3_HW_CALL(unit)          = &fb_hw_call;
                BCM_XGS3_L3_MEM_FIELDS(unit, nh)   = (void *)&fb_nh_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, v6)   = (void *)&fb_l3_v6_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v6)   = (void *)&fb_l3_v6_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, intf) = (void *)&fb_l3_intf_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, v4)   = (void *)&fb_l3_v4_fields;
                BCM_XGS3_L3_MEM_FIELDS(unit, ipmc_v4)   = (void *)&fb_l3_v4_fields;
            }
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_xgs3_l3_hw_op_init
 * Purpose:
 *      Initialize L3 hardware operations memories & callbacks.
 * Parameters:
 *      unit - SOC unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_xgs3_l3_hw_op_init(int unit)
{
    /* Identical memories. */
    /* Ecmp table. */
    if (SOC_MEM_IS_VALID(unit, L3_ECMPm)) {
        BCM_XGS3_L3_MEM(unit, ecmp) = L3_ECMPm;
        BCM_XGS3_L3_ENT_SZ(unit, ecmp) = 
             BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, ecmp));
    }  else {
        BCM_XGS3_L3_MEM(unit, ecmp) = INVALIDm;
    }

    BCM_XGS3_L3_MEM(unit, ing_intf) = INVALIDm;

    /* Tunnel terminator table. */
    if (SOC_MEM_IS_VALID(unit, L3_TUNNELm)) {
        BCM_XGS3_L3_MEM(unit, tnl_term) = L3_TUNNELm;
        BCM_XGS3_L3_ENT_SZ(unit, tnl_term) = 
                 BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, tnl_term));
    } else {
        BCM_XGS3_L3_MEM(unit, tnl_term) = INVALIDm;
    }

    /* Chip specific memories. */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* L3 interface table. */
        BCM_XGS3_L3_MEM(unit, intf) = L3INTFm;
        BCM_XGS3_L3_ENT_SZ(unit, intf) = 
             BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, intf));

        /* L3 ipv4 table. */
        if(soc_mem_index_count(unit, L3_ENTRY_V4m)) {
            BCM_XGS3_L3_MEM(unit, v4) = L3_ENTRY_V4m;
            BCM_XGS3_L3_ENT_SZ(unit, v4) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, v4));
        }  else {
            BCM_XGS3_L3_MEM(unit, v4) = INVALIDm;
        }

        /* L3 ipv6 table. */
        if(soc_mem_index_count(unit, L3_ENTRY_V6m)) {
            BCM_XGS3_L3_MEM(unit, v6) = L3_ENTRY_V6m;
            BCM_XGS3_L3_ENT_SZ(unit, v6) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, v6));
        }  else {
            BCM_XGS3_L3_MEM(unit, v6) = INVALIDm;
        }

        /* L3 ipv4 ipmc table. */
        if(soc_mem_index_count(unit, IPMC_GROUP_V4m)) {
            BCM_XGS3_L3_MEM(unit, ipmc_v4) = IPMC_GROUP_V4m;
            BCM_XGS3_L3_ENT_SZ(unit, ipmc_v4) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, ipmc_v4));
        }  else {
            BCM_XGS3_L3_MEM(unit, ipmc_v4) = INVALIDm;
        }

        /* L3 ipv6 ipmc table. */
        if(soc_mem_index_count(unit, IPMC_GROUP_V6m)) {
            BCM_XGS3_L3_MEM(unit, ipmc_v6) = IPMC_GROUP_V6m;
            BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, ipmc_v6));
        }  else {
            BCM_XGS3_L3_MEM(unit, ipmc_v6) = INVALIDm;
        }

        /* L3 adjacent MAC table. */
        BCM_XGS3_L3_MEM(unit, adj_mac) = EGRESS_ADJACENT_MACm;
        BCM_XGS3_L3_ENT_SZ(unit, adj_mac) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, adj_mac));

        /* Defip table. */
        if (soc_mem_index_count(unit, L3_DEFIP_CAMm) > 0) {
            BCM_XGS3_L3_MEM(unit, defip) = L3_DEFIP_CAMm;
        } else {
            BCM_XGS3_L3_MEM(unit, defip) = L3_DEFIP_ALGm;
        }
        BCM_XGS3_L3_ENT_SZ(unit, defip) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, defip));

        /* IPv6 prefix map table. */
        BCM_XGS3_L3_MEM(unit, v6_prefix_map) = INVALIDm;

        /* Tunnel initiator table. */
        BCM_XGS3_L3_MEM(unit, tnl_init_v4) = EGRESS_IP_TUNNELm;
        BCM_XGS3_L3_ENT_SZ(unit, tnl_init_v4) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, tnl_init_v4));

        /* Next hop table. */
        if (SOC_MEM_IS_VALID(unit, NEXT_HOP_EXTm) &&
            (soc_mem_index_count(unit, NEXT_HOP_EXTm) > 0)) {
            BCM_XGS3_L3_MEM(unit, nh) = NEXT_HOP_EXTm;
        } else if (SOC_MEM_IS_VALID(unit, NEXT_HOP_INTm) &&
                   (soc_mem_index_count(unit, NEXT_HOP_INTm) > 0)) {
            BCM_XGS3_L3_MEM(unit, nh) = NEXT_HOP_INTm;
        } else {
            return (BCM_E_INTERNAL);
        }
        BCM_XGS3_L3_ENT_SZ(unit, nh) = 
             BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, nh));

        /* Egress Tunnel DSCP table. */
        BCM_XGS3_L3_MEM(unit, tnl_dscp) = EGRESS_DSCP_EXPm;
        BCM_XGS3_L3_ENT_SZ(unit, tnl_dscp) = 
            BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, tnl_dscp));
    }
#endif /* BCM_EASYRIDER_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        /* L3 interface table. */
        BCM_XGS3_L3_MEM(unit, intf) = EGR_L3_INTFm;
        BCM_XGS3_L3_ENT_SZ(unit, intf) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, intf));

#if defined(BCM_TRIUMPH_SUPPORT) 
        /* L3 ingress interface table. */
        if (SOC_MEM_IS_VALID(unit, L3_IIFm)) { 
            BCM_XGS3_L3_MEM(unit, ing_intf) = L3_IIFm;
            BCM_XGS3_L3_ENT_SZ(unit, ing_intf) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, ing_intf));
        } else {
                BCM_XGS3_L3_MEM(unit, ing_intf) = INVALIDm;
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */  

        /* L3 ipv4 table. */
        if (SOC_MEM_IS_VALID(unit, L3_ENTRY_IPV4_UNICASTm)) {
            BCM_XGS3_L3_MEM(unit, v4) = L3_ENTRY_IPV4_UNICASTm;
            BCM_XGS3_L3_ENT_SZ(unit, v4) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, v4));
        }
        else {
            BCM_XGS3_L3_MEM(unit, v4) = INVALIDm;
        }

        /* L3 ipv6 table. */
        if (SOC_MEM_IS_VALID(unit, L3_ENTRY_IPV6_UNICASTm)) { 
            BCM_XGS3_L3_MEM(unit, v6) = L3_ENTRY_IPV6_UNICASTm;
            BCM_XGS3_L3_ENT_SZ(unit, v6) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, v6));
        }  else {
            BCM_XGS3_L3_MEM(unit, v6) = INVALIDm;
        }

        /* L3 ipv4 ipmc table. */
        if (SOC_MEM_IS_VALID(unit, L3_ENTRY_IPV4_MULTICASTm)) { 
            BCM_XGS3_L3_MEM(unit, ipmc_v4) = L3_ENTRY_IPV4_MULTICASTm;
            BCM_XGS3_L3_ENT_SZ(unit, ipmc_v4) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, ipmc_v4));
        }  else {
            BCM_XGS3_L3_MEM(unit, ipmc_v4) = INVALIDm;
        }

        /* L3 ipv6 table. */
        if (SOC_MEM_IS_VALID(unit, L3_ENTRY_IPV6_MULTICASTm)) { 
            BCM_XGS3_L3_MEM(unit, ipmc_v6) = L3_ENTRY_IPV6_MULTICASTm;
            BCM_XGS3_L3_ENT_SZ(unit, ipmc_v6) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, ipmc_v6));
        }  else {
            BCM_XGS3_L3_MEM(unit, ipmc_v6) = INVALIDm;
        }

        /* Defip table. */
        if (SOC_MEM_IS_VALID(unit, L3_DEFIPm)) { 
            BCM_XGS3_L3_MEM(unit, defip) = L3_DEFIPm;
            BCM_XGS3_L3_ENT_SZ(unit, defip) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, defip));
        }  else {
            BCM_XGS3_L3_MEM(unit, defip) = INVALIDm;
        }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        /* IPv6 prefix map table. */
        if (SOC_MEM_IS_VALID(unit, IPV4_IN_IPV6_PREFIX_MATCH_TABLEm)) { 
            BCM_XGS3_L3_MEM(unit, v6_prefix_map) =
                IPV4_IN_IPV6_PREFIX_MATCH_TABLEm;
            BCM_XGS3_L3_ENT_SZ(unit, v6_prefix_map) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, v6_prefix_map));
        }  else 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */  
        {
            BCM_XGS3_L3_MEM(unit, v6_prefix_map) = INVALIDm;
        }

        /* IPv4 Tunnel initiator table. */
        if (SOC_MEM_IS_VALID(unit, EGR_IP_TUNNELm)) { 
            BCM_XGS3_L3_MEM(unit, tnl_init_v4) = EGR_IP_TUNNELm;
            BCM_XGS3_L3_ENT_SZ(unit, tnl_init_v4) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, tnl_init_v4));
        }  else {
            BCM_XGS3_L3_MEM(unit, tnl_init_v4) = INVALIDm;
        }
#if defined(BCM_TRX_SUPPORT)
        /* IPv6 Tunnel initiator table. */
        if (SOC_MEM_IS_VALID(unit, EGR_IP_TUNNEL_IPV6m)) { 
            BCM_XGS3_L3_MEM(unit, tnl_init_v6) = EGR_IP_TUNNEL_IPV6m;
            BCM_XGS3_L3_ENT_SZ(unit, tnl_init_v6) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, tnl_init_v6));
        }  else 
#endif /* BCM_TRX_SUPPORT */
        {
            BCM_XGS3_L3_MEM(unit, tnl_init_v6) = INVALIDm;
        }

#if defined(BCM_TRX_SUPPORT)
        /* MPLS Tunnel initiator table. */
        if (SOC_MEM_IS_VALID(unit, EGR_IP_TUNNEL_MPLSm)) { 
            BCM_XGS3_L3_MEM(unit, tnl_init_mpls) = EGR_IP_TUNNEL_MPLSm;
            BCM_XGS3_L3_ENT_SZ(unit, tnl_init_mpls) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, tnl_init_mpls));
        }  else
#endif /* BCM_TRX_SUPPORT */
        { 
            BCM_XGS3_L3_MEM(unit, tnl_init_mpls) = INVALIDm;
        }

        /* Next hop table. */
        BCM_XGS3_L3_MEM(unit, nh) = ING_L3_NEXT_HOPm;
        BCM_XGS3_L3_ENT_SZ(unit, nh) = 
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, nh));

        /* Egress Tunnel DSCP table. */
        BCM_XGS3_L3_MEM(unit, tnl_dscp) = EGR_DSCP_TABLEm;
        BCM_XGS3_L3_ENT_SZ(unit, tnl_dscp) =
                BCM_L3_MEM_ENT_SIZE(unit, BCM_XGS3_L3_MEM(unit, tnl_dscp));
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    _bcm_xgs3_l3_hw_call_init(unit);
    return (BCM_E_NONE);
}

#endif /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_l3_sw_dump
 * Purpose:
 *     Displays L3 information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_l3_sw_dump(int unit)
{
    int                      i, j;
    _bcm_l3_tbl_t 	     *tbl_ptr;     /* Generic table pointer     */

    soc_cm_print("\nSW Information L3 - Unit %d\n", unit);
    soc_cm_print("  Initialized : %d\n", BCM_XGS3_L3_INITIALIZED(unit));

    /* Interface table */
    soc_cm_print("  L3 Interface -\n");
    soc_cm_print("    table size : %d\n", BCM_XGS3_L3_IF_TBL_SIZE(unit));
    soc_cm_print("    count      : %d\n", BCM_XGS3_L3_IF_COUNT(unit));
  
    soc_cm_print("    Used index from bk info:");
    if (BCM_XGS3_L3_IF_INUSE(unit) != NULL) {
        for (i = 0, j = 0; i < BCM_XGS3_L3_IF_TBL_SIZE(unit); i++) {
            /* If not set, skip print */
            if (!BCM_L3_INTF_USED_GET(unit, i)) {
                continue;
            }
            if (!(j % 10)) {
                soc_cm_print("\n    ");
            }
            soc_cm_print("  %5d", i);
            j++;
        }
    }
    soc_cm_print("\n");

    soc_cm_print("    ARL  index :");
    if (BCM_XGS3_L3_IF_ADD2ARL(unit) != NULL) {
        for (i = 0, j = 0; i < BCM_XGS3_L3_IF_TBL_SIZE(unit); i++) {
            /* If not set, skip print */
            if (!BCM_L3_INTF_ARL_GET(unit, i)) {
                continue;
            }
            if (!(j % 10)) {
                soc_cm_print("\n    ");
            }
            soc_cm_print("  %5d", i);
            j++;
        }
    }
    soc_cm_print("\n");

    /* Defip table */
    soc_cm_print("  DEF IP -\n");
    soc_cm_print("    table size : %d\n", BCM_XGS3_L3_DEFIP_TBL_SIZE(unit));
    soc_cm_print("    IP4 count  : %d\n", BCM_XGS3_L3_DEFIP_IP4_CNT(unit));
    soc_cm_print("    IP6 count  : %d\n", BCM_XGS3_L3_DEFIP_IP6_CNT(unit));
    /* Skip 'strata_defip_info' since this is for Strata only */

    /* L3 Host table */
    soc_cm_print("  L3 -\n");
    soc_cm_print("    table size     : %d\n", BCM_XGS3_L3_TBL_SIZE(unit));
    soc_cm_print("    IP4 added      : %d\n", BCM_XGS3_L3_IP4_CNT(unit));
    soc_cm_print("    IP6 added      : %d\n", BCM_XGS3_L3_IP6_CNT(unit));
    soc_cm_print("    IP4 IPMC added : %d\n", BCM_XGS3_L3_IP4_IPMC_CNT(unit));
    soc_cm_print("    IP6 IPMC added : %d\n", BCM_XGS3_L3_IP6_IPMC_CNT(unit));

    /* ECMP table */
    soc_cm_print("  ECMP   table size : %d\n",
              BCM_XGS3_L3_ECMP_TBL_SIZE(unit));
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp_grp);
    soc_cm_print("\n    L3 ECMP Table -\n");
    soc_cm_print("        min       : %d\n", tbl_ptr->idx_min);
    soc_cm_print("        max       : %d\n", tbl_ptr->idx_max);
    soc_cm_print("        max paths : %d\n", BCM_XGS3_L3_ECMP_MAX_PATHS(unit));
    soc_cm_print("        in use    : %d\n", BCM_XGS3_L3_ECMP_IN_USE(unit));
  
    if(!(tbl_ptr->idx_min == 0 && tbl_ptr->idx_max == 0)) { /* mem not valid */
        soc_cm_print("        table index : hash : ref_count");
        for (i = tbl_ptr->idx_min, j = 0; i <= tbl_ptr->idx_max; i++) {
            /* If zero, skip print */
            if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i)) {
                continue;
            }
            if (!(j % 4)) {
                soc_cm_print("\n        ");
            }
            soc_cm_print("  %5d:%-5d:%-5d", i, BCM_XGS3_L3_ENT_HASH(tbl_ptr, i),
                BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i));
            j++;
        }
    }
    soc_cm_print("\n");
  
    /* Next hop table */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, next_hop);
    soc_cm_print("  NH      table size : %d\n", BCM_XGS3_L3_NH_TBL_SIZE(unit));
    soc_cm_print("  NH      index min  : %d\n", tbl_ptr->idx_min);
    soc_cm_print("  NH      index max  : %d\n", tbl_ptr->idx_max);
    soc_cm_print("  NH index max used  : %d\n", tbl_ptr->idx_maxused);    
    for (i = tbl_ptr->idx_min, j = 0; i <= tbl_ptr->idx_max; i++) {
        if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i)) {
           continue;
        }
        if (!(j % 4)) {
            soc_cm_print("\n    ");
        }
        soc_cm_print(" %5d:%-5d:%-5d", i, BCM_XGS3_L3_ENT_HASH(tbl_ptr, i),
              BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i));
        j++;
        soc_tunnel_term_sw_dump(unit);
    }
    
    /* Tunnel initiator table */
    tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, tnl_init);
    soc_cm_print("\n    L3 Tunnel Initiator Table -\n");
    soc_cm_print("        min     : %d\n", tbl_ptr->idx_min);
    soc_cm_print("        max     : %d\n", tbl_ptr->idx_max);
    soc_cm_print("        total   : %d\n", BCM_XGS3_L3_TUNNEL_TBL_SIZE(unit));
  
    if(!(tbl_ptr->idx_min == 0 && tbl_ptr->idx_max == 0)) { /* mem not valid */
        soc_cm_print("        use count (index:hash:count) :");
        for (i = tbl_ptr->idx_min, j = 0; i <= tbl_ptr->idx_max; i++) {
            /* If zero count, skip print */
            if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i)) {
                continue;
            }
            if (!(j % 4)) {
                soc_cm_print("\n        ");
            }
            soc_cm_print("  %5d:%-5d:%-5d", i, BCM_XGS3_L3_ENT_HASH(tbl_ptr, i),
                BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i));
            j++;
        }
    }
    soc_cm_print("\n");
    
    /* Adjacent MAC table (ER only) */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, adj_mac);
        soc_cm_print("\n    Adjacent MAC Table -\n");
        soc_cm_print("        min     : %d\n", tbl_ptr->idx_min);
        soc_cm_print("        max     : %d\n", tbl_ptr->idx_max);
  
        soc_cm_print("        use count (index:count) :");
        for (i = tbl_ptr->idx_min, j = 0; i <= tbl_ptr->idx_max; i++) {
            /* If zero count, skip print */
            if (!BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i)) {
                continue;
            }
            if (!(j % 4)) {
               soc_cm_print("\n        ");
            }
            soc_cm_print("  %5d:%-5d", i, BCM_XGS3_L3_ENT_REF_CNT(tbl_ptr, i));
            j++;
        }
        soc_cm_print("\n");	
    }	
#endif /* BCM_EASYRIDER_SUPPORT */    

    /* Prefix trackers (FB only) */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        soc_fb_lpm_sw_dump(unit);
    }
#endif /* BCM_FIREBOLT_SUPPORT */  	 

    return;
}

#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
#else  /* INCLUDE_L3 */
int bcm_esw_firebolt_l3_not_empty;
#endif /* INCLUDE_L3 */
