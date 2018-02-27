/*
 * $Id: port.c 1.1012.2.58 Broadcom SDK $
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
 * File:        port.c
 * Purpose:     Tracks and manages ports.
 *              P-VLAN table is managed directly.
 *              MAC/PHY interfaces are managed through respective drivers.
 */

#include <assert.h>

#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/phy.h>
#include <soc/ll.h>
#include <soc/ptable.h>
#include <soc/tucana.h>
#include <soc/firebolt.h>
#include <soc/xaui.h>
#include <soc/phyctrl.h>
#include <soc/phyreg.h>
#include <soc/higig.h>
#include <soc/hash.h>

#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/mirror.h>

#include <bcm_int/api_xlate_port.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/diffserv.h>
#include <bcm_int/esw/link.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/stat.h>
#include <bcm_int/esw/strata.h>
#include <bcm_int/esw/lynx.h>
#include <bcm_int/esw/draco.h>
#include <bcm_int/esw/tucana.h>
#include <bcm_int/esw/firebolt.h>
#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/timesync.h>
#endif /* BCM_KATANA_SUPPORT */
#if defined(BCM_EASYRIDER_SUPPORT)
#include <bcm_int/esw/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/virtual.h>
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_SCORPION_SUPPORT)
#include <bcm_int/esw/scorpion.h>
#include <bcm_int/esw/triumph.h>
#endif /* BCM_SCORPION_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
#include <bcm_int/esw/trx.h>
#endif /* BCM_TRX_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <bcm_int/esw/triumph2.h>
#include <soc/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
#include <bcm_int/esw/trident.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_ENDURO_SUPPORT)
#include <bcm_int/esw/enduro.h>
#endif /* BCM_ENDURO_SUPPORT */
#if defined(BCM_HURRICANE_SUPPORT)
#include <bcm_int/esw/hurricane.h>
#endif /* BCM_HURRICANE_SUPPORT */
#if defined(BCM_SHADOW_SUPPORT)
#include <soc/shadow.h>
#endif /* BCM_SHADOW_SUPPORT */
#include <bcm_int/esw/xgs3.h>
#if defined(BCM_METER_SUPPORT)
#include <bcm/diffserv.h>
#endif

#include <bcm_int/esw/flex_ctr.h>
#include <bcm_int/esw/policer.h>
#if defined(INCLUDE_CES)
#include <soc/defs.h>
#include <sal/appl/sal.h>
#include <bcm_int/esw/ces.h>
#include <pub/nd_api.h>

#endif 
#include <bcm_int/esw_dispatch.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Variable:
 *      bcm_port_info
 * Purpose:
 *      One entry for each SOC device containing port information
 *      for that device.  Use the PORT macro to access it.
 */
static _bcm_port_info_t     *bcm_port_info[BCM_MAX_NUM_UNITS];

#define PORT(unit, port)        bcm_port_info[unit][port]

/* Accessor to bcm_port_info used by other modules */
void _bcm_port_info_access(int unit, bcm_port_t port, _bcm_port_info_t **info)
{
    *info = &PORT(unit, port);
    return;
}

/*
 * Define:
 *      PORT_LOCK/PORT_UNLOCK
 * Purpose:
 *      Serialization Macros.
 *      Here the cmic memory lock also serves to protect the
 *      bcm_port_info structure and EPC_PFM registers.
 */

#define PORT_LOCK(unit) \
        if (soc_mem_is_valid(unit, PORT_TABm)) \
        { soc_mem_lock(unit, PORT_TABm); }


#define PORT_UNLOCK(unit) \
        if (soc_mem_is_valid(unit, PORT_TABm)) \
        { soc_mem_unlock(unit, PORT_TABm); }

/*
 * Define:
 *      PORT_INIT
 * Purpose:
 *      Causes a routine to return BCM_E_INIT if port is not yet initialized.
 */

#define PORT_INIT(unit) \
        if (bcm_port_info[unit] == NULL) { return BCM_E_INIT; }

/*
 * Define:
 *      PORT_PARAM_CHECK
 * Purpose:
 *      Check unit and port parameters for most bcm_port api calls
 */
#define PORT_PARAM_CHECK(unit, port) do { \
        PORT_INIT(unit); \
        if (!SOC_PORT_VALID(unit, port)) { return BCM_E_PORT; } \
        } while (0);

/*
 * Define:
 *      PORT_SWITCHED_CHECK
 * Purpose:
 *      Check unit and port for switching feature support
 */
#define PORT_SWITCHED_CHECK(unit, port) do { \
        if (IS_ST_PORT(unit, port)) { \
            return BCM_E_PORT; \
        } else { \
            if (IS_CPU_PORT(unit, port)) { \
                if (!soc_feature(unit, soc_feature_cpuport_switched)) { \
                    return BCM_E_PORT; \
                } \
            } \
        } \
        } while (0);

#ifdef BCM_TRIUMPH2_SUPPORT
typedef int _src_mod_egr_prof_ref_t;
STATIC _src_mod_egr_prof_ref_t src_mod_egr_prof_ref[BCM_MAX_NUM_UNITS][8];
#define SRC_MOD_EGR_REF_COUNT(_u, _r) src_mod_egr_prof_ref[_u][_r]

typedef int _port_src_mod_egr_prof_ptr_t;
STATIC _port_src_mod_egr_prof_ptr_t 
    port_src_mod_egr_prof_ptr[BCM_MAX_NUM_UNITS][SOC_MAX_NUM_PORTS];
#define PORT_SRC_MOD_EGR_PROF_PTR(_u, _p) port_src_mod_egr_prof_ptr[_u][_p]
#if defined(BCM_KATANA_SUPPORT)
STATIC
bcm_error_t _bcm_esw_port_stat_get_table_info(
                   int                        unit,
                   bcm_gport_t                port,
                   uint32                     *num_of_tables,
                   bcm_stat_flex_table_info_t *table_info);
#endif
#endif

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_TRIDENT_SUPPORT) ||\
defined(BCM_HAWKEYE_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
/* EEE standard compliance Work Around:
 * To Maintain software state of native eee in MAC*/ 
STATIC int eee_cfg[BCM_MAX_NUM_UNITS][SOC_MAX_NUM_PORTS]; 
#endif

/*
 * The Port Monitor Thread is intended for handling work-arounds
 * that are unneeded in most cases. In order to avoid unnecessary
 * processing overhead the thread is not started by default, but
 * only if so requested through configuration parameters.
 */

typedef struct _port_mon_ctrl_s
{
    char taskname[16];
    sal_sem_t sema;
    int interval;
    int running;
    int stop;
    int unit;
    uint32 gtuc;
    uint32 gtmca;
    uint32 gtbca;
    uint32 gtlcl;
    uint32 gtpok;
} _port_mon_ctrl_t;

STATIC _port_mon_ctrl_t _bcm_esw_port_mon_ctrl[BCM_MAX_NUM_UNITS];

#define PORT_MON_CTRL(_u) (&_bcm_esw_port_mon_ctrl[_u])

#define PORT_MON_INTERVAL_MIN   100000

/*
 * Function:
 *      _bcm_esw_port_mon_unimac_lock_up
 * Description:
 *      Check for UniMAC half duplex collision lock-up issue
 * Parameters:
 *      pmc  - (IN) port monitor control data
 * Return Value:
 *      None
 * Notes:
 *      See errata 56020-ES105-R for additional details.
 */
STATIC void
_bcm_esw_port_mon_unimac_lock_up(_port_mon_ctrl_t *pmc)
{
    int unit = pmc->unit;
    int port, rv, link_up, fulld, lock_up;
    uint32 gtuc, gtmca, gtbca, gtlcl, gtpok;
    uint32 ccnt, rcnt, txfifo;

    if (!soc_feature(unit, soc_feature_unimac)) {
        return;
    }

    PBMP_E_ITER(unit, port) {
        /* Check only half duplex ports */
        rv = _bcm_esw_link_get(unit, port, &link_up);
        if (BCM_FAILURE(rv) || !link_up) {
            continue;
        }
        rv = MAC_DUPLEX_GET(PORT(unit, port).p_mac, unit, port, &fulld);
        if (BCM_FAILURE(rv) || fulld) {
            continue;
        }

        lock_up = 0;
        rv = 0;

        /* Collect test counters */
        rv += soc_counter_get32(unit, port, GTUCr, 0, &gtuc);
        rv += soc_counter_get32(unit, port, GTMCAr, 0, &gtmca);
        rv += soc_counter_get32(unit, port, GTBCAr, 0, &gtbca);
        rv += soc_counter_get32(unit, port, GTLCLr, 0, &gtlcl);
        rv += soc_counter_get32(unit, port, GTPOKr, 0, &gtpok);

        /* Test lock-up conditions */
        if (gtuc == pmc->gtuc &&
            gtmca == pmc->gtmca &&
            gtbca == pmc->gtbca) {
            if (gtlcl != pmc->gtlcl &&
                gtpok != pmc->gtpok) {
                lock_up = 2;
            } else {
                rcnt = ccnt = 0;
                if (SOC_REG_IS_VALID(unit, GE0_GBODE_CELL_CNTr)) {
                    uint32 reg, addr, idx;
                    idx = (port < 6) ? port : ((port - 6) % 12);
                    reg = GE0_GBODE_CELL_CNTr;
                    addr = soc_reg_addr(unit, reg, port, 0);
                    addr += idx;
                    rv += soc_reg32_get(unit, reg, port, 0, &ccnt);
                    reg = GE0_GBODE_CELL_REQ_CNTr;
                    addr = soc_reg_addr(unit, reg, port, 0);
                    addr += idx;
                    rv += soc_reg32_get(unit, reg, port, 0, &rcnt);
                } else if (SOC_REG_IS_VALID(unit, GE_GBODE_CELL_CNTr)) {
                    rv += READ_GE_GBODE_CELL_CNTr(unit, port, &ccnt);
                    rv += READ_GE_GBODE_CELL_REQ_CNTr(unit, port, &rcnt);
                }
                if (gtlcl && rcnt == 0 && ccnt == 4) {
                    lock_up = 1;
                }
            }
        }
        if (lock_up == 0) {
            rv += READ_TXFIFO_STATr(unit, port, &txfifo);
            if (soc_reg_field_get(unit, TXFIFO_STATr,
                                  txfifo, TXFIFO_UNDERRUNf)) {
                lock_up = 2;
            }
        }

        /* Reset MAC if lock-up is detected */
        if (rv == 0 && lock_up) {
            SOC_DEBUG_PRINT((DK_VERBOSE,
                             "Port %s: UniMAC lock up (%d) detected\n",
                             SOC_PORT_NAME(unit, port), lock_up));
            PORT_LOCK(unit);
            MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                            SOC_MAC_CONTROL_SW_RESET, TRUE);
            MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                            SOC_MAC_CONTROL_SW_RESET, FALSE);
            PORT_UNLOCK(unit);
        }

        /* Sync test counters */
        pmc->gtuc = gtuc;
        pmc->gtmca = gtmca;
        pmc->gtbca = gtbca;
        pmc->gtlcl = gtlcl;
        pmc->gtpok = gtpok;
    }
}

/*
 * Function:
 *      _bcm_esw_port_mon_ctrl_thread
 * Description:
 *      Port monitor thread
 * Parameters:
 *      context  - (IN) Port monitor control data
 * Return Value:
 *      None
 */
STATIC void
_bcm_esw_port_mon_ctrl_thread(void *context)
{
    _port_mon_ctrl_t *pmc = (_port_mon_ctrl_t *)context;

    SOC_DEBUG_PRINT((DK_VERBOSE,
                     "Port monitor started on unit %d\n", pmc->unit));

    pmc->running = 1;
    while (!pmc->stop) {
        _bcm_esw_port_mon_unimac_lock_up(pmc);
        (void)sal_sem_take(pmc->sema, pmc->interval);
    }
    pmc->running = 0;
}

/*
 * Function:
 *      _bcm_esw_port_mon_start
 * Description:
 *      Start port monitor thread
 * Parameters:
 *      unit  - (IN) BCM device number
 * Return Value:
 *      BCM_E_NONE if thread started successfully
 */
int
_bcm_esw_port_mon_start(int unit)
{
    _port_mon_ctrl_t *pmc = PORT_MON_CTRL(unit);
    int cnt;

    pmc->interval = soc_property_get(unit, spn_PORTMON_INTERVAL, 0);
    if (pmc->interval == 0) {
        return BCM_E_NONE;
    }

    if (pmc->interval < PORT_MON_INTERVAL_MIN) {
        pmc->interval = PORT_MON_INTERVAL_MIN;
    }

    pmc->unit = unit;
    if (pmc->running) {
        SOC_DEBUG_PRINT((DK_VERBOSE,
                         "Port monitor already running on unit %d\n",
                         pmc->unit));
    } else {
        if (pmc->sema == NULL) {
            pmc->sema = sal_sem_create("port_mon_SLEEP", sal_sem_BINARY, 0);
            if (pmc->sema == NULL) {
                return BCM_E_MEMORY;
            }
        }
        sal_snprintf(pmc->taskname, sizeof (pmc->taskname),
                     "bcmPortMon.%d", unit);
        pmc->stop = 0;
        if (sal_thread_create(pmc->taskname, SAL_THREAD_STKSZ,
                              soc_property_get(unit,
                                               spn_PORTMON_THREAD_PRI,
                                               50),
                              _bcm_esw_port_mon_ctrl_thread,
                              pmc) == SAL_THREAD_ERROR) {
            return BCM_E_MEMORY;
        }
        cnt = 10;
        while (!pmc->running && cnt--) {
            sal_usleep(100000);
        }
        if (!pmc->running) {
            SOC_DEBUG_PRINT((DK_ERR,
                             "%s: Thread did not start\n", pmc->taskname));
            return BCM_E_TIMEOUT;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_port_mon_stop
 * Description:
 *      Stop port monitor thread
 * Parameters:
 *      unit  - (IN) BCM device number
 * Return Value:
 *      BCM_E_NONE if thread stopped successfully
 */
int
_bcm_esw_port_mon_stop(int unit)
{
    int cnt;
    _port_mon_ctrl_t *pmc = PORT_MON_CTRL(unit);

    if (pmc->sema == NULL) {
        return BCM_E_NONE;
    }

    pmc->stop = 1;
    sal_sem_give(pmc->sema);
    cnt = 10;
    while (pmc->running && cnt--) {
        sal_usleep(100000);
    }
    if (pmc->running) {
        SOC_DEBUG_PRINT((DK_ERR,
                         "%s: Thread did not start\n", pmc->taskname));
        return BCM_E_TIMEOUT;
    }
    sal_sem_destroy(pmc->sema);
    pmc->sema = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_port_gport_validate
 * Description:
 *      Helper funtion to validate port/gport parameter 
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port_in  - (IN) Port / Gport to validate
 *      port_out - (OUT) Port number if valid. 
 * Return Value:
 *      BCM_E_NONE - Port OK 
 *      BCM_E_INIT - Not initialized
 *      BCM_E_PORT - Port Invalid
 */
int
_bcm_esw_port_gport_validate(int unit, bcm_port_t port_in, bcm_port_t *port_out)
{
    if (bcm_port_info[unit] == NULL) { 
        return BCM_E_INIT; 
    }

    if (BCM_GPORT_IS_SET(port_in)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port_in, port_out));
    } else if (SOC_PORT_VALID(unit, port_in)) { 
        *port_out = port_in;
    } else {
        return BCM_E_PORT; 
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_info_get
 * Description:
 *      Helper funtion  
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port  - (IN) Port number
 *      pinfo - (OUT) Port info. 
 * Return Value:
 *      BCM_E_XXX 
 */
int
_bcm_port_info_get(int unit, bcm_port_t port, _bcm_port_info_t **pinfo)
{
    /* Input parameters check. */
    if (NULL == pinfo) {
        return (BCM_E_PARAM);
    }

    PORT_INIT(unit);

    *pinfo = &PORT(unit, port);
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_esw_iport_tab_set
 * Description:
 *      Set field in either IPORT_TABLE or the special CPU Higig entry in
 *      the PORT_TAB.
 * Parameters:
 *      unit  - Device number
 *      port  - Port number
 *      field - Field name within PORT_TAB table entry
 *      value - new field value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_XXX - an error occurred accessing PORT_TAB table.
 */
STATIC int
_bcm_esw_iport_tab_set(int unit, bcm_port_t port, 
                       soc_field_t field, int value)

{
    port_tab_entry_t pent;
    soc_mem_t mem;
    int rv, index, cur_val;

    if (SOC_MEM_IS_VALID(unit, IPORT_TABLEm)) {
        mem = IPORT_TABLEm;
        index = SOC_PORT_MOD_OFFSET(unit, port);
    } else if (IS_CPU_PORT(unit, port) && SOC_INFO(unit).cpu_hg_index != -1) {
        mem = PORT_TABm;
        index = SOC_INFO(unit).cpu_hg_index;
    } else {
        return BCM_E_NONE;
    }

    if (!SOC_MEM_FIELD_VALID(unit, mem, field)) {
        return (BCM_E_UNAVAIL);
    }

    PORT_LOCK(unit);
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, index, &pent);
    if (BCM_SUCCESS(rv)) {
        cur_val = soc_mem_field32_get(unit, mem, &pent, field);
        if (value != cur_val) {
            soc_mem_field32_set(unit, mem, &pent, field, value);
            rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, index, &pent);
        }
    }
    PORT_UNLOCK(unit);
    return rv;
}

#define _BCM_CPU_TABS_NONE        0
#define _BCM_CPU_TABS_ETHER     0x1
#define _BCM_CPU_TABS_HIGIG     0x2
#define _BCM_CPU_TABS_BOTH      0x3


/*
 * Function:
 *      _bcm_port_autoneg_advert_remote_get
 * Purpose:
 *      Main part of bcm_port_advert_get_remote
 */

STATIC int
_bcm_port_autoneg_advert_remote_get(int unit, bcm_port_t port,
                            bcm_port_ability_t *ability_mask)
{
    int                 an, an_done;

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_auto_negotiate_get(unit,  port,
                                &an, &an_done));

    if (!an) {
        return BCM_E_DISABLED;
    }

    if (!an_done) {
        return BCM_E_BUSY;
    }

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_ability_remote_get(unit, port, ability_mask));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_ability_local_get
 * Purpose:
 *      Main part of bcm_port_ability_local_get
 */

STATIC int
_bcm_port_ability_local_get(int unit, bcm_port_t port,
                           bcm_port_ability_t *ability_mask)
{
    soc_port_ability_t             mac_ability, phy_ability;
    soc_pa_encap_t                 encap_ability; 

    sal_memset(&phy_ability, 0, sizeof(soc_port_ability_t));
    sal_memset(&mac_ability, 0, sizeof(soc_port_ability_t));

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_ability_local_get(unit, port, &phy_ability));

    if (!(IS_TDM_PORT(unit, port)))
	{
    SOC_IF_ERROR_RETURN
        (MAC_ABILITY_LOCAL_GET(PORT(unit, port).p_mac, unit, 
                              port, &mac_ability));
	}

    /* Combine MAC and PHY abilities */
    ability_mask->speed_half_duplex =
        mac_ability.speed_half_duplex & phy_ability.speed_half_duplex;
    ability_mask->speed_full_duplex =
        mac_ability.speed_full_duplex & phy_ability.speed_full_duplex;
    ability_mask->pause     = mac_ability.pause & phy_ability.pause;
    if (phy_ability.interface == 0) {
        ability_mask->interface = mac_ability.interface;
    } else {
        ability_mask->interface = phy_ability.interface;
    }
    ability_mask->medium    = phy_ability.medium;
    /* mac_ability.eee without phy_ability.eee makes no sense */
    ability_mask->eee    = phy_ability.eee;
    ability_mask->loopback  = mac_ability.loopback | phy_ability.loopback |
                               BCM_PORT_ABILITY_LB_NONE;
    ability_mask->flags     = mac_ability.flags | phy_ability.flags;

    /* Get port encapsulation ability */
    encap_ability = mac_ability.encap; 

    if ((soc_feature(unit, soc_feature_embedded_higig)) 
        && IS_E_PORT(unit, port)) {
        encap_ability |= BCM_PA_ENCAP_HIGIG2_L2;
        encap_ability |= BCM_PA_ENCAP_HIGIG2_IP_GRE;
    }

#if defined(BCM_RAPTOR_SUPPORT)
    if ((SOC_IS_RAPTOR(unit) || SOC_IS_RAVEN(unit)) &&
        IS_S_PORT(unit, port)) {
        encap_ability |= BCM_PA_ENCAP_HIGIG2;
    }
#endif

    if (IS_HL_PORT(unit, port) && 
         (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit))) {
        encap_ability |= BCM_PA_ENCAP_HIGIG2;
    }
#if defined (BCM_HURRICANE_SUPPORT)
    if (IS_XQ_PORT(unit, port) && SOC_IS_HURRICANE(unit) &&
        (CMDEV(unit).dev.rev_id == BCM56142_A0_REV_ID) ) {
        ability_mask->speed_half_duplex &= ~(BCM_PORT_ABILITY_100MB);
        ability_mask->speed_full_duplex &= ~(BCM_PORT_ABILITY_100MB);
    }
#endif
    ability_mask->encap = encap_ability;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_lport_tab_set
 * Description:
 *      Helper funtion for bcm_esw_port_control_set.
 * Parameters:
 *      unit  - Device number
 *      port  - Port number
 *      field - Field name within PORT_TAB table entry
 *      value - new field value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_XXX - an error occurred accessing PORT_TAB table.
 */
STATIC int
_bcm_esw_lport_tab_set(int unit, bcm_port_t port, 
                      soc_field_t field, int value)

{
    bcm_port_config_t port_config;
    lport_tab_entry_t pent;
    int rv, cur_val;
    int idx;
    soc_mem_t mem = LPORT_TABm;

    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &port_config));

    if (!SOC_MEM_FIELD_VALID(unit, mem, field)) {
        return (BCM_E_UNAVAIL);
    }

    if (0 == PBMP_MEMBER(port_config.hg, port)) { 
        return (BCM_E_NONE);
    }

    if (soc_feature(unit, soc_feature_lport_tab_profile)) {
        idx = soc_mem_index_min(unit, LPORT_TABm);
    } else {
        idx = SOC_PORT_MOD_OFFSET(unit, port);
    }

    sal_memset(&pent, 0, sizeof(lport_tab_entry_t));
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, idx, &pent);
    BCM_IF_ERROR_RETURN(rv);

    cur_val = soc_LPORT_TABm_field32_get(unit, &pent, field);
    if (value != cur_val) {
        soc_LPORT_TABm_field32_set(unit, &pent, field, value);
        if (0 == soc_feature(unit, soc_feature_lport_tab_profile)) {
            idx = port;
        }
        rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, idx, &pent);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_port_tab_set
 * Description:
 *      Helper funtion for bcm_esw_port_control_set.
 * Parameters:
 *      unit  - Device number
 *      port  - Port number
 *      cpu_tabs - If CPU port, which ingress tables should be written?
 *      field - Field name within PORT_TAB table entry
 *      value - new field value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_XXX - an error occurred accessing PORT_TAB table.
 */
STATIC int
_bcm_esw_port_tab_set(int unit, bcm_port_t port, int cpu_tabs,
                      soc_field_t field, int value)

{
    port_tab_entry_t pent;
    soc_mem_t mem;
    int rv, index, cur_val;

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        rv = _bcm_tr2_wlan_port_set(unit, port, field, value);
        return rv;
    }
#endif

    mem = SOC_PORT_MEM_TAB(unit, port);
    if (!SOC_MEM_FIELD_VALID(unit, mem, field)) {
        return (BCM_E_UNAVAIL);
    }

    index = SOC_PORT_MOD_OFFSET(unit, port);

    PORT_LOCK(unit);

    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, index, &pent);
    if (BCM_SUCCESS(rv)) {
        cur_val = soc_PORT_TABm_field32_get(unit, &pent, field);
        if (value != cur_val) {
            soc_PORT_TABm_field32_set(unit, &pent, field, value);
            if (!IS_CPU_PORT(unit, port) || (cpu_tabs & _BCM_CPU_TABS_ETHER)) {
                rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, index, &pent);
            }
            if (BCM_SUCCESS(rv) &&
                IS_CPU_PORT(unit, port) && (cpu_tabs & _BCM_CPU_TABS_HIGIG)) {
                rv = _bcm_esw_iport_tab_set(unit, port,field, value);
            }
        }
    }

    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_port_tab_get
 * Description:
 *      Helper funtion for bcm_esw_port_control_get.
 * Parameters:
 *      unit  - Device number
 *      port  - Port number
 *      field - Field name within PORT_TAB table entry
 *      value - (OUT) field value
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_XXX - an error occurred accessing PORT_TAB table.
 */
STATIC int
_bcm_esw_port_tab_get(int unit, bcm_port_t port, 
                      soc_field_t field, int *value)

{
    port_tab_entry_t pent;
    soc_mem_t mem;
    int rv;

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        rv = _bcm_tr2_lport_field_get(unit, port, field, value);
        return rv;
    }
#endif

    mem = SOC_PORT_MEM_TAB(unit, port);
    if (!SOC_MEM_FIELD_VALID(unit, mem, field)) {
        return (BCM_E_UNAVAIL);
    }

    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                      SOC_PORT_MOD_OFFSET(unit, port), &pent);

    if (BCM_SUCCESS(rv)) {
        *value = soc_PORT_TABm_field32_get(unit, &pent, field);
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_untagged_vlan_get
 * Purpose:
 *      Retrieve the default VLAN ID for the port.
 *      This is the VLAN ID assigned to received untagged packets.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - StrataSwitch port number of port to get info for
 *      vid_ptr - (OUT) Pointer to VLAN ID for return
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL if table read failed.
 */

int
bcm_esw_port_untagged_vlan_get(int unit, bcm_port_t port, bcm_vlan_t *vid_ptr)
{
    bcm_port_cfg_t              pcfg;
    int                         rv;

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        return bcm_tr2_wlan_port_untagged_vlan_get(unit, port, vid_ptr);
    }
#endif    
    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_validate(unit, port, &port));

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        *vid_ptr = pcfg.pc_vlan;
    } else {
        *vid_ptr = BCM_VLAN_INVALID;
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_untagged_vlan_get: u=%d p=%d vid=%d rv=%d\n",
                     unit, port, *vid_ptr, rv));

    return rv;
}

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_SCORPION_SUPPORT)
/*
 * Function:
 *      _bcm_port_mmu_update
 * Purpose:
 *      Adjust MMU settings depending on port status.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      link -  True if link is active, false if link is inactive.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

STATIC int
_bcm_port_mmu_update(int unit, bcm_port_t port, int link)
{
    int pause_tx, pause_rx, q_limit_enable, cos;
    uint32 psl_rval, opc_rval, oqc_rval;

    if (!SOC_IS_HBX(unit)) {
        return (BCM_E_UNAVAIL);
    }

    if (link < 0) {
        SOC_IF_ERROR_RETURN
            (bcm_esw_port_link_status_get(unit, port, &link));
    }

    SOC_IF_ERROR_RETURN
        (MAC_PAUSE_GET(PORT(unit, port).p_mac, unit, port,
                       &pause_tx, &pause_rx));

    SOC_IF_ERROR_RETURN(READ_PORT_SHARED_LIMITr(unit, port, &psl_rval));
    SOC_IF_ERROR_RETURN(READ_OP_PORT_CONFIGr(unit, port, &opc_rval));
    if (link && pause_tx) {
        soc_reg_field_set(unit, PORT_SHARED_LIMITr, &psl_rval, 
                          PORT_SHARED_LIMITf,
                          SOC_IS_SC_CQ(unit) ? 0x5 : 0x3);
        soc_reg_field_set(unit, PORT_SHARED_LIMITr, &psl_rval, 
                          PORT_SHARED_DYNAMICf, 1);
        soc_reg_field_set(unit, OP_PORT_CONFIGr, &opc_rval, 
                          PORT_LIMIT_ENABLEf, 1);
        q_limit_enable = 0;
    } else {
        /* Turn off */
        soc_reg_field_set(unit, PORT_SHARED_LIMITr, &psl_rval, 
                          PORT_SHARED_LIMITf,
                          SOC_IS_SC_CQ(unit) ? 0x3fff : 0x3000);
        soc_reg_field_set(unit, PORT_SHARED_LIMITr, &psl_rval, 
                          PORT_SHARED_DYNAMICf, 0);
        soc_reg_field_set(unit, OP_PORT_CONFIGr, &opc_rval, 
                          PORT_LIMIT_ENABLEf, 0);
        q_limit_enable = 1;
    }
    SOC_IF_ERROR_RETURN(WRITE_PORT_SHARED_LIMITr(unit, port, psl_rval));
    SOC_IF_ERROR_RETURN(WRITE_OP_PORT_CONFIGr(unit, port, opc_rval));
    for (cos = 0; cos < NUM_COS(unit); cos++) {
        SOC_IF_ERROR_RETURN
            (READ_OP_QUEUE_CONFIGr(unit, port, cos, &oqc_rval));
        soc_reg_field_set(unit, OP_QUEUE_CONFIGr, &oqc_rval, 
                          Q_LIMIT_ENABLEf, q_limit_enable);
        SOC_IF_ERROR_RETURN
            (WRITE_OP_QUEUE_CONFIGr(unit, port, cos, oqc_rval));
    }

    return (BCM_E_NONE);
}
#endif /* BCM_BRADLEY_SUPPORT || BCM_SCORPION_SUPPORT */

/*
 * Function:
 *      _bcm_port_untagged_vlan_set
 * Purpose:
 *      Main part of bcm_port_untagged_vlan_set.
 * Notes:
 *      Port does not have to be a member of the VLAN.
 */

STATIC int
_bcm_port_untagged_vlan_set(int unit, bcm_port_t port, bcm_vlan_t vid)
{
    bcm_port_cfg_t      pcfg;
    int                 ut_prio;
    bcm_vlan_t          pdvid;

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_vlan_action)) {
        bcm_vlan_action_set_t action;

        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_default_action_get(unit, port, &action));
        action.new_outer_vlan = vid;
        action.priority = PORT(unit, port).p_ut_prio;

        return _bcm_trx_vlan_port_default_action_set(unit, port, &action);
    }
#endif

    BCM_IF_ERROR_RETURN
        (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));

    pdvid = pcfg.pc_vlan;
    pcfg.pc_vlan = vid;

    BCM_IF_ERROR_RETURN
        (mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg));

#if defined(BCM_DRACO15_SUPPORT)
    if (SOC_IS_DRACO15(unit)) {
        /*
         * Update default VLAN ID in VLAN_DATA
         */
        int rv = _bcm_d15_port_untagged_vlan_data_update(unit, port,
                                                         pdvid, vid);
        if (rv != BCM_E_NONE) {
            soc_cm_debug(DK_ERR,
                         "failed updating default VLAN ID "
                         "in VLAN_DATA: %s\n", bcm_errmsg(rv));
        }
    }
#endif /* BCM_DRACO15_SUPPORT */
#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_FBX(unit)) {
        /*
         * Update default VLAN ID in VLAN_PROTOCOL_DATA
         */
        int rv = _bcm_fb_port_untagged_vlan_data_update(unit, port,
                                                        pdvid, vid);
        if (rv != BCM_E_NONE) {
            soc_cm_debug(DK_ERR,
                         "failed updating default VLAN ID "
                         "in VLAN_PROTOCOL_DATA: %s\n", bcm_errmsg(rv));
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    if (!soc_feature(unit, soc_feature_remap_ut_prio)) {
        /* Reset the untagged port priority filter entry, if any */

        BCM_IF_ERROR_RETURN
            (bcm_esw_port_untagged_priority_get(unit, port, &ut_prio));

        BCM_IF_ERROR_RETURN
            (bcm_esw_port_untagged_priority_set(unit, port, ut_prio));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_untagged_vlan_set
 * Purpose:
 *      Set the default VLAN ID for the port.
 *      This is the VLAN ID assigned to received untagged packets.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - StrataSwitch port number.
 *      vid -  VLAN ID used for packets that ingress the port untagged
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_NOT_FOUND if vid not in VTABLE
 *      BCM_E_INTERNAL if table read failed.
 *      BCM_E_CONFIG - port does not belong to the VLAN
 * Notes:
 *      BCM_LOCK is used because bcm_vlan_port_get is called internally
 *      which also takes it.  BCM_LOCK must be taken before PORT_LOCK.
 */

int
bcm_esw_port_untagged_vlan_set(int unit, bcm_port_t port, bcm_vlan_t vid)
{
    int rv;

    VLAN_CHK_ID(unit, vid);
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        return bcm_tr2_wlan_port_untagged_vlan_set(unit, port, vid);
    }
#endif
    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_validate(unit, port, &port));

    BCM_LOCK(unit);
    PORT_LOCK(unit);
    rv = _bcm_port_untagged_vlan_set(unit, port, vid);
    PORT_UNLOCK(unit);
    BCM_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_untagged_vlan_set: u=%d p=%d vid=%d rv=%d\n",
                     unit, port, vid, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_enable_set
 * Purpose:
 *      Physically enable/disable the MAC/PHY on this port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If linkscan is running, it also controls the MAC enable state.
 */

int
bcm_esw_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int         rv, link;

    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    if (enable) {
        if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), port)) {
            PORT_UNLOCK(unit);
            return BCM_E_NONE;
        }

        rv = bcm_esw_port_link_status_get(unit, port, &link);
        if (BCM_SUCCESS(rv)) {
            if (link) {
                rv = MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, TRUE);
            }
        } else {
            rv = BCM_E_NONE;
        }

        if (SOC_SUCCESS(rv)) {
            rv = soc_phyctrl_enable_set(unit, port, TRUE);
        }
    } else {
        MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                        SOC_MAC_CONTROL_DISABLE_PHY, TRUE);
        rv = soc_phyctrl_enable_set(unit, port, FALSE);
        MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                        SOC_MAC_CONTROL_DISABLE_PHY, FALSE);

        if (SOC_SUCCESS(rv)) {
            rv = MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, FALSE);
        }
    }

    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_enable_set: u=%d p=%d enable=%d rv=%d\n",
                     unit, port, enable, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_enable_get
 * Purpose:
 *      Gets the enable state as defined by bcm_port_enable_set()
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - (OUT) TRUE, port is enabled, FALSE port is disabled.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The PHY enable holds the port enable state set by the user.
 *      The MAC enable transitions up and down automatically via linkscan
 *      even if user port enable is always up.
 */

int
bcm_esw_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    int                 rv;

    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_validate(unit, port, &port));

    if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), port)) {
        *enable = 0;
        return BCM_E_NONE;
    }

    PORT_LOCK(unit);
    rv = soc_phyctrl_enable_get(unit, port, enable);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_enable_get: u=%d p=%d rv=%d enable=%d\n",
                     unit, port, rv, *enable));

    return rv;
}

/*
 * Function:
 *      _bcm_port_mode_setup
 * Purpose:
 *      Set initial operating mode for a port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - Whether to enable or disable
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Must be called with PORT_LOCK held.
 */

int
_bcm_port_mode_setup(int unit, bcm_port_t port, int enable)
{
    soc_port_if_t       pif;
    bcm_port_ability_t  local_pa, advert_pa;

    SOC_DEBUG_PRINT((DK_PORT,
        "_bcm_port_mode_setup: u=%d p=%d\n", unit, port));

    SOC_IF_ERROR_RETURN(bcm_esw_port_ability_local_get(unit, port, &local_pa));

    if (!(IS_TDM_PORT(unit, port)))
	{
    /* If MII supported, enable it, otherwise use TBI */
    if (local_pa.interface & (SOC_PA_INTF_MII | SOC_PA_INTF_GMII |
                              SOC_PA_INTF_SGMII | SOC_PA_INTF_XGMII)) {
        if (IS_GE_PORT(unit, port)) {
            pif = SOC_PORT_IF_GMII;
        } else if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) {
            if (local_pa.interface & SOC_PA_INTF_XGMII) {
                pif = SOC_PORT_IF_XGMII;
            } else { /*  external GbE phy in xe port mode */
                pif = SOC_PORT_IF_SGMII;
            }
        } else {
            pif = SOC_PORT_IF_MII;
        }
    } else {
        pif = SOC_PORT_IF_TBI;
    }

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_interface_set(unit, port, pif));
    SOC_IF_ERROR_RETURN
        (MAC_INTERFACE_SET(PORT(unit, port).p_mac, unit, port, pif));

    if (IS_ST_PORT(unit, port)) {
        
        /* Since stacking port doesn't support flow control,
         * make sure that PHY is not advertising flow control capabilities.
         */
        SOC_IF_ERROR_RETURN(
            soc_phyctrl_ability_advert_get(unit, port, &advert_pa));
        advert_pa.pause &= ~(SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM);
        SOC_IF_ERROR_RETURN(
            soc_phyctrl_ability_advert_set(unit, port, &advert_pa));
    }

    if (!SOC_WARM_BOOT(unit) && 
        !(SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), port)) ) {
        SOC_IF_ERROR_RETURN
            (MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, enable));
    }
	}

    return BCM_E_NONE;
}
#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_VERSION_1_1                SOC_SCACHE_VERSION(1,1)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_1

STATIC int
_bcm_port_vd_pbvl_reinit(int unit)
{

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        /*
         * Reinit of vd_pbvl takes place at the end of function
         * _bcm_trx_vlan_action_profile_init().  This routine initializes
         * the VLAN profile tables first, which need to happen before
         * the vd_pbvl data is recovered.
         */
        return BCM_E_NONE;
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        bcm_port_t port;
        _bcm_port_info_t *pinfo; 
        int        idxmin, idx, i, index;
        int start, end;
        vlan_data_entry_t     vde;
        vlan_protocol_entry_t vpe;
        bcm_vlan_t            cvid, defvid;
        uint32                ft;

        idxmin = soc_mem_index_min(unit, VLAN_PROTOCOL_DATAm);
        idx = soc_mem_index_min(unit, VLAN_PROTOCOLm);
        PBMP_ALL_ITER(unit, port) {

            start = idxmin + port*16;
            end   = start + 16;
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_untagged_vlan_get(unit, port, &defvid));
            for (i = start; i < end; i++) {

                SOC_IF_ERROR_RETURN
                    (READ_VLAN_PROTOCOL_DATAm(unit, MEM_BLOCK_ANY,
                                              i, &vde));
                cvid = soc_VLAN_PROTOCOL_DATAm_field32_get(unit, &vde,
                                                           VLAN_IDf);
                if ((cvid == 0) || (cvid == defvid)) {
                    
                    continue;
                }

                /*
                 * Check against the VLAN_PROTOCOL table to see if a
                 * valid entry exists for matching the packet protocol
                 */
                index = i - start;
                SOC_IF_ERROR_RETURN
                    (READ_VLAN_PROTOCOLm(unit, MEM_BLOCK_ANY,
                                         idx+index, &vpe));
                ft = 0;
                if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, ETHERIIf)) {
                    ft |= BCM_PORT_FRAMETYPE_ETHER2;
                }
                if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, SNAPf)) {
                    ft |= BCM_PORT_FRAMETYPE_8023;
                }
                if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, LLCf)) {
                    ft |= BCM_PORT_FRAMETYPE_LLC;
                }
                if (ft == 0) {
                    /*
                     * Something wrong here.. bail out
                     */
                    continue;
                }
                BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
                if (!_BCM_PORT_VD_PBVL_IS_SET(pinfo, idx+index)) {
                    /* Set as explicit VID */
                    _BCM_PORT_VD_PBVL_SET(pinfo, idx+index);
                }
            }
        }
        return BCM_E_NONE;
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_DRACO15_SUPPORT
    if (SOC_IS_DRACO15(unit)) {
        bcm_port_t port;
        int        idxmin, idx, i, index;
        int        idxmax;
        _bcm_port_info_t *pinfo;
        int        vlan_prot_entries, vlan_data_prot_start;
        int start, end;
        vlan_protocol_entry_t   vpe;
        vlan_data_entry_t       vde;
        bcm_vlan_t              cvid, defvid;
        uint32                  ft;

        idxmin = soc_mem_index_min(unit, VLAN_DATAm);
        idxmax = soc_mem_index_max(unit, VLAN_PROTOCOLm);
        vlan_prot_entries = idxmax + 1;
        vlan_data_prot_start = soc_mem_index_max(unit, VLAN_SUBNETm) + 1;
        idx = soc_mem_index_min(unit, VLAN_PROTOCOLm);

        /*
         * Set VLAN ID for target port. For all other GE ports,
         * set default VLAN ID in entries indexed by the matched entry in
         * VLAN_PROTOCOL.
         */
        PBMP_E_ITER(unit, port) {
            start = idxmin + (port * vlan_prot_entries);
            end = start + vlan_prot_entries;
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_untagged_vlan_get(unit, port, &defvid));
            for (i = start; i < end; i++) {
                SOC_IF_ERROR_RETURN
                    (READ_VLAN_DATAm(unit, MEM_BLOCK_ANY, i, &vde));
                cvid = soc_VLAN_DATAm_field32_get(unit, &vde, VLAN_IDf);
                if ((cvid == 0) || (cvid == defvid)) {
                    continue;
                }

                /*
                 * Check against the VLAN_PROTOCOL table to see if a
                 * valid entry exists for matching the packet protocol
                 */
                index = i - start;
                SOC_IF_ERROR_RETURN
                    (READ_VLAN_PROTOCOLm(unit, MEM_BLOCK_ANY,
                                         idx+index, &vpe));
                ft = 0;
                if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, ETHERIIf)) {
                    ft |= BCM_PORT_FRAMETYPE_ETHER2;
                }
                if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, SNAPf)) {
                    ft |= BCM_PORT_FRAMETYPE_8023;
                }
                if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, LLCf)) {
                    ft |= BCM_PORT_FRAMETYPE_LLC;
                }
                if (ft == 0) {
                    /*
                     * Something wrong here.. bail out
                     */
                    continue;
                }
                BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));

                if (!_BCM_PORT_VD_PBVL_IS_SET(pinfo, idx+index)) {
                    /* Set as explicit VID */
                    _BCM_PORT_VD_PBVL_SET(pinfo, idx+index);
                }
            }
        }
        return BCM_E_NONE;
    }
#endif /* BCM_DRACO15_SUPPORT */

    return BCM_E_NONE;
}

#if defined(BCM_TRX_SUPPORT)
/* Recover the port dtag_mode from Warm Boot cached info, if available. */
STATIC int
_bcm_trx_port_dtag_mode_reinit(int unit, bcm_pbmp_t dtm_pbmp, int use_pbmp)
{
    uint32   tpid_enable;  
    port_tab_entry_t ptab;
    bcm_port_t port;
    _bcm_port_info_t *pinfo;
    bcm_vlan_action_set_t action;

    PBMP_ALL_ITER(unit, port) {
        BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &ptab));
        tpid_enable = soc_mem_field32_get(unit, PORT_TABm,
                                          &ptab, OUTER_TPID_ENABLEf);

        if (0 == tpid_enable) {
            if (use_pbmp && !BCM_PBMP_MEMBER(dtm_pbmp, port)) {
                /* Bitmap claims this is DT mode NONE, but it is
                 * configured for EXTERNAL */
                BCM_IF_ERROR_RETURN
                    (soc_event_generate(unit,
                                        BCM_SWITCH_EVENT_STABLE_ERROR, 
                                        SOC_STABLE_STALE,
                                        SOC_STABLE_BASIC, 0));
                use_pbmp = FALSE; /* Cached info is not trustworthy */
            }
            pinfo->dtag_mode = BCM_PORT_DTAG_MODE_EXTERNAL;
        } else {
            if (use_pbmp && BCM_PBMP_MEMBER(dtm_pbmp, port)) {
                pinfo->dtag_mode = BCM_PORT_DTAG_MODE_INTERNAL;
            } else {
                pinfo->dtag_mode = BCM_PORT_DTAG_MODE_NONE;
            }
        }

        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_default_action_get(unit, port, &action));

        if (bcmVlanActionAdd == action.ut_inner) {
            pinfo->dtag_mode |= BCM_PORT_DTAG_ADD_EXTERNAL_TAG;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_egress_default_action_get(unit, port,
                                                          &action));
        if ((bcmVlanActionDelete == action.dt_inner) &&
            (bcmVlanActionDelete == action.dt_inner_prio)) {
            pinfo->dtag_mode |= BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG;
        }
    }
    return BCM_E_NONE;
}

STATIC int      
_bcm_esw_port_e2ecc_reinit(int unit)
{
    bcm_port_t port;
    bcm_port_congestion_config_t *config;
    int    time_unit_sel = 0, time_units = 0;
    soc_higig_e2ecc_hdr_t e2ecc_hdr;
    int    src_modid, src_pid;
    uint64 regval64;
    uint32 regval, field_val;
    int tx_enabled = FALSE, rx_enabled = FALSE;
    int port_tx_enabled, port_rx_enabled, voqfc = 0;
    int blk, blk_num = -1, bindex = 0;

    if (SOC_IS_SC_CQ(unit)) {
        SOC_IF_ERROR_RETURN(READ_E2E_HOL_ENr(unit, &regval));
        if (0 != soc_reg_field_get(unit, E2E_HOL_ENr, regval, ENf)) {
            tx_enabled = TRUE;
        }
    } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
               SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
               SOC_IS_KATANA(unit)) { 
        SOC_IF_ERROR_RETURN(READ_E2ECC_HOL_ENr(unit, &regval));
        if (0 != soc_reg_field_get(unit, E2ECC_HOL_ENr, regval, ENf)) {
            tx_enabled = TRUE;
        }
    }

    SOC_IF_ERROR_RETURN(READ_ING_CONFIG_64r(unit, &regval64));
    if (0 == soc_reg64_field32_get(unit, ING_CONFIG_64r, regval64,
                              DISABLE_E2E_HOL_CHECKf)) {
        rx_enabled = TRUE;
    }


    if (!tx_enabled && !rx_enabled) {
        /* Nothing further to do */
        return BCM_E_NONE;
    }

    PBMP_HG_ITER(unit, port) {
        port_tx_enabled = port_rx_enabled = FALSE;
        
        if (tx_enabled) {
            if ((!SOC_IS_TD_TT(unit)) && (!SOC_IS_KATANA(unit))) {
                SOC_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &regval));
                port_tx_enabled =
                    (0 != soc_reg_field_get(unit, XPORT_CONFIGr, regval,
                                            E2E_HOL_ENf));
            } else {
                blk = SOC_PORT_BLOCK(unit, port);
                blk_num = SOC_BLOCK_INFO(unit, blk).number;
                bindex = SOC_PORT_BINDEX(unit, port);
                SOC_IF_ERROR_RETURN(READ_E2ECC_TX_ENABLE_BMPr(unit, blk_num,
                                                              &regval));
                if ((regval & (1 << bindex))) {
                    port_tx_enabled= TRUE;
                }
            }
        }

        if (rx_enabled) {
            SOC_IF_ERROR_RETURN(READ_IE2E_CONTROLr(unit, port, &regval));
            port_rx_enabled = 0;
            voqfc = 0;
            if (0 != soc_reg_field_get(unit, IE2E_CONTROLr, regval, HOL_ENABLEf)) {
                port_rx_enabled = 1;
            }
           
            if (SOC_IS_TD_TT(unit)) {
                if (0 != soc_reg_field_get(unit, IE2E_CONTROLr, regval, VOQFC_ENABLEf)) {
                    port_rx_enabled = 1;
                    voqfc = 1;
                }
            }
        }

        if (!port_tx_enabled && !port_rx_enabled) {
            /* On to the next port */
            continue;
        }

        /* Allocate space for the E2ECC configuration */
        if (PORT(unit, port).e2ecc_config == NULL) {
            PORT(unit, port).e2ecc_config =
                sal_alloc(sizeof(bcm_port_congestion_config_t),
                          "bcm_port_congestion_config");
            if (PORT(unit, port).e2ecc_config == NULL) {
                soc_cm_debug(DK_ERR,
                             "Error: unable to allocate memory for"
                             " bcm_port_congestion_config.\n");
                return BCM_E_MEMORY;
            }
        } else {
            /* This is initialized to NULL pointers by
             * port init.  How did it get changed? */
            return BCM_E_INTERNAL;
        }

        config = PORT(unit, port).e2ecc_config;
        bcm_port_congestion_config_t_init(config);

        if (port_tx_enabled) {
            /* Recover TX state */
            config->flags |= BCM_PORT_CONGESTION_CONFIG_E2ECC |
                BCM_PORT_CONGESTION_CONFIG_TX;

            /* Message timer paramters to packets/sec */
            if (SOC_IS_SC_CQ(unit)) {
                SOC_IF_ERROR_RETURN(READ_E2E_MAX_TX_TIMERr(unit, &regval));
                time_unit_sel = soc_reg_field_get(unit, E2E_MAX_TX_TIMERr,
                                                  regval, LGf);
                time_units = soc_reg_field_get(unit, E2E_MAX_TX_TIMERr,
                                               regval, TIMERf);
            } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                       SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
                       SOC_IS_KATANA(unit)) { 
                SOC_IF_ERROR_RETURN(READ_E2ECC_MAX_TX_TIMERr(unit, &regval));
                time_unit_sel = soc_reg_field_get(unit, E2ECC_MAX_TX_TIMERr,
                                                  regval, LGf);
                time_units = soc_reg_field_get(unit, E2ECC_MAX_TX_TIMERr,
                                               regval, TIMERf);
                SOC_IF_ERROR_RETURN(WRITE_E2ECC_MAX_TX_TIMERr(unit, regval));
            }

            /* See _bcm_esw_port_e2ecc_tx for an explanation of
             * the timer calculation. */
            config->packets_per_sec = time_unit_sel ?
                (40000 / time_units) : (4000000 / time_units);

            /* Recover the E2ECC module header
             * (see _bcm_esw_port_e2ecc_tx for the header format) */
            sal_memset(&e2ecc_hdr, 0, sizeof(soc_higig_e2ecc_hdr_t));
            SOC_IF_ERROR_RETURN
                (READ_XHOL_MH0r(unit, port, &e2ecc_hdr.overlay1.words[0]));
            SOC_IF_ERROR_RETURN
                (READ_XHOL_MH1r(unit, port, &e2ecc_hdr.overlay1.words[1]));
            SOC_IF_ERROR_RETURN
                (READ_XHOL_MH2r(unit, port, &e2ecc_hdr.overlay1.words[2]));
            SOC_IF_ERROR_RETURN
                (READ_XHOL_MH3r(unit, port, &e2ecc_hdr.overlay1.words[3]));

            SOC_IF_ERROR_RETURN
                (READ_XHOL_D0r(unit, port, &e2ecc_hdr.overlay1.words[4]));
            SOC_IF_ERROR_RETURN
                (READ_XHOL_D1r(unit, port, &e2ecc_hdr.overlay1.words[5]));
            SOC_IF_ERROR_RETURN
                (READ_XHOL_D2r(unit, port, &e2ecc_hdr.overlay1.words[6]));
            SOC_IF_ERROR_RETURN
                (READ_XHOL_D3r(unit, port, &e2ecc_hdr.overlay1.words[7]));

            config->traffic_class =
                (e2ecc_hdr.overlay1.words[0] >> 16) & 0x0F;
            config->multicast_id  = e2ecc_hdr.overlay1.words[0] & 0xFFFF;

            src_modid = (e2ecc_hdr.overlay1.words[1] >> 24) & 0xFF;
            src_pid = (e2ecc_hdr.overlay1.words[1] >> 16) & 0xFF;
            BCM_GPORT_MODPORT_SET(config->src_port, src_modid, src_pid);

            config->color = (e2ecc_hdr.overlay1.words[1] >> 6) & 0x3;

            config->pri = (e2ecc_hdr.overlay1.words[3] >> 29) & 0x7;
            config->cfi = (e2ecc_hdr.overlay1.words[3] >> 28) & 0x1;
            config->vlan = (e2ecc_hdr.overlay1.words[3] >> 16) & 0xfff;

            config->dest_mac[0] =
                ((e2ecc_hdr.overlay1.words[4] >> 24) & 0xff);
            config->dest_mac[1] =
                ((e2ecc_hdr.overlay1.words[4] >> 16) & 0xff);
            config->dest_mac[2] =
                ((e2ecc_hdr.overlay1.words[4] >> 8) & 0xff);
            config->dest_mac[3] =
                (e2ecc_hdr.overlay1.words[4] & 0xff);
            config->dest_mac[4] =
                ((e2ecc_hdr.overlay1.words[5] >> 24) & 0xff);
            config->dest_mac[5] =
                ((e2ecc_hdr.overlay1.words[5] >> 16) & 0xff);

            config->src_mac[0] =
                ((e2ecc_hdr.overlay1.words[5] >> 8) & 0xff);
            config->src_mac[1] =
                (e2ecc_hdr.overlay1.words[5] & 0xff);
            config->src_mac[2] =
                ((e2ecc_hdr.overlay1.words[6] >> 24) & 0xff);
            config->src_mac[3] =
                ((e2ecc_hdr.overlay1.words[6] >> 16) & 0xff);
            config->src_mac[4] =
                ((e2ecc_hdr.overlay1.words[6] >> 8) & 0xff);
            config->src_mac[5] =
                (e2ecc_hdr.overlay1.words[6] & 0xff);

            config->ethertype =
                (e2ecc_hdr.overlay1.words[7] >> 16) & 0xFFFF;
            config->opcode = e2ecc_hdr.overlay1.words[7] & 0xFFFF;

            /* All the info from RX is encoded in the header, so
             * just mark if it is enabled. */
            if (port_rx_enabled) {
                config->flags |= BCM_PORT_CONGESTION_CONFIG_RX;
            }
        } else if (port_rx_enabled) {
            /* Recover RX state */
            config->flags |= BCM_PORT_CONGESTION_CONFIG_RX;

            if (voqfc) {
                config->flags |= BCM_PORT_CONGESTION_CONFIG_DESTMOD_FLOW_CONTROL;
                SOC_IF_ERROR_RETURN(READ_ING_VOQFC_MACDA_MSr(unit, &regval));
                field_val = soc_reg_field_get(unit, ING_VOQFC_MACDA_MSr,
                                              regval, DAf);
                config->dest_mac[0] = ((field_val >> 8) & 0xff);
                config->dest_mac[1] = (field_val & 0xff);
        
                SOC_IF_ERROR_RETURN(READ_ING_VOQFC_MACDA_LSr(unit, &regval));
                field_val = soc_reg_field_get(unit, ING_VOQFC_MACDA_LSr,
                                              regval, DAf);
                config->dest_mac[2] = ((field_val >> 24) & 0xff);
                config->dest_mac[3] = ((field_val >> 16) & 0xff);
                config->dest_mac[4] = ((field_val >> 8) & 0xff);
                config->dest_mac[5] = (field_val & 0xff);

                SOC_IF_ERROR_RETURN(READ_ING_VOQFC_IDr(unit, &regval));
                config->ethertype =
                    soc_reg_field_get(unit, ING_VOQFC_IDr, regval, LENGTH_TYPEf);
                config->opcode =
                    soc_reg_field_get(unit, ING_VOQFC_IDr, regval, OPCODEf);

            } else {
                config->flags |= BCM_PORT_CONGESTION_CONFIG_E2ECC;
                SOC_IF_ERROR_RETURN(READ_E2E_HOL_RX_DA_MSr(unit, &regval)); 
                field_val = soc_reg_field_get(unit, E2E_HOL_RX_DA_MSr,
                                              regval, DAf);
                config->dest_mac[0] = ((field_val >> 8) & 0xff);
                config->dest_mac[1] = (field_val & 0xff);

                SOC_IF_ERROR_RETURN(READ_E2E_HOL_RX_DA_LSr(unit, &regval)); 
                field_val = soc_reg_field_get(unit, E2E_HOL_RX_DA_LSr,
                                              regval, DAf);
                config->dest_mac[2] = ((field_val >> 24) & 0xff);
                config->dest_mac[3] = ((field_val >> 16) & 0xff);
                config->dest_mac[4] = ((field_val >> 8) & 0xff);
                config->dest_mac[5] = (field_val & 0xff);

                SOC_IF_ERROR_RETURN
                    (READ_E2E_HOL_RX_LENGTH_TYPEr(unit, &regval));
                config->ethertype =
                    soc_reg_field_get(unit, E2E_HOL_RX_LENGTH_TYPEr,
                                      regval, LENGTH_TYPEf);

                SOC_IF_ERROR_RETURN
                    (READ_E2E_HOL_RX_OPCODEr(unit, &regval));
                config->opcode =
                    soc_reg_field_get(unit, E2E_HOL_RX_OPCODEr,
                                      regval, OPCODEf);
            }
        }
    }

    return BCM_E_NONE;
}
#endif /* BCM_TRX_SUPPORT */

int
_bcm_esw_port_sync(int unit)
{
#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        int alloc_sz, rv = BCM_E_NONE;
        soc_scache_handle_t scache_handle;
        bcm_port_t port;
        uint8 *port_state;
        _bcm_port_info_t *pinfo;
        bcm_pbmp_t dtm_pbmp;
        int stable_size;
#ifdef BCM_TRIDENT_SUPPORT
        soc_info_t *si = &SOC_INFO(unit);
#endif

        alloc_sz = 0;
        SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
        PBMP_ALL_ITER(unit, port) {
            alloc_sz += sizeof(int); /* vp count */
        }
        alloc_sz += sizeof(dtm_pbmp); /* Bitmap of dtag_mode set */

        PBMP_ALL_ITER(unit, port) {
            if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) && (stable_size > 0)) {
                if (SOC_REG_IS_VALID(unit, COSMASKr)) {
                    alloc_sz += sizeof(uint32);
                } else {
                    alloc_sz += sizeof(_bcm_port_metering_info_t);
                }
                alloc_sz += sizeof(uint8); /* flags */
            }  
        }

#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) && 
            soc_feature(unit, soc_feature_flex_port)) {
            /* Number of lanes per port needs to be stored.*/
            alloc_sz += (SOC_MAX_NUM_PORTS * sizeof(int)); 
        }
#endif

        SOC_SCACHE_HANDLE_SET(scache_handle,
                              unit, BCM_MODULE_PORT, 0);
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                     alloc_sz, (uint8**)&port_state, 
                                     BCM_WB_DEFAULT_VERSION, NULL);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }

        BCM_PBMP_CLEAR(dtm_pbmp);

        PORT_LOCK(unit);
        PBMP_ALL_ITER(unit, port) {
            BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
            sal_memcpy(port_state, &(pinfo->vp_count),
                       sizeof(pinfo->vp_count));
            port_state += sizeof(pinfo->vp_count);
            if (0 != (pinfo->dtag_mode &
                      (BCM_PORT_DTAG_MODE_INTERNAL |
                       BCM_PORT_DTAG_MODE_EXTERNAL))) {
                /* Some sort of double-tagged mode */
                BCM_PBMP_PORT_ADD(dtm_pbmp, port); 
            }
        }
        sal_memcpy(port_state, &dtm_pbmp, sizeof(dtm_pbmp));
        port_state += sizeof(dtm_pbmp);
        PBMP_ALL_ITER(unit, port) {
            if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) && (stable_size > 0)) {
                if (SOC_REG_IS_VALID(unit, COSMASKr)) {
                    sal_memcpy(port_state, &(PORT(unit, port).cosmask), sizeof(uint32));
                    port_state += sizeof(uint32);
                } else {
                    sal_memcpy(port_state, &(PORT(unit, port).m_info), 
                               sizeof(_bcm_port_metering_info_t));
                    port_state += sizeof(_bcm_port_metering_info_t);
                }
                sal_memcpy(port_state, &(PORT(unit, port).flags), sizeof(uint8));
                port_state += sizeof(uint8);
            } 
        }
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) &&
            soc_feature(unit, soc_feature_flex_port)) {
            PBMP_ALL_ITER(unit, port) {
                sal_memcpy(port_state, &(si->port_num_lanes[port]),
                            sizeof(int));
                port_state += sizeof(int);
            }
        }
#endif
        PORT_UNLOCK(unit);

        return rv;
    }
#endif /* BCM_TRX_SUPPORT */
    return BCM_E_NONE;
}

int
_bcm_esw_port_wb_alloc(int unit)
{
#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        int alloc_sz, rv = BCM_E_NONE;
        soc_scache_handle_t scache_handle;
        bcm_port_t port;
        uint8 *port_state;
        int stable_size;

        alloc_sz = 0;
        SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
        PBMP_ALL_ITER(unit, port) {
            alloc_sz += sizeof(int); /* vp count */
        }
        alloc_sz += sizeof(bcm_pbmp_t); /* Bitmap of dtag_mode == NONE */
        PBMP_ALL_ITER(unit, port) {
            if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) && (stable_size > 0)) {
                if (SOC_REG_IS_VALID(unit, COSMASKr)) {
                    alloc_sz += sizeof(uint32);
                } else {
                    alloc_sz += sizeof(_bcm_port_metering_info_t);
                }
                alloc_sz += sizeof(uint8); /* flags */
            }
        }

#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) && 
            soc_feature(unit, soc_feature_flex_port)) {
            alloc_sz += (SOC_MAX_NUM_PORTS * sizeof(int)); 
        }
#endif

        SOC_SCACHE_HANDLE_SET(scache_handle,
                              unit, BCM_MODULE_PORT, 0);
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, TRUE,
                                     alloc_sz, (uint8**)&port_state, 
                                     BCM_WB_DEFAULT_VERSION, NULL);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
    }
#endif /* BCM_TRX_SUPPORT */

    return BCM_E_NONE;
}

#ifdef BCM_TRIUMPH2_SUPPORT
/*
 * Function:
 *      _bcm_tr2_flexport_recover
 * Purpose:
 *     To recover/reconstruct the HL port configuration
 *     during warmboot. This is a 'level one' warmboot
 *     recovery. It depends on the reg configuration of
 *     EDATABUF_XQP_FLEXPORT_CONFIGr which maintains the 
 *     flexport config of the HL ports that was done
 *     in coldboot mode.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port number of the HL
 *             flexport controlling port. 
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr2_flexport_recover(int unit, bcm_port_t port) 
{
    uint8 rev_id;
    uint16 dev_id;
    uint32 port_type;
    int i, old_value;
    bcm_port_t it_port;
    port_tab_entry_t pent;
    soc_info_t *si = &SOC_INFO(unit);

    soc_cm_get_id(unit, &dev_id, &rev_id);
    BCM_IF_ERROR_RETURN(_bcm_tr2_port_lanes_get(unit, port, &old_value));

#define RECONFIGURE_TR2_PORT_TYPE_INFO(ptype) \
    si->ptype.num = 0; \
    si->ptype.min = si->ptype.max = -1; \
    PBMP_ITER(si->ptype.bitmap, it_port) { \
        si->ptype.port[si->ptype.num++] = it_port; \
        if (si->ptype.min < 0) { \
            si->ptype.min = it_port; \
        } \
        if (it_port > si->ptype.max) { \
            si->ptype.max = it_port; \
        } \
    }

    /* Step 1: Change the SOC bitmaps */
    if ((old_value == 1)) {
        /* The block originally had 4 GE ports */
        SOC_CONTROL_LOCK(unit);
        SOC_PBMP_PORT_REMOVE(si->st.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->hg.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->xe.bitmap, port);
        SOC_PBMP_PORT_ADD(si->ether.bitmap, port);
        SOC_PBMP_PORT_ADD(si->ge.bitmap, port);
        si->port_speed_max[port] = 1000;
        soc_port_cmap_set(unit, port, SOC_CTR_TYPE_GE);
        for (i = port + 1; i < port + 4; i++) {
            SOC_PBMP_PORT_REMOVE(si->ge.disabled_bitmap, i);
            SOC_PBMP_PORT_REMOVE(si->ether.disabled_bitmap, i);
            SOC_PBMP_PORT_REMOVE(si->port.disabled_bitmap, i);
            SOC_PBMP_PORT_REMOVE(si->all.disabled_bitmap, i);
            si->port_speed_max[i] = 1000;
            soc_port_cmap_set(unit, i, SOC_CTR_TYPE_GE);
        }
        RECONFIGURE_TR2_PORT_TYPE_INFO(ether);
        RECONFIGURE_TR2_PORT_TYPE_INFO(st);
        RECONFIGURE_TR2_PORT_TYPE_INFO(hg);
        RECONFIGURE_TR2_PORT_TYPE_INFO(xe);
        RECONFIGURE_TR2_PORT_TYPE_INFO(ge);
        SOC_CONTROL_UNLOCK(unit);
    } else if (old_value == 4) {
        /* The block was originally a single high-speed port */
        SOC_CONTROL_LOCK(unit);
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, SOC_PORT_MEM_TAB(unit, port),
                MEM_BLOCK_ANY, SOC_PORT_MOD_OFFSET(unit, port), &pent));
        port_type = soc_PORT_TABm_field32_get(unit, &pent, PORT_TYPEf);
        switch(port_type) {
          case 0 : /* xe */
            SOC_PBMP_PORT_ADD(si->xe.bitmap, port);
            SOC_PBMP_PORT_ADD(si->ether.bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->hg.bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->st.bitmap, port);
            break;
          case 1 : /* hg */
            SOC_PBMP_PORT_ADD(si->hg.bitmap, port);
            SOC_PBMP_PORT_ADD(si->st.bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->xe.bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->ether.bitmap, port);
          default: 
            break;
        }
        SOC_PBMP_PORT_REMOVE(si->ge.bitmap, port);
	switch (dev_id) {
	case BCM56526_DEVICE_ID:
	  si->port_speed_max[port] = 13000;
	  break;
	case BCM56636_DEVICE_ID:
	case BCM56638_DEVICE_ID:
	  si->port_speed_max[port] = 12000;
	  break;
	default:
	  si->port_speed_max[port] = 10000;
	}
        soc_port_cmap_set(unit, port, SOC_CTR_TYPE_XE);
        for (i = port + 1; i < port + 4; i++) {
            SOC_PBMP_PORT_ADD(si->ge.disabled_bitmap, i);
            SOC_PBMP_PORT_ADD(si->ether.disabled_bitmap, i);
            SOC_PBMP_PORT_ADD(si->port.disabled_bitmap, i);
            SOC_PBMP_PORT_ADD(si->all.disabled_bitmap, i);
        }
        RECONFIGURE_TR2_PORT_TYPE_INFO(ether);
        RECONFIGURE_TR2_PORT_TYPE_INFO(st);
        RECONFIGURE_TR2_PORT_TYPE_INFO(hg);
        RECONFIGURE_TR2_PORT_TYPE_INFO(ge);
        SOC_CONTROL_UNLOCK(unit);
    }
#undef RECONFIGURE_PORT_TYPE_INFO
    soc_dport_map_update(unit);

    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH2_SUPPORT */

int
_bcm_esw_port_wb_recover(int unit)
{
#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        int alloc_sz, use_pbmp = FALSE, rv = BCM_E_NONE;
        uint8 *port_state;
        uint16 recovered_ver;
        soc_scache_handle_t scache_handle;
        bcm_port_t port;
        _bcm_port_info_t *pinfo;
        bcm_pbmp_t dtm_pbmp;
        int stable_size;
#ifdef BCM_TRIUMPH2_SUPPORT
        pbmp_t port_bitmap;
#ifdef BCM_TRIDENT_SUPPORT
        soc_info_t *si = &SOC_INFO(unit);
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_TRIUMPH2_SUPPORT */

        alloc_sz = 0;
        SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
        PBMP_ALL_ITER(unit, port) {
            alloc_sz += sizeof(int); /* vp count */
        }
        alloc_sz += sizeof(bcm_pbmp_t); /* Bitmap of dtag_mode == NONE */
        PBMP_ALL_ITER(unit, port) {
            if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) && (stable_size > 0)) {
                if (SOC_REG_IS_VALID(unit, COSMASKr)) {
                    alloc_sz += sizeof(uint32);
                } else {
                    alloc_sz += sizeof(_bcm_port_metering_info_t);
                }
                alloc_sz += sizeof(uint8);
            }
        }

#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) &&
            soc_feature(unit, soc_feature_flex_port)) {
            alloc_sz += (SOC_MAX_NUM_PORTS * sizeof(int)); 
        }
#endif
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_PORT, 0);
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE,
                                     alloc_sz, (uint8**)&port_state, 
                                     BCM_WB_DEFAULT_VERSION, &recovered_ver);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            return rv;
        }
        if (BCM_SUCCESS(rv)) {
            PBMP_ALL_ITER(unit, port) {
                BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
                /* Recover VP reference counts */
                sal_memcpy(&(pinfo->vp_count), port_state,
                           sizeof(pinfo->vp_count));
                port_state += sizeof(int);
            }

            /* Recover dtag_mode port bitmap */
            sal_memcpy(&dtm_pbmp, port_state, sizeof(dtm_pbmp));
            port_state += sizeof(dtm_pbmp);
            use_pbmp = TRUE; /* Cached info available */
            
            if (recovered_ver >= BCM_WB_VERSION_1_1 ) {
                PBMP_ALL_ITER(unit, port) {
                    if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) && (stable_size > 0)) {
                        /* Recover port tx stop related info */ 
                        if (SOC_REG_IS_VALID(unit, COSMASKr)) {
                            sal_memcpy(&(PORT(unit, port).cosmask), port_state, 
                                       sizeof(uint32));
                            port_state += sizeof(uint32);
                        } else {
                            sal_memcpy(&(PORT(unit, port).m_info), port_state, 
                                       sizeof(_bcm_port_metering_info_t));
                            port_state += sizeof(_bcm_port_metering_info_t);
                        }
                        /* Recover flags */
                        sal_memcpy(&(PORT(unit, port).flags), port_state, sizeof(uint8));
                        port_state += sizeof(uint8);
                    }
                }
#ifdef BCM_TRIDENT_SUPPORT
                if (SOC_IS_TD_TT(unit) &&
                    soc_feature(unit, soc_feature_flex_port)) {
                    PBMP_ALL_ITER(unit, port) {
                        sal_memcpy(&(si->port_num_lanes[port]), port_state,
                                    sizeof(int));
                        port_state += sizeof(int);
                    }
                    /* Call PHY probe for config to take effect */
                    if ((rv = bcm_esw_port_probe(unit, PBMP_PORT_ALL(unit),
                           &port_bitmap)) != BCM_E_NONE) {
                        soc_cm_debug(DK_ERR, "Error unit %d:\
                                              Failed port probe: %s\n",
                                              unit, bcm_errmsg(rv));
                        return rv;
                    }
                }
#endif
            } else {
                uint32  port_scache_size = 0;
                PBMP_ALL_ITER(unit, port) {
                    if (!SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit) && (stable_size > 0)) {
                        /* Recover port tx stop related info */ 
                        if (SOC_REG_IS_VALID(unit, COSMASKr)) {
                            port_scache_size += sizeof(uint32);
                        } else {
                            port_scache_size += sizeof(_bcm_port_metering_info_t);
                        }
                        /* Recover flags */
                        port_scache_size += sizeof(uint8);
                    }
                }
                if (port_scache_size) {
                 SOC_IF_ERROR_RETURN
                    (soc_scache_realloc(unit, scache_handle, port_scache_size));
                }
            }
        } else {
            /*
             * No level-2 cache configured. Proceed to Level-1 only recovery
             */
            BCM_PBMP_CLEAR(dtm_pbmp); /* Default is dtag_mode == NONE */
        }

        /* Recover DT mode */
        rv = _bcm_trx_port_dtag_mode_reinit(unit, dtm_pbmp, use_pbmp);

#ifdef BCM_TRIUMPH2_SUPPORT
        if (BCM_SUCCESS(rv) &&
            (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
             SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
             SOC_IS_HURRICANE(unit) || SOC_IS_TD_TT(unit) ||
             SOC_IS_KATANA(unit))) {
            /* Recover ING_PRI_CNG_MAP references */
            port_tab_entry_t pent;
            int ptr;
            uint32 ipc_ref_bmp[3];
            /* Required: ING_PRI_CNG_MAP size <= 3 * 32 * 16 = 1536 */

            SHR_BITCLR_RANGE(ipc_ref_bmp, 0, 3*32);

            /* Read port pointer, recover reference count */
            PBMP_ALL_ITER(unit, port) {
                rv = soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY,
                                  SOC_PORT_MOD_OFFSET(unit, port), &pent);
                if (BCM_FAILURE(rv)) {
                    break;
                }
                if (soc_feature(unit, soc_feature_color)) {
                    ptr = soc_PORT_TABm_field32_get(unit, &pent,
                                                    TRUST_DOT1P_PTRf);
                    if (0 != ptr) {
                        rv = _bcm_ing_pri_cng_map_entry_reference(unit, 
                                  ptr * _BCM_TR2_PRI_CNG_MAP_SIZE,
                                        _BCM_TR2_PRI_CNG_MAP_SIZE);
                        if (BCM_FAILURE(rv)) {
                            break;
                        }
                    }
                }
                
                /* Recover PROTOCOL_PKT_CONTROL &
                 * IGMP_MLD_PKT_CONTROL references */
                if (soc_mem_field_valid(unit, PORT_TABm,
                                        PROTOCOL_PKT_INDEXf)) {
                    ptr = soc_PORT_TABm_field32_get(unit, &pent,
                                                    PROTOCOL_PKT_INDEXf);
                    if (0 != ptr) {
                        rv = _bcm_prot_pkt_ctrl_reference(unit, ptr);
                        if (BCM_FAILURE(rv)) {
                            break;
                        }
                        /* Remove default reference */
                        rv = _bcm_prot_pkt_ctrl_delete(unit, 0);
                        if (BCM_FAILURE(rv)) {
                            break;
                        }
                    }
                }
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

        if (BCM_SUCCESS(rv) &&
            (SOC_IS_SC_CQ(unit) || SOC_IS_TRIUMPH2(unit) ||
             SOC_IS_APOLLO(unit) || SOC_IS_VALKYRIE2(unit) ||
             SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit))) {
            rv = _bcm_esw_port_e2ecc_reinit(unit);
        }

#ifdef BCM_TRIUMPH2_SUPPORT
        /* Recover source modid egress blocking profiles */
        if (BCM_SUCCESS(rv) &&
            (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
             SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
             SOC_IS_KATANA(unit))) {
            uint32 regval, srcmod_index;

            PBMP_ALL_ITER(unit, port) {
                BCM_IF_ERROR_RETURN
                    (READ_SRC_MODID_EGRESS_SELr(unit, port, &regval));
                if (soc_reg_field_get(unit,
                            SRC_MODID_EGRESS_SELr, regval, ENABLEf)) { 
                    srcmod_index = soc_reg_field_get(unit,
                            SRC_MODID_EGRESS_SELr, regval, SRCMOD_INDEXf);
                    PORT_SRC_MOD_EGR_PROF_PTR(unit, port) = srcmod_index;
                    SRC_MOD_EGR_REF_COUNT(unit, srcmod_index)++;
                }
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
        if ((SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
             SOC_IS_VALKYRIE2(unit)) &&
            soc_feature(unit, soc_feature_flex_port)) {
            PBMP_ALL_ITER(unit, port) {
                if (_bcm_esw_valid_flex_port_controlling_port(unit, port)) {
                    BCM_IF_ERROR_RETURN(_bcm_tr2_flexport_recover(unit, port));
                }
            }
            /* Call PHY probe for config to take effect */
            if ((rv = bcm_esw_port_probe(unit, PBMP_PORT_ALL(unit),
                   &port_bitmap)) != BCM_E_NONE) {
                soc_cm_debug(DK_ERR, "Error unit %d:\
                                      Failed port probe: %s\n",
                                      unit, bcm_errmsg(rv));
                return rv;
            }
        }    
#endif /* BCM_TRIUMPH2_SUPPORT */

        return rv;
    }
#endif /* BCM_TRX_SUPPORT */
    return BCM_E_NONE;
}
#else
#define _bcm_port_vd_pbvl_reinit(u)  (BCM_E_NONE)
#define _bcm_esw_port_wb_recover(u)  (BCM_E_NONE)
#define _bcm_esw_port_wb_alloc(u) (BCM_E_NONE)
#endif /* BCM_WARM_BOOT_SUPPORT */


#if defined(BCM_DRACO15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
/*
 * Function:
 *      _bcm_port_vd_pbvl_init
 * Purpose:
 *      Initialization of vd_pbvl bitmap in port
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_MEMORY - failed to allocate required memory.
 */

STATIC int
_bcm_port_vd_pbvl_init(int unit)
{
    bcm_port_t port;
    int idxmax = soc_mem_index_count(unit, VLAN_PROTOCOLm);
    /* Round to the next entry */
    int inds_bytes = (idxmax + (_BCM_PORT_VD_PBVL_ESIZE -  1)) / \
                     _BCM_PORT_VD_PBVL_ESIZE;

    PBMP_ALL_ITER(unit, port) {
        PORT(unit, port).p_vd_pbvl = sal_alloc(inds_bytes, "vdv_info");
        if (NULL == PORT(unit, port).p_vd_pbvl) {
            return (BCM_E_MEMORY);
        }

        sal_memset(PORT(unit, port).p_vd_pbvl, 0, inds_bytes);
    }

    if (SOC_WARM_BOOT(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_port_vd_pbvl_reinit(unit));
    }

    return BCM_E_NONE;
}
#endif /* BCM_DRACO15_SUPPORT || BCM_FIREBOLT_SUPPORT */

/*
 * Function:
 *      _bcm_esw_port_software_detach
 * Purpose:
 *      De-initialization of software for port subsystem.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_INTERNAL- failed to write PTABLE entries
 *      BCM_E_MEMORY - failed to allocate required memory.
 */

int
_bcm_esw_port_software_detach(int unit)
{
    bcm_port_t          port;

    if (bcm_port_info[unit] == NULL) {
         return (BCM_E_NONE);
    }

    PBMP_ALL_ITER(unit, port) {
        if (NULL != PORT(unit, port).p_vd_pbvl) {
            sal_free(PORT(unit, port).p_vd_pbvl);
            PORT(unit, port).p_vd_pbvl = NULL;
        }

        if (NULL != PORT(unit, port).e2ecc_config) {
            sal_free(PORT(unit, port).e2ecc_config);
            PORT(unit, port).e2ecc_config = NULL;
        }
    }

    SOC_IF_ERROR_RETURN(soc_phy_common_detach(unit));

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
 || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_vlan_ctrl)) {
        BCM_IF_ERROR_RETURN(_bcm_fb2_outer_tpid_detach(unit));
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT \
          || BCM_RAPTOR_SUPPORT */


    sal_free (bcm_port_info[unit]);
    bcm_port_info[unit] = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_software_init
 * Purpose:
 *      Initialization of software for port subsystem.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_INTERNAL- failed to write PTABLE entries
 *      BCM_E_MEMORY - failed to allocate required memory.
 */

STATIC int
_bcm_port_software_init(int unit)
{
    bcm_port_t          port;

    if (bcm_port_info[unit] != NULL) {
        PBMP_ALL_ITER(unit, port) {
#ifdef BCM_FILTER_SUPPORT
            if (PORT(unit, port).p_ut_filter) {
                (void) bcm_esw_filter_destroy(unit, PORT(unit, port).p_ut_filter);
                PORT(unit, port).p_ut_filter = 0;
            }
#endif
#ifdef BCM_METER_SUPPORT
            if (PORT(unit, port).meter_dpid >= 0) {
                assert(PORT(unit, port).meter_cfid != -1);
                (void) bcm_esw_ds_datapath_delete(unit,
                                                  PORT(unit, port).meter_dpid);
            }
#endif
            if (PORT(unit, port).p_vd_pbvl != NULL) {
                sal_free(PORT(unit, port).p_vd_pbvl);
                PORT(unit, port).p_vd_pbvl = NULL;
            }

            if (PORT(unit, port).e2ecc_config != NULL) {
                sal_free(PORT(unit, port).e2ecc_config);
                PORT(unit, port).e2ecc_config = NULL;
            }

            PORT(unit, port).flags = 0;
        }
    }

    if (bcm_port_info[unit] == NULL) {
        bcm_port_info[unit] = sal_alloc(sizeof(_bcm_port_info_t) * SOC_MAX_NUM_PORTS,
                                        "bcm_port_info");
        if (bcm_port_info[unit] == NULL) {
            return BCM_E_MEMORY;
        }
    }
    sal_memset(bcm_port_info[unit], 0, sizeof(_bcm_port_info_t) * SOC_MAX_NUM_PORTS);

    SOC_IF_ERROR_RETURN(soc_phy_common_init(unit));

#ifdef BCM_METER_SUPPORT
    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        PORT(unit, port).meter_dpid = -1;
        PORT(unit, port).meter_cfid = -1;
    }
#endif

#if defined(BCM_DRACO15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_DRACO15(unit) || SOC_IS_FBX(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_port_vd_pbvl_init(unit));
    }
#endif /* BCM_DRACO15_SUPPORT || BCM_FIREBOLT_SUPPORT */

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
 || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_vlan_ctrl)) {
        BCM_IF_ERROR_RETURN(_bcm_fb2_outer_tpid_init(unit));
    }
    if (soc_feature(unit, soc_feature_color_prio_map)) {
        BCM_IF_ERROR_RETURN(_bcm_fb2_priority_map_init(unit));
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT \
          || BCM_RAPTOR_SUPPORT */

#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            PORT(unit, port).dtag_mode = BCM_PORT_DTAG_MODE_NONE;
        }
    }
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
#if defined(BCM_XGS_SWITCH_SUPPORT)
        if (soc_feature(unit, soc_feature_remap_ut_prio)) {
            bcm_port_cfg_t pcfg;
            PBMP_ALL_ITER(unit, port) {
                SOC_IF_ERROR_RETURN
                    (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port,
                                                          &pcfg));
                PORT(unit, port).p_ut_prio = pcfg.pc_new_opri;
            }
        }
#endif /* BCM_XGS_SWITCH_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
        if (SOC_IS_TRX(unit)) {
            PBMP_ALL_ITER(unit, port) {
                /* Recover port's dtag_mode */
                PORT(unit, port).dtag_mode = BCM_PORT_DTAG_MODE_NONE;
            }
        }
#endif
    }
#endif /* BCM_WARM_BOOT_SUPPORT */
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_settings_init
 * Purpose:
 *      Initialize port settings if they are to be different from the
 *      default ones
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - port number
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_INTERNAL- failed to write PTABLE entries
 * Notes:
 *      This function initializes port settings based on the folowing config
 *      variables:
 *           port_init_speed
 *           port_init_duplex
 *           port_init_adv
 *           port_init_autoneg
 *      If a variable is not set, then no additional initialization of the
 *      corresponding parameter is done (and the defaults will normally be
 *      advertize everything you can do and use autonegotiation).
 *
 *      A typical use would be to set:
 *          port_init_adv=0
 *          port_init_autoneg=1
 *      to force link down in the beginning.
 *
 *      Another setup that makes sense is something like:
 *          port_init_speed=10
 *          port_init_duplex=0
 *          port_init_autoneg=0
 *      in order to force link into a certain mode. (It is very important to
 *      disable autonegotiation in this case).
 *
 *      PLEASE NOTE:
 *          The standard rc.soc forces autoneg=on on all the ethernet ports
 *          (FE and GE). Thus, to use the second example one has to edit rc.soc
 *          as well.
 *
 *     This function has been declared as global, but not exported. This will
 *     make port initialization easier when using VxWorks shell.
 */
int
bcm_port_settings_init(int unit, bcm_port_t port)
{
    int             val;
    bcm_port_info_t info;

    SOC_DEBUG_PRINT((DK_PORT,
        "bcm_port_settings_init: u=%d p=%d\n",unit, port));

    bcm_port_info_t_init(&info);
    
    val = soc_property_port_get(unit, port, spn_PORT_INIT_SPEED, -1);
    if (val != -1) {
        info.speed = val;
        info.action_mask |= BCM_PORT_ATTR_SPEED_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_DUPLEX, -1);
    if (val != -1) {
        info.duplex = val;
        info.action_mask |= BCM_PORT_ATTR_DUPLEX_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_ADV, -1);
    if (val != -1) {
        info.local_advert = val;
        info.action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_AUTONEG, -1);
    if (val != -1) {
        info.autoneg = val;
        info.action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
    }

    return bcm_esw_port_selective_set(unit, port, &info);
}

#ifdef BCM_TRIUMPH2_SUPPORT
/* Helper function to program logical to physical (and reverse) port mapping */
STATIC int
_bcm_port_remap_set(int unit, bcm_port_t phys, bcm_port_t logical)
{
    port_tab_entry_t ptab;
    sys_portmap_entry_t sys_portmap;
    uint32 regval;

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm,
                                     MEM_BLOCK_ANY, phys, &ptab));
    soc_PORT_TABm_field32_set(unit, &ptab, SRC_SYS_PORT_IDf, logical);
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, PORT_TABm,
                                      MEM_BLOCK_ALL, phys, &ptab));

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, SYS_PORTMAPm,
                                     MEM_BLOCK_ANY, logical, &sys_portmap));
    soc_SYS_PORTMAPm_field32_set(unit, &sys_portmap, PHYS_PORT_IDf, phys);
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, SYS_PORTMAPm,
                                      MEM_BLOCK_ALL, logical, &sys_portmap));

    /* Configure E2ECC port mapping register */
    SOC_IF_ERROR_RETURN(READ_E2ECC_PORT_MAPPINGr(unit, phys, &regval));
    soc_reg_field_set(unit, E2ECC_PORT_MAPPINGr, &regval, MAPPED_PORT_NUMf, logical);
    SOC_IF_ERROR_RETURN(WRITE_E2ECC_PORT_MAPPINGr(unit, phys, regval));

    return BCM_E_NONE; 
}

STATIC int
_bcm_tr2_system_tpid_init(int unit)
{
    int tpid_index, sys_index, rv = BCM_E_NONE;
    bcm_module_t module;
    bcm_port_t port;
    system_config_table_entry_t systab; 
    uint32 tpid_enable;
    uint16 tpid;

    tpid = _bcm_fb2_outer_tpid_default_get(unit);
    BCM_IF_ERROR_RETURN(_bcm_fb2_outer_tpid_lkup(unit, tpid, &tpid_index));

    for (module = 0; module <= SOC_MODID_MAX(unit); module++) {
        for (port = 0; port < 64; port++) {
            sys_index = module * 64 + port; 
            rv = READ_SYSTEM_CONFIG_TABLEm(unit, MEM_BLOCK_ANY, 
                                           sys_index, &systab);
            BCM_IF_ERROR_RETURN(rv);
            tpid_enable = (1 << tpid_index);
            soc_SYSTEM_CONFIG_TABLEm_field32_set(unit, &systab, 
                                                 OUTER_TPID_ENABLEf, 
                                                 tpid_enable);
            rv = WRITE_SYSTEM_CONFIG_TABLEm(unit, MEM_BLOCK_ALL, 
                                            sys_index, &systab);
            BCM_IF_ERROR_RETURN(rv);
        }
    }
    return rv;
}
#endif

/*
 * Function:
 *      _bcm_esw_port_deinit
 * Purpose:
 *      De-initialize the PORT interface layer for the specified SOC device.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_port_deinit(int unit)
{
    int  rv;

    if (NULL == bcm_port_info[unit]) {
        return (BCM_E_NONE);
    }

    rv = _bcm_esw_port_software_detach(unit);
    BCM_IF_ERROR_RETURN(rv);

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_vlan_action)) {
#ifdef BCM_TRIUMPH2_SUPPORT
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
            SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
            SOC_IS_TD_TT(unit) || SOC_IS_HURRICANE(unit) ||
            SOC_IS_KATANA(unit)) {
            _bcm_tr2_port_vpd_bitmap_free(unit);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

        /* Initialize the vlan action profile table */
        rv = _bcm_trx_vlan_action_profile_detach(unit);
        BCM_IF_ERROR_RETURN(rv);
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_gport_service_counters)) {
        rv = _bcm_esw_flex_stat_detach(unit, _bcmFlexStatTypeGport);
        BCM_IF_ERROR_RETURN(rv);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit) &&
        (soc_feature(unit,soc_feature_timesync_support))) {
        _bcm_esw_port_timesync_profile_delete(unit);
    }
#endif /* defined(BCM_KATANA_SUPPORT) */


    return BCM_E_NONE;
}

#ifdef BCM_TRIDENT_SUPPORT
/*
 * Function:
 *      _bcm_td_phymode_reconfigure
 * Purpose:
 *      Reconfigure the mode of the phys during warmboot.
 *      This function has code copied from _soc_trident_misc_init
 *      to keep the phy modes consistent in cold and warmboot modes. 
 *      This is required to be called before the phy probe routines
 *      in order for the mode to be configured correctly.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_td_phymode_reconfigure(int unit) {

    soc_info_t *si = &SOC_INFO(unit);
    uint32 core_mode, phy_mode, rval;
    int port, blk, blk_port, num_lanes, phy_port, bindex;
    static const soc_field_t port_field[4] = {
        PORT0f, PORT1f, PORT2f, PORT3f
    };
    static const soc_field_t mac_mode_field[4] = {
        PORT0_MAC_MODEf, PORT1_MAC_MODEf, PORT2_MAC_MODEf, PORT3_MAC_MODEf
    };

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_XLPORT) {
        blk_port = SOC_BLOCK_PORT(unit, blk);
        if (blk_port < 0) {
            continue;
        }
        /*
         * Program XLPORT mode and default Warpcore lanes setting
         * core_mode: the number of ports on the system side of XMAC
         * phy_mode: the number of ports on the Warpcore side of XMAC
         * num_lanes: the number of lanes on the line side of Warpcore
         */
        if (si->port_speed_max[blk_port] > 20000) {
            core_mode = 0; /* single, XMAC works as a single MAC */
            phy_mode = 0; /* single, use all 16 lanes for a port */
            num_lanes = 4;
        } else if (si->port_speed_max[blk_port] > 10000) {
            core_mode = 1; /* dual, XMAC works as 2 MACs */
            phy_mode = 1; /* dual, use 8 lanes per port  */
            num_lanes = 2;
        } else {
            core_mode = 2; /* quad, XMAC works as 4 MACs */
            phy_mode = 2; /* quad, use 4 lanes per port */
            num_lanes = 1;
        }
        rval = 0;
        soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval, CORE_PORT_MODEf,
                          core_mode);
        soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval, PHY_PORT_MODEf,
                          phy_mode);
        phy_port = ((si->port_l2p_mapping[blk_port] - 1) & ~0x3) + 1;
        for (bindex = 0; bindex < 4; bindex++) {
            port = si->port_p2l_mapping[phy_port + bindex];
            if (port == -1) {
                continue;
            }
            if (si->port_speed_max[port] < 10000) {
                soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval,
                                  mac_mode_field[bindex], 1);
            }
            si->port_num_lanes[port] = num_lanes;
        }
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_MODE_REGr(unit, blk_port, rval));
        
        /* Enable XLPORT */
        rval = 0;
        for (bindex = 0; bindex < 4; bindex++) {
            if (si->port_p2l_mapping[phy_port + bindex] != -1) {
                soc_reg_field_set(unit, XLPORT_PORT_ENABLEr, &rval,
                port_field[bindex], 1);
            }
        }
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_PORT_ENABLEr(unit, blk_port, rval));
    }
    return BCM_E_NONE;
}
#endif

/*
 * Function:
 *      bcm_port_init
 * Purpose:
 *      Initialize the PORT interface layer for the specified SOC device.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_INTERNAL- failed to write PTABLE entries
 *      BCM_E_MEMORY - failed to allocate required memory.
 * Notes:
 *      By default ports come up enabled. They can be made to come up disabled
 *      at startup by a compile-time application policy flag in your Make.local
 *      PTABLE initialized.
 */

int
bcm_esw_port_init(int unit)
{
    int                 rv, port_enable, length_check;
    bcm_port_t          p;
    pbmp_t              okay_ports;
    pbmp_t              temp;
    bcm_vlan_data_t     vd;
    char                pfmtok[SOC_PBMP_FMT_LEN],
                        pfmtall[SOC_PBMP_FMT_LEN];
    bcm_pbmp_t bmp;


    soc_cm_debug(DK_VERBOSE, "bcm_port_init: unit %d\n", unit);
    
    assert(unit < BCM_MAX_NUM_UNITS);

    if ((rv = _bcm_port_software_init(unit)) != BCM_E_NONE) {
        soc_cm_debug(DK_ERR,
                     "Error unit %d:  Failed software port init: %s\n",
                     unit, bcm_errmsg(rv));
        return rv;
    }

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_vlan_action)) {
        bcm_vlan_action_set_t action;

        bcm_vlan_action_set_t_init(&action);

#ifdef BCM_TRIUMPH2_SUPPORT
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
            SOC_IS_TD_TT(unit) || SOC_IS_HURRICANE(unit) ||
            SOC_IS_KATANA(unit)) {
            BCM_IF_ERROR_RETURN(_bcm_tr2_port_vpd_bitmap_alloc(unit)); 
        }
#endif

        PBMP_ALL_ITER(unit, p) {
            if (!IS_LB_PORT(unit, p)) {
#ifdef BCM_TRIUMPH2_SUPPORT
                if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
                    SOC_IS_TD_TT(unit) || SOC_IS_HURRICANE(unit) ||
                    SOC_IS_KATANA(unit)) {
                    BCM_IF_ERROR_RETURN
                        (_bcm_port_vlan_prot_index_alloc
                            (unit, &(PORT(unit, p).vlan_prot_ptr)));
                } else
#endif
                {
                    PORT(unit, p).vlan_prot_ptr = p * soc_mem_index_count
                                                  (unit, VLAN_PROTOCOLm);
                }
                PORT(unit, p).vp_count = 0;
            }
        }

        /* Initialize the vlan action profile table */
        BCM_IF_ERROR_RETURN (_bcm_trx_vlan_action_profile_init(unit));

        PBMP_HG_ITER(unit, p) {
            BCM_IF_ERROR_RETURN
                (_bcm_trx_vlan_port_egress_default_action_get(unit,
                                                             p, &action));
            /* Backward compatible defaults */
            action.ot_outer = bcmVlanActionDelete;
            action.dt_outer = bcmVlanActionDelete;
            BCM_IF_ERROR_RETURN
                (_bcm_trx_vlan_port_egress_default_action_set(unit,
                                                             p, &action));
        }
    }
#endif

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_gport_service_counters)) {
        if (SOC_WARM_BOOT(unit)) {
            port_tab_entry_t ptab;
            int fs_idx;
            bcm_gport_t gport;
            
#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit)) {
                BCM_IF_ERROR_RETURN(_bcm_td_phymode_reconfigure(unit));
            }
#endif
            PBMP_ALL_ITER(unit, p) {
                BCM_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm,
                                                 MEM_BLOCK_ANY, p, &ptab));
                fs_idx =
                    soc_mem_field32_get(unit, PORT_TABm, &ptab,
                                        VINTF_CTR_IDXf);
                if (fs_idx) {
                    BCM_IF_ERROR_RETURN
                        (bcm_esw_port_gport_get(unit, p, &gport));
                    _bcm_esw_flex_stat_reinit_add(unit,
                             _bcmFlexStatTypeGport, fs_idx, gport);
                }
            }
        }
    }
#endif


    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RCPU_ONLY(unit)) {
        /*
         * Write port configuration tables to contain the Initial System
         * Configuration (see init.c).
         */
	BCM_PBMP_ASSIGN(temp, PBMP_ALL(unit));
	BCM_PBMP_REMOVE(temp, PBMP_TDM(unit));

        vd.vlan_tag = BCM_VLAN_DEFAULT;
        BCM_PBMP_ASSIGN(vd.port_bitmap, temp);
        BCM_PBMP_ASSIGN(vd.ut_port_bitmap, temp);
        BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_CMIC(unit));

	SOC_PBMP_CLEAR(bmp);
	SOC_PBMP_ASSIGN(bmp, PBMP_ALL(unit));
	SOC_PBMP_REMOVE(bmp, PBMP_TDM(unit));
	PBMP_ITER(bmp, p) { 
            BCM_IF_ERROR_RETURN
                (mbcm_driver[unit]->mbcm_port_cfg_init(unit, p, &vd));
        }
    
        /*
         * Clear egress port blocking table
         */
        if (SOC_IS_EASYRIDER(unit)) {
#ifdef BCM_EASYRIDER_SUPPORT
            int i;

            for (i = 0; i < SOC_REG_NUMELS(unit, MAC_BLOCK_TABLEr); i++) {
                SOC_IF_ERROR_RETURN(WRITE_MAC_BLOCK_TABLEr(unit, i, 0));
            }
#endif /* BCM_EASYRIDER_SUPPORT */
        } else if (SOC_IS_XGS_SWITCH(unit)) {
#ifdef BCM_SHADOW_SUPPORT
            if (!SOC_IS_SHADOW(unit)) {
#endif
                if (!SAL_BOOT_SIMULATION || SAL_BOOT_QUICKTURN) {
                    SOC_IF_ERROR_RETURN
                        (soc_mem_clear(unit, MAC_BLOCKm, COPYNO_ALL, TRUE));
                }
#ifdef BCM_SHADOW_SUPPORT
            }
#endif
        }

#if defined(BCM_TUCANA_SUPPORT)
        /* Initialize DSCP-based priority mapping... */
        if (SOC_IS_TUCANA(unit)) {
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit, DSCP_PRIORITY_TABLEm, COPYNO_ALL, TRUE));
        }
#endif /* BCM_TUCANA_SUPPORT */
    }

    /* Probe for ports */
    SOC_PBMP_CLEAR(okay_ports);
    if ((rv = bcm_esw_port_probe(unit, PBMP_PORT_ALL(unit), &okay_ports)) !=
        BCM_E_NONE) {
        soc_cm_debug(DK_ERR,
                     "Error unit %d:  Failed port probe: %s\n",
                     unit, bcm_errmsg(rv));
        return rv;
    }
    
    soc_cm_debug(DK_VERBOSE, "Probed ports okay: %s of %s\n",
                 SOC_PBMP_FMT(okay_ports, pfmtok),
                 SOC_PBMP_FMT(PBMP_PORT_ALL(unit), pfmtall));


    length_check = soc_property_get(unit, spn_MAC_LENGTH_CHECK_ENABLE, 0);

    /* Probe and initialize MAC and PHY drivers for ports that were OK */
    PBMP_ITER(okay_ports, p) {
        soc_cm_debug(DK_PORT | DK_VERBOSE, "bcm_port_init: unit %d port %s\n",
                     unit, SOC_PORT_NAME(unit, p));

        if (SOC_IS_RCPU_ONLY(unit)) {
            /* No other port initialization necessary */
            soc_phyctrl_enable_set(unit, p, TRUE);
            continue;
        }

        PORT_LOCK(unit);
        if ((rv = _bcm_port_mode_setup(unit, p, TRUE)) < 0) {
            soc_cm_debug(DK_WARN, "Warning: Unit %d Port %s: "
                         "Failed to set initial mode: %s\n",
                         unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv));
        }
        PORT_UNLOCK(unit);

        if (SOC_WARM_BOOT(unit)) {
            continue;
        }

        if ((rv = bcm_port_settings_init(unit, p)) < 0) {
            soc_cm_debug(DK_WARN, "Warning: Unit %d Port %s: "
                         "Failed to configure initial settings: %s\n",
                         unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv));
        }

        /* Control 802.3 frame length field check in MAC for XL or CL blocks */
        if (IS_XL_PORT(unit, p)) {
            uint32 rval;

            SOC_IF_ERROR_RETURN(READ_MAC_RSV_MASKr(unit, p, &rval));
            if (length_check) {
                rval |= 0x20;  /* set bit 5 to enable frame length check */
            } else {
                rval &= ~0x20; /* clear bit 5 to disable frame length check */
            }
            SOC_IF_ERROR_RETURN(WRITE_MAC_RSV_MASKr(unit, p, rval));
        }

        /*
         * A compile-time application policy may prefer to disable ports 
         * when switch boots up
         */

#ifdef BCM_PORT_DEFAULT_DISABLE
            port_enable = FALSE;
#else
            port_enable = TRUE;
#endif  /* BCM_PORT_DEFAULT_DISABLE */
        if ((rv = bcm_esw_port_enable_set(unit, p, port_enable)) < 0) {
            soc_cm_debug(DK_WARN, "Warning: Unit %d Port %s: "
                         "Failed to %s port: %s\n",
                         unit, SOC_PORT_NAME(unit, p),(port_enable) ? "enable" : "disable" ,bcm_errmsg(rv));
        }

        /*
         *  JAM should be enabled by default
         */
        if (IS_E_PORT(unit, p)) {     
            if (soc_reg_field_valid(unit, CONFIGr, JAM_ENf) || 
                soc_mem_field_valid(unit, PORT_TABm, JAM_ENf) ||
                soc_reg_field_valid(unit, GE_PORT_CONFIGr, JAM_ENf) ||
                soc_feature(unit, soc_feature_unimac)) {

#if defined(BCM_HURRICANE_SUPPORT)
				if (SOC_IS_HURRICANE(unit)) {
					rv = bcm_esw_port_jam_set(unit, p, 0);
				} else
#endif
				{
					rv = bcm_esw_port_jam_set(unit, p, 1);
				}
                if (BCM_FAILURE(rv) && (rv != BCM_E_UNAVAIL)) {
                    soc_cm_debug(DK_WARN, "Warning: Unit %d Port %s: "
                                 "Failed to enable JAM %s\n", unit, 
                                 SOC_PORT_NAME(unit, p), bcm_errmsg(rv));
                }
            }
        }
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_unimac_tx_crs) && IS_XQ_PORT(unit, p) 
            && IS_GE_PORT(unit, p)) {
            BCM_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, XPORT_CONFIGr, p, 
                                        UNIMAC_HD_ECO_ENABLEf, 1)); 
        }
#endif
#ifdef INCLUDE_RCPU
        if ((uint32)p == soc_property_get(unit, spn_RCPU_PORT, -1)) {
            bcm_esw_port_frame_max_set(unit, p, BCM_PORT_JUMBO_MAXSZ);
        }
#endif /* INCLUDE_RCPU */
    }

#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        BCM_IF_ERROR_RETURN(bcm_td_port_init(unit));
    }
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RELOADING(unit) && !SOC_IS_RCPU_ONLY(unit)) {
        /* Enable the default outer TPID. */
        if (soc_feature(unit, soc_feature_vlan_ctrl)) {
        /* if (soc_feature(unit, soc_feature_vlan_ctrl) && * SOC_IS_SHADOW(unit)) { */

	    SOC_PBMP_CLEAR(bmp);
	    SOC_PBMP_ASSIGN(bmp, PBMP_ALL(unit));
	    SOC_PBMP_REMOVE(bmp, PBMP_TDM(unit));
	    PBMP_ITER(bmp, p) { 
                 rv = bcm_esw_port_tpid_set(unit, p,
                                    _bcm_fb2_outer_tpid_default_get(unit));
                 if (BCM_FAILURE(rv)) {
                     soc_cm_debug(DK_WARN, "Warning: Unit %d : "
                              "Failed to set %s port default TPID: %s\n",
                              unit, SOC_PORT_NAME(unit, p), bcm_errmsg(rv));
                 }
             }
         }
    }
#endif

#ifdef BCM_XGS3_FABRIC_SUPPORT
    if (SOC_IS_XGS3_FABRIC(unit)) {
        PBMP_ALL_ITER(unit, p) {
            bcm_esw_port_learn_set(unit, p, BCM_PORT_LEARN_FWD);
        }
    }
#endif

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANE(unit) ||
        SOC_IS_KATANA(unit)) {
        port_tab_entry_t ptab;
        uint32 regval, profile_index;

        /* Set the chip-wide E2ECC port mapping enable bit */ 
        if (soc_feature(unit, soc_feature_sysport_remap)) {
            SOC_IF_ERROR_RETURN(READ_E2ECC_PORT_MAPPING_CONFIGr(unit, &regval));
            soc_reg_field_set(unit, E2ECC_PORT_MAPPING_CONFIGr, &regval, PORT_MAPPING_ENf, 1);
            SOC_IF_ERROR_RETURN(WRITE_E2ECC_PORT_MAPPING_CONFIGr(unit, regval));
        }

        PBMP_ALL_ITER(unit, p) {
            /* Initialize the logical to physical port mapping */
            if (!SOC_IS_ENDURO(unit) && !SOC_IS_HURRICANE(unit) &&
                !SOC_IS_TD_TT(unit) && !SOC_IS_KATANA(unit)) {
                bcm_port_t sp = p;
                if (soc_feature(unit, soc_feature_sysport_remap)) {
                    BCM_XLATE_SYSPORT_P2S(unit, &sp);
                }
                _bcm_port_remap_set(unit, p, sp);
            }

            /* Initialize egress block profile pointer to invalid value */
            PORT_SRC_MOD_EGR_PROF_PTR(unit, p) = -1;

            /* Program the VLAN_PROTOCOL_DATA / FP_PORT_FIELD_SEL_INDEX 
               / PROTOCOL_PKT_INDEX pointers */
            if (!IS_LB_PORT(unit, p) && !IS_TDM_PORT(unit, p)) {
                BCM_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm,
                                    MEM_BLOCK_ANY, p, &ptab));
                soc_PORT_TABm_field32_set(unit, &ptab, 
                    VLAN_PROTOCOL_DATA_INDEXf, PORT(unit, p).vlan_prot_ptr 
                        / soc_mem_index_count(unit, VLAN_PROTOCOLm));

                if (soc_mem_field_valid(unit, PORT_TABm, FP_PORT_FIELD_SEL_INDEXf)) {
                    soc_PORT_TABm_field32_set(unit, &ptab, 
                                          FP_PORT_FIELD_SEL_INDEXf, p);
                }
                if (soc_mem_field_valid(unit, PORT_TABm,
                                        PROTOCOL_PKT_INDEXf)) {
                    if (SOC_REG_INFO(unit, PROTOCOL_PKT_CONTROLr).regtype ==
                        soc_portreg) {
                        soc_PORT_TABm_field32_set(unit, &ptab,
                                                  PROTOCOL_PKT_INDEXf, p);
                    } else {
                        BCM_IF_ERROR_RETURN
                            (_bcm_prot_pkt_ctrl_add(unit, 0, 0,
                                                    &profile_index));
                        soc_PORT_TABm_field32_set(unit, &ptab,
                                                  PROTOCOL_PKT_INDEXf,
                                                  profile_index);
                    }
                }

                BCM_IF_ERROR_RETURN(soc_mem_write(unit, PORT_TABm,
                                    MEM_BLOCK_ALL, p, &ptab));

                if (soc_feature(unit, soc_feature_embedded_higig) 
                    && IS_E_PORT(unit, p)) {
                    uint32 buffer[_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ];

                    sal_memset(buffer, 0, 
                           WORDS2BYTES(_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ));
                    BCM_IF_ERROR_RETURN(
                        _bcm_port_ehg_header_write(unit, p, buffer, buffer,
                                        _BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ));
                }
            }
        }
        if (!SOC_MEM_IS_VALID(unit, SYSTEM_CONFIG_TABLE_MODBASEm)) {
            BCM_IF_ERROR_RETURN(_bcm_tr2_system_tpid_init(unit));
        }
    }
#endif

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit) &&
        (soc_feature(unit,soc_feature_timesync_support))) {
        _bcm_esw_port_timesync_profile_init(unit);
    }
#endif /* defined(BCM_KATANA_SUPPORT) */


#ifdef BCM_SHADOW_SUPPORT
        /* Static port load balancing configuration for shadow */
    if (SOC_IS_SHADOW(unit))  {
        port_tab_entry_t ptab;
        /* For shadow-petrab mode, configure uflow_A(xe0) and uflow_b(xe1)*/
        if(soc_property_get(unit, spn_BCM88732_DEVICE_MODE, 0) &&
                ((soc_property_get(unit, spn_BCM88732_2X40_1X40, 0)) ||
                 (soc_property_get(unit, spn_BCM88732_2X40_2X40, 0)))) {
            sal_memset(&ptab, 0, sizeof(ptab));
            /* port 1 (xe0) configure uflow_a for static load balancing */
            SOC_IF_ERROR_RETURN(
                    READ_PORT_TABm(unit, MEM_BLOCK_ANY, 1, &ptab));
            soc_PORT_TABm_field32_set(unit, &ptab, AGGREGATION_GROUP_SELECTf, 0);
            SOC_IF_ERROR_RETURN(
                    WRITE_PORT_TABm(unit,MEM_BLOCK_ALL, 1, &ptab));

            /* port 5 (xe1) configure uflow_b for static load balancing */
            SOC_IF_ERROR_RETURN(
                    READ_PORT_TABm(unit, MEM_BLOCK_ANY, 5, &ptab));
            soc_PORT_TABm_field32_set(unit, &ptab, AGGREGATION_GROUP_SELECTf, 1);
            SOC_IF_ERROR_RETURN(
                    WRITE_PORT_TABm(unit,MEM_BLOCK_ALL, 5, &ptab));
        }

        /* Enable SAND interop header for Interlaken ports */
        for (p = 9; p <= 16; p++) {
            sal_memset(&ptab, 0, sizeof(ptab));
            SOC_IF_ERROR_RETURN(
                    soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, p, &ptab));
            soc_PORT_TABm_field32_set(unit, &ptab, SAND_COS_ENABLEf, 1);
            SOC_IF_ERROR_RETURN(
                    soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, p, &ptab));
        }

        /* By default Disable VLAN checks and enable TPID in port table */
        for (p = 0; p <= 16; p++) {
            sal_memset(&ptab, 0, sizeof(ptab));
            SOC_IF_ERROR_RETURN(
                    soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, p, &ptab));
            soc_PORT_TABm_field32_set(unit, &ptab, DISABLE_VLAN_CHECKSf, 1);
            soc_PORT_TABm_field32_set(unit, &ptab, OUTER_TPID_ENABLEf, 1);
            SOC_IF_ERROR_RETURN(
                    soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, p, &ptab));
        }
    }
#endif /* BCM_SHADOW_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        port_tab_entry_t ptab;
        uint32 regval;
        int blk;
        int blk_num = -1;
        int bindex = 0;
        int tx_pbm = 0;

        PBMP_ALL_ITER(unit, p) {
            if (!IS_LB_PORT(unit, p) && !IS_TDM_PORT(unit, p)) {
                BCM_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm,
                            MEM_BLOCK_ANY, p, &ptab));
                if (soc_mem_field_valid(unit, PORT_TABm,
                            FP_PORT_FIELD_SEL_INDEXf)) {
                    soc_PORT_TABm_field32_set(unit, &ptab, 
                            FP_PORT_FIELD_SEL_INDEXf, p);
                }
                BCM_IF_ERROR_RETURN(soc_mem_write(unit, PORT_TABm,
                            MEM_BLOCK_ALL, p, &ptab));
            }
            /* In Trident all ports are enabled to send E2ECC messages by
             * default. Disable E2ECC message TX here until congestion config
             * set enables the port.
             */
            blk = SOC_PORT_BLOCK(unit, p);
            blk_num = SOC_BLOCK_INFO(unit, blk).number;
            bindex = SOC_PORT_BINDEX(unit, p);
            SOC_IF_ERROR_RETURN(READ_E2ECC_TX_ENABLE_BMPr(unit, blk_num, 
                                                          &regval));
            tx_pbm = regval & ~(1 << bindex);
            soc_reg_field_set(unit, E2ECC_TX_ENABLE_BMPr, &regval, 
                                      TX_ENABLE_BMPf, tx_pbm);
            SOC_IF_ERROR_RETURN(WRITE_E2ECC_TX_ENABLE_BMPr(unit,
                                                           blk_num, regval));
        }
        /* Set the chip-wide E2ECC port mapping enable bit */ 
        if (soc_feature(unit, soc_feature_sysport_remap)) {
            SOC_IF_ERROR_RETURN(READ_E2ECC_PORT_MAPPING_CONFIGr(unit, &regval));
            soc_reg_field_set(unit, E2ECC_PORT_MAPPING_CONFIGr, &regval, PORT_MAPPING_ENf, 1);
            SOC_IF_ERROR_RETURN(WRITE_E2ECC_PORT_MAPPING_CONFIGr(unit, regval));
        }
    }
#endif

#if defined (BCM_HURRICANE_SUPPORT) || defined(BCM_TRIDENT_SUPPORT) \
    || defined (BCM_HAWKEYE_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
    if (SOC_IS_HURRICANE (unit) || SOC_IS_TD_TT (unit) || SOC_IS_HAWKEYE (unit)
        || SOC_IS_SHADOW(unit) || SOC_IS_KATANA(unit)) {
        if (soc_feature (unit, soc_feature_eee)) {
            int mac_val = 0;            
            _bcm_port_info_t *port_info;
            PBMP_ALL_ITER(unit, p) {
                if (!IS_LB_PORT(unit, p) && !IS_CPU_PORT (unit, p) && !IS_TDM_PORT(unit, p)) {
                    /* EEE standard compliance Work Around:
                     * Initialize the software state of native eee in MAC
                     */ 
                    eee_cfg[unit][p] = 0;
                    /* Check if MAC supports Native EEE and set the value of
                     * eee_cfg by reading the EEE status from MAC*/
                    _bcm_port_info_access(unit, p, &port_info);
                    if ((MAC_CONTROL_GET(port_info->p_mac, unit, p,
                        SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL)) {
                        eee_cfg[unit][p] = mac_val;
                    }
                }
            }
        }            
    }
#endif /* (BCM_HURRICANE_SUPPORT) || (BCM_TRIDENT_SUPPORT) || (BCM_HAWKEYE_SUPPORT)*/

    /*
     * CES TDM Port initialization
     */
#ifdef INCLUDE_CES
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
	 if (soc_feature(unit, soc_feature_ces)) {
	     extern int bcm_esw_port_tdm_init(int unit);
	     BCM_IF_ERROR_RETURN(bcm_esw_port_tdm_init(unit));
	 }
    }
#endif
#endif

    BCM_IF_ERROR_RETURN(_bcm_esw_port_mon_start(unit));

    if (SOC_WARM_BOOT(unit)) {
        return (_bcm_esw_port_wb_recover(unit));
    } else {
        return (_bcm_esw_port_wb_alloc(unit));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_port_clear
 * Purpose:
 *      Initialize the PORT interface layer for the specified SOC device
 *      without resetting stacking ports.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_NONE - success (or already initialized)
 *      BCM_E_INTERNAL- failed to write PTABLE entries
 *      BCM_E_MEMORY - failed to allocate required memory.
 * Notes:
 *      By default ports come up enabled. They can be made to come up disabled
 *      at startup by a compile-time application policy flag in your Make.local
 *      A call to bcm_port_clear should exhibit similar behavior for 
 *      non-stacking ethernet ports
 *      PTABLE initialized.
 */

int
bcm_esw_port_clear(int unit)
{
    bcm_port_config_t port_config;
    bcm_pbmp_t reset_ports;
    bcm_port_t port;
    int rv, port_enable;

    PORT_INIT(unit);

    BCM_IF_ERROR_RETURN(bcm_esw_port_config_get(unit, &port_config));

    /* Clear all non-stacking ethernet ports */
    BCM_PBMP_ASSIGN(reset_ports, port_config.e);
    BCM_PBMP_REMOVE(reset_ports, SOC_PBMP_STACK_CURRENT(unit));

    PBMP_ITER(reset_ports, port) {
        soc_cm_debug(DK_PORT | DK_VERBOSE,
                     "bcm_port_clear: unit %d port %s\n",
                     unit, SOC_PORT_NAME(unit, port));

        PORT_LOCK(unit);
        if ((rv = _bcm_port_mode_setup(unit, port, TRUE)) < 0) {
            soc_cm_debug(DK_WARN, "Warning: Unit %d Port %s: "
                         "Failed to set initial mode: %s\n",
                         unit, SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
        }
        PORT_UNLOCK(unit);



        /*
         * A compile-time application policy may prefer to disable 
         * ports at startup. The same behavior should be observed 
         * when bcm_port_clear gets called.
         */

#ifdef BCM_PORT_DEFAULT_DISABLE
            port_enable = FALSE;
#else
            port_enable = TRUE;
#endif  /* BCM_PORT_DEFAULT_DISABLE */


        if ((rv = bcm_esw_port_enable_set(unit, port, port_enable)) < 0) {
            soc_cm_debug(DK_WARN, "Warning: Unit %d Port %s: "
                         "Failed to %s port: %s\n",
                         unit, SOC_PORT_NAME(unit, port),(port_enable) ? "enable" : "disable" ,bcm_errmsg(rv));
        }

    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_phy_probe
 * Purpose:
 *      Probe the phy and set up the phy of the indicated port
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port to probe
 *      okay - Output parameter indicates port can be enabled.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      If error is returned, the port should not be enabled.
 */
int
_bcm_port_phy_probe(int unit, bcm_port_t port, int *okay)
{
    int                 rv;

    *okay = FALSE;

    soc_cm_debug(DK_PORT | DK_VERBOSE, "Init port %d PHY...\n", port);

    if ((rv = soc_phyctrl_probe(unit, port)) < 0) {
        soc_cm_debug(DK_WARN,
                     "Unit %d Port %s: Failed to probe PHY: %s\n",
                     unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv));
        return rv;
    }

    if ((rv = soc_phyctrl_init(unit, port)) < 0) {
        soc_cm_debug(DK_WARN,
                     "Unit %d Port %s: Failed to initialize PHY: %s\n",
                     unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv));
        return rv;
    }

    if (!SOC_WARM_BOOT(unit)) {
        /* Probe function should leave port disabled */
        if ((rv = soc_phyctrl_enable_set(unit, port, 0)) < 0) {
            return rv;
        }
    }

    *okay = TRUE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_mac_init
 * Purpose:
 *      Set up the mac of the indicated port
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port to setup
 *      okay - Output parameter indicates port can be enabled.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      If error is returned, the port should not be enabled.
 */
int
_bcm_port_mac_init(int unit, bcm_port_t port, int *okay)
{
    int                 rv;
    mac_driver_t        *macd;

    *okay = FALSE;

    soc_cm_debug(DK_PORT | DK_VERBOSE, "Init port %d MAC...\n", port);

    if ((rv = soc_mac_probe(unit, port, &macd)) < 0) {
        soc_cm_debug(DK_WARN,
                     "Unit %d Port %s: Failed to probe MAC: %s\n",
                     unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv));
        return rv;
    }

    PORT(unit, port).p_mac = macd;

    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RELOADING(unit) && !SOC_IS_RCPU_ONLY(unit)) {
        if ((rv = MAC_INIT(PORT(unit, port).p_mac, unit, port)) < 0) {
            soc_cm_debug(DK_WARN,
                         "Unit %d Port %s: Failed to initialize MAC: %s\n",
                         unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv));
            return rv;
        }

        /* Probe function should leave port disabled */
        if ((rv = MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, 0)) < 0) {
            return rv;
        }
    }

    *okay = TRUE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_probe
 * Purpose:
 *      Probe the phy and set up the phy and mac of the indicated port
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - Port to probe
 *      okay - Output parameter indicates port can be enabled.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      If error is returned, the port should not be enabled.
 */

int
_bcm_port_probe(int unit, bcm_port_t port, int *okay)
{
    BCM_IF_ERROR_RETURN(_bcm_port_phy_probe(unit, port, okay));

    /*
     * Currently initializing MAC after PHY is required because of
     * phy_5690_notify_init().
     */

    BCM_IF_ERROR_RETURN(_bcm_port_mac_init(unit, port, okay));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_probe
 * Purpose:
 *      Probe the PHY and set up the PHY and MAC for the specified ports.
 *      This is purely a discovery routine and does no configuration.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      pbmp - Bitmap of ports to probe.
 *      okay_pbmp (OUT) - Ports which were successfully probed.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      If error is returned, the port should not be enabled.
 *      Assumes port_init done.
 *      Note that if a PHY is not present, the port will still probe
 *      successfully.  The default driver will be installed.
 */

int
bcm_esw_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
    int rv = BCM_E_NONE;
    bcm_port_t port;
    int okay;

    SOC_PBMP_CLEAR(*okay_pbmp);

    PORT_INIT(unit);
    PORT_LOCK(unit);

    rv = soc_phyctrl_pbm_probe_init(unit,pbmp,okay_pbmp);

    if (!SOC_WARM_BOOT(unit)) {
        PBMP_ITER(*okay_pbmp, port) {
            /* Probe function should leave port disabled */
            if ((rv = soc_phyctrl_enable_set(unit, port, 0)) < 0) {
                break;
            }
        }
    }

    if (SOC_REG_IS_VALID(unit, GPORT_UMAC_CONTROLr)) {
        int blk;
        uint32 rval;
        SOC_BLOCK_ITER(unit, blk, SOC_BLK_GPORT) {
            port = SOC_BLOCK_PORT(unit, blk);
            /* just Write.. No need for RMW as no other field is there.. */
            /* (void)READ_GPORT_UMAC_CONTROLr(unit, port, &rval); */
            rval = 0xff; /* Set UMAC0_RESETf..UMAC7_RESETf */
            (void)WRITE_GPORT_UMAC_CONTROLr(unit, port, rval);
            sal_udelay(10);
            rval = 0; /* Reset UMAC0_RESETf..UMAC7_RESETf */
            (void)WRITE_GPORT_UMAC_CONTROLr(unit, port, rval);
        }
    }

    if (SOC_REG_IS_VALID(unit, XLPORT_XMAC_CONTROLr)) {
        int blk;
        uint32 rval;
        pbmp_t block_pbmp;

        SOC_BLOCK_ITER(unit, blk, SOC_BLK_XLPORT) {
            BCM_PBMP_ASSIGN(block_pbmp, SOC_BLOCK_BITMAP(unit, blk));
            BCM_PBMP_AND(block_pbmp, pbmp);
            if (BCM_PBMP_IS_NULL(block_pbmp)) {
                continue;
            }

            port = SOC_BLOCK_PORT(unit, blk);

            /* Reset the entire block only if it is called from port init */
            if (PORT(unit, port).p_mac != NULL) {
                continue;
            }

            (void)READ_XLPORT_XMAC_CONTROLr(unit, port, &rval);
            soc_reg_field_set(unit, XLPORT_XMAC_CONTROLr, &rval, XMAC_RESETf,
                              1);
            (void)WRITE_XLPORT_XMAC_CONTROLr(unit, port, rval);
            sal_udelay(10);

            soc_reg_field_set(unit, XLPORT_XMAC_CONTROLr, &rval, XMAC_RESETf,
                              0);
            (void)WRITE_XLPORT_XMAC_CONTROLr(unit, port, rval);
        }
    }
#ifdef BCM_SHADOW_SUPPORT
    /* Interlaken need to be taken out of reset after PHY is initialized */
    if (SOC_IS_SHADOW(unit)) {
        int active_il = 0;
        uint32 rval;

        if (soc_property_get(unit, spn_BCM88732_2X40_1X40, 0) ||
            soc_property_get(unit, spn_BCM88732_8X10_1X40, 0)) {
            active_il = 1;
        } else {
            if (soc_property_get(unit, spn_BCM88732_1X40_4X10, 0) ||
                soc_property_get(unit, spn_BCM88732_4X10_4X10, 0) ||
                soc_property_get(unit, spn_BCM88732_2X40_2X40, 0) ||
                soc_property_get(unit, spn_BCM88732_2X40_8X12, 0) ||
                soc_property_get(unit, spn_BCM88732_8X10_8X12, 0) ||
                soc_property_get(unit, spn_BCM88732_8X10_4X12, 0) ||
                soc_property_get(unit, spn_BCM88732_8X10_2X12, 0) ||
                soc_property_get(unit, spn_BCM88732_1X40_4X10_8X12, 0) ||
                soc_property_get(unit, spn_BCM88732_4X10_1X40_8X12, 0) ||
                soc_property_get(unit, spn_BCM88732_6X10_2X12, 0)) {
                active_il = 0;
            } else {
                active_il = 2;
            }
        }
        if (active_il) {
            soc_cm_print("Take interlaken out of reset\n");
            SOC_IF_ERROR_RETURN(READ_IL_GLOBAL_CONTROLr(unit, 9, &rval));
            soc_reg_field_set(unit, IL_GLOBAL_CONTROLr, &rval, SOFT_RESETf, 0);
            SOC_IF_ERROR_RETURN(WRITE_IL_GLOBAL_CONTROLr(unit, 9, rval));
        }
        if (active_il == 2) {
            SOC_IF_ERROR_RETURN(READ_IL_GLOBAL_CONTROLr(unit, 13, &rval));
            soc_reg_field_set(unit, IL_GLOBAL_CONTROLr, &rval, SOFT_RESETf, 0);
            SOC_IF_ERROR_RETURN(WRITE_IL_GLOBAL_CONTROLr(unit, 13, rval));
        }
    }
#endif /* BCM_SHADOW_SUPPORT */

    if (SOC_REG_IS_VALID(unit, XPORT_XMAC_CONTROLr)) {
        int blk;
        uint32 rval;

        SOC_BLOCK_ITER(unit, blk, SOC_BLK_MXQPORT) {
            port = SOC_BLOCK_PORT(unit, blk);

            (void)READ_XPORT_XMAC_CONTROLr(unit, port, &rval);
            soc_reg_field_set(unit, XPORT_XMAC_CONTROLr, &rval, XMAC_RESETf,
                              1);
            (void)WRITE_XPORT_XMAC_CONTROLr(unit, port, rval);
            sal_udelay(10);

            soc_reg_field_set(unit, XPORT_XMAC_CONTROLr, &rval, XMAC_RESETf,
                              0);
            (void)WRITE_XPORT_XMAC_CONTROLr(unit, port, rval);
        }
    }

    PBMP_ITER(pbmp, port) {
	if (!(IS_TDM_PORT(unit, port))) {
	    rv = _bcm_port_mac_init(unit, port, &okay);
	    if (!okay) {
		SOC_PBMP_PORT_REMOVE(*okay_pbmp, port);
	    }
	    if (rv < 0) {
		soc_cm_debug(DK_WARN,
			     "MAC init failed on port %s\n",
			     SOC_PORT_NAME(unit, port));
		break;
	    }
	}
    }

    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_port_detach
 * Purpose:
 *      Main part of bcm_port_detach
 */

int
_bcm_port_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
    bcm_port_t          port;
    int                 rv;

    SOC_PBMP_CLEAR(*detached);

    PBMP_ITER(pbmp, port) {
        SOC_IF_ERROR_RETURN(soc_phyctrl_detach(unit, port));
        BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, port, BCM_STG_STP_DISABLE));
        PORT_LOCK(unit);
        rv = _bcm_port_mode_setup(unit, port, FALSE);
        PORT_UNLOCK(unit);
        BCM_IF_ERROR_RETURN(rv);
        SOC_PBMP_PORT_ADD(*detached, port);
    }
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
        SOC_IS_TD_TT(unit) || SOC_IS_HURRICANE(unit) ||
        SOC_IS_KATANA(unit)) {
        _bcm_tr2_port_vpd_bitmap_free(unit);
    }
#endif
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_detach
 * Purpose:
 *      Detach a port.  Set phy driver to no connection.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      pbmp - Bitmap of ports to detach.
 *      detached (OUT) - Bitmap of ports successfully detached.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL - internal error.
 * Notes:
 *      If a port to be detached does not appear in detached, its
 *      state is not defined.
 */

int
bcm_esw_port_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
    int         rv;
#ifdef  BROADCOM_DEBUG
    char        pfmtp[SOC_PBMP_FMT_LEN],
                pfmtd[SOC_PBMP_FMT_LEN];
#endif  /* BROADCOM_DEBUG */

    PORT_INIT(unit);

    PORT_LOCK(unit);
    rv = _bcm_port_detach(unit, pbmp, detached);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT, "bcm_port_detach: u=%d pbmp=%s det=%s rv=%d\n",
                     unit,
                     SOC_PBMP_FMT(pbmp, pfmtp),
                     SOC_PBMP_FMT(*detached, pfmtd),
                     rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_config_get
 * Purpose:
 *      Get port configuration of a device
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      config - (OUT) Structure returning configuration
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_config_get(int unit, bcm_port_config_t *config)
{
    PORT_INIT(unit);

    config->fe          = PBMP_FE_ALL(unit);
    config->ge          = PBMP_GE_ALL(unit);
    config->xe          = PBMP_XE_ALL(unit);
    config->e           = PBMP_E_ALL(unit);
    config->hg          = PBMP_HG_ALL(unit);
    config->tdm         = PBMP_TDM_ALL(unit);
    config->port        = PBMP_PORT_ALL(unit);
    config->cpu         = PBMP_CMIC(unit);
    config->all         = PBMP_ALL(unit);
    /* Remove LB port from the PBMP_ALL bitmap for backward compatibility */
    SOC_PBMP_REMOVE(config->all, PBMP_LB(unit));
    config->stack_ext   = PBMP_ST_ALL(unit);

    BCM_PBMP_CLEAR(config->stack_int);
    BCM_PBMP_CLEAR(config->sci);
    BCM_PBMP_CLEAR(config->sfi);
    BCM_PBMP_CLEAR(config->spi);
    BCM_PBMP_CLEAR(config->spi_subport);

    return BCM_E_NONE;
}

STATIC int
_bcm_td_port_mode_update(int unit, bcm_port_t port, int speed)
{
    soc_info_t *si;
    int enable[2], duplex[2], loopback[2], pause_tx[2], pause_rx[2];
    int frame_max[2], encap[2];
    sal_mac_addr_t address[2];
    bcm_port_t port_list[2], phy_port, cur_port;
    uint32 rval;
    int core_mode, phy_mode, old_phy_mode;
    int i;

    /* Get the original phy_mode */
    SOC_IF_ERROR_RETURN(READ_XLPORT_MODE_REGr(unit, port, &rval));
    core_mode = soc_reg_field_get(unit, XLPORT_MODE_REGr, rval,
                                  CORE_PORT_MODEf);
    old_phy_mode = soc_reg_field_get(unit, XLPORT_MODE_REGr, rval,
                                     PHY_PORT_MODEf);

    /* speed is the only factor to determine phy_mode */
    if (speed > 20000) {
        phy_mode = 0;
    } else if (speed > 10000) {
        phy_mode = 1;
    } else {
        si = &SOC_INFO(unit);
        if ((si->port_num_lanes[port] == 2) && (speed == 10000) &&
            IS_HG_PORT(unit, port)) {
            phy_mode = 1; /* Warpcore uses DXGXS 10.5G in HG mode */
        } else {
            phy_mode = 2;
        }
    }

    if (phy_mode == old_phy_mode) {
        return BCM_E_NONE;
    }

    port_list[0] = port;
    port_list[1] = -1;
    if (core_mode == 1) { /* dual mode */
        si = &SOC_INFO(unit);
        phy_port = si->port_l2p_mapping[port];
        port_list[1] = si->port_p2l_mapping[((phy_port - 1) ^ 2) + 1];
    }
    soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval, PHY_PORT_MODEf, phy_mode);
    SOC_IF_ERROR_RETURN(WRITE_XLPORT_MODE_REGr(unit, port, rval));

    /* Retain MAC setting */
    for (i = 0; i < 2; i++) {
        if (port_list[i] == -1) {
            continue;
        }
        cur_port = port_list[i];

        /* no need to save speed since speed change trigger this, in addition,
         * mac may not be able to tell you exact speed in many cases */
        SOC_IF_ERROR_RETURN
            (MAC_DUPLEX_GET(PORT(unit, cur_port).p_mac, unit, cur_port,
                            &duplex[i]));
        SOC_IF_ERROR_RETURN
            (MAC_PAUSE_GET(PORT(unit, cur_port).p_mac, unit, cur_port,
                           &pause_tx[i], &pause_rx[i]));
        SOC_IF_ERROR_RETURN
            (MAC_PAUSE_ADDR_GET(PORT(unit, cur_port).p_mac, unit, cur_port,
                                address[i]));
        SOC_IF_ERROR_RETURN
            (MAC_FRAME_MAX_GET(PORT(unit, cur_port).p_mac, unit, cur_port,
                               &frame_max[i]));
        /* not saving ifg setting */
        SOC_IF_ERROR_RETURN
            (MAC_ENCAP_GET(PORT(unit, cur_port).p_mac, unit, cur_port,
                           &encap[i]));
        SOC_IF_ERROR_RETURN
            (MAC_LOOPBACK_GET(PORT(unit, cur_port).p_mac, unit, cur_port,
                              &loopback[i]));
        SOC_IF_ERROR_RETURN
            (MAC_ENABLE_GET(PORT(unit, cur_port).p_mac, unit, cur_port,
                            &enable[i]));
    }

    /* Do XMAC hard reset when phy_mode changed */
    (void)READ_XLPORT_XMAC_CONTROLr(unit, port, &rval);
    soc_reg_field_set(unit, XLPORT_XMAC_CONTROLr, &rval, XMAC_RESETf, 1);
    (void)WRITE_XLPORT_XMAC_CONTROLr(unit, port, rval);
    sal_udelay(10);
    soc_reg_field_set(unit, XLPORT_XMAC_CONTROLr, &rval, XMAC_RESETf, 0);
    (void)WRITE_XLPORT_XMAC_CONTROLr(unit, port, rval);

    for (i = 0; i < 2; i++) {
        if (port_list[i] == -1) {
            continue;
        }
        cur_port = port_list[i];

        /* mac init */
        SOC_IF_ERROR_RETURN
            (MAC_INIT(PORT(unit, cur_port).p_mac, unit, cur_port));

        /* Restore MAC setting */
        /* no need to save speed since speed change trigger this, in addition,
         * mac may not be able to tell you exact speed in many cases */
        /* coverity[uninit_use_in_call] : FALSE */
        SOC_IF_ERROR_RETURN
            (MAC_DUPLEX_SET(PORT(unit, cur_port).p_mac, unit, cur_port,
                            duplex[i]));
        /* coverity[uninit_use_in_call] : FALSE */
        SOC_IF_ERROR_RETURN
            (MAC_PAUSE_SET(PORT(unit, cur_port).p_mac, unit, cur_port,
                           pause_tx[i], pause_rx[i]));
        SOC_IF_ERROR_RETURN
            (MAC_PAUSE_ADDR_SET(PORT(unit, cur_port).p_mac, unit, cur_port,
                                address[i]));
        /* coverity[uninit_use_in_call] : FALSE */
        SOC_IF_ERROR_RETURN
            (MAC_FRAME_MAX_SET(PORT(unit, cur_port).p_mac, unit, cur_port,
                               frame_max[i]));
        /* not saving ifg setting */
        /* coverity[uninit_use_in_call] : FALSE */
        SOC_IF_ERROR_RETURN
            (MAC_ENCAP_SET(PORT(unit, cur_port).p_mac, unit, cur_port,
                           encap[i]));
        /* coverity[uninit_use_in_call] : FALSE */
        SOC_IF_ERROR_RETURN
            (MAC_LOOPBACK_SET(PORT(unit, cur_port).p_mac, unit, cur_port,
                              loopback[i]));
        /* coverity[uninit_use_in_call] : FALSE */
        SOC_IF_ERROR_RETURN
            (MAC_ENABLE_SET(PORT(unit, cur_port).p_mac, unit, cur_port,
                            enable[i]));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_update
 * Purpose:
 *      Get port characteristics from PHY and program MAC to match.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      link -  True if link is active, false if link is inactive.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

STATIC int
_bcm_port_update(int unit, bcm_port_t port, int link)
{
    int                 rv;
    int                 duplex, speed, an, an_done;
    soc_port_if_t       pif;

    if (!link) {
        /* PHY is down.  Disable the MAC. */

        rv = (MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, FALSE));
        if (BCM_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                    "u=%d p=%d MAC_ENABLE_SET FALSE rv=%d\n",
                    unit, port, rv));
            return rv;
        }

        /* PHY link down event */
        rv = (soc_phyctrl_linkdn_evt(unit, port));
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                    "u=%d p=%d soc_phyctrl_linkdn_evt rv=%d\n",unit, port, rv));
            return rv;
        }

        return BCM_E_NONE;
    }

    /* PHY link up event may not be support by all PHY driver. 
     * Just ignore it if not supported */
    rv = (soc_phyctrl_linkup_evt(unit, port));
    if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
        SOC_DEBUG_PRINT((DK_WARN,
                "u=%d p=%d soc_phyctrl_linkup_evt rv=%d\n",unit, port, rv));
        return rv;
    }

    /*
     * Set MAC speed first, since for GTH ports, this will switch
     * between the 1000Mb/s or 10/100Mb/s MACs.
     */

    if (!IS_HG_PORT(unit, port) || IS_GX_PORT(unit, port)) {
        rv = (soc_phyctrl_speed_get(unit, port, &speed));
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                "u=%d p=%d phyctrl_speed_get rv=%d\n",unit, port, rv));
            return rv;
        }
        if (IS_HG_PORT(unit, port) && speed < 10000) {
            speed = 0;
        }
        SOC_DEBUG_PRINT((DK_PORT,
            "u=%d p=%d phyctrl_speed_get speed=%d\n",unit, port, speed));

        if (BCM_E_UNAVAIL == rv ) {
            /* If PHY driver doesn't support speed_get, don't change 
             * MAC speed. E.g, Null PHY driver 
             */
            rv = BCM_E_NONE;
        } else {
            if (SOC_IS_TD_TT(unit)) {
                BCM_IF_ERROR_RETURN
                    (_bcm_td_port_mode_update(unit, port, speed));
            }
#ifdef INCLUDE_FCMAP
            /* 
             * In FCMAP mode, the port is operating in FC-Mode. The switch and the
             * system side of the PHY operates in 10G, FullDuplex and Pause enabled
             * mode
             */
            if (soc_property_port_get(unit, port, spn_FCMAP_ENABLE, 0)) {
                speed = 10000;
            }
#endif

            rv =  (MAC_SPEED_SET(PORT(unit, port).p_mac, unit, port, speed));
        }
        if (BCM_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN, 
                            "u=%d p=%d MAC_SPEED_SET speed=%d rv=%d\n",
                            unit, port, speed, rv));
            return rv;
        }

        rv =   (soc_phyctrl_duplex_get(unit, port, &duplex));
        if (BCM_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN, "u=%d p=%d phyctrl_duplex_get rv=%d\n",
                                unit, port, rv));
            return rv;
        }

#ifdef INCLUDE_MACSEC
        /* When macsec is enabled, the switch and system side of the PHY 
         * operate in full-duplex mode.
         * 
         */
        if(soc_property_port_get(unit, port, spn_MACSEC_ENABLE, 0)) {
            duplex = 1; 
        }
#endif

#ifdef INCLUDE_FCMAP
            /* 
             * In FCMAP mode, the port is operating in FC-Mode. The switch and the
             * system side of the PHY operates in 10G, FullDuplex and Pause enabled
             * mode
             */
            if (soc_property_port_get(unit, port, spn_FCMAP_ENABLE, 0)) {
                duplex = 1;
            }
#endif
        rv = (MAC_DUPLEX_SET(PORT(unit, port).p_mac, unit, port, duplex));
        if (BCM_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN, "u=%d p=%d MAC_DUPLEX_SET %s sp=%d rv=%d\n", 
                             unit, port, 
                             duplex ? "FULL" : "HALF", speed, rv));
            return rv;
        }
    } else {

        duplex = 1;
    }

    rv = (soc_phyctrl_interface_get(unit, port, &pif));
    if (BCM_FAILURE(rv)) {
        SOC_DEBUG_PRINT((DK_WARN,
                        "u=%d p=%d phyctrl_interface_get rv=%d\n",
                        unit, port, rv));
        return rv;
    }
    rv = (MAC_INTERFACE_SET(PORT(unit, port).p_mac, unit, port, pif));
    if (BCM_FAILURE(rv)) {
        SOC_DEBUG_PRINT((DK_WARN,
                        "u=%d p=%d MAC_INTERFACE_GET rv=%d\n",
                        unit, port,rv));
        return rv;
    }

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_auto_negotiate_get(unit, port, &an, &an_done));

    /*
     * If autonegotiating, check the negotiated PAUSE values, and program
     * MACs accordingly. Link can also be achieved thru parallel detect.
     * In this case, it should be treated as in the forced mode.
     */

    if (an && an_done) {
        bcm_port_ability_t      remote_advert, local_advert;
        int                     tx_pause, rx_pause;

        rv = soc_phyctrl_ability_advert_get(unit, port, &local_advert); 
        if (BCM_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                            "u=%d p=%d soc_phyctrl_adv_local_get rv=%d\n",
                             unit, port, rv));
            return rv;
        }
        rv = soc_phyctrl_ability_remote_get(unit, port, &remote_advert); 
        if (BCM_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                            "u=%d p=%d soc_phyctrl_adv_remote_get rv=%d\n",
                             unit, port, rv));
            return rv;
        }

        /*
         * IEEE 802.3 Flow Control Resolution.
         * Please see $SDK/doc/pause-resolution.txt for more information.
         */

        if (duplex) {
            tx_pause =
                     ((remote_advert.pause & SOC_PA_PAUSE_RX) &&     
                      (local_advert.pause & SOC_PA_PAUSE_RX)) ||     
                     ((remote_advert.pause & SOC_PA_PAUSE_RX) &&     
                      !(remote_advert.pause & SOC_PA_PAUSE_TX) &&    
                      (local_advert.pause & SOC_PA_PAUSE_TX));   
         
            rx_pause =
                     ((remote_advert.pause & SOC_PA_PAUSE_RX) &&     
                      (local_advert.pause & SOC_PA_PAUSE_RX)) ||     
                     ((local_advert.pause & SOC_PA_PAUSE_RX) &&      
                      (remote_advert.pause & SOC_PA_PAUSE_TX) &&     
                      !(local_advert.pause & SOC_PA_PAUSE_TX));
        } else {
            rx_pause = tx_pause = 0;
        }

        rv = (MAC_PAUSE_SET(PORT(unit, port).p_mac,
                           unit, port, tx_pause, rx_pause));
        if (BCM_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                            "u=%d p=%d MAC_PAUSE_SET rv=%d\n",
                            unit, port, rv));
            return rv;
        }
    }

#ifdef INCLUDE_MACSEC
    {
    /* 
     * In MACSEC based PHYs the switch side could be operating with 
     * different speed,duplex and pause compared to line speed, duplex and
     * pause settings. The above setting will be overridden if the switch side
     * of the PHY is set to operate in fixed mode
     */
    uint32 value;
    rv = (soc_phyctrl_control_get(unit, port,
                                  BCM_PORT_PHY_CONTROL_MACSEC_SWITCH_FIXED, 
                                  &value));

    if (BCM_E_UNAVAIL != rv) { /* Switch fixed speed is supported */

        if (value == 1) {
            /* Get and Set Speed */
            rv = (soc_phyctrl_control_get(unit, port,
                              BCM_PORT_PHY_CONTROL_MACSEC_SWITCH_FIXED_SPEED, 
                              &value));
            if (BCM_FAILURE(rv)) {
                SOC_DEBUG_PRINT((DK_WARN,
                    "u=%d p=%d phyctrl_control_get(speed) rv=%d\n",
                    unit, port, rv));
                return rv;
            }

            rv =  (MAC_SPEED_SET(PORT(unit, port).p_mac, unit, port, value));
            if (BCM_FAILURE(rv)) {
                SOC_DEBUG_PRINT((DK_WARN, 
                            "u=%d p=%d MAC_SPEED_SET speed=%d rv=%d\n",
                            unit, port, value, rv));
                return rv;
            }

            /* Get and Set Duplex */
            rv = (soc_phyctrl_control_get(unit, port,
                              BCM_PORT_PHY_CONTROL_MACSEC_SWITCH_FIXED_DUPLEX, 
                              &value));
            if (BCM_FAILURE(rv)) {
                SOC_DEBUG_PRINT((DK_WARN,
                    "u=%d p=%d phyctrl_control_get(duplex) rv=%d\n",
                    unit, port, rv));
                return rv;
            }
            rv = (MAC_DUPLEX_SET(PORT(unit, port).p_mac, unit, port, value));
            if (BCM_FAILURE(rv)) {
                SOC_DEBUG_PRINT((DK_WARN, "u=%d p=%d MAC_DUPLEX_SET %s sp=%d rv=%d\n", 
                                 unit, port, 
                                 value ? "FULL" : "HALF", value, rv));
                return rv;
            }

            /* Get and Set PAUSE */
            rv = (soc_phyctrl_control_get(unit, port,
                              BCM_PORT_PHY_CONTROL_MACSEC_SWITCH_FIXED_PAUSE, 
                              &value));
            if (BCM_FAILURE(rv)) {
                SOC_DEBUG_PRINT((DK_WARN,
                    "u=%d p=%d phyctrl_control_get(Pause) rv=%d\n",
                    unit, port, rv));
                return rv;
            }
            if (value == 1) { 
                /* 
                 * Flow control domain is between switch MAC and PHY MAC.
                 * If the flow control domain is not between switch and PHY's
                 * switch MAC, then the flow control domain is extended to that 
                 * of GPHY.
                 */
                rv = (MAC_PAUSE_SET(PORT(unit, port).p_mac,
                               unit, port, value, value));
                if (BCM_FAILURE(rv)) {
                    SOC_DEBUG_PRINT((DK_WARN,
                                "u=%d p=%d MAC_PAUSE_SET rv=%d\n",
                                unit, port, rv));
                    return rv;
                }
            }

        }
    }
    }
#endif

#ifdef BCM_GXPORT_SUPPORT
    if (link && soc_feature(unit, soc_feature_port_lag_failover) && 
        IS_GX_PORT(unit, port)) {
        soc_reg_t reg;
        uint32 addr, rval;

        if (SOC_REG_IS_VALID(unit, LAG_FAILOVER_CONFIGr)) {
            reg = LAG_FAILOVER_CONFIGr;
        } else {
            reg = GXPORT_LAG_FAILOVER_CONFIGr;
        }
        addr = soc_reg_addr(unit, reg, port, 0);

        /* Toggle link bit to notify IPIPE on link up */
        BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg, port, 0, &rval));
        soc_reg_field_set(unit, reg, &rval, LINK_STATUS_UPf, 1);
        BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, port, 0, rval));
        soc_reg_field_set(unit, reg, &rval, LINK_STATUS_UPf, 0);
        BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, port, 0, rval));
    }
#endif /* BCM_GXPORT_SUPPORT */

    /* Enable the MAC. */
    rv =  (MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, TRUE));
    if (BCM_FAILURE(rv)) {
        SOC_DEBUG_PRINT((DK_WARN,
                        "u=%d p=%d MAC_ENABLE_SET TRUE rv=%d\n",
                        unit, port, rv));
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_update
 * Purpose:
 *      Get port characteristics from PHY and program MAC to match.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      link - TRUE - process as link up.
 *             FALSE - process as link down.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_update(int unit, bcm_port_t port, int link)
{
    int         rv;
    
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (SOC_IS_RCPU_ONLY(unit)) {
        return BCM_E_NONE;
    }

    PORT_LOCK(unit);
    rv = _bcm_port_update(unit, port, link);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return (rv);
    }

#if defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_HBX(unit) && !SOC_IS_SHADOW(unit)) {
        rv = _bcm_port_mmu_update(unit, port, link);
    }
#endif /* BCM_BRADLEY_SUPPORT */

    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_update: u=%d p=%d link=%d rv=%d\n",
                     unit, port, link, rv));

    return(rv);
}

/*
 * Function:
 *      bcm_port_stp_set
 * Purpose:
 *      Set the spanning tree state for a port.
 *      All STGs containing all VLANs containing the port are updated.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - StrataSwitch port number.
 *      stp_state - State to place port in, one of BCM_PORT_STP_xxx.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 * Notes:
 *      BCM_LOCK is taken so that the current list of VLANs
 *      can't change during the operation.
 */

int
bcm_esw_port_stp_set(int unit, bcm_port_t port, int stp_state)
{
    bcm_stg_t           *list = NULL;
    int                 count = 0, i;
    int                 rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    BCM_LOCK(unit);

    rv = bcm_esw_stg_list(unit, &list, &count);

    if (rv == BCM_E_UNAVAIL) {
        if (stp_state == BCM_STG_STP_FORWARD) {
            rv = BCM_E_NONE;
        } else {
            rv = BCM_E_PARAM;
        }
    } else if (BCM_SUCCESS(rv)) {
        for (i = 0; i < count; i++) {
            if ((rv = bcm_esw_stg_stp_set(unit, list[i], 
                                          port, stp_state)) < 0) {
                break;
            }
        }
        
        bcm_esw_stg_list_destroy(unit, list, count);
    }

    BCM_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_stp_set: u=%d p=%d state=%d rv=%d\n",
                     unit, port, stp_state, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_stp_get
 * Purpose:
 *      Get the spanning tree state for a port in the default STG.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - StrataSwitch port number.
 *      stp_state - Pointer where state stored.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
bcm_esw_port_stp_get(int unit, bcm_port_t port, int *stp_state)
{
    int                 stg_defl, rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = bcm_esw_stg_default_get(unit, &stg_defl);
    if (rv >= 0) {
        rv = bcm_esw_stg_stp_get(unit, stg_defl, port, stp_state);
    } else if (rv == BCM_E_UNAVAIL) {   /* FABRIC switches, etc */
        *stp_state = BCM_STG_STP_FORWARD;
        rv = BCM_E_NONE;
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_stp_get: u=%d p=%d state=%d rv=%d\n",
                     unit, port, *stp_state, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_bpdu_enable_set
 * Purpose:
 *      Enable/Disable BPDU reception on the specified port.
 * Parameters:
 *      unit - SOC unit #
 *      port - Port number (0 based)
 *      enable - TRUE to enable, FALSE to disable (reject bpdu).
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
bcm_esw_port_bpdu_enable_set(int unit, bcm_port_t port, int enable)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        pcfg.pc_bpdu_disable = !enable;
        rv = mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg);
    }

    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_bpdu_enable_get
 * Purpose:
 *      Return whether BPDU reception is enabled on the specified port.
 * Parameters:
 *      unit - SOC unit #
 *      port - Port number (0 based)
 *      enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
bcm_esw_port_bpdu_enable_get(int unit, bcm_port_t port, int *enable)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        *enable = !pcfg.pc_bpdu_disable;
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_l3_enable_set
 * Purpose:
 *      Enable/Disable L3 switching on the specified port.
 * Parameters:
 *      unit -          device number
 *      port -          port number
 *      enable -        TRUE to enable, FALSE to disable.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_l3_enable_set(int unit, bcm_port_t port, int enable)
{
    int  rv = BCM_E_UNAVAIL;
#if defined(INCLUDE_L3)
    bcm_port_cfg_t      pcfg;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        if (enable) {
            pcfg.pc_l3_flags |= (BCM_PORT_L3_V4_ENABLE | BCM_PORT_L3_V6_ENABLE);
        } else {
            pcfg.pc_l3_flags &= 
                ~(BCM_PORT_L3_V4_ENABLE | BCM_PORT_L3_V6_ENABLE);
        }
        rv = mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg);
    }

    PORT_UNLOCK(unit);
#endif /* INCLUDE_L3 */
    return rv;
}

/*
 * Function:
 *      bcm_port_l3_enable_get
 * Purpose:
 *      Return whether L3 switching is enabled on the specified port.
 * Parameters:
 *      unit -          device number
 *      port -          port number
 *      enable -        (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_l3_enable_get(int unit, bcm_port_t port, int *enable)
{
    int  rv = BCM_E_UNAVAIL;
#if defined(INCLUDE_L3)
    bcm_port_cfg_t      pcfg;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        *enable = ((pcfg.pc_l3_flags & BCM_PORT_L3_V4_ENABLE) || 
                   (pcfg.pc_l3_flags & BCM_PORT_L3_V6_ENABLE));
    }
#endif /* INCLUDE_L3 */

    return rv;
}

/*
 * Function:
 *      bcm_port_tgid_get
 * Purpose:
 *      Get the trunk group for a given port.
 * Parameters:
 *      unit - SOC unit #
 *      port - Port number (0 based)
 *      tid - trunk ID
 *      psc - trunk selection criterion
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
bcm_esw_port_tgid_get(int unit, bcm_port_t port, int *tid, int *psc)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#if defined(BCM_FIREBOLT_SUPPORT) || defined (BCM_EASYRIDER_SUPPORT)
    if (soc_mem_field_valid(unit, SOURCE_TRUNK_MAP_TABLEm, TGIDf)) {
        rv = _bcm_xgs3_trunk_table_read(unit, port, tid);
        if (BCM_SUCCESS(rv)) {
            *psc = 0;
        }
        return rv;
    }
#endif /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT */

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        *tid = pcfg.pc_tgid;
        *psc = 0;
    }

    return rv;

}

/*
 * Function:
 *      bcm_port_tgid_set
 * Purpose:
 *      Set the trunk group for a given port.
 * Parameters:
 *      unit - SOC unit #
 *      port - Port number (0 based)
 *      tid - trunk ID
 *      psc - trunk selection criterion
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
bcm_esw_port_tgid_set(int unit, bcm_port_t port, int tid, int psc)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#if defined(BCM_FIREBOLT_SUPPORT) || defined (BCM_EASYRIDER_SUPPORT)
    if (soc_mem_field_valid(unit, SOURCE_TRUNK_MAP_TABLEm, TGIDf)) {
        return _bcm_xgs3_trunk_table_write(unit, port, tid);
    }
#endif /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT */

    PORT_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        if (tid != BCM_TRUNK_INVALID) {
            pcfg.pc_tgid = tid;
            pcfg.pc_trunk = 1;
        } else {
            pcfg.pc_tgid = 0;
            pcfg.pc_trunk = 0;
        }
        rv = mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg);
    }

    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_port_mirror_enable_get (internal)
 * Purpose:
 *       Return whether mirroring is enabled on the specified port.
 * Parameters:
 *      unit   - (IN) BCM device number. 
 *      port   - (IN) Port number (0 based).
 *      enable - (OUT) Bitmap of ingress mirror enabled. 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
_bcm_port_mirror_enable_get(int unit, bcm_port_t port, int *enable)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    /* Input parameters check. */
    if (NULL == enable) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        *enable = pcfg.pc_mirror_ing;
    }

    return rv;
}

/*
 * Function:
 *      _bcm_port_mirror_enable_set (internal)
 * Purpose:
 *       Enable/Disable mirroring for a given port.
 * Parameters:
 *      unit -   (IN) BCM unit #
 *      port -   (IN) Port number (0 based)
 *      enable - (IN) Bitmap of ingress mirror enabled. 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
_bcm_port_mirror_enable_set(int unit, bcm_port_t port, int enable)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        pcfg.pc_mirror_ing = enable;
        rv = mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg);
    }

    PORT_UNLOCK(unit);

    return rv;
}

#ifdef BCM_TRIUMPH2_SUPPORT
/*
 * Function:
 *      _bcm_port_mirror_egress_true_enable_get (internal)
 * Purpose:
 *       Return whether true egress mirroring is enabled on the
 *       specified port.
 * Parameters:
 *      unit   - (IN) BCM device number. 
 *      port   - (IN) Port number (0 based).
 *      enable - (OUT) Bitmap of true egress mirrors enabled. 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
_bcm_port_mirror_egress_true_enable_get(int unit, bcm_port_t port,
                                        int *enable)
{
    uint64 egr_val64;
    int                 rv;

    /* Input parameters check. */
    if (NULL == enable) {
        return (BCM_E_PARAM);
    }

    rv = READ_EGR_PORT_64r(unit, port, &egr_val64);
    if (BCM_SUCCESS(rv)) {
        *enable = soc_reg64_field32_get(unit, EGR_PORT_64r, egr_val64, MIRRORf);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_port_mirror_egress_true_enable_set (internal)
 * Purpose:
 *       Enable/Disable true egress mirroring for a given port.
 * Parameters:
 *      unit -   (IN) BCM unit #
 *      port -   (IN) Port number (0 based)
 *      enable - (IN) Bitmap of true egress mirrors enabled. 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */

int
_bcm_port_mirror_egress_true_enable_set(int unit, bcm_port_t port,
                                        int enable)
{
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    rv = soc_reg_field32_modify(unit, EGR_PORT_64r, port, MIRRORf, enable);
    if (BCM_SUCCESS(rv) && IS_HG_PORT(unit, port)) {
        rv = soc_reg_field32_modify(unit, IEGR_PORT_64r, port,
                                    MIRRORf, enable);
    }

    PORT_UNLOCK(unit);
    return rv;
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      bcm_port_linkscan_get
 * Purpose:
 *      Get the link scan state of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      linkscan - (OUT) Linkscan value (None, S/W, H/W)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_linkscan_get(int unit, bcm_port_t port, int *linkscan)
{
    return bcm_esw_linkscan_mode_get(unit, port, linkscan);
}

/*
 * Function:
 *      bcm_port_linkscan_set
 * Purpose:
 *      Set the linkscan state for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      linkscan - Linkscan value (None, S/W, H/W)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_linkscan_set(int unit, bcm_port_t port, int linkscan)
{
    return bcm_esw_linkscan_mode_set(unit, port, linkscan);
}

/*
 * Function:
 *      bcm_port_autoneg_get
 * Purpose:
 *      Get the autonegotiation state of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      autoneg - (OUT) Boolean value
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_autoneg_get(int unit, bcm_port_t port, int *autoneg)
{
    int done, rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_auto_negotiate_get(unit, port, autoneg, &done);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_autoneg_get: u=%d p=%d an=%d done=%d rv=%d\n",
                     unit, port, *autoneg, done, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_autoneg_set
 * Purpose:
 *      Set the autonegotiation state for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      autoneg - Boolean value
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_autoneg_set(int unit, bcm_port_t port, int autoneg)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_auto_negotiate_set(unit, port, autoneg);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_autoneg_set: u=%d p=%d an=%d rv=%d\n",
                     unit, port, autoneg, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_speed_get
 * Purpose:
 *      Getting the speed of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      If port is in MAC loopback, the speed of the loopback is returned.
 */

int
bcm_esw_port_speed_get(int unit, bcm_port_t port, int *speed)
{
    int         rv = SOC_E_NONE;
    int         mac_lb;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    rv = MAC_LOOPBACK_GET(PORT(unit, port).p_mac, unit, port, &mac_lb);

    if (BCM_SUCCESS(rv)) {
        if (mac_lb || (IS_HG_PORT(unit, port) && !IS_GX_PORT(unit, port))) {
            rv = MAC_SPEED_GET(PORT(unit, port).p_mac, unit, port, speed);
        } else {
            rv = soc_phyctrl_speed_get(unit, port, speed);
            if (BCM_E_UNAVAIL == rv) {
                /* PHY driver doesn't support speed_get. Get the speed from
                 * MAC.
                 */
                rv = MAC_SPEED_GET(PORT(unit, port).p_mac, unit, port, speed);
            }
            if (IS_HG_PORT(unit, port) && *speed < 10000) {
                *speed = 0;
            }
        }
    }

    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_speed_get: u=%d p=%d speed=%d rv=%d\n",
                     unit, port, BCM_SUCCESS(rv) ? *speed : 0, rv));

    return rv;
}
/*
 * Function:
 *      bcm_port_speed_max
 * Purpose:
 *      Getting the maximum speed of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_speed_max(int unit, bcm_port_t port, int *speed)
{
    bcm_port_ability_t  ability;
    int                 rv;

    if (NULL == speed) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_port_gport_validate(unit, port, &port));
    rv = bcm_esw_port_ability_local_get(unit, port, &ability);

    if (BCM_SUCCESS(rv)) {
        
        *speed = BCM_PORT_ABILITY_SPEED_MAX(ability.speed_full_duplex | ability.speed_half_duplex);
        if (10000 == *speed) {
            if (IS_HG_PORT(unit, port) && SOC_INFO(unit).port_speed_max[port]) {
                *speed = SOC_INFO(unit).port_speed_max[port];
            }
        }
    } else {
        *speed = 0;
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_speed_max: u=%d p=%d speed=%d rv=%d\n",
                     unit, port, *speed, rv));

    return rv;
}

/*
 * Function:
 *      _bcm_port_speed_set
 * Purpose:
 *      Main part of bcm_port_speed_set.
 */

STATIC int
_bcm_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int             phy_lb, rv, enable;
    int             mac_lb;
    bcm_port_ability_t mac_ability, phy_ability, requested_ability;

    /*
     * If port is in MAC loopback mode, do not try setting the PHY
     * speed.  This allows MAC loopback at 10/100 even if the PHY is
     * 1000 only.  Loopback diagnostic tests should enable loopback
     * before setting the speed, and vice versa when cleaning up.
     */

    SOC_IF_ERROR_RETURN
        (MAC_LOOPBACK_GET(PORT(unit, port).p_mac, unit, port, &mac_lb));

    if (speed == 0) {
        /* if speed is 0, set the port speed to max */
        SOC_IF_ERROR_RETURN
            (bcm_esw_port_speed_max(unit, port, &speed));    
    }

    /* Make sure MAC can handle the requested speed. */
    SOC_IF_ERROR_RETURN
        (MAC_ABILITY_LOCAL_GET(PORT(unit, port).p_mac, unit, port,
                               &mac_ability));
    requested_ability.speed_full_duplex = SOC_PA_SPEED(speed);
    requested_ability.speed_half_duplex = SOC_PA_SPEED(speed);
    SOC_DEBUG_PRINT((DK_PHY,
                     "_bcm_port_speed_set: u=%u p=%d "
                     "MAC FD speed %08X MAC HD speed %08X "
                     "Requested FD Speed %08X Requested HD Speed %08X\n",
                     unit,
                     port,
                     mac_ability.speed_full_duplex,
                     mac_ability.speed_half_duplex,       
                     requested_ability.speed_full_duplex,
                     requested_ability.speed_half_duplex));

    if (((mac_ability.speed_full_duplex &
          requested_ability.speed_full_duplex) == 0) &&
        ((mac_ability.speed_half_duplex &
          requested_ability.speed_half_duplex) == 0)) {
        SOC_DEBUG_PRINT((DK_VERBOSE,
                         "u=%d p=%d MAC doesn't support %d Mbps speed.\n",
                         unit, port, speed));
        return SOC_E_CONFIG;
    }

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_ability_local_get(unit, port, &phy_ability));

    if (((phy_ability.speed_full_duplex &
          requested_ability.speed_full_duplex) == 0) &&
        ((phy_ability.speed_half_duplex &
          requested_ability.speed_half_duplex) == 0)) {
        if (!mac_lb) {
            SOC_DEBUG_PRINT((DK_VERBOSE,
                             "u=%d p=%d PHY doesn't support %d Mbps speed.\n",
                             unit, port, speed));
            return SOC_E_CONFIG;
        }
    } else {
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_auto_negotiate_set(unit, port, FALSE));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_speed_set(unit, port, speed));
    }
    
    /* Prevent PHY register access while resetting BigMAC and Fusion core */
    if (IS_HG_PORT(unit, port)) {
        soc_phyctrl_enable_get(unit, port, &enable);
        soc_phyctrl_enable_set(unit, port, 0);
        soc_phyctrl_loopback_get(unit, port, &phy_lb);
    }

    if (SOC_IS_TD_TT(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_td_port_mode_update(unit, port, speed));
    }

    rv = MAC_SPEED_SET(PORT(unit, port).p_mac, unit, port, speed);
    SOC_IF_ERROR_DEBUG_PRINT
        (rv, (DK_VERBOSE, "MAC_SPEED_SET failed: %s\n", bcm_errmsg(rv)));

    /* Restore PHY register access */
    if (IS_HG_PORT(unit, port)) {
        soc_phyctrl_enable_set(unit, port, enable);
        soc_phyctrl_loopback_set(unit, port, phy_lb);
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_speed_set
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      speed - Value in megabits/sec (10, 100, etc)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Turns off autonegotiation.  Caller must make sure other forced
 *      parameters (such as duplex) are set.
 */

int
bcm_esw_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int         rv, max_speed;
    pbmp_t      pbm;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    /*    coverity[uninit_use_in_call : FALSE]    */
    BCM_IF_ERROR_RETURN(
        bcm_esw_port_speed_max(unit,port, &max_speed));

    if (speed < 0 || speed > max_speed) {
        return BCM_E_CONFIG;
    }

    PORT_LOCK(unit);                    /* multiple operations operation */
    rv = _bcm_port_speed_set(unit, port, speed);
    PORT_UNLOCK(unit);                  /* Unlock before link call */

    if (BCM_SUCCESS(rv) && !SAL_BOOT_SIMULATION) {
        SOC_PBMP_CLEAR(pbm);
        SOC_PBMP_PORT_ADD(pbm, port);
        (void)bcm_esw_link_change(unit, pbm);
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_speed_set: u=%d p=%d speed=%d rv=%d\n",
                     unit, port, speed, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_master_get
 * Purpose:
 *      Getting the master status of the port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ms - (OUT) BCM_PORT_MS_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */

int
bcm_esw_port_master_get(int unit, bcm_port_t port, int *ms)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_master_get(unit, port, ms);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_master_set
 * Purpose:
 *      Setting the master status for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ms - BCM_PORT_MS_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Ignored if not supported on port.
 *      WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */

int
bcm_esw_port_master_set(int unit, bcm_port_t port, int ms)
{
    int         rv;
    pbmp_t      pbm;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);                    /* multiple operations operation */

    rv = soc_phyctrl_master_set(unit, port, ms);
    SOC_IF_ERROR_DEBUG_PRINT
        (rv, (DK_VERBOSE, "PHY_MASTER_SET failed: %s\n", bcm_errmsg(rv)));

    PORT_UNLOCK(unit);                  /* Unlock before link call */

    if (BCM_SUCCESS(rv)) {
        SOC_PBMP_CLEAR(pbm);
        SOC_PBMP_PORT_ADD(pbm, port);
        (void)bcm_esw_link_change(unit, pbm);
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_interface_get
 * Purpose:
 *      Getting the interface type of a port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      intf - (OUT) BCM_PORT_IF_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */

int
bcm_esw_port_interface_get(int unit, bcm_port_t port, bcm_port_if_t *intf)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_interface_get(unit, port, intf);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_interface_set
 * Purpose:
 *      Setting the interface type for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      if - BCM_PORT_IF_*
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */

int
bcm_esw_port_interface_set(int unit, bcm_port_t port, bcm_port_if_t intf)
{
    int         rv;
    pbmp_t      pbm;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);                    /* multiple operations operation */

    rv = soc_phyctrl_interface_set(unit, port, intf);
    SOC_IF_ERROR_DEBUG_PRINT
        (rv, (DK_VERBOSE, "PHY_INTERFACE_SET failed: %s\n", bcm_errmsg(rv)));

    PORT_UNLOCK(unit);                  /* Unlock before link call */

    if (BCM_SUCCESS(rv)) {
        SOC_PBMP_CLEAR(pbm);
        SOC_PBMP_PORT_ADD(pbm, port);
        (void)bcm_esw_link_change(unit, pbm);
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_loopback_set
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      loopback - one of:
 *              BCM_PORT_LOOPBACK_NONE
 *              BCM_PORT_LOOPBACK_MAC
 *              BCM_PORT_LOOPBACK_PHY
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_loopback_set(int unit, bcm_port_t port, int loopback)
{
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = BCM_E_NONE;
    /*
     * Always force link before changing hardware to avoid
     * race with the linkscan thread.
     */
    if (!(loopback == BCM_PORT_LOOPBACK_NONE)) {
        rv = _bcm_esw_link_force(unit, port, TRUE, FALSE);
    }

    PORT_LOCK(unit);

    if (BCM_SUCCESS(rv)) {
        rv = MAC_LOOPBACK_SET(PORT(unit, port).p_mac, unit, port,
                              (loopback == BCM_PORT_LOOPBACK_MAC));
    }
    if (BCM_SUCCESS(rv)) {
        rv = soc_phyctrl_loopback_set(unit, port,
                              (loopback == BCM_PORT_LOOPBACK_PHY));
    }

    /* some mac loopback implementations require the phy to also be in loopback */
    if (soc_feature(unit, soc_feature_phy_lb_needed_in_mac_lb) &&
        (loopback == BCM_PORT_LOOPBACK_MAC)) {
              rv = soc_phyctrl_loopback_set(unit, port, 1);
     }

    PORT_UNLOCK(unit);                  /* unlock before link call */

    if ((loopback == BCM_PORT_LOOPBACK_NONE) || !BCM_SUCCESS(rv)) {
        _bcm_esw_link_force(unit, port, FALSE, DONT_CARE);
    } else {
        /* Enable only MAC instead of calling bcm_port_enable_set so
         * that this API doesn't silently enable the port if the 
         * port is disabled by application.
         */ 
        rv = MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, TRUE);

        if (BCM_SUCCESS(rv)) {
            /* Make sure that the link status is updated only after the
             * MAC is enabled so that link_mask2 is set before the
             * calling thread synchronizes with linkscan thread in
             * _bcm_link_force call.
             * If the link is forced before MAC is enabled, there could
             * be a race condition in _soc_link_update where linkscan 
             * may use an old view of link_mask2 and override the
             * EPC_LINK_BMAP after the mac_enable_set updates 
             * link_mask2 and EPC_LINK_BMAP. 
             */
            rv = _bcm_esw_link_force(unit, port, TRUE, TRUE);
        }
        if (BCM_FAILURE(rv)) {
            return (rv);
        }

#if defined(BCM_BRADLEY_SUPPORT)
        /*
         * Call _bcm_port_mmu_update explicitly because linkscan
         * will not call bcm_port_update when the link is forced.
         */
        if (SOC_IS_HBX(unit)) {
#ifdef BCM_SHADOW_SUPPORT
            if (!SOC_IS_SHADOW(unit)) {
#endif
            rv = _bcm_port_mmu_update(unit, port, 1);
#ifdef BCM_SHADOW_SUPPORT
            }
#endif
        }
#endif /* BCM_BRADLEY_SUPPORT */
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_loopback_set: u=%d p=%d lb=%d rv=%d\n",
                     unit, port, loopback, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_loopback_get
 * Purpose:
 *      Recover the current loopback operation for the specified port.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      loopback - (OUT) one of:
 *              BCM_PORT_LOOPBACK_NONE
 *              BCM_PORT_LOOPBACK_MAC
 *              BCM_PORT_LOOPBACK_PHY
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_loopback_get(int unit, bcm_port_t port, int *loopback)
{
    int         rv = BCM_E_NONE;
    int         phy_lb = 0;
    int         mac_lb = 0;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);                    /* multiple operations operation */

    rv = soc_phyctrl_loopback_get(unit, port, &phy_lb);
    if (rv >= 0) {
        rv = MAC_LOOPBACK_GET(PORT(unit, port).p_mac, unit, port, &mac_lb);
    }

    PORT_UNLOCK(unit);

    if (rv >= 0) {
        if (mac_lb) {
            *loopback = BCM_PORT_LOOPBACK_MAC;
        } else if (phy_lb) {
            *loopback = BCM_PORT_LOOPBACK_PHY;
        } else {
            *loopback = BCM_PORT_LOOPBACK_NONE;
        }
    } else {
        *loopback = BCM_PORT_LOOPBACK_NONE;
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_loopback_get: u=%d p=%d lb=%d rv=%d\n",
                     unit, port, *loopback, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_duplex_get
 * Purpose:
 *      Get the port duplex settings
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      duplex - (OUT) Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_duplex_get(int unit, bcm_port_t port, int *duplex)
{
    int         phy_duplex;
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_duplex_get(unit, port, &phy_duplex);
    PORT_UNLOCK(unit);

    if (BCM_SUCCESS(rv)) {
        *duplex = phy_duplex ? SOC_PORT_DUPLEX_FULL : SOC_PORT_DUPLEX_HALF;
    } else {
        *duplex = SOC_PORT_DUPLEX_FULL;
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_duplex_get: u=%d p=%d dup=%d rv=%d\n",
                     unit, port, *duplex, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_duplex_set
 * Purpose:
 *      Set the port duplex settings.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      duplex - Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Turns off autonegotiation.  Caller must make sure other forced
 *      parameters (such as speed) are set.
 */

int
bcm_esw_port_duplex_set(int unit, bcm_port_t port, int duplex)
{
    int         rv;
    pbmp_t      pbm;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);
    SOC_IF_ERROR_DEBUG_PRINT
        (rv, (DK_VERBOSE, "PHY_AUTONEG_SET failed: %s\n", bcm_errmsg(rv)));

    if (BCM_SUCCESS(rv)) {
        rv = soc_phyctrl_duplex_set(unit, port, duplex);
        SOC_IF_ERROR_DEBUG_PRINT
            (rv, (DK_VERBOSE, "PHY_DUPLEX_SET failed: %s\n", bcm_errmsg(rv)));
    }

    if (BCM_SUCCESS(rv)) {
        rv = MAC_DUPLEX_SET(PORT(unit, port).p_mac, unit, port, duplex);
        SOC_IF_ERROR_DEBUG_PRINT
            (rv, (DK_VERBOSE, "MAC_DUPLEX_SET failed: %s\n", bcm_errmsg(rv)));
    }

    PORT_UNLOCK(unit);                  /* Unlock before link call */

    if (BCM_SUCCESS(rv) && !SAL_BOOT_SIMULATION) {
        SOC_PBMP_CLEAR(pbm);
        SOC_PBMP_PORT_ADD(pbm, port);
        (void)bcm_esw_link_change(unit, pbm);
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_duplex_set: u=%d p=%d dup=%d rv=%d\n",
                     unit, port, duplex, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_pause_get
 * Purpose:
 *      Get the source address for transmitted PAUSE frames.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      pause_tx - (OUT) Boolean value
 *      pause_rx - (OUT) Boolean value
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_pause_get(int unit, bcm_port_t port, int *pause_tx, int *pause_rx)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = MAC_PAUSE_GET(PORT(unit, port).p_mac, unit, port, pause_tx, pause_rx);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_pause_set
 * Purpose:
 *      Set the pause state for a given port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      pause_tx - Boolean value, or -1 (don't change)
 *      pause_rx - Boolean value, or -1 (don't change)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Symmetric pause requires the two "pause" values to be the same.
 */

int
bcm_esw_port_pause_set(int unit, bcm_port_t port, int pause_tx, int pause_rx)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = MAC_PAUSE_SET(PORT(unit, port).p_mac, unit, port, pause_tx, pause_rx);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return (rv);
    }

#if defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_HBX(unit)) {
#ifdef BCM_SHADOW_SUPPORT
    if (!SOC_IS_SHADOW(unit)) {
#endif
        rv = _bcm_port_mmu_update(unit, port, -1);
#ifdef BCM_SHADOW_SUPPORT
    }
#endif
    }
#endif /* BCM_BRADLEY_SUPPORT */ 

    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_pause_sym_get
 * Purpose:
 *      Get the current pause setting for pause
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      pause - (OUT) returns a bcm_port_pause_e enum value
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_pause_sym_get(int unit, bcm_port_t port, int *pause)
{
    int         rv;
    int         pause_rx, pause_tx;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = MAC_PAUSE_GET(PORT(unit, port).p_mac, unit, port,
                       &pause_tx, &pause_rx);
    PORT_UNLOCK(unit);

    BCM_IF_ERROR_RETURN(rv);
    if (pause_tx) {
        if (pause_rx) {
            *pause = BCM_PORT_PAUSE_SYM;
        } else {
            *pause = BCM_PORT_PAUSE_ASYM_TX;
        }
    } else if (pause_rx) {
        *pause = BCM_PORT_PAUSE_ASYM_RX;
    } else {
        *pause = BCM_PORT_PAUSE_NONE;
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_pause_sym_set
 * Purpose:
 *      Set the pause values for the port using single integer
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      pause - a bcm_port_pause_e enum value
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_pause_sym_set(int unit, bcm_port_t port, int pause)
{
    int         pause_rx, pause_tx;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    pause_tx = pause_rx = 0;

    switch (pause) {
    case BCM_PORT_PAUSE_SYM:
        pause_tx = pause_rx = 1;
        break;
    case BCM_PORT_PAUSE_ASYM_RX:
        pause_rx = 1;
        break;
    case BCM_PORT_PAUSE_ASYM_TX:
        pause_tx = 1;
        break;
    case BCM_PORT_PAUSE_NONE:
        break;
    default:
        return BCM_E_PARAM;
    }

    return bcm_esw_port_pause_set(unit, port, pause_tx, pause_rx);
}

/*
 * Function:
 *      bcm_port_pause_addr_get
 * Purpose:
 *      Get the source address for transmitted PAUSE frames.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      mac - (OUT) MAC address sent with pause frames.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_pause_addr_get(int unit, bcm_port_t port, bcm_mac_t mac)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = MAC_PAUSE_ADDR_GET(PORT(unit, port).p_mac, unit, port, mac);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_pause_addr_set
 * Purpose:
 *      Set the source address for transmitted PAUSE frames.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      mac - station address used for pause frames.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Symmetric pause requires the two "pause" values to be the same.
 */

int
bcm_esw_port_pause_addr_set(int unit, bcm_port_t port, bcm_mac_t mac)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = MAC_PAUSE_ADDR_SET(PORT(unit, port).p_mac, unit, port, mac);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_advert_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - (OUT) Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_advert_get(int unit, bcm_port_t port, bcm_port_abil_t *ability_mask)
{
    int                 rv;
    bcm_port_ability_t  ability;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_ability_advert_get(unit, port, &ability);
    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&ability, ability_mask);
    }
    PORT_UNLOCK(unit);



    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_advert_get: u=%d p=%d abil=0x%x rv=%d\n",
                     unit, port, *ability_mask, rv));

    return rv;
}

/*
 * Function:
 *      bcm_esw_port_ability_advert_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - (OUT) Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_ability_advert_get(int unit, bcm_port_t port, 
                                bcm_port_ability_t *ability_mask)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_ability_advert_get(unit, port, ability_mask);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_esw_port_ability_advert_get: u=%d p=%d rv=%d\n",
                     unit, port, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_advert_set
 * Purpose:
 *      Set the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      This call MAY NOT restart autonegotiation (depending on the phy).
 *      To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */

int
bcm_esw_port_advert_set(int unit, bcm_port_t port, bcm_port_abil_t ability_mask)
{
    int                 rv;
    bcm_port_ability_t  given_ability, port_ability;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    
    BCM_IF_ERROR_RETURN
        (bcm_esw_port_ability_local_get(unit, port, &port_ability));

    BCM_IF_ERROR_RETURN
        (bcm_esw_port_ability_advert_get(unit, port, &given_ability));

    BCM_IF_ERROR_RETURN(
        soc_port_mode_to_ability(ability_mask, &given_ability));

    /* make sure that the ability advertising in PHY is supported by MAC */ 
    given_ability.flags &= port_ability.flags;
    given_ability.loopback &= port_ability.loopback;
    given_ability.medium &= port_ability.medium;
    given_ability.eee    &= port_ability.eee;
    given_ability.pause &= port_ability.pause;
    given_ability.speed_full_duplex &= port_ability.speed_full_duplex;
    given_ability.speed_half_duplex &= port_ability.speed_half_duplex;

    if (IS_HG_PORT(unit, port) && SOC_INFO(unit).port_speed_max[port]) {
        if (SOC_INFO(unit).port_speed_max[port] < 16000) {
            given_ability.speed_full_duplex &= ~(BCM_PORT_ABILITY_16GB);
            given_ability.speed_half_duplex &= ~(BCM_PORT_ABILITY_16GB);
        }
        
        if (SOC_INFO(unit).port_speed_max[port] < 13000) {
            given_ability.speed_full_duplex &= ~(BCM_PORT_ABILITY_13GB);
            given_ability.speed_half_duplex &= ~(BCM_PORT_ABILITY_13GB);
        }

        if (SOC_INFO(unit).port_speed_max[port] < 12000) {
            given_ability.speed_full_duplex &= ~(BCM_PORT_ABILITY_12GB);
            given_ability.speed_half_duplex &= ~(BCM_PORT_ABILITY_12GB);
        }  

        if (!(given_ability.speed_full_duplex & (BCM_PORT_ABILITY_16GB |
              BCM_PORT_ABILITY_13GB | BCM_PORT_ABILITY_12GB | 
              BCM_PORT_ABILITY_10GB))) {
            return BCM_E_CONFIG;
        }
    }
 
    PORT_LOCK(unit);
    rv = soc_phyctrl_ability_advert_set(unit, port, &given_ability);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_advert_set: u=%d p=%d abil=0x%x rv=%d\n",
                     unit, port, ability_mask, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_ability_advert_set
 * Purpose:
 *      Set the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - Local advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      This call MAY NOT restart autonegotiation (depending on the phy).
 *      To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */

int
bcm_esw_port_ability_advert_set(int unit, bcm_port_t port,
                                bcm_port_ability_t *ability_mask)
{
    int             rv;
    bcm_port_ability_t port_ability;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    
    BCM_IF_ERROR_RETURN
        (bcm_esw_port_ability_local_get(unit, port, &port_ability));

    /* Make sure to advertise only abilities supported by the port */
    port_ability.speed_half_duplex   &= ability_mask->speed_half_duplex;
    port_ability.speed_full_duplex   &= ability_mask->speed_full_duplex;
    port_ability.pause      &= ability_mask->pause;
    port_ability.interface  &= ability_mask->interface;
    port_ability.medium     &= ability_mask->medium;
    port_ability.eee        &= ability_mask->eee;
    port_ability.loopback   &= ability_mask->loopback;
    port_ability.flags      &= ability_mask->flags;

    PORT_LOCK(unit);
    rv = soc_phyctrl_ability_advert_set(unit, port, &port_ability);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_ability_advert_set: u=%d p=%d rv=%d\n",
                     unit, port, rv));
    SOC_DEBUG_PRINT((DK_PORT | DK_VERBOSE,
                     "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x\n"
                     "Interface=0x%08x Medium=0x%08x Loopback=0x%08x Flags=0x%08x\n",
                     port_ability.speed_half_duplex,
                     port_ability.speed_full_duplex,
                     port_ability.pause, port_ability.interface,
                     port_ability.medium, port_ability.loopback,
                     port_ability.flags));

    return rv;
}

/*
 * Function:
 *      bcm_port_advert_remote_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - (OUT) Remote advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_advert_remote_get(int unit, bcm_port_t port, bcm_port_abil_t *ability_mask)
{
    int                 rv;
    bcm_port_ability_t  port_ability;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = _bcm_port_autoneg_advert_remote_get(unit, port, &port_ability);
    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&port_ability, ability_mask);
    }
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_advert_remote_get: u=%d p=%d abil=0x%x rv=%d\n",
                     unit, port, *ability_mask, rv));

    return rv;
}


/*
 * Function:
 *      bcm_port_ability_remote_get
 * Purpose:
 *      Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - (OUT) Remote advertisement.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_esw_port_ability_remote_get(int unit, bcm_port_t port,
                           bcm_port_ability_t *ability_mask)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = _bcm_port_autoneg_advert_remote_get(unit, port, ability_mask);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_ability_remote_get: u=%d p=%d rv=%d\n",
                     unit, port, rv));
    SOC_DEBUG_PRINT((DK_PORT | DK_VERBOSE,
                     "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x\n"
                     "Interface=0x%08x Medium=0x%08x Loopback=0x%08x Flags=0x%08x\n",
                     ability_mask->speed_half_duplex,
                     ability_mask->speed_full_duplex,
                     ability_mask->pause, ability_mask->interface,
                     ability_mask->medium, ability_mask->loopback,
                     ability_mask->flags));

    return rv;
}

/*
 * Function:
 *      bcm_port_ability_get
 * Purpose:
 *      Retrieve the local port abilities.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *              ability of the MAC/PHY.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_ability_get(int unit, bcm_port_t port, bcm_port_abil_t *ability_mask)
{
    int                 rv;
    bcm_port_ability_t  port_ability;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = _bcm_port_ability_local_get(unit, port, &port_ability);
    if (BCM_SUCCESS(rv)) {
        rv = soc_port_ability_to_mode(&port_ability, ability_mask);
    }
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_ability_get: u=%d p=%d abil=0x%x rv=%d\n",
                     unit, port, *ability_mask, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_ability_local_get
 * Purpose:
 *      Retrieve the local port abilities.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *              ability of the MAC/PHY.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_ability_local_get(int unit, bcm_port_t port,
                               bcm_port_ability_t *ability_mask)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = _bcm_port_ability_local_get(unit, port, ability_mask);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_ability_local_get: u=%d p=%d rv=%d\n",
                     unit, port, rv));
    SOC_DEBUG_PRINT((DK_PORT | DK_VERBOSE,
                     "Speed(HD=0x%08x, FD=0x%08x) Pause=0x%08x\n"
                     "Interface=0x%08x Medium=0x%08x EEE=0x%08x Loopback=0x%08x Flags=0x%08x\n",
                     ability_mask->speed_half_duplex,
                     ability_mask->speed_full_duplex,
                     ability_mask->pause, ability_mask->interface,
                     ability_mask->medium, ability_mask->eee,
                     ability_mask->loopback, ability_mask->flags));

    return rv;
}
/*
 * Function:
 *      bcm_port_discard_get
 * Purpose:
 *      Get port discard attributes for the specified port
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) Port discard mode, one of BCM_PORT_DISCARD_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_discard_get(int unit, bcm_port_t port, int *mode)
{
    bcm_port_cfg_t pcfg;
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if ((rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg)) ==
        BCM_E_NONE) {
        *mode = pcfg.pc_disc;
    }

    return(rv);
}

/*
 * Function:
 *      bcm_port_discard_set
 * Purpose:
 *      Set port discard attributes for the specified port.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      mode - Port discard mode, one of BCM_PORT_DISCARD_xxx
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_discard_set(int unit, bcm_port_t port, int mode)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    switch (mode) {
    case BCM_PORT_DISCARD_TAG:
#ifdef BCM_XGS12_SWITCH_SUPPORT
        if (SOC_IS_DRACO1(unit) || SOC_IS_LYNX(unit) ||
            SOC_IS_TUCANA(unit)) {
            /* This mode not supported on 5690/73/74/65/50 */
            return BCM_E_UNAVAIL;
        }
#endif
        /* Fall through */
    case BCM_PORT_DISCARD_NONE:
    case BCM_PORT_DISCARD_ALL:
    case BCM_PORT_DISCARD_UNTAG:
        break;
    default:
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    if (BCM_SUCCESS(rv)) {
        pcfg.pc_disc = mode;
        rv = mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg);
    }

    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_port_learn_modify
 * Purpose:
 *      Main part of bcm_port_learn_modify.
 */

STATIC int
_bcm_port_learn_modify(int unit, bcm_port_t port, uint32 add, uint32 remove)
{
    uint32      flags;

    SOC_IF_ERROR_RETURN(bcm_esw_port_learn_get(unit, port, &flags));

    flags |= add;
    flags &= ~remove;

    SOC_IF_ERROR_RETURN(bcm_esw_port_learn_set(unit, port, flags));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_learn_modify
 * Purpose:
 *      Modify the port learn flags, adding add and removing remove flags.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      add  - Flags to set.
 *      remove - Flags to clear.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_learn_modify(int unit, bcm_port_t port, uint32 add, uint32 remove)
{
    int         rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = _bcm_port_learn_modify(unit, port, add, remove);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_learn_get
 * Purpose:
 *      Get the ARL hardware learning options for this port.
 *      This defines what the hardware will do when a packet
 *      is seen with an unknown address.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      flags - (OUT) Logical OR of BCM_PORT_LEARN_xxx flags
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_learn_get(int unit, bcm_port_t port, uint32 *flags)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

#if defined(BCM_TRX_SUPPORT) && defined(INCLUDE_L3) 
    if (BCM_GPORT_IS_SUBPORT_GROUP(port)) {
        return bcm_tr_subport_learn_get(unit, (bcm_gport_t) port, flags);
    }
#endif /* BCM_TRX_SUPPORT && INCLUDE_L3 */
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT) && \
    defined(INCLUDE_L3)
    if (BCM_GPORT_IS_MPLS_PORT(port)) {
        return bcm_tr_mpls_port_learn_get(unit, (bcm_gport_t) port, flags);
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3) 
    if (BCM_GPORT_IS_MIM_PORT(port)) {
        return bcm_tr2_mim_port_learn_get(unit, (bcm_gport_t) port, flags);
    }
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        return bcm_tr2_wlan_port_learn_get(unit, (bcm_gport_t) port, flags);
    }
#endif /* BCM_TRIUMPH2_SUPPORT && INCLUDE_L3 */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    
    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    PORT_UNLOCK(unit);

    BCM_IF_ERROR_RETURN(rv);

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        *flags = 0;
        BCM_IF_ERROR_RETURN(
            _bcm_trx_port_cml_hw2flags(unit, pcfg.pc_cml, flags));
    } else 
#endif
    {
        switch (pcfg.pc_cml) {
        case PVP_CML_SWITCH:
            *flags = (BCM_PORT_LEARN_ARL |
                      BCM_PORT_LEARN_FWD |
                      (pcfg.pc_cpu ? BCM_PORT_LEARN_CPU : 0));
            break;
        case PVP_CML_CPU:
            *flags = BCM_PORT_LEARN_CPU;
            break;
        case PVP_CML_FORWARD:
            *flags = BCM_PORT_LEARN_FWD;
            break;
        case PVP_CML_DROP:
            *flags = 0;
            break;
        case PVP_CML_CPU_SWITCH:            /* Possible on Draco only */
            *flags = (BCM_PORT_LEARN_ARL |
                      BCM_PORT_LEARN_CPU |
                      BCM_PORT_LEARN_FWD);
            break;
        case PVP_CML_CPU_FORWARD:           /* Possible on Draco only */
            *flags = BCM_PORT_LEARN_CPU | BCM_PORT_LEARN_FWD;
            break;
        default:
            return BCM_E_INTERNAL;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_learn_set
 * Purpose:
 *      Set the ARL hardware learning options for this port.
 *      This defines what the hardware will do when a packet
 *      is seen with an unknown address.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      flags - Logical OR of BCM_PORT_LEARN_xxx flags
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

#define Arl     BCM_PORT_LEARN_ARL
#define Cpu     BCM_PORT_LEARN_CPU
#define Fwd     BCM_PORT_LEARN_FWD

int
bcm_esw_port_learn_set(int unit, bcm_port_t port, uint32 flags)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

#if defined(BCM_TRX_SUPPORT)  && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_SUBPORT_GROUP(port)) {
        return bcm_tr_subport_learn_set(unit, (bcm_gport_t) port, flags);
    }
#endif /* BCM_TRX_SUPPORT && INCLUDE_L3 */
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT) && \
    defined(INCLUDE_L3)
    if (BCM_GPORT_IS_MPLS_PORT(port)) {
        return bcm_tr_mpls_port_learn_set(unit, (bcm_gport_t) port, flags);
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3) 
    if (BCM_GPORT_IS_MIM_PORT(port)) {
        return bcm_tr2_mim_port_learn_set(unit, (bcm_gport_t) port, flags);
    }
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        return bcm_tr2_wlan_port_learn_set(unit, (bcm_gport_t) port, flags);
    }
#endif /* BCM_TRIUMPH2_SUPPORT && INCLUDE_L3 */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (flags & BCM_PORT_LEARN_PENDING) {
        if (!soc_feature(unit, soc_feature_l2_pending)) {
            return BCM_E_UNAVAIL;
        } else if (!IS_CPU_PORT(unit, port) & (!(flags & BCM_PORT_LEARN_ARL))) {
            /* When the PENDING flag is set, the ARL must also be set */
            return BCM_E_PARAM;
        }
    }

    PORT_LOCK(unit);

    rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);

    pcfg.pc_cpu = ((flags & BCM_PORT_LEARN_CPU) != 0);

    /* Use shortened names to handle each flag combination individually */

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        pcfg.pc_cml = 0;
        BCM_IF_ERROR_RETURN(
            _bcm_trx_port_cml_flags2hw(unit, flags, (uint32 *)&pcfg.pc_cml));
    } else
#endif
    {
        switch (flags) {
        case ((!Arl) | (!Cpu) | (!Fwd)):
            pcfg.pc_cml = PVP_CML_DROP;
            break;
        case ((!Arl) | (!Cpu) | ( Fwd)):
            pcfg.pc_cml = PVP_CML_FORWARD;
            break;
        case ((!Arl) | ( Cpu) | (!Fwd)):
            pcfg.pc_cml = PVP_CML_CPU;
            break;
        case ((!Arl) | ( Cpu) | ( Fwd)):
            if (SOC_IS_XGS_SWITCH(unit)) {
                pcfg.pc_cml = PVP_CML_CPU_FORWARD;
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case (( Arl) | (!Cpu) | (!Fwd)):
            rv = BCM_E_UNAVAIL;
            break;
        case (( Arl) | (!Cpu) | ( Fwd)):
            pcfg.pc_cml = PVP_CML_SWITCH;
            break;
        case (( Arl) | ( Cpu) | (!Fwd)):
            rv = BCM_E_UNAVAIL;
            break;
        case (( Arl) | ( Cpu) | ( Fwd)):
            if (SOC_IS_XGS_SWITCH(unit)) {
                pcfg.pc_cml = PVP_CML_CPU_SWITCH;
            } else {
                pcfg.pc_cml = PVP_CML_SWITCH;       /* pc_cpu also being set */
            }
            break;
        }
    }

    if (BCM_SUCCESS(rv)) {
        rv = mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg);
    }

    PORT_UNLOCK(unit);

    return rv;
}

#undef Arl
#undef Cpu
#undef Fwd

/*
 * Function:
 *      bcm_port_ifilter_get
 * Description:
 *      Return input filter mode for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number to operate on
 *      mode - (OUT) Filter mode, 1 is enable; 0 is disable. 
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_INTERNAL          Chip access failure.
 * Notes:
 *      This API is superseded by bcm_port_vlan_member_get.
 */

int
bcm_esw_port_ifilter_get(int unit, bcm_port_t port, int *mode)
{
    int rv;
    uint32 flags;

    rv = bcm_esw_port_vlan_member_get(unit, port, &flags);

    if (BCM_SUCCESS(rv)) {
        *mode = (flags & BCM_PORT_VLAN_MEMBER_INGRESS) ? 1 : 0;
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_ifilter_set
 * Description:
 *      Set input filter mode for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number to operate on
 *      mode - 1 to enable, 0 to disable.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_INTERNAL          Chip access failure.
 * Notes:
 *      This API is superseded by bcm_port_vlan_member_set.
 *
 *      When input filtering is turned on for a port, packets received
 *      on the port that do not match the port's VLAN classifications
 *      are discarded.
 */

int
bcm_esw_port_ifilter_set(int unit, bcm_port_t port, int mode)
{
    uint32 flags;

    BCM_IF_ERROR_RETURN(bcm_esw_port_vlan_member_get(unit, port, &flags));

    if (mode) {
        flags |= BCM_PORT_VLAN_MEMBER_INGRESS;
    }  else {
        flags &= (~BCM_PORT_VLAN_MEMBER_INGRESS);
    }

    return bcm_esw_port_vlan_member_set(unit, port, flags);
}

/*
 * Function:
 *      bcm_esw_port_vlan_member_get
 * Description:
 *      Return filter mode for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number to operate on
 *      flags - (OUT) Filter mode, one of BCM_PORT_VLAN_MEMBER_xxx. 
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_INTERNAL          Chip access failure.
 */

int
bcm_esw_port_vlan_member_get(int unit, bcm_port_t port, uint32 *flags)
{
    int  rv = BCM_E_UNAVAIL;

    *flags = 0;

#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return bcm_td_vp_vlan_member_get(unit, port, flags);
    } else
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        rv = BCM_E_NONE;
    } else {
        rv = _bcm_esw_port_gport_validate(unit, port, &port);
    }
    BCM_IF_ERROR_RETURN(rv);
    rv = BCM_E_UNAVAIL;

    switch (BCM_CHIP_FAMILY(unit)) {

    case BCM_FAMILY_HERCULES:
        rv = BCM_E_NONE;
        break;

    case BCM_FAMILY_TUCANA:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_LYNX:

    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_EASYRIDER:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
        if (SOC_IS_XGS12_SWITCH(unit) && IS_HG_PORT(unit, port)) {
            rv = BCM_E_NONE;
        } else {
            port_tab_entry_t    pent;
            soc_mem_t mem;

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
            if (BCM_GPORT_IS_WLAN_PORT(port)) {
                int value;
                if (!soc_feature(unit, soc_feature_wlan)) {
                    return BCM_E_PORT;
                }
                rv = _bcm_tr2_lport_field_get(unit, port, EN_IFILTERf, &value);
                if (value) {
                    *flags |= BCM_PORT_VLAN_MEMBER_INGRESS;
                }
                return rv;
            }
#endif
            mem = SOC_PORT_MEM_TAB(unit, port);
            rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                              SOC_PORT_MOD_OFFSET(unit, port), &pent);

            if (BCM_SUCCESS(rv) &&
                soc_PORT_TABm_field32_get(unit, &pent, EN_IFILTERf)) {
                    *flags |= BCM_PORT_VLAN_MEMBER_INGRESS;
            }

#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_MEM_IS_VALID(unit, EGR_PORTm)) {
                egr_port_entry_t egr_port_entry;

                rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port,
                                    &egr_port_entry);
                if (BCM_SUCCESS(rv) &&
                    soc_mem_field32_get(unit, EGR_PORTm, &egr_port_entry,
                                        EN_EFILTERf)) {
                    *flags |= BCM_PORT_VLAN_MEMBER_EGRESS;
                }
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            if (SOC_IS_XGS3_SWITCH(unit)) {
                uint32 egr_val;
                uint64 egr_val64;
                if (SOC_REG_IS_VALID(unit, EGR_PORTr)) {
                    rv = READ_EGR_PORTr(unit, port, &egr_val);
                } else {
                    rv = READ_EGR_PORT_64r(unit, port, &egr_val64);
                }
                if (BCM_SUCCESS(rv)) {
                    if (SOC_REG_IS_VALID(unit, EGR_PORTr)) {
                        if (soc_reg_field_valid(unit, EGR_PORTr, EN_EFILTERf)) {
                            if (soc_reg_field_get(unit, EGR_PORTr, 
                                                  egr_val, EN_EFILTERf)) {
                                *flags |= BCM_PORT_VLAN_MEMBER_EGRESS;
                            }
                        }
                    } else if (soc_reg64_field32_get(unit, 
                                   EGR_PORT_64r, egr_val64, EN_EFILTERf)) {
                            *flags |= BCM_PORT_VLAN_MEMBER_EGRESS;
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_port_vlan_member_set
 * Description:
 *      Set ingress and egress filter mode for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number to operate on
 *      flags - BCM_PORT_VLAN_MEMBER_xxx.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_INTERNAL          Chip access failure.
 */

int
bcm_esw_port_vlan_member_set(int unit, bcm_port_t port, uint32 flags)
{
    int       rv = BCM_E_UNAVAIL;

#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return bcm_td_vp_vlan_member_set(unit, port, flags);
    } else
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        rv = BCM_E_NONE;
    } else {
        rv = _bcm_esw_port_gport_validate(unit, port, &port);
    }
    BCM_IF_ERROR_RETURN(rv);
    rv = BCM_E_UNAVAIL;

    PORT_LOCK(unit);

    switch (BCM_CHIP_FAMILY(unit)) {

    case BCM_FAMILY_HERCULES:
        rv = (flags & BCM_PORT_VLAN_MEMBER_INGRESS) ? BCM_E_UNAVAIL :
              ((flags & BCM_PORT_VLAN_MEMBER_EGRESS) ? BCM_E_UNAVAIL : BCM_E_NONE);
        break;

    case BCM_FAMILY_TUCANA:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_LYNX:

    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_EASYRIDER:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
        if (SOC_IS_XGS12_SWITCH(unit) && IS_HG_PORT(unit, port)) {
            rv = (flags & BCM_PORT_VLAN_MEMBER_INGRESS) ? BCM_E_UNAVAIL :
                  ((flags & BCM_PORT_VLAN_MEMBER_EGRESS) ? BCM_E_UNAVAIL : 
                  BCM_E_NONE);
        } else {
            port_tab_entry_t pent;
            soc_mem_t mem;

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
            if (BCM_GPORT_IS_WLAN_PORT(port)) {
                if (!soc_feature(unit, soc_feature_wlan)) {
                    PORT_UNLOCK(unit);
                    return BCM_E_PORT;
                }
                if (flags & BCM_PORT_VLAN_MEMBER_INGRESS) {
                    rv = _bcm_tr2_wlan_port_set(unit, port, EN_IFILTERf, 1);
                } else {
                    rv = _bcm_tr2_wlan_port_set(unit, port, EN_IFILTERf, 0);
                }
                PORT_UNLOCK(unit);
                return rv;
            }
#endif
            mem = SOC_PORT_MEM_TAB(unit, port);
            rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                              SOC_PORT_MOD_OFFSET(unit, port), &pent);

            if (BCM_SUCCESS(rv)) {
                soc_PORT_TABm_field32_set(unit, &pent, EN_IFILTERf,
                              (flags & BCM_PORT_VLAN_MEMBER_INGRESS) ? 1 : 0);
                rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, port, &pent);
            }

#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_MEM_IS_VALID(unit, ING_EN_EFILTER_BITMAPm)) {
                uint32 efilter;
                ing_en_efilter_bitmap_entry_t entry;
                pbmp_t pbmp;

                efilter = flags & BCM_PORT_VLAN_MEMBER_EGRESS ? 1 : 0;
                rv = soc_mem_field32_modify(unit, EGR_PORTm, port, EN_EFILTERf,
                                            efilter);
                if (BCM_SUCCESS(rv)) {
                    rv = soc_mem_read(unit, ING_EN_EFILTER_BITMAPm,
                                      MEM_BLOCK_ANY, 0, &entry);
                }
                if (BCM_SUCCESS(rv)) {
                    soc_mem_pbmp_field_get(unit, ING_EN_EFILTER_BITMAPm,
                                           &entry, BITMAPf, &pbmp);
                    if (efilter && PORT(unit, port).vp_count == 0) {
                        SOC_PBMP_PORT_ADD(pbmp, port);
                    } else {
                        SOC_PBMP_PORT_REMOVE(pbmp, port);
                    }
                    soc_mem_pbmp_field_set(unit, ING_EN_EFILTER_BITMAPm,
                                           &entry, BITMAPf, &pbmp);
                    rv = soc_mem_write(unit, ING_EN_EFILTER_BITMAPm,
                                       MEM_BLOCK_ANY, 0, &entry);
                }
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            if (SOC_IS_XGS3_SWITCH(unit)) {
                soc_reg_t egr_port_reg;
                egr_port_reg = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                                SOC_IS_VALKYRIE2(unit)) ? EGR_PORT_64r : EGR_PORTr;
                rv = soc_reg_field32_modify(unit, egr_port_reg, port, EN_EFILTERf, 
                                          (flags & BCM_PORT_VLAN_MEMBER_EGRESS) ? 1 : 0);
                if (BCM_SUCCESS(rv)) {
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || \
        defined(BCM_SCORPION_SUPPORT)
                    if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit) ||
                        SOC_IS_SC_CQ(unit) || SOC_IS_HAWKEYE(unit)) {
                        uint32 egr_val;
                        pbmp_t pbmp;
                        uint32 bitmap, pbmp32;

                        rv = READ_ING_EN_EFILTER_BITMAPr(unit, &egr_val);
                        if (BCM_SUCCESS(rv)) {
                            SOC_PBMP_PORT_SET(pbmp, port);
                            pbmp32 = SOC_PBMP_WORD_GET(pbmp, 0);
                            bitmap =
                                soc_reg_field_get(unit,
                                                  ING_EN_EFILTER_BITMAPr,
                                                  egr_val, BITMAPf);
                            if (flags & BCM_PORT_VLAN_MEMBER_EGRESS) {
                                bitmap |= pbmp32;
                            } else {
                                bitmap &= ~pbmp32;
                            }
                            soc_reg_field_set(unit, ING_EN_EFILTER_BITMAPr,
                                              &egr_val, BITMAPf, bitmap);
                            rv = WRITE_ING_EN_EFILTER_BITMAPr(unit, egr_val);
                        }
                    }
#endif /* BCM_FIREBOLT_SUPPORT || BCM_RAVEN_SUPPORT || BCM_SCORPION_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
                    /* Do this only if there is no VP on this port */
                    if (SOC_IS_TR_VL(unit)) {
                        uint64 egr_val_64;
                        pbmp_t pbmp;
                        uint32 bitmap, pbmp32;

                        rv = READ_ING_EN_EFILTER_BITMAP_64r(unit, &egr_val_64);
                        if (BCM_SUCCESS(rv)) {
                            SOC_PBMP_PORT_SET(pbmp, port);
                            /* Low bitmap */
                            bitmap =
                                soc_reg64_field32_get(unit,
                                                      ING_EN_EFILTER_BITMAP_64r,
                                                      egr_val_64, BITMAP_LOf);
                            pbmp32 = SOC_PBMP_WORD_GET(pbmp, 0);
                            if ((flags & BCM_PORT_VLAN_MEMBER_EGRESS)  && 
                                (PORT(unit, port).vp_count == 0)) {
                                bitmap |= pbmp32;
                            } else {
                                bitmap &= ~pbmp32;
                            }
                            soc_reg64_field32_set(unit, ING_EN_EFILTER_BITMAP_64r,
                                                  &egr_val_64, BITMAP_LOf, bitmap);

                            if(!SOC_IS_ENDURO(unit) && !SOC_IS_HURRICANE(unit)) {
                                    /* High bitmap */
                                    bitmap =
                                        soc_reg64_field32_get(unit,
                                                              ING_EN_EFILTER_BITMAP_64r,
                                                              egr_val_64, BITMAP_HIf);
                                    pbmp32 = SOC_PBMP_WORD_GET(pbmp, 1);
                                    if (flags & BCM_PORT_VLAN_MEMBER_EGRESS) {
                                        bitmap |= pbmp32;
                                    } else {
                                        bitmap &= ~pbmp32;
                                    }
                                    soc_reg64_field32_set(unit, ING_EN_EFILTER_BITMAP_64r,
                                                          &egr_val_64, BITMAP_HIf, bitmap);
                            }

                            rv = WRITE_ING_EN_EFILTER_BITMAP_64r(unit, egr_val_64);
                        }
                    }
#endif
                }
            }
        }
        break;

    default:
        break;
    }

    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_port_link_get
 * Purpose:
 *      Return current PHY up/down status
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      hw - If TRUE, assume hardware linkscan is active and use it
 *              to reduce PHY reads.
 *           If FALSE, do not use information from hardware linkscan.
 *      up - (OUT) TRUE for link up, FALSE for link down.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
_bcm_port_link_get(int unit, bcm_port_t port, int hw, int *up)
{
    int     rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    if (hw) {
        pbmp_t hw_linkstat;

        rv = soc_linkscan_hw_link_get(unit, &hw_linkstat);

        *up = PBMP_MEMBER(hw_linkstat, port);

        /*
         * We need to confirm link down because we may receive false link
         * change interrupts when hardware and software linkscan are mixed.
         * Processing a false link down event is known to cause packet
         * loss, which is obviously unacceptable.
         */
        if(!(*up)) {
            rv = soc_phyctrl_link_get(unit, port, up);
        }
    } else {
        if (SOC_IS_RCPU_ONLY(unit)) {
            rv = MAC_ENABLE_GET(PORT(unit, port).p_mac, unit, port, up);
        } else {
            rv = soc_phyctrl_link_get(unit, port, up);
        }
    }

    if (BCM_SUCCESS(rv)) {
        if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_MEDIUM_CHANGE)) {
            soc_port_medium_t  medium;
            soc_phyctrl_medium_get(unit, port, &medium);
            soc_phy_medium_status_notify(unit, port, medium);
        }
    }

    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT | DK_VERBOSE | DK_LINK,
                     "_bcm_port_link_get: u=%d p=%d hw=%d up=%d rv=%d\n",
                     unit, port, hw, *up, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_link_status_get
 * Purpose:
 *      Return current Link up/down status, queries linkscan, if unable to
 *      retrieve status queries the PHY.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      up - (OUT) Boolean value, FALSE for link down and TRUE for link up
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_link_status_get(int unit, bcm_port_t port, int *up)
{
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = _bcm_esw_link_get(unit, port, up);

    if (rv == BCM_E_DISABLED) {
        int mode;
        
        BCM_IF_ERROR_RETURN(bcm_esw_linkscan_mode_get(unit, port, &mode));

        if (mode == BCM_LINKSCAN_MODE_HW) {
            rv = _bcm_port_link_get(unit, port, 1, up);
        } else {
            rv = _bcm_port_link_get(unit, port, 0, up);
        }
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_link_status_get: u=%d p=%d up=%d rv=%d\n",
                     unit, port, *up, rv));

    return rv;
}

/*
 * Function:
 *      bcm_esw_port_link_failed_clear
 * Purpose:
 *      Clear failed link status from a port which has undergone
 *      LAG failover.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 * Notes:
 *      The port is moved to down status.  The application is responsible
 *      for removing the port from all trunk memberships before calling this
 *      function.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_link_failed_clear(int unit, bcm_port_t port)
{
    return _bcm_esw_link_failed_clear(unit, port);
}

/*
 * Function:
 *      _bcm_port_pfm_set
 * Purpose:
 *      Main part of function bcm_port_pfm_set
 */

STATIC int
_bcm_port_pfm_set(int unit, bcm_port_t port, int mode)
{
    int                 rv = BCM_E_NONE;

    /*
     * The filter mode for the first 16 ports is in one register
     * and for the rest in another
     */
    switch (BCM_CHIP_FAMILY(unit)) {

#ifdef BCM_XGS12_FABRIC_SUPPORT
    case BCM_FAMILY_HERCULES:
        /* See remarks under bcm_port_pfm_get below */
        break;
#endif

#ifdef BCM_XGS3_SWITCH_SUPPORT
    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_EASYRIDER:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
        /*
         * PFM is a per VLAN attribute. It is not a per port attribute
         * on XGS3
         */
        return BCM_E_UNAVAIL;
#endif

#ifdef  BCM_XGS12_SWITCH_SUPPORT
    case BCM_FAMILY_TUCANA:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_LYNX:
        if (IS_E_PORT(unit,port)) {
            soc_mem_t mem = SOC_PORT_MEM_TAB(unit, port);
            port_tab_entry_t    pent;

            sal_memset(&pent, 0, sizeof(pent));
            BCM_IF_ERROR_RETURN(
                soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                             SOC_PORT_MOD_OFFSET(unit, port), &pent));
            soc_mem_field32_force(unit, mem, &pent, PFMf, (uint32)mode);
            soc_PORT_TABm_field32_set(unit, &pent, PFMf, mode);
            BCM_IF_ERROR_RETURN(
                soc_mem_write(unit, mem, MEM_BLOCK_ALL,
                              SOC_PORT_MOD_OFFSET(unit, port), &pent));
        } else if (IS_HG_PORT(unit, port)) {
            /* For now, ignore on IPIC.  Pending final decision */
        } else {
            rv = BCM_E_PORT;
        }
        break;
#endif /* BCM_XGS12_SWITCH_SUPPORT */

    default:
        return BCM_E_UNAVAIL;
        break;
    }

    return rv;
}

/*
 * Function:
 *      bcm_port_pfm_set
 * Purpose:
 *      Set current port filtering mode (see port.h)
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      mode - mode for PFM bits (see port.h)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_pfm_set(int unit, bcm_port_t port, int mode)
{
    int                 rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    if (!IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    PORT_LOCK(unit);
    rv = _bcm_port_pfm_set(unit, port, mode);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_pfm_get
 * Purpose:
 *      Return current port filtering mode (see port.h)
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) mode read from StrataSwitch for PFM bits (see port.h)
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      This function not supported on 5670.
 */

int
bcm_esw_port_pfm_get(int unit, bcm_port_t port, int *mode)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    if (!IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    switch (BCM_CHIP_FAMILY(unit)) {

#ifdef BCM_HERCULES_SUPPORT
    case BCM_FAMILY_HERCULES:
        /*
         * Packets marked as multicast prompt a lookup in the
         * MC table in 567x.  There is no notion of missing a multicast
         * address.
         *
         * Alternatively, we can mess with the MC/IPMC table
         * entries to support the different modes.  This is
         * probably not worth the trouble when we look at how
         * 567x is meant to fit into the system.
         */
        *mode = BCM_PORT_PFM_MODEC;
        break;
#endif

#ifdef BCM_XGS3_SWITCH_SUPPORT
    case BCM_FAMILY_FIREBOLT:
    case BCM_FAMILY_EASYRIDER:
    case BCM_FAMILY_BRADLEY:
    case BCM_FAMILY_HUMV:
    case BCM_FAMILY_TRIUMPH:
    case BCM_FAMILY_SCORPION:
    case BCM_FAMILY_CONQUEROR:
    case BCM_FAMILY_TRIUMPH2:
    case BCM_FAMILY_TRIDENT:
        /*
         * PFM is a per VLAN attribute. It is not a per port attribute
         * on XGS3
         */
        *mode = BCM_PORT_PFM_MODEC;
        return BCM_E_UNAVAIL;
#endif

#ifdef BCM_XGS_SWITCH_SUPPORT
    case BCM_FAMILY_TUCANA:
    case BCM_FAMILY_DRACO15:
    case BCM_FAMILY_DRACO:
    case BCM_FAMILY_LYNX:
        if (IS_E_PORT(unit, port)) {
            port_tab_entry_t    pent;
            soc_mem_t mem = SOC_PORT_MEM_TAB(unit, port);

            BCM_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                    SOC_PORT_MOD_OFFSET(unit, port),  &pent));
            *mode = soc_PORT_TABm_field32_get(unit, &pent, PFMf);
        } else if (IS_HG_PORT(unit, port)) {
            *mode = BCM_PORT_PFM_MODEC;  /* Pending final decision */
        } else {
            return BCM_E_PORT;
        }
        break;
#endif

    default:
        return BCM_E_UNAVAIL;
        break;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_untagged_priority_set
 * Purpose:
 *      Main part of bcm_port_untagged_priority_set.
 */

STATIC int
_bcm_port_untagged_priority_set(int unit, bcm_port_t port, int priority)
{
#ifdef BCM_FILTER_SUPPORT
    bcm_filterid_t      f;
    int                 rv;
    bcm_vlan_t          vid;
    pbmp_t              pbmp;
#endif

    if (priority > 7) {
        return BCM_E_PARAM;
    }

#if defined(BCM_XGS_SWITCH_SUPPORT)
    if (soc_feature(unit, soc_feature_remap_ut_prio) && (priority >= 0)) {
        bcm_port_cfg_t pcfg;
        SOC_IF_ERROR_RETURN(
            mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
        pcfg.pc_remap_pri_en = 1;
            pcfg.pc_new_opri = priority;
        SOC_IF_ERROR_RETURN(
            mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg));
        PORT(unit, port).p_ut_prio = pcfg.pc_new_opri;
        return SOC_E_NONE;
    }
#endif /* BCM_XGS_SWITCH_SUPPORT */

    if (priority < 0) {
        return BCM_E_PARAM;
    }

#ifdef BCM_FILTER_SUPPORT
    /*
     * Remap priority by using a filter
     */

    BCM_IF_ERROR_RETURN(bcm_esw_port_untagged_vlan_get(unit, port, &vid));

    /*
     * If a filter currently exists, remove it.
     */

    if (PORT(unit, port).p_ut_filter != 0) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_remove(unit, PORT(unit, port).p_ut_filter));

        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_destroy(unit, PORT(unit, port).p_ut_filter));

        PORT(unit, port).p_ut_filter = 0;
    }

    PORT(unit, port).p_ut_prio = priority;

    if (priority == 0) {
        return BCM_E_NONE;      /* No filter needed because default is 0 */
    }

    /*
     * Create filter for Insert 802.1p Priority matching untagged
     * packets with the correct VLAN ID.
     */

    BCM_IF_ERROR_RETURN(bcm_esw_filter_create(unit, &f));

    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_PORT_ADD(pbmp, port);
    (void) bcm_esw_filter_qualify_ingress(unit, f, pbmp);
    (void) bcm_esw_filter_qualify_data16(unit, f, 12, 0x8100, 0xffff);
    (void) bcm_esw_filter_qualify_data16(unit, f, 14, vid, 0x0fff);
    (void) bcm_esw_filter_action_match(unit, f, bcmActionInsPrio, priority);

    rv = bcm_esw_filter_install(unit, f);

    if (rv < 0) {
        (void) bcm_esw_filter_destroy(unit, f);
    } else {
        PORT(unit, port).p_ut_filter = f;
    }

    return rv;
#else /* !BCM_FILTER_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_FILTER_SUPPORT */
}

/*
 * Function:
 *      bcm_port_untagged_priority_set
 * Purpose:
 *      Set the 802.1P priority for untagged packets coming in on a
 *      port.  This value will be written into the priority field of the
 *      tag that is added at the ingress.
 * Parameters:
 *      unit      - StrataSwitch Unit #.
 *      port      - StrataSwitch port #.
 *      priority  - Priority to be set in 802.1p priority tag, from 0 to 7.
 *                  A negative priority leaves the ingress port priority as
 *                  is, but disables it from overriding ARL-based priorities.
 *                  (on those devices that support ARL-based priority).
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_untagged_priority_set(int unit, bcm_port_t port, int priority)
{
    int         rv;

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        return bcm_tr2_wlan_port_untagged_prio_set(unit, port, priority);
    }
#endif
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = _bcm_port_untagged_priority_set(unit, port, priority);
    PORT_UNLOCK(unit);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_ut_priority_set: u=%d p=%d pri=%d rv=%d\n",
                     unit, port, priority, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_untagged_priority_get
 * Purpose:
 *      Returns priority being assigned to untagged receive packets
 * Parameters:
 *      unit      - StrataSwitch Unit #.
 *      port      - StrataSwitch port #.
 *      priority  - Pointer to an int in which priority value is returned.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */

int
bcm_esw_port_untagged_priority_get(int unit, bcm_port_t port, int *priority)
{
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        return bcm_tr2_wlan_port_untagged_prio_get(unit, port, priority);
    }
#endif
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (priority != NULL) {
        *priority = PORT(unit, port).p_ut_prio;
        SOC_DEBUG_PRINT((DK_PORT,
            "bcm_port_ut_priority_get: u=%d p=%d pri=%d\n",
            unit, port, *priority));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_selective_get
 * Purpose:
 *      Get requested port parameters
 * Parameters:
 *      unit - switch Unit
 *      port - switch port
 *      info - (IN/OUT) port information structure
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The action_mask field of the info argument is used as an input
 */

int
bcm_esw_port_selective_get(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    int                 r;
    uint32              mask;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENCAP_MASK) {
        r = bcm_esw_port_encap_get(unit, port, &info->encap_mode);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_encap_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_ENABLE_MASK) {
        r = bcm_esw_port_enable_get(unit, port, &info->enable);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_enable_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LINKSTAT_MASK) {
        r = bcm_esw_port_link_status_get(unit, port, &info->linkstatus);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_link_status_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK) {
        r = bcm_esw_port_autoneg_get(unit, port, &info->autoneg);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_autoneg_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK) {
        r = bcm_esw_port_ability_advert_get(unit, port,
                                            &info->local_ability);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_ability_advert_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
        r = soc_port_ability_to_mode(&info->local_ability,
                                     &info->local_advert);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_REMOTE_ADVERT_MASK) {

        if ((r = bcm_esw_port_ability_remote_get(unit, port,
                                                 &info->remote_ability)) < 0) {
            info->remote_advert = 0;
            info->remote_advert_valid = FALSE;
        } else {
            r = soc_port_ability_to_mode(&info->remote_ability,
                                         &info->remote_advert);
            BCM_IF_ERROR_RETURN(r);
            info->remote_advert_valid = TRUE;
        }
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK) {
        if ((r = bcm_esw_port_speed_get(unit, port, &info->speed)) < 0) {
            if (r != BCM_E_BUSY) {
                SOC_DEBUG_PRINT((DK_VERBOSE,
                                 "bcm_port_speed_get failed: %s\n", bcm_errmsg(r)));
                return(r);
            } else {
                info->speed = 0;
            }
        }
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK) {
        if ((r = bcm_esw_port_duplex_get(unit, port, &info->duplex)) < 0) {
            if (r != BCM_E_BUSY) {
                SOC_DEBUG_PRINT((DK_VERBOSE,
                                 "bcm_port_duplex_get failed: %s\n", bcm_errmsg(r)));
                return r;
            } else {
                info->duplex = 0;
            }
        }
    }

    /* get both if either mask bit set */
    if (mask & (BCM_PORT_ATTR_PAUSE_TX_MASK |
                BCM_PORT_ATTR_PAUSE_RX_MASK)) {
        r = bcm_esw_port_pause_get(unit, port,
                                   &info->pause_tx, &info->pause_rx);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_pause_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
        r = bcm_esw_port_pause_addr_get(unit, port, info->pause_mac);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_pause_addr_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LINKSCAN_MASK) {
        r = bcm_esw_port_linkscan_get(unit, port, &info->linkscan);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_linkscan_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK) {
        r = bcm_esw_port_learn_get(unit, port, &info->learn);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_learn_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_DISCARD_MASK) {
        r = bcm_esw_port_discard_get(unit, port, &info->discard);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_discard_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_VLANFILTER_MASK) {
        r = bcm_esw_port_vlan_member_get(unit, port, &info->vlanfilter);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_esw_port_vlan_member_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_PRI_MASK) {
        r = bcm_esw_port_untagged_priority_get(unit, port,
                                               &info->untagged_priority);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_untagged_priority_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_VLAN_MASK) {
        r = bcm_esw_port_untagged_vlan_get(unit, port,
                                           &info->untagged_vlan);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_untagged_vlan_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_STP_STATE_MASK) {
        r = bcm_esw_port_stp_get(unit, port, &info->stp_state);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_stp_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PFM_MASK) {
        r = bcm_esw_port_pfm_get(unit, port, &info->pfm);
        if (r != BCM_E_UNAVAIL) {
            SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                         "bcm_port_pfm_get failed: %s\n", bcm_errmsg(r)));
        }
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK) {
        r = bcm_esw_port_loopback_get(unit, port, &info->loopback);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_loopback_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PHY_MASTER_MASK) {
        r = bcm_esw_port_master_get(unit, port, &info->phy_master);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_master_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK) {
        r = bcm_esw_port_interface_get(unit, port, &info->interface);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_interface_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_MCAST_MASK) {
        r = bcm_esw_rate_mcast_get(unit, &info->mcast_limit,
                                   &info->mcast_limit_enable, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_rate_mcast_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_BCAST_MASK) {
        r = bcm_esw_rate_bcast_get(unit, &info->bcast_limit,
                                   &info->bcast_limit_enable, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_rate_bcast_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_DLFBC_MASK) {
        r = bcm_esw_rate_dlfbc_get(unit, &info->dlfbc_limit,
                                   &info->dlfbc_limit_enable, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_rate_dlfbc_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_SPEED_MAX_MASK) {
        r = bcm_esw_port_speed_max(unit, port, &info->speed_max);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_speed_max failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_ABILITY_MASK) {
        r = bcm_esw_port_ability_local_get(unit, port, &info->port_ability);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_ability_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
        r = soc_port_ability_to_mode(&info->port_ability,
                                     &info->ability);
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        r = bcm_esw_port_frame_max_get(unit, port, &info->frame_max);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_frame_max_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MDIX_MASK) {
        r = bcm_esw_port_mdix_get(unit, port, &info->mdix);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_mdix_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MDIX_STATUS_MASK) {
        r = bcm_esw_port_mdix_status_get(unit, port, &info->mdix_status);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_mdix_status_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MEDIUM_MASK) {
        r = bcm_esw_port_medium_get(unit, port, &info->medium);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_medium_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_FAULT_MASK) {
        r = bcm_esw_port_fault_get(unit, port, &info->fault);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_fault_get failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_selective_set
 * Purpose:
 *      Set requested port parameters
 * Parameters:
 *      unit - switch unit
 *      port - switch port
 *      info - port information structure
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Does not set spanning tree state.
 */

int
bcm_esw_port_selective_set(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    int                 r;
    uint32              mask;
    int                 flags = 0;

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_selective_set: u=%d p=%d\n", unit, port));

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENCAP_MASK) {
        r = bcm_esw_port_encap_set(unit, port, info->encap_mode);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_encap_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_ENABLE_MASK) {
        r = bcm_esw_port_enable_set(unit, port, info->enable);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_enable_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
        r = bcm_esw_port_pause_addr_set(unit, port, info->pause_mac);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_pause_addr_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_INTERFACE_MASK) {
        r = bcm_esw_port_interface_set(unit, port, info->interface);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_interface_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PHY_MASTER_MASK) {
        r = bcm_esw_port_master_set(unit, port, info->phy_master);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_master_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LINKSCAN_MASK) {
        r = bcm_esw_port_linkscan_set(unit, port, info->linkscan);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_linkscan_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LEARN_MASK) {
        r = bcm_esw_port_learn_set(unit, port, info->learn);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_learn_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_DISCARD_MASK) {
        r = bcm_esw_port_discard_set(unit, port, info->discard);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_discard_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_VLANFILTER_MASK) {
        r = bcm_esw_port_vlan_member_set(unit, port, info->vlanfilter);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_esw_port_vlan_member_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_PRI_MASK) {
        r = bcm_esw_port_untagged_priority_set(unit, port,
                                               info->untagged_priority);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_untagged_priority_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_UNTAG_VLAN_MASK) {
        r = bcm_esw_port_untagged_vlan_set(unit, port, info->untagged_vlan);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_untagged_vlan_set (%d) failed: %s\n",
                                     info->untagged_vlan, bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_PFM_MASK) {
        r = bcm_esw_port_pfm_set(unit, port, info->pfm);
        if (r != BCM_E_UNAVAIL) {
            SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                         "bcm_port_pfm_set failed: %s\n", bcm_errmsg(r)));
        }
        BCM_IF_ERROR_NOT_UNAVAIL_RETURN(r);
    }

    /*
     * Set loopback mode before setting the speed/duplex, since it may
     * affect the allowable values for speed/duplex.
     */

    if (mask & BCM_PORT_ATTR_LOOPBACK_MASK) {
        r = bcm_esw_port_loopback_set(unit, port, info->loopback);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_loopback_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK) {
        if (info->action_mask2 & BCM_PORT_ATTR2_PORT_ABILITY) {
            r = bcm_esw_port_ability_advert_set(unit, port,
                                                &(info->local_ability));
            SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                         "bcm_port_ability_advert_set failed: %s\n",
                                         bcm_errmsg(r)));
            BCM_IF_ERROR_RETURN(r);
        } else {
            r = bcm_esw_port_advert_set(unit, port, info->local_advert);
            SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                         "bcm_port_advert_set failed: (0x%x): %s\n",
                                         info->local_advert, bcm_errmsg(r)));
            BCM_IF_ERROR_RETURN(r);
        }
    }

    if (mask & BCM_PORT_ATTR_AUTONEG_MASK) {
        r = bcm_esw_port_autoneg_set(unit, port, info->autoneg);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_autoneg_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_SPEED_MASK) {
        r = bcm_esw_port_speed_set(unit, port, info->speed);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_speed_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_DUPLEX_MASK) {
        r = bcm_esw_port_duplex_set(unit, port, info->duplex);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_duplex_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & (BCM_PORT_ATTR_PAUSE_TX_MASK |
                BCM_PORT_ATTR_PAUSE_RX_MASK)) {
        int     tpause, rpause;

        tpause = rpause = -1;
        if (mask & BCM_PORT_ATTR_PAUSE_TX_MASK) {
            tpause = info->pause_tx;
        }
        if (mask & BCM_PORT_ATTR_PAUSE_RX_MASK) {
            rpause = info->pause_rx;
        }
        r = bcm_esw_port_pause_set(unit, port, tpause, rpause);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_pause_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_MCAST_MASK) {
        flags = (info->mcast_limit_enable) ? BCM_RATE_MCAST : 0;
        r = bcm_esw_rate_mcast_set(unit, info->mcast_limit, flags, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_rate_mcast_port_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_BCAST_MASK) {
        flags = (info->bcast_limit_enable) ? BCM_RATE_BCAST : 0;
        r = bcm_esw_rate_bcast_set(unit, info->bcast_limit, flags, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_rate_bcast_port_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_RATE_DLFBC_MASK) {
        flags = (info->dlfbc_limit_enable) ? BCM_RATE_DLF : 0;
        r = bcm_esw_rate_dlfbc_set(unit, info->dlfbc_limit, flags, port);
        if (r == BCM_E_UNAVAIL) {
            r = BCM_E_NONE;     /* Ignore if not supported on chip */
        }
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_rate_dlfbcast_port_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_STP_STATE_MASK) {
        r = bcm_esw_port_stp_set(unit, port, info->stp_state);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_stp_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        r = bcm_esw_port_frame_max_set(unit, port, info->frame_max);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_frame_max_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    if (mask & BCM_PORT_ATTR_MDIX_MASK) {
        r = bcm_esw_port_mdix_set(unit, port, info->mdix);
        SOC_IF_ERROR_DEBUG_PRINT(r, (DK_VERBOSE,
                                     "bcm_port_mdix_set failed: %s\n", bcm_errmsg(r)));
        BCM_IF_ERROR_RETURN(r);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_info_get
 * Purpose:
 *      Get all information on the port
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      info - Pointer to structure in which to save values
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_info_get(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    bcm_port_info_t_init(info);

    if (IS_ST_PORT(unit, port)) {
        info->action_mask = BCM_PORT_HERC_ATTRS;
        if (SOC_IS_XGS_SWITCH(unit)) {
            info->action_mask |= BCM_PORT_ATTR_STP_STATE_MASK |
                                 BCM_PORT_ATTR_DISCARD_MASK |
                                 BCM_PORT_ATTR_LEARN_MASK;
        }
    } else {
        info->action_mask = BCM_PORT_ATTR_ALL_MASK;
    }

    return bcm_esw_port_selective_get(unit, port, info);
}

/*
 * Function:
 *      bcm_port_info_set
 * Purpose:
 *      Set all information on the port
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      info - Pointer to structure in which to save values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Checks if AN is on, and if so, clears the
 *      proper bits in the action mask.
 */

int
bcm_esw_port_info_set(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (IS_ST_PORT(unit, port)) {
        info->action_mask = BCM_PORT_HERC_ATTRS;
        if (SOC_IS_XGS_SWITCH(unit)) {
            info->action_mask |= BCM_PORT_ATTR_STP_STATE_MASK |
                                 BCM_PORT_ATTR_DISCARD_MASK |
                                 BCM_PORT_ATTR_LEARN_MASK;
        }
    } else {
        info->action_mask = BCM_PORT_ATTR_ALL_MASK;
    }

    /* If autoneg is set, remove those attributes controlled by it */
    if (info->autoneg) {
        info->action_mask &= ~BCM_PORT_AN_ATTRS;
    }

    return bcm_esw_port_selective_set(unit, port, info);
}

/*
 * Function:
 *      bcm_port_info_save
 * Purpose:
 *      Save the current settings of a port
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      info - Pointer to structure in which to save values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The action_mask will be adjusted so that the
 *      proper values will be set when a restore is made.
 *      This mask should not be altered between these calls.
 */

int
bcm_esw_port_info_save(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (IS_ST_PORT(unit, port)) {
        info->action_mask = BCM_PORT_HERC_ATTRS;
        if (SOC_IS_XGS_SWITCH(unit)) {
            info->action_mask |= BCM_PORT_ATTR_STP_STATE_MASK;
        }
#ifdef BCM_GXPORT_SUPPORT
        if (IS_GX_PORT(unit, port)) {
            info->action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
        }
#endif
    } else {
        info->action_mask = BCM_PORT_ATTR_ALL_MASK;
    }
    info->action_mask2 = BCM_PORT_ATTR2_PORT_ABILITY;

    BCM_IF_ERROR_RETURN(bcm_esw_port_selective_get(unit, port, info));

    if (info->autoneg) {
        info->action_mask &= ~BCM_PORT_AN_ATTRS;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_info_restore
 * Purpose:
 *      Restore port settings saved by info_save
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      info - Pointer to structure with info from port_info_save
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      bcm_port_info_save has done all the work.
 *      We just call port_selective_set.
 */

int
bcm_esw_port_info_restore(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    return bcm_esw_port_selective_set(unit, port, info);
}

/*
 * Function:
 *      bcm_port_phy_drv_name_get
 * Purpose:
 *      Return the name of the PHY driver being used on a port.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 * Returns:
 *      Pointer to static string
 */
int
bcm_esw_port_phy_drv_name_get(int unit, bcm_port_t port, char *name, int len)
{
    int str_len;

    if (bcm_port_info[unit] == NULL) {
        str_len = sal_strlen("driver not initialized");
        if (str_len <= len) {
            sal_strcpy(name, "driver not initialized");
        }
        return BCM_E_INIT;
    }
    if (!SOC_PORT_VALID(unit, port)) {
        str_len = sal_strlen("invalid port");
        if (str_len <= len) {
            sal_strcpy(name, "invalid port");
        }
        return BCM_E_PORT;
    }
    return (soc_phyctrl_drv_name_get(unit, port, name, len)); 
}

/*
 * Function:
 *      _bcm_port_encap_xport_set
 * Purpose:
 *      Convert 10G Ether port to Higig port, or reverse
 * Notes;
 *      Must be called with PORT_LOCK held.
 */

STATIC int
_bcm_port_encap_xport_set(int unit, bcm_port_t port, int mode)
{
#ifdef BCM_FIREBOLT_SUPPORT
    bcm_stg_t        stg;
    int              to_higig;
    soc_field_t      port_type_field;
    soc_reg_t        egr_port_reg;
    soc_port_ability_t ability;
    soc_port_mode_t  non_ieee_speed;
    int              an, an_done, force_speed, port_type;

    to_higig = (mode != BCM_PORT_ENCAP_IEEE);
    port_type_field = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                       SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
                       SOC_IS_HURRICANE(unit) || SOC_IS_TD_TT(unit) ||
                       SOC_IS_KATANA(unit)) ? 
        PORT_TYPEf : HIGIG_PACKETf;
    egr_port_reg = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit)) ? EGR_PORT_64r : EGR_PORTr;

    SOC_IF_ERROR_RETURN
        (MAC_ENCAP_SET(PORT(unit, port).p_mac, unit, port, mode));

    if (!to_higig) {
        SOC_IF_ERROR_RETURN
            (MAC_INTERFACE_SET(PORT(unit, port).p_mac,
                               unit, port, SOC_PORT_IF_XGMII));
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_port_ability_local_get(unit, port, &ability));

    non_ieee_speed = ability.speed_full_duplex &
        ~(SOC_PA_SPEED_10GB | SOC_PA_SPEED_2500MB | SOC_PA_SPEED_1000MB |
          SOC_PA_SPEED_100MB | SOC_PA_SPEED_10MB);

    BCM_IF_ERROR_RETURN
        (bcm_esw_port_ability_advert_get(unit, port, &ability));
    if (to_higig) {
        ability.speed_full_duplex |= non_ieee_speed;
        ability.pause &= ~(SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM);
        force_speed = SOC_CONTROL(unit)->info.port_speed_max[port];
    } else {
        ability.speed_full_duplex &= ~non_ieee_speed;
        force_speed = 10000;
    }
    BCM_IF_ERROR_RETURN
        (bcm_esw_port_ability_advert_set(unit, port, &ability));

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_auto_negotiate_get(unit, port, &an, &an_done));
    /* Some mac driver re-init phy while executing MAC_ENCAP_SET, in that case
     * autoneg is probably always true here */
    if (an) {
        SOC_IF_ERROR_RETURN(bcm_esw_port_autoneg_set(unit, port, TRUE));
    } else {
        SOC_IF_ERROR_RETURN(soc_phyctrl_speed_set(unit, port, force_speed));
    }

    /* Now we propagate the changes */
    port_type = to_higig ? 1 : 0;
    SOC_IF_ERROR_RETURN
        (soc_mem_field32_modify(unit, PORT_TABm, port, port_type_field,
                                port_type));
    if (SOC_MEM_IS_VALID(unit, EGR_PORTm)) {
        SOC_IF_ERROR_RETURN
            (soc_mem_field32_modify(unit, EGR_PORTm, port, port_type_field,
                                    port_type));
    } else {
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, egr_port_reg, port, port_type_field,
                                    port_type));
    }

    if (SOC_MEM_IS_VALID(unit, EGR_ING_PORTm)) {
        if (!IS_CPU_PORT(unit, port) && !IS_LB_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN
                (soc_mem_field32_modify(unit, EGR_ING_PORTm, port, PORT_TYPEf,
                                        mode == BCM_PORT_ENCAP_IEEE ? 0 : 1));
        }
    }

    
    if (SOC_MEM_IS_VALID(unit, ICONTROL_OPCODE_BITMAPm)) {
        icontrol_opcode_bitmap_entry_t entry;
        soc_pbmp_t pbmp;

        SOC_IF_ERROR_RETURN
            (READ_ICONTROL_OPCODE_BITMAPm(unit, MEM_BLOCK_ANY, port, &entry));
        SOC_PBMP_CLEAR(pbmp);
        if (to_higig) {
            SOC_PBMP_PORT_SET(pbmp, CMIC_PORT(unit));
        }
        soc_mem_pbmp_field_set(unit, ICONTROL_OPCODE_BITMAPm, &entry,
                               BITMAPf, &pbmp);
        SOC_IF_ERROR_RETURN
            (WRITE_ICONTROL_OPCODE_BITMAPm(unit, MEM_BLOCK_ANY, port, &entry));
    } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        uint64 cpu_pbm64;
        uint64 rval64;      /* Current 64 bit register data.  */
        COMPILER_64_ZERO(cpu_pbm64);
        COMPILER_64_SET(cpu_pbm64, 0, to_higig ? 1 : 0);
        SOC_IF_ERROR_RETURN(READ_ICONTROL_OPCODE_BITMAP_64r(unit, port,
                                                            &rval64));
        soc_reg64_field_set(unit, ICONTROL_OPCODE_BITMAP_64r, &rval64,
                            BITMAPf, cpu_pbm64);
        SOC_IF_ERROR_RETURN(WRITE_ICONTROL_OPCODE_BITMAP_64r(unit, port,
                                                             rval64));
    } else {

        /* Set HG ingress CPU Opcode map to the CPU */
        int pbm_len;
        uint32 cpu_pbm = 0;

        if (to_higig) {
            if (SOC_IS_TR_VL(unit)) {
                soc_xgs3_port_to_higig_bitmap(unit, CMIC_PORT(unit),
                                              &cpu_pbm);
            } else if (CMIC_PORT(unit)) {
                pbm_len = soc_reg_field_length(unit, ICONTROL_OPCODE_BITMAPr,
                                               BITMAPf);
                cpu_pbm = 1 << (pbm_len - 1);
            } else {
                cpu_pbm = 1;
            }
        } /* else, cpu_pbm = 0 */

        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, ICONTROL_OPCODE_BITMAPr,
                                    port, BITMAPf, cpu_pbm));
    }

    if (to_higig) {
        /* HG ports to forwarding */
        BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, port,
                                                 BCM_STG_STP_FORWARD));
    }  

    /* Clear mirror enable settings */
    BCM_IF_ERROR_RETURN(bcm_esw_mirror_port_set(unit, port, -1, -1, 0));

    /* Set untagged state in default VLAN properly */
    BCM_IF_ERROR_RETURN(_bcm_esw_vlan_untag_update(unit, port, to_higig));

    /* Resolve STG 0 */
    BCM_IF_ERROR_RETURN(bcm_esw_stg_default_get(unit, &stg));
    BCM_IF_ERROR_RETURN
        (bcm_esw_stg_stp_set(unit, 0, port,
                             to_higig ? BCM_STG_STP_FORWARD : BCM_STG_STP_DISABLE));

#ifdef BCM_TRX_SUPPORT
    /* Reset the vlan default action */
    if (SOC_IS_TRX(unit) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_vlan_action_set_t action;

        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_egress_default_action_get(unit,
                                                         port, &action));
        /* Backward compatible defaults */
        if (to_higig) {
            action.ot_outer = bcmVlanActionDelete;
            action.dt_outer = bcmVlanActionDelete;
        } else {
            action.ot_outer = bcmVlanActionNone;
            action.dt_outer = bcmVlanActionNone;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_egress_default_action_set(unit,
                                                         port, &action));
    }
#endif

#ifdef INCLUDE_L3
    BCM_IF_ERROR_RETURN
        (bcm_esw_ipmc_egress_port_set(unit, port, to_higig ? _soc_mac_all_ones :
                                      _soc_mac_all_zeroes, 0, 0, 0));
#endif /* INCLUDE_L3 */

    return BCM_E_NONE;
#else
    return BCM_E_PARAM;
#endif
}

/*
 * Function:
 *      _bcm_port_encap_stport_set
 * Purpose:
 *      Convert 2.5G Higig 2 port to Ether port, or reverse
 */
STATIC int
_bcm_port_encap_stport_set(int unit, bcm_port_t port, int mode)
{
#if defined(BCM_RAVEN_SUPPORT) || defined (BCM_TRIUMPH2_SUPPORT)
    port_tab_entry_t ptab;
    uint32           rval;
    bcm_stg_t        stg;
    int              to_higig = 0;
    soc_info_t       *si = &SOC_INFO(unit);
    soc_field_t      hg_en[6] = {-1, HGIG2_EN_S0f, HGIG2_EN_S1f, -1, 
                                 HGIG2_EN_S3f, HGIG2_EN_S4f};
    soc_field_t      port_type;
    soc_reg_t        egr_port;
    
    if (mode == BCM_PORT_ENCAP_HIGIG2 || mode == BCM_PORT_ENCAP_HIGIG2_LITE) {
        to_higig = TRUE;
    }

    /* This check is not correct for devices supporting embedded higig */
    if (!soc_feature(unit, soc_feature_embedded_higig)) {
        if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
            if ((port != 26) && (port != 27) && (port != 28) && (port != 29)) {
                return BCM_E_PORT;
            }
            SOC_IF_ERROR_RETURN
                (soc_reg_field32_modify(unit, EHG_RX_CONTROLr, port, MODEf, 
                                        to_higig ? 2 : 0));
        } else {
            if ((port != 1) && (port != 2) && (port != 4) && (port != 5)) {
                return BCM_E_PORT;
            }
        }
    }

    sal_memset(&ptab, 0, sizeof(ptab));

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm,
                                     MEM_BLOCK_ANY, port, &ptab));
    port_type = (SOC_IS_ENDURO(unit) || SOC_IS_TRIUMPH2(unit) || 
                 SOC_IS_APOLLO(unit) || SOC_IS_VALKYRIE2(unit)  ||
                 SOC_IS_HURRICANE(unit)) ? 
                 PORT_TYPEf : HIGIG_PACKETf;

    soc_PORT_TABm_field32_set(unit, &ptab, port_type, to_higig);
    SOC_IF_ERROR_RETURN(soc_mem_write(unit, PORT_TABm,
                                      MEM_BLOCK_ALL, port, &ptab));
    if (SOC_REG_IS_VALID(unit, EGR_PORT_64r)) {
        egr_port = EGR_PORT_64r;
    } else {
        egr_port = EGR_PORTr;
    }
    SOC_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, egr_port, port, port_type, to_higig));


    if (SOC_IS_RAVEN(unit)) {
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, GPORT_CONFIGr, port,
                                    hg_en[port], to_higig));
    }

    if (to_higig) {
        BCM_IF_ERROR_RETURN(bcm_esw_port_pause_set(unit, port, 0, 0));

        SOC_PBMP_PORT_ADD(si->st.bitmap, port);
        SOC_PBMP_PORT_ADD(si->hl.bitmap, port);  
        SOC_PBMP_PORT_REMOVE(si->ether.bitmap, port);

        BCM_IF_ERROR_RETURN(bcm_esw_port_frame_max_set(unit, port, 0x3fe8));
    } else {
        SOC_PBMP_PORT_REMOVE(si->st.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->hl.bitmap, port);
        SOC_PBMP_PORT_ADD(si->ether.bitmap, port);

        /* Set pause and frame - other abilities depend on the board and
         * the application needs to call other APIs appropriately */
        BCM_IF_ERROR_RETURN(bcm_esw_port_pause_set(unit, port, 1, 1));
        BCM_IF_ERROR_RETURN(bcm_esw_port_frame_max_set(unit, port, 1518));
    }

    /* Set HG ingress CPU Opcode map to the CPU */
    if (SOC_REG_IS_VALID(unit, ICONTROL_OPCODE_BITMAP_64r)) {
        uint64 cpu_pbm64;
        uint64 rval64;      /* Current 64 bit register data.  */
        COMPILER_64_ZERO(cpu_pbm64);
        COMPILER_64_SET(cpu_pbm64, 0, to_higig ? 1 : 0);
        SOC_IF_ERROR_RETURN(
            READ_ICONTROL_OPCODE_BITMAP_64r(unit, port,&rval64));

        soc_reg64_field_set(unit, ICONTROL_OPCODE_BITMAP_64r, &rval64,
                            BITMAPf, cpu_pbm64);

        SOC_IF_ERROR_RETURN(
            WRITE_ICONTROL_OPCODE_BITMAP_64r(unit, port, rval64));
    } else {
        uint32 cpu_pbm;

        if(SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
            if(to_higig) {
                SOC_IF_ERROR_RETURN
                        (soc_xgs3_port_to_higig_bitmap(unit, CMIC_PORT(unit),
                                           &cpu_pbm));
            } else {
                cpu_pbm = 0;
            }
        } else {
            cpu_pbm = to_higig ? 1 : 0;
        }
        SOC_IF_ERROR_RETURN(
            READ_ICONTROL_OPCODE_BITMAPr(unit, port,&rval));

        soc_reg_field_set(unit, ICONTROL_OPCODE_BITMAPr, 
                          &rval, BITMAPf, cpu_pbm);

        SOC_IF_ERROR_RETURN(
            WRITE_ICONTROL_OPCODE_BITMAPr(unit, port, rval));
    }

    /* HG ports to forwarding */
    BCM_IF_ERROR_RETURN(
        bcm_esw_port_stp_set(unit, port,
                to_higig ? BCM_STG_STP_FORWARD : BCM_STG_STP_DISABLE));

    /* Set untagged state in default VLAN properly */
    BCM_IF_ERROR_RETURN(_bcm_esw_vlan_untag_update(unit, port, to_higig));

    /* Resolve STG 0 */
    BCM_IF_ERROR_RETURN(bcm_esw_stg_default_get(unit, &stg));
    BCM_IF_ERROR_RETURN(
        bcm_esw_stg_stp_set(unit, stg, port,
                to_higig ? BCM_STG_STP_FORWARD : BCM_STG_STP_DISABLE));

#ifdef BCM_ENDURO_SUPPORT
    /* Reset the vlan default action */
    if ((SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) && soc_feature(unit, soc_feature_vlan_action)) {
        bcm_vlan_action_set_t action;

        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_egress_default_action_get(unit,
                                                         port, &action));
        if (to_higig) {
            action.ot_outer = bcmVlanActionDelete;
            action.dt_outer = bcmVlanActionDelete;
        } else {
            action.ot_outer = bcmVlanActionNone;
            action.dt_outer = bcmVlanActionNone;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_egress_default_action_set(unit,
                                                         port, &action));
    }
#endif /* BCM_ENDURO_SUPPORT */

    return BCM_E_NONE;
#else
    return BCM_E_PARAM;
#endif
}

/*
 * Function:
 *      _bcm_port_ehg_header_mem_get
 * Purpose:
 *      Calculates the correct memories for a given port
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      tx_mem          - (OUT) tx memory for EHG heade
 *      rx_mem          - (OUT) rx memory for EHG heade
 *      rx_mask_mem     - (OUT) rx mask memory for EHG heade
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_port_ehg_header_mem_get(int unit, bcm_port_t port, soc_mem_t *tx_mem, 
                             soc_mem_t *rx_mem, soc_mem_t *rx_mask_mem)
{
    int phy_port;

    if (NULL == tx_mem || NULL == rx_mem || NULL == rx_mask_mem) {
        return BCM_E_PARAM;
    }

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    } else {
        phy_port = port;
    }
    switch (SOC_PORT_BLOCK_TYPE(unit, phy_port)) {
        case SOC_BLK_GXPORT: 
            *tx_mem = XPORT_EHG_TX_TUNNEL_DATAm;
            *rx_mem = XPORT_EHG_RX_TUNNEL_DATAm;
            *rx_mask_mem = XPORT_EHG_RX_TUNNEL_MASKm;
            break;
        case SOC_BLK_GPORT:
            *tx_mem = GPORT_EHG_TX_TUNNEL_DATAm;
            *rx_mem = GPORT_EHG_RX_TUNNEL_DATAm;
            *rx_mask_mem = GPORT_EHG_RX_TUNNEL_MASKm;
            break; 
        case SOC_BLK_SPORT:
            *tx_mem = SPORT_EHG_TX_TUNNEL_DATAm;
            *rx_mem = SPORT_EHG_RX_TUNNEL_DATAm;
            *rx_mask_mem = SPORT_EHG_RX_TUNNEL_MASKm;
            break; 
        case SOC_BLK_XQPORT:
            *tx_mem = XQPORT_EHG_TX_TUNNEL_DATAm;
            *rx_mem = XQPORT_EHG_RX_TUNNEL_DATAm;
            *rx_mask_mem = XQPORT_EHG_RX_TUNNEL_MASKm;
            break; 
        default: 
            *tx_mem = *rx_mem = *rx_mask_mem = INVALIDm;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_ehg_header_read
 * Purpose:
 *      Reads EHG header from the HW tables
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      buffer          - (OUT) Pointer to the buffer to contain header
 * Returns:
 *      BCM_E_XXX
 */

STATIC int 
_bcm_port_ehg_header_read(int unit, bcm_port_t port, uint32 *buffer)
{
    soc_mem_t   tx_mem, rx_mem, rx_mask_mem;
    int         tbl_idx_start, tbl_idx_end, idx;
    xqport_ehg_rx_tunnel_data_entry_t      entry;
    int         phy_port;
    
    BCM_IF_ERROR_RETURN
        (_bcm_port_ehg_header_mem_get(unit, port, &tx_mem, &rx_mem, 
                                      &rx_mask_mem));

    if (INVALIDm == tx_mem) {
        return (BCM_E_PORT);
    }

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    } else {
        phy_port = port;
    }

    tbl_idx_start = SOC_PORT_BINDEX(unit, phy_port) * 4; 
    tbl_idx_end = tbl_idx_start + 3;

    for (idx = 0 ; idx <= (tbl_idx_end - tbl_idx_start); idx ++) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, tx_mem, SOC_PORT_BLOCK(unit, phy_port), 
                          (idx + tbl_idx_start), (void *)&entry));
        soc_mem_field_get(unit, tx_mem, (void *)&entry, TUNNEL_DATAf, 
                          (buffer + (idx * 4)));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_port_ehg_header_write
 * Purpose:
 *      Writes EHG header into the HW tables
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      tx_buff         - (IN) Pointer to the tx header
 *      rx_buff         - (IN) Pointer to the rx header
 *      len             - (IN) Header length
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_port_ehg_header_write(int unit, bcm_port_t port, uint32 *tx_buf, 
                           uint32 *rx_buf, int len)
{
    soc_mem_t   tx_mem, rx_mem, rx_mask_mem;
    int         tbl_idx_start, tbl_idx_end, idx;
    xqport_ehg_rx_tunnel_data_entry_t data_entry;
    xqport_ehg_rx_tunnel_mask_entry_t mask_entry;
    int         phy_port;

    BCM_IF_ERROR_RETURN
        (_bcm_port_ehg_header_mem_get(unit, port, &tx_mem, &rx_mem, 
                                      &rx_mask_mem));

    if (INVALIDm == tx_mem) {
        return (BCM_E_PORT);
    }

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    } else {
        phy_port = port;
    }

    tbl_idx_end = (SOC_PORT_BINDEX(unit, phy_port) + 1) * 4 - 1;
    tbl_idx_start = tbl_idx_end - (len / 4) + 1;    

    for (idx = 0; idx <= (tbl_idx_end - tbl_idx_start); idx ++) {
        /* Reset hw buffer. */
        sal_memset(&data_entry, 0, sizeof(xqport_ehg_rx_tunnel_data_entry_t));
        soc_mem_field_set(unit, tx_mem, (uint32 *)&data_entry, TUNNEL_DATAf, 
                          (tx_buf + (idx * 4)));
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, tx_mem, SOC_PORT_BLOCK(unit, phy_port), 
                           (idx + tbl_idx_start), &data_entry));

        /* Reset hw buffer. */
        sal_memset(&data_entry, 0, sizeof(xqport_ehg_rx_tunnel_data_entry_t));
        soc_mem_field_set(unit, rx_mem, (uint32 *)&data_entry, TUNNEL_DATAf, 
                          (rx_buf + (idx * 4)));
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, rx_mem, SOC_PORT_BLOCK(unit, phy_port), 
                           (idx + tbl_idx_start), &data_entry));

        /* Reset hw buffer. */
       sal_memset(&mask_entry, 0xFF, sizeof(xqport_ehg_rx_tunnel_mask_entry_t));
        BCM_IF_ERROR_RETURN 
            (soc_mem_write(unit, rx_mask_mem, SOC_PORT_BLOCK(unit, phy_port), 
                           (idx + tbl_idx_start), &mask_entry));
        
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_port_ehg_setup_txrx_ethernet_hdr
 * Purpose:
 *      Prepares SRC and DEST MAC addresses into rx and tx buffers on a given index
 * Parameters:
 *      tx_buffer       -(IN/OUT) - tx buffer to fill
 *      rx_buffer       -(IN/OUT) - rx buffer to fill
 *      index           -(IN/OUT) - index to the right location in buffer
 *      encap_config    - (IN) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */

STATIC void 
_bcm_port_ehg_setup_txrx_ethernet_hdr(uint32 *tx_buffer, uint32 *rx_buffer,
                                      int *index, 
                                      bcm_port_encap_config_t *encap_config)
{
    int idx = *index;

    /* Destination mac address. */
    tx_buffer[idx] = (((uint32)(encap_config->dst_mac)[0]) << 16 | \
                      ((uint32)(encap_config->dst_mac)[1]) << 8  | \
                      ((uint32)(encap_config->dst_mac)[2]));
    /* TX dest MAC is RX src MAC */
    rx_buffer[idx] = (((uint32)(encap_config->src_mac)[0]) << 16 | \
                      ((uint32)(encap_config->src_mac)[1]) << 8  | \
                      ((uint32)(encap_config->src_mac)[2]));
    idx--;

    tx_buffer[idx] = (((uint32)(encap_config->dst_mac)[3]) << 24 | \
                      ((uint32)(encap_config->dst_mac)[4]) << 16 | \
                      ((uint32)(encap_config->dst_mac)[5]) << 8  | \
                      ((uint32)(encap_config->src_mac)[0])); 
    /* TX dest MAC is RX src MAC */
    rx_buffer[idx] = (((uint32)(encap_config->src_mac)[3]) << 24 | \
                      ((uint32)(encap_config->src_mac)[4]) << 16 | \
                      ((uint32)(encap_config->src_mac)[5]) << 8  | \
                      ((uint32)(encap_config->dst_mac)[0])); 
    idx--;
    /* Source mac address. */
    tx_buffer[idx] = (((uint32)(encap_config->src_mac)[1]) << 24 | \
                      ((uint32)(encap_config->src_mac)[2]) << 16 | \
                      ((uint32)(encap_config->src_mac)[3]) << 8  | \
                      ((uint32)(encap_config->src_mac)[4])); 
    /* TX src MAC is RX dest MAC */
    rx_buffer[idx] = (((uint32)(encap_config->dst_mac)[1]) << 24 | \
                      ((uint32)(encap_config->dst_mac)[2]) << 16 | \
                      ((uint32)(encap_config->dst_mac)[3]) << 8  | \
                      ((uint32)(encap_config->dst_mac)[4])); 
    idx--;
    tx_buffer[idx] = (((uint32)(encap_config->src_mac)[5]) << 24);
    rx_buffer[idx] = (((uint32)(encap_config->dst_mac)[5]) << 24);

    /* Set tpid & vlan id. */
    if (BCM_VLAN_VALID(encap_config->vlan)) {
        /* Tpid. */
        tx_buffer[idx] |= (((uint32)(encap_config->tpid >> 8)) << 16 | \
                           ((uint32)(encap_config->tpid & 0xff)) << 8);
        rx_buffer[idx] |= (((uint32)(encap_config->tpid >> 8)) << 16 | \
                           ((uint32)(encap_config->tpid & 0xff)) << 8);

        /* Priority,  Cfi, Vlan id. */
        tx_buffer[idx] |= (((uint32)(encap_config->vlan >> 8)));
        rx_buffer[idx] |= (((uint32)(encap_config->vlan >> 8)));
        idx--;
        tx_buffer[idx] = ((uint32)(encap_config->vlan & 0xff) << 24);
        rx_buffer[idx] = ((uint32)(encap_config->vlan & 0xff) << 24);
    }

    *index = idx;
    return;
}


/*
 * Function:
 *      _bcm_port_ehg_ethernet_header_parse
 * Purpose:
 *      Parses Ethernet header from EHG header
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      header          - (IN) pointer to the EHG header buffer
 *      encap_config    - (IN/OUT) structure describes port encapsulation configuration
 *      index           - (IN/OUT) index to the right location in buffer
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_port_ehg_ethernet_header_parse(int unit, bcm_port_t port, uint32 *header,
                                    bcm_port_encap_config_t *encap_config, int *index)
{
    uint32  val;
    int     idx = *index;
    
    BCM_IF_ERROR_RETURN(
        READ_EHG_TX_CONTROLr(unit, port, &val));
    encap_config->dst_mac[0] = (uint8)(header[idx] >> 16);
    encap_config->dst_mac[1] = (uint8)(header[idx] >> 8);
    encap_config->dst_mac[2] = (uint8)(header[idx]);
    idx--;
    encap_config->dst_mac[3] = (uint8)(header[idx] >> 24);
    encap_config->dst_mac[4] = (uint8)(header[idx] >> 16);
    encap_config->dst_mac[5] = (uint8)(header[idx] >> 8);
    encap_config->src_mac[0] = (uint8)(header[idx]);
    idx--;
    encap_config->src_mac[1] = (uint8)(header[idx] >> 24);
    encap_config->src_mac[2] = (uint8)(header[idx] >> 16);
    encap_config->src_mac[3] = (uint8)(header[idx] >> 8);
    encap_config->src_mac[4] = (uint8)(header[idx]);
    idx--;
    encap_config->src_mac[5] = (uint8)(header[idx] >> 24);

    if (soc_reg_field_get(unit, EHG_TX_CONTROLr, val, VLAN_TAG_CONTROLf)) {
        encap_config->tpid = (uint16)(header[idx] >> 8);
        encap_config->vlan = ((uint8)(header[idx])) << 8;
        idx--;
        encap_config->vlan |= (uint8)((header[idx] >> 24));
    } else {
        encap_config->vlan = BCM_VLAN_NONE;
        encap_config->tpid = BCM_VLAN_NONE;
    }

    *index = idx;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_port_ip4_tunnel_header_set
 * Purpose:
 *      Prepares and sets IP GRE tunnel header for Embedded higig
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      encap_config    - (IN) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_port_ip4_tunnel_header_set(int unit, bcm_port_t port, 
                               bcm_port_encap_config_t *encap_config)
{
    /*SW tunnel tx encap buffer.*/
    uint32 tx_buffer[_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ]; 
    /*SW tunnel rx encap buffer.*/
    uint32 rx_buffer[_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ]; 
    int    idx;  /* Headers offset iterator.*/
    
    sal_memset(tx_buffer, 0, WORDS2BYTES(_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ));
    sal_memset(rx_buffer, 0, WORDS2BYTES(_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ));

    idx = _BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ - 1; 
    
    _bcm_port_ehg_setup_txrx_ethernet_hdr(tx_buffer, rx_buffer, &idx, 
                                          encap_config);

    /* Set ether type to ip. 0x800  */
    tx_buffer[idx] |= (uint32)(0x08 << 16);
    rx_buffer[idx] |= (uint32)(0x08 << 16);
    /*
     *   IPv4 header. 
     */
    /* Version + 5 word no options length.  + Tos */
    /* Length, Id, Flags, Fragmentation offset. */
    tx_buffer[idx] |= ((uint32)(0x45)); 
    rx_buffer[idx] |= ((uint32)(0x45)); 
    idx--;

    tx_buffer[idx] = ((uint32)(encap_config->tos) << 24);
    rx_buffer[idx] = tx_buffer[idx];
    idx--;

    /* Ttl */
    tx_buffer[idx] = ((uint32)encap_config->ttl); 
    rx_buffer[idx] = ((uint32)encap_config->ttl); 
    idx--;

    /* Protocol (GRE 0x2f) */
    tx_buffer[idx] = ((uint32)(0x2f << 24));
    rx_buffer[idx] = ((uint32)(0x2f << 24));

    /* TX Src Ip. */
    tx_buffer[idx] |= ((uint32)(encap_config->src_addr >> 24));

    /* RX Dst Ip. */
    rx_buffer[idx] |= ((uint32)(encap_config->dst_addr >> 24));

    idx--;
    tx_buffer[idx] = (((uint32)((encap_config->src_addr << 8))));
    rx_buffer[idx] = (((uint32)((encap_config->dst_addr << 8))));

    /* TX Dst Ip. */
    tx_buffer[idx] |= ((uint32)(encap_config->dst_addr >> 24));
    /* RX Src Ip */
    rx_buffer[idx] |= ((uint32)(encap_config->src_addr >> 24));

    idx--;
    tx_buffer[idx] = (((uint32)((encap_config->dst_addr << 8))));
    rx_buffer[idx] = (((uint32)((encap_config->src_addr << 8))));

    /*
     *   Gre header. 
     */

    /* Protocol. 0x88be */
    tx_buffer[idx] |= ((uint32)(0x88));
    rx_buffer[idx] |= ((uint32)(0x88));
    idx--;

    tx_buffer[idx] = ((uint32)(0xbe) << 24);

    /* Set ether type to OUI  */
    tx_buffer[idx] |= (((uint32)(encap_config->oui_ethertype >> 8)) << 16 | \
                      ((uint32)(encap_config->oui_ethertype & 0xff)) << 8);

    /* Set OUI and HG ether type */
    tx_buffer[idx] |=  (((uint32)(encap_config->oui)[0]));
    /* From this point no difference between RX and TX */
    rx_buffer[idx] = tx_buffer[idx];    
    idx--;

    tx_buffer[idx] = (((uint32)(encap_config->oui)[1]) << 24 | \
                      ((uint32)(encap_config->oui)[2]) << 16);

    tx_buffer[idx] |= (((uint32)(encap_config->higig_ethertype >> 8)) << 8 | \
                       ((uint32)(encap_config->higig_ethertype)));

    rx_buffer[idx] = tx_buffer[idx];

    return _bcm_port_ehg_header_write(unit, port, tx_buffer, rx_buffer, 
                                      _BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ);
}

/*
 * Function:
 *      _bcm_port_ipv4_tunnel_header_parse
 * Purpose:
 *      Parse IPv4 GRE tunnel header for Embedded higig and updates encap_config 
 *      structure accordingly 
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      header          - (IN) Buffer contains EHG header to parse          
 *      encap_config    - (OUT) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_port_ipv4_tunnel_header_parse(int unit, bcm_port_t port, uint32 *header,
                                   bcm_port_encap_config_t *encap_config)
{
    /* Headers offset iterator, begins at end of header */
    int  idx = 4 * BYTES2WORDS(sizeof(xport_ehg_tx_tunnel_data_entry_t)) - 1; 

    BCM_IF_ERROR_RETURN(
    _bcm_port_ehg_ethernet_header_parse(unit, port, header, encap_config,
                                            &idx));

    idx--;
    encap_config->tos = (uint8)(header[idx] >> 24);
    idx--;
    encap_config->ttl = (uint8)(header[idx]);
    idx--;
    encap_config->src_addr = ((uint8)(header[idx])) << 24;
    idx--;
    encap_config->src_addr |=(uint32)(header[idx] >> 8);
    encap_config->dst_addr = ((uint8)(header[idx])) << 24;
    idx--;
    encap_config->dst_addr |=(uint32)(header[idx] >> 8);
    idx--;
    encap_config->oui_ethertype = (uint16)(header[idx] >> 8);
    encap_config->oui[0] = (uint8)(header[idx]);
    idx--;
    encap_config->oui[1] = (uint8)(header[idx] >> 24);
    encap_config->oui[2] = (uint8)(header[idx] >> 16);
    encap_config->higig_ethertype = (uint16)(header[idx]);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_l2_tunnel_header_set
 * Purpose:
 *      Prepares and sets L2 tunnel header for Embedded higig
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      encap_config    - (IN) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_port_l2_tunnel_header_set(int unit, bcm_port_t port, 
                               bcm_port_encap_config_t *encap_config)
{
    /*SW tunnel tx encap buffer.*/
    uint32 tx_buffer[_BCM_PORT_EHG_L2_HEADER_BUFFER_SZ]; 
    /*SW tunnel rx encap buffer.*/
    uint32 rx_buffer[_BCM_PORT_EHG_L2_HEADER_BUFFER_SZ]; 
    int     idx;  /* Headers offset iterator.*/
    
    sal_memset(tx_buffer, 0, WORDS2BYTES(_BCM_PORT_EHG_L2_HEADER_BUFFER_SZ));
    sal_memset(rx_buffer, 0, WORDS2BYTES(_BCM_PORT_EHG_L2_HEADER_BUFFER_SZ));

    idx = _BCM_PORT_EHG_L2_HEADER_BUFFER_SZ - 1;
    
    _bcm_port_ehg_setup_txrx_ethernet_hdr(tx_buffer, rx_buffer, &idx, 
                                          encap_config);

    /* Set ether type to OUI  */
    tx_buffer[idx] |= (((uint32)(encap_config->oui_ethertype >> 8)) << 16 | \
                       ((uint32)(encap_config->oui_ethertype & 0xff)) << 8);
    rx_buffer[idx] |= (((uint32)(encap_config->oui_ethertype >> 8)) << 16 | \
                       ((uint32)(encap_config->oui_ethertype & 0xff)) << 8);

    /* Set TX OUI and HG ether type */
    tx_buffer[idx] |=  (((uint32)(encap_config->oui)[0]));
    rx_buffer[idx] |=  (((uint32)(encap_config->oui)[0]));
    idx--;
    tx_buffer[idx] = (((uint32)(encap_config->oui)[1]) << 24 | \
                      ((uint32)(encap_config->oui)[2]) << 16);

    tx_buffer[idx] |= (((uint32)(encap_config->higig_ethertype >> 8)) << 8 | \
                       ((uint32)(encap_config->higig_ethertype)));
    rx_buffer[idx] = tx_buffer[idx];

    return _bcm_port_ehg_header_write(unit, port, tx_buffer, rx_buffer,
                                       _BCM_PORT_EHG_L2_HEADER_BUFFER_SZ);
}


/*
 * Function:
 *      _bcm_port_l2_tunnel_header_parse
 * Purpose:
 *      Parse L2 tunnel header for Embedded higig and updates encap_config 
 *      structure accordingly 
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      header          - (IN) Buffer contains EHG header to parse          
 *      encap_config    - (OUT) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_port_l2_tunnel_header_parse(int unit, bcm_port_t port, uint32 *header,
                                 bcm_port_encap_config_t *encap_config)
{
    /* Headers offset iterator, begins at end of header */
    int  idx = 4 * BYTES2WORDS(sizeof(xport_ehg_tx_tunnel_data_entry_t)) - 1; 
    
    BCM_IF_ERROR_RETURN
        (_bcm_port_ehg_ethernet_header_parse(unit, port, header,
                                             encap_config, &idx));

    encap_config->oui_ethertype = (uint16)(header[idx] >> 8);
    encap_config->oui[0] = (uint8)(header[idx]);
    idx--;
    encap_config->oui[1] = (uint8)(header[idx] >> 24);
    encap_config->oui[2] = (uint8)(header[idx] >> 16);
    encap_config->higig_ethertype = (uint16)(header[idx]);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_encap_ehg_xport_update
 * Purpose:
 *      Updates XPORT higig config for EHG
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 * Returns:
 *      
 */
STATIC int 
_bcm_port_encap_ehg_xport_update(int unit, bcm_port_t port)
{
#ifdef BCM_GXPORT_SUPPORT
    soc_reg_t   egr_port_reg;
    uint32      val;

    if (IS_GX_PORT(unit, port) || IS_XQ_PORT(unit,port)) {
        
        egr_port_reg = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                        SOC_IS_VALKYRIE2(unit)) ? EGR_PORT_64r : EGR_PORTr;
        
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_NONE,
                                   HIGIG2f, 1));
        
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_NONE,
                                   PORT_TYPEf, BCM_EGR_PORT_TYPE_EHG));
        BCM_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, egr_port_reg, port, HIGIG2f, 
                                    BCM_EGR_PORT_HIGIG2));
        BCM_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, egr_port_reg, port, PORT_TYPEf, 
                                    BCM_EGR_PORT_TYPE_EHG));
        
        if(SOC_REG_IS_VALID(unit, XPORT_CONFIGr)) {
            BCM_IF_ERROR_RETURN
                (READ_XPORT_CONFIGr(unit, port, &val));
            soc_reg_field_set(unit, XPORT_CONFIGr, &val, HIGIG2_MODEf, 1);
            BCM_IF_ERROR_RETURN
                (WRITE_XPORT_CONFIGr(unit, port, val));
        }

    }
#endif /* BCM_GXPORT_SUPPORT */
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_port_ehg_transport_mode_set
 * Purpose:
 *      Set the port into embedded higig transport mode
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      encap_config    - (IN) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_port_ehg_transport_mode_set(int unit, bcm_port_t port, 
                                 bcm_port_encap_config_t *encap_config)
{
    int         vlan_valid;
    uint32      val;

    if (IS_HG_PORT(unit, port)) {
        return BCM_E_CONFIG;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_port_encap_ehg_xport_update(unit, port));

    vlan_valid = BCM_VLAN_VALID(encap_config->vlan);

    /* Configure TX control */
    SOC_IF_ERROR_RETURN
        (READ_EHG_TX_CONTROLr(unit, port, &val));
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, ENABLEf, 1);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, TUNNEL_TYPEf, 0);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, VLAN_TAG_CONTROLf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, UPDATE_VLAN_PRI_CFIf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, PAYLOAD_LENGTH_ADJUSTMENTf,
                      0);    
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, UPDATE_DSCPf, 0);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, UPDATE_IPf, 0);
    SOC_IF_ERROR_RETURN
        (WRITE_EHG_TX_CONTROLr(unit, port, val));

    /* Configure RX control */
    SOC_IF_ERROR_RETURN
        (READ_EHG_RX_CONTROLr(unit, port, &val));
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, TUNNEL_TYPEf, 0);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, MODEf, 1);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, VLAN_TAG_CONTROLf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, COMPARE_VLANf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, VLANf, 
                      (vlan_valid) ? encap_config->vlan : 0);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, IPV4_CHKSUM_ENABLEf, 0);
    SOC_IF_ERROR_RETURN
        (WRITE_EHG_RX_CONTROLr(unit, port, val));

    return _bcm_port_l2_tunnel_header_set(unit, port, encap_config);

}

/*
 * Function:
 *      _bcm_port_ehg_tunnel_mode_set
 * Purpose:
 *      Set the port into embedded higig tunnel mode
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) Physical port
 *      encap_config    - (IN) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_port_ehg_tunnel_mode_set(int unit, bcm_port_t port, 
                                 bcm_port_encap_config_t *encap_config)
{
    int         vlan_valid;
    uint32      val;

    if (IS_HG_PORT(unit, port)) {
        return BCM_E_CONFIG;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_port_encap_ehg_xport_update(unit, port));

    vlan_valid = BCM_VLAN_VALID(encap_config->vlan);

    /* Configure TX control */
    SOC_IF_ERROR_RETURN
        (READ_EHG_TX_CONTROLr(unit, port, &val));
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, ENABLEf, 1);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, TUNNEL_TYPEf, 1);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, VLAN_TAG_CONTROLf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, UPDATE_VLAN_PRI_CFIf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, PAYLOAD_LENGTH_ADJUSTMENTf, 
         vlan_valid ? (WORDS2BYTES(_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ) - 1):
                 (WORDS2BYTES(_BCM_PORT_EHG_IP_GRE_HEADER_BUFFER_SZ - 1) - 1));
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, UPDATE_DSCPf, 1);
    soc_reg_field_set(unit, EHG_TX_CONTROLr, &val, UPDATE_IPf, 1);
    SOC_IF_ERROR_RETURN
      (WRITE_EHG_TX_CONTROLr(unit, port, val));

    /* Configure RX control */
    SOC_IF_ERROR_RETURN
        (READ_EHG_RX_CONTROLr(unit, port, &val));
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, MODEf, 1);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, TUNNEL_TYPEf, 1);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, VLAN_TAG_CONTROLf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, COMPARE_VLANf, 
                      vlan_valid);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, VLANf, 
                      (vlan_valid) ? encap_config->vlan : 0);
    soc_reg_field_set(unit, EHG_RX_CONTROLr, &val, IPV4_CHKSUM_ENABLEf, 1);
    SOC_IF_ERROR_RETURN
      (WRITE_EHG_RX_CONTROLr(unit, port, val));

    return _bcm_port_ip4_tunnel_header_set(unit, port, encap_config);
}


/*
 * Function:
 *      _bcm_esw_port_encap_higig_lite_set
 * Purpose:
 *      Helper function to force a port into HIGIG2_LITE mode
 * Parameters:
 *      unit            - (IN) device id
 *      port            - (IN) port number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_esw_port_encap_higig_lite_set(int unit, bcm_port_t port)
{
    soc_port_ability_t  ability;
    int                 an, an_done;

    if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
        /* Restrict the speed to <= 2.5G and set the encap to HG2 */
        BCM_IF_ERROR_RETURN(
            soc_phyctrl_ability_local_get(unit, port, &ability));

        ability.speed_full_duplex &= ~(SOC_PA_SPEED_10GB | 
                SOC_PA_SPEED_12GB | SOC_PA_SPEED_13GB | SOC_PA_SPEED_15GB | 
                SOC_PA_SPEED_16GB | SOC_PA_SPEED_20GB | SOC_PA_SPEED_21GB | 
                SOC_PA_SPEED_25GB | SOC_PA_SPEED_30GB | SOC_PA_SPEED_40GB);

        BCM_IF_ERROR_RETURN(
            soc_phyctrl_ability_advert_set(unit, port, &ability));
        BCM_IF_ERROR_RETURN(
            soc_phyctrl_auto_negotiate_get(unit, port, &an, &an_done));
        if (!an) {
            BCM_IF_ERROR_RETURN(
                soc_phyctrl_speed_set(unit, port, 2500));
        }

        return _bcm_port_encap_xport_set(unit, port, BCM_PORT_ENCAP_HIGIG2);
    } else if (IS_ST_PORT(unit, port) || IS_E_PORT(unit, port)) {
        return _bcm_port_encap_stport_set(unit, port, BCM_PORT_ENCAP_HIGIG2);
    }

    return BCM_E_CONFIG; 
}


/*
 * Function:
 *      bcm_esw_port_encap_config_set
 * Purpose:
 *      Set the port encapsulation 
 * Parameters:
 *      unit            - (IN) device id
 *      gport           - (IN) Generic port
 *      encap_config    - (IN) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_port_encap_config_set(int unit, bcm_gport_t gport, 
                                  bcm_port_encap_config_t *encap_config)
{
    bcm_port_t  port;
    int         rv = BCM_E_UNAVAIL;
    soc_info_t  *si = &SOC_INFO(unit);
    bcm_stg_t        stg = 0;

    if (NULL == encap_config) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, gport, &port));

    if (encap_config->encap != BCM_PORT_ENCAP_HIGIG2_L2 && 
        encap_config->encap != BCM_PORT_ENCAP_HIGIG2_LITE &&
        encap_config->encap != BCM_PORT_ENCAP_HIGIG2_IP_GRE ) {
        return bcm_esw_port_encap_set(unit, port, encap_config->encap);
    }

    if (encap_config->encap == BCM_PORT_ENCAP_HIGIG2_LITE &&
        (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit) || SOC_IS_RAVEN(unit))) {
        return bcm_esw_port_encap_set(unit, port, encap_config->encap);
    }

    if (!soc_feature(unit, soc_feature_embedded_higig)) {
        return (BCM_E_UNAVAIL);
    }

    PORT_LOCK(unit);

    if (encap_config->encap == BCM_PORT_ENCAP_HIGIG2_LITE) {
        rv = _bcm_esw_port_encap_higig_lite_set(unit, port);
    } else {
        if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
            rv = _bcm_port_encap_xport_set(unit, port, BCM_PORT_ENCAP_IEEE);
        } else if (IS_ST_PORT(unit, port) || IS_E_PORT(unit, port)) {
            rv = _bcm_port_encap_stport_set(unit, port, BCM_PORT_ENCAP_IEEE);
        }
    }

    if (BCM_SUCCESS(rv)) {
        if (BCM_PORT_ENCAP_HIGIG2_L2 == encap_config->encap) {
            rv = _bcm_port_ehg_transport_mode_set(unit, port, encap_config);
        } else if (BCM_PORT_ENCAP_HIGIG2_IP_GRE == encap_config->encap){
            rv = _bcm_port_ehg_tunnel_mode_set(unit, port, encap_config);
        } else if (BCM_PORT_ENCAP_HIGIG2_LITE == encap_config->encap){ 
            rv = soc_reg_field32_modify(unit, EHG_RX_CONTROLr, port, MODEf, 2);
        }
    }
    /* Embedded Higig ports should be configured and marked as ST ports */
    if (BCM_SUCCESS(rv)) {
        rv = bcm_esw_port_pause_set(unit, port, 0, 0);
    }
    if (BCM_SUCCESS(rv)) {
        /* Stack ports to forwarding */
        rv = bcm_esw_port_stp_set(unit, port, BCM_STG_STP_FORWARD);
    }
    if (BCM_SUCCESS(rv)) {
        /* Set untagged state in default VLAN properly */
        rv = _bcm_esw_vlan_untag_update(unit, port, TRUE);
    }
    if (BCM_SUCCESS(rv)) {
        /* Resolve STG 0 */
        rv = bcm_esw_stg_default_get(unit, &stg);
    }
    if (BCM_SUCCESS(rv)) {
        rv = bcm_esw_stg_stp_set(unit, stg, port, BCM_STG_STP_FORWARD);
    }
    if (BCM_SUCCESS(rv)) {
        SOC_PBMP_PORT_ADD(si->st.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->ether.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->hg2_pbm, port);
    }

    PORT_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *      bcm_esw_port_encap_config_get
 * Purpose:
 *      Get the port encapsulation 
 * Parameters:
 *      unit            - (IN) device id
 *      gport           - (IN) Generic port
 *      encap_config    - (OUT) structure describes port encapsulation configuration
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_port_encap_config_get(int unit, bcm_gport_t gport, 
                                  bcm_port_encap_config_t *encap_config)
{
    bcm_port_t      port;
    uint32          val, buffer[16];   /* 64 bytes max EHG header size */
    int             rv = BCM_E_NONE, mode = 0; 

    if (NULL == encap_config) {
        return (BCM_E_PARAM);
    }

    sal_memset(encap_config, 0, sizeof(bcm_port_encap_config_t));

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, gport, &port));
    
    if (!soc_feature(unit, soc_feature_embedded_higig)) {
        rv = bcm_esw_port_encap_get(unit, port, &mode);
        if (BCM_SUCCESS(rv)) {
            encap_config->encap = (bcm_port_encap_t)mode; 
        }
        return rv;
    }

    /* Check for Raptor-style stacking */
    SOC_IF_ERROR_RETURN(READ_EHG_RX_CONTROLr(unit, port, &val));
    if (soc_reg_field_get(unit, EHG_RX_CONTROLr, val, MODEf) == 2) {
        encap_config->encap = BCM_PORT_ENCAP_HIGIG2_LITE;
        return rv;
    }

    SOC_IF_ERROR_RETURN(READ_EHG_TX_CONTROLr(unit, port, &val));
    /* If no EHG was programmed the just read port encapsulation */
    if (!soc_reg_field_get(unit, EHG_TX_CONTROLr, val, ENABLEf)) {
        rv = bcm_esw_port_encap_get(unit, port, &mode);
        if (BCM_SUCCESS(rv)) {
            encap_config->encap = (bcm_port_encap_t)mode; 
        }
        return rv;
    }

    /* EHG is enabled. */
    if (!IS_ST_PORT(unit, port)) {
        return (BCM_E_CONFIG);
    }

    BCM_IF_ERROR_RETURN(_bcm_port_ehg_header_read(unit, port, buffer));

    if (!soc_reg_field_get(unit, EHG_TX_CONTROLr, val, TUNNEL_TYPEf)) {
        encap_config->encap = BCM_PORT_ENCAP_HIGIG2_L2;
        rv = _bcm_port_l2_tunnel_header_parse(unit, port, buffer, 
                                              encap_config);
    } else {
        encap_config->encap = BCM_PORT_ENCAP_HIGIG2_IP_GRE;
        rv = _bcm_port_ipv4_tunnel_header_parse(unit, port, buffer, 
                                                encap_config);
    }

    return (rv);
}

/*
 * Function:
 *      bcm_port_encap_set
 * Purpose:
 *      Set the port encapsulation mode
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      mode - One of BCM_PORT_ENCAP_xxx (see port.h)
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_encap_set(int unit, bcm_port_t port, int mode)
{
    int         rv, xport_swap = FALSE;
    int         stport_swap = FALSE;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_esw_port_encap_set: u=%d p=%d mode=%d\n",
                     unit, port, mode));

    PORT_LOCK(unit);

    if ((IS_HG_PORT(unit,port) && (mode == BCM_PORT_ENCAP_IEEE)) ||
        (IS_XE_PORT(unit,port) && (mode != BCM_PORT_ENCAP_IEEE))) {
        if (soc_feature(unit, soc_feature_xport_convertible)) {
            xport_swap =  TRUE;
        } else {
            /* Ether <=> Higig not allowed on all systems */
            PORT_UNLOCK(unit);
            return BCM_E_UNAVAIL;
        }
    }

    if (SOC_IS_RAVEN(unit) || SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
        if ((IS_ST_PORT(unit,port) && (mode == BCM_PORT_ENCAP_IEEE)) ||
            (IS_E_PORT(unit,port) && (mode == BCM_PORT_ENCAP_HIGIG2 ||
                                      mode == BCM_PORT_ENCAP_HIGIG2_LITE))) {
                stport_swap =  TRUE;
        }
    }

    if (xport_swap) {
        COUNTER_LOCK(unit);
        if ((BCM_PORT_ENCAP_HIGIG2_LITE == mode)) {
            rv = _bcm_esw_port_encap_higig_lite_set(unit, port);
        } else {
            rv = _bcm_port_encap_xport_set(unit, port, mode);
        }
        
        COUNTER_UNLOCK(unit);
    } else if (stport_swap) {
        rv = _bcm_port_encap_stport_set(unit, port, mode);
    } else if (IS_HG_PORT(unit, port)) {
        rv = MAC_ENCAP_SET(PORT(unit, port).p_mac, unit, port, mode);
    } else if (IS_GE_PORT(unit, port) && IS_ST_PORT(unit, port)) {
        if (mode == BCM_PORT_ENCAP_IEEE) {
            bcm_port_encap_config_t encap_config;

            rv = bcm_esw_port_encap_config_get(unit, port,
                                               &encap_config);
            if (BCM_SUCCESS(rv)) {
                if ((BCM_PORT_ENCAP_HIGIG2_L2 == encap_config.encap) ||
                    (BCM_PORT_ENCAP_HIGIG2_IP_GRE == encap_config.encap) ||
                    (BCM_PORT_ENCAP_HIGIG2_LITE == encap_config.encap)) {
                    rv = _bcm_port_encap_stport_set(unit, port,
                                                    BCM_PORT_ENCAP_IEEE);
                } else {
                    rv = BCM_E_UNAVAIL;
                }
            }
        } else {
            if ((mode == BCM_PORT_ENCAP_HIGIG2) || 
                (mode == BCM_PORT_ENCAP_HIGIG2_LITE && 
                 (SOC_IS_RAVEN(unit) || SOC_IS_ENDURO(unit) || 
                  SOC_IS_HURRICANE( unit)))) {
                rv = BCM_E_NONE;
            } else {
                rv = BCM_E_UNAVAIL;
            }
        }
    } else if (mode == BCM_PORT_ENCAP_IEEE) {
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_UNAVAIL;
    }

#ifdef BCM_GXPORT_SUPPORT
    if (IS_GX_PORT(unit, port) || IS_XG_PORT(unit,port) ||
        (IS_XQ_PORT(unit, port) && (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)))
		|| ((IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) && SOC_IS_KATANA(unit))) {
        int hg2 = FALSE;
        soc_reg_t egr_port_reg;
        soc_reg_t port_config_reg;
        
        egr_port_reg = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                        SOC_IS_VALKYRIE2(unit)) ? EGR_PORT_64r : EGR_PORTr;
        if (SOC_REG_IS_VALID(unit, XLPORT_CONFIGr)) {
            port_config_reg = XLPORT_CONFIGr;
        } else {
            port_config_reg = XPORT_CONFIGr;
        }
        if (mode == BCM_PORT_ENCAP_HIGIG2 || mode == BCM_PORT_ENCAP_HIGIG2_LITE) {
            hg2 = TRUE;
        }

        if (BCM_SUCCESS(rv)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_NONE,
                                       HIGIG2f, hg2);
        }

        if (BCM_SUCCESS(rv)) {
            if (SOC_REG_FIELD_VALID(unit, egr_port_reg, HIGIG2f)) {
                rv = soc_reg_field32_modify(unit, egr_port_reg, port, HIGIG2f,
                                            (uint32)hg2);
            } else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm, HIGIG2f)) {
                rv = soc_mem_field32_modify(unit, EGR_PORTm, port, HIGIG2f,
                                            hg2);
            }
        }
        if (BCM_SUCCESS(rv)) {
            rv = soc_reg_field32_modify(unit, port_config_reg, port,
                                        HIGIG2_MODEf, hg2);
        }
        if (BCM_SUCCESS(rv)) {
            if (SOC_MEM_IS_VALID(unit, EGR_ING_PORTm)) {
                if (IS_CPU_PORT(unit, port)) {
                    rv = soc_mem_field32_modify(unit, EGR_ING_PORTm,
                                                SOC_INFO(unit).cpu_hg_index,
                                                HIGIG2f, hg2);
                } else {
                    rv = soc_mem_field32_modify(unit, EGR_ING_PORTm, port,
                                                HIGIG2f, hg2);
                }
            }
        }
    }
#endif /* BCM_GXPORT_SUPPORT */

    if (soc_feature(unit, soc_feature_embedded_higig)) {
        /* Clear embedded Higig regs, if present */
        if (BCM_SUCCESS(rv) && SOC_REG_IS_VALID(unit, EHG_TX_CONTROLr)) {
            rv = WRITE_EHG_TX_CONTROLr(unit, port, 0);
        }
        if (BCM_SUCCESS(rv) && SOC_REG_IS_VALID(unit, EHG_RX_CONTROLr)) {
            rv = WRITE_EHG_RX_CONTROLr(unit, port, 0);
        }
    }

#ifdef BCM_HIGIG2_SUPPORT
    /* Update cached version of HiGig2 encapsulation */
    if (BCM_SUCCESS(rv)) {
        if (mode == BCM_PORT_ENCAP_HIGIG2 || mode == BCM_PORT_ENCAP_HIGIG2_LITE) {
            SOC_HG2_ENABLED_PORT_ADD(unit, port);
        } else {
            SOC_HG2_ENABLED_PORT_REMOVE(unit, port);
        }
    }
#endif /* BCM_HIGIG2_SUPPORT */

    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_encap_get
 * Purpose:
 *      Get the port encapsulation mode
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      mode (OUT) - One of BCM_PORT_ENCAP_xxx (see port.h)
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_encap_get(int unit, bcm_port_t port, int *mode)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
        if (IS_HL_PORT(unit, port)) {
            *mode = BCM_PORT_ENCAP_HIGIG2;
        } else {
            SOC_IF_ERROR_RETURN
                (MAC_ENCAP_GET(PORT(unit, port).p_mac, unit, port, mode));
        }
    } else
#endif  /* BCM_ENDURO_SUPPORT */
    if (IS_GE_PORT(unit, port) && IS_ST_PORT(unit, port)) {
        uint32 rval;
        soc_field_t hg_en[6] = {-1, HGIG2_EN_S0f, HGIG2_EN_S1f, -1, 
                                HGIG2_EN_S3f, HGIG2_EN_S4f};
        BCM_IF_ERROR_RETURN(READ_GPORT_CONFIGr(unit, port, &rval));
        if (soc_feature(unit, soc_feature_embedded_higig)) {
            *mode = BCM_PORT_ENCAP_IEEE;
        } else {
            if (SOC_REG_FIELD_VALID(unit, GPORT_CONFIGr, hg_en[port])) {
                *mode = soc_reg_field_get(unit, GPORT_CONFIGr, rval, 
                    hg_en[port]) ? BCM_PORT_ENCAP_HIGIG2 : BCM_PORT_ENCAP_IEEE;
            } else {
                return (BCM_E_CONFIG);
            }
        }
    } else {
        SOC_IF_ERROR_RETURN
            (MAC_ENCAP_GET(PORT(unit, port).p_mac, unit, port, mode));
    }
    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_esw_port_encap_get: u=%d p=%d mode=%d\n",
                     unit, port, *mode));
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_queued_count_get
 * Purpose:
 *      Returns the count of packets (or cells) currently buffered
 *      for a port.  Useful to know when a port has drained all
 *      data and can then be re-configured.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      count (OUT) - count of packets currently buffered
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      "packets" may actually be cells on most chips,
 *      If no packets are buffered, the cell count is 0,
 *      If some packets are buffered the cell count will be
 *      greater than or equal to the packet count.
 */

int
bcm_esw_port_queued_count_get(int unit, bcm_port_t port, uint32 *count)
{
    uint32      regval;
    int         cos;

    *count = 0;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (SOC_IS_DRACO(unit) || SOC_IS_LYNX(unit) || SOC_IS_FB_FX_HX(unit)) {
        for (cos = 0; cos < NUM_COS(unit); cos++) {
            regval = 0;
            SOC_IF_ERROR_RETURN(READ_COSLCCOUNTr(unit, port, cos, &regval));
            *count += soc_reg_field_get(unit, COSLCCOUNTr, regval, LCCOUNTf);
        }
        return BCM_E_NONE;
    }

#if defined(BCM_TUCANA_SUPPORT) || defined(BCM_EASYRIDER_SUPPORT)
    /*
     * Tucana and Easyrider actually keep the packet count!
     */
    if (SOC_IS_TUCANA(unit) || SOC_IS_EASYRIDER(unit)) {
        soc_field_t count_f = SOC_IS_EASYRIDER(unit) ? COUNTf : PKTCOUNTf;
        for (cos = 0; cos < NUM_COS(unit); cos++) {
            regval = 0;
            SOC_IF_ERROR_RETURN(READ_MTPCOSr(unit, port, cos, &regval));
            *count += soc_reg_field_get(unit, MTPCOSr, regval,
                                        count_f);
        }
        return BCM_E_NONE;
    }
#endif  /* BCM_TUCANA_SUPPORT || BCM_EASYRIDER_SUPPORT */

#if defined(BCM_XGS12_FABRIC_SUPPORT)
    if (SOC_IS_XGS12_FABRIC(unit)) {
        regval = 0;
        SOC_IF_ERROR_RETURN(READ_MMU_CELLCNTTOTALr(unit, port, &regval));
        *count += soc_reg_field_get(unit, MMU_CELLCNTTOTALr, regval, COUNTf);
        return BCM_E_NONE;
    }
#endif /* BCM_XGS12_FABRIC_SUPPORT */

#if defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_HBX(unit)) {
        regval = 0;
        SOC_IF_ERROR_RETURN(READ_OP_PORT_TOTAL_COUNTr(unit, port, &regval));
        *count += soc_reg_field_get(unit, OP_PORT_TOTAL_COUNTr,
                                    regval, OP_PORT_TOTAL_COUNTf);
        return BCM_E_NONE;
    }
#endif /* BCM_BRADLEY_SUPPORT  */

    return BCM_E_UNAVAIL;
}

    /*
 * Function:
 *      bcm_port_protocol_vlan_add
 * Purpose:
 *      Adds a protocol based vlan to a port.  The protocol
 *      is matched by frame type and ether type.  Returns an
 *      error if hardware does not support protocol vlans
 *      (Strata and Hercules).
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      frame - one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - 16 bit Ethernet type field
 *      vid - VLAN ID
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_protocol_vlan_add(int unit,
                           bcm_port_t port,
                           bcm_port_frametype_t frame,
                           bcm_port_ethertype_t ether,
                           bcm_vlan_t vid)
{
#ifdef  BCM_XGS_SWITCH_SUPPORT

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    PORT_SWITCHED_CHECK(unit, port);

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        bcm_vlan_action_set_t action;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        bcm_vlan_action_set_t_init(&action);
        action.new_outer_vlan = vid;
        action.new_inner_vlan = 0;
        action.priority = PORT(unit, port).p_ut_prio;
        action.ut_outer = bcmVlanActionAdd;
        action.it_outer = bcmVlanActionAdd;
        action.it_inner = bcmVlanActionDelete;
        action.it_inner_prio = bcmVlanActionNone;
        action.ot_outer_prio = bcmVlanActionReplace;
        action.dt_outer_prio = bcmVlanActionReplace;

        return _bcm_trx_vlan_port_protocol_action_add(unit, port, frame,
                                                     ether, &action);
    }
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_DRACO1_SUPPORT) || defined(BCM_TUCANA_SUPPORT)
    if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {
        return _bcm_draco_port_protocol_vlan_add(unit, port, frame,
                                                 ether, vid);
    }
#endif /* BCM_DRACO1_SUPPORT || BCM_TUCANA_SUPPORT */

#if defined(BCM_DRACO15_SUPPORT)
    if (SOC_IS_DRACO15(unit)) {
        return _bcm_draco15_port_protocol_vlan_add(unit, port, frame, 
                                                   ether, vid);
    }
#endif /* BCM_DRACO15_SUPPORT */

#if defined(BCM_LYNX_SUPPORT)
    if (SOC_IS_LYNX(unit)) {
        return _bcm_lynx_port_protocol_vlan_add(unit, port, frame, ether, vid);
    }
#endif /* BCM_LYNX_SUPPORT */

#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_FBX(unit)) {
        return _bcm_fb_port_protocol_vlan_add(unit, port, frame, ether, vid);
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)) {
        return _bcm_er_port_protocol_vlan_add(unit, port, frame, ether, vid);
    }
#endif /* BCM_EASYRIDER_SUPPORT */
#endif /* BCM_XGS_SWITCH_SUPPORT */
    /* not supported on STRATA and HERCULES */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_protocol_vlan_delete
 * Purpose:
 *      Remove an already created proto protocol based vlan
 *      on a port.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 *      frame - one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - 16 bit Ethernet type field
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_protocol_vlan_delete(int unit,
                              bcm_port_t port,
                              bcm_port_frametype_t frame,
                              bcm_port_ethertype_t ether)
{
#ifdef  BCM_XGS_SWITCH_SUPPORT
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    PORT_SWITCHED_CHECK(unit, port);

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        return _bcm_trx_vlan_port_protocol_delete(unit, port, frame, ether);
    }
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_DRACO1_SUPPORT) || defined(BCM_TUCANA_SUPPORT)
    if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {
        return _bcm_draco_port_protocol_vlan_delete(unit, port, frame, ether);
    }
#endif /* BCM_DRACO1_SUPPORT || BCM_TUCANA_SUPPORT */

#if defined(BCM_DRACO15_SUPPORT)
    if (SOC_IS_DRACO15(unit)) {
        return _bcm_draco15_port_protocol_vlan_delete(unit, port, frame, ether);
    }
#endif /* BCM_DRACO15_SUPPORT */

#if defined(BCM_LYNX_SUPPORT)
    if (SOC_IS_LYNX(unit)) {
        return _bcm_lynx_port_protocol_vlan_delete(unit, port, frame, ether);
    }
#endif /* BCM_LYNX_SUPPORT */

#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_FBX(unit)) {
        return _bcm_fb_port_protocol_vlan_delete(unit, port, frame, ether);
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)) {
        return _bcm_er_port_protocol_vlan_delete(unit, port, frame, ether);
    }
#endif /* BCM_EASYRIDER_SUPPORT */
#endif /* BCM_XGS_SWITCH_SUPPORT */
    /* not supported on STRATA and HERCULES */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_protocol_vlan_delete_all
 * Purpose:
 *      Remove all protocol based vlans on a port.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - StrataSwitch port #
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_protocol_vlan_delete_all(int unit, bcm_port_t port)
{
#ifdef  BCM_XGS_SWITCH_SUPPORT
    int         i;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    PORT_SWITCHED_CHECK(unit, port);

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }
        return _bcm_trx_vlan_port_protocol_delete_all(unit, port);
    }
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_DRACO1_SUPPORT) || defined(BCM_TUCANA_SUPPORT)
    if (SOC_IS_DRACO1(unit) || SOC_IS_TUCANA(unit)) {
        uint64  regval;

        COMPILER_64_ZERO(regval);

        for (i = 0; i < SOC_REG_NUMELS(unit, PRTABLE_ENTRYr); i++) {
            SOC_IF_ERROR_RETURN(WRITE_PRTABLE_ENTRYr(unit, port, i, regval));
        }
        return BCM_E_NONE;
    }
#endif /* BCM_DRACO1_SUPPORT || BCM_TUCANA_SUPPORT */

#if defined(BCM_DRACO15_SUPPORT)
    if (SOC_IS_DRACO15(unit)) {
        vlan_protocol_entry_t   vpe;
        vlan_data_entry_t       vde;
        bcm_vlan_t              cvid, defvid;
        _bcm_port_info_t        *pinfo;
        int                     idxmin, idxmax;
        int                     vlan_prot_entries, vlan_data_prot_start;
        int                     vdentry, p, valid;

        idxmin = soc_mem_index_min(unit, VLAN_PROTOCOLm);
        idxmax = soc_mem_index_max(unit, VLAN_PROTOCOLm);
        vlan_prot_entries = soc_mem_index_count(unit, VLAN_PROTOCOLm);
        vlan_data_prot_start = soc_mem_index_max(unit, VLAN_SUBNETm) + 1;

        for (i = idxmin; i <= idxmax; i++) {
            vdentry = vlan_data_prot_start + (port * vlan_prot_entries) + i;
            SOC_IF_ERROR_RETURN
                (READ_VLAN_DATAm(unit, MEM_BLOCK_ANY, vdentry, &vde));
            cvid = soc_VLAN_DATAm_field32_get(unit, &vde, VLAN_IDf);
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_untagged_vlan_get(unit, port, &defvid));
            BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
            if (cvid == 0 ||
                (cvid == defvid && (!_BCM_PORT_VD_PBVL_IS_SET(pinfo, i)))) {
                continue;
            }
            sal_memset(&vde, 0, sizeof(vde));
            soc_VLAN_DATAm_field32_set(unit, &vde, VLAN_IDf, defvid);
            SOC_IF_ERROR_RETURN
                (WRITE_VLAN_DATAm(unit, MEM_BLOCK_ALL, vdentry, &vde));
            _BCM_PORT_VD_PBVL_CLEAR(pinfo, i);
            /* see if any vlan_data entries are still valid */
            valid = 0;
            PBMP_E_ITER(unit, p) {
                BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, p, &pinfo));
                if (p == port) {        /* skip the entry we just wrote */
                    continue;
                }
                /* entry valid if programmed VLAN ID for port */
                if (_BCM_PORT_VD_PBVL_IS_SET(pinfo, i)) {
                    valid = 1;
                    break;
                }
            }
            if (!valid) {
                /* clear all VLAN_DATA entries associated with this protocol */
                PBMP_E_ITER(unit, p) {
                    vdentry = vlan_data_prot_start + (p * vlan_prot_entries) + i;
                    sal_memset(&vde, 0, sizeof(vde));
                    SOC_IF_ERROR_RETURN
                        (WRITE_VLAN_DATAm(unit, MEM_BLOCK_ALL, vdentry, &vde));
                    BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, p, &pinfo));
                    _BCM_PORT_VD_PBVL_CLEAR(pinfo, i);
                }
                /* clear VLAN_PROTOCOL entry */
                sal_memset(&vpe, 0, sizeof(vpe));
                SOC_IF_ERROR_RETURN
                    (WRITE_VLAN_PROTOCOLm(unit, MEM_BLOCK_ALL, i, &vpe));
            }
        }
        return BCM_E_NONE;
    }
#endif /* BCM_DRACO15_SUPPORT */

#if defined(BCM_LYNX_SUPPORT)
    if (SOC_IS_LYNX(unit)) {
        int     idxmin, idxmax, blk;
        pr_tab_entry_t  pte;

        blk = SOC_PORT_BLOCK(unit, port);
        sal_memset(&pte, 0, sizeof(pte));

        idxmin = soc_mem_index_min(unit, PR_TABm);
        idxmax = soc_mem_index_max(unit, PR_TABm);
        for (i = idxmin; i <= idxmax; i++) {
            SOC_IF_ERROR_RETURN(WRITE_PR_TABm(unit, blk, i, &pte));
        }
        return BCM_E_NONE;
    }
#endif /* BCM_LYNX_SUPPORT */

#if defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_FBX(unit)) {
        vlan_protocol_entry_t        vpe;
        vlan_protocol_data_entry_t   vde;
        bcm_vlan_t                   cvid, defvid;
        int                          idxmin, idxmax;
        int                          vlan_prot_entries;
        int                          vdentry, p, valid;
        _bcm_port_info_t             *pinfo;
        bcm_pbmp_t                   switched_pbm;

        idxmin = soc_mem_index_min(unit, VLAN_PROTOCOLm);
        idxmax = soc_mem_index_max(unit, VLAN_PROTOCOLm);
        vlan_prot_entries = idxmax + 1;

        for (i = idxmin; i <= idxmax; i++) {
            vdentry = (port * vlan_prot_entries) + i;
            SOC_IF_ERROR_RETURN
                (READ_VLAN_PROTOCOL_DATAm(unit, MEM_BLOCK_ANY,
                                          vdentry, &vde));
            cvid = soc_VLAN_PROTOCOL_DATAm_field32_get(unit, &vde, VLAN_IDf);
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_untagged_vlan_get(unit, port, &defvid));
            BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
            if (cvid == 0 ||
                (cvid == defvid && (!_BCM_PORT_VD_PBVL_IS_SET(pinfo, i)))) {
                continue;
            }
            sal_memset(&vde, 0, sizeof(vde));
            soc_VLAN_PROTOCOL_DATAm_field32_set(unit, &vde, VLAN_IDf, defvid);
            SOC_IF_ERROR_RETURN
                (WRITE_VLAN_PROTOCOL_DATAm(unit, MEM_BLOCK_ALL, vdentry, &vde));
            _BCM_PORT_VD_PBVL_CLEAR(pinfo, i);
            /*
             * see if any vlan_protocol_data entries are still valid
             * for the current protocol.
             */
            valid = 0;
            switched_pbm = PBMP_E_ALL(unit);
            if (soc_feature(unit, soc_feature_cpuport_switched)) {
                BCM_PBMP_OR(switched_pbm, PBMP_CMIC(unit));
            }
       
            BCM_PBMP_ITER(switched_pbm, p) {
                if (p == port) {    /* skip the entry we just "defaulted" */
                    continue;
                }
                BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, p, &pinfo));
                /* entry valid if programmed VLAN ID for port */
                if (_BCM_PORT_VD_PBVL_IS_SET(pinfo, i)) {
                    valid = 1;
                    break;
                }
            }
            if (!valid) {
                /*
                 * Clear all VLAN_PROTOCOL_DATA entries associated
                 * with this protocol.
                 */
                BCM_PBMP_ITER(switched_pbm, p) {
                    vdentry = (p * vlan_prot_entries) + i;
                    sal_memset(&vde, 0, sizeof(vde));
                    SOC_IF_ERROR_RETURN
                        (WRITE_VLAN_PROTOCOL_DATAm(unit, MEM_BLOCK_ALL,
                                                   vdentry, &vde));
                    BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, p, &pinfo));
                    _BCM_PORT_VD_PBVL_CLEAR(pinfo, i);
                }
                /* clear VLAN_PROTOCOL entry */
                sal_memset(&vpe, 0, sizeof(vpe));
                SOC_IF_ERROR_RETURN
                    (WRITE_VLAN_PROTOCOLm(unit, MEM_BLOCK_ALL, i, &vpe));
            }
        }
        return BCM_E_NONE;
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)) {
        /*
         * Disable all protocol mapping & map data for target port.
         */
        for (i = 0; i < SOC_REG_NUMELS(unit, VLAN_PROTOCOLr); i++) {
            SOC_IF_ERROR_RETURN
                (WRITE_VLAN_PROTOCOLr(unit, port, i, 0));
            SOC_IF_ERROR_RETURN
                (WRITE_VLAN_PROTOCOL_DATAr(unit, port, i, 0));
        }
        return BCM_E_NONE;
    }
#endif /* BCM_EASYRIDER_SUPPORT */
#endif /* BCM_XGS_SWITCH_SUPPORT */

    /* not supported on STRATA and HERCULES */

    return BCM_E_UNAVAIL;
}

/*
 * Differentiated Services Code Point Mapping
 */

/*
 * Function:
 *      bcm_esw_port_dscp_map_mode_set
 * Purpose:
 *      Set DSCP mapping for the port.
 * Parameters:
 *      unit - switch device
 *      port - switch port      or -1 to apply on all the ports.
 *      mode - BCM_PORT_DSCP_MAP_NONE
 *           - BCM_PORT_DSCP_MAP_ZERO Map if incomming DSCP = 0
 *           - BCM_PORT_DSCP_MAP_ALL DSCP -> DSCP mapping. 
 * Returns:
 *      BCM_E_XXX
 */
static int
_bcm_esw_port_dscp_map_mode_set(int unit, bcm_port_t port, int mode)
{
    bcm_port_cfg_t      pcfg;

    BCM_IF_ERROR_RETURN
        (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
    if (pcfg.pc_dse_mode == -1) {       /* STRATA1 for example */
        return BCM_E_UNAVAIL;
    }
    /*
     * Strata/Draco/Lynx/Tucana supports an additional mode. mode = 2
     */
    switch(mode) {
        case BCM_PORT_DSCP_MAP_NONE:
            pcfg.pc_dse_mode = 0;
#if defined(BCM_XGS_SWITCH_SUPPORT)
            pcfg.pc_dscp_prio = 0;
#endif /* BCM_XGS_SWITCH_SUPPORT */
            break;
        case BCM_PORT_DSCP_MAP_ZERO:
            if (soc_feature(unit, soc_feature_dscp_map_mode_all)) {
                return BCM_E_UNAVAIL;
            }
            pcfg.pc_dse_mode = 1;
#if defined(BCM_XGS_SWITCH_SUPPORT)
            pcfg.pc_dscp_prio = 1;
#endif /* BCM_XGS_SWITCH_SUPPORT */
            break;
        case BCM_PORT_DSCP_MAP_ALL:
            if (soc_feature(unit, soc_feature_dscp_map_mode_all)) {
                pcfg.pc_dse_mode = 1;
            } else {
                pcfg.pc_dse_mode = 2;
            }
#if defined(BCM_XGS_SWITCH_SUPPORT)
            pcfg.pc_dscp_prio = 1;
#endif /* BCM_XGS_SWITCH_SUPPORT */
            break;
        default:
            return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN
        (mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg));

    return BCM_E_NONE;
}

int
bcm_esw_port_dscp_map_mode_set(int unit, bcm_port_t port, int mode)
{
    int rv;
    bcm_port_config_t port_conf;
    bcm_pbmp_t pbmp;

    if (!soc_feature(unit, soc_feature_dscp)) {
        return BCM_E_UNAVAIL;
    }

    if (port != -1) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    }

    PORT_LOCK(unit);

    /* Get device port configuration. */
    rv = bcm_esw_port_config_get(unit, &port_conf);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }
    BCM_PBMP_ASSIGN(pbmp, port_conf.e);
    if (SOC_IS_XGS3_SWITCH(unit) || SOC_IS_XGS3_FABRIC(unit)) {
        BCM_PBMP_OR(pbmp, port_conf.cpu);
    }

    if (port == -1) {
        PBMP_ITER(pbmp, port) {
            rv = _bcm_esw_port_dscp_map_mode_set(unit, port, mode);
            if (BCM_FAILURE(rv)) {
                PORT_UNLOCK(unit);
                return rv;
            }
        }
    } else {
        if (BCM_PBMP_MEMBER(pbmp, port)) {
            rv = _bcm_esw_port_dscp_map_mode_set(unit, port, mode);
        } else {
            rv = BCM_E_PORT;
        }
    }
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_dscp_map_mode_get
 * Purpose:
 *      DSCP mapping status for the port.
 * Parameters:
 *      unit - switch device
 *      port - switch port or -1 to get mode from first available port
 *      mode - (OUT)
 *           - BCM_PORT_DSCP_MAP_NONE
 *           - BCM_PORT_DSCP_MAP_ZERO Map if incomming DSCP = 0
 *           - BCM_PORT_DSCP_MAP_ALL DSCP -> DSCP mapping. 
 * Returns:
 *      BCM_E_XXX
 */

static int
_bcm_esw_port_dscp_map_mode_get(int unit, bcm_port_t port, int *mode)
{
    bcm_port_cfg_t      pcfg;

    BCM_IF_ERROR_RETURN
        (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
    if (pcfg.pc_dse_mode == -1) {       /* STRATA1 for example */
        return BCM_E_UNAVAIL;
    }
    /*
     * Strata/Draco/Lynx/Tucana supports an additional mode. mode = 2
     */
    switch(pcfg.pc_dse_mode) {
        case 1:
            *mode = soc_feature(unit, soc_feature_dscp_map_mode_all) ?
                        BCM_PORT_DSCP_MAP_ALL : BCM_PORT_DSCP_MAP_ZERO;
            break;
        case 2:
            *mode = BCM_PORT_DSCP_MAP_ALL;
            break;
        case 0:
        default:
            *mode = BCM_PORT_DSCP_MAP_NONE;
            break;
    }

    return BCM_E_NONE;
}

int
bcm_esw_port_dscp_map_mode_get(int unit, bcm_port_t port, int *mode)
{
    int rv;
    bcm_port_config_t port_conf;
    bcm_pbmp_t pbmp;

    if (!soc_feature(unit, soc_feature_dscp)) {
        return BCM_E_UNAVAIL;
    }

    if (port != -1) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    }

    PORT_LOCK(unit);

    /* Get device port configuration. */
    rv = bcm_esw_port_config_get(unit, &port_conf);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }
    BCM_PBMP_ASSIGN(pbmp, port_conf.e);
    if (SOC_IS_XGS3_SWITCH(unit) || SOC_IS_XGS3_FABRIC(unit)) {
        BCM_PBMP_OR(pbmp, port_conf.cpu);
    }

    if (port == -1) {
        PBMP_ITER(pbmp, port) {
            break;
        }
    }

    /* coverity[overrun-local : FALSE] */
    if (BCM_PBMP_MEMBER(pbmp, port)) {
        rv = _bcm_esw_port_dscp_map_mode_get(unit, port, mode);
    } else {
        rv = BCM_E_PORT;
    }
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_port_dscp_map_set
 * Purpose:
 *      Internal implementation for bcm_port_dscp_map_set
 * Parameters:
 *      unit - switch device
 *      port - switch port or -1 for global table
 *      srccp - src code point or -1
 *      mapcp - mapped code point or -1
 *      prio - priority value for mapped code point
 *              -1 to use port default untagged priority
 *              BCM_PRIO_RED    can be or'ed into the priority
 *              BCM_PRIO_YELLOW can be or'ed into the priority
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_port_dscp_map_set(int unit, bcm_port_t port,
                       int srccp, int mapcp, int prio)
{
#define DSCP_CODE_POINT_CNT 64
#define DSCP_CODE_POINT_MAX (DSCP_CODE_POINT_CNT - 1)
    bcm_port_cfg_t      pcfg;

    if (mapcp < -1 || mapcp > DSCP_CODE_POINT_MAX) {
        return BCM_E_PARAM;
    }

#if defined(BCM_DRACO15_SUPPORT)
    if (SOC_IS_DRACO15(unit)) {
        int             base, i, cng;
        dscp_entry_t    de;
        if (srccp < -1 || srccp > DSCP_CODE_POINT_MAX) {
            return BCM_E_PARAM;
        }
        /* Extract cng bit and check for valid priority. */
        cng = 0;
        if (prio < 0) {
            prio = PORT(unit, port).p_ut_prio;
        }
        if (prio & BCM_PRIO_DROP_FIRST) {
            cng = 1;
            prio &= ~BCM_PRIO_DROP_FIRST;
        }
        if ((prio & ~BCM_PRIO_MASK) != 0) {
            return BCM_E_PARAM;
        }
        if (srccp < 0 && mapcp < 0) {
            /* No mapping */
            return BCM_E_NONE;
        } else if (srccp < 0) {
            /* Map all DSCPs to a new DSCP */
            /* fill all DSCP_CODE_POINT_CNT entries in DSCPm with mapcp */
            sal_memset(&de, 0, sizeof(de));
            soc_DSCPm_field32_set(unit, &de, DSCPf, mapcp);
            soc_DSCPm_field32_set(unit, &de, PRIf, prio);
            soc_DSCPm_field32_set(unit, &de, CNGf, cng); /* congestion */
            base = port * DSCP_CODE_POINT_CNT;
            for (i = 0; i < DSCP_CODE_POINT_CNT; i++) {
                SOC_IF_ERROR_RETURN
                    (WRITE_DSCPm(unit, MEM_BLOCK_ALL, base + i, &de));
            }
        } else {
            /* Map a specific DSCP to a new DSCP */
            sal_memset(&de, 0, sizeof(de));
            base = port * DSCP_CODE_POINT_CNT;

            /* fill specific srccp entry in DSCPm with mapcp */
            soc_DSCPm_field32_set(unit, &de, DSCPf, mapcp);
            soc_DSCPm_field32_set(unit, &de, PRIf, prio);
            soc_DSCPm_field32_set(unit, &de, CNGf, cng);

            SOC_IF_ERROR_RETURN
                (WRITE_DSCPm(unit, MEM_BLOCK_ALL, base + srccp, &de));
        }
        return BCM_E_NONE;
    }
#endif /* BCM_DRACO15_SUPPORT */

#if defined(BCM_TUCANA_SUPPORT)
    if (SOC_IS_TUCANA(unit)) {
        /*
         * DSCP -> DSCP mapping
         *      Map src Codepoint = 0 to a new value 
         *      Identity mapping
         */
        int cng = 0;
        if (srccp != mapcp) {
            if (srccp < -1 || srccp > 0) {
                return BCM_E_PARAM;
            }
        } else if (prio >= 0) {
            /* Extract cng bit and check for valid priority. */
            if (prio & BCM_PRIO_DROP_FIRST) {
                cng = 1;
                prio &= ~BCM_PRIO_DROP_FIRST;
            }
            if ((prio & ~BCM_PRIO_MASK) != 0) {
                return BCM_E_PARAM;
            }
        } else if (srccp > 0) {
            /* srccp == mapcd > 0 && prio < 0 */
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN
          (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));

        if ((mapcp >= 0) && (srccp == mapcp)) {
            if (prio >= 0) {
                /*
                 * Tucana can map DSCP to 802.1p priority. This mapping is
                 * derived from a chip-wide lookup table, although each port
                 * can be individually configured to use it.
                 *
                 * Set only the designated DSCP-to-priority lookup
                 * table entry (device global, irrespective of port).
                 * Leave the other table entries as they currently are.
                 *
                 * We'll also enable DSCP-to-priority mapping for this port.
                 */

                dscp_priority_table_entry_t dpe;

                SOC_IF_ERROR_RETURN
                  (READ_DSCP_PRIORITY_TABLEm(unit, MEM_BLOCK_ANY,
                                             srccp, &dpe));
                soc_DSCP_PRIORITY_TABLEm_field32_set(unit, &dpe,
                                                     PRIORITYf, prio);
                soc_DSCP_PRIORITY_TABLEm_field32_set(unit, &dpe,
                                                     DROP_PRECEDENCEf, cng);
                SOC_IF_ERROR_RETURN
                  (WRITE_DSCP_PRIORITY_TABLEm(unit, MEM_BLOCK_ALL,
                                              srccp, &dpe));
            } else {
                /* srccp = mapcp = 0 with invalid prio same as "no mapping" */
                srccp = mapcp = -1;
            }
        }

        /*
         * DSCP-to-DSCP mapping configuration if map zero (src dscp = 0)
         * or map all  ((src dscp = -1)
         * if srccp == mapcp adjust only the priority mapping
         * as arbitrary DSCP -> DSCP mapping not supported.
         */
        if ((mapcp >= 0) && (srccp <= 0)) {
            pcfg.pc_dscp = mapcp;
        }

        BCM_IF_ERROR_RETURN
            (mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg));

        return BCM_E_NONE;
    }
#endif /* BCM_TUCANA_SUPPORT */

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        int                  i, cng;
        dscp_table_entry_t   de;
        int                  max_index;   
        int                  min_index;

        if (srccp < -1 || srccp > DSCP_CODE_POINT_MAX) {
            return BCM_E_PARAM;
        }
        /* Extract cng bits and check for valid priority. */
        
        cng = 0; /* Green */
        if (prio < 0) {
            return BCM_E_PARAM;
        }
        if (prio & BCM_PRIO_RED) {
            cng = 0x01;  /* Red */
            prio &= ~BCM_PRIO_RED;
        } else if (prio & BCM_PRIO_YELLOW) {
            cng = 0x03;  /* Yellow  */
            prio &= ~BCM_PRIO_YELLOW;
        }
        if ((prio & ~BCM_PRIO_MASK) != 0) {
            return BCM_E_PARAM;
        }
        if (srccp < 0 && mapcp < 0) {
            /* No mapping */
            return BCM_E_NONE;
        } else if (srccp < 0) {
            /* Map all DSCPs to a new DSCP */
            /* fill all entries in DSCP_TABLEm with mapcp */
            sal_memset(&de, 0, sizeof(de));
            soc_DSCP_TABLEm_field32_set(unit, &de, DSCPf, mapcp);
            soc_DSCP_TABLEm_field32_set(unit, &de, PRIf, prio);
            soc_DSCP_TABLEm_field32_set(unit, &de, CNGf, cng);
            if (soc_feature(unit, soc_feature_dscp_map_per_port)) {
                min_index = port * DSCP_CODE_POINT_CNT;
                max_index = min_index + DSCP_CODE_POINT_MAX;
            } else {
                max_index = soc_mem_index_max(unit, DSCP_TABLEm);
                min_index = 0;
            }

            for (i = min_index; i <= max_index; i++) {
                SOC_IF_ERROR_RETURN
                    (WRITE_DSCP_TABLEm(unit, MEM_BLOCK_ALL, i, &de));
            }
        } else {
            int num_cosq;
            if (SOC_IS_EASYRIDER(unit)) {
                num_cosq = 8; /* Per-cosq */
            } else {
                num_cosq = 1; /* For all cosqs */
            }
            /* Map a specific DSCP to a new DSCP */
            sal_memset(&de, 0, sizeof(de));
            /* fill specific srccp entry/entries in DSCP_TABLEm with mapcp */
            soc_DSCP_TABLEm_field32_set(unit, &de, DSCPf, mapcp);
            soc_DSCP_TABLEm_field32_set(unit, &de, PRIf, prio);
            soc_DSCP_TABLEm_field32_set(unit, &de, CNGf, cng);

            if (soc_feature(unit, soc_feature_dscp_map_per_port)) {
                SOC_IF_ERROR_RETURN
                  (WRITE_DSCP_TABLEm(unit, MEM_BLOCK_ALL, 
                                     (port * DSCP_CODE_POINT_CNT) + srccp, &de));
            } else {
                for (i = 0; i < num_cosq ; i++) {
                    SOC_IF_ERROR_RETURN
                      (WRITE_DSCP_TABLEm(unit, MEM_BLOCK_ALL, 
                                         (i * DSCP_CODE_POINT_CNT) + srccp, &de));
                }
            }
        }
        return BCM_E_NONE;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    /* all other switches */
    if (srccp < -1 || srccp > 0) {
        return BCM_E_PARAM;
    }

    pcfg.pc_dse_mode = -1;
    BCM_IF_ERROR_RETURN
        (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
    if (pcfg.pc_dse_mode == -1) {       /* STRATA1 for example */
        return BCM_E_UNAVAIL;
    }

    if (mapcp >= 0) {
        pcfg.pc_dscp = mapcp;

        BCM_IF_ERROR_RETURN
            (mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_dscp_map_set
 * Purpose:
 *      Define a mapping for diffserv code points
 *      The mapping enable/disable is controlled by a seperate API
 *      bcm_port_dscp_map_enable_set/get
 *      Also Enable/Disable control for DSCP mapping for tunnels
 *      are available in the respective tunnel create APIs.
 * Parameters:
 *      unit - switch device
 *      port - switch port      or -1 to setup global mapping table.
 *      srccp - src code point or -1
 *      mapcp - mapped code point or -1
 *      prio - priority value for mapped code point
 *              -1 to use port default untagged priority
 *              BCM_PRIO_DROP_FIRST can be or'ed into the priority
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      On all strata2 and most xgs switches, there are a limited set
 *      of mappings that are possible:
 *              srccp -1, mapcp -1:     no mapping
 *              srccp -1, mapcp 0..63:  map all packets
 *              srccp 0, mapcp 0..63:   map packets with cp 0
 *      On Draco1.5 switches, there is a full per port mapping table
 *      for all 64 possible code points.
 *      On Firebolt/Helix/Felix, there is no per port mapping table.
 *      Only a global mapping table is available.
 */

int
bcm_esw_port_dscp_map_set(int unit, bcm_port_t port, int srccp, int mapcp,
                      int prio)
{
    int rv;
    bcm_port_config_t port_conf;
    bcm_pbmp_t pbmp;

    if (!soc_feature(unit, soc_feature_dscp)) {
        return BCM_E_UNAVAIL;
    }

    if (port != -1) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
        if (!soc_feature(unit, soc_feature_dscp_map_per_port)) {
            return BCM_E_PORT;
        }
    }

    PORT_LOCK(unit);

    /* Get device port configuration. */
    rv = bcm_esw_port_config_get(unit, &port_conf);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }
    BCM_PBMP_ASSIGN(pbmp, port_conf.e);
    if (SOC_IS_XGS3_SWITCH(unit) || SOC_IS_XGS3_FABRIC(unit)) {
        BCM_PBMP_OR(pbmp, port_conf.cpu);
    }

    if ((port == -1) && (soc_feature(unit, soc_feature_dscp_map_per_port))) {
        PBMP_ITER(pbmp, port) {
            rv = _bcm_port_dscp_map_set(unit, port, srccp, mapcp, prio);
            if (BCM_FAILURE(rv)) {
                PORT_UNLOCK(unit);
                return rv;
            }
        }
    } else {
        if (BCM_PBMP_MEMBER(pbmp, port) || (port == -1)) {
            rv = _bcm_port_dscp_map_set(unit, port, srccp, mapcp, prio);
        } else {
            rv = BCM_E_PORT;
        }
    }
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_port_dscp_map_get
 * Purpose:
 *      Get a mapping for diffserv code points
 * Parameters:
 *      unit - switch device
 *      port - switch port
 *      srccp - src code point or -1
 *      mapcp - (OUT) pointer to returned mapped code point
 *      prio - (OUT) Priority value for mapped code point or -1
 *                      May have BCM_PRIO_DROP_FIRST or'ed into it
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_port_dscp_map_get(int unit, bcm_port_t port, int srccp, int *mapcp,
                      int *prio)
{
    bcm_port_cfg_t      pcfg;

    if (srccp < -1 || srccp > DSCP_CODE_POINT_MAX || 
        mapcp == NULL || prio == NULL) {
        return BCM_E_PARAM;
    }

#if defined(BCM_DRACO15_SUPPORT)
    if (SOC_IS_DRACO15(unit)) {
        int             base;
        dscp_entry_t    de;
        /* look up in DSCPm */
        if (srccp < 0) {
            srccp = 0;
        }
        base = port * DSCP_CODE_POINT_CNT;
        SOC_IF_ERROR_RETURN
            (READ_DSCPm(unit, MEM_BLOCK_ANY, base + srccp, &de));
        *mapcp = soc_DSCPm_field32_get(unit, &de, DSCPf);
        *prio = soc_DSCPm_field32_get(unit, &de, PRIf);
        if (soc_DSCPm_field32_get(unit, &de, CNGf)) {
            *prio |= BCM_PRIO_DROP_FIRST;
        }
        return BCM_E_NONE;
    }
#endif /* BCM_DRACO15_SUPPORT */

#if defined(BCM_TUCANA_SUPPORT)
    if (SOC_IS_TUCANA(unit)) {
        dscp_priority_table_entry_t dpe;

        BCM_IF_ERROR_RETURN
            (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
        *mapcp = pcfg.pc_dscp;

        /* Mode 2: look up mapped priority in DSCP_PRIORITY_TABLEm */
        if (srccp < 0) {
            srccp = 0;
        }
        SOC_IF_ERROR_RETURN
            (READ_DSCP_PRIORITY_TABLEm(unit, MEM_BLOCK_ANY,
                                       srccp, &dpe));
        *prio = soc_DSCP_PRIORITY_TABLEm_field32_get(unit, &dpe,
                                                     PRIORITYf);
        if (soc_DSCP_PRIORITY_TABLEm_field32_get(unit, &dpe,
                                                 DROP_PRECEDENCEf)) {
            *prio |= BCM_PRIO_DROP_FIRST;
        }
        return BCM_E_NONE;
    }
#endif /* BCM_TUCANA_SUPPORT */

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (SOC_IS_XGS3_SWITCH(unit)) {
        int                     base;
        dscp_table_entry_t      de;

        /* look up in DSCP_TABLEm */
        if (srccp < 0) {
            srccp = 0;
        }
        if (soc_feature(unit, soc_feature_dscp_map_per_port)) {
            base = (port * DSCP_CODE_POINT_CNT);
        } else {
            base = 0;
        }
        SOC_IF_ERROR_RETURN
            (READ_DSCP_TABLEm(unit, MEM_BLOCK_ANY, base + srccp, &de));
        *mapcp = soc_DSCP_TABLEm_field32_get(unit, &de, DSCPf);
        *prio = soc_DSCP_TABLEm_field32_get(unit, &de, PRIf);
        if (soc_DSCP_TABLEm_field32_get(unit, &de, CNGf)) {
            
            *prio |= BCM_PRIO_DROP_FIRST;
        }

        return BCM_E_NONE;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    /* all other switches */

    pcfg.pc_dse_mode = -1;
    BCM_IF_ERROR_RETURN
        (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
    if (pcfg.pc_dse_mode == -1) {       /* STRATA1 for example */
        return BCM_E_UNAVAIL;
    }

    /*
     * The mapping table is set independently of the mapping mode.
     * So the _get API will return the mapping table setup and not
     * necessarily the actual mapping that will be applied to the packet.
     * The mapping applied to the packet is determined by mapping mode
     * set using bcm_port_dscp_map_mode_set
     */
    *mapcp = pcfg.pc_dscp;

    *prio = -1;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_dscp_map_get
 * Purpose:
 *      Get a mapping for diffserv code points
 * Parameters:
 *      unit - switch device
 *      port - switch port
 *      srccp - src code point or -1
 *      mapcp - (OUT) pointer to returned mapped code point
 *      prio - (OUT) Priority value for mapped code point or -1
 *                      May have BCM_PRIO_DROP_FIRST or'ed into it
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_dscp_map_get(int unit, bcm_port_t port, int srccp, int *mapcp,
                      int *prio)
{
    int rv;
    bcm_port_config_t port_conf;
    bcm_pbmp_t pbmp;

    if (!soc_feature(unit, soc_feature_dscp)) {
        return BCM_E_UNAVAIL;
    }

    if (port != -1) {
        if (0 == soc_feature(unit, soc_feature_dscp_map_per_port)) {
            return BCM_E_PORT;
        }
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    }

    PORT_LOCK(unit);

    /* Get device port configuration. */
    rv = bcm_esw_port_config_get(unit, &port_conf);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }
    BCM_PBMP_ASSIGN(pbmp, port_conf.e);
    if (SOC_IS_XGS3_SWITCH(unit) || SOC_IS_XGS3_FABRIC(unit)) {
        BCM_PBMP_OR(pbmp, port_conf.cpu);
    }

    if (port == -1) {
        PBMP_ITER(pbmp, port) {
            break;
        }
    }

    if (SOC_PORT_VALID(unit, port) && BCM_PBMP_MEMBER(pbmp, port)) {
        rv = _bcm_esw_port_dscp_map_get(unit, port, srccp, mapcp, prio);
    } else {
        rv = BCM_E_PORT;
    }

    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_port_dscp_unmap_set
 * Purpose:
 *      Internal implementation for bcm_port_dscp_unmap_set
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_port_dscp_unmap_set(int unit, bcm_port_t port, int internal_pri, 
                         bcm_color_t color, int pkt_dscp)
{
    int index, port_shift = 6;
    egr_dscp_table_entry_t  dscp_unmap;
    if (pkt_dscp < 0 || pkt_dscp > DSCP_CODE_POINT_MAX) {
        return BCM_E_PARAM;
    }
    if ((internal_pri < 0) || (internal_pri > 15) ||
        ((color != bcmColorGreen) && (color != bcmColorYellow) &&
         (color != bcmColorRed))) {
        return BCM_E_PARAM;
    }

    /* EGR_DSCP_TABLE table is indexed with
     * port[5:0] priority[3:0] CNG [1:0]
     */
    index = (port << port_shift) | (internal_pri << 2) | 
             _BCM_COLOR_ENCODING(unit, color);

    sal_memset(&dscp_unmap, 0, sizeof(dscp_unmap));
    soc_mem_field32_set(unit, EGR_DSCP_TABLEm, &dscp_unmap, DSCPf, pkt_dscp);
    SOC_IF_ERROR_RETURN
        (WRITE_EGR_DSCP_TABLEm(unit, MEM_BLOCK_ALL, index, &dscp_unmap));
    return BCM_E_NONE;

}

/*
 * Function:
 *      bcm_port_dscp_unmap_set
 * Purpose:
 *      Define a mapping for diffserv code points
 *      The mapping enable/disable is controlled by a seperate API
 *      bcm_port_dscp_map_enable_set/get
 *      Also Enable/Disable control for DSCP mapping for tunnels
 *      are available in the respective tunnel create APIs.
 * Parameters:
 *      unit - switch device
 *      port - switch port      or -1 to setup global mapping table.
 *      internal_pri - internal priority
 *      color - Red/Yellow/Green
 *      pkt_dscp - DSCP marking on outgoing packet
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_port_dscp_unmap_set(int unit, bcm_port_t port, int internal_pri, 
                            bcm_color_t color, int pkt_dscp)
{
    int rv;
    bcm_port_config_t port_conf;
    bcm_pbmp_t pbmp;

    if (!soc_feature(unit, soc_feature_dscp) || 
        !soc_feature(unit, soc_feature_egr_dscp_map_per_port)) {
        return BCM_E_UNAVAIL;
    }

    if (port != -1) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
        if (IS_HG_PORT(unit, port) || IS_ST_PORT(unit, port)) {
            return BCM_E_PORT;
        }
    }

    PORT_LOCK(unit);

    /* Get device port configuration. */
    rv = bcm_esw_port_config_get(unit, &port_conf);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }
    BCM_PBMP_ASSIGN(pbmp, port_conf.e);
    BCM_PBMP_OR(pbmp, port_conf.cpu);

    if (port == -1) {
        PBMP_ITER(pbmp, port) {
            rv = _bcm_port_dscp_unmap_set(unit, port, internal_pri, color, 
                                          pkt_dscp);
            if (BCM_FAILURE(rv)) {
                PORT_UNLOCK(unit);
                return rv;
            }
        }
    } else if (SOC_PORT_VALID(unit, port) && BCM_PBMP_MEMBER(pbmp, port)) {
        rv = _bcm_port_dscp_unmap_set(unit, port, internal_pri, color, 
                                      pkt_dscp);
    } else {
        rv = BCM_E_PORT;
    }
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_port_dscp_unmap_get
 * Purpose:
 *      Internal implementation for bcm_port_dscp_unmap_get
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_port_dscp_unmap_get(int unit, bcm_port_t port, int internal_pri, 
                         bcm_color_t color, int *pkt_dscp)
{
    int index, port_shift = 6;
    egr_dscp_table_entry_t  dscp_unmap;

    if ((internal_pri < 0) || (internal_pri > 15) || 
        ((color != bcmColorGreen) && (color != bcmColorYellow) &&
         (color != bcmColorRed))) {
        return BCM_E_PARAM;
    }

    /* EGR_DSCP_TABLE table is indexed with
     * port[5:0] priority[3:0] CNG [1:0]
     */
    index = (port << port_shift) | (internal_pri << 2) | 
             _BCM_COLOR_ENCODING(unit, color);


    SOC_IF_ERROR_RETURN
        (READ_EGR_DSCP_TABLEm(unit, MEM_BLOCK_ANY, index, &dscp_unmap));
    *pkt_dscp = soc_mem_field32_get(unit, EGR_DSCP_TABLEm, 
                                    &dscp_unmap, DSCPf);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_dscp_unmap_get
 * Purpose:
 *      Retrieve a mapping for diffserv code points
 *      The mapping enable/disable is controlled by a seperate API
 *      bcm_port_dscp_map_enable_set/get
 *      Also Enable/Disable control for DSCP mapping for tunnels
 *      are available in the respective tunnel create APIs.
 * Parameters:
 *      unit - switch device
 *      port - switch port.
 *      internal_pri - internal priority
 *      color - Red/Yellow/Green
 *      pkt_dscp - DSCP marking on outgoing packet
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_port_dscp_unmap_get(int unit, bcm_port_t port, int internal_pri, 
                            bcm_color_t color, int *pkt_dscp)
{
    int rv;
    bcm_port_config_t port_conf;
    bcm_pbmp_t pbmp;

    if (!soc_feature(unit, soc_feature_dscp) || 
        !soc_feature(unit, soc_feature_egr_dscp_map_per_port)) {
        return BCM_E_UNAVAIL;
    }

    if (port != -1) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
        if (IS_HG_PORT(unit, port) || IS_ST_PORT(unit, port)) {
            return BCM_E_PORT;
        }
    }

    PORT_LOCK(unit);

    /* Get device port configuration. */
    rv = bcm_esw_port_config_get(unit, &port_conf);
    if (BCM_FAILURE(rv)) {
        PORT_UNLOCK(unit);
        return rv;
    }
    BCM_PBMP_ASSIGN(pbmp, port_conf.e);
    BCM_PBMP_OR(pbmp, port_conf.cpu);

    if (port == -1) {
        PBMP_ITER(pbmp, port) {
            break;
        }
    }

    if (SOC_PORT_VALID(unit, port) && BCM_PBMP_MEMBER(pbmp, port)) {
        rv = _bcm_port_dscp_unmap_get(unit, port, internal_pri, color, 
                                      pkt_dscp);
    } else {
        rv = BCM_E_PORT;
    }

    PORT_UNLOCK(unit);
    return rv;
}

#undef DSCP_CODE_POINT_MAX
#undef DSCP_CODE_POINT_CNT

STATIC int 
_bcm_esw_port_modid_egress_resolve(int unit, _bcm_port_egr_dest_t *egr_dst)
{
    int             port_adjst = 0, mod_adjust = 0;
    int             isLocal;
    bcm_module_t    my_mod, modid;

    if (BCM_GPORT_IS_SET(egr_dst->in_port)) {
        bcm_trunk_t     tid;
        int             id;
        bcm_port_t      port;

        BCM_IF_ERROR_RETURN(
            _bcm_esw_gport_resolve(unit, egr_dst->in_port, &modid, &port, 
                                   &tid, &id));
        if ((-1 != id) || (-1 != tid)) {
            return BCM_E_PORT;
        }
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_mod));
        BCM_IF_ERROR_RETURN(
            _bcm_esw_modid_is_local(unit, modid, &isLocal));

        if ((TRUE == isLocal) && (modid != my_mod)){
            port_adjst = 32;
            mod_adjust = 1;
        }
        egr_dst->out_min_port = egr_dst->out_max_port = port + port_adjst;
        egr_dst->out_min_modid = egr_dst->out_max_modid = modid - mod_adjust;
    } else {
        if (egr_dst->in_modid < 0) {
            egr_dst->out_min_modid = 0;
            egr_dst->out_max_modid = SOC_MODID_MAX(unit);
        } else if (!SOC_MODID_ADDRESSABLE(unit, egr_dst->in_modid)) {
            return BCM_E_PARAM;
        } else {
            egr_dst->out_min_modid = egr_dst->out_max_modid = egr_dst->in_modid;
        }

        if (egr_dst->in_port < 0) {
            egr_dst->out_min_port = 0;
            egr_dst->out_max_port = SOC_PORT_MAX(unit, all);
        } else if (!SOC_PORT_ADDRESSABLE(unit, egr_dst->in_port)) {
            return BCM_E_PORT;
        } else {
            egr_dst->out_min_port = egr_dst->out_max_port = egr_dst->in_port; 
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_port_egress_set
 * Description:
 *      Set switching only to indicated ports from given (modid, port).
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *      pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 * Note:
 *      if port < 0, means all/any port
 *      if modid < 0, means all/any modid
 */
int
bcm_esw_port_egress_set(int unit, bcm_port_t port, int modid, bcm_pbmp_t pbmp)
{
    egr_mask_entry_t        em_entry;
    int                     em_index;
    bcm_module_t            cur_mod; 
    bcm_pbmp_t              em_pbmp;
    bcm_port_t              cur_port;
    int                     rv = BCM_E_NONE;
    _bcm_port_egr_dest_t    egr_dst;

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        return bcm_td_port_egress_set(unit, port, modid, pbmp);
    }
#endif /* BCM_TRIDENT_SUPPORT */

    if (!SOC_IS_XGS_SWITCH(unit)) {
        if (BCM_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
            return BCM_E_NONE;
        } else {
            return BCM_E_UNAVAIL;
        }
    }

    sal_memset(&em_entry, 0, sizeof(egr_mask_entry_t));

    BCM_PBMP_NEGATE(em_pbmp, pbmp);
    BCM_PBMP_AND(em_pbmp, PBMP_ALL(unit));
    BCM_PBMP_REMOVE(em_pbmp, PBMP_LB(unit));
    if (SOC_IS_TUCANA(unit)) {
        soc_EGR_MASKm_field32_set(unit, &em_entry, EGRESS_MASK_M0f,
                                  SOC_PBMP_WORD_GET(em_pbmp, 0));
        soc_EGR_MASKm_field32_set(unit, &em_entry, EGRESS_MASK_M1f,
                                  SOC_PBMP_WORD_GET(em_pbmp, 1));
    } else {
        soc_mem_pbmp_field_set(unit, EGR_MASKm, &em_entry, EGRESS_MASKf,
                               &em_pbmp);
    }
    egr_dst.in_modid = modid;
    egr_dst.in_port = port;

        BCM_IF_ERROR_RETURN(
        _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));
    
    soc_mem_lock(unit, EGR_MASKm);
    for (cur_mod = egr_dst.out_min_modid; cur_mod <= egr_dst.out_max_modid; cur_mod++) {
        for (cur_port = egr_dst.out_min_port; cur_port <= egr_dst.out_max_port; cur_port++) {
                em_index = (cur_mod & SOC_MODID_MAX(unit)) *
                           (SOC_PORT_ADDR_MAX(unit) + 1) |
                           (cur_port & SOC_PORT_ADDR_MAX(unit));

#if defined(BCM_DRACO15_SUPPORT)
            if (SOC_IS_DRACO15(unit)) {
                /* Preserve EGR_MASK T/TGID fields. */
                rv = soc_mem_read(unit, EGR_MASKm, MEM_BLOCK_ANY,
                                  em_index, &em_entry);
                if (!BCM_SUCCESS(rv)) {
                    break;
                }
                soc_EGR_MASKm_field32_set(unit, &em_entry, EGRESS_MASKf,
                                          SOC_PBMP_WORD_GET(em_pbmp, 0));
            }
#endif /* BCM_DRACO15_SUPPORT */
            rv = soc_mem_write(unit, EGR_MASKm, MEM_BLOCK_ALL,
                               em_index, &em_entry);
            if (!BCM_SUCCESS(rv)) {
                break;
            }
        }
        if (!BCM_SUCCESS(rv)) {
            break;
        }
    }
    soc_mem_unlock(unit, EGR_MASKm);

    return rv;
}

/*
 * Function:
 *      bcm_port_egress_get
 * Description:
 *      Retrieve bitmap of ports for which switching is enabled
 *      for (modid, port).
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *      pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_port_egress_get(int unit, bcm_port_t port, int modid, bcm_pbmp_t *pbmp)
{
    egr_mask_entry_t    em_entry;
    int                 em_index;
    bcm_pbmp_t          em_pbmp, temp_pbmp;
    _bcm_port_egr_dest_t    egr_dst;

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        return bcm_td_port_egress_get(unit, port, modid, pbmp);
    }
#endif /* BCM_TRIDENT_SUPPORT */

    if (!SOC_IS_XGS_SWITCH(unit)) {
        BCM_PBMP_ASSIGN(*pbmp, PBMP_ALL(unit));
        return BCM_E_NONE;
    }

    if ((modid < 0) || (port < 0) ){
            return BCM_E_PARAM;
        }

    egr_dst.in_modid = modid;
    egr_dst.in_port = port;

            BCM_IF_ERROR_RETURN(
        _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        em_index =
        (egr_dst.out_min_modid & SOC_MODID_MAX(unit)) * 
        (SOC_PORT_ADDR_MAX(unit) + 1) | (egr_dst.out_min_port 
                                         & SOC_PORT_ADDR_MAX(unit));

    BCM_IF_ERROR_RETURN
        (READ_EGR_MASKm(unit, MEM_BLOCK_ALL, em_index, &em_entry));

    BCM_PBMP_CLEAR(em_pbmp);
    if (SOC_IS_TUCANA(unit)) {
        SOC_PBMP_WORD_SET(em_pbmp, 0,
                          soc_EGR_MASKm_field32_get(unit, &em_entry,
                                                    EGRESS_MASK_M0f));
        SOC_PBMP_WORD_SET(em_pbmp, 1,
                          soc_EGR_MASKm_field32_get(unit, &em_entry,
                                                    EGRESS_MASK_M1f));
    } else {
        soc_mem_pbmp_field_get(unit, EGR_MASKm, &em_entry, EGRESS_MASKf,
                               &em_pbmp);
    }
    BCM_PBMP_NEGATE(temp_pbmp, em_pbmp);
    BCM_PBMP_AND(temp_pbmp, PBMP_ALL(unit));
    BCM_PBMP_REMOVE(temp_pbmp, PBMP_LB(unit));
    BCM_PBMP_ASSIGN(*pbmp, temp_pbmp);

    return BCM_E_NONE;
}

#ifdef BCM_TRIUMPH2_SUPPORT
STATIC int
_bcm_port_src_mod_egress_profile_remove(int unit, bcm_port_t port, 
                                        int min_mod, int max_mod)
{
    int idx, i, region, modid_cnt;
    uint32 val;
    src_modid_egress_entry_t srcmodegr;

    BCM_IF_ERROR_RETURN(READ_SRC_MODID_EGRESS_SELr(unit, port, &val));
    soc_reg_field_set(unit, SRC_MODID_EGRESS_SELr, &val, ENABLEf, 0); 
    soc_reg_field_set(unit, SRC_MODID_EGRESS_SELr, &val, SRCMOD_INDEXf, 0);
    BCM_IF_ERROR_RETURN(WRITE_SRC_MODID_EGRESS_SELr(unit, port, val));

    /* Null out the egress pbmp from the profile region */
    region = PORT_SRC_MOD_EGR_PROF_PTR(unit, port);
    SRC_MOD_EGR_REF_COUNT(unit, region)--;
    modid_cnt = SOC_MODID_MAX(unit) + 1;
    PORT_SRC_MOD_EGR_PROF_PTR(unit, port) = -1;

    if (SRC_MOD_EGR_REF_COUNT(unit, region) == 0) {
        for (i = min_mod; i <= max_mod; i++) {
            idx = region * modid_cnt + i;
            sal_memset(&srcmodegr, 0, sizeof(src_modid_egress_entry_t));
            BCM_IF_ERROR_RETURN(
                WRITE_SRC_MODID_EGRESSm(unit, MEM_BLOCK_ALL, idx, &srcmodegr));
        }
    }
    return BCM_E_NONE;
}

STATIC int
_bcm_port_src_mod_egress_profile_add(int unit, bcm_port_t port, 
                                     int min_mod, int max_mod, pbmp_t pbmp)
{
    int idx, i, j, num_regions, region, modid_cnt, match, rv = BCM_E_NONE;
    uint32 val;
    src_modid_egress_entry_t srcmodegr;
    pbmp_t old_pbmp;

    modid_cnt = SOC_MODID_MAX(unit) + 1;
    region = PORT_SRC_MOD_EGR_PROF_PTR(unit, port);
    if (region >= 0) {
        /* Profile for this port already exists - update it */
        for (i = min_mod; i <= max_mod; i++) {
            idx = region * modid_cnt + i;
            sal_memset(&srcmodegr, 0, sizeof(src_modid_egress_entry_t));
            soc_mem_pbmp_field_set(unit, SRC_MODID_EGRESSm, &srcmodegr,
                                   PORT_BLOCK_MASK_BITMAPf, &pbmp);
            BCM_IF_ERROR_RETURN(
                WRITE_SRC_MODID_EGRESSm(unit, MEM_BLOCK_ALL, idx, &srcmodegr));
        }

    } else {
        /* Allocate a new / find existing profile */
        num_regions = soc_mem_index_count(unit, SRC_MODID_EGRESSm) / 
                      SOC_MODID_MAX(unit);
        for (j = 0; j < num_regions; j++) {
            match = 1;
            for (i = min_mod; i <= max_mod; i++) {
                BCM_IF_ERROR_RETURN(READ_SRC_MODID_EGRESSm(unit, MEM_BLOCK_ANY, 
                                                           j * modid_cnt + i, 
                                                           &srcmodegr));
                soc_mem_pbmp_field_get(unit, SRC_MODID_EGRESSm, &srcmodegr,
                                       PORT_BLOCK_MASK_BITMAPf, &old_pbmp);
                if (BCM_PBMP_NEQ(old_pbmp, pbmp)) {
                    match = 0;
                    break;
                }                            
            }
            if (match) {
                BCM_IF_ERROR_RETURN(READ_SRC_MODID_EGRESS_SELr(unit, port, &val));
                soc_reg_field_set(unit, SRC_MODID_EGRESS_SELr, &val, ENABLEf, 1); 
                soc_reg_field_set(unit, SRC_MODID_EGRESS_SELr, &val, SRCMOD_INDEXf, j);
                BCM_IF_ERROR_RETURN(WRITE_SRC_MODID_EGRESS_SELr(unit, port, val));

                PORT_SRC_MOD_EGR_PROF_PTR(unit, port) = j;
                SRC_MOD_EGR_REF_COUNT(unit, j)++;
                break;
            }
            if (SRC_MOD_EGR_REF_COUNT(unit, j) == 0) {
                sal_memset(&srcmodegr, 0, sizeof(src_modid_egress_entry_t));
                soc_mem_pbmp_field_set(unit, SRC_MODID_EGRESSm, &srcmodegr,
                                       PORT_BLOCK_MASK_BITMAPf, &pbmp);
                for (i = min_mod; i <= max_mod; i++) {
                    BCM_IF_ERROR_RETURN
                        (WRITE_SRC_MODID_EGRESSm(unit, MEM_BLOCK_ALL, j *   
                                                 modid_cnt + i, &srcmodegr));
                }
                BCM_IF_ERROR_RETURN(READ_SRC_MODID_EGRESS_SELr(unit, port, &val));
                soc_reg_field_set(unit, SRC_MODID_EGRESS_SELr, &val, ENABLEf, 1); 
                soc_reg_field_set(unit, SRC_MODID_EGRESS_SELr, &val, SRCMOD_INDEXf, j);
                BCM_IF_ERROR_RETURN(WRITE_SRC_MODID_EGRESS_SELr(unit, port, val));

                PORT_SRC_MOD_EGR_PROF_PTR(unit, port) = j;
                SRC_MOD_EGR_REF_COUNT(unit, j)++;
                break;
            }
        }
        if (j == num_regions) {
            rv = BCM_E_RESOURCE;
        }
    }
    return rv;
}
#endif

/*
 * Function:
 *      bcm_port_modid_egress_set
 * Description:
 *      Set port bitmap of egress ports on which the incoming packets
 *      will be forwarded.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *      pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 * Note:
 *      if port < 0, means all/any port
 *      if modid < 0, means all/any modid
 */
int
bcm_esw_port_modid_egress_set(int unit, bcm_port_t port,
                              bcm_module_t modid, bcm_pbmp_t pbmp)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_UNAVAIL;

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_ing_srcmodblk_entry_t srcmod;
        uint32 smf;
        bcm_module_t mod_idx, min_mod, max_mod;
        bcm_pbmp_t mod_pbmp;
        int bk=-1, blk;

        if (port >= 0) {
            if (!SOC_PORT_VALID(unit, port) || !IS_HG_PORT(unit, port)) {
                return BCM_E_PORT;
            }
            bk = SOC_PORT_BLOCK(unit, port);
        }

        if (modid < 0) {
            min_mod = soc_mem_index_min(unit, MEM_ING_SRCMODBLKm);
            max_mod = soc_mem_index_max(unit, MEM_ING_SRCMODBLKm);
        } else if (modid > soc_mem_index_max(unit, MEM_ING_SRCMODBLKm)) {
            return BCM_E_PARAM;
        } else {
            min_mod = max_mod = modid;
        }

        BCM_PBMP_ASSIGN(mod_pbmp, pbmp);
        BCM_PBMP_AND(mod_pbmp, PBMP_ALL(unit));
        if (BCM_PBMP_NEQ(pbmp, mod_pbmp)) {
            return BCM_E_PARAM;
        }

        soc_mem_lock(unit, MEM_ING_SRCMODBLKm);
        SOC_MEM_BLOCK_ITER(unit, MEM_ING_SRCMODBLKm, blk) {
            if ((port >= 0) && (bk !=blk)) {
                continue;
            }
            for (mod_idx = min_mod; mod_idx <= max_mod; mod_idx++) {
                 sal_memset(&srcmod, 0, sizeof(srcmod));
                 rv = READ_MEM_ING_SRCMODBLKm(unit, blk, mod_idx, &srcmod);
                 if (rv >= 0) {
                     SOC_PBMP_WORD_SET(mod_pbmp, 0,
                         soc_MEM_ING_SRCMODBLKm_field32_get(unit, &srcmod, BITMAPf));
                     if (BCM_PBMP_NEQ(pbmp, mod_pbmp)) {
                         smf = SOC_PBMP_WORD_GET(pbmp, 0);
                         soc_MEM_ING_SRCMODBLKm_field_set(unit, &srcmod, BITMAPf, &smf);
                         rv = WRITE_MEM_ING_SRCMODBLKm(unit, blk, mod_idx, &srcmod);
                     }
                 }
            }
        }
        soc_mem_unlock(unit, MEM_ING_SRCMODBLKm);
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
        SOC_IS_KATANA(unit)) {
        src_modid_ingress_block_entry_t srcmoding;
        _bcm_port_egr_dest_t egr_dst;
        int all_st_ports;
        int modid_cnt, i;
        bcm_port_t p;
        pbmp_t mod_pbmp;

        /* Number of supported MODIDs for this unit */
        modid_cnt = SOC_MODID_MAX(unit) + 1;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_min_port == egr_dst.out_max_port) {
            all_st_ports = 0;
        } else {
            all_st_ports = 1;
        }

        /* First program the ingress blocking */
        soc_mem_lock(unit, SRC_MODID_INGRESS_BLOCKm);
        for (i = egr_dst.out_min_modid; i <= egr_dst.out_max_modid; i++) { 
            rv = READ_SRC_MODID_INGRESS_BLOCKm(unit, MEM_BLOCK_ANY, i, 
                                               &srcmoding);
            if (BCM_FAILURE(rv)) {
                soc_mem_unlock(unit, SRC_MODID_INGRESS_BLOCKm);
                return rv;
            }
            if (!all_st_ports) {
                soc_mem_pbmp_field_get(unit, SRC_MODID_INGRESS_BLOCKm,
                                       &srcmoding, PORT_BITMAPf, &mod_pbmp);
                BCM_PBMP_PORT_REMOVE(mod_pbmp, egr_dst.out_min_port);
            } else {
                BCM_PBMP_ASSIGN(mod_pbmp, PBMP_ALL(unit));
                BCM_PBMP_OR(mod_pbmp, PBMP_LB(unit));
                PBMP_ST_ITER(unit, p) {
                    BCM_PBMP_PORT_REMOVE(mod_pbmp, p);
                }
            }
            soc_mem_pbmp_field_set(unit, SRC_MODID_INGRESS_BLOCKm,
                                   &srcmoding, PORT_BITMAPf, &mod_pbmp);
            rv = WRITE_SRC_MODID_INGRESS_BLOCKm(unit, MEM_BLOCK_ALL, i, 
                                                &srcmoding);
            if (BCM_FAILURE(rv)) {
                soc_mem_unlock(unit, SRC_MODID_INGRESS_BLOCKm);
                return rv;
            }
        } 
        soc_mem_unlock(unit, SRC_MODID_INGRESS_BLOCKm);

        /* Deal with the egress blocking - use one of 8 profiles */
        BCM_PBMP_ASSIGN(mod_pbmp, pbmp);
        BCM_PBMP_AND(mod_pbmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(mod_pbmp, PBMP_LB(unit));
        if (BCM_PBMP_NEQ(pbmp, mod_pbmp)) {
            return BCM_E_PARAM;
        }

        BCM_PBMP_NEGATE(mod_pbmp, pbmp);
        BCM_PBMP_AND(mod_pbmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(mod_pbmp, PBMP_LB(unit));

        soc_mem_lock(unit, SRC_MODID_EGRESSm);
        if (!all_st_ports) {
            /* Deal with a single stack port and its profile */
            if (BCM_PBMP_IS_NULL(mod_pbmp)) {
                rv = _bcm_port_src_mod_egress_profile_remove(unit, 
                                                      egr_dst.out_min_port, 
                                                      egr_dst.out_min_modid, 
                                                      egr_dst.out_max_modid);
                if (BCM_FAILURE(rv)) {
                    soc_mem_unlock(unit, SRC_MODID_EGRESSm);
                    return rv;
                }
            } else {
                rv = _bcm_port_src_mod_egress_profile_add(unit,
                                                       egr_dst.out_min_port,
                                                       egr_dst.out_min_modid, 
                                                       egr_dst.out_max_modid,
                                                       mod_pbmp);
                if (BCM_FAILURE(rv)) {
                    soc_mem_unlock(unit, SRC_MODID_EGRESSm);
                    return rv;
                }
            }
        } else {
            /* Deal with all stack ports and their profiles */
            PBMP_ST_ITER(unit, p) {
                if (BCM_PBMP_IS_NULL(mod_pbmp)) {
                    rv = _bcm_port_src_mod_egress_profile_remove
                             (unit, p, egr_dst.out_min_modid, egr_dst.out_max_modid);
                    if (BCM_FAILURE(rv)) {
                        soc_mem_unlock(unit, SRC_MODID_EGRESSm);
                        return rv;
                    }
                } else {
                    rv = _bcm_port_src_mod_egress_profile_add
                             (unit, p, egr_dst.out_min_modid, egr_dst.out_max_modid, mod_pbmp);
                    if (BCM_FAILURE(rv)) {
                        soc_mem_unlock(unit, SRC_MODID_EGRESSm);
                        return rv;
                    }
                }
            }
        }
        soc_mem_unlock(unit, SRC_MODID_EGRESSm);

        return rv;
    }
#endif

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        src_modid_block_entry_t srcmod;
        int modid_cnt, idx, i, j;
        bcm_pbmp_t mod_pbmp;
        _bcm_port_egr_dest_t    egr_dst;

        /* Number of supported MODIDs for this unit */
        modid_cnt = SOC_MODID_MAX(unit) + 1;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_min_port == egr_dst.out_max_port) {
            SOC_IF_ERROR_RETURN(
                soc_xgs3_port_num_to_higig_port_num(unit, egr_dst.out_max_port,
                                                    &egr_dst.out_min_port));
            egr_dst.out_max_port = egr_dst.out_min_port;
        } else {
            egr_dst.out_min_port = soc_mem_index_min(unit, SRC_MODID_BLOCKm) / modid_cnt;
            egr_dst.out_max_port = soc_mem_index_max(unit, SRC_MODID_BLOCKm) / modid_cnt;
        }

        BCM_PBMP_ASSIGN(mod_pbmp, pbmp);
        BCM_PBMP_AND(mod_pbmp, PBMP_ALL(unit));
        if (BCM_PBMP_NEQ(pbmp, mod_pbmp)) {
            return BCM_E_PARAM;
        }

        BCM_PBMP_NEGATE(mod_pbmp, pbmp);
        BCM_PBMP_AND(mod_pbmp, PBMP_ALL(unit));

        soc_mem_lock(unit, SRC_MODID_BLOCKm);
        for (i = egr_dst.out_min_port; i <= egr_dst.out_max_port; i++) {
            for (j = egr_dst.out_min_modid; j <= egr_dst.out_max_modid; j++) {
                idx = i * modid_cnt + j;
                rv = READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, idx, &srcmod);
                if (rv >= 0) {
                    soc_SRC_MODID_BLOCKm_field_set(unit, &srcmod,
                                                   PORT_BLOCK_MASK_BITMAPf,
                                                   (uint32 *)&mod_pbmp);
                    rv = WRITE_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ALL, idx,
                                                &srcmod);
                }
                if (rv < 0) {
                    soc_mem_unlock(unit, SRC_MODID_BLOCKm);
                    return rv;
                }
            }
        }
        soc_mem_unlock(unit, SRC_MODID_BLOCKm);
        return rv;
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (soc_feature(unit, soc_feature_src_modid_blk)) {
        src_modid_block_entry_t srcmod;
        bcm_module_t mod_idx;
        int p_idx, min_p, max_p;
        int tbl_size, modid_cnt, idx;
        bcm_pbmp_t mod_pbmp, tst_pbmp, hg_pbmp;
        int cpu=0, ether=0;
        uint32 xge_val;
        _bcm_port_egr_dest_t    egr_dst; 
#if defined(BCM_RAPTOR_SUPPORT)
        uint32  rp_val;
#endif /* BCM_RAPTOR_SUPPORT */
        /* Number of supported MODIDs for this unit */
        modid_cnt = SOC_MODID_MAX(unit) + 1;


        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_min_port == egr_dst.out_min_port) {
            min_p = max_p = egr_dst.out_min_port - SOC_HG_OFFSET(unit);
        } else {
            min_p = 0;
            tbl_size = soc_mem_index_count(unit, SRC_MODID_BLOCKm);
            max_p = (tbl_size / modid_cnt) - 1;
            assert(max_p >= 0);
        }

        BCM_PBMP_ASSIGN(mod_pbmp, pbmp);
        BCM_PBMP_AND(mod_pbmp, PBMP_ALL(unit));
        if (BCM_PBMP_NEQ(pbmp, mod_pbmp)) {
            return BCM_E_PARAM;
        }
    
        BCM_PBMP_NEGATE(mod_pbmp, pbmp);
        BCM_PBMP_AND(mod_pbmp, PBMP_ALL(unit));

        BCM_PBMP_ASSIGN(tst_pbmp, mod_pbmp);
        BCM_PBMP_AND(tst_pbmp, PBMP_CMIC(unit));
        if (BCM_PBMP_NOT_NULL(tst_pbmp)) {
            cpu = 1;
        }

        BCM_PBMP_ASSIGN(tst_pbmp, mod_pbmp);
        BCM_PBMP_AND(tst_pbmp, PBMP_E_ALL(unit));
        if (BCM_PBMP_NOT_NULL(tst_pbmp)) {
            ether = 1;
            BCM_PBMP_ASSIGN(tst_pbmp, PBMP_XE_ALL(unit));
        }

        BCM_PBMP_ASSIGN(hg_pbmp, mod_pbmp);
        BCM_PBMP_AND(hg_pbmp, PBMP_ST_ALL(unit));

        if (ether && BCM_PBMP_NOT_NULL(tst_pbmp)) {
            BCM_PBMP_OR(hg_pbmp, tst_pbmp);
        }
        xge_val = SOC_PBMP_WORD_GET(hg_pbmp, 0) >> SOC_HG_OFFSET(unit);

        /* In Raven, port #3 is the GMII port and it introduces a hole in the ST pbmp */ 
        soc_mem_lock(unit, SRC_MODID_BLOCKm);
        for (p_idx = min_p; p_idx <= max_p; p_idx++) {
            if (SOC_IS_RAVEN(unit) && p_idx == 3) {
                continue;
            }
            for (mod_idx = egr_dst.out_min_modid; mod_idx <= egr_dst.out_max_modid; mod_idx++) {
                 sal_memset(&srcmod, 0, sizeof(srcmod));
                 /* Table is 2-dimensional array */
                 if (SOC_IS_RAVEN(unit) && p_idx >= 4) {
                     idx = (p_idx - 1) * modid_cnt + mod_idx;
                 } else {
                     idx = (p_idx * modid_cnt) + mod_idx;
                 }
                 rv = READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, idx, &srcmod);
                 if (rv >= 0) {
                     soc_SRC_MODID_BLOCKm_field32_set(unit, &srcmod, 
                                                      CPU_PORT_BLOCK_MASKf, 
                                                      (cpu) ? 1 : 0);
                     if (SOC_IS_FB_FX_HX(unit)) {
                         soc_SRC_MODID_BLOCKm_field32_set(unit, &srcmod, 
                                                          GE_PORT_BLOCK_MASKf,
                                                          (ether) ? 1 : 0);
                     }
                     soc_SRC_MODID_BLOCKm_field32_set(unit, &srcmod, 
                                                      HIGIG_XGE_PORT_BLOCK_MASKf, 
                                                      xge_val);
#if defined(BCM_RAPTOR_SUPPORT)
                     if (soc_mem_field_valid(unit, SRC_MODID_BLOCKm, 
                                             PORT_0_5_BLOCK_MASKf)) {
                          rp_val = (SOC_PBMP_WORD_GET(mod_pbmp, 0)) & 
                              ((1 << soc_reg_field_length(unit, SRC_MODID_BLOCKm,
                                                   PORT_0_5_BLOCK_MASKf )) - 1);
                          soc_SRC_MODID_BLOCKm_field32_set(unit, &srcmod, 
                                                           PORT_0_5_BLOCK_MASKf, 
                                                           rp_val);
                     }
#endif /* BCM_RAPTOR_SUPPORT */
                     rv = WRITE_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ALL, 
                                                 idx, &srcmod);
                 }
            }
        }
        soc_mem_unlock(unit, SRC_MODID_BLOCKm);
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *      bcm_port_modid_egress_get
 * Description:
 *      Retrieve port bitmap of egress ports on which the incoming packets
 *      will be forwarded.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port  - ingress port number.
 *      modid - source module ID index (in HiGig mod header).
 *      pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_port_modid_egress_get(int unit, bcm_port_t port,
                              bcm_module_t modid, bcm_pbmp_t *pbmp)
{
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_UNAVAIL;

#ifdef  BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        mem_ing_srcmodblk_entry_t srcmod;
        bcm_module_t min_mod, max_mod;
        int blk, rv=BCM_E_NONE;

        if (!SOC_PORT_VALID(unit, port) || !IS_HG_PORT(unit, port)) {
            return BCM_E_PORT;
        }

        min_mod = soc_mem_index_min(unit, MEM_ING_SRCMODBLKm);
        max_mod = soc_mem_index_max(unit, MEM_ING_SRCMODBLKm);
        if ((modid < min_mod) || (modid > max_mod))  {
            return BCM_E_PARAM;
        }

        blk = SOC_PORT_BLOCK(unit, port);
        SOC_PBMP_CLEAR(*pbmp);
        rv = READ_MEM_ING_SRCMODBLKm(unit, blk, modid, &srcmod);
        if (rv >= 0) {
            SOC_PBMP_WORD_SET(*pbmp, 0,
                soc_MEM_ING_SRCMODBLKm_field32_get(unit, &srcmod, BITMAPf));
        }
    }
#endif /* BCM_HERCULES15_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
        SOC_IS_KATANA(unit)) {
        src_modid_egress_entry_t srcmodegr;
        int idx;
        _bcm_port_egr_dest_t    egr_dst;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_min_modid != egr_dst.out_max_modid) {
            return BCM_E_PARAM;
        }
        if (egr_dst.out_max_port != egr_dst.out_min_port) {
            return BCM_E_PORT;
        }

        idx = PORT_SRC_MOD_EGR_PROF_PTR(unit, egr_dst.out_min_port) *
              (SOC_MODID_MAX(unit) + 1) + egr_dst.out_min_modid;
        SOC_IF_ERROR_RETURN(READ_SRC_MODID_EGRESSm(unit, MEM_BLOCK_ANY, idx,
                                                   &srcmodegr));
        soc_mem_pbmp_field_get(unit, SRC_MODID_EGRESSm, &srcmodegr,
                               PORT_BLOCK_MASK_BITMAPf, pbmp);
        BCM_PBMP_NEGATE(*pbmp, *pbmp);
        BCM_PBMP_AND(*pbmp, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(*pbmp, PBMP_LB(unit));
        return SOC_E_NONE;
    }
#endif

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        src_modid_block_entry_t srcmod;
        int hg_port, idx;
        _bcm_port_egr_dest_t    egr_dst;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_min_modid != egr_dst.out_max_modid) {
            return BCM_E_PARAM;
        }
        if (egr_dst.out_max_port != egr_dst.out_min_port) {
            return BCM_E_PORT;
        }

        SOC_IF_ERROR_RETURN(
            soc_xgs3_port_num_to_higig_port_num(unit, egr_dst.out_min_port, 
                                                &hg_port));
        idx = hg_port * (SOC_MODID_MAX(unit) + 1) + egr_dst.out_min_modid;
        SOC_IF_ERROR_RETURN(READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, idx,
                                                  &srcmod));
        soc_SRC_MODID_BLOCKm_field_get(unit, &srcmod, PORT_BLOCK_MASK_BITMAPf,
                                       (uint32 *)pbmp);
        BCM_PBMP_NEGATE(*pbmp, *pbmp);
        BCM_PBMP_AND(*pbmp, PBMP_ALL(unit));

        return SOC_E_NONE;
    }
#endif

#ifdef BCM_FIREBOLT_SUPPORT
    if (soc_feature(unit, soc_feature_src_modid_blk)) {
        src_modid_block_entry_t srcmod;
        int modid_cnt, p_idx, idx;
        _bcm_port_egr_dest_t    egr_dst;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_min_modid != egr_dst.out_max_modid) {
            return BCM_E_PARAM;
        }
        if (egr_dst.out_max_port != egr_dst.out_min_port) {
            return BCM_E_PORT;
        }

        modid_cnt = SOC_MODID_MAX(unit) + 1;
        p_idx = egr_dst.out_min_port - SOC_HG_OFFSET(unit);
        if (SOC_IS_RAVEN(unit) && p_idx >= 4) {
            idx = (p_idx - 1) * modid_cnt + egr_dst.out_min_modid;
        } else {
            idx = (p_idx * modid_cnt) + egr_dst.out_min_modid;
        }

        rv = READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, idx, &srcmod);
        BCM_PBMP_CLEAR(*pbmp);
        if (rv >= 0) {
            int cpu, ether;
            pbmp_t hg_pbmp;
            uint32 xge_val;
#if defined(BCM_RAPTOR_SUPPORT)
            uint32 rp_val;
#endif /* BCM_RAPTOR_SUPPORT */
            cpu = soc_SRC_MODID_BLOCKm_field32_get(unit, &srcmod, 
                                                   CPU_PORT_BLOCK_MASKf);
            if (SOC_IS_FB_FX_HX(unit)) {
                ether = soc_SRC_MODID_BLOCKm_field32_get(unit, &srcmod, 
                                                         GE_PORT_BLOCK_MASKf);
            } else {
                ether = 0;
            }
            xge_val = soc_SRC_MODID_BLOCKm_field32_get(unit, &srcmod, 
                                                       HIGIG_XGE_PORT_BLOCK_MASKf);
            SOC_PBMP_CLEAR(hg_pbmp);
            SOC_PBMP_WORD_SET(hg_pbmp, 0, xge_val << SOC_HG_OFFSET(unit));

#if defined(BCM_RAPTOR_SUPPORT)
            if (soc_mem_field_valid(unit, SRC_MODID_BLOCKm, 
                                    PORT_0_5_BLOCK_MASKf)) {
                rp_val = soc_SRC_MODID_BLOCKm_field32_get(unit, &srcmod, 
                                                         PORT_0_5_BLOCK_MASKf);
                SOC_PBMP_WORD_SET(hg_pbmp, 0, rp_val);
            }
#endif /* BCM_RAPTOR_SUPPORT */
            BCM_PBMP_ASSIGN(*pbmp, PBMP_ALL(unit));
            if (cpu) {
                BCM_PBMP_REMOVE(*pbmp, PBMP_CMIC(unit));
            }
            if (ether) {
                BCM_PBMP_REMOVE(*pbmp, PBMP_E_ALL(unit)); 
            }
            if (BCM_PBMP_NOT_NULL(hg_pbmp)) {
                BCM_PBMP_REMOVE(*pbmp, hg_pbmp);
            }
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif  /* BCM_HERCULES15_SUPPORT  || BCM_FIREBOLT_SUPPORT */
}

/*
 * Function:
 *    bcm_port_modid_enable_set
 * Purpose:
 *    Enable/block packets from a specific module on a port.
 * Parameters:
 *    unit - StrataSwitch PCI device unit number (driver internal).
 *    port - StrataSwitch port number.
 *    modid - Which source module id to enable/disable
 *    enable - TRUE/FALSE Enable/disable forwarding packets from
 *             source module arriving on port.
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_esw_port_modid_enable_set(int unit, bcm_port_t port, int modid, int enable)
{
    int max_mod;
#ifdef BCM_HERCULES_SUPPORT
    uint32 smf, osmf;
#endif /* BCM_HERCULES_SUPPORT */
#ifdef BCM_HERCULES1_SUPPORT
    uint32 mask;
#endif /* BCM_HERCULES1_SUPPORT */
#ifdef BCM_HERCULES15_SUPPORT
    mem_ing_srcmodblk_entry_t srcmod;
    int blk;
#endif /* BCM_HERCULES15_SUPPORT */
#if defined(BCM_HERCULES15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    int rv=BCM_E_NONE;
#endif /* BCM_HERCULES15_SUPPORT || BCM_FIREBOLT_SUPPORT */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        src_modid_block_entry_t srcmod;
        int modid_cnt, hg_port, idx, i;
        bcm_pbmp_t pbmp;
        _bcm_port_egr_dest_t    egr_dst;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_max_port != egr_dst.out_min_port) {
            return BCM_E_PORT;
        }

        if (!IS_E_PORT(unit, egr_dst.out_max_port) && 
            !IS_ST_PORT(unit, egr_dst.out_max_port)) {
            return BCM_E_PORT;
        }


        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
            SOC_IS_KATANA(unit)) {
            soc_mem_lock(unit, SRC_MODID_INGRESS_BLOCKm);
            for (i = egr_dst.out_min_modid; i <= egr_dst.out_max_modid; i++) {
                rv = READ_SRC_MODID_INGRESS_BLOCKm(unit, MEM_BLOCK_ANY, i, &srcmod);
                if (rv >= 0) {
                    soc_mem_pbmp_field_get(unit, SRC_MODID_INGRESS_BLOCKm,
                                           &srcmod, PORT_BITMAPf, &pbmp);
                    if (enable) {
                        BCM_PBMP_PORT_REMOVE(pbmp, egr_dst.out_max_port);
                    } else {
                        BCM_PBMP_PORT_ADD(pbmp, egr_dst.out_max_port);
                    }
                    soc_mem_pbmp_field_set(unit, SRC_MODID_INGRESS_BLOCKm,
                                           &srcmod, PORT_BITMAPf, &pbmp);
                    rv = WRITE_SRC_MODID_INGRESS_BLOCKm(unit, MEM_BLOCK_ALL, i, &srcmod);
                }
                if (rv < 0) {
                    soc_mem_unlock(unit, SRC_MODID_INGRESS_BLOCKm);
                    return rv;
                }
            }
            soc_mem_unlock(unit, SRC_MODID_INGRESS_BLOCKm);
        } else {
            modid_cnt = SOC_MODID_MAX(unit) + 1;
            SOC_IF_ERROR_RETURN(
                soc_xgs3_port_num_to_higig_port_num(unit, port, &hg_port));

            soc_mem_lock(unit, SRC_MODID_BLOCKm);
            for (i = egr_dst.out_min_modid; i <= egr_dst.out_max_modid; i++) {
                idx = hg_port * modid_cnt + i;
                rv = READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, idx, &srcmod);
                if (rv >= 0) {
                    if (!enable) {
                        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
                    } else {
                        SOC_PBMP_CLEAR(pbmp);
                    }
                    soc_mem_pbmp_field_set(unit, SRC_MODID_BLOCKm, &srcmod, 
                                           PORT_BLOCK_MASK_BITMAPf,&pbmp);
                    rv = WRITE_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ALL, idx, &srcmod);
                }
                if (rv < 0) {
                    soc_mem_unlock(unit, SRC_MODID_BLOCKm);
                    return rv;
                }
            }
            soc_mem_unlock(unit, SRC_MODID_BLOCKm);
        }
        return rv;
    }
#endif  /* BCM_TRX_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (soc_feature(unit, soc_feature_src_modid_blk)) {
        src_modid_block_entry_t src_mod;
        int modid_cnt, mod_idx, p_idx, idx, val;
        _bcm_port_egr_dest_t    egr_dst;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_max_port != egr_dst.out_min_port) {
            return BCM_E_PORT;
        }

        modid_cnt = SOC_MODID_MAX(unit) + 1;
        p_idx = egr_dst.out_min_port - SOC_HG_OFFSET(unit);
        soc_mem_lock(unit, SRC_MODID_BLOCKm);
        for (mod_idx = egr_dst.out_min_modid; mod_idx <= egr_dst.out_max_modid; mod_idx++) {
             sal_memset(&src_mod, 0, sizeof(src_mod));
             if (SOC_IS_RAVEN(unit) && p_idx >= 4) {
                 idx = (p_idx - 1) * modid_cnt + mod_idx;
             } else {
                 idx = (p_idx * modid_cnt) + mod_idx;
             }
             rv = READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, idx, &src_mod);
             if (rv >= 0) {
                 soc_SRC_MODID_BLOCKm_field32_set(unit, &src_mod,
                                                  CPU_PORT_BLOCK_MASKf,
                                                  (enable) ? 0 : 1);
                 if (SOC_IS_FB_FX_HX(unit)) {
                     soc_SRC_MODID_BLOCKm_field32_set(unit, &src_mod,
                                                      GE_PORT_BLOCK_MASKf,
                                                      (enable) ? 0 : 1);
                 }

                 val = (1 << soc_mem_field_length(unit, SRC_MODID_BLOCKm,
                                             HIGIG_XGE_PORT_BLOCK_MASKf)) - 1;
                 
                 soc_SRC_MODID_BLOCKm_field32_set(unit, &src_mod,
                                                  HIGIG_XGE_PORT_BLOCK_MASKf,
                                                  (enable) ? 0 : val);
#if defined(BCM_RAPTOR_SUPPORT)
                 if (soc_mem_field_valid(unit, SRC_MODID_BLOCKm, 
                                         PORT_0_5_BLOCK_MASKf)) {

                     val = (1 << soc_mem_field_length(unit, SRC_MODID_BLOCKm,
                                                 PORT_0_5_BLOCK_MASKf)) - 1;

                     soc_SRC_MODID_BLOCKm_field32_set(unit, &src_mod,
                                                      PORT_0_5_BLOCK_MASKf,
                                                      (enable) ? 0 : val);

                 }
#endif /* BCM_RAPTOR_SUPPORT */
                 rv = WRITE_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ALL,
                                             idx, &src_mod);
             }
        }
        soc_mem_unlock(unit, SRC_MODID_BLOCKm);

        return rv;
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    if (!IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (!soc_feature(unit, soc_feature_srcmod_filter)) {
        if (enable) {
            return BCM_E_NONE;
        } else {
            return BCM_E_UNAVAIL;
        }
    }

    max_mod = SOC_IS_HERCULES1(unit) ? 32 : 128;
    if (modid >= max_mod) {
        return BCM_E_PARAM;
    }

#ifdef BCM_HERCULES1_SUPPORT
    if (SOC_IS_HERCULES1(unit)) {
        SOC_IF_ERROR_RETURN(READ_ING_SRCMODFILTERr(unit, port, &smf));
        osmf = smf;
        if (modid < 0) {
            mask = 0xffffffff;
        } else {
            mask = 1 << modid;
        }
        if (enable) {
            smf |= mask;
        } else {
            smf &= ~mask;
        }
        if (smf != osmf) {
            SOC_IF_ERROR_RETURN(WRITE_ING_SRCMODFILTERr(unit, port, smf));
        }
    }
#endif /* BCM_HERCULES1_SUPPORT */

#ifdef BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        blk = SOC_PORT_BLOCK(unit, port);
        soc_mem_lock(unit, MEM_ING_SRCMODBLKm);
        if (modid < 0) {
            for (modid = 0; modid < max_mod; modid++) {
                rv = READ_MEM_ING_SRCMODBLKm(unit, blk, modid, &srcmod);
                if (rv < 0) {
                    break;
                }
                soc_MEM_ING_SRCMODBLKm_field_get(unit, &srcmod, BITMAPf,
                                                 &osmf);
                smf = (enable) ? SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0) : 0;
                if (osmf != smf) {
                    soc_MEM_ING_SRCMODBLKm_field_set(unit, &srcmod, BITMAPf,
                                                     &smf);
                    rv = WRITE_MEM_ING_SRCMODBLKm(unit, blk, modid, &srcmod);
                    if (rv < 0) {
                        break;
                    }
                }
            }
        } else {
            rv = READ_MEM_ING_SRCMODBLKm(unit, blk, modid, &srcmod);
            if (rv >= 0) {
                soc_MEM_ING_SRCMODBLKm_field_get(unit, &srcmod, BITMAPf,
                                                 &osmf);
                smf = (enable) ? SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0) : 0;
                if (osmf != smf) {
                    soc_MEM_ING_SRCMODBLKm_field_set(unit, &srcmod, BITMAPf,
                                                     &smf);
                    rv = WRITE_MEM_ING_SRCMODBLKm(unit, blk, modid, &srcmod);
                }
            }
        }
        soc_mem_unlock(unit, MEM_ING_SRCMODBLKm);
    }
#endif /* BCM_HERCULES15_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_port_modid_enable_get
 * Purpose:
 *    Return enable/block state for a specific module on a port.
 * Parameters:
 *    unit - StrataSwitch PCI device unit number (driver internal).
 *    port - StrataSwitch port number.
 *    modid - Which source module id to enable/disable
 *    enable - (OUT) TRUE/FALSE Enable/disable forwarding packets from
 *             source module arriving on port.
 * Returns:
 *    BCM_E_XXX
 */
int
bcm_esw_port_modid_enable_get(int unit, bcm_port_t port, int modid, int *enable)
{
    int max_mod;
#ifdef BCM_HERCULES_SUPPORT
    uint32 smf;
#endif /* BCM_HERCULES_SUPPORT */
#ifdef BCM_HERCULES1_SUPPORT
    uint32 mask;
#endif /* BCM_HERCULES1_SUPPORT */
#ifdef BCM_HERCULES15_SUPPORT
    mem_ing_srcmodblk_entry_t srcmod;
    int blk, rv;
#endif /* BCM_HERCULES15_SUPPORT */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        src_modid_block_entry_t srcmod;
        int hg_port, idx;
        bcm_pbmp_t pbmp;
        _bcm_port_egr_dest_t    egr_dst;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_max_port != egr_dst.out_min_port) {
            return BCM_E_PORT;
        }
        if (egr_dst.out_max_modid != egr_dst.out_min_modid) {
            return BCM_E_PARAM;
        }

        if (!IS_E_PORT(unit, egr_dst.out_max_port) && 
            !IS_ST_PORT(unit, egr_dst.out_max_port)) {
            return BCM_E_PORT;
        }

        SOC_PBMP_CLEAR(pbmp);
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
            SOC_IS_KATANA(unit)) {
            SOC_IF_ERROR_RETURN(
                READ_SRC_MODID_INGRESS_BLOCKm(unit, MEM_BLOCK_ANY, 
                                              egr_dst.out_min_modid, &srcmod));
            soc_mem_pbmp_field_get(unit, SRC_MODID_INGRESS_BLOCKm,
                                   &srcmod, PORT_BITMAPf, &pbmp);
        } else {
            SOC_IF_ERROR_RETURN(
                soc_xgs3_port_num_to_higig_port_num(unit, egr_dst.out_min_port, 
                                                    &hg_port));

            idx = hg_port * (SOC_MODID_MAX(unit) + 1) + egr_dst.out_min_modid;
            SOC_IF_ERROR_RETURN(READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, idx,
                                                      &srcmod));
            soc_mem_pbmp_field_get(unit, SRC_MODID_BLOCKm, &srcmod, 
                                   PORT_BLOCK_MASK_BITMAPf, &pbmp);
        }

        *enable = !SOC_PBMP_MEMBER(pbmp, egr_dst.out_max_port);

        return BCM_E_NONE;
    }
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (soc_feature(unit, soc_feature_src_modid_blk)) {
        uint32 config;
        src_modid_block_entry_t src_mod;
        int gable, modid_cnt, p_idx, idx;
        _bcm_port_egr_dest_t    egr_dst;

        egr_dst.in_port = port;
        egr_dst.in_modid = modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_port_modid_egress_resolve(unit, &egr_dst));

        if (egr_dst.out_max_port != egr_dst.out_min_port) {
            return BCM_E_PORT;
        }
        if (egr_dst.out_max_modid != egr_dst.out_min_modid) {
            return BCM_E_PARAM;
        }
  
        SOC_IF_ERROR_RETURN(READ_ING_CONFIGr(unit, &config));
        gable = !soc_reg_field_get(unit, ING_CONFIGr, config,
                                   FB_A0_COMPATIBLEf);
        if (gable) {
            modid_cnt = SOC_MODID_MAX(unit) + 1;
            p_idx = egr_dst.out_min_port - SOC_HG_OFFSET(unit);
            if (SOC_IS_RAVEN(unit) && p_idx >= 4) {
                idx = (p_idx - 1) * modid_cnt + egr_dst.out_min_modid;
            } else {
                idx = (p_idx * modid_cnt) + egr_dst.out_min_modid;
            } 
            SOC_IF_ERROR_RETURN
                (READ_SRC_MODID_BLOCKm(unit, MEM_BLOCK_ANY, 
                                       idx, &src_mod));
            *enable = (soc_SRC_MODID_BLOCKm_field32_get(unit, &src_mod, 
                                                        CPU_PORT_BLOCK_MASKf) |
                       soc_SRC_MODID_BLOCKm_field32_get(unit, &src_mod, 
                                                        HIGIG_XGE_PORT_BLOCK_MASKf)
                                                        ) ? 0 : 1;
            if (SOC_IS_FB_FX_HX(unit)) {
                *enable = (soc_SRC_MODID_BLOCKm_field32_get(unit, &src_mod, 
                                                            GE_PORT_BLOCK_MASKf)
                                                            ) ? 0 : 1;
            }

#if defined(BCM_RAPTOR_SUPPORT)
            if (soc_mem_field_valid(unit, SRC_MODID_BLOCKm, 
                                    PORT_0_5_BLOCK_MASKf)) {
                *enable = (soc_SRC_MODID_BLOCKm_field32_get(unit, &src_mod,
                                                 PORT_0_5_BLOCK_MASKf)
                                                 ) ? 0 : 1;
            }
#endif /* BCM_RAPTOR_SUPPORT */

        } else {
            *enable = gable;
        }

        return BCM_E_NONE;
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    if (!IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (!soc_feature(unit, soc_feature_srcmod_filter)) {
        *enable = TRUE;
        return BCM_E_NONE;
    }

    max_mod = SOC_IS_HERCULES1(unit) ? 32 : 128;
    if (!((modid >= 0) && (modid < max_mod))) {
        return BCM_E_PARAM;
    }

#ifdef BCM_HERCULES1_SUPPORT
    if (SOC_IS_HERCULES1(unit)) {
        SOC_IF_ERROR_RETURN(READ_ING_SRCMODFILTERr(unit, port, &smf));
        mask = 1 << modid;
        *enable = ((smf & mask) != 0);
    }
#endif /* BCM_HERCULES1_SUPPORT */

#ifdef BCM_HERCULES15_SUPPORT
    if (SOC_IS_HERCULES15(unit)) {
        blk = SOC_PORT_BLOCK(unit, port);
        rv = READ_MEM_ING_SRCMODBLKm(unit, blk, modid, &srcmod);
        if (rv >= 0) {
            soc_MEM_ING_SRCMODBLKm_field_get(unit, &srcmod, BITMAPf, &smf);
            *enable = (smf & SOC_PBMP_WORD_GET(PBMP_ALL(unit),0)) ? 1 : 0;
        }
    }
#endif /* BCM_HERCULES15_SUPPORT */

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_port_flood_block_op
 * Purpose:
 *      Set or get port membership in one of the egress block registers
 * Parameters:
 *      unit
 *      set_op          - TRUE for set, FALSE for get
 *      ingress_port    - ingress port on which the egress blocking
 *                        will be enabled
 *      egress_port     - port for which egress will be blocked
 *      mode            - BCM_PORT_FLOOD_BLOCK_XXX
 *      enable          - Whether blocking should be enabled or disabled
 *                        This is also BCM_PORT_FLOOD_BLOCK_XXX
 * Returns:
 *      BCM_E_UNAVAIL   - This chip does not support this feature
 *      BCM_E_NONE
 */
STATIC int
_bcm_port_flood_block_op(int unit, int set_op,
                         bcm_port_t ingress_port, bcm_port_t egress_port,
                         uint32 mode, uint32 *enable)
{
    soc_reg_t reg, mask_reg[2];
    soc_mem_t mem;
    soc_field_t field;
    uint32 addr;
    uint64 rval;
    uint32 fval;
    uint32 entry[SOC_MAX_MEM_WORDS];
    pbmp_t pbmp;
    int index;

    mask_reg[0] = mask_reg[1] = INVALIDr;
    mem = INVALIDm;
    field = INVALIDf;

    switch (mode) {
    case BCM_PORT_FLOOD_BLOCK_BCAST:
        if (SOC_MEM_IS_VALID(unit, BCAST_BLOCK_MASKm)) {
            mem = BCAST_BLOCK_MASKm;
            field = BLK_BITMAPf;
        } else if (SOC_REG_IS_VALID(unit, BCAST_BLOCK_MASK_64r)) {
            mask_reg[0] = BCAST_BLOCK_MASK_64r;
            mask_reg[1] = IBCAST_BLOCK_MASK_64r;
        } else if (SOC_REG_IS_VALID(unit, BCAST_BLOCK_MASKr)) {
            mask_reg[0] = BCAST_BLOCK_MASKr;
            mask_reg[1] = IBCAST_BLOCK_MASKr;
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
    case BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST:
        if (SOC_MEM_IS_VALID(unit, UNKNOWN_UCAST_BLOCK_MASKm)) {
            mem = UNKNOWN_UCAST_BLOCK_MASKm;
            field = BLK_BITMAPf;
        } else if (SOC_REG_IS_VALID(unit, UNKNOWN_UCAST_BLOCK_MASK_64r)) {
            mask_reg[0] = UNKNOWN_UCAST_BLOCK_MASK_64r;
            mask_reg[1] = IUNKNOWN_UCAST_BLOCK_MASK_64r;
        } else if (SOC_REG_IS_VALID(unit, UNKNOWN_UCAST_BLOCK_MASKr)) {
            mask_reg[0] = UNKNOWN_UCAST_BLOCK_MASKr;
            mask_reg[1] = IUNKNOWN_UCAST_BLOCK_MASKr;
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
    case BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST:
        if (SOC_MEM_IS_VALID(unit, UNKNOWN_UCAST_BLOCK_MASKm)) {
            mem = UNKNOWN_MCAST_BLOCK_MASKm;
            field = BLK_BITMAPf;
        } else if (SOC_REG_IS_VALID(unit, UNKNOWN_MCAST_BLOCK_MASK_64r)) {
            mask_reg[0] = UNKNOWN_MCAST_BLOCK_MASK_64r;
            mask_reg[1] = IUNKNOWN_MCAST_BLOCK_MASK_64r;
        } else if (SOC_REG_IS_VALID(unit, UNKNOWN_MCAST_BLOCK_MASKr)) {
            mask_reg[0] = UNKNOWN_MCAST_BLOCK_MASKr;
            mask_reg[1] = IUNKNOWN_MCAST_BLOCK_MASKr;
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
    case BCM_PORT_FLOOD_BLOCK_ALL:
        if (SOC_MEM_IS_VALID(unit, ING_EGRMSKBMAPm)) {
            mem = ING_EGRMSKBMAPm;
            field = BITMAPf;
        } else if (SOC_REG_IS_VALID(unit, ING_EGRMSKBMAP_64r)) {
            mask_reg[0] = ING_EGRMSKBMAP_64r;
            mask_reg[1] = IING_EGRMSKBMAP_64r;
        } else if (SOC_REG_IS_VALID(unit, ING_EGRMSKBMAPr)) {
            mask_reg[0] = ING_EGRMSKBMAPr;
            mask_reg[1] = IING_EGRMSKBMAPr;
        } else {
            return BCM_E_UNAVAIL;
        }
        break;
    default:
        return BCM_E_INTERNAL;
    }

    if (mem != INVALIDm) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, mem, MEM_BLOCK_ANY, ingress_port, entry));
        soc_mem_pbmp_field_get(unit, mem, entry, field, &pbmp);
        if (!set_op) {
            *enable = SOC_PBMP_WORD_GET(pbmp, egress_port / 32) &
                (1 << (egress_port & 0x1f)) ? mode : 0;
            return BCM_E_NONE;
        }
        if (*enable & mode) {
            SOC_PBMP_PORT_ADD(pbmp, egress_port);
        } else {
            SOC_PBMP_PORT_REMOVE(pbmp, egress_port);
        }
        soc_mem_pbmp_field_set(unit, mem, entry, field, &pbmp);
        BCM_IF_ERROR_RETURN
            (soc_mem_write(unit, mem, MEM_BLOCK_ANY, ingress_port, entry));

        if (IS_CPU_PORT(unit, ingress_port) &&
            SOC_INFO(unit).cpu_hg_index != -1) {
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                              SOC_INFO(unit).cpu_hg_index, entry));
            soc_mem_pbmp_field_get(unit, mem, entry, field, &pbmp);
            if (*enable & mode) {
                SOC_PBMP_PORT_ADD(pbmp, egress_port);
            } else {
                SOC_PBMP_PORT_REMOVE(pbmp, egress_port);
            }
            soc_mem_pbmp_field_set(unit, mem, entry, field, &pbmp);
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, mem, MEM_BLOCK_ANY,
                               SOC_INFO(unit).cpu_hg_index, entry));
        }
    } else {
        if (!IS_CPU_PORT(unit, ingress_port)) {
            if (IS_ST_PORT(unit, ingress_port)) {
                mask_reg[0] = INVALIDr;
            } else {
                mask_reg[1] = INVALIDr;
            }
        }

        for (index = 0; index < 2; index++) {
            reg = mask_reg[index];
            if (reg == INVALIDr) {
                continue;
            }
            addr = soc_reg_addr(unit, reg, ingress_port, 0);
            BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg, ingress_port, 0, &rval));
            if (SOC_REG_IS_64(unit, reg)) {
                if (egress_port < 32) {
                    field = BLK_BITMAP_0f;
                } else {
                    field = BLK_BITMAP_1f;
                }
            } else {
                field = BLK_BITMAPf;
            }
            fval = soc_reg64_field32_get(unit, reg, rval, field);
            if (!set_op) {
                *enable = fval & (1 << (egress_port & 0x1f)) ? mode : 0;
                return BCM_E_NONE;
            }
            if (*enable & mode) {
                fval |= 1 << (egress_port & 0x1f);
            } else {
                fval &= ~(1 << (egress_port & 0x1f));
            }
            soc_reg64_field32_set(unit, reg, &rval, field, fval);
            BCM_IF_ERROR_RETURN(soc_reg_set(unit, reg, ingress_port, 0, rval));
        }
    }

    return BCM_E_NONE;
}

#if defined(BCM_RAPTOR_SUPPORT)
/*
 * Function:
 *      _bcm_port_flood_block_hi_set
 * Purpose:
 *      Enable/disable port membership in one of the egress block registers
 * Parameters:
 *      unit
 *      ingress_port    - ingress port on which the egress blocking
 *                        will be enabled
 *      egress_port     - port for which egress will be blocked
 *      reg             - egress block register.
 *      enable          - Whether blocking should be enabled or disabled
 * Returns:
 *      BCM_E_UNAVAIL   - This chip does not support this feature
 *      BCM_E_NONE
 * Notes:
 *      'reg' must be one of BCAST_BLOCK_MASKr,
 *      UNKNOWN_UCAST_BLOCK_MASKr, UNKNOWN_MCAST_BLOCK_MASKr, or
 *      their Stack port variants.
 *
 *      This function is a helper for bcm_port_flood_block_set()
 */

STATIC int
_bcm_port_flood_block_hi_set(int unit,
                              bcm_port_t ingress_port, bcm_port_t egress_port,
                              soc_reg_t reg, uint32 enable)
{
    uint32 rval;
    uint32 addr;
    pbmp_t pbmp;
    uint32 pbmp32;
    uint32 bitmap32, obitmap32;

    if (!soc_feature(unit, soc_feature_register_hi)) {
        return BCM_E_UNAVAIL;
    }

    SOC_PBMP_PORT_SET(pbmp, egress_port);
    pbmp32 = SOC_PBMP_WORD_GET(pbmp, 1);

    if (!pbmp32) {
        return BCM_E_NONE;
    }

    addr = soc_reg_addr(unit, reg, ingress_port, 0);
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, ingress_port, 0, &rval));

    obitmap32 = bitmap32 = soc_reg_field_get(unit, reg, rval, BLK_BITMAPf);
    if (enable) {
        bitmap32 |= pbmp32;
    } else {
        bitmap32 &= ~pbmp32;
    }

    if (obitmap32 != bitmap32) {
        soc_reg_field_set(unit, reg, &rval, BLK_BITMAPf, bitmap32);
        return soc_reg32_set(unit, reg, ingress_port, 0, rval);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_port_flood_block_hi_get
 * Purpose:
 *      Retrieve the current state egress block state on a port
 * Parameters:
 *      unit
 *      ingress_port    - ingress_port for which the state will be retreived
 *      egress_port     - port for which the egress blocking state
 *                        on ingress_port is requested
 *      reg             - block mask register
 *      enable          - (OUT) receives the block state
 * Returns:
 *      BCM_E_UNAVAIL   - Feature unavailable on this chip
 *      BCM_E_NONE
 * Notes:
 *      See _bcm_port_flood_block_hi_set()
 *      This is a helper function for bcm_port_flood_block_get()
 */

STATIC int
_bcm_port_flood_block_hi_get(int unit,
                          bcm_port_t ingress_port, bcm_port_t egress_port,
                          soc_reg_t reg, uint32 *enable)
{
    uint32 rval;
    uint32 addr;
    pbmp_t pbmp;
    uint32 pbmp32;
    uint32 bitmap32;

    if (!soc_feature(unit, soc_feature_register_hi)) {
        return BCM_E_UNAVAIL;
    }

    SOC_PBMP_PORT_SET(pbmp, egress_port);
    pbmp32 = SOC_PBMP_WORD_GET(pbmp, 1);
    if (!pbmp32) {
        *enable = 0;
        return BCM_E_NONE;
    }

    addr = soc_reg_addr(unit, reg, ingress_port, 0);
    BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg, ingress_port, 0, &rval));

    bitmap32 = soc_reg_field_get(unit, reg, rval, BLK_BITMAPf);
    *enable = (pbmp32 & bitmap32) ? 1 : 0;

    return BCM_E_NONE;
}
#endif /* BCM_RAPTOR_SUPPORT */

/*
 * Function:
 *      bcm_port_flood_block_set
 * Purpose:
 *      Set selective per-port blocking of flooded VLAN traffic
 * Parameters:
 *      unit            - unit number
 *      ingress_port    - Port traffic is ingressing
 *      egress_port     - Port for which the traffic should be blocked.
 *      flags           - Specifies the type of traffic to block
 * Returns:
 *      BCM_E_UNAVAIL   - Functionality not available
 *      BCM_E_NONE
 *
 * NOTE:  For fabrics, this changes egrmskbmap; as a result, all traffic
 * will be blocked (or enabled) to the given egress port, including
 * unicast traffic.
 */

int
bcm_esw_port_flood_block_set(int unit,
                         bcm_port_t ingress_port, bcm_port_t egress_port,
                         uint32 flags)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, ingress_port, 
                                                     &ingress_port));
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, egress_port, 
                                                     &egress_port));

#if defined(BCM_XGS12_FABRIC_SUPPORT)
    /* See note above */
    if (SOC_IS_XGS12_FABRIC(unit)) {
        uint32  ports, oports;
        pbmp_t  pbmp;

        if (!IS_HG_PORT(unit, ingress_port)) {
            return BCM_E_PORT;
        }

        SOC_IF_ERROR_RETURN
            (READ_ING_EGRMSKBMAPr(unit, ingress_port, &ports));
        oports = ports;
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, ports);
        if (flags) {
            SOC_PBMP_PORT_REMOVE(pbmp, egress_port);
        } else {
            SOC_PBMP_PORT_ADD(pbmp, egress_port);
        }
        ports = SOC_PBMP_WORD_GET(pbmp, 0);
        if (ports != oports) {
            SOC_IF_ERROR_RETURN
                (WRITE_ING_EGRMSKBMAPr(unit, ingress_port, ports));
        }
        return BCM_E_NONE;
    }
#endif /* BCM_XGS12_FABRIC_SUPPORT */

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit)||
        SOC_IS_HBX(unit) || SOC_IS_TRX(unit) || SOC_IS_HAWKEYE(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_port_flood_block_op(unit, TRUE, ingress_port, egress_port,
                                      BCM_PORT_FLOOD_BLOCK_ALL, &flags));
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || \
          BCM_BRADLEY_SUPPORT */

    BCM_IF_ERROR_RETURN
        (_bcm_port_flood_block_op(unit, TRUE, ingress_port, egress_port,
                                  BCM_PORT_FLOOD_BLOCK_BCAST, &flags));

    BCM_IF_ERROR_RETURN
        (_bcm_port_flood_block_op(unit, TRUE, ingress_port, egress_port,
                                  BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST, &flags));

    BCM_IF_ERROR_RETURN
        (_bcm_port_flood_block_op(unit, TRUE, ingress_port, egress_port,
                                  BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST, &flags));

    /*
     * Handle the devices with > 32 ports that use _HI convention
     */
#if defined(BCM_RAPTOR_SUPPORT)
       if (soc_feature(unit, soc_feature_register_hi)){
           soc_reg_t reg;
           uint32 enable;
            reg = IS_ST_PORT(unit, ingress_port) ?
                IUNKNOWN_UCAST_BLOCK_MASK_HIr : UNKNOWN_UCAST_BLOCK_MASK_HIr;
            enable = flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST;
            BCM_IF_ERROR_RETURN
                (_bcm_port_flood_block_hi_set(unit, ingress_port, egress_port,
                                              reg, enable));

            reg = IS_ST_PORT(unit, ingress_port) ?
                IUNKNOWN_MCAST_BLOCK_MASK_HIr : UNKNOWN_MCAST_BLOCK_MASK_HIr;
            enable = flags & BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST;
            BCM_IF_ERROR_RETURN
                (_bcm_port_flood_block_hi_set(unit, ingress_port, egress_port,
                                              reg, enable));

            reg = IS_ST_PORT(unit, ingress_port) ?
                IBCAST_BLOCK_MASK_HIr : BCAST_BLOCK_MASK_HIr;
            enable = flags & BCM_PORT_FLOOD_BLOCK_BCAST;
            BCM_IF_ERROR_RETURN
                (_bcm_port_flood_block_hi_set(unit, ingress_port, egress_port,
                                              reg, enable));
       }
#endif /* BCM_RAPTOR_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_flood_block_get
 * Purpose:
 *      Get the current per-port flood block settings
 * Parameters:
 *      unit            - unit number
 *      ingress_port    - Port traffic is ingressing
 *      egress_port     - Port for which the traffic would be blocked
 *      flags           - (OUT) Returns the current settings
 * Returns:
 *      BCM_E_UNAVAIL   - Functionality not available
 *      BCM_E_NONE
 */

int
bcm_esw_port_flood_block_get(int unit,
                         bcm_port_t ingress_port, bcm_port_t egress_port,
                         uint32 *flags)
{
    uint32 enable;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, ingress_port, 
                                                     &ingress_port));
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, egress_port, 
                                                     &egress_port));

    *flags = 0;

#if defined(BCM_XGS12_FABRIC_SUPPORT)
    if (SOC_IS_XGS12_FABRIC(unit)) {
        uint32  ports;
        pbmp_t  pbmp;

        if (!IS_HG_PORT(unit, ingress_port)) {
            return BCM_E_PORT;
        }

        SOC_IF_ERROR_RETURN
            (READ_ING_EGRMSKBMAPr(unit, ingress_port, &ports));
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, ports);
        if (!SOC_PBMP_MEMBER(pbmp, egress_port)) {
            *flags = -1;        
        }
        return BCM_E_NONE;
    }
#endif

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined(BCM_BRADLEY_SUPPORT)
    if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit)||
        SOC_IS_HBX(unit) || SOC_IS_TRX(unit) || SOC_IS_HAWKEYE(unit)) {
        BCM_IF_ERROR_RETURN
            (_bcm_port_flood_block_op(unit, FALSE, ingress_port, egress_port,
                                      BCM_PORT_FLOOD_BLOCK_ALL, &enable));
        *flags |= enable;
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_BRADLEY_SUPPORT */

    BCM_IF_ERROR_RETURN
        (_bcm_port_flood_block_op(unit, FALSE, ingress_port, egress_port,
                                  BCM_PORT_FLOOD_BLOCK_BCAST, &enable));
    *flags |= enable;

    BCM_IF_ERROR_RETURN
        (_bcm_port_flood_block_op(unit, FALSE, ingress_port, egress_port,
                                  BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST,
                                  &enable));
    *flags |= enable;

    BCM_IF_ERROR_RETURN
        (_bcm_port_flood_block_op(unit, FALSE, ingress_port, egress_port,
                                  BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST,
                                  &enable));
    *flags |= enable;

#if defined(BCM_RAPTOR_SUPPORT)
    if (soc_feature(unit, soc_feature_register_hi)){
        soc_reg_t reg;
        reg = IS_ST_PORT(unit, ingress_port) ?
            IBCAST_BLOCK_MASK_HIr : BCAST_BLOCK_MASK_HIr;
        BCM_IF_ERROR_RETURN
            (_bcm_port_flood_block_hi_get(unit, ingress_port, egress_port,
                                       reg, &enable));
        *flags |= (enable) ? BCM_PORT_FLOOD_BLOCK_BCAST : 0;

        reg = IS_ST_PORT(unit, ingress_port) ?
            IUNKNOWN_MCAST_BLOCK_MASK_HIr : UNKNOWN_MCAST_BLOCK_MASK_HIr;
        BCM_IF_ERROR_RETURN
            (_bcm_port_flood_block_hi_get(unit, ingress_port, egress_port,
                                       reg, &enable));
        *flags |= (enable) ? BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST : 0;

        reg = IS_ST_PORT(unit, ingress_port) ?
            IUNKNOWN_UCAST_BLOCK_MASK_HIr : UNKNOWN_UCAST_BLOCK_MASK_HIr;
        BCM_IF_ERROR_RETURN
            (_bcm_port_flood_block_hi_get(unit, ingress_port, egress_port,
                                       reg, &enable));
        *flags |= (enable) ? BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST : 0;
    }
#endif /* BCM_RAPTOR_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Per-port leaky bucket ingress and egress rate limiting
 *
 * Granularity is in kbits/sec and a rate of 0 disables rate limiting.
 * The max burst size is set in kbits.
 */

/*
 * Function:
 *      bcm_port_rate_egress_set
 * Purpose:
 *      Set egress rate limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      kbits_sec - Rate in kilobits (1000 bits) per second.
 *                      Rate of 0 disables rate limiting.
 *      kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_rate_egress_set(int unit,
                             bcm_port_t port,
                             uint32 kbits_sec,
                             uint32 kbits_burst)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (!soc_feature(unit, soc_feature_egress_metering)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_METER_SUPPORT
    else {
        return mbcm_driver[unit]->mbcm_port_rate_egress_set(unit, port,
                                                            kbits_sec,
                                                            kbits_burst);
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_rate_egress_get
 * Purpose:
 *      Get egress rate limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *                        zero if rate limiting is disabled.
 *      kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_rate_egress_get(int unit,
                             bcm_port_t port,
                             uint32 *kbits_sec,
                             uint32 *kbits_burst)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (!soc_feature(unit, soc_feature_egress_metering)) {
        return BCM_E_UNAVAIL;
    }

#ifdef BCM_METER_SUPPORT
    else {
        return mbcm_driver[unit]->mbcm_port_rate_egress_get(unit, port,
                                                            kbits_sec,
                                                            kbits_burst);
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_rate_ingress_set
 * Purpose:
 *      Set ingress rate limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      kbits_sec - Rate in kilobits (1000 bits) per second.
 *                  Rate of 0 disables rate limiting.
 *      kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_rate_ingress_set(int unit,
                              bcm_port_t port,
                              uint32 kbits_sec,
                              uint32 kbits_burst)
{
#ifdef BCM_METER_SUPPORT
    int                 rv = BCM_E_NONE;
#endif

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        /* Use port-based ingress metering function */
        return bcm_tucana_port_rate_ingress_set(unit, port, kbits_sec,
                                                kbits_burst);
    }
#endif

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        /*
         * Use port-based ingress metering function.
         * Enabling rate-based ingress metering on the Firebolt
         * will also enable rate-based pause frames on this port.
         */
        return bcm_xgs3_port_rate_ingress_set(unit, port, kbits_sec,
                                              kbits_burst);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_METER_SUPPORT
    /* Use FFP and metering to implement port-based ingress metering */
    /* If it exists, kill existing filter/meter */
    if (PORT(unit, port).meter_dpid != -1) {
        assert(PORT(unit, port).meter_cfid != -1);
        rv = bcm_esw_ds_datapath_delete(unit,
                                        PORT(unit, port).meter_dpid);
        PORT(unit, port).meter_dpid = -1;
        PORT(unit, port).meter_cfid = -1;
        if (rv < 0) {
            return rv;
        }
    }
    if (!kbits_sec) {
        return BCM_E_NONE;
    } else {
        bcm_ds_clfr_t clfr;
        bcm_ds_outprofile_actn_t outp;
        pbmp_t pbmp;

        /* Setup the profile */
        bcm_ds_clfr_t_init(&clfr);
        outp.opa_flags = BCM_DS_OUT_ACTN_DO_NOT_SWITCH;
        outp.opa_kbits_burst = kbits_burst;
        outp.opa_kbits_sec = kbits_sec;
        clfr.cf_flags = 0; /* no options */

        BCM_PBMP_PORT_SET(pbmp, port);
        rv = bcm_esw_ds_datapath_create(unit, 0, pbmp,
                                        &PORT(unit, port).meter_dpid);
        if (rv < 0) {
            return rv;
        }

        rv = bcm_esw_ds_classifier_create(unit, PORT(unit, port).meter_dpid,
                                          &clfr,
                                          NULL,
                                          &outp,
                                          NULL,
                                          &PORT(unit, port).meter_cfid);
        if (rv < 0) {
            (void) bcm_esw_ds_datapath_delete(unit,
                                              PORT(unit, port).meter_dpid);
            return rv;
        }

        rv = bcm_esw_ds_datapath_install(unit, PORT(unit, port).meter_dpid);
        if (rv < 0) {
            (void) bcm_esw_ds_classifier_delete(unit,
                                                PORT(unit, port).meter_dpid,
                                                PORT(unit, port).meter_cfid);
            (void) bcm_esw_ds_datapath_delete(unit,
                                              PORT(unit, port).meter_dpid);
            return rv;
        }
    } /* Set an FFP meter */

    return BCM_E_NONE;

#else /* !BCM_METER_SUPPORT */

    return BCM_E_UNAVAIL;

#endif /* !BCM_METER_SUPPORT */

}

/*
 * Function:
 *      bcm_port_rate_ingress_get
 * Purpose:
 *      Get ingress rate limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *                        zero if rate limiting is disabled.
 *      kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_rate_ingress_get(int unit,
                              bcm_port_t port,
                              uint32 *kbits_sec,
                              uint32 *kbits_burst)
{

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (!kbits_sec || !kbits_burst) {
        return BCM_E_PARAM;
    }

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        /* Use port-based ingress metering function */
        return bcm_tucana_port_rate_ingress_get(unit, port, kbits_sec,
                                                kbits_burst);
    }
#endif

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
       /* Use port-based ingress metering function */
        return bcm_xgs3_port_rate_ingress_get(unit, port, kbits_sec,
                                            kbits_burst);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_METER_SUPPORT
    if (PORT(unit, port).meter_dpid == -1) {
        assert(PORT(unit, port).meter_cfid == -1);
        *kbits_sec = 0;
        *kbits_burst = 0;
    } else {
        return bcm_esw_ds_rate_get(unit,
                                   PORT(unit, port).meter_dpid,
                                   PORT(unit, port).meter_cfid,
                                   kbits_sec,
                                   kbits_burst);
    }

    return BCM_E_NONE;

#else /* !BCM_METER_SUPPORT */

    *kbits_sec = 0;
    *kbits_burst = 0;
    return BCM_E_UNAVAIL;

#endif /* !BCM_METER_SUPPORT */

}

/*
 * Function:
 *      bcm_port_rate_pause_set
 * Purpose:
 *      Set ingress rate limiting pause frame control parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      kbits_pause - Pause threshold in kbits (1000 bits).
 *              A value of zero disables the pause/resume mechanism.
 *      kbits_resume - Resume threshold in kbits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This interface applies only when port ingress rate limiting
 *      is active.  Currently, only the 566x supports this feature.
 *
 *      If the maximum of bits that could be received before dropping a
 *      frame falls below the kbits_pause, a pause frame is sent.
 *      A resume frame will be sent once it becomes possible to receive
 *      kbits_resume bits of data without dropping.
 */

int
bcm_esw_port_rate_pause_set(int unit,
                            bcm_port_t port,
                            uint32 kbits_pause,
                            uint32 kbits_resume)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        return bcm_tucana_port_rate_pause_set(unit, port, kbits_pause,
                                              kbits_resume);
    }
#endif /* BCM_TUCANA_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    /*
     * For Firebolt, rate-based pause frames are generated as
     * a result of rate-based metering. As such, their behavior
     * is not directly controllable, but may be read.
     * see bcm_port_rate_pause_get().
     */
#endif /* BCM_FIREBOLT_SUPPORT */
    /* Otherwise, not supported  */
    return (kbits_pause == 0) ? BCM_E_NONE : BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_rate_pause_get
 * Purpose:
 *      Get ingress rate limiting pause frame control parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      kbits_pause - (OUT) Pause threshold in kbits (1000 bits).
 *              Zero indicates the pause/resume mechanism is disabled.
 *      kbits_resume - (OUT) Resume threshold in kbits (1000 bits).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_port_rate_pause_get(int unit,
                            bcm_port_t port,
                            uint32 *kbits_pause,
                            uint32 *kbits_resume)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit))  {
        return bcm_tucana_port_rate_pause_get(unit, port, kbits_pause,
                                              kbits_resume);
    }
#endif /* BCM_TUCANA_SUPPORT */
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit) && !SOC_IS_HURRICANE(unit)) {
        return bcm_tr_port_rate_pause_get(unit, port, kbits_pause,
                                          kbits_resume);
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        return bcm_fb_port_rate_pause_get(unit, port, kbits_pause,
                                          kbits_resume);
    }
#endif /* BCM_FIREBOLT_SUPPORT */
    /* Otherwise, not supported */
    *kbits_pause = 0;
    *kbits_resume = 0;
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_sample_rate_set
 * Purpose:
 *      Control the sampling of packets ingressing or egressing a port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      ingress_rate - Every 1/ingress_rate packets will be sampled
 *              0 indicates no sampling
 *              1 indicates sampling all packets
 *      egress_rate - Every 1/egress_rate packets will be sampled
 *              0 indicates no sampling
 *              1 indicates sampling all packets
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This API is a building block for supporting sFlow (RFC 3176)
 */

int
bcm_esw_port_sample_rate_set(int unit, bcm_port_t port,
                         int ingress_rate, int egress_rate)
{
    /*
     * IMPLEMENTATION NOTES
     *
     * ingress_rate maps into the per port SFLOW_ING_THRESHOLD reg:
     *  if (ingress_rate <= 0) {
     *          SFLOW_ING_THRESHOLD.ENABLE=0
     *          SFLOW_ING_THRESHOLD.THRESHOLD=0
     *  } else {
     *          SFLOW_ING_THRESHOLD.ENABLE=1
     *          SFLOW_ING_THRESHOLD.THRESHOLD=0xffff/ingress_rate
     *  }
     * egress_rate maps into the per port SFLOW_EGR_THRESHOLD reg:
     *  if (egress_rate <= 0) {
     *          SFLOW_EGR_THRESHOLD.ENABLE=0
     *          SFLOW_EGR_THRESHOLD.THRESHOLD=0
     *  } else {
     *          SFLOW_EGR_THRESHOLD.ENABLE=1
     *          SFLOW_EGR_THRESHOLD.THRESHOLD=0xffff/egress_rate
     *  }
     *
     * bcmSwitchCpuSamplePrio maps to CPU_CONTROL_2.CPU_SFLOW_PRIORITY
     * bcmSwitchSampleIngressRandomSeed maps to SFLOW_ING_RAND_SEED.SEED
     * bcmSwitchSampleEgressRandomSeed maps to SFLOW_EGR_RAND_SEED.SEED
     *
     * There are two related RX reason codes defined in <bcm/rx.h>
     *  BCM_RX_PR_SAMPLE_SRC
     *  BCM_RX_PR_SAMPLE_DST
     */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    if (ingress_rate < 0 || egress_rate < 0) {
        return BCM_E_PARAM;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        return bcm_xgs3_port_sample_rate_set(unit, port,
                                             ingress_rate, egress_rate);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_sample_rate_get
 * Purpose:
 *      Get the sampling of packets ingressing or egressing a port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      ingress_rate - Every 1/ingress_rate packets will be sampled
 *              0 indicates no sampling
 *              1 indicates sampling all packets
 *      egress_rate - Every 1/egress_rate packets will be sampled
 *              0 indicates no sampling
 *              1 indicates sampling all packets
 * Returns:
 *      BCM_E_UNIT
 *      BCM_E_PARAM
 *      BCM_E_UNAVAIL
 *      BCM_E_XXX
 * Notes:
 *      This API is a building block for supporting sFlow (RFC 3176)
 */

int
bcm_esw_port_sample_rate_get(int unit, bcm_port_t port,
                         int *ingress_rate, int *egress_rate)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (ingress_rate == NULL  || egress_rate == NULL) {
        return BCM_E_PARAM;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        return bcm_xgs3_port_sample_rate_get(unit, port,
                                           ingress_rate, egress_rate);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */


    return BCM_E_UNAVAIL;
}

/*
 * Double Tagging
 */

/*
 * Function:
 *      bcm_port_dtag_mode_set
 * Description:
 *      Set the double-tagging mode of a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - Double-tagging mode, one of:
 *              BCM_PORT_DTAG_MODE_NONE            No double tagging
 *              BCM_PORT_DTAG_MODE_INTERNAL        Service Provider port
 *              BCM_PORT_DTAG_MODE_EXTERNAL        Customer port
 *              BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG  Remove customer tag
 *              BCM_PORT_DTAG_ADD_EXTERNAL_TAG     Add customer tag
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      BCM_PORT_DTAG_MODE_INTERNAL is for service provider ports.
 *              A tag will be added if the packet does not already
 *              have the internal TPID (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and clears IGNORE_TAG.
 *      BCM_PORT_DTAG_MODE_EXTERNAL is for customer ports.
 *              The service provider TPID will always be added
 *              (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and sets IGNORE_TAG.
 *      On some chips, such as BCM5665, double-tag enable is a system-wide
 *              setting rather than a port setting, so enabling double-
 *              tagging on one port may enable it on all ports.
 */

int
bcm_esw_port_dtag_mode_set(int unit, bcm_port_t port, int mode)
{
    int         dt_mode, ignore_tag;
    int         dt_mode_mask;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    /* Skip PORT_SWITCHED_CHECK for Higig lookup (for q-in-q proxy) */
    if (!(IS_ST_PORT(unit, port) &&
          soc_feature(unit, soc_feature_higig_lookup))) {
        PORT_SWITCHED_CHECK(unit, port);
    }

    dt_mode_mask = BCM_PORT_DTAG_MODE_INTERNAL | 
        BCM_PORT_DTAG_MODE_EXTERNAL;

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) || defined(BCM_TRX_SUPPORT)
    if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit) || \
        SOC_IS_HAWKEYE(unit) || SOC_IS_TRX(unit)) {
        if (mode & ~(BCM_PORT_DTAG_MODE_INTERNAL |
                     BCM_PORT_DTAG_MODE_EXTERNAL |
                     BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG |
                     BCM_PORT_DTAG_ADD_EXTERNAL_TAG)) {
            return BCM_E_UNAVAIL;
        }

        if (IS_ST_PORT(unit, port)) {
            /* Addition or removal of two tags is not supported on 
             * Higig / Higig Lite ports.
             */
            if (mode & BCM_PORT_DTAG_ADD_EXTERNAL_TAG ||
                mode & BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG) {
                return BCM_E_PARAM;
            }
        }

        /* Removal of two tags should be enable only for UNI egress ports. */ 
        if (mode == (BCM_PORT_DTAG_MODE_INTERNAL | 
                     BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG)) {
            return BCM_E_PARAM;
        } 

#ifdef BCM_TRX_SUPPORT
        if (SOC_IS_TRX(unit)) {
            if (!soc_feature(unit, soc_feature_vlan_action)) {
                return BCM_E_UNAVAIL;
            }
            return  _bcm_trx_port_dtag_mode_set(unit, port, mode);
        }
#endif
    } else 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    if (0 != (mode & ~dt_mode_mask)) {
        /* Some devices do not support the add/remove tag options */
        return BCM_E_UNAVAIL;
    }

    switch (mode & dt_mode_mask) {
      case BCM_PORT_DTAG_MODE_NONE:
          dt_mode = 0;
          ignore_tag = 0;
          break;
      case BCM_PORT_DTAG_MODE_INTERNAL:
          dt_mode = 1;
          ignore_tag = 0;
          break;
      case BCM_PORT_DTAG_MODE_EXTERNAL:
          dt_mode = 1;
          ignore_tag = 1;
          break;
      default:
          return BCM_E_PARAM;
    }

#ifdef BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        return _bcm_lynx_port_dtag_mode_set(unit, port, mode, 
                                            dt_mode, ignore_tag);
    }
#endif

#ifdef BCM_DRACO15_SUPPORT
    if (SOC_IS_DRACO15(unit)) {
        return _bcm_draco15_port_dtag_mode_set(unit, port, mode, 
                                               dt_mode, ignore_tag);
    }
#endif

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        return _bcm_tucana_port_dtag_mode_set(unit, port, mode, 
                                              dt_mode, ignore_tag);
    }
#endif

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        return _bcm_fb_port_dtag_mode_set(unit, port, mode, 
                                          dt_mode, ignore_tag);
    }
#endif

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return _bcm_er_port_dtag_mode_set(unit, port, mode, 
                                          dt_mode, ignore_tag);
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_dtag_mode_get
 * Description:
 *      Return the current double-tagging mode of a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - (OUT) Double-tagging mode
 * Return Value:
 *      BCM_E_XXX
 */

int
bcm_esw_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    /* Skip PORT_SWITCHED_CHECK for Higig lookup (for q-in-q proxy) */
    if (!(IS_ST_PORT(unit, port) &&
          soc_feature(unit, soc_feature_higig_lookup))) {
        PORT_SWITCHED_CHECK(unit, port);
    }

    if (mode == NULL) {
        return BCM_E_PARAM;
    }

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        *mode = PORT(unit, port).dtag_mode;
        return BCM_E_NONE;
    }
#endif

#ifdef BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        return _bcm_lynx_port_dtag_mode_get(unit, port, mode);
    }
#endif

#ifdef BCM_DRACO15_SUPPORT
    if (SOC_IS_DRACO15(unit)) {
        return _bcm_draco15_port_dtag_mode_get(unit, port, mode);
    }
#endif

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        return _bcm_tucana_port_dtag_mode_get(unit, port, mode);
    }
#endif

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        return _bcm_fb_port_dtag_mode_get(unit, port, mode);
    }
#endif

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return _bcm_er_port_dtag_mode_get(unit, port, mode);
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_tpid_set
 * Description:
 *      Set the default Tag Protocol ID for a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      tpid - Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      This API is not specifically double-tagging-related, but
 *      the port TPID becomes the service provider TPID when double-tagging
 *      is enabled on a port.  The default TPID is 0x8100.
 *      On BCM5665, only 0x8100 is allowed for the inner (customer) tag.
 */

int
bcm_esw_port_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    int rv = BCM_E_UNAVAIL; 
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t tgid_out;
    int id_out;
    int is_local;

    if (BCM_GPORT_IS_SET(port)) {
        /* Handle virtual ports separetly */
        if (BCM_GPORT_IS_MPLS_PORT(port) || 
            BCM_GPORT_IS_MIM_PORT(port)) {
            if (soc_feature(unit, soc_feature_vlan_ctrl)) {
#if defined(BCM_TRX_SUPPORT)
                PORT_LOCK(unit);
                rv = _bcm_trx_vp_tpid_set(unit, port, tpid);
                PORT_UNLOCK(unit);
#endif
            }
            return rv;
        } else {
            BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, port,
                        &mod_out, &port_out, &tgid_out, &id_out));
            if ((-1 != tgid_out) || (-1 != id_out)) {
                return BCM_E_PARAM;
            }

            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, mod_out, &is_local));
            if (!is_local) {
#ifdef BCM_TRIDENT_SUPPORT
                if (soc_mem_is_valid(unit, SYSTEM_CONFIG_TABLE_MODBASEm)) {
                    return _bcm_td_mod_port_tpid_set(unit, mod_out, port_out,
                            tpid);
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    return BCM_E_PARAM;
                }
            } else {
                BCM_IF_ERROR_RETURN(
                        bcm_esw_port_local_get(unit, port, &port));
            }
        }
    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } 

#if defined (BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) &&
        (PORT(unit, port).dtag_mode == BCM_PORT_DTAG_MODE_EXTERNAL)) {
        /* For DT mode external ports, don't enable any outer TPIDs */
        return BCM_E_NONE;
    }
#endif

#if (defined(BCM_DRACO15_SUPPORT) || defined(BCM_LYNX_SUPPORT) || \
     defined(BCM_TUCANA_SUPPORT))
    if (SOC_IS_DRACO15(unit) || SOC_IS_LYNX(unit) || SOC_IS_TUCANA(unit)) {
        return _bcm_draco_port_tpid_set(unit, port, tpid);
    }
#endif

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        PORT_LOCK(unit);
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) || \
        defined(BCM_RAVEN_SUPPORT)
        if (soc_feature(unit, soc_feature_vlan_ctrl)) {
            rv = _bcm_fb2_port_tpid_set(unit, port, tpid);
        } else 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
        {
            rv = _bcm_fb_port_tpid_set(unit, port, tpid);
        }
        PORT_UNLOCK(unit);
        return rv;
    }
#endif

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return _bcm_er_port_tpid_set(unit, port, tpid);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    if (tpid == 0x8100) {
        return BCM_E_NONE;
    }
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_tpid_get
 * Description:
 *      Retrieve the default Tag Protocol ID for a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      tpid - (OUT) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */

int
bcm_esw_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    int rv = BCM_E_UNAVAIL; 
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t tgid_out;
    int id_out;
    int is_local;

    if (BCM_GPORT_IS_SET(port)) {
        /* Handle virtual ports separetly */
        if (BCM_GPORT_IS_MPLS_PORT(port) || 
            BCM_GPORT_IS_MIM_PORT(port)) {

            if (soc_feature(unit, soc_feature_vlan_ctrl)) {
#if defined(BCM_TRX_SUPPORT)
                PORT_LOCK(unit);
                rv = _bcm_trx_vp_tpid_get(unit, port, tpid);
                PORT_UNLOCK(unit);
#endif
            }
            return rv;
        } else {
            BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, port,
                        &mod_out, &port_out, &tgid_out, &id_out));
            if ((-1 != tgid_out) || (-1 != id_out)) {
                return BCM_E_PARAM;
            }

            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, mod_out, &is_local));
            if (!is_local) {
#ifdef BCM_TRIDENT_SUPPORT
                if (soc_mem_is_valid(unit, SYSTEM_CONFIG_TABLE_MODBASEm)) {
                    return _bcm_td_mod_port_tpid_get(unit, mod_out, port_out,
                            tpid);
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    return BCM_E_PARAM;
                }
            } else {
                BCM_IF_ERROR_RETURN(
                        bcm_esw_port_local_get(unit, port, &port));
            }
        }

    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } 

    if (tpid == NULL) {
        return BCM_E_PARAM;
    }

#if (defined(BCM_DRACO15_SUPPORT) || defined(BCM_LYNX_SUPPORT) || \
     defined(BCM_TUCANA_SUPPORT))
    if (SOC_IS_DRACO15(unit) || SOC_IS_LYNX(unit) || SOC_IS_TUCANA(unit)) {
        return _bcm_draco_port_tpid_get(unit, port, tpid);
    }
#endif /* BCM_DRACO15_SUPPORT || BCM_LYNX_SUPPORT || BCM_TUCANA_SUPPORT */


#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
         PORT_LOCK(unit);
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
        || defined(BCM_RAVEN_SUPPORT)
        if (soc_feature(unit, soc_feature_vlan_ctrl)) {
            rv = _bcm_fb2_port_tpid_get(unit, port, tpid);
        } else
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
        {
            rv = _bcm_fb_port_tpid_get(unit, port, tpid);
        }
        PORT_UNLOCK(unit);
        return rv;
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return  _bcm_er_port_tpid_get(unit, port, tpid);
    }
#endif /* BCM_EASYRIDER_SUPPORT */
    *tpid = 0x8100;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_port_tpid_add
 * Description:
 *      Add allowed TPID for a port.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      tpid         - (IN) Tag Protocol ID
 *      color_select - (IN) Color mode for TPID
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_tpid_add(int unit, bcm_port_t port, 
                      uint16 tpid, int color_select)
{
    int rv = BCM_E_UNAVAIL;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t tgid_out;
    int id_out;
    int is_local;

    if (color_select != BCM_COLOR_PRIORITY && 
        color_select != BCM_COLOR_OUTER_CFI &&
        color_select != BCM_COLOR_INNER_CFI) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(port)) {
        /* Handle virtual ports separetly */
        if (BCM_GPORT_IS_MPLS_PORT(port) || 
            BCM_GPORT_IS_MIM_PORT(port) ||
            BCM_GPORT_IS_TRILL_PORT(port)) {

            if (soc_feature(unit, soc_feature_vlan_ctrl)) {
#if defined(BCM_TRX_SUPPORT)
                PORT_LOCK(unit);
                rv = _bcm_trx_vp_tpid_add(unit, port, tpid, color_select);
                PORT_UNLOCK(unit);
#endif
            }
            return rv;
        } else {
            BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, port,
                        &mod_out, &port_out, &tgid_out, &id_out));
            if ((-1 != tgid_out) || (-1 != id_out)) {
                return BCM_E_PARAM;
            }

            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, mod_out, &is_local));
            if (!is_local) {
#ifdef BCM_TRIDENT_SUPPORT
                if (soc_mem_is_valid(unit, SYSTEM_CONFIG_TABLE_MODBASEm)) {
                    return _bcm_td_mod_port_tpid_add(unit, mod_out, port_out,
                            tpid);
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    return BCM_E_PARAM;
                }
            } else {
                BCM_IF_ERROR_RETURN(
                        bcm_esw_port_local_get(unit, port, &port));
            }
        }

    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } 

#if defined (BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit) &&
        (PORT(unit, port).dtag_mode == BCM_PORT_DTAG_MODE_EXTERNAL)) {
        /* For DT mode external ports, don't enable any outer TPIDs */
        return BCM_E_NONE;
    }
#endif

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
    || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_vlan_ctrl)) {
        PORT_LOCK(unit);
        rv = _bcm_fb2_port_tpid_add(unit, port, tpid, color_select);
        PORT_UNLOCK(unit);
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_tpid_delete
 * Description:
 *      Delete allowed TPID for a port.
 * Parameters:
 *      unit - (IN) Device number
 *      port - (IN) Port number
 *      tpid - (IN) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_tpid_delete(int unit, bcm_port_t port, uint16 tpid)
{
    int rv = BCM_E_UNAVAIL;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t tgid_out;
    int id_out;
    int is_local;

    if (BCM_GPORT_IS_SET(port)) {
        /* Handle virtual ports separetly */
        if (BCM_GPORT_IS_MPLS_PORT(port) || 
            BCM_GPORT_IS_MIM_PORT(port)) {

            if (soc_feature(unit, soc_feature_vlan_ctrl)) {
#if defined(BCM_TRX_SUPPORT)
                PORT_LOCK(unit);
                rv = _bcm_trx_vp_tpid_delete(unit, port, tpid);
                PORT_UNLOCK(unit);
#endif
            }
            return rv;
        } else {
            BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, port,
                        &mod_out, &port_out, &tgid_out, &id_out));
            if ((-1 != tgid_out) || (-1 != id_out)) {
                return BCM_E_PARAM;
            }

            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, mod_out, &is_local));
            if (!is_local) {
#ifdef BCM_TRIDENT_SUPPORT
                if (soc_mem_is_valid(unit, SYSTEM_CONFIG_TABLE_MODBASEm)) {
                    return _bcm_td_mod_port_tpid_delete(unit, mod_out, port_out,
                            tpid);
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    return BCM_E_PARAM;
                }
            } else {
                BCM_IF_ERROR_RETURN(
                        bcm_esw_port_local_get(unit, port, &port));
            }
        }

    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } 

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
    || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_vlan_ctrl)) {
        PORT_LOCK(unit);
        rv = _bcm_fb2_port_tpid_delete(unit, port, tpid);
        PORT_UNLOCK(unit);
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_tpid_delete_all
 * Description:
 *      Delete all allowed TPID for a port.
 * Parameters:
 *      unit - (IN) Device number
 *      port - (IN) Port number
 * Return Value:
 *      BCM_E_XXX
 */
int 
bcm_esw_port_tpid_delete_all(int unit, bcm_port_t port)
{
    int rv = BCM_E_UNAVAIL;
    bcm_module_t mod_out;
    bcm_port_t port_out;
    bcm_trunk_t tgid_out;
    int id_out;
    int is_local;

    if (BCM_GPORT_IS_SET(port)) {
        /* Handle virtual ports separetly */
        if (BCM_GPORT_IS_MPLS_PORT(port) || 
            BCM_GPORT_IS_MIM_PORT(port)) {

            if (soc_feature(unit, soc_feature_vlan_ctrl)) {
#if defined(BCM_TRX_SUPPORT)
                PORT_LOCK(unit);
                rv = _bcm_trx_vp_tpid_delete_all(unit, port);
                PORT_UNLOCK(unit);
#endif
            }
            return rv;
        } else {
            BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, port,
                        &mod_out, &port_out, &tgid_out, &id_out));
            if ((-1 != tgid_out) || (-1 != id_out)) {
                return BCM_E_PARAM;
            }

            BCM_IF_ERROR_RETURN
                (_bcm_esw_modid_is_local(unit, mod_out, &is_local));
            if (!is_local) {
#ifdef BCM_TRIDENT_SUPPORT
                if (soc_mem_is_valid(unit, SYSTEM_CONFIG_TABLE_MODBASEm)) {
                    return _bcm_td_mod_port_tpid_delete_all(unit, mod_out,
                            port_out);
                } else
#endif /* BCM_TRIDENT_SUPPORT */
                {
                    return BCM_E_PARAM;
                }
            } else {
                BCM_IF_ERROR_RETURN(
                        bcm_esw_port_local_get(unit, port, &port));
            }
        }

    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } 

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
 || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_vlan_ctrl)) {
        PORT_LOCK(unit);
        rv = _bcm_fb2_port_tpid_delete_all(unit, port);
        PORT_UNLOCK(unit);
    }
#endif
    return rv;
}

/*
 * Function:
 *      bcm_port_inner_tpid_set
 * Purpose:
 *      Set the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      tpid - Tag Protocol ID
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_esw_port_inner_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_GPORT_IS_SET(port)) {
        /* Handle virtual ports separetly */
        if (BCM_GPORT_IS_MPLS_PORT(port) || 
            BCM_GPORT_IS_MIM_PORT(port)) {

            if (soc_feature(unit, soc_feature_vlan_ctrl)) {
#if defined(BCM_TRX_SUPPORT)
                PORT_LOCK(unit);
                rv = _bcm_trx_vp_tpid_set(unit, port,tpid);
                PORT_UNLOCK(unit);
#endif
            }
            return rv;
        } else {
            BCM_IF_ERROR_RETURN(
                bcm_esw_port_local_get(unit, port, &port));
        }

    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } 


    PORT_SWITCHED_CHECK(unit, port);

#ifdef BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        uint32          rval, orval;

        BCM_IF_ERROR_RETURN(READ_DT_CONFIG1r(unit, port, &rval));
        orval = rval;
        soc_reg_field_set(unit, DT_CONFIG1r, &rval, INNER_TPIDf, tpid);
        if (rval != orval) {
            BCM_IF_ERROR_RETURN(WRITE_DT_CONFIG1r(unit, port, rval));
        }

        return BCM_E_NONE;
    }
#endif

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        uint32 rval;
        BCM_IF_ERROR_RETURN(READ_VLAN_CTRLr(unit, &rval));
        soc_reg_field_set(unit, VLAN_CTRLr, &rval, INNER_TPIDf, tpid);
        BCM_IF_ERROR_RETURN(WRITE_VLAN_CTRLr(unit, rval));
        BCM_IF_ERROR_RETURN(soc_reg_field32_modify(unit, EGR_CONFIGr,
                                                   REG_PORT_ANY, INNER_TPIDf,
                                                   tpid));
        return BCM_E_NONE;
    }
#endif

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return _bcm_er_port_inner_tpid_set(unit, port, tpid);
    }
#endif

    return rv;
}

/*
 * Function:
 *      bcm_port_inner_tpid_get
 * Purpose:
 *      Get the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      tpid - (OUT) Tag Protocol ID
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_esw_port_inner_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_GPORT_IS_SET(port)) {
        /* Handle virtual ports separetly */
        if (BCM_GPORT_IS_MPLS_PORT(port) || 
            BCM_GPORT_IS_MIM_PORT(port)) {

            if (soc_feature(unit, soc_feature_vlan_ctrl)) {
#if defined(BCM_TRX_SUPPORT)
                PORT_LOCK(unit);
                rv = _bcm_trx_vp_tpid_get(unit, port,tpid);
                PORT_UNLOCK(unit);
#endif
            }
            return rv;
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_port_gport_validate(unit, port, &port));
        }

    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    PORT_SWITCHED_CHECK(unit, port);

    if (tpid == NULL) {
        return BCM_E_PARAM;
    }

#ifdef BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        uint32          rval;

        BCM_IF_ERROR_RETURN(READ_DT_CONFIG1r(unit, port, &rval));
        *tpid = soc_reg_field_get(unit, DT_CONFIG1r, rval, INNER_TPIDf);
        return BCM_E_NONE;
    }
#endif

#if defined(BCM_DRACO15_SUPPORT) || defined(BCM_TUCANA_SUPPORT)
    if (SOC_IS_DRACO15(unit) || SOC_IS_TUCANA(unit)) {
        *tpid = 0x8100;
        return BCM_E_NONE;
    }
#endif

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        uint32 rval;
        BCM_IF_ERROR_RETURN(READ_VLAN_CTRLr(unit, &rval));
        *tpid = soc_reg_field_get(unit, VLAN_CTRLr, rval, INNER_TPIDf);
        return BCM_E_NONE;
    }
#endif

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return _bcm_er_port_inner_tpid_get(unit, port, tpid);
    }
#endif

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_frame_max_set
 * Description:
 *      Set the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Depending on chip or port type the actual maximum receive frame size
 *      might be slightly higher.
 *
 *      It looks like this operation is performed the same way on all the chips
 *      and the only depends on the port type.
 */
int
bcm_esw_port_frame_max_set(int unit, bcm_port_t port, int size)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    uint32 rval;
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (size < 0 || size > BCM_PORT_JUMBO_MAXSZ) {
        return BCM_E_PARAM;
    }

    SOC_IF_ERROR_RETURN
        (MAC_FRAME_MAX_SET(PORT(unit, port).p_mac, unit, port, size));

#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        SOC_IF_ERROR_RETURN(READ_EGR_L2_MTUr(unit, port, &rval));
        soc_reg_field_set(unit, EGR_L2_MTUr, &rval,
                          VALUEf, size + 4);
        SOC_IF_ERROR_RETURN(WRITE_EGR_L2_MTUr(unit, port, rval));
    } else
#endif /* BCM_SHADOW_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        soc_reg_t reg;
        uint32 addr;

        reg = SOC_REG_IS_VALID(unit, EGR_MTUr) ? EGR_MTUr : EGR_MTU_SIZEr;
        addr = soc_reg_addr(unit, reg, port, 0);

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, port, 0, &rval));
        soc_reg_field_set(unit, reg, &rval, MTU_SIZEf, size + 4);
        if (soc_reg_field_valid(unit, reg, MTU_ENABLEf)) {
            soc_reg_field_set(unit, reg, &rval, MTU_ENABLEf, 1); 
        } 
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, port, 0, rval));
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        SOC_IF_ERROR_RETURN(READ_EGR_PORT_MTUr(unit, port, &rval));
        soc_reg_field_set(unit, EGR_PORT_MTUr, &rval, MTUf, size + 4);
        SOC_IF_ERROR_RETURN(WRITE_EGR_PORT_MTUr(unit, port, rval));
#ifdef INCLUDE_L3
        SOC_IF_ERROR_RETURN(bcm_er_ipmc_port_mtu_update(unit,port));
#endif /* INCLUDE_L3 */
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_frame_max_get
 * Description:
 *      Get the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Depending on chip or port type the actual maximum receive frame size
 *      might be slightly higher.
 *
 *      For GE ports that use 2 separate MACs (one for GE and another one for
 *      10/100 modes) the function returns the maximum rx frame size set for
 *      the current mode.
 */
int
bcm_esw_port_frame_max_get(int unit, bcm_port_t port, int *size)
{
    int            rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = MAC_FRAME_MAX_GET(PORT(unit, port).p_mac, unit, port, size);

    return rv;
}

/*
 * Function:
 *      bcm_esw_port_l3_mtu_set
 * Description:
 *      Set the maximum L3 frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_l3_mtu_set(int unit, bcm_port_t port, int size)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_egr_l3_mtu)) {
        soc_reg_t reg;

        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
        reg = SOC_REG_IS_VALID(unit, EGR_MTUr) ? EGR_MTUr : EGR_MTU_SIZEr;
        return soc_reg_field32_modify(unit, reg, port, L3_MTU_SIZEf, size); 
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_l3_mtu_get
 * Description:
 *      Get the maximum L3 frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_l3_mtu_get(int unit, bcm_port_t port, int *size)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(INCLUDE_L3)
    if (soc_feature(unit, soc_feature_egr_l3_mtu)) {
        soc_reg_t reg;
        uint32 addr, rval;

        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

        reg = SOC_REG_IS_VALID(unit, EGR_MTUr) ? EGR_MTUr : EGR_MTU_SIZEr;
        addr = soc_reg_addr(unit, reg, port, 0);

        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, port, 0, &rval));
        *size = soc_reg_field_get(unit, reg, rval, L3_MTU_SIZEf);
        return BCM_E_NONE;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT && INCLUDE_L3 */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_ifg_set
 * Description:
 *      Sets the new ifg (Inter-frame gap) value
 * Parameters:
 *      unit   - Device number
 *      port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *      ifg    - Inter-frame gap in bit-times
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      The function makes sure the IFG value makes sense and updates the
 *      IPG register in case the speed/duplex match the current settings
 */
int
bcm_esw_port_ifg_set(int unit, bcm_port_t port,
                 int speed, bcm_port_duplex_t duplex,
                 int ifg)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    return MAC_IFG_SET(PORT(unit, port).p_mac, unit, port,
                       speed, duplex, ifg);
}

/*
 * Function:
 *      bcm_port_ifg_get
 * Description:
 *      Gets the new ifg (Inter-frame gap) value
 * Parameters:
 *      unit   - Device number
 *      port   - Port number
 *      speed  - the speed for which the IFG is being set
 *      duplex - the duplex for which the IFG is being set
 *      ifg    - Inter-frame gap in bit-times
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_esw_port_ifg_get(int unit, bcm_port_t port,
                 int speed, bcm_port_duplex_t duplex,
                 int *ifg)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    return MAC_IFG_GET(PORT(unit, port).p_mac, unit, port,
                       speed, duplex, ifg);
}

/*
 * Function:
 *      bcm_port_phy_get
 * Description:
 *      General PHY register read
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - (OUT) Data that was read
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_port_phy_get(int unit, bcm_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 *phy_data)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint32 reg_flag;
    int    rv;

    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    }

    if (flags & (SOC_PHY_I2C_DATA8 | SOC_PHY_I2C_DATA16)) {
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_read(unit, port, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
        return rv;
    }
    
    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_read(unit, port, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad, 
                                  phy_reg, &phy_rd_data);

        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
        }
        PORT_UNLOCK(unit);

        if (BCM_SUCCESS(rv)) {
           *phy_data = phy_rd_data;
        }
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_esw_port_phy_get: u=%d p=%d flags=0x%08x "
                     "phy_reg=0x%08x, phy_data=0x%08x, rv=%d\n",
                     unit, port, flags, phy_reg_addr, *phy_data, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_phy_set
 * Description:
 *      General PHY register write
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - Data to write
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_port_phy_set(int unit, bcm_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 phy_data)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;

    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_esw_port_phy_set: u=%d p=%d flags=0x%08x "
                     "phy_reg=0x%08x phy_data=0x%08x\n",
                     unit, port, flags, phy_reg_addr, phy_data));

    if (flags & (SOC_PHY_I2C_DATA8 | SOC_PHY_I2C_DATA16)) {
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_write(unit, port, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
        return rv;
    }

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_write(unit, port, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        phy_wr_data = (uint16) (phy_data & 0xffff);
        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_write(unit, phy_id, phy_devad, 
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
        }
        PORT_UNLOCK(unit);
    }
    return rv;
}

/*
 * Function:
 *      bcm_port_phy_modify
 * Description:
 *      General PHY register modify
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - Data to write
 *      phy_mask - Bits to modify using phy_data
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_port_phy_modify(int unit, bcm_port_t port, uint32 flags,
                        uint32 phy_reg_addr, uint32 phy_data, uint32 phy_mask)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;

    if (!(flags & BCM_PORT_PHY_NOMAP)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_esw_port_phy_modify: u=%d p=%d flags=0x%08x "
                     "phy_reg=0x%08x phy_data=0x%08x phy_mask=0x%08x\n",
                     unit, port, flags, phy_reg_addr, phy_data, phy_mask));

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_modify(unit, port, flags, phy_reg_addr,
                                    phy_data, phy_mask);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        phy_wr_data = (uint16) (phy_data & phy_mask & 0xffff);
        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad, 
                                  phy_reg, &phy_rd_data);
            phy_wr_data |= (phy_rd_data & ~phy_mask);
            rv = soc_miimc45_write(unit, phy_id, phy_devad, 
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
            if (BCM_SUCCESS(rv)) {
                phy_wr_data |= (phy_rd_data & ~phy_mask);
                rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
            }
        }
        PORT_UNLOCK(unit);
    }
    return rv;
}

/*
 * Function:
 *      bcm_port_mdix_set
 * Description:
 *      Set the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - One of:
 *              BCM_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              BCM_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              BCM_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              BCM_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      BCM_E_UNAVAIL - feature unsupported by hardware
 *      BCM_E_XXX - other error
 */
int
bcm_esw_port_mdix_set(int unit, bcm_port_t port, bcm_port_mdix_t mode)
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_mdix_set(unit, port, mode);
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_mdix_get
 * Description:
 *      Get the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - (Out) One of:
 *              BCM_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              BCM_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              BCM_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              BCM_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      BCM_E_UNAVAIL - feature unsupported by hardware
 *      BCM_E_XXX - other error
 */
int
bcm_esw_port_mdix_get(int unit, bcm_port_t port, bcm_port_mdix_t *mode)
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_mdix_get(unit, port, mode);
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_mdix_status_get
 * Description:
 *      Get the current MDIX status on a port/PHY
 * Parameters:
 *      unit    - Device number
 *      port    - Port number
 *      status  - (OUT) One of:
 *              BCM_PORT_MDIX_STATUS_NORMAL
 *                      Straight connection
 *              BCM_PORT_MDIX_STATUS_XOVER
 *                      Crossover has been performed
 * Return Value:
 *      BCM_E_UNAVAIL - feature unsupported by hardware
 *      BCM_E_XXX - other error
 */
int
bcm_esw_port_mdix_status_get(int unit, bcm_port_t port,
                         bcm_port_mdix_status_t *status)
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_mdix_status_get(unit, port, status);
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_medium_config_set
 * Description:
 *      Set the medium-specific configuration for a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 to apply the configuration to
 *      config   - per-medium configuration
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_medium_config_set(int unit, bcm_port_t port,
                           bcm_port_medium_t medium,
                           bcm_phy_config_t  *config)
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    
    PORT_LOCK(unit);
    rv = soc_phyctrl_medium_config_set(unit, port, medium, config);
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_medium_config_get
 * Description:
 *      Get the medium-specific configuration for a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 to get the config for
 *      config   - per-medium configuration
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_medium_config_get(int unit, bcm_port_t port,
                           bcm_port_medium_t  medium,
                           bcm_phy_config_t  *config)
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_medium_config_get(unit, port, medium, config);
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_medium_get
 * Description:
 *      Get the current medium used by a combo port
 * Parameters:
 *      unit     - Device number
 *      port     - Port number
 *      medium   - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *                 which is currently selected
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_medium_get(int unit, bcm_port_t port,
                    bcm_port_medium_t *medium)
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_medium_get(unit, port, medium);
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_medium_status_register
 * Description:
 *      Register a callback function to be called on medium change event
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM  -- NULL function pointer or bad {unit, port} combination
 *      BCM_E_FULL   -- Cannot register more than 1 callback per {unit, port}
 */
int
bcm_esw_port_medium_status_register(int                          unit,
                                bcm_port_t                   port,
                                bcm_port_medium_status_cb_t  callback,
                                void                        *user_data)
{
    return soc_phy_medium_status_register(unit, port, callback, user_data);
}

/*
 * Function:
 *      bcm_port_medium_status_unregister
 * Description:
 *      Unregister a callback function to be called on medium change event
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM       -- Bad {unit, port} combination
 *      BCM_E_NOT_FOUND   -- The specified {unit, port, callback, user_data}
 *                           combination have not been registered before
 */
int
bcm_esw_port_medium_status_unregister(int                          unit,
                                  bcm_port_t                   port,
                                  bcm_port_medium_status_cb_t  callback,
                                  void                        *user_data)
{
    return soc_phy_medium_status_unregister(unit, port, callback, user_data);
}

/*
 * Function:
 *      bcm_port_phy_reset
 * Description:
 *      This function performs the low-level PHY reset and is intended to be
 *      called ONLY from callback function registered with
 *      bcm_port_phy_reset_register. Attempting to call it from any other
 *      place will break lots of things.
 * Parameters:
 *      unit    - Device number
 *      port    - Port number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_port_phy_reset(int unit, bcm_port_t port)
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_reset(unit, port, NULL);
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_port_phy_reset_register
 * Description:
 *      Register a callback function to be called whenever a PHY driver
 *      needs to perform a PHY reset
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM       -- Bad {unit, port} combination
 *      BCM_E_NOT_FOUND   -- The specified {unit, port, callback, user_data}
 *                           combination have not been registered before
 */
int
bcm_esw_port_phy_reset_register(int unit, bcm_port_t port,
                            bcm_port_phy_reset_cb_t callback,
                            void  *user_data)
{
    return soc_phy_reset_register(unit, port, callback, user_data, FALSE);
}

/*
 * Function:
 *      bcm_port_phy_reset_unregister
 * Description:
 *      Unregister a callback function to be called whenever a PHY driver
 *      needs to perform a PHY reset
 * Parameters:
 *      unit      - Device number
 *      port      - port number
 *      callback  - The callback function to call
 *      user_data - An opaque cookie to pass to callback function
 *                  whenever it is called
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_PARAM       -- Bad {unit, port} combination
 *      BCM_E_NOT_FOUND   -- The specified {unit, port, callback, user_data}
 *                           combination have not been registered before
 */
int
bcm_esw_port_phy_reset_unregister(int unit, bcm_port_t port,
                              bcm_port_phy_reset_cb_t callback,
                              void  *user_data)
{
    return soc_phy_reset_unregister(unit, port, callback, user_data);
}

/*
 * Function:
 *      bcm_port_jam_set
 * Description:
 *      Enable or disable half duplex jamming on a port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      enable - non-zero to enable jamming
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_jam_set(int unit, bcm_port_t port, int enable)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (!IS_E_PORT(unit, port)) {       /* HG, CPU ports */
        return enable ? BCM_E_CONFIG : BCM_E_NONE;
    }

#if defined(BCM_DRACO_SUPPORT)
    if (SOC_IS_DRACO(unit)) {
        uint32  config, oconfig;

        SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
        oconfig = config;
        soc_reg_field_set(unit, CONFIGr, &config, JAM_ENf, enable ? 1 : 0);
        if (config != oconfig) {
            SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config));
        }
        return BCM_E_NONE;
    }
#endif

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        port_tab_entry_t        pent;
        soc_mem_t               mem;
        int                     oenable;

        mem = SOC_PORT_MEM_TAB(unit, port);
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                         SOC_PORT_MOD_OFFSET(unit, port),
                                         &pent));
        oenable = soc_PORT_TABm_field32_get(unit, &pent, JAM_ENf);
        enable = enable ? 1 : 0;
        if (oenable != enable) {
            soc_PORT_TABm_field32_set(unit, &pent, JAM_ENf, enable);
            SOC_IF_ERROR_RETURN(soc_mem_write(unit, mem, MEM_BLOCK_ANY,
                                              SOC_PORT_MOD_OFFSET(unit, port),
                                              &pent));
        }
        return BCM_E_NONE;
    }
#endif

#ifdef  BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        uint32                  rval;
        int                     oenable;

        if (!IS_GE_PORT(unit, port) && !IS_FE_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        } else if (!soc_reg_field_valid(unit, GE_PORT_CONFIGr, JAM_ENf) &&
                   !soc_feature(unit, soc_feature_unimac)) {
            return BCM_E_UNAVAIL;
        } 

        if (soc_feature(unit, soc_feature_unimac)) { 
            SOC_IF_ERROR_RETURN(READ_IPG_HD_BKP_CNTLr(unit, port, &rval));
            oenable = soc_reg_field_get(unit, IPG_HD_BKP_CNTLr, rval, 
                                        HD_FC_ENAf);
#ifdef BCM_HAWKEYE_SUPPORT
            if(SOC_IS_HAWKEYE(unit)) {
                /* 
                * In HAWKEYE A0, the back-pressure is disabled 
                *      when the bit HD_FC_ENA is cleared (binary 0).
                * In other revision of HAWKEYE, the back-pressure is disabled 
                *      when the bit HD_FC_ENA is set (binary 1).
                */
                if(soc_feature(unit, soc_feature_hawkeye_a0_war)) {
                    enable = enable ? 1 : 0;
                } else {
                    enable = enable ? 0 : 1;
                }
            } else
#endif /* BCM_HAWKEYE_SUPPORT */
            {
                enable = enable ? 1 : 0;
            }
            if (oenable != enable) {
                soc_reg_field_set(unit, IPG_HD_BKP_CNTLr, &rval, HD_FC_ENAf, 
                                  enable);
                SOC_IF_ERROR_RETURN(WRITE_IPG_HD_BKP_CNTLr(unit, port, rval));
            }
        } else {
            SOC_IF_ERROR_RETURN(READ_GE_PORT_CONFIGr(unit, port, &rval));
            oenable = soc_reg_field_get(unit, GE_PORT_CONFIGr, rval, JAM_ENf);
            enable = enable ? 1 : 0;
            if (oenable != enable) {
                soc_reg_field_set(unit, GE_PORT_CONFIGr, &rval, JAM_ENf, enable);
                SOC_IF_ERROR_RETURN(WRITE_GE_PORT_CONFIGr(unit, port, rval));
            }
        }
        return BCM_E_NONE;
    }
#endif

    /* LYNX XE ports */
    return enable ? BCM_E_CONFIG : BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_jam_get
 * Description:
 *      Return half duplex jamming state
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      enable - (OUT) non-zero if jamming enabled
 * Return Value:
 *      BCM_E_XXX
 */
int
bcm_esw_port_jam_get(int unit, bcm_port_t port, int *enable)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (!IS_E_PORT(unit, port)) {       /* HG, CPU ports */
        *enable = 0;
        return BCM_E_NONE;
    }

#if defined(BCM_DRACO_SUPPORT)
    if (SOC_IS_DRACO(unit)) {
        uint32  config;

        SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
        *enable = soc_reg_field_get(unit, CONFIGr, config, JAM_ENf);
        return BCM_E_NONE;
    }
#endif

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        port_tab_entry_t        pent;
        soc_mem_t               mem;

        mem = SOC_PORT_MEM_TAB(unit, port);
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                         SOC_PORT_MOD_OFFSET(unit, port),
                                         &pent));
        *enable = soc_PORT_TABm_field32_get(unit, &pent, JAM_ENf);
        return BCM_E_NONE;
    }
#endif

#ifdef  BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        uint32                  rval;

        if (!IS_GE_PORT(unit, port) && !IS_FE_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        } else if (!soc_reg_field_valid(unit, GE_PORT_CONFIGr, JAM_ENf) &&
                   !soc_feature(unit, soc_feature_unimac)) {
            return BCM_E_UNAVAIL;
        } 

        if (soc_feature(unit, soc_feature_unimac)) {
            SOC_IF_ERROR_RETURN(READ_IPG_HD_BKP_CNTLr(unit, port, &rval));
            *enable = soc_reg_field_get(unit, IPG_HD_BKP_CNTLr, rval, HD_FC_ENAf);
        } else {
            SOC_IF_ERROR_RETURN(READ_GE_PORT_CONFIGr(unit, port, &rval));
            *enable = soc_reg_field_get(unit, GE_PORT_CONFIGr, rval, JAM_ENf);
        }
        return BCM_E_NONE;
    }
#endif

    /* LYNX XE ports */
    *enable = 0;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_cable_diag
 * Description:
 *      Run Cable Diagnostics on port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      status - (OUT) cable diag status structure
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Cable diagnostics are only supported by some phy types
 *      (currently 5248 10/100 phy and 546x 10/100/1000 phys)
 */
int
bcm_esw_port_cable_diag(int unit, bcm_port_t port,
                    bcm_port_cable_diag_t *status)
{
    int rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit); 
    rv = soc_phyctrl_cable_diag(unit, port, status);
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_esw_port_fault_get
 * Description:
 *      Get link fault type 
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - (OUT) flags to indicate fault type 
 * Return Value:
 *      BCM_E_XXX
 */

int
bcm_esw_port_fault_get(int unit, bcm_port_t port, uint32 *flags)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    *flags = 0; 
    if (soc_feature(unit, soc_feature_bigmac_fault_stat) &&
        (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port))) {
        uint64 lss;
        int lcl_fault, rmt_fault;
        soc_reg_t reg;
        soc_field_t rmt_fault_field, lcl_fault_field;

        if (soc_feature(unit, soc_feature_xmac)) {
            reg = XMAC_RX_LSS_STATUSr;
            rmt_fault_field = REMOTE_FAULT_STATUSf;
            lcl_fault_field = LOCAL_FAULT_STATUSf;
        } else {
            if (SOC_IS_LYNX(unit)) {
                /*
                 * This will "work" on Lynx A0, which does not have a register
                 * by this name. Rather, the resultant register address points
                 * to an unused register on Lynx A0, which always returns zero.
                 * On Lynx B0 and Lynx 1.5, a real MAC_RXLSSCTRL register is
                 * implemented.
                 */
                reg = MAC_RXLSSCTRLr;
            } else {
                reg = MAC_RXLSSSTATr;
            }
            rmt_fault_field = REMOTEFAULTSTATf;
            lcl_fault_field = LOCALFAULTSTATf;
        }
        SOC_IF_ERROR_RETURN
            (soc_reg_get(unit, reg, port, 0, &lss));
        rmt_fault = soc_reg64_field32_get(unit, reg, lss, rmt_fault_field);
        lcl_fault = soc_reg64_field32_get(unit, reg, lss, lcl_fault_field);

        if (rmt_fault) { 
            *flags |= BCM_PORT_FAULT_REMOTE; 
        }
        if (lcl_fault) {
            *flags |= BCM_PORT_FAULT_LOCAL; 
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_port_trunk_index_set
 * Description:
 *      Set port index of a trunk for ingress port that is used to select 
 *      the egress port in the trunk.
 * Parameters:
 *      unit       - StrataSwitch PCI device unit number (driver internal).
 *      port       - Ingress port.
 *      port_index - Port index of a trunk.
 * Returns:
 *      BCM_E_xxxx
 * Note:
 *      The psc (aka rtag) vlaue needs to be 7 (set in bcm_trunk_set()) for 
 *      a trunk in order to utilize this programmable hashing mechanism.   
 */

int
bcm_esw_port_trunk_index_set(int unit, bcm_port_t port, int port_index)
{
#if defined(BCM_FIREBOLT_SUPPORT) || defined(BCM_EASYRIDER_SUPPORT)
    uint32 val, mask = 0x7;

    if (soc_feature(unit, soc_feature_port_trunk_index)) {
        if (IS_ST_PORT(unit, port)) {
            val = 0;
            if (!SOC_IS_RAVEN(unit)) {
                mask = 0x3;
            }
            soc_reg_field_set(unit, IUSER_TRUNK_HASH_SELECTr, &val,
                              TRUNK_CFG_VALf, port_index & mask);
            SOC_IF_ERROR_RETURN
                (WRITE_IUSER_TRUNK_HASH_SELECTr(unit, port, val));
        } else if (IS_E_PORT(unit, port)) {
            val = 0;
            soc_reg_field_set(unit, USER_TRUNK_HASH_SELECTr, &val,
                              TRUNK_CFG_VALf, port_index & mask);
            SOC_IF_ERROR_RETURN
                (WRITE_USER_TRUNK_HASH_SELECTr(unit, port, val));
        }

        return BCM_E_NONE;
    }
#endif  /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_trunk_index_get 
 * Description:
 *      Get port index of a trunk for ingress port that is used to select 
 *      the egress port in the trunk.
 * Parameters:
 *      unit       - StrataSwitch PCI device unit number (driver internal).
 *      port       - Ingress port.
 *      port_index - (OUT) Port index of a trunk.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_esw_port_trunk_index_get(int unit, bcm_port_t port, int *port_index)
{
#if defined(BCM_FIREBOLT_SUPPORT) || defined(BCM_EASYRIDER_SUPPORT)
    uint32 val;

    if (soc_feature(unit, soc_feature_port_trunk_index)) {
        if (IS_ST_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN
               (READ_IUSER_TRUNK_HASH_SELECTr(unit, port, &val));
            *port_index = soc_reg_field_get(unit, IUSER_TRUNK_HASH_SELECTr,
                                            val, TRUNK_CFG_VALf);
        } else if (IS_E_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN
               (READ_USER_TRUNK_HASH_SELECTr(unit, port, &val));
            *port_index = soc_reg_field_get(unit, USER_TRUNK_HASH_SELECTr,
                                            val, TRUNK_CFG_VALf);
        }

        return BCM_E_NONE;
    }
#endif  /* BCM_FIREBOLT_SUPPORT || BCM_EASYRIDER_SUPPORT */

    return BCM_E_UNAVAIL;
}

#ifdef BCM_XGS3_SWITCH_SUPPORT

static soc_field_t _bcm_xgs3_priority_fields[] = {
    PRIORITY0_CNGf,
    PRIORITY1_CNGf,
    PRIORITY2_CNGf,
    PRIORITY3_CNGf,
    PRIORITY4_CNGf,
    PRIORITY5_CNGf,
    PRIORITY6_CNGf,
    PRIORITY7_CNGf,
};

#endif /* BCM_XGS3_SWITCH_SUPPORT */

/*
 * Function:
 *      bcm_port_priority_color_set
 * Purpose:
 *      Specify the color selection for the given priority.
 * Parameters:
 *      unit -  StrataSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      prio -  priority (aka 802.1p CoS)
 *      color - color assigned to packets with indicated priority.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int
bcm_esw_port_priority_color_set(int unit, bcm_port_t port,
                                int prio, bcm_color_t color)
{
#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_color)) {

        VLAN_CHK_PRIO(unit, prio);

        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_TRIUMPH2_SUPPORT
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            return bcm_td_port_ing_pri_cng_set(unit, port, FALSE, prio, -1,
                                               -1, color); 
        } else
#endif /* BCM_TRIDENT_SUPPORT */
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) 
            || SOC_IS_HURRICANE(unit)) {

            return _bcm_tr2_port_priority_color_set(unit, port, prio, color); 

        } else
#endif
        {
            uint32 val, oval;

            SOC_IF_ERROR_RETURN(READ_CNG_MAPr(unit, port, &val));
            oval = val;
            soc_reg_field_set(unit, CNG_MAPr, &val,
                              _bcm_xgs3_priority_fields[prio],
                              _BCM_COLOR_ENCODING(unit, color));
            if (oval != val) {
                SOC_IF_ERROR_RETURN(WRITE_CNG_MAPr(unit, port, val));
            }
            return BCM_E_NONE;
        }
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_priority_color_get
 * Purpose:
 *      Get the color selection for the given priority.
 * Parameters:
 *      unit -  StrataSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      prio -  priority (aka 802.1p CoS)
 *      color - (OUT) color assigned to packets with indicated priority.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int
bcm_esw_port_priority_color_get(int unit, bcm_port_t port,
                                int prio, bcm_color_t *color)
{
#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_color)) {
        uint32 val, hw_color;
        int ptr;

        VLAN_CHK_PRIO(unit, prio);
        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
        ptr = port;

#ifdef BCM_TRIUMPH2_SUPPORT
#if defined BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            return bcm_td_port_ing_pri_cng_get(unit, port, FALSE, prio, 0,
                                               NULL, color);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) 
            || SOC_IS_HURRICANE(unit)) {
            /* Get the pointer from the ING_PRI_CNG_MAP table */
            port_tab_entry_t pent;
            BCM_IF_ERROR_RETURN(soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY,
                                SOC_PORT_MOD_OFFSET(unit, port), &pent));
            ptr = soc_PORT_TABm_field32_get(unit, &pent, TRUST_DOT1P_PTRf);
        }
#endif
        SOC_IF_ERROR_RETURN(READ_CNG_MAPr(unit, ptr, &val));
        hw_color = soc_reg_field_get(unit, CNG_MAPr, val,
                                     _bcm_xgs3_priority_fields[prio]);
        *color = _BCM_COLOR_DECODING(unit, hw_color);
        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_cfi_color_set
 * Purpose:
 *      Specify the color selection for the given CFI.
 * Parameters:
 *      unit -  StrataSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      cfi -   Canonical format indicator (TRUE/FALSE) 
 *      color - color assigned to packets with indicated CFI.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int
bcm_esw_port_cfi_color_set(int unit, bcm_port_t port,
                           int cfi, bcm_color_t color)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_color)) {
        if (cfi < 0 || cfi > 1) {
            return BCM_E_PARAM;
        }

#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            return bcm_td_port_ing_pri_cng_set(unit, port, FALSE, -1, cfi, -1,
                                               color);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined (BCM_TRX_SUPPORT)
        if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit) || SOC_IS_TRX(unit) || SOC_IS_HAWKEYE(unit)) {
            int                     index, pkt_pri;
            ing_pri_cng_map_entry_t pri_map;
     
            sal_memset(&pri_map, 0, sizeof(pri_map));
            for (pkt_pri = 0; pkt_pri <= 7; pkt_pri++) {
                /* ING_PRI_CNG_MAP table is indexed with 
                 * port[0:4] incoming priority[2:0] incoming CFI[0]
                 */
                index = (port << 4) | (pkt_pri << 1) | cfi;
                soc_mem_field32_set(unit, ING_PRI_CNG_MAPm, &pri_map, PRIf, 
                                    pkt_pri);
                soc_mem_field32_set(unit, ING_PRI_CNG_MAPm, &pri_map, CNGf, 
                                    _BCM_COLOR_ENCODING(unit, color));
                SOC_IF_ERROR_RETURN(WRITE_ING_PRI_CNG_MAPm(unit, MEM_BLOCK_ALL,
                                                           index, &pri_map));
            }
        } else 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
        {
            uint32 val, oval;
            SOC_IF_ERROR_RETURN(READ_CNG_MAPr(unit, port, &val));
            oval = val;
            soc_reg_field_set(unit, CNG_MAPr, &val,
                              cfi ? CFI1_CNGf : CFI0_CNGf,
                             _BCM_COLOR_ENCODING(unit, color));
            if (oval != val) {
                SOC_IF_ERROR_RETURN(WRITE_CNG_MAPr(unit, port, val));
            }
        }
        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_cfi_color_get
 * Purpose:
 *      Get the color selection for the given CFI.
 * Parameters:
 *      unit -  StrataSwitch PCI device unit number (driver internal).
 *      port -  Port to configure
 *      cfi -   Canonical format indicator (TRUE/FALSE) 
 *      color - (OUT) color assigned to packets with indicated CFI.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int
bcm_esw_port_cfi_color_get(int unit, bcm_port_t port,
                           int cfi, bcm_color_t *color)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_color)) {
        uint32 hw_color;

        if (cfi < 0 || cfi > 1) {
            return BCM_E_PARAM;
        }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        return bcm_td_port_ing_pri_cng_get(unit, port, FALSE, 0, cfi, NULL,
                                           color);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined(BCM_TRX_SUPPORT)
        if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit)
            || SOC_IS_TRX(unit) || SOC_IS_HAWKEYE(unit)) {
            int index;
            ing_pri_cng_map_entry_t pri_map;
            /* ING_PRI_CNG_MAP table is indexed with 
             * port[0:4] incoming priority[2:0] incoming CFI[0]
             */
            index = (port << 4) | cfi; 
            SOC_IF_ERROR_RETURN(READ_ING_PRI_CNG_MAPm(unit, MEM_BLOCK_ANY, 
                                                      index, &pri_map));
            hw_color = soc_mem_field32_get(unit, ING_PRI_CNG_MAPm, 
                                           &pri_map, CNGf);
        } else 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT */
        {
            uint32 val;

            SOC_IF_ERROR_RETURN(READ_CNG_MAPr(unit, port, &val));
            hw_color = soc_reg_field_get(unit, CNG_MAPr, val,
                                         cfi ? CFI1_CNGf : CFI0_CNGf);
        }
        *color = _BCM_COLOR_DECODING(unit, hw_color);
        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_vlan_priority_map_set
 * Description:
 *      Define the mapping of incomming port, packet priority, and cfi bit to
 *      switch internal priority and color.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      pkt_pri      - (IN) Packet priority
 *      cfi          - (IN) Packet CFI
 *      internal_pri - (IN) Internal priority
 *      color        - (IN) color
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      This API programs only the mapping table. 
 */
int
bcm_esw_port_vlan_priority_map_set(int unit, bcm_port_t port, int pkt_pri,
                                   int cfi, int internal_pri, bcm_color_t color)
{
    int untagged;

    if (!soc_feature(unit, soc_feature_color_prio_map)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    untagged = FALSE;
    if (SOC_MEM_IS_VALID(unit, ING_UNTAGGED_PHBm)) {
        if (pkt_pri == -1 && cfi == -1) {
            untagged = TRUE;
            pkt_pri = 0;
            cfi = 0;
        }
    }

    if (pkt_pri < 0 || pkt_pri > 7 || cfi < 0 || cfi > 1 || internal_pri < 0 ||
        internal_pri >=
        (1 << soc_mem_field_length(unit, ING_PRI_CNG_MAPm, PRIf))) {
        return BCM_E_PARAM;
    }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined(BCM_TRX_SUPPORT)
#if defined(BCM_TRIUMPH2_SUPPORT)
#if defined BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        return bcm_td_port_ing_pri_cng_set(unit, port, untagged, pkt_pri, cfi,
                                           internal_pri, color);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) || 
        SOC_IS_HURRICANE(unit)) {

         return _bcm_tr2_port_vlan_priority_map_set(unit, port, pkt_pri, cfi,
                                                    internal_pri, color);
    } else
#endif
    if (soc_feature(unit, soc_feature_color_prio_map)) {
        int                      index;
        ing_pri_cng_map_entry_t  pri_map;

        /* ING_PRI_CNG_MAP table is indexed with
         * port[0:4] incoming priority[2:0] incoming CFI[0]
         */
        index = (port << 4) | (pkt_pri << 1) | cfi;

        memset(&pri_map, 0, sizeof(pri_map));
        soc_mem_field32_set(unit, ING_PRI_CNG_MAPm, &pri_map, PRIf, 
                            internal_pri);
        soc_mem_field32_set(unit, ING_PRI_CNG_MAPm, &pri_map, CNGf, 
                            _BCM_COLOR_ENCODING(unit, color));
        SOC_IF_ERROR_RETURN
            (WRITE_ING_PRI_CNG_MAPm(unit, MEM_BLOCK_ALL, index, &pri_map));
        return BCM_E_NONE;
   }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_vlan_priority_map_get
 * Description:
 *      Get the mapping of incomming port, packet priority, and cfi bit to
 *      switch internal priority and color.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      pkt_pri      - (IN) Packet priority
 *      cfi          - (IN) Packet CFI
 *      internal_pri - (OUT) Internal priority
 *      color        - (OUT) color
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      This API programs only the mapping table. 
 */
int
bcm_esw_port_vlan_priority_map_get(int unit, bcm_port_t port, int pkt_pri,
                                   int cfi, int *internal_pri, 
                                   bcm_color_t *color)
{
    int untagged;
    
    if (!soc_feature(unit, soc_feature_color_prio_map)) {
        return BCM_E_UNAVAIL;
    }

    /* Input parameters check. */
    if ((NULL == internal_pri) || (NULL == color)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    untagged = FALSE;
    if (SOC_MEM_IS_VALID(unit, ING_UNTAGGED_PHBm)) {
        if (pkt_pri == -1 && cfi == -1) {
            untagged = TRUE;
            pkt_pri = 0;
            cfi = 0;
        }
    }

    if (pkt_pri < 0 || pkt_pri > 7 || cfi < 0 || cfi > 1) {
        return BCM_E_PARAM;
    }

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined(BCM_TRX_SUPPORT)
#if defined(BCM_TRIUMPH2_SUPPORT)
#if defined BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
        return bcm_td_port_ing_pri_cng_get(unit, port, untagged, pkt_pri, cfi,
                                           internal_pri, color);
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) || 
        SOC_IS_HURRICANE(unit)) {
        return _bcm_tr2_port_vlan_priority_map_get(unit, port, pkt_pri,
                                                   cfi, internal_pri, color);
    } else
#endif
    if (soc_feature(unit, soc_feature_color_prio_map)) {
        int index, hw_color;
        ing_pri_cng_map_entry_t pri_map;

        /* ING_PRI_CNG_MAP table is indexed with
         * port[4:0] incoming priority[2:0] incoming CFI[0]
         */
        index = (port << 4) | (pkt_pri << 1) | cfi;

        SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_esw_port_vlan_priority_map_get: u=%d p=%d index=%d\n",
                         unit, port, index));
 
        SOC_IF_ERROR_RETURN
            (READ_ING_PRI_CNG_MAPm(unit, MEM_BLOCK_ANY, index, &pri_map));
        *internal_pri = soc_mem_field32_get(unit, ING_PRI_CNG_MAPm, &pri_map, 
                                           PRIf);
        hw_color = soc_mem_field32_get(unit, ING_PRI_CNG_MAPm, &pri_map, CNGf);
        *color = _BCM_COLOR_DECODING(unit, hw_color);
        return BCM_E_NONE;
   }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    return BCM_E_UNAVAIL;

}

/*
 * Function:
 *      bcm_esw_port_vlan_priority_unmap_set
 * Description:
 *      Define the mapping of outgoing port, internal priority, and color to
 *  outgoing packet priority and cfi bit.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      internal_pri - (IN) Internal priority
 *      color        - (IN) Color
 *      pkt_pri      - (IN) Packet priority
 *      cfi          - (IN) Packet CFI
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      This API programs only the mapping table. 
 */
int
bcm_esw_port_vlan_priority_unmap_set(int unit, bcm_port_t port, 
                                     int internal_pri, bcm_color_t color,
                                     int pkt_pri, int cfi)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined(BCM_TRX_SUPPORT)
    if (soc_feature(unit, soc_feature_color_prio_map)) {
        int index, int_pri_shift;
        egr_pri_cng_map_entry_t  pri_unmap;

        int_pri_shift = SOC_IS_TRX(unit) ? 4 : 3;
        if ((internal_pri < 0 || internal_pri >= (1 << int_pri_shift)) || 
            ((color != bcmColorGreen) && (color != bcmColorYellow) &&
             (color != bcmColorRed))) {
            return BCM_E_PARAM;
        }
        if (pkt_pri < 0 || pkt_pri > 7 || cfi < 0 || cfi > 1) {
            return BCM_E_PARAM;
        }

        /* EGR_PRI_CNG_MAP table is indexed with
         * port[4:0] priority[2:0] CNG [1:0]
         * port[5:0] priority[3:0] CNG [1:0] (TRX)
         */
        index = (port << (int_pri_shift + 2)) | (internal_pri << 2) |
            _BCM_COLOR_ENCODING(unit, color);

        memset(&pri_unmap, 0, sizeof(pri_unmap));
        soc_mem_field32_set(unit, EGR_PRI_CNG_MAPm, &pri_unmap, PRIf, pkt_pri);
        soc_mem_field32_set(unit, EGR_PRI_CNG_MAPm, &pri_unmap, CFIf, cfi);
        SOC_IF_ERROR_RETURN
            (WRITE_EGR_PRI_CNG_MAPm(unit, MEM_BLOCK_ALL, index, &pri_unmap));

        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            egr_map_mh_entry_t  egr_map;

            sal_memset(&egr_map, 0, sizeof(egr_map));
            index = (port << int_pri_shift) | (internal_pri) ;
            soc_mem_field32_set(unit, EGR_MAP_MHm, &egr_map, HG_TCf, internal_pri);
            SOC_IF_ERROR_RETURN(WRITE_EGR_MAP_MHm(unit, MEM_BLOCK_ALL, index, &egr_map));
        }

        return BCM_E_NONE;
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_vlan_priority_unmap_get
 * Description:
 *      Get the mapping of outgoing port, internal priority, and color to
 *  outgoing packet priority and cfi bit.
 * Parameters:
 *      unit         - (IN) Device number
 *      port         - (IN) Port number
 *      internal_pri - (IN) Internal priority
 *      color        - (IN) Color
 *      pkt_pri      - (OUT) Packet priority
 *      cfi          - (OUT) Packet CFI
 * Return Value:
 *      BCM_E_XXX
 * Note:
 *      This API programs only the mapping table. 
 */
int
bcm_esw_port_vlan_priority_unmap_get(int unit, bcm_port_t port, 
                                     int internal_pri, bcm_color_t color,
                                     int *pkt_pri, int *cfi)
{
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) \
        || defined(BCM_TRX_SUPPORT)
    if (soc_feature(unit, soc_feature_color_prio_map)) {
        int index, int_pri_shift;
        egr_pri_cng_map_entry_t  pri_unmap;

        int_pri_shift = SOC_IS_TRX(unit) ? 4 : 3;
        if ((internal_pri < 0 || internal_pri >= (1 << int_pri_shift)) || 
            ((color != bcmColorGreen) && (color != bcmColorYellow) &&
             (color != bcmColorRed))) {
            return BCM_E_PARAM;
        }

        /* EGR_PRI_CNG_MAP table is indexed with
         * port[4:0] priority[2:0] CNG [1:0]
         * port[5:0] priority[3:0] CNG [1:0] (TRX)
         */
        index = (port << (int_pri_shift + 2)) | (internal_pri << 2) | 
            _BCM_COLOR_ENCODING(unit, color);

        SOC_IF_ERROR_RETURN
            (READ_EGR_PRI_CNG_MAPm(unit, MEM_BLOCK_ANY, index, &pri_unmap));
        *pkt_pri = soc_mem_field32_get(unit, EGR_PRI_CNG_MAPm, 
                                       &pri_unmap, PRIf);
        *cfi = soc_mem_field32_get(unit, EGR_PRI_CNG_MAPm, &pri_unmap, CFIf);
        return BCM_E_NONE;
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_l3_modify_set
 * Description:
 *      Enable/Disable ingress port based L3 unicast packet operations.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - bitmap of the packet operations to be enabled or disabled.
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_esw_port_l3_modify_set(int unit, bcm_port_t port, uint32 flags)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(INCLUDE_L3)
    uint64 r;
    uint32 addr;
    soc_reg_t reg;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
#if defined (BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)) {
        if (IS_HG_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        reg = EGR_PORT_L3UC_MODS_TABLEr;
    } else 
#endif /* BCM_EASYRIDER_SUPPORT */
    if (SOC_IS_XGS3_SWITCH(unit)) {
        reg = IS_ST_PORT(unit, port) ?
            IEGR_PORT_L3UC_MODSr : EGR_PORT_L3UC_MODSr;
    } else {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_REG_IS_VALID(unit, reg)) {
        return BCM_E_UNAVAIL;
    }

    addr = soc_reg_addr(unit, reg, port, 0);
    BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &r));

    soc_reg64_field32_set(unit, reg, &r, L3_UC_SA_DISABLEf, 
                          (flags & BCM_PORT_L3_MODIFY_NO_SRCMAC) ? 1 : 0);

    soc_reg64_field32_set(unit, reg, &r, L3_UC_DA_DISABLEf, 
                          (flags & BCM_PORT_L3_MODIFY_NO_DSTMAC) ? 1 : 0);

    soc_reg64_field32_set(unit, reg, &r, L3_UC_TTL_DISABLEf, 
                          (flags & BCM_PORT_L3_MODIFY_NO_TTL) ? 1 : 0);

    soc_reg64_field32_set(unit, reg, &r, L3_UC_VLAN_DISABLEf, 
                          (flags & BCM_PORT_L3_MODIFY_NO_VLAN) ? 1 : 0);

    return soc_reg_set(unit, reg, port, 0, r);
#endif /* BCM_XGS3_SWITCH_SUPPORT && INCLUDE_L3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_l3_modify_get
 * Description:
 *      Get ingress port based L3 unicast packet operations status.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - (OUT) pointer to uint32 where bitmap of the current L3 packet 
 *              operations is returned.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_esw_port_l3_modify_get(int unit, bcm_port_t port, uint32 *flags)
{
#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(INCLUDE_L3)
    uint64 r;
    uint32 addr;
    soc_reg_t reg;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
#if defined(BCM_EASYRIDER_SUPPORT)
    if (SOC_IS_EASYRIDER(unit)) {
        if (IS_HG_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        reg = EGR_PORT_L3UC_MODS_TABLEr;
    } else 
#endif /* BCM_EASYRIDER_SUPPORT */
    if (SOC_IS_XGS3_SWITCH(unit)) {
        reg = IS_ST_PORT(unit, port) ?
            IEGR_PORT_L3UC_MODSr : EGR_PORT_L3UC_MODSr;
    } else {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_REG_IS_VALID(unit, reg)) {
        return BCM_E_UNAVAIL;
    }

    addr = soc_reg_addr(unit, reg, port, 0);
    BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &r));

    *flags = 0;

    if (soc_reg64_field32_get(unit, reg, r, L3_UC_SA_DISABLEf)) {
        *flags |= BCM_PORT_L3_MODIFY_NO_SRCMAC;
    }

    if (soc_reg64_field32_get(unit, reg, r, L3_UC_DA_DISABLEf)) {
        *flags |= BCM_PORT_L3_MODIFY_NO_DSTMAC;
    }

    if (soc_reg64_field32_get(unit, reg, r, L3_UC_TTL_DISABLEf)) {
        *flags |= BCM_PORT_L3_MODIFY_NO_TTL;
    }

    if (soc_reg64_field32_get(unit, reg, r, L3_UC_VLAN_DISABLEf)) {
        *flags |= BCM_PORT_L3_MODIFY_NO_VLAN;
    }

    return BCM_E_NONE;
#endif /* BCM_XGS3_SWITCH_SUPPORT && INCLUDE_L3 */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_ipmc_modify_set
 * Description:
 *      Enable/Disable ingress port based L3 multicast packet operations.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - bitmap of the packet operations to be enabled or disabled.
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_esw_port_ipmc_modify_set(int unit, bcm_port_t port, uint32 flags)
{
#if defined(INCLUDE_L3)
    int          idx;
    soc_field_t  fields[]={DISABLE_SA_REPLACEf, DISABLE_TTL_DECREMENTf};
    uint32       values[]={0, 0};

    if (!soc_feature(unit, soc_feature_ip_mcast)) { 
        return BCM_E_UNAVAIL; 
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#if defined(BCM_RAPTOR_SUPPORT)
    if (SOC_IS_RAPTOR(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_RAPTOR_SUPPORT */


    for (idx = 0; idx < COUNTOF(fields); idx++) {
        if (!SOC_REG_FIELD_VALID(unit, EGR_IPMC_CFG2r, fields[idx])) {
            return (BCM_E_UNAVAIL);
        }
    }

    values[0] = (flags & BCM_PORT_IPMC_MODIFY_NO_SRCMAC) ? 1 : 0;
    values[1] = (flags & BCM_PORT_IPMC_MODIFY_NO_TTL) ? 1 : 0;

    return soc_reg_fields32_modify(unit, EGR_IPMC_CFG2r, port, 
                                   COUNTOF(fields), fields, values);
#else /* INCLUDE_L3 */
    return (BCM_E_UNAVAIL);
#endif /* INCLUDE_L3 */
}

/*
 * Function:
 *      bcm_esw_port_ipmc_modify_get
 * Description:
 *      Enable/Disable ingress port based L3 multicast packet operations.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - (OUT) pointer to uint32 where bitmap of the current L3 packet
 *              operations is returned.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 * Notes:
 *      Available on XGS3 only.
 */
int 
bcm_esw_port_ipmc_modify_get(int unit, bcm_port_t port, uint32 *flags)
{
#if defined(INCLUDE_L3)
    uint32 reg_val;

    if (!soc_feature(unit, soc_feature_ip_mcast)) { 
        return BCM_E_UNAVAIL; 
    }

#if defined(BCM_RAPTOR_SUPPORT)
    if (SOC_IS_RAPTOR(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_RAPTOR_SUPPORT */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    if (NULL == flags) {
        return (BCM_E_PARAM);
    }

    if (!SOC_REG_FIELD_VALID(unit, EGR_IPMC_CFG2r, DISABLE_SA_REPLACEf)) {
        return (BCM_E_UNAVAIL);
    }

    if (!SOC_REG_FIELD_VALID(unit, EGR_IPMC_CFG2r, DISABLE_TTL_DECREMENTf)) {
        return (BCM_E_UNAVAIL);
    }

    BCM_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &reg_val));

    *flags = 0;
    if (soc_reg_field_get(unit, EGR_IPMC_CFG2r, reg_val, DISABLE_SA_REPLACEf)) {
        *flags |=  BCM_PORT_IPMC_MODIFY_NO_SRCMAC;
    }
    if (soc_reg_field_get(unit, EGR_IPMC_CFG2r, reg_val, DISABLE_TTL_DECREMENTf)) {
        *flags |=  BCM_PORT_IPMC_MODIFY_NO_TTL;
    }

    return (BCM_E_NONE);
#else /* INCLUDE_L3 */
    return (BCM_E_UNAVAIL);
#endif /* INCLUDE_L3 */
}

/*
 * Function:
 *      bcm_port_force_forward_mode_set
 * Purpose:
 *      Set egress override port mode.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port number
 *      egr_port - (IN) Egress port number
 *      flags - (IN) Force forward flags from PORT_FORCE_FORWARD_*
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_force_forward_mode_set(int unit, bcm_port_t port, 
                                    bcm_port_t egr_port, uint32 flags)
{
#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) \
    || defined(BCM_TRX_SUPPORT) || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_force_forward)) {

        int enable = FALSE;

        if ((0 != (flags & BCM_PORT_FORCE_FORWARD_LOCAL) &&
             !SOC_IS_TRX(unit))) {
            /* We don't support local only mode on pre-TRX devices */
            return BCM_E_PARAM;
        }

        if (0 != (flags & (BCM_PORT_FORCE_FORWARD_ALL |
                           BCM_PORT_FORCE_FORWARD_LOCAL))) {
            enable = TRUE;
        }

        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
        if (enable) {
            BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, egr_port, 
                                                             &egr_port));
        }

#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_IS_TR_VL(unit)) {
            pbmp_t pbmp;
            uint64 val;

            COMPILER_64_SET(val, 0, 0);
            SOC_PBMP_CLEAR(pbmp);
            if (enable) {
                SOC_PBMP_PORT_SET(pbmp, egr_port);
            }
            if (soc_mem_is_valid(unit, LOCAL_SW_DISABLE_DEFAULT_PBMm)) {
                local_sw_disable_default_pbm_entry_t entry;
                sal_memset(&entry, 0, sizeof(entry));
                soc_mem_pbmp_field_set(unit, LOCAL_SW_DISABLE_DEFAULT_PBMm,
                                       &entry, PORT_BITMAPf, &pbmp);
                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, LOCAL_SW_DISABLE_DEFAULT_PBMm,
                                   MEM_BLOCK_ANY, port, &entry));
                if (IS_CPU_PORT(unit, port)) {
                    BCM_IF_ERROR_RETURN
                        (soc_mem_write(unit, LOCAL_SW_DISABLE_DEFAULT_PBMm,
                                       MEM_BLOCK_ANY,
                                       SOC_INFO(unit).cpu_hg_index, &entry));
                }
            } else {
                soc_reg64_field32_set(unit
                                      , LOCAL_SW_DISABLE_DEFAULT_PBM_64r, 
                                      &val, PORT_BITMAP_LOf, 
                                      SOC_PBMP_WORD_GET(pbmp, 0));
                if(!SOC_IS_ENDURO(unit) && !SOC_IS_HURRICANE(unit)){
                    soc_reg64_field32_set(unit,
                                          LOCAL_SW_DISABLE_DEFAULT_PBM_64r, 
                                          &val, PORT_BITMAP_HIf, 
                                          SOC_PBMP_WORD_GET(pbmp, 1));
                }
                
                if (IS_ST_PORT(unit, port)) {
                    BCM_IF_ERROR_RETURN
                        (WRITE_ILOCAL_SW_DISABLE_DEFAULT_PBM_64r(unit, port,
                                                                 val));
                } else {
                    BCM_IF_ERROR_RETURN
                        (WRITE_LOCAL_SW_DISABLE_DEFAULT_PBM_64r(unit, port,
                                                                val));
                }
            }
        }  else 
#endif /* BCM_TRIUMPH_SUPPORT */  
        {      
#if defined(BCM_FIREBOLT2_SUPPORT) ||  defined(BCM_RAVEN_SUPPORT)  || \
            defined(BCM_SCORPION_SUPPORT)
            if ((SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit) ||
                 SOC_IS_SC_CQ(unit)) && IS_ST_PORT(unit, port)) {
                BCM_IF_ERROR_RETURN
                    (WRITE_ILOCAL_SW_DISABLE_DEFAULT_PBMr(unit, port,
                                         (enable) ? 1 << egr_port : 0));
            } else 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT || BCM_SCORPION_SUPPORT */
            {
                BCM_IF_ERROR_RETURN
                    (WRITE_LOCAL_SW_DISABLE_DEFAULT_PBMr(unit, port,
                                    (enable) ? 1 << egr_port : 0));
            }
        }

#if defined(BCM_TRX_SUPPORT)
        if (SOC_IS_TRX(unit)) {
            uint32 mode;

            mode = (flags & BCM_PORT_FORCE_FORWARD_LOCAL) ?
                _BCM_TRX_PORT_FORCE_FORWARD_LOCAL :
                ((flags & BCM_PORT_FORCE_FORWARD_ALL) ?
                 _BCM_TRX_PORT_FORCE_FORWARD_ALL :
                 _BCM_TRX_PORT_FORCE_FORWARD_DISABLE);

            return soc_reg_field32_modify(unit, LOCAL_SW_DISABLE_CTRLr,
                                          port, SW_MODEf, mode);
        } else 
#endif
        {
            return _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                         LOCAL_SW_DISABLEf, 
                                         (enable) ? 1 : 0);
        }
    }
#endif /* BCM_BRADLEY_SUPPORT || BCM_FIREBOLT2_SUPPORT \
          || BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_force_forward_mode_get
 * Purpose:
 *      Get egress override port.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port number
 *      egr_port - (OUT) Egress port number
 *      flags - (OUT) Force forward flags from PORT_FORCE_FORWARD_*
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_force_forward_mode_get(int unit, bcm_port_t port, 
                                    bcm_port_t *egr_port, uint32 *flags)
{
#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) \
        || defined(BCM_TRX_SUPPORT) || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_force_forward)) {
        pbmp_t pbmp;
        uint32 egr_pbm;
        int ep, enabled;

        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#if defined(BCM_TRX_SUPPORT)
        if (SOC_IS_TRX(unit)) {
            uint32 val_32, mode;

            BCM_IF_ERROR_RETURN
                (READ_LOCAL_SW_DISABLE_CTRLr(unit, port, &val_32));
            mode = soc_reg_field_get(unit, LOCAL_SW_DISABLE_CTRLr, 
                                         val_32, SW_MODEf);

            if (_BCM_TRX_PORT_FORCE_FORWARD_LOCAL == mode) {
                *flags = BCM_PORT_FORCE_FORWARD_LOCAL;
            } else if (_BCM_TRX_PORT_FORCE_FORWARD_ALL == mode) {
                *flags = BCM_PORT_FORCE_FORWARD_ALL;
            } else {
                *flags = BCM_PORT_FORCE_FORWARD_DISABLE;
            }
            enabled = (BCM_PORT_FORCE_FORWARD_DISABLE != mode);
        } else
#endif
        {
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_tab_get(unit, port, LOCAL_SW_DISABLEf, &enabled));
            *flags = enabled ? BCM_PORT_FORCE_FORWARD_ALL :
                BCM_PORT_FORCE_FORWARD_DISABLE;
        }

        if (enabled) {
            _bcm_gport_dest_t   dest;
            bcm_module_t        modid;
            int                 isGport = 0;

            _bcm_gport_dest_t_init(&dest);
#if defined(BCM_TRIUMPH_SUPPORT)
            if (SOC_IS_TR_VL(unit)) {
                uint64 val;

                if (soc_mem_is_valid(unit, LOCAL_SW_DISABLE_DEFAULT_PBMm)) {
                    local_sw_disable_default_pbm_entry_t entry;
                    BCM_IF_ERROR_RETURN
                        (soc_mem_read(unit, LOCAL_SW_DISABLE_DEFAULT_PBMm,
                                      MEM_BLOCK_ANY, port, &entry));
                    soc_mem_pbmp_field_get(unit, LOCAL_SW_DISABLE_DEFAULT_PBMm,
                                           &entry, PORT_BITMAPf, &pbmp);
                } else {
                    BCM_IF_ERROR_RETURN
                        (READ_LOCAL_SW_DISABLE_DEFAULT_PBM_64r(unit, port,
                                                               &val));
                    egr_pbm = 
                        soc_reg64_field32_get(unit,
                                              LOCAL_SW_DISABLE_DEFAULT_PBM_64r,
                                              val, PORT_BITMAP_LOf);
                    SOC_PBMP_CLEAR(pbmp);
                    SOC_PBMP_WORD_SET(pbmp, 0, egr_pbm);
                    if ((egr_pbm == 0) &&
                        (!SOC_IS_ENDURO(unit)) && (!SOC_IS_HURRICANE(unit))) {
                        egr_pbm = soc_reg64_field32_get
                            (unit,  LOCAL_SW_DISABLE_DEFAULT_PBM_64r,
                             val, PORT_BITMAP_HIf);
                        SOC_PBMP_WORD_SET(pbmp, 1, egr_pbm);
                    }
                }
            } else 
#endif /* BCM_TRIUMPH_SUPPORT */
            if ((SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit) ||
                 SOC_IS_SC_CQ(unit)) && IS_ST_PORT(unit, port)) {
                BCM_IF_ERROR_RETURN
                    (READ_ILOCAL_SW_DISABLE_DEFAULT_PBMr(unit, port, &egr_pbm));
                SOC_PBMP_CLEAR(pbmp);
                SOC_PBMP_WORD_SET(pbmp, 0, egr_pbm);
            } else {
                BCM_IF_ERROR_RETURN
                    (READ_LOCAL_SW_DISABLE_DEFAULT_PBMr(unit, port, &egr_pbm));
                SOC_PBMP_CLEAR(pbmp);
                SOC_PBMP_WORD_SET(pbmp, 0, egr_pbm);
            }
            if (SOC_PBMP_IS_NULL(pbmp)) {
                return BCM_E_INTERNAL;
            }
            /* Convert port bitmap into port number */
            SOC_PBMP_ITER(pbmp, ep) {
                break;
            }
            BCM_IF_ERROR_RETURN
                (bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
            if (isGport) {
                /* ignore return code to suport devices w/out modid assigned */
                (void)bcm_esw_stk_my_modid_get(unit, &modid);

                if (BCM_MODID_INVALID == modid) {
                    dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
                } else {
                    dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
                    dest.modid = modid;
                }
                dest.port = ep;

                BCM_IF_ERROR_RETURN
                    (_bcm_esw_gport_construct(unit, &dest, &ep));
            }
            *egr_port = ep;
        }
        return BCM_E_NONE;
    }
#endif /* BCM_BRADLEY_SUPPORT || BCM_FIREBOLT2_SUPPORT \
          || BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_port_force_forward_set
 * Purpose:
 *      This function allows packets to bypass the normal forwarding
 *      logic and be sent out on a specific port instead.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      egr_port - Egress port number
 *      enable - Bypass switching logic and forward to egr_port
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 * Notes:
 *      All normal ingress processing (table lookups etc.) is still
 *      performed and used for contructing the Higig header if the
 *      forced egress port is a Higig port.
 */
int 
bcm_esw_port_force_forward_set(int unit, bcm_port_t port, 
                               bcm_port_t egr_port, int enable)
{
    return bcm_esw_port_force_forward_mode_set(unit, port, egr_port,
                         enable ? BCM_PORT_FORCE_FORWARD_ALL :
                                  BCM_PORT_FORCE_FORWARD_DISABLE);
}

/*
 * Function:
 *      bcm_esw_port_force_forward_get
 * Purpose:
 *      Determine forced forwarding setting for a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      egr_port - (OUT) Egress port number
 *      enabled - (OUT) Forced forwarding enabled
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 * Notes:
 *      If the value of enabled is zero, the value of egr_port should 
 *      be ignored.
 */
int 
bcm_esw_port_force_forward_get(int unit, bcm_port_t port, 
                               bcm_port_t *egr_port, int *enabled)
{
    uint32 flags;

    BCM_IF_ERROR_RETURN
        (bcm_esw_port_force_forward_mode_get(unit, port, egr_port, &flags));

    *enabled = (0 != (flags & (BCM_PORT_FORCE_FORWARD_ALL |
                              BCM_PORT_FORCE_FORWARD_LOCAL)));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_port_err_sym_detect_set
 * Description:
 *      Enable/Disable XAUI error symbol monitoring feature.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      enable - TRUE, enable |E| monitoring feature on the port.
 *               FALSE, disable |E| monitoring feature on the port.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 */
#ifdef BCM_XGS3_SWITCH_SUPPORT
STATIC int
_bcm_esw_port_err_sym_detect_set(int unit, bcm_port_t port, int enable) 
{
    int rv;

    SOC_DEBUG_PRINT((DK_PORT,
                     "_bcm_esw_port_err_sym_detect_set: u=%d p=%d enable=%d\n",
                     unit, port, enable));

    if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port) 
#ifdef BCM_GXPORT_SUPPORT
        || IS_GX_PORT(unit, port)
#endif /* BCM_GXPORT_SUPPORT */
                                 ) {
        PORT_LOCK(unit);
        rv = soc_xaui_err_sym_detect_set(unit, port, enable);
        PORT_UNLOCK(unit);
    } else {
        rv = BCM_E_UNAVAIL;
    }


    return rv;
}
#endif

/*
 * Function:
 *      _bcm_esw_port_err_sym_detect_get
 * Description:
 *      Get the status of XAUI error symbol monitoring feature.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      status - (OUT) TRUE, port is enabled, FALSE port is disabled.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 */
#ifdef BCM_XGS3_SWITCH_SUPPORT
STATIC int
_bcm_esw_port_err_sym_detect_get(int unit, bcm_port_t port, int *status) 
{
    int rv;

    if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port) 
#ifdef BCM_GXPORT_SUPPORT
        || IS_GX_PORT(unit, port)
#endif /* BCM_GXPORT_SUPPORT */
                                 ) {
        PORT_LOCK(unit);
        rv = soc_xaui_err_sym_detect_get(unit, port, status);
        PORT_UNLOCK(unit);
    } else {
        rv = BCM_E_UNAVAIL;
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "_bcm_esw_port_err_sym_detect_get: u=%d p=%d status=%d\n",
                     unit, port, *status));

    return rv;
}
#endif

/*
 * Function:
 *      _bcm_esw_port_err_sym_count_get
 * Description:
 *      Get the number of |E| symbol in XAUI lanes since last read.
 *      The |E| symbol count is cleared on read.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      count - (OUT) Number of |E| error since last read.
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 *      BCM_E_INIT    - Error symbol detect feature is not enabled
 */
STATIC int 
_bcm_esw_port_err_sym_count_get(int unit, bcm_port_t port, int *count) 
{
    int rv;
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
 
    rv = BCM_E_UNAVAIL;

#ifdef BCM_XGS3_SWITCH_SUPPORT

    if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port) 
#ifdef BCM_GXPORT_SUPPORT
        || IS_GX_PORT(unit, port)
#endif /* BCM_GXPORT_SUPPORT */
                                 ) {
        PORT_LOCK(unit);
        rv = soc_xaui_err_sym_count(unit, port, count); 
        PORT_UNLOCK(unit);
    }
#endif

    SOC_DEBUG_PRINT((DK_PORT,
                     "_bcm_esw_port_err_sym_count_get: u=%d p=%d count=%d\n",
                     unit, port, *count));

    return rv;
}

STATIC int 
_bcm_esw_port_control_bridge_set(int unit, bcm_port_t port, int value)
{
    int rv = BCM_E_UNAVAIL;

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit)) {
        bcm_pbmp_t  pbmp;            /* Port bitmap to update.*/
        if (IS_E_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
            if (SOC_MEM_IS_VALID(unit, PORT_BRIDGE_BMAPm)) {
                port_bridge_bmap_entry_t entry;

                SOC_IF_ERROR_RETURN(soc_mem_read(unit, PORT_BRIDGE_BMAPm,
                                                 MEM_BLOCK_ANY, 0, &entry));
                soc_mem_pbmp_field_get(unit, PORT_BRIDGE_BMAPm, &entry,
                                       BITMAPf, &pbmp);
                if (value && !SOC_PBMP_MEMBER(pbmp, port)) {
                    SOC_PBMP_PORT_ADD(pbmp, port);
                } else if (!value && SOC_PBMP_MEMBER(pbmp, port)) {
                    SOC_PBMP_PORT_REMOVE(pbmp, port);
                } else {
                    return BCM_E_NONE;
                }
                soc_mem_pbmp_field_set(unit, PORT_BRIDGE_BMAPm, &entry,
                                       BITMAPf, &pbmp);
                SOC_IF_ERROR_RETURN(soc_mem_write(unit, PORT_BRIDGE_BMAPm,
                                                  MEM_BLOCK_ANY, 0, &entry));
            }
            if (SOC_REG_IS_VALID(unit, PORT_BRIDGE_BMAP_64r)) {
                uint64      buf64;           /* Buffer for reg64 value. */
                SOC_PBMP_CLEAR(pbmp);

                BCM_IF_ERROR_RETURN
                    (READ_PORT_BRIDGE_BMAP_64r(unit, &buf64));

                /* Set l3 port bitmap. */
                SOC_PBMP_WORD_SET(pbmp, 0,  
                    soc_reg64_field32_get(unit, PORT_BRIDGE_BMAP_64r, 
                                          buf64, BITMAP_LOf));
                if(!SOC_IS_ENDURO(unit) && !SOC_IS_HURRICANE(unit)) {
                    SOC_PBMP_WORD_SET(pbmp, 1,  
                        soc_reg64_field32_get(unit, PORT_BRIDGE_BMAP_64r, 
                                              buf64, BITMAP_HIf));
                }

                if (SOC_PBMP_MEMBER(pbmp, port) && value) {
                    return BCM_E_NONE;
                }
                if (!SOC_PBMP_MEMBER(pbmp, port) && !value) {
                    return BCM_E_NONE;
                }
                if (!SOC_PBMP_MEMBER(pbmp, port) && value) {
                    SOC_PBMP_PORT_ADD(pbmp, port);
                } else if (SOC_PBMP_MEMBER(pbmp, port) && !value) {
                    SOC_PBMP_PORT_REMOVE(pbmp, port);
                }

                soc_reg64_field32_set(unit, PORT_BRIDGE_BMAP_64r,
                                      &buf64, BITMAP_LOf,
                                      SOC_PBMP_WORD_GET(pbmp, 0));
                if(!SOC_IS_ENDURO(unit) && !SOC_IS_HURRICANE(unit)) {
                    soc_reg64_field32_set(unit, PORT_BRIDGE_BMAP_64r,
                                          &buf64, BITMAP_HIf,
                                          SOC_PBMP_WORD_GET(pbmp, 1));
                }

                BCM_IF_ERROR_RETURN
                    (WRITE_PORT_BRIDGE_BMAP_64r(unit, buf64));
            } 
        }

        /* Allow to set bridging on higig ports as well */
            return _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       PORT_BRIDGEf, (value) ? 1 : 0);
    } else
#endif /* BCM_TRIUMPH_SUPPORT */
        if (SOC_IS_XGS3_SWITCH(unit)) {
            /* Allow to set bridging on higig ports as well */
            rv =_bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       PORT_BRIDGEf, (value) ? 1 : 0);
            BCM_IF_ERROR_RETURN(rv);
        
            if (IS_E_PORT(unit, port) || IS_CPU_PORT(unit, port)) {
            
#if defined(BCM_RAPTOR_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_SCORPION_SUPPORT)
            if (soc_feature(unit, soc_feature_src_trunk_port_bridge)) {
                bcm_pbmp_t  pbmp;            /* Port bitmap to update.*/
                uint32      port_bridge_map;
#if defined (BCM_RAPTOR_SUPPORT)
                uint32 port_bridge_map_hi;
#endif

                SOC_PBMP_CLEAR(pbmp);
                BCM_IF_ERROR_RETURN
                    (READ_PORT_BRIDGE_BMAPr(unit, &port_bridge_map));

                SOC_PBMP_WORD_SET(pbmp, 0,  
                    soc_reg_field_get(unit, PORT_BRIDGE_BMAPr, 
                                      port_bridge_map, BITMAPf));
#if defined (BCM_RAPTOR_SUPPORT)
                if (soc_feature(unit, soc_feature_register_hi)) {
                    BCM_IF_ERROR_RETURN
                        (READ_PORT_BRIDGE_BMAP_HIr(unit,
                                                   &port_bridge_map_hi));
                    SOC_PBMP_WORD_SET(pbmp, 1,  
                        soc_reg_field_get(unit, PORT_BRIDGE_BMAP_HIr, 
                                          port_bridge_map_hi, BITMAPf));
                }
#endif
                if (!SOC_PBMP_MEMBER(pbmp, port) && value) {
                    SOC_PBMP_PORT_ADD(pbmp, port);
                    soc_reg_field_set(unit, PORT_BRIDGE_BMAPr,
                                      &port_bridge_map, BITMAPf,
                                      SOC_PBMP_WORD_GET(pbmp, 0));
                    BCM_IF_ERROR_RETURN
                        (WRITE_PORT_BRIDGE_BMAPr(unit, port_bridge_map));
#if defined (BCM_RAPTOR_SUPPORT)

                    if (soc_feature(unit, soc_feature_register_hi)) {
                        soc_reg_field_set(unit, PORT_BRIDGE_BMAP_HIr,
                                          &port_bridge_map_hi, BITMAPf,
                                          SOC_PBMP_WORD_GET(pbmp, 1));
                        BCM_IF_ERROR_RETURN
                            (WRITE_PORT_BRIDGE_BMAP_HIr(unit,
                                                        port_bridge_map_hi));
                    }
#endif
                } else if (SOC_PBMP_MEMBER(pbmp, port) && !value) {
                    SOC_PBMP_PORT_REMOVE(pbmp, port);
                    soc_reg_field_set(unit, PORT_BRIDGE_BMAPr,
                                      &port_bridge_map, BITMAPf,
                                      SOC_PBMP_WORD_GET(pbmp, 0));
                    BCM_IF_ERROR_RETURN
                        (WRITE_PORT_BRIDGE_BMAPr(unit, port_bridge_map));
#if defined (BCM_RAPTOR_SUPPORT)
                    if (soc_feature(unit, soc_feature_register_hi)) {
                        soc_reg_field_set(unit, PORT_BRIDGE_BMAP_HIr,
                                          &port_bridge_map_hi, BITMAPf,
                                          SOC_PBMP_WORD_GET(pbmp, 1));
                        BCM_IF_ERROR_RETURN
                            (WRITE_PORT_BRIDGE_BMAP_HIr(unit,
                                                        port_bridge_map_hi));
                    }
#endif
                }
            }
#endif /* BCM_RAPTOR_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        }
        }
#ifdef BCM_EASYRIDER_SUPPORT
        else if (SOC_IS_EASYRIDER(unit) && IS_HG_PORT(unit, port)) {
            bcm_port_cfg_t      pcfg;
            BCM_IF_ERROR_RETURN
                (mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg));
            pcfg.pc_bridge_port = (value) ? 1 : 0;
            rv = mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg);
        }
#endif
    return rv;
}

#if defined(BCM_SHADOW_SUPPORT)
STATIC int
_bcm_port_type_shadow_support (int unit, bcm_port_t port,
                                    bcm_port_control_t type)
{
    SOC_DEBUG_PRINT((DK_PORT,
                     "_bcm_port_type_shadow_support : u=%d p=%d type=%d\n",
                     unit, port, (int)type));

    switch (type)
    {
        /* Shadoow supported port control types */
        case bcmPortControlRxEnable:
        case bcmPortControlTxEnable:
        case bcmPortControlPFCEthertype:
        case bcmPortControlPFCOpcode:
        case bcmPortControlPFCReceive:
        case bcmPortControlPFCTransmit:
        case bcmPortControlPFCClasses:
        case bcmPortControlPFCPassFrames:
        case bcmPortControlPFCDestMacOui:
        case bcmPortControlPFCDestMacNonOui:
        case bcmPortControlPFCRefreshTime:
        case bcmPortControlEEETransmitWakeTime:
        case bcmPortControlEEEReceiveWakeTime:
        case bcmPortControlEEETransmitSleepTime:
        case bcmPortControlEEEReceiveSleepTime:
        case bcmPortControlEEETransmitQuietTime:
        case bcmPortControlEEEReceiveQuietTime:
        case bcmPortControlEEETransmitRefreshTime:
        case bcmPortControlEEEEnable:
        case bcmPortControlEEETransmitIdleTime:
        case bcmPortControlEEETransmitEventCount:
        case bcmPortControlEEETransmitDuration:
        case bcmPortControlEEEReceiveEventCount:
        case bcmPortControlEEEReceiveDuration:
        case bcmPortControlEEETransmitIdleTimeHund:
        case bcmPortControlEEETransmitWakeTimeHund:
        case bcmPortControlEEETransmitMinLPITime:
        case bcmPortControlEEETransmitMinLPITimeHund:
            return TRUE;
        
        default:
            return FALSE;
    }
}
#endif /* BCM_SHADOW_SUPPORT */


static int
_bcm_esw_mac_rx_control(int unit, bcm_port_t port, uint8 optype, int *enable)
{
    int rv = BCM_E_NONE;
    
    PORT_LOCK(unit);
    if (optype) { /* get */
        rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                              SOC_MAC_CONTROL_RX_SET, enable);
    } else { /* set */
        rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                         SOC_MAC_CONTROL_RX_SET, *enable);
    }
    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_control_set
 * Description:
 *      Enable/Disable specified port feature.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      type - Enum value of the feature
 *      value - value to be set
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 */
int 
bcm_esw_port_control_set(int unit, bcm_port_t port, 
                         bcm_port_control_t type, int value)
{
    int rv = BCM_E_UNAVAIL;
    soc_reg_t egr_port_reg;
    egr_port_reg = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit)) ? EGR_PORT_64r : EGR_PORTr;

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_port_control_set: u=%d p=%d rv=%d : %d\n",
                     unit, port, value, (int)type));

#if defined(BCM_SHADOW_SUPPORT)
    if (SOC_IS_SHADOW(unit)) {
        /*  If IL port and unsupported port type, return unavail */
        if (IS_IL_PORT(unit, port) || 
            !_bcm_port_type_shadow_support (unit, port, type)) {
            return BCM_E_UNAVAIL;
        }
    }
#endif /* BCM_SHADOW_SUPPORT */


#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return bcm_td_vp_control_set(unit, port, type, value);
    } else
#endif
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        rv = BCM_E_NONE;
    } else {
        rv = _bcm_esw_port_gport_validate(unit, port, &port);
    }
    BCM_IF_ERROR_RETURN(rv);
    rv = BCM_E_UNAVAIL;

    switch (type) {
    case bcmPortControlBridge:
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            rv = _bcm_tr2_wlan_port_set(unit, port, PORT_BRIDGEf, value);
        } else
#endif 
        {
            PORT_LOCK(unit);
            rv = _bcm_esw_port_control_bridge_set(unit, port, value);
            PORT_UNLOCK(unit);
        }
        break;
    case bcmPortControlTrunkHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       RTAG7_HASH_CFG_SEL_TRUNKf, 
                                       (value) ? 1 : 0);
        }
#endif /* BCM_BRADLEY_SUPPORT */
        break; 
    case bcmPortControlFabricTrunkHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       RTAG7_HASH_CFG_SEL_HIGIG_TRUNKf, 
                                       (value) ? 1 : 0);
        }
#endif /* BCM_BRADLEY_SUPPORT */
        break; 
    case bcmPortControlECMPHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       RTAG7_HASH_CFG_SEL_ECMPf, 
                                       (value) ? 1 : 0);
        }
#endif /* BCM_BRADLEY_SUPPORT */
        break; 
    case bcmPortControlLoadBalanceHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       RTAG7_HASH_CFG_SEL_LBIDf, 
                                       (value) ? 1 : 0);
        }
#endif /* BCM_BRADLEY_SUPPORT */
        break; 
    case bcmPortControlLoadBalancingNumber:
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       RTAG7_PORT_LBNf, 
                                       (value) & 0xf);
        }
#endif /* BCM_BRADLEY_SUPPORT */
        break; 
    case bcmPortControlErrorSymbolDetect:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_XGS3_SWITCH(unit)) {
             rv = _bcm_esw_port_err_sym_detect_set(unit, port, value);
        }
#endif
        break;
    case bcmPortControlErrorSymbolCount:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_XGS3_SWITCH(unit)) {
            int temp = 0;
            /* Error symbol count is read only and cleared on read. */
            rv = _bcm_esw_port_err_sym_count_get(unit, port, &temp);
        }
#endif
        break;
#if defined(INCLUDE_L3)
    case bcmPortControlIP4:
        if (SOC_IS_XGS3_SWITCH(unit) && 
            (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       V4L3_ENABLEf, (value) ? 1 : 0);
#if defined(BCM_TRIUMPH_SUPPORT)
            if (BCM_SUCCESS(rv) && 
                soc_feature(unit, soc_feature_esm_support)) {
                rv = _bcm_tr_l3_enable(unit, port, 0, (value) ? 1 : 0);
            }
#endif /* BCM_TRIUMPH_SUPPORT */
        }
        break;
    case bcmPortControlIP6:
        if (SOC_IS_XGS3_SWITCH(unit) && 
            (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       V6L3_ENABLEf, (value) ? 1 : 0);
#if defined(BCM_TRIUMPH_SUPPORT)
            if (BCM_SUCCESS(rv) && 
                soc_feature(unit, soc_feature_esm_support)) {
                rv = _bcm_tr_l3_enable(unit, port, BCM_L3_IP6, 
                                       (value) ? 1 : 0);
            }
#endif /* BCM_TRIUMPH_SUPPORT */
        }
        break;
    case bcmPortControlIP4Mcast:
        if (SOC_IS_XGS3_SWITCH(unit) && 
            (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       V4IPMC_ENABLEf, (value) ? 1 : 0);
        }
        break;
    case bcmPortControlIP6Mcast:
        if (SOC_IS_XGS3_SWITCH(unit) && 
            (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       V6IPMC_ENABLEf, (value) ? 1 : 0);
        }
        break;
    case bcmPortControlMpls:
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
        if ((BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, MPLS_ENABLEf)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       MPLS_ENABLEf, (value) ? 1 : 0);
        }
#endif
        break;
    case bcmPortControlMacInMac:
#if defined(BCM_TRIUMPH2_SUPPORT)
        if ((BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, MIM_TERM_ENABLEf)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       MIM_TERM_ENABLEf, (value) ? 1 : 0);
        }
#endif
        break;
    case bcmPortControlFabricQueue:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (IS_ST_PORT(unit, port) && SOC_REG_FIELD_VALID(unit, EGR_PORT_64r, 
            EH_EXT_HDR_ENABLEf)) {
            rv = soc_reg_field32_modify(unit, EGR_PORT_64r, port,
                    EH_EXT_HDR_ENABLEf, (value) ? 1 : 0);
        }
#endif
        break;
    case bcmPortControlMplsIngressPortCheck:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_EASYRIDER_SUPPORT
        if (!IS_ST_PORT(unit, port) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, MPLS_PORT_CHECKf)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       MPLS_PORT_CHECKf, (value) ? 1 : 0);
        }
#endif
        break;
    case bcmPortControlMplsMultiLabelSwitching:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_EASYRIDER_SUPPORT
        if (!IS_ST_PORT(unit, port) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, THREE_MPLS_LABELf)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       THREE_MPLS_LABELf, (value) ? 1 : 0);
        }
#endif
        break;
#endif /* INCLUDE_L3 */
    case bcmPortControlIP4McastL2:
#ifdef BCM_TRX_SUPPORT
        if ((BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, V4IPMC_L2_ENABLEf)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       V4IPMC_L2_ENABLEf, (value) ? 1 : 0);
        }
#endif
        break;
    case bcmPortControlIP6McastL2:
#ifdef BCM_TRX_SUPPORT
        if ((BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, V6IPMC_L2_ENABLEf)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       V6IPMC_L2_ENABLEf, (value) ? 1 : 0);
        }
#endif
        break;
    case bcmPortControlPassControlFrames:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_XGS3_SWITCH(unit) && IS_E_PORT(unit, port)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_NONE,
                                       PASS_CONTROL_FRAMESf, (value) ? 1 : 0);
        }
#endif
#ifdef BCM_XGS12_SWITCH_SUPPORT
        if ((SOC_IS_DRACO(unit) || SOC_IS_TUCANA(unit)) && 
            IS_E_PORT(unit, port)) {
            uint64 rsv_mask;
            /* Set/clear the Control Frame Purge bit */
            PORT_LOCK(unit);
            rv = READ_RSV_MASKr(unit, port, &rsv_mask);
            if (BCM_SUCCESS(rv)) {
                if (value) {
                    COMPILER_64_SET(rsv_mask, 0,
                                    COMPILER_64_LO(rsv_mask) & ~0x08000000);
                } else {
                    COMPILER_64_SET(rsv_mask, 0,
                                    COMPILER_64_LO(rsv_mask) | 0x08000000);
                }
                rv = WRITE_RSV_MASKr(unit, port, rsv_mask);
            }
            PORT_UNLOCK(unit);
        }
#endif
#ifdef BCM_XGS_SWITCH_SUPPORT
        if (IS_XE_PORT(unit, port)) {
            /* Enable Control Frames in BigMAC */
            PORT_LOCK(unit);
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_PASS_CONTROL_FRAME, value);
            PORT_UNLOCK(unit);
        }
#endif
        break;
#ifdef BCM_XGS3_SWITCH_SUPPORT
      case bcmPortControlFilterLookup:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
          value = value ? 1 : 0; 
          if (soc_mem_field_valid(unit, PORT_TABm, VFP_ENABLEf)) {
              rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                         VFP_ENABLEf, value);
          } 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
          break;
      case bcmPortControlFilterIngress:
          value = value ? 1 : 0;
              /* Enable/disable ingress filtering. */
          rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                     FILTER_ENABLEf, value);
          break;
      case bcmPortControlFilterEgress:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        } else
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        {
             value = value ? 1 : 0;
             /* Enable/disable egress filtering. */
             if (SOC_REG_FIELD_VALID(unit, egr_port_reg, EFP_FILTER_ENABLEf)) {
                 rv = soc_reg_field32_modify(unit, egr_port_reg, port,
                                             EFP_FILTER_ENABLEf, value);
             } else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm,
                                            EFP_FILTER_ENABLEf)) {
                 rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                                             EFP_FILTER_ENABLEf, value);
             }
         }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
          break;
      case bcmPortControlFrameSpacingStretch:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
          rv = (MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                  SOC_MAC_CONTROL_FRAME_SPACING_STRETCH,
                                  value));
          break;
      case bcmPortControlPreservePacketPriority:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
    || defined(BCM_RAVEN_SUPPORT)
          rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                     USE_INCOMING_DOT1Pf, (value) ? 1 : 0);
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT || defined(BCM_RAVEN_SUPPORT) */
#if defined(BCM_SHADOW_SUPPORT)
          if (SOC_IS_SHADOW(unit)) {
              rv = soc_reg_field32_modify(unit, PRIORITY_MAPPING_SELECTr, port,
                                              USE_INPUT_PRIORIYf, 1 );
          }
#endif
          break;
      case bcmPortControlLearnClassEnable:
#if defined(BCM_TRX_SUPPORT) 
          if (soc_feature(unit, soc_feature_class_based_learning)) {
              rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                         CLASS_BASED_SM_ENABLEf, value ? 1 : 0);
          }
#endif /* BCM_TRX_SUPPORT */
          break;
      case bcmPortControlTrustIncomingVlan:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_TRX_SUPPORT)
          rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                     TRUST_INCOMING_VIDf, (value) ? 1 : 0);
#endif /* BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
          break;
      case bcmPortControlDoNotCheckVlan:
#if defined(BCM_TRX_SUPPORT)
          if (!SOC_IS_SHADOW(unit)) {
              rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                     DISABLE_VLAN_CHECKSf, (value) ? 1 : 0);
          }
#endif /* BCM_TRX_SUPPORT */
          break;
       case bcmPortControlIEEE8021ASEnableIngress:
          if (soc_feature(unit, soc_feature_rx_timestamp)) {
              rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                     IEEE_802_1AS_ENABLEf, (value) ? 1 : 0);
          }
          break;
      case bcmPortControlIEEE8021ASEnableEgress:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_HAWKEYE_SUPPORT)
          if (SOC_IS_HAWKEYE(unit)) {
              rv = soc_reg_field32_modify(unit, EGR_PORTr, port,
                                         IEEE_802_1AS_ENABLEf, (value) ? 1 : 0);
          }
#endif /* BCM_HAWKEYE_SUPPORT */
          break;
      case bcmPortControlEgressVlanPriUsesPktPri:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRX_SUPPORT) 
          if (SOC_IS_TRX(unit) && !SOC_IS_SHADOW(unit)) {
              rv = soc_reg_field32_modify(unit, EGR_VLAN_CONTROL_1r, port,
                                         REMARK_OUTER_DOT1Pf,
                                         value ? 0 : 1);
          }
#endif /* BCM_TRX_SUPPORT */
          break;
      case bcmPortControlEgressModifyDscp:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRX_SUPPORT)
          if (SOC_IS_TRX(unit) && !SOC_IS_SHADOW(unit)) {
              rv = soc_reg_field32_modify(unit, EGR_VLAN_CONTROL_1r, port,
                                          REMARK_OUTER_DSCPf,
                                          value ? 1 : 0);
          }
#endif /* BCM_TRX_SUPPORT */
          break;
    case bcmPortControlIpfixRate:
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_ipfix_rate)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       IPFIX_FLOW_METER_IDf, value);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        break;
    case bcmPortControlCustomerQueuing:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_ENDURO_SUPPORT
        if (SOC_IS_ENDURO(unit) && !IS_HG_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
#endif
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_vlan_queue_map)) {
            uint32 fval, rv;
            if (value) {
                fval = 3;
            } else {
                fval = 0;
            }

            rv = BCM_E_UNAVAIL;

            if (SOC_REG_FIELD_VALID(unit, ING_COS_MODEr, SELECTf)) {
                rv = soc_reg_field32_modify(unit, ING_COS_MODEr, REG_PORT_ANY, 
                                            SELECTf, fval);
            }
            if (SOC_REG_FIELD_VALID(unit, COS_MODEr, SELECTf)) {
                rv = soc_reg_field32_modify(unit, COS_MODEr, REG_PORT_ANY, 
                                            SELECTf, fval);
            }
            if (SOC_REG_FIELD_VALID(unit, ING_COS_MODEr, QUEUE_MODEf)) {
                rv = soc_reg_field32_modify(unit, ING_COS_MODEr, REG_PORT_ANY, 
                                            QUEUE_MODEf, fval);
                /* 0 - COS1 port COS, 1 - COS2 vlan COS */
                rv = soc_reg_field32_modify(unit, ING_COS_MODEr, REG_PORT_ANY, 
                                            COS_MODEf, 1);

            }

        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        break;
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT)
      case bcmPortControlOAMEnable:
          if (BCM_GPORT_IS_WLAN_PORT(port)) {
              return BCM_E_UNAVAIL;
          }
          rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                     OAM_ENABLEf, value ? 1 : 0);
          break;
#endif /* defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT) */

#ifdef BCM_TRX_SUPPORT
      case bcmPortControlOamLoopback:
          if (BCM_GPORT_IS_WLAN_PORT(port)) {
              return BCM_E_UNAVAIL;
          }

          if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANE(unit) && !SOC_IS_KATANA(unit)
              && !SOC_IS_SHADOW(unit))
          {
              rv = soc_reg_field32_modify(unit, ING_MISC_PORT_CONFIGr, port,
                  OAM_DO_NOT_MODIFYf, value ? 1 : 0);
          }

          break;
#endif /* BCM_TRX_SUPPORT */

    case bcmPortControlLanes:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (soc_feature(unit, soc_feature_flex_port)) {
#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit)) {
                bcm_port_t phy_port;
                int bindex;

                if (!SOC_PORT_VALID(unit, port)) {
                    return BCM_E_PORT;
                }
                if (value != 1 && value != 2 && value != 4) {
                    return BCM_E_PARAM;
                }
                phy_port = SOC_INFO(unit).port_l2p_mapping[port];
                bindex = SOC_PORT_INFO(unit, phy_port).bindex;
                if (bindex & (value - 1)) { /* port alignment check */
                    return BCM_E_PARAM;
                }
            }
#endif /* BCM_TRIDENT_SUPPORT */
            PORT_LOCK(unit);
#if defined(BCM_ENDURO_SUPPORT)
            if (SOC_IS_ENDURO(unit)) {
                rv = _bcm_en_port_lanes_set(unit, port, value);
            } else 
#endif
#if defined(BCM_HURRICANE_SUPPORT)
            if (SOC_IS_HURRICANE(unit)) {
                rv = _bcm_hu_port_lanes_set(unit, port, value);
            } else 
#endif
#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit)) {
                soc_info_t *si;
                int okay, i;
                uint32 rval, phy_mode;
                bcm_port_t phy_port;

                rv = READ_XLPORT_MODE_REGr(unit, port, &rval);
                if (BCM_SUCCESS(rv)) {
                    phy_mode = soc_reg_field_get(unit, XLPORT_MODE_REGr, rval,
                                                 PHY_PORT_MODEf);
                    if (phy_mode == 0) { /* single mode */
                        if (value != 4) {
                            rv = BCM_E_PARAM;
                        }
                    } else if (phy_mode == 1) { /* dual mode */
                        if (value != 2 && value != 4) {
                            rv = BCM_E_PARAM;
                        }
                    }
                }
                if (BCM_SUCCESS(rv)) {
                    si = &SOC_INFO(unit);
                    phy_port = si->port_l2p_mapping[port];
                    for (i = 0; i < value; i++) {
                        if ((si->port_p2l_mapping[phy_port + i]) == -1) {
                            continue;
                        }
                        rv = soc_phyctrl_detach
                            (unit, si->port_p2l_mapping[phy_port + i]);
                        if (BCM_FAILURE(rv)) {
                            break;
                        }
                    }
                }
                if (BCM_SUCCESS(rv)) {
                    si->port_num_lanes[port] = value;
                    if (BCM_SUCCESS(rv)) {
                        rv = _bcm_port_phy_probe(unit, port, &okay);
                    }
                }
            } else
#endif /* BCM_TRIDENT_SUPPORT */
            {
                rv = _bcm_tr2_port_lanes_set(unit, port, value);
            }
            PORT_UNLOCK(unit);
        }
#endif
        break;

    case bcmPortControlPFCEthertype:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT)
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            if (value < 0 || value > 0xffff) {
                return BCM_E_PARAM;
            }
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_TYPE, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCOpcode:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT)
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            if (value < 0 || value > 0xffff) {
                return BCM_E_PARAM;
            }
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_OPCODE, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCReceive:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT)
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            int pfc_enable;

            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }

            BCM_IF_ERROR_RETURN
                (MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_RX_ENABLE, value));
            if (value == 0) {
                BCM_IF_ERROR_RETURN
                    (MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                     SOC_MAC_CONTROL_PFC_TX_ENABLE,
                                     &pfc_enable));
            } else {
                pfc_enable = TRUE;
            }

            if (SOC_IS_SC_CQ(unit)) {
                BCM_IF_ERROR_RETURN
                    (soc_reg_field32_modify(unit, MMU_LLFC_RX_CONFIGr,
                                            port, RX_ENABLEf, value ? 1 : 0));
            }

            if (value == 0) {
                /* Disabling RX, flush MMU XOFF state */
                BCM_IF_ERROR_RETURN
                    (MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                     SOC_MAC_CONTROL_PFC_FORCE_XON, 1));
                BCM_IF_ERROR_RETURN
                    (MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                     SOC_MAC_CONTROL_PFC_FORCE_XON, 0));
            }

            if (SOC_IS_TD_TT(unit) || SOC_IS_SHADOW(unit)) {
                uint32 rval;

                BCM_IF_ERROR_RETURN(READ_XLPORT_CONFIGr(unit, port, &rval));
                if (pfc_enable) {
                    soc_reg_field_set(unit, XLPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      1);
                    soc_reg_field_set(unit, XLPORT_CONFIGr, &rval, LLFC_ENf,
                                      0);
                } else {
                    soc_reg_field_set(unit, XLPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      0);
                }
                BCM_IF_ERROR_RETURN(WRITE_XLPORT_CONFIGr(unit, port, rval));
            } else if (SOC_IS_KATANA(unit)) {
                uint32 rval;
            
                BCM_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &rval));
                if (pfc_enable) {
                    soc_reg_field_set(unit, XPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      1);
                    soc_reg_field_set(unit, XPORT_CONFIGr, &rval, LLFC_ENf,
                                      0);
                } else {
                    soc_reg_field_set(unit, XPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      0);
                }
                BCM_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, rval));
            } else if (SOC_IS_SC_CQ(unit) || SOC_IS_TR_VL(unit)) {
                /* Feature enable */
                BCM_IF_ERROR_RETURN
                    (soc_reg_field32_modify(unit, PPFC_ENr, port,
                                            PPFC_FEATURE_ENf,
                                            pfc_enable ? 1 : 0));
                if (soc_reg_field_valid(unit, XPORT_CONFIGr, PPP_ENABLEf)) {
                    BCM_IF_ERROR_RETURN
                        (soc_reg_field32_modify(unit, XPORT_CONFIGr,
                                                port, PPP_ENABLEf,
                                                pfc_enable ? 1 : 0));
                }
            }

            /* Stats enable */
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_STATS_ENABLE,
                                 pfc_enable ? 1 : 0);

            if (SOC_REG_IS_VALID(unit, XPORT_TO_MMU_BKPr) && pfc_enable == 0) {
                BCM_IF_ERROR_RETURN(WRITE_XPORT_TO_MMU_BKPr(unit, port, 0));
            }
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCTransmit:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            int pfc_enable;
            uint32 rval;

            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }

            BCM_IF_ERROR_RETURN
                (MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_TX_ENABLE, value));
            if (value == 0) {
                BCM_IF_ERROR_RETURN
                    (MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                     SOC_MAC_CONTROL_PFC_RX_ENABLE,
                                     &pfc_enable));
            } else {
                pfc_enable = TRUE;
            }

            if (!soc_feature(unit, soc_feature_mmu_config_property)) {
                rval = 0;
                if (value) {
                    soc_reg_field_set(unit, PORT_PRI_XON_ENABLEr, &rval,
                                      PORT_PRI_XON_ENABLEf, 0xff);
                    if (soc_reg_field_valid(unit, PORT_PRI_XON_ENABLEr,
                                            PORT_PG7PAUSE_DISABLEf)) {
                        soc_reg_field_set(unit, PORT_PRI_XON_ENABLEr, &rval,
                                          PORT_PG7PAUSE_DISABLEf, 1);
                    }
                }
                BCM_IF_ERROR_RETURN
                    (WRITE_PORT_PRI_XON_ENABLEr(unit, port, rval));
            }

            /* Feature enable */
            if (SOC_IS_TD_TT(unit) || SOC_IS_SHADOW(unit)) {
                BCM_IF_ERROR_RETURN(READ_XLPORT_CONFIGr(unit, port, &rval));
                if (pfc_enable) {
                    soc_reg_field_set(unit, XLPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      1);
                    soc_reg_field_set(unit, XLPORT_CONFIGr, &rval, LLFC_ENf,
                                      0);
                } else {
                    soc_reg_field_set(unit, XLPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      0);
                }
                BCM_IF_ERROR_RETURN(WRITE_XLPORT_CONFIGr(unit, port, rval));
            } else if (SOC_IS_KATANA(unit)) {
                BCM_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &rval));
                if (pfc_enable) {
                    soc_reg_field_set(unit, XPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      1);
                    soc_reg_field_set(unit, XPORT_CONFIGr, &rval, LLFC_ENf,
                                      0);
                } else {
                    soc_reg_field_set(unit, XPORT_CONFIGr, &rval, PFC_ENABLEf,
                                      0);
                }
                BCM_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, rval));
            } else if (SOC_IS_SC_CQ(unit) || SOC_IS_TR_VL(unit)) {
                BCM_IF_ERROR_RETURN
                      (soc_reg_field32_modify(unit, PPFC_ENr, port,
                                              PPFC_FEATURE_ENf,
                                              pfc_enable ? 1 : 0));
                if (soc_reg_field_valid(unit, XPORT_CONFIGr, PPP_ENABLEf)) {
                    BCM_IF_ERROR_RETURN
                        (soc_reg_field32_modify(unit, XPORT_CONFIGr, port,
                                                PPP_ENABLEf,
                                                pfc_enable ? 1 : 0));
                }
            }

            /* Stats enable */
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_STATS_ENABLE,
                                 pfc_enable ? 1 : 0);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCClasses:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_CLASSES, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCPassFrames:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_RX_PASS, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

      case bcmPortControlPFCDestMacOui:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            if (value < 0 || value > 0xffffff) {
                return BCM_E_PARAM;
            }
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_MAC_DA_OUI, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCDestMacNonOui:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            if (value < 0 || value > 0xffffff) {
                return BCM_E_PARAM;
            }
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_MAC_DA_NONOUI, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCRefreshTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (value < 0 || value > 0xffff) {
                return BCM_E_PARAM;
            }
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_REFRESH_TIME, value);
        }
#endif /* BCM_TRIDENT_SUPPORT */
        break;

      case bcmPortControlVrf:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_HAWKEYE_SUPPORT) || defined(BCM_HURRICANE_SUPPORT)
        if (SOC_IS_HAWKEYE(unit) || SOC_IS_HURRICANE(unit)) {
            return BCM_E_UNAVAIL;
        }
#endif /* BCM_HAWKEYE_SUPPORT || BCM_HURRICANE_SUPPORT */
        if (soc_mem_field_valid(unit, PORT_TABm, VRF_IDf)) {
            if ((value > 0 ) && (value < SOC_VRF_MAX(unit) )) {
                  rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                                            VRF_IDf, value);
                  BCM_IF_ERROR_RETURN(rv);
                  if (soc_mem_field_valid(unit, PORT_TABm, VRF_PORT_ENABLEf)) {
                        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                                            VRF_PORT_ENABLEf, 0x1);
                  }
            } else {
                    if (soc_mem_field_valid(unit, PORT_TABm, VRF_PORT_ENABLEf)) {
                        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                                            VRF_PORT_ENABLEf, 0x0);
                        BCM_IF_ERROR_RETURN(rv);
                    }
                    if (soc_mem_field_valid(unit, PORT_TABm, VRF_IDf)) {
                        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                                            VRF_IDf, 0x0);
                    }
            }
        }else if (soc_mem_field_valid(unit, PORT_TABm, PORT_VRFf)) {
            if ((value > 0 ) && (value < SOC_VRF_MAX(unit) )) {
                  rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                                            PORT_VRFf, value);
            } else {
                  rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                                            PORT_VRFf, 0x0);
            }
        }

#if defined(BCM_TRIUMPH_SUPPORT)
        else {
            if (soc_mem_field_valid(unit, SOURCE_TRUNK_MAP_TABLEm, VRF_IDf)) {
                if  ((value > 0 ) && (value < SOC_VRF_MAX(unit) )) {
                    rv = _bcm_trx_source_trunk_map_set(unit, port, VRF_IDf, value);
                    BCM_IF_ERROR_RETURN(rv);
                    if (soc_mem_field_valid(unit, PORT_TABm, PORT_OPERATIONf)) {
                        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                PORT_OPERATIONf, 0x3);
                    } 
                } else {
                    if (soc_mem_field_valid(unit, PORT_TABm, PORT_OPERATIONf)) {
                        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                PORT_OPERATIONf, 0x0);
                    }
                }
            }
        }
#endif  /* BCM_TRIUMPH_SUPPORT */
        break;

      case bcmPortControlL3Ingress:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRIUMPH_SUPPORT)
      {
          if (soc_mem_field_valid(unit, SOURCE_TRUNK_MAP_TABLEm, L3_IIFf)) {
              if ((value >= 1) && (value < soc_mem_index_count(unit, L3_IIFm))) {
                  rv = _bcm_trx_source_trunk_map_set(unit, port, L3_IIFf, value);
                  BCM_IF_ERROR_RETURN(rv);
                  if (soc_mem_field_valid(unit, PORT_TABm, PORT_OPERATIONf)) {
                      rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                              PORT_OPERATIONf, 0x2);
                  }
              } else {
                  if (soc_mem_field_valid(unit, PORT_TABm, PORT_OPERATIONf)) {
                      rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                              PORT_OPERATIONf, 0x0);
                  }
              }
          } 
      }
#endif  /* BCM_TRIUMPH_SUPPORT */
        break;

      case bcmPortControlL2Learn:
#if defined(BCM_TRX_SUPPORT)
          if (SOC_IS_TRX(unit)) {
              uint32 hw_val = 0;
              BCM_IF_ERROR_RETURN(
                  _bcm_trx_port_cml_flags2hw(unit, value, &hw_val));
              rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH, 
                                         CML_FLAGS_NEWf, hw_val);
          } 
#endif /* BCM_TRX_SUPPORT */
          break;

      case bcmPortControlL2Move:
#if defined(BCM_TRX_SUPPORT)
          if (SOC_IS_TRX(unit)) {
              uint32 hw_val = 0;
              BCM_IF_ERROR_RETURN(
                  _bcm_trx_port_cml_flags2hw(unit, value, &hw_val));
              rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH, 
                                         CML_FLAGS_MOVEf, hw_val);
          } 
#endif /* BCM_TRX_SUPPORT */
          break;

      case bcmPortControlForwardStaticL2MovePkt:
#if defined(BCM_TRX_SUPPORT)
         rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                    DISABLE_STATIC_MOVE_DROPf, value ? 1 : 0);
#endif /* BCM_TRX_SUPPORT */
        break;

    case bcmPortControlPrbsMode:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (value != 0) {
            return BCM_E_PARAM;
        }
        rv = BCM_E_NONE;
        break;

    case bcmPortControlPrbsPolynomial:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port, 
                                     SOC_PHY_CONTROL_PRBS_POLYNOMIAL, value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlPrbsTxEnable:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port, 
                                     SOC_PHY_CONTROL_PRBS_TX_ENABLE, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlPrbsRxEnable:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port, 
                                     SOC_PHY_CONTROL_PRBS_RX_ENABLE, value);
        PORT_UNLOCK(unit);
        break;

    case bcmPortControlLinkFaultLocalEnable:
    case bcmPortControlLinkFaultRemoteEnable:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        {
            soc_reg_t reg;
            soc_field_t field;

            if (soc_feature(unit, soc_feature_xmac)) {
                reg = XMAC_RX_LSS_CTRLr;
                if (type == bcmPortControlLinkFaultLocalEnable) {
                    field = LOCAL_FAULT_DISABLEf;
                } else {
                    field = REMOTE_FAULT_DISABLEf;
                }
            } else {
                reg = MAC_RXLSSCTRLr;
                if (type == bcmPortControlLinkFaultLocalEnable) {
                    field = LOCALFAULTDISABLEf;
                } else {
                    field = REMOTEFAULTDISABLEf;
                }
            }
            if (!soc_reg_field_valid(unit, reg, field)) {
                return BCM_E_UNAVAIL;
            }
            rv = soc_reg_field32_modify(unit, reg, port, field,
                                        (value) ? 0 : 1);
        }
        break;
    case bcmPortControlSerdesDriverTune:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port,
                SOC_PHY_CONTROL_SERDES_DRIVER_TUNE, value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlLinkdownTransmit:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port,
                SOC_PHY_CONTROL_LINKDOWN_TRANSMIT, value);
        PORT_UNLOCK(unit);
        if (rv == SOC_E_NONE) {
            rv = _bcm_esw_link_down_tx_set(unit, port, value);
        }
        break;
    case bcmPortControlSerdesTuneMarginMode:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port,
                SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE, value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlSerdesTuneMarginValue:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port,
                SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE, value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlStatOversize:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        rv = _bcm_esw_stat_ovr_threshold_set(unit, port, value);
        break;

    case bcmPortControlTimestampEnable: 
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_HAWKEYE_SUPPORT)
        if (SOC_IS_HAWKEYE(unit)) {
            uint32 rval;
            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
            SOC_IF_ERROR_RETURN(READ_EAV_ENABLE_BMAPr(unit, &rval));
            if (value) {
                rval |= (1 << port);
            } else {
                rval &= ~(1 << port);
            }
            SOC_IF_ERROR_RETURN(WRITE_EAV_ENABLE_BMAPr(unit, rval));

            /* Enable MMU */
            SOC_IF_ERROR_RETURN(WRITE_MMUEAVENABLEr(unit, rval));

            /* Program EGR_PORT register */
            SOC_IF_ERROR_RETURN(READ_EGR_PORTr(unit, port, &rval));
            soc_reg_field_set(unit, EGR_PORTr, &rval, EAV_CAPABLEf, value);
            SOC_IF_ERROR_RETURN(WRITE_EGR_PORTr(unit, port, rval));

            rv = BCM_E_NONE;
        }
#endif /* BCM_HAWKEYE_SUPPORT */
        break;

    case bcmPortControlEEEEnable:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }

        PORT_LOCK(unit);            

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        if (soc_feature (unit, soc_feature_eee)) {
            int mac_val;                
            uint32 phy_val;                

            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                            SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                            BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {

                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native EEE mode
                 */ 

                /* a. Disable AutoGrEEEn mode by PHY if applicable */
                rv = (soc_phyctrl_control_get(unit, port,
                            BCM_PORT_PHY_CONTROL_EEE_AUTO, &phy_val));

                if ((rv != BCM_E_UNAVAIL) && (phy_val != 0)) {
                    rv = soc_phyctrl_control_set (unit, port, 
                            BCM_PORT_PHY_CONTROL_EEE_AUTO, 0);
                }

                /* b. Enable/Disable Native EEE in PHY */
                rv = soc_phyctrl_control_set (unit, port, 
                        BCM_PORT_PHY_CONTROL_EEE, value ? 1 : 0);

                if (BCM_SUCCESS(rv)) {
                    /* EEE standard compliance Work Around:
                     * Store the software copy of eee value in eee_cfg flag
                     */
                    eee_cfg[unit][port] = value;
                    /* If (value==1), EEE will be enabled in MAC after 1 sec.
                     * during linkscan update*/
                    if (value == 0) {                        
                        /* Disable EEE in MAC immediately*/
                        rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                            SOC_MAC_CONTROL_EEE_ENABLE, 0);
                    }
                }

            } else {
                /* If native EEE mode is not supported,
                 * set PHY in AutoGrEEEn mode.
                 */
                
                /* a. Disable Native EEE mode in PHY if applicable */
                rv = (soc_phyctrl_control_get(unit, port,
                            BCM_PORT_PHY_CONTROL_EEE, &phy_val));

                if ((rv != BCM_E_UNAVAIL) && (phy_val != 0)) {
                    rv = soc_phyctrl_control_set (unit, port, 
                            BCM_PORT_PHY_CONTROL_EEE, 0);
                }

                /* b. Enable/Disable AutoGrEEEn in PHY.
                 * If PHY does not support AutoGrEEEn mode,
                 * rv will be assigned BCM_E_UNAVAIL.
                 */
                rv = soc_phyctrl_control_set (unit, port, 
                        BCM_PORT_PHY_CONTROL_EEE_AUTO, value ? 1 : 0);
            } 
        } else 
#endif /*BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */

        {
            /* For legacy devices that don't support native EEE,
             * set PHY in AutoGrEEEn mode.
             */
            uint32 phy_val;                

            /* a. Disable Native EEE mode in PHY if applicable */
            rv = (soc_phyctrl_control_get(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE, &phy_val));

            if ((rv != BCM_E_UNAVAIL) && (phy_val != 0)) {
                rv = soc_phyctrl_control_set (unit, port, 
                        BCM_PORT_PHY_CONTROL_EEE, 0);
            }

            /* b. Enable/Disable AutoGrEEEn in PHY.
             * If PHY does not support AutoGrEEEn mode,
             * rv will be assigned BCM_E_UNAVAIL.
             */
            rv = soc_phyctrl_control_set (unit, port, 
                    BCM_PORT_PHY_CONTROL_EEE_AUTO, value ? 1 : 0);
        }

        PORT_UNLOCK(unit);

        break;

    case bcmPortControlEEEStatisticsClear:

        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }

        PORT_LOCK(unit);

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_eee)) {
            uint32 val = 0;
            uint64 val64;
            int mac_val;                
            uint32 phy_val;                

            COMPILER_64_ZERO(val64);
            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                            SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                            BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {
                /* MAC/Switch is EEE aware (Native EEE mode is supported) */ 

                /*set counter rollover bit to 0*/
                SOC_IF_ERROR_RETURN(
                    READ_GE0_EEE_CONFIGr(unit, port, &val));
                soc_reg_field_set(unit, GE0_EEE_CONFIGr, &val,
                    COUNTER_ROLLOVERf, 0);
                SOC_IF_ERROR_RETURN(WRITE_GE0_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE1_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE2_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE3_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE4_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE5_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE6_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE7_EEE_CONFIGr(unit, port, val));

                /*Read counters to clear them*/
                SOC_IF_ERROR_RETURN(
                        READ_RX_EEE_LPI_DURATION_COUNTERr(unit, port, &val64));
                SOC_IF_ERROR_RETURN(
                        READ_RX_EEE_LPI_EVENT_COUNTERr(unit, port, &val64));
                SOC_IF_ERROR_RETURN(
                        READ_TX_EEE_LPI_DURATION_COUNTERr(unit, port, &val64));
                SOC_IF_ERROR_RETURN(
                        READ_TX_EEE_LPI_EVENT_COUNTERr(unit, port, &val64));

                /*set counter rollover bit to 1*/
                SOC_IF_ERROR_RETURN(
                    READ_GE0_EEE_CONFIGr(unit, port, &val));
                soc_reg_field_set(unit, GE0_EEE_CONFIGr, &val,
                    COUNTER_ROLLOVERf, 1);
                SOC_IF_ERROR_RETURN(WRITE_GE0_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE1_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE2_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE3_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE4_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE5_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE6_EEE_CONFIGr(unit, port, val));
                SOC_IF_ERROR_RETURN(WRITE_GE7_EEE_CONFIGr(unit, port, val));
            } else {
                /* native EEE mode is not supported, */
                rv = soc_phyctrl_control_set(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE_STATISTICS_CLEAR, value);
            }
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_TRIDENT_SUPPORT */
        {
            /* On legacy devices that don't support native EEE,
             * get counter value from PHY. If PHY does not support
             * EEE counters, rv will be assigned BCM_E_UNAVAIL.
             */
            rv = soc_phyctrl_control_set(unit, port,
                    BCM_PORT_PHY_CONTROL_EEE_STATISTICS_CLEAR, value);
        }
        PORT_UNLOCK (unit);
        break;


 /**********************      EEE Mode Overview    ************************
 *                      |DET|                              |  WT |
 *   Signalling   |idles|   |------------------------------|     | idles   |
 *   from Tx MAC  | or  |   |   Low Power Idle (LPI)       |idles|  or     |
 *   to local PHY |data |   |------------------------------|     | data    |
 *                          *                              * 
 *                          *                              *
 *                          *  -------LPI state------------*
 *   Local PHY    |         |  |      |  |       |  |      |   |           |
 *   signaling    |   Active|Ts|  Tq  |Tr|  Tq   |Tr|  Tq  |Tw |Active     |
 *   on MDI       |         |  |      |  |       |  |      |   |           |
 *                          *------------------------------*
 *                          *                              *
 *                          *                               *
 *   Signaling    |   idles |-------------------------------|id| PHY is    |  
 *   from LP PHY  |     or  |  Low Power Idle (LPI)         |le| ready     |
 *   to Rx MAC    |   data  |-------------------------------|s | for data  |
 *                                                          
 *   where DET = Delay Entry Timer    WT = Tx MAC Wake Timer
 */
    case bcmPortControlEEETransmitIdleTime:
        /* DET = Time (in microsecs) for which condition to move to LPI state 
         * is satisfied, at the end of which MAC TX transitions to LPI state */
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        if (soc_feature (unit, soc_feature_eee)) {
            rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                    SOC_MAC_CONTROL_EEE_TX_IDLE_TIME, value);
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */
        {
            rv = BCM_E_UNAVAIL;
        }
        PORT_UNLOCK(unit);
        break;
        
    case bcmPortControlEEETransmitRefreshTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_set(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_REFRESH_TIME, value);
        PORT_UNLOCK (unit);            
        break; 
        
    case bcmPortControlEEETransmitSleepTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK (unit);
        rv = soc_phyctrl_control_set(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_SLEEP_TIME, value);
        PORT_UNLOCK (unit);
        break;
        
    case bcmPortControlEEETransmitQuietTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);            
        rv = soc_phyctrl_control_set(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_QUIET_TIME, value);
        PORT_UNLOCK (unit); 
        break;
        
    case bcmPortControlEEETransmitWakeTime:
        /* Time(in microsecs) to wait before transmitter can leave LPI State*/
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }

        PORT_LOCK (unit);         

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
            if (soc_feature (unit, soc_feature_eee)) {
                rv = MAC_CONTROL_SET(PORT(unit, port).p_mac, unit, port,
                        SOC_MAC_CONTROL_EEE_TX_WAKE_TIME, value);
            } 
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */

        PORT_UNLOCK (unit);            
        break;
        
     case bcmPortControlEEEReceiveSleepTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK (unit);            
        rv = soc_phyctrl_control_set(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_RECEIVE_SLEEP_TIME, value);
        PORT_UNLOCK (unit); 
        break;
        
    case bcmPortControlEEEReceiveQuietTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);            
        rv = soc_phyctrl_control_set(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_RECEIVE_QUIET_TIME, value);
        PORT_UNLOCK(unit);        
        break;
        
    case bcmPortControlEEEReceiveWakeTime:
    	if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);            
        rv = soc_phyctrl_control_set (unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_RECEIVE_WAKE_TIME, value);
        PORT_UNLOCK(unit);        
        break;
        
    case bcmPortControlRemoteCpuEnable: 
    	if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (soc_feature(unit, soc_feature_rcpu_1)) {
           bcm_pbmp_t   pbmp;
           uint32       rval;

#ifdef BCM_KATANA_SUPPORT
           if (SOC_IS_KATANA (unit)) {
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_0r)) {
                   READ_CMIC_PKT_PORTS_0r(unit, &rval);
                   SOC_PBMP_WORD_SET(pbmp, 0, rval);
               }               
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_1r)) {
                   READ_CMIC_PKT_PORTS_1r(unit, &rval);
                   SOC_PBMP_WORD_SET(pbmp, 1, rval);
               }
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_2r)) {
                   READ_CMIC_PKT_PORTS_2r(unit, &rval);
                   SOC_PBMP_WORD_SET(pbmp, 2, rval);
               }

               if (value) {
                   SOC_PBMP_PORT_ADD(pbmp, port);
               } else {
                   SOC_PBMP_PORT_REMOVE(pbmp, port);
               }
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_0r)) {
                   rval = 0;
                   soc_reg_field_set(unit, CMIC_PKT_PORTS_0r, &rval, PORTSf, 
                                     SOC_PBMP_WORD_GET(pbmp, 0));
                   SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_PORTS_0r(unit, rval));
			   }
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_1r)) {
                   rval = 0;
                   soc_reg_field_set(unit, CMIC_PKT_PORTS_1r, &rval, PORTS_HIf, 
                                 SOC_PBMP_WORD_GET(pbmp, 1));
                   SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_PORTS_1r(unit, rval));               
               }
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_2r)) {
                   rval = 0;
                   soc_reg_field_set(unit, CMIC_PKT_PORTS_2r, &rval, PORTS_HIf, 
                                 SOC_PBMP_WORD_GET(pbmp, 2));
                   SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_PORTS_2r(unit, rval));               
               }
           } else
#endif
           {
               READ_CMIC_PKT_PORTSr(unit, &rval);
               SOC_PBMP_WORD_SET(pbmp, 0, rval);
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_HIr)) {
                   READ_CMIC_PKT_PORTS_HIr(unit, &rval);
                   SOC_PBMP_WORD_SET(pbmp, 1, rval);
               }
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_HI_2r)) {
                   READ_CMIC_PKT_PORTS_HI_2r(unit, &rval);
                   SOC_PBMP_WORD_SET(pbmp, 2, rval);
               }

               if (value) {
                   SOC_PBMP_PORT_ADD(pbmp, port);
               } else {
                   SOC_PBMP_PORT_REMOVE(pbmp, port);
               }
               rval = 0;
               soc_reg_field_set(unit, CMIC_PKT_PORTSr, &rval, PORTSf, 
                                 SOC_PBMP_WORD_GET(pbmp, 0));
               SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_PORTSr(unit, rval));
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_HIr)) {
                   rval = 0;
                   soc_reg_field_set(unit, CMIC_PKT_PORTS_HIr, &rval, PORTS_HIf, 
                                 SOC_PBMP_WORD_GET(pbmp, 1));
                   SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_PORTS_HIr(unit, rval));               
               }
               if (SOC_REG_IS_VALID(unit, CMIC_PKT_PORTS_HI_2r)) {
                   rval = 0;
                   soc_reg_field_set(unit, CMIC_PKT_PORTS_HI_2r, &rval, PORTS_HIf, 
                                 SOC_PBMP_WORD_GET(pbmp, 2));
                   SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_PORTS_HI_2r(unit, rval));               
               }
           }

           rv = BCM_E_NONE;
        } else {
            rv = BCM_E_UNAVAIL;
        }
        break;

    case bcmPortControlTrill:
#if defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, TRILL_ENABLEf)) {
                   rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       TRILL_ENABLEf, (value) ? 1 : 0);
              }  
              if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm, TRILL_ENABLEf) && BCM_SUCCESS(rv)) {
                   rv = soc_mem_field32_modify(unit, EGR_PORTm, port, 
                                       TRILL_ENABLEf, (value) ? 1 : 0);
              }
        }
#endif /* BCM_TRIDENT_SUPPORT */
       break;

    case bcmPortControlTrillAllow:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, ALLOW_TRILL_FRAMESf)) {
                   rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       ALLOW_TRILL_FRAMESf, (value) ? 1 : 0);
              }
              if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm, ALLOW_TRILL_FRAMESf) && BCM_SUCCESS(rv)) {
                   rv = soc_mem_field32_modify(unit, EGR_PORTm, port, 
                                       ALLOW_TRILL_FRAMESf, (value) ? 1 : 0);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNonTrillAllow:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, ALLOW_NON_TRILL_FRAMESf)) {
                   rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       ALLOW_NON_TRILL_FRAMESf, (value) ? 1 : 0);
              }
              if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm, ALLOW_NON_TRILL_FRAMESf) && BCM_SUCCESS(rv)) {
                   rv = soc_mem_field32_modify(unit, EGR_PORTm, port, 
                                       ALLOW_NON_TRILL_FRAMESf, (value) ? 1 : 0);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlTrillCoreISISToCPU:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, COPY_CORE_IS_IS_TO_CPUf)) {
                   rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       COPY_CORE_IS_IS_TO_CPUf, (value) ? 1 : 0);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlTrillHashSelect:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, RTAG7_HASH_CFG_SEL_TRILL_ECMPf)) {
                   rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       RTAG7_HASH_CFG_SEL_TRILL_ECMPf, (value) ? 1 : 0);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivAccessVirtualInterfaceId:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             bcm_module_t my_modid;
             int mod_port_index;

             if ((value < 0) || (value > 0xfff)) {
                 return BCM_E_PARAM;
             }
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit, port,
                         _BCM_CPU_TABS_ETHER, NIV_VIF_IDf, value));
             BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, EGR_PORTm,
                         port, NIV_VIF_IDf, value));
             BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));
             BCM_IF_ERROR_RETURN(_bcm_esw_src_mod_port_table_index_get(unit,
                         my_modid, port, &mod_port_index));
             SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                         EGR_GPP_ATTRIBUTESm, mod_port_index,
                         SRC_NIV_VIF_IDf, value));
             rv = BCM_E_NONE;
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivNonVntagDrop:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                     DISCARD_IF_VNTAG_NOT_PRESENTf, (value) ? 1 : 0);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagDrop:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                     DISCARD_IF_VNTAG_PRESENTf, (value) ? 1 : 0);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivNonVntagAdd:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                     VNTAG_ACTIONS_IF_NOT_PRESENTf,
                     (value) ? VNTAG_ADD : VNTAG_NOOP);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagIngressReplace:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                     VNTAG_ACTIONS_IF_PRESENTf,
                     (value) ? VNTAG_REPLACE : VNTAG_NOOP);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagEgressReplace:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                     VNTAG_ACTIONS_IF_PRESENTf,
                     (value) ? VNTAG_REPLACE : VNTAG_NOOP);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagIngressDelete:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                     VNTAG_ACTIONS_IF_PRESENTf,
                     (value) ? VNTAG_DELETE : VNTAG_NOOP);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagEgressDelete:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                     VNTAG_ACTIONS_IF_PRESENTf,
                     (value) ? VNTAG_DELETE: VNTAG_NOOP);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivRpfCheck:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                     NIV_RPF_CHECK_ENABLEf, (value) ? 1 : 0);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivSourceKnockout:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                     NIV_PRUNE_ENABLEf, (value) ? 1 : 0);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivForwardPort:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             bcm_module_t my_modid;
             int mod_port_index;
             bcm_module_t mod_out;
             bcm_port_t port_out;
             bcm_trunk_t trunk_out;
             int id_out;
             int tx_dest_port;

             if (value == -1) {
                 BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit, port,
                             _BCM_CPU_TABS_ETHER, NIV_UPLINK_PORTf, 1));
                 BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, EGR_PORTm, 
                                             port, NIV_UPLINK_PORTf, 1));
                 BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));
                 BCM_IF_ERROR_RETURN(_bcm_esw_src_mod_port_table_index_get(unit,
                             my_modid, port, &mod_port_index));
                 SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                             EGR_GPP_ATTRIBUTESm, mod_port_index,
                             SRC_IS_NIV_UPLINK_PORTf, 1));
                 rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                         NIV_VIF_LOOKUP_ENABLEf, 1);
             } else {
                 BCM_IF_ERROR_RETURN
                     (_bcm_esw_gport_resolve(unit, value, &mod_out, &port_out,
                                             &trunk_out, &id_out));
                 if (id_out != -1) {
                     return BCM_E_PARAM;
                 } else if (trunk_out != -1) {
                     tx_dest_port = (1 << SOC_TRUNK_BIT_POS(unit)) + trunk_out;
                 } else {
                     tx_dest_port = mod_out * (SOC_PORT_ADDR_MAX(unit) + 1) +
                                    port_out;
                 }
                 BCM_IF_ERROR_RETURN
                     (_bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                         TX_DEST_PORTf, tx_dest_port));
                 rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                         TX_DEST_PORT_ENABLEf, 1);
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivType:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             bcm_module_t my_modid;
             int mod_port_index;

             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         DISCARD_IF_VNTAG_NOT_PRESENTf, 0));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         DISCARD_IF_VNTAG_PRESENTf, 0));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         VNTAG_ACTIONS_IF_PRESENTf, VNTAG_NOOP));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         VNTAG_ACTIONS_IF_NOT_PRESENTf, VNTAG_NOOP));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         TX_DEST_PORT_ENABLEf, 0));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         NIV_VIF_LOOKUP_ENABLEf, 0));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         NIV_RPF_CHECK_ENABLEf, 0));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         NIV_UPLINK_PORTf, 0));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         VT_ENABLEf, 0));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                         port, _BCM_CPU_TABS_ETHER,
                         DISABLE_VLAN_CHECKSf, 0));
             BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                         EGR_PORTm, port,
                         VNTAG_ACTIONS_IF_PRESENTf, VNTAG_NOOP));
             BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                         EGR_PORTm, port,
                         NIV_PRUNE_ENABLEf, 0));
             BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                         EGR_PORTm, port,
                         NIV_UPLINK_PORTf, 0));
             BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));
             BCM_IF_ERROR_RETURN(_bcm_esw_src_mod_port_table_index_get(unit,
                         my_modid, port, &mod_port_index));
             SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                         EGR_GPP_ATTRIBUTESm, mod_port_index,
                         SRC_IS_NIV_UPLINK_PORTf, 0));
             switch (value) {
                 case BCM_PORT_NIV_TYPE_SWITCH:
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 VT_KEY_TYPEf, TR_VLXLT_HASH_KEY_TYPE_VIF));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 VT_KEY_TYPE_USE_GLPf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 VT_ENABLEf, 1));
                     break;
                 case BCM_PORT_NIV_TYPE_UPLINK_ACCESS:
                 case BCM_PORT_NIV_TYPE_UPLINK_TRANSIT:
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 DISCARD_IF_VNTAG_NOT_PRESENTf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 NIV_VIF_LOOKUP_ENABLEf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 NIV_UPLINK_PORTf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 DISABLE_VLAN_CHECKSf, 1));
                     BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                                 EGR_PORTm, port,
                                 NIV_UPLINK_PORTf, 1));
                     SOC_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                                 EGR_GPP_ATTRIBUTESm, mod_port_index,
                                 SRC_IS_NIV_UPLINK_PORTf, 1));
                     break;
                 case BCM_PORT_NIV_TYPE_DOWNLINK_ACCESS:
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 DISCARD_IF_VNTAG_PRESENTf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 VNTAG_ACTIONS_IF_NOT_PRESENTf, VNTAG_ADD));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 TX_DEST_PORT_ENABLEf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 DISABLE_VLAN_CHECKSf, 1));
                     BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                                 EGR_PORTm, port,
                                 VNTAG_ACTIONS_IF_PRESENTf, VNTAG_DELETE));
                     BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit,
                                 EGR_PORTm, port,
                                 NIV_PRUNE_ENABLEf, 1));
                     break;
                 case BCM_PORT_NIV_TYPE_DOWNLINK_TRANSIT:
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 DISCARD_IF_VNTAG_NOT_PRESENTf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 TX_DEST_PORT_ENABLEf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 NIV_RPF_CHECK_ENABLEf, 1));
                     BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_set(unit,
                                 port, _BCM_CPU_TABS_ETHER,
                                 DISABLE_VLAN_CHECKSf, 1));
                     break;
                 default:
                     break;
             }
             rv = BCM_E_NONE;
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivNameSpace:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             if ((value < 0) || (value > 0xfff)) {
                 return BCM_E_PARAM;
             }
             rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                     NIV_NAMESPACEf, value);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;
    case bcmPortControlFabricSourceKnockout:
        if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, REMOVE_HG_HDR_SRC_PORTf)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                REMOVE_HG_HDR_SRC_PORTf, (value) ? 1 : 0);
        }
        break;
    case bcmPortControlRxEnable:
        rv = _bcm_esw_mac_rx_control(unit, port, 0, &value);
        break;
    case bcmPortControlTxEnable:
        if (SOC_REG_IS_VALID(unit, COSMASKr)) {
            /* Use egress scheduling control if available */
            uint32 val; 
            if (value > 0) {
                if (!(PORT(unit, port).flags & _PORT_INFO_STOP_TX)) {
                    /* Port tx was never stopped */
                    rv = BCM_E_NONE;
                    break;
                }
                /* Restore values */
                val = PORT(unit, port).cosmask;
                rv =  soc_reg_field32_modify(unit, COSMASKr, port, 
                                             COSMASKf, val);
                /* clear flag */
                PORT(unit, port).flags &= ~_PORT_INFO_STOP_TX;
            } else {
                uint32 rval;
                /* Read and save current values */
                SOC_IF_ERROR_RETURN(READ_COSMASKr(unit, port, &rval));
                PORT(unit, port).cosmask = soc_reg_field_get(unit, COSMASKr, 
                                                             rval, COSMASKf);
                /* set flag */
                PORT(unit, port).flags |= _PORT_INFO_STOP_TX;
                /* Disable Tx */
                val = (1 << soc_reg_field_length(unit, COSMASKr, 
                                                 COSMASKf)) - 1;
                rv =  soc_reg_field32_modify(unit, COSMASKr, port, 
                                             COSMASKf, val);
            }
        } else { /* Use egress metering control if available */ 
            uint32 rval1 = 0, rval2 = 0;
            if (!(SOC_REG_IS_VALID(unit, EGRMETERINGCONFIGr)) || 
                !(SOC_REG_IS_VALID(unit, EGRMETERINGBUCKETr))) {
                break; 
            }
            if (value > 0 ) {
                if (!(PORT(unit, port).flags & _PORT_INFO_STOP_TX)) {
                    /* Port tx was never stopped */
                    rv = BCM_E_NONE;
                    break;
                }
                /* Restore values */
                BCM_IF_ERROR_RETURN(READ_EGRMETERINGCONFIGr(unit, port, 
                                                            &rval1));
                BCM_IF_ERROR_RETURN(READ_EGRMETERINGBUCKETr(unit, port, 
                                                            &rval2));
                soc_reg_field_set(unit, EGRMETERINGCONFIGr, &rval1,
                                  REFRESHf, PORT(unit, port).m_info.refresh);
                soc_reg_field_set(unit, EGRMETERINGCONFIGr, &rval1,
                                  THD_SELf, PORT(unit, port).m_info.threshold);
                soc_reg_field_set(unit, EGRMETERINGBUCKETr, &rval2,
                                  BUCKETf, PORT(unit, port).m_info.bucket);
                soc_reg_field_set(unit, EGRMETERINGBUCKETr, &rval2,
                                  IN_PROFILE_FLAGf, 
                                  PORT(unit, port).m_info.in_profile);
                BCM_IF_ERROR_RETURN(WRITE_EGRMETERINGCONFIGr(unit, port, 
                                                             rval1));
                BCM_IF_ERROR_RETURN(WRITE_EGRMETERINGBUCKETr(unit, port,
                                                             rval2)); 
                /* clear flag */
                PORT(unit, port).flags &= ~_PORT_INFO_STOP_TX;
                rv = BCM_E_NONE;

            } else {
                /* Read and save current values */
                BCM_IF_ERROR_RETURN(READ_EGRMETERINGCONFIGr(unit, port, 
                                                            &rval1));
                PORT(unit, port).m_info.refresh = 
                    soc_reg_field_get(unit, EGRMETERINGCONFIGr, rval1,
                                      REFRESHf);
                PORT(unit, port).m_info.threshold = 
                    soc_reg_field_get(unit, EGRMETERINGCONFIGr, rval1,
                                      THD_SELf);
                BCM_IF_ERROR_RETURN(READ_EGRMETERINGBUCKETr(unit, port, 
                                                            &rval2));
                PORT(unit, port).m_info.bucket = 
                    soc_reg_field_get(unit, EGRMETERINGBUCKETr, rval2,
                                      BUCKETf);
                PORT(unit, port).m_info.in_profile = 
                    soc_reg_field_get(unit, EGRMETERINGBUCKETr, rval2,
                                      IN_PROFILE_FLAGf);
                /* set flag */
                PORT(unit, port).flags |= _PORT_INFO_STOP_TX;
                /* Disable Tx */
                soc_reg_field_set(unit, EGRMETERINGCONFIGr, &rval1,
                                  REFRESHf, 0);
                soc_reg_field_set(unit, EGRMETERINGCONFIGr, &rval1,
                                  THD_SELf, 1);
                soc_reg_field_set(unit, EGRMETERINGBUCKETr, &rval2,
                                  BUCKETf, 0x100000);
                soc_reg_field_set(unit, EGRMETERINGBUCKETr, &rval2,
                                  IN_PROFILE_FLAGf, 1);
                BCM_IF_ERROR_RETURN(WRITE_EGRMETERINGCONFIGr(unit, port, 
                                                             rval1));
                BCM_IF_ERROR_RETURN(WRITE_EGRMETERINGBUCKETr(unit, port,
                                                             rval2)); 
                rv = BCM_E_NONE;
            }
        } 
        break;

    case bcmPortControlMplsEntropyHashSet:
#if defined(BCM_KATANA_SUPPORT)
         if (soc_feature(unit, soc_feature_mpls)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, RTAG7_HASH_CFG_SEL_ENTROPY_LABELf)) {
                   rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_ETHER,
                                       RTAG7_HASH_CFG_SEL_ENTROPY_LABELf, (value) ? 1 : 0);
              }
         }
#endif /* BCM_KATANA_SUPPORT */
         break;

    default:
        break;
    }
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_control_get
 * Description:
 *      Get the status of specified port feature.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      type - Enum  value of the feature
 *      value - (OUT) Current value of the port feature
 * Return Value:
 *      BCM_E_NONE
 *      BCM_E_UNAVAIL - Functionality not available
 */
int 
bcm_esw_port_control_get(int unit, bcm_port_t port, 
                         bcm_port_control_t type, int *value)
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_SHADOW_SUPPORT)
    if (SOC_IS_SHADOW(unit)) {
        /*  If IL port and unsupported port type, return unavail */
        if (IS_IL_PORT(unit, port) || 
            !_bcm_port_type_shadow_support (unit, port, type)) {
            return BCM_E_UNAVAIL;
        }
    }
#endif /* BCM_SHADOW_SUPPORT */


#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return bcm_td_vp_control_get(unit, port, type, value);
    } else
#endif
    if (BCM_GPORT_IS_WLAN_PORT(port)) {
        rv = BCM_E_NONE;
    } else {
        rv = _bcm_esw_port_gport_validate(unit, port, &port);
    }
    BCM_IF_ERROR_RETURN(rv);
    rv = BCM_E_UNAVAIL;

    switch (type) {
      case bcmPortControlBridge:
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            rv = _bcm_tr2_lport_field_get(unit, port, PORT_BRIDGEf, value);
        } else
#endif
        {
            if (SOC_IS_XGS3_SWITCH(unit)) {
                rv = _bcm_esw_port_tab_get(unit, port, PORT_BRIDGEf, value);
            }
#ifdef BCM_EASYRIDER_SUPPORT
            else if (SOC_IS_EASYRIDER(unit) && IS_HG_PORT(unit, port)) {
                bcm_port_cfg_t      pcfg;

                PORT_LOCK(unit);
                rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg);
                if (BCM_SUCCESS(rv)) {
                    *value = pcfg.pc_bridge_port;
                }
                PORT_UNLOCK(unit);
            }
#endif
        }
          break;
      case bcmPortControlTrunkHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
          if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
              rv = _bcm_esw_port_tab_get(unit, port, 
                                         RTAG7_HASH_CFG_SEL_TRUNKf, value);
          }
#endif /* BCM_BRADLEY_SUPPORT */
          break;
      case bcmPortControlFabricTrunkHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
          if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
              rv = _bcm_esw_port_tab_get(unit, port, 
                                         RTAG7_HASH_CFG_SEL_HIGIG_TRUNKf, value);
          }
#endif /* BCM_BRADLEY_SUPPORT */
          break;
      case bcmPortControlECMPHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
          if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
              rv = _bcm_esw_port_tab_get(unit, port, 
                                         RTAG7_HASH_CFG_SEL_ECMPf, value);
          }
#endif /* BCM_BRADLEY_SUPPORT */
          break;
      case bcmPortControlLoadBalanceHashSet:
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
            rv = _bcm_esw_port_tab_get(unit, port, 
                                       RTAG7_HASH_CFG_SEL_LBIDf, value);
        }
#endif /* BCM_BRADLEY_SUPPORT */
        break;
    case bcmPortControlLoadBalancingNumber:
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(unit) || SOC_IS_TRX(unit)) {
            rv = _bcm_esw_port_tab_get(unit, port, 
                                       RTAG7_PORT_LBNf, value);
        }
#endif /* BCM_BRADLEY_SUPPORT */
        break;
      case bcmPortControlErrorSymbolDetect:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_XGS3_SWITCH(unit)) {
             rv = _bcm_esw_port_err_sym_detect_get(unit, port, value);
        }
#endif
        break;
      case bcmPortControlErrorSymbolCount:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_XGS3_SWITCH(unit)) {
            rv = _bcm_esw_port_err_sym_count_get(unit, port, value);
        }
#endif
        break;
      case bcmPortControlIP4:
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
              rv = _bcm_esw_port_tab_get(unit, port, V4L3_ENABLEf, value);
          }
          break;
      case bcmPortControlIP6:
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
              rv = _bcm_esw_port_tab_get(unit, port, V6L3_ENABLEf, value);
          }
          break;
      case bcmPortControlIP4Mcast:
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
              rv = _bcm_esw_port_tab_get(unit, port, V4IPMC_ENABLEf, value);
          }
          break;
      case bcmPortControlIP6Mcast:
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port))) {
              rv = _bcm_esw_port_tab_get(unit, port, V6IPMC_ENABLEf, value);
          }
          break;
      case bcmPortControlIP4McastL2:
#ifdef BCM_TRX_SUPPORT
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) && 
              SOC_MEM_FIELD_VALID(unit, PORT_TABm, V4IPMC_L2_ENABLEf)) {
              rv = _bcm_esw_port_tab_get(unit, port, V4IPMC_L2_ENABLEf, value);
          }
#endif
          break;
      case bcmPortControlIP6McastL2:
#ifdef BCM_TRX_SUPPORT
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) && 
              SOC_MEM_FIELD_VALID(unit, PORT_TABm, V6IPMC_L2_ENABLEf)) {
              rv = _bcm_esw_port_tab_get(unit, port, V6IPMC_L2_ENABLEf, value);
          }
#endif
          break;

      case bcmPortControlMpls:
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) &&
              SOC_MEM_FIELD_VALID(unit, PORT_TABm, MPLS_ENABLEf)) {
              rv = _bcm_esw_port_tab_get(unit, port, MPLS_ENABLEf, value);
          }
#endif
          break;

      case bcmPortControlMacInMac:
#if defined(BCM_TRIUMPH2_SUPPORT)
          if (SOC_IS_XGS3_SWITCH(unit) && 
              (BCM_GPORT_IS_WLAN_PORT(port) || !IS_ST_PORT(unit, port)) &&
              SOC_MEM_FIELD_VALID(unit, PORT_TABm, MIM_TERM_ENABLEf)) {
              rv = _bcm_esw_port_tab_get(unit, port, MIM_TERM_ENABLEf, value);
          }
#endif
          break;
      case bcmPortControlFabricQueue:
          if (BCM_GPORT_IS_WLAN_PORT(port)) {
              return BCM_E_UNAVAIL;
          }
#if defined(BCM_TRIUMPH2_SUPPORT)
          if (IS_ST_PORT(unit, port)) {
              if  (SOC_REG_FIELD_VALID(unit, EGR_PORT_64r, 
                                       EH_EXT_HDR_ENABLEf)) {
                  uint64 egr_val64;
                  rv = READ_EGR_PORT_64r(unit, port, &egr_val64);
                  if (BCM_SUCCESS(rv)) {
                      *value = soc_reg64_field32_get(unit, EGR_PORT_64r, 
                              egr_val64, EH_EXT_HDR_ENABLEf);
                  }
              }
          }
#endif
          break;

    case bcmPortControlMplsIngressPortCheck:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_EASYRIDER_SUPPORT
        if (!IS_ST_PORT(unit, port) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, MPLS_PORT_CHECKf)) {
            rv = _bcm_esw_port_tab_get(unit, port, MPLS_PORT_CHECKf, value);
        }
#endif /* BCM_EASYRIDER_SUPPORT */
        break;

    case bcmPortControlMplsMultiLabelSwitching:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_EASYRIDER_SUPPORT
        if (!IS_ST_PORT(unit, port) &&
            SOC_MEM_FIELD_VALID(unit, PORT_TABm, THREE_MPLS_LABELf)) {
            rv = _bcm_esw_port_tab_get(unit, port, THREE_MPLS_LABELf, value);
        }
#endif /* BCM_EASYRIDER_SUPPORT */
        break;

    case bcmPortControlPassControlFrames:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_XGS_SWITCH_SUPPORT
        if (IS_XE_PORT(unit, port)) {
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_PASS_CONTROL_FRAME, value);
        }
#endif
#ifdef BCM_XGS12_SWITCH_SUPPORT
        if ((SOC_IS_DRACO(unit) || SOC_IS_TUCANA(unit)) && 
            IS_E_PORT(unit, port)) {
            uint64 rsv_mask;
            rv = READ_RSV_MASKr(unit, port, &rsv_mask);
            if (BCM_SUCCESS(rv)) {
                *value = (COMPILER_64_LO(rsv_mask) & 0x08000000) ? 0 : 1;
            }
        }
#endif
        break;
#ifdef BCM_XGS3_SWITCH_SUPPORT
    case bcmPortControlFilterLookup:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        if (soc_mem_field_valid(unit, PORT_TABm, VFP_ENABLEf)) {
            rv = _bcm_esw_port_tab_get(unit, port, VFP_ENABLEf, value);
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case bcmPortControlFilterIngress:
        rv = _bcm_esw_port_tab_get(unit, port, FILTER_ENABLEf, value);
        break;
    case bcmPortControlFilterEgress:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        } else
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        {
            if (SOC_REG_FIELD_VALID(unit, EGR_PORTr, EFP_FILTER_ENABLEf)) {
                uint32 egr_val;

                rv = READ_EGR_PORTr(unit, port, &egr_val);
                if (BCM_SUCCESS(rv)) {
                    *value = soc_reg_field_get(unit, EGR_PORTr, egr_val,
                                               EFP_FILTER_ENABLEf);
                }
            } else if (SOC_REG_FIELD_VALID(unit, EGR_PORT_64r,
                                           EFP_FILTER_ENABLEf)) {
                uint64 egr_val64;
            
                rv = READ_EGR_PORT_64r(unit, port, &egr_val64);
                if (BCM_SUCCESS(rv)) {
                    *value = soc_reg64_field32_get(unit, EGR_PORT_64r, 
                                                   egr_val64,
                                                   EFP_FILTER_ENABLEf);
                }
            } else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm,
                                           EFP_FILTER_ENABLEf)) {
                egr_port_entry_t entry;
            
                rv = soc_mem_read(unit, EGR_PORTm, MEM_BLOCK_ANY, port,
                                  &entry);
                if (BCM_SUCCESS(rv)) {
                    *value = soc_mem_field32_get(unit, EGR_PORTm, &entry,
                                                 EFP_FILTER_ENABLEf);
                }
            }
        }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
      case bcmPortControlFrameSpacingStretch:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
          rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                               SOC_MAC_CONTROL_FRAME_SPACING_STRETCH, value);
          break;
      case bcmPortControlPreservePacketPriority:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
          rv = _bcm_esw_port_tab_get(unit, port, USE_INCOMING_DOT1Pf, value);
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
          break;
      case bcmPortControlLearnClassEnable:
#if defined(BCM_TRX_SUPPORT) 
           if (soc_feature(unit, soc_feature_class_based_learning)) {
               rv = _bcm_esw_port_tab_get(unit, port, CLASS_BASED_SM_ENABLEf,
                                          value);
           }
#endif /* BCM_TRX_SUPPORT */
          break;
      case bcmPortControlTrustIncomingVlan:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_TRX_SUPPORT)
           rv = _bcm_esw_port_tab_get(unit, port, TRUST_INCOMING_VIDf, value);
#endif /* BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
          break;
      case bcmPortControlDoNotCheckVlan:
#if defined(BCM_TRX_SUPPORT)
          if (!SOC_IS_SHADOW(unit)) {
              rv = _bcm_esw_port_tab_get(unit, port, DISABLE_VLAN_CHECKSf, value);
          }
#endif /* BCM_TRX_SUPPORT */
          break;
       case bcmPortControlIEEE8021ASEnableIngress:
#if defined(BCM_HAWKEYE_SUPPORT) || defined(BCM_ENDURO_SUPPORT)
          if (soc_feature(unit, soc_feature_rx_timestamp)) {
              rv = _bcm_esw_port_tab_get(unit, port, IEEE_802_1AS_ENABLEf, value);
          }
#endif /* BCM_HAWKEYE_SUPPORT */
          break;
       case bcmPortControlIEEE8021ASEnableEgress:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_HAWKEYE_SUPPORT)
          if (SOC_IS_HAWKEYE(unit)) {
              uint32 egr_val;

              rv = READ_EGR_PORTr(unit, port, &egr_val);
              if (BCM_SUCCESS(rv)) {
                  *value = soc_reg_field_get(unit, EGR_PORTr, egr_val,
                                           IEEE_802_1AS_ENABLEf);
              }
          }
#endif /* BCM_HAWKEYE_SUPPORT */
          break;
      case bcmPortControlEgressVlanPriUsesPktPri:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRX_SUPPORT)
          if (SOC_IS_TRX(unit) && !SOC_IS_SHADOW(unit)) {
              uint32 val;
              rv = READ_EGR_VLAN_CONTROL_1r(unit, port, &val);
              if (BCM_SUCCESS(rv)) {
                  *value = (soc_reg_field_get(unit, EGR_VLAN_CONTROL_1r, val,
                                              REMARK_OUTER_DOT1Pf)) ? 0 : 1;
              }
          }
#endif /* BCM_TRX_SUPPORT */
          break;
      case bcmPortControlEgressModifyDscp:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRX_SUPPORT) 
          if (SOC_IS_TRX(unit) && !SOC_IS_SHADOW(unit)) {
              uint32 val;
              rv = READ_EGR_VLAN_CONTROL_1r(unit, port, &val);
              if (BCM_SUCCESS(rv)) {
                  *value = (soc_reg_field_get(unit, EGR_VLAN_CONTROL_1r, val,
                                              REMARK_OUTER_DSCPf)) ? 1 : 0;
              }
          }
#endif /* BCM_TRX_SUPPORT */
          break;
    case bcmPortControlIpfixRate:
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_ipfix_rate)) {
            rv = _bcm_esw_port_tab_get(unit, port, IPFIX_FLOW_METER_IDf,
                                       value);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        break;
    case bcmPortControlCustomerQueuing:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#ifdef BCM_ENDURO_SUPPORT
        if (SOC_IS_ENDURO(unit) && !IS_HG_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
#endif
#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_vlan_queue_map)) {
            uint32 rval;
            soc_field_t field; 
            rv = READ_ING_COS_MODEr(unit, port, &rval);
            if (SOC_REG_FIELD_VALID(unit, ING_COS_MODEr, SELECTf)) {
                field = SELECTf;
            } else if (SOC_REG_FIELD_VALID(unit, ING_COS_MODEr, QUEUE_MODEf)) {
                field = QUEUE_MODEf;
            } else {
                rv = BCM_E_UNAVAIL;
                break;
            }
            if (soc_reg_field_get(unit, ING_COS_MODEr, rval, field) == 3) {  
                *value = 1;
            } else {
                *value = 0;
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
        break;
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT)
      case bcmPortControlOAMEnable:
          if (BCM_GPORT_IS_WLAN_PORT(port)) {
              return BCM_E_UNAVAIL;
          }
          rv = _bcm_esw_port_tab_get(unit, port, OAM_ENABLEf, value);
          break;
#endif /* defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT) */

#ifdef BCM_TRX_SUPPORT
      case bcmPortControlOamLoopback:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
          if (SOC_IS_TRX(unit) && !SOC_IS_HURRICANE(unit) && !SOC_IS_KATANA(unit)
              && !SOC_IS_SHADOW(unit))
          {
              uint32 reg_value;

              rv = READ_ING_MISC_PORT_CONFIGr(unit, port, &reg_value);

              if (SOC_SUCCESS(rv))
              {
                  *value = soc_reg_field_get(unit, ING_MISC_PORT_CONFIGr,
                      reg_value, OAM_DO_NOT_MODIFYf);
              }
          }

          break;
#endif /* BCM_TRX_SUPPORT */

    case bcmPortControlLanes:
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (soc_feature(unit, soc_feature_flex_port)) {
#if defined(BCM_ENDURO_SUPPORT)
            if (SOC_IS_ENDURO(unit)) {
                rv = _bcm_en_port_lanes_get(unit, port, value);
            } else 
#endif
#if defined(BCM_HURRICANE_SUPPORT)
            if (SOC_IS_HURRICANE(unit)) {
                rv = _bcm_hu_port_lanes_get(unit, port, value);
            } else 
#endif
#ifdef BCM_TRIDENT_SUPPORT
            if (SOC_IS_TD_TT(unit)) {
                if (!SOC_PORT_VALID(unit, port)) {
                    return BCM_E_PORT;
                }
                *value = SOC_INFO(unit).port_num_lanes[port];
                rv = BCM_E_NONE;
            } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
            if (SOC_IS_SHADOW(unit)) {
                if (!SOC_PORT_VALID(unit, port)) {
                    return BCM_E_PORT;
                }
                *value = SOC_INFO(unit).port_num_lanes[port];
                rv = BCM_E_NONE;
            } else
#endif /* BCM_SHADOW_SUPPORT */
            {
                rv = _bcm_tr2_port_lanes_get(unit, port, value);
            }
        }
#endif
        break;
    case bcmPortControlPFCEthertype:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_TYPE, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
          break;
    case bcmPortControlPFCOpcode:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_OPCODE, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
          break;
    case bcmPortControlPFCReceive:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_RX_ENABLE, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
          break;
    case bcmPortControlPFCTransmit:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_TX_ENABLE, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
          break;
    case bcmPortControlPFCClasses:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_CLASSES, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;
    case bcmPortControlPFCPassFrames:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_RX_PASS, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCDestMacOui:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_MAC_DA_OUI, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCDestMacNonOui:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT) 
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            if (!IS_GX_PORT(unit, port)) { /* IS_GX_PORT == IS_XL_PORT */
                return BCM_E_PORT;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_MAC_DA_NONOUI, value);
        }
#endif /* BCM_SCORPION_SUPPORT || BCM_TRIUMPH2_SUPPORT */
        break;

    case bcmPortControlPFCRefreshTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_priority_flow_control)) {
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_PFC_REFRESH_TIME, value);
        }
#endif /* BCM_TRIDENT_SUPPORT */
        break;

      case bcmPortControlVrf:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_HAWKEYE_SUPPORT) || defined(BCM_HURRICANE_SUPPORT)
        if (SOC_IS_HAWKEYE(unit) || SOC_IS_HURRICANE(unit)) {
            return BCM_E_UNAVAIL;
        }
#endif /* BCM_HAWKEYE_SUPPORT || BCM_HURRICANE_SUPPORT */
        if (soc_mem_field_valid(unit, PORT_TABm, VRF_PORT_ENABLEf)) {
            rv = _bcm_esw_port_tab_get(unit, port, VRF_PORT_ENABLEf, value);

            if ((*value == 0x1 ) && (soc_mem_field_valid(unit, PORT_TABm, VRF_IDf ))) {
                  rv = _bcm_esw_port_tab_get(unit, port, VRF_IDf, value);
            } else {
                  *value = 0;
            }
        }else if (soc_mem_field_valid(unit, PORT_TABm, PORT_VRFf)) {
            rv = _bcm_esw_port_tab_get(unit, port, PORT_VRFf, value);
        }else if (soc_mem_field_valid(unit, PORT_TABm, VRF_IDf)) {
            rv = _bcm_esw_port_tab_get(unit, port, VRF_IDf, value);
        }

#if defined(BCM_TRIUMPH_SUPPORT)
        else {
            *value = 0;
            if (soc_mem_field_valid(unit, PORT_TABm, PORT_OPERATIONf)) {
                rv = _bcm_esw_port_tab_get(unit, port, PORT_OPERATIONf, value);
                BCM_IF_ERROR_RETURN(rv);
                if ((*value == 0x3) && 
                    (soc_mem_field_valid(unit, SOURCE_TRUNK_MAP_TABLEm, VRF_IDf))) {
                    rv = _bcm_trx_source_trunk_map_get(unit, port, VRF_IDf, (uint32 *)value);
                }
            }
        }     
#endif  /* BCM_TRIUMPH_SUPPORT */
          break;

      case bcmPortControlL3Ingress:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRIUMPH_SUPPORT)
        {
            *value = 0;
            if (soc_mem_field_valid(unit, PORT_TABm, PORT_OPERATIONf)) {
                rv = _bcm_esw_port_tab_get(unit, port, PORT_OPERATIONf, value);
                BCM_IF_ERROR_RETURN(rv);
                if ((*value == 0x2) && 
                    (soc_mem_field_valid(unit, SOURCE_TRUNK_MAP_TABLEm, L3_IIFf)) ) {
                    rv = _bcm_trx_source_trunk_map_get(unit, port, L3_IIFf, (uint32 *)value);
                } 
            } 
        }  
#endif  /* BCM_TRIUMPH_SUPPORT */
        break;

      case bcmPortControlL2Learn:
#if defined(BCM_TRX_SUPPORT)
          if (SOC_IS_TRX(unit)) {
              uint32 hw_val;

              rv = _bcm_esw_port_tab_get(unit, port, CML_FLAGS_NEWf, 
                                         (int *)&hw_val);
              if (BCM_SUCCESS(rv)) {
                  rv = _bcm_trx_port_cml_hw2flags(unit, hw_val, (uint32 *)value);

              }
          }          
#endif /* BCM_TRX_SUPPORT */
          break;

      case bcmPortControlL2Move:
#if defined(BCM_TRX_SUPPORT)
          if (SOC_IS_TRX(unit)) {
              uint32 hw_val;

              rv = _bcm_esw_port_tab_get(unit, port, CML_FLAGS_MOVEf, 
                                         (int *)&hw_val);
              if (BCM_SUCCESS(rv)) {
                  rv = _bcm_trx_port_cml_hw2flags(unit, hw_val, (uint32 *)value);

              }
          }          
#endif /* BCM_TRX_SUPPORT */
          break;

    case bcmPortControlForwardStaticL2MovePkt:
#if defined(BCM_TRX_SUPPORT)
         rv = _bcm_esw_port_tab_get(unit, port, DISABLE_STATIC_MOVE_DROPf, 
                                    value);
#endif /* BCM_TRX_SUPPORT */
         break;

    case bcmPortControlPrbsRxStatus:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        } 

        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, 
                                     SOC_PHY_CONTROL_PRBS_RX_STATUS, 
                                     (uint32 *)value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlLinkFaultLocalEnable:
    case bcmPortControlLinkFaultRemoteEnable:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        {
            uint64  rval;
            soc_reg_t reg;
            soc_field_t field;
            int addr;

            if (soc_feature(unit, soc_feature_xmac)) {
                reg = XMAC_RX_LSS_CTRLr;
                if (type == bcmPortControlLinkFaultLocalEnable) {
                    field = LOCAL_FAULT_DISABLEf;
                } else {
                    field = REMOTE_FAULT_DISABLEf;
                }
            } else {
                reg = MAC_RXLSSCTRLr;
                if (type == bcmPortControlLinkFaultLocalEnable) {
                    field = LOCALFAULTDISABLEf;
                } else {
                    field = REMOTEFAULTDISABLEf;
                }
            }

            if (!soc_reg_field_valid(unit, reg, field)) {
                return BCM_E_UNAVAIL;
            }
            addr = soc_reg_addr(unit, reg, port, 0);
            BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval));
            *value = soc_reg64_field32_get(unit, reg, rval, field) ? 0 : 1;
        }
        break;
    case bcmPortControlLinkFaultLocal:
    case bcmPortControlLinkFaultRemote:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        {
            uint64  rval;
            soc_reg_t reg;
            soc_field_t field;
            int addr;

            if (soc_feature(unit, soc_feature_xmac)) {
                reg = XMAC_RX_LSS_STATUSr;
                if (type == bcmPortControlLinkFaultLocal) {
                    field = LOCAL_FAULT_STATUSf;
                } else {
                    field = REMOTE_FAULT_STATUSf;
                }
            } else {
                if (SOC_IS_LYNX(unit)) {
                    reg = MAC_RXLSSCTRLr;
                } else {
                    reg = MAC_RXLSSSTATr;
                }
                if (type == bcmPortControlLinkFaultLocal) {
                    field = LOCALFAULTSTATf;
                } else {
                    field = REMOTEFAULTSTATf;
                }
            }

            if (!soc_reg_field_valid(unit, reg, field)) {
                return BCM_E_UNAVAIL;
            }
            addr = soc_reg_addr(unit, reg, port, 0);
            BCM_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval));
            *value = soc_reg64_field32_get(unit, reg, rval, field) ? 1 : 0;
        }
        break;
    case bcmPortControlTimestampTransmit:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT)
        {
            if (!soc_feature(unit, soc_feature_timesync_support)) {
                return BCM_E_UNAVAIL;
            }
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                                 SOC_MAC_CONTROL_TIMESTAMP_TRANSMIT, value);
        }
#endif  /* BCM_TRIUMPH2_SUPPORT || BCM_HAWKEYE_SUPPORT */ 
        break;
    case bcmPortControlTimestampEnable: 
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
#if defined(BCM_HAWKEYE_SUPPORT)
        if (SOC_IS_HAWKEYE(unit)) {
            uint32 rval;

            if (!SOC_PORT_VALID(unit, port)) {
                return BCM_E_PORT;
            }
            SOC_IF_ERROR_RETURN(READ_EAV_ENABLE_BMAPr(unit, &rval));
            *value = (rval & (1 << port)) ? TRUE:FALSE;
            rv = BCM_E_NONE;
        }
#endif /* BCM_HAWKEYE_SUPPORT */
        break;
    case bcmPortControlSerdesDriverEqualizationTuneStatusFarEnd:
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port,
             SOC_PHY_CONTROL_SERDES_DRIVER_EQUALIZATION_TUNE_STATUS_FAR_END,
                                     (uint32 *)value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlLinkdownTransmit:
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port,
                                     SOC_PHY_CONTROL_LINKDOWN_TRANSMIT,
                                     (uint32 *)value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlSerdesTuneMarginMode:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port,
                                     SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MODE, 
                                     (uint32 *)value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlSerdesTuneMarginValue:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port,
                                     SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_VALUE, 
                                     (uint32 *)value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlSerdesTuneMarginMax:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port,
                                     SOC_PHY_CONTROL_SERDES_TUNE_MARGIN_MAX, 
                                     (uint32 *)value);
        PORT_UNLOCK(unit);
        break;
    case bcmPortControlStatOversize:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        rv = _bcm_esw_stat_ovr_threshold_get(unit, port, value);
        break;

    case bcmPortControlEEEEnable:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;                
        }
        
        PORT_LOCK(unit);

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        if (soc_feature(unit, soc_feature_eee)) {
            int mac_val = 0;
            uint32 phy_val;

            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                     SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                     BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {
                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native mode, then get Native EEE
                 * status from MAC.
                 */
                /* EEE standard compliance Work Around:
                 * Return the software copy of eee status from eee_cfg
                 * instead of getting it from MAC
                 */
                *value = eee_cfg[unit][port];
                /**value = mac_val;*/
                rv = BCM_E_NONE;

            } else {
                /* If native EEE mode is not supported, get AutoGrEEEn status
                 * from PHY. If PHY does not support AutoGrEEEn mode,
                 * rv will be assigned BCM_E_UNAVAIL.
                 */
               rv = soc_phyctrl_control_get(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE_AUTO, &phy_val);
               if (BCM_SUCCESS(rv)) {
                   *value = phy_val;
               }
            }
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */

        {
            uint32 eeenative, eeeauto;
            int rv1 = BCM_E_UNAVAIL, rv2 = BCM_E_UNAVAIL;

            rv = BCM_E_NONE;

            if ((rv1 = soc_phyctrl_control_get(unit, port,
                            BCM_PORT_PHY_CONTROL_EEE, &eeenative)) != BCM_E_UNAVAIL) {
                *value = eeenative;
            }

            if ((rv2 = soc_phyctrl_control_get(unit, port,
                            BCM_PORT_PHY_CONTROL_EEE_AUTO, &eeeauto)) != BCM_E_UNAVAIL) {
                *value = eeeauto;
            }

            /* If neither Native EEE nor AutoGrEEEn mode is supported */                
            if ((rv1 == BCM_E_UNAVAIL) && (rv2 == BCM_E_UNAVAIL)) {
                rv = BCM_E_UNAVAIL;
            }

            /* If both Native EEE and AutoGrEEEn mode are supported */
            if ((rv1 != BCM_E_UNAVAIL) && (rv2 != BCM_E_UNAVAIL)) {
                *value = eeenative ? (eeeauto ? 0 : 1) : (eeeauto ? 1 : 0);
            }
        }

        PORT_UNLOCK (unit);

        break;

 /**********************      EEE Mode Overview    ************************
 *                      |DET|                              |  WT |
 *   Signalling   |idles|   |------------------------------|     | idles   |
 *   from Tx MAC  | or  |   |   Low Power Idle (LPI)       |idles|  or     |
 *   to local PHY |data |   |------------------------------|     | data    |
 *                          *                              * 
 *                          *                              *
 *                          *  -------LPI state------------*
 *   Local PHY    |         |  |      |  |       |  |      |   |           |
 *   signaling    |   Active|Ts|  Tq  |Tr|  Tq   |Tr|  Tq  |Tw |Active     |
 *   on MDI       |         |  |      |  |       |  |      |   |           |
 *                          *------------------------------*
 *                          *                              *
 *                          *                               *
 *   Signaling    |   idles |-------------------------------|id| PHY is    |  
 *   from LP PHY  |     or  |  Low Power Idle (LPI)         |le| ready     |
 *   to Rx MAC    |   data  |-------------------------------|s | for data  |
 *                                                          
 *   where DET = Delay Entry Timer    WT = MAC Wake Timer
 */
    case bcmPortControlEEETransmitIdleTime:
        /* DET = Time (in microsecs) for which condition to move to LPI state 
         * is satisfied, at the end of which MAC TX transitions to LPI state */
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        if (soc_feature (unit, soc_feature_eee)) {
            rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                    SOC_MAC_CONTROL_EEE_TX_IDLE_TIME, value);
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */
        {
            rv = BCM_E_UNAVAIL;
        }
        PORT_UNLOCK(unit);
        break;
        
    case bcmPortControlEEETransmitEventCount:
        /* Number of time MAC TX enters LPI state for 
         * a given measurement interval*/
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;                
        }

        PORT_LOCK(unit);

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        if (soc_feature(unit, soc_feature_eee)) {
            int mac_val;
            uint32 phy_val;
            uint64 rval64;

            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                     SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                     BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {
                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native mode
                 */

                if (SOC_IS_KATANA(unit)) {
                    /* On Katana TX/RX register are swapped */
                    rv = READ_RX_EEE_LPI_EVENT_COUNTERr(unit, port, &rval64);
                } else {
                    rv = READ_TX_EEE_LPI_EVENT_COUNTERr(unit, port, &rval64);
                }
                if (BCM_SUCCESS(rv)) {
                    *value = COMPILER_64_LO(rval64);
                }
            } else {
                /* Get counter value from PHY. If PHY does not support
                 * EEE counters, rv will be assigned BCM_E_UNAVAIL.
                 */
                rv = soc_phyctrl_control_get(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_EVENTS, &phy_val);
                if (BCM_SUCCESS(rv)) {
                    *value = phy_val;
                }
            } 
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */

        {
            /* On legacy devices that don't support native EEE,
             * get counter value from PHY. If PHY does not support
             * EEE counters, rv will be assigned BCM_E_UNAVAIL.
             */
            uint32 phy_val;
            rv = soc_phyctrl_control_get(unit, port,
                    BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_EVENTS, &phy_val);
            if (BCM_SUCCESS(rv)) {
                *value = phy_val;
            }
        }

        PORT_UNLOCK (unit);

        break;
        
    case bcmPortControlEEETransmitDuration:
        /* Time in (microsecs) for which MAC TX enters LPI state 
         * during a measurement interval*/
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;                
        }

        PORT_LOCK(unit);

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        if (soc_feature(unit, soc_feature_eee)) {
            int mac_val;
            uint32 phy_val;
            uint64 rval64;

            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                     SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                     BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {
                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native mode
                 */
                if (SOC_IS_KATANA(unit)) {
                    /* On Katana TX/RX register are swapped */
                    rv = READ_RX_EEE_LPI_DURATION_COUNTERr(unit, port, &rval64);
                } else {
                    rv = READ_TX_EEE_LPI_DURATION_COUNTERr(unit, port, &rval64);
                }
                if (BCM_SUCCESS(rv)) {
                    *value = COMPILER_64_LO(rval64);
                }
            } else {
                /* Get counter value from PHY. If PHY does not support
                 * EEE counters, rv will be assigned BCM_E_UNAVAIL.
                 */
                rv = soc_phyctrl_control_get(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_DURATION, &phy_val);
                if (BCM_SUCCESS(rv)) {
                    *value = phy_val;
                }
            }
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */
        {
            /* On legacy devices that don't support native EEE,
             * get counter value from PHY. If PHY does not support
             * EEE counters, rv will be assigned BCM_E_UNAVAIL.
             */
            uint32 phy_val;
            rv = soc_phyctrl_control_get(unit, port,
                    BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_DURATION, &phy_val);
            if (BCM_SUCCESS(rv)) {
                *value = phy_val;
            }
        }

        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEEReceiveEventCount:
        /* Number of time MAC RX enters LPI state for
         * a given measurement interval */
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;                
        }

        PORT_LOCK(unit);

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
        if (soc_feature(unit, soc_feature_eee)) {
            int mac_val;
            uint32 phy_val;
            uint64 rval64;

            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                     SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                     BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {
                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native mode
                 */
                if (SOC_IS_KATANA(unit)) {
                    /* On Katana TX/RX register are swapped */
                    rv = READ_TX_EEE_LPI_EVENT_COUNTERr(unit, port, &rval64);
                } else {
                    rv = READ_RX_EEE_LPI_EVENT_COUNTERr(unit, port, &rval64);
                }
                if (BCM_SUCCESS(rv)) {
                    *value = COMPILER_64_LO(rval64);
                }
            } else {
                /* Get counter value from PHY. If PHY does not support
                 * EEE counters, rv will be assigned BCM_E_UNAVAIL.
                 */
                rv = soc_phyctrl_control_get(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE_RECEIVE_EVENTS, &phy_val);
                if (BCM_SUCCESS(rv)) {
                    *value = phy_val;
                }
            }
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT || BCM_SHADOW_SUPPORT */

        {
            /* On legacy devices that don't support native EEE,
             * get counter value from PHY. If PHY does not support
             * EEE counters, rv will be assigned BCM_E_UNAVAIL.
             */
            uint32 phy_val;
            rv = soc_phyctrl_control_get(unit, port,
                    BCM_PORT_PHY_CONTROL_EEE_RECEIVE_EVENTS, &phy_val);
            if (BCM_SUCCESS(rv)) {
                *value = phy_val;
            }
        }

        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEEReceiveDuration:
        /* Time in (microsecs) for which MAC RX enters LPI state
         * during a measurement interval*/
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;                
        }

        PORT_LOCK(unit);

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_eee)) {
            int mac_val;
            uint32 phy_val;
            uint64 rval64;

            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                     SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                     BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {
                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native mode
                 */
                if (SOC_IS_KATANA(unit)) {
                    /* On Katana TX/RX register are swapped */
                    rv = READ_TX_EEE_LPI_DURATION_COUNTERr(unit, port, &rval64);
                } else {
                    rv = READ_RX_EEE_LPI_DURATION_COUNTERr(unit, port, &rval64);
                }
                if (BCM_SUCCESS(rv)) {
                    *value = COMPILER_64_LO(rval64);
                }
            } else {
                /* Get counter value from PHY. If PHY does not support
                 * EEE counters, rv will be assigned BCM_E_UNAVAIL.
                 */
                rv = soc_phyctrl_control_get(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE_RECEIVE_DURATION, &phy_val);
                if (BCM_SUCCESS(rv)) {
                    *value = phy_val;
                }
            } 
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT */
        {
            /* On legacy devices that don't support native EEE,
             * get counter value from PHY. If PHY does not support
             * EEE counters, rv will be assigned BCM_E_UNAVAIL.
             */
            uint32 phy_val;
            rv = soc_phyctrl_control_get(unit, port,
                    BCM_PORT_PHY_CONTROL_EEE_RECEIVE_DURATION, &phy_val);
            if (BCM_SUCCESS(rv)) {
                *value = phy_val;
            }
        }

        PORT_UNLOCK (unit);
        break;

    case bcmPortControlEEETransmitRefreshTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_REFRESH_TIME, (uint32 *)value);
        PORT_UNLOCK (unit);            
        break; 

    case bcmPortControlEEETransmitSleepTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_SLEEP_TIME, (uint32 *)value);
        PORT_UNLOCK (unit);            
        break; 

    case bcmPortControlEEETransmitQuietTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_QUIET_TIME, (uint32 *)value);
        PORT_UNLOCK (unit);            
        break; 

    case bcmPortControlEEETransmitWakeTime:
        /* Time(in microsecs) to wait before transmitter can leave LPI State*/
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;                
        }

        PORT_LOCK(unit);

#if defined(BCM_HURRICANE_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT) \
        || defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_eee)) {
            int mac_val;
            uint32 phy_val;

            if ((MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                     SOC_MAC_CONTROL_EEE_ENABLE, &mac_val) != BCM_E_UNAVAIL) &&
                (soc_phyctrl_control_get(unit, port,
                     BCM_PORT_PHY_CONTROL_EEE, &phy_val) != BCM_E_UNAVAIL)) {
                /* If MAC/Switch is EEE aware (Native EEE mode is supported)
                 * and PHY also supports Native mode
                 */
                rv = MAC_CONTROL_GET(PORT(unit, port).p_mac, unit, port,
                        SOC_MAC_CONTROL_EEE_TX_WAKE_TIME, value);

            } else {
                /* Get timer value from PHY. If PHY does not support
                 * EEE timers, rv will be assigned BCM_E_UNAVAIL.
                 */
                rv = soc_phyctrl_control_get(unit, port,
                        BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_WAKE_TIME, &phy_val);
                if (BCM_SUCCESS(rv)) {
                    *value = phy_val;
                }
            }
        } else
#endif /* BCM_HURRICANE_SUPPORT || BCM_HAWKEYE_SUPPORT || BCM_TRIDENT_SUPPORT */
        {
            /* On legacy devices that don't support native EEE,
             * get timer value from PHY. If PHY does not support
             * EEE timers, rv will be assigned BCM_E_UNAVAIL.
             */
            uint32 phy_val;
            rv = soc_phyctrl_control_get(unit, port,
                    BCM_PORT_PHY_CONTROL_EEE_TRANSMIT_WAKE_TIME, &phy_val);
            if (BCM_SUCCESS(rv)) {
                *value = phy_val;
            }
        }

        PORT_UNLOCK (unit);
        break;

     case bcmPortControlEEEReceiveSleepTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_RECEIVE_SLEEP_TIME, (uint32 *)value);
        PORT_UNLOCK (unit);            
        break; 

    case bcmPortControlEEEReceiveQuietTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_RECEIVE_QUIET_TIME, (uint32 *)value);
        PORT_UNLOCK (unit);            
        break; 

    case bcmPortControlEEEReceiveWakeTime:
        if (BCM_GPORT_IS_WLAN_PORT(port)) {
            return BCM_E_UNAVAIL;
        }
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (!IS_E_PORT(unit, port)) {
            return BCM_E_UNAVAIL;
        }
        PORT_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, 
                BCM_PORT_PHY_CONTROL_EEE_RECEIVE_WAKE_TIME, (uint32 *)value);
        PORT_UNLOCK (unit);            
        break; 

    case bcmPortControlTrill:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, TRILL_ENABLEf)) {
                   rv = _bcm_esw_port_tab_get(unit, port, TRILL_ENABLEf, value);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;
		 
    case bcmPortControlTrillAllow:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, ALLOW_TRILL_FRAMESf)) {
                   rv = _bcm_esw_port_tab_get(unit, port, ALLOW_TRILL_FRAMESf, value);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNonTrillAllow:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, ALLOW_NON_TRILL_FRAMESf)) {
                   rv = _bcm_esw_port_tab_get(unit, port, ALLOW_NON_TRILL_FRAMESf, value);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlTrillCoreISISToCPU:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, COPY_CORE_IS_IS_TO_CPUf)) {
                   rv = _bcm_esw_port_tab_get(unit, port, COPY_CORE_IS_IS_TO_CPUf, value);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlTrillHashSelect:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_trill)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, RTAG7_HASH_CFG_SEL_TRILL_ECMPf)) {
                   rv = _bcm_esw_port_tab_get(unit, port, RTAG7_HASH_CFG_SEL_TRILL_ECMPf, value);
              }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivAccessVirtualInterfaceId:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_get(unit, port, NIV_VIF_IDf, value);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivNonVntagDrop:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_get(unit, port,
                     DISCARD_IF_VNTAG_NOT_PRESENTf, value);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagDrop:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_get(unit, port,
                     DISCARD_IF_VNTAG_PRESENTf, value);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivNonVntagAdd:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             int vntag_action;
             rv = _bcm_esw_port_tab_get(unit, port, 
                     VNTAG_ACTIONS_IF_NOT_PRESENTf, &vntag_action);
             if (BCM_SUCCESS(rv)) {
                 *value = (vntag_action == VNTAG_ADD) ? TRUE : FALSE;
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagIngressReplace:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             int vntag_action;
             rv = _bcm_esw_port_tab_get(unit, port,
                     VNTAG_ACTIONS_IF_PRESENTf, &vntag_action);
             if (BCM_SUCCESS(rv)) {
                 *value = (vntag_action == VNTAG_REPLACE) ? TRUE : FALSE;
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagEgressReplace:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             egr_port_entry_t egr_port_entry;
             int vntag_action;
             rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);
             if (BCM_SUCCESS(rv)) {
                 vntag_action = soc_mem_field32_get(unit, EGR_PORTm,
                         &egr_port_entry, VNTAG_ACTIONS_IF_PRESENTf);
                 *value = (vntag_action == VNTAG_REPLACE) ? TRUE : FALSE;
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagIngressDelete:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             int vntag_action;
             rv = _bcm_esw_port_tab_get(unit, port,
                     VNTAG_ACTIONS_IF_PRESENTf, &vntag_action);
             if (BCM_SUCCESS(rv)) {
                 *value = (vntag_action == VNTAG_DELETE) ? TRUE : FALSE;
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivVntagEgressDelete:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             egr_port_entry_t egr_port_entry;
             int vntag_action;
             rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);
             if (BCM_SUCCESS(rv)) {
                 vntag_action = soc_mem_field32_get(unit, EGR_PORTm,
                         &egr_port_entry, VNTAG_ACTIONS_IF_PRESENTf);
                 *value = (vntag_action == VNTAG_DELETE) ? TRUE : FALSE;
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivRpfCheck:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_get(unit, port, 
                     NIV_RPF_CHECK_ENABLEf, value);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivSourceKnockout:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             egr_port_entry_t egr_port_entry;
             rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &egr_port_entry);
             if (BCM_SUCCESS(rv)) {
                 *value = soc_mem_field32_get(unit, EGR_PORTm,
                         &egr_port_entry, NIV_PRUNE_ENABLEf);
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivForwardPort:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             int is_niv_uplink;
             int tx_dest_port;
             _bcm_gport_dest_t dest;

             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_get(unit, port, 
                         NIV_UPLINK_PORTf, &is_niv_uplink));
             if (is_niv_uplink) {
                 *value = -1;
             } else {
                 BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_get(unit, port, 
                             TX_DEST_PORTf, &tx_dest_port));
                 if ((tx_dest_port >> SOC_TRUNK_BIT_POS(unit)) & 0x1) {
                     dest.tgid = tx_dest_port - (1 << SOC_TRUNK_BIT_POS(unit));
                     dest.gport_type = _SHR_GPORT_TYPE_TRUNK;
                 } else {
                     dest.port = tx_dest_port & SOC_PORT_ADDR_MAX(unit); 
                     dest.modid = (tx_dest_port - dest.port) /
                         (SOC_PORT_ADDR_MAX(unit) + 1); 
                     dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
                 }
                 BCM_IF_ERROR_RETURN
                     (_bcm_esw_gport_construct(unit, &dest, value));
             }
             rv = BCM_E_NONE;
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivType:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             int vt_key_type, vt_enable, uplink, rpf_enable, prune_enable;
             egr_port_entry_t egr_port_entry;
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_get(unit,
                         port, VT_KEY_TYPEf, &vt_key_type));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_get(unit,
                         port, VT_ENABLEf, &vt_enable));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_get(unit,
                         port, NIV_UPLINK_PORTf, &uplink));
             BCM_IF_ERROR_RETURN(_bcm_esw_port_tab_get(unit,
                         port, NIV_RPF_CHECK_ENABLEf, &rpf_enable));
             BCM_IF_ERROR_RETURN(READ_EGR_PORTm(unit, MEM_BLOCK_ANY,
                         port, &egr_port_entry));
             prune_enable = soc_mem_field32_get(unit, EGR_PORTm,
                         &egr_port_entry, NIV_PRUNE_ENABLEf);
             rv = BCM_E_NONE;
             if (vt_enable && (vt_key_type == TR_VLXLT_HASH_KEY_TYPE_VIF)) {
                 *value = BCM_PORT_NIV_TYPE_SWITCH;
             } else if (uplink) {
                 *value = BCM_PORT_NIV_TYPE_UPLINK_ACCESS;
             } else if (rpf_enable) {
                 *value = BCM_PORT_NIV_TYPE_DOWNLINK_TRANSIT;
             } else if (prune_enable) {
                 *value = BCM_PORT_NIV_TYPE_DOWNLINK_ACCESS;
             } else {
                 rv = BCM_E_PORT;
             }
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;

    case bcmPortControlNivNameSpace:
#if defined(BCM_TRIDENT_SUPPORT)
         if (soc_feature(unit, soc_feature_niv)) {
             rv = _bcm_esw_port_tab_get(unit, port, NIV_NAMESPACEf, value);
         }
#endif /* BCM_TRIDENT_SUPPORT */
         break;
    case bcmPortControlFabricSourceKnockout:
        if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, REMOVE_HG_HDR_SRC_PORTf)) {
            rv = _bcm_esw_port_tab_get(unit, port, REMOVE_HG_HDR_SRC_PORTf, 
                                       value);
        }
        break;
    case bcmPortControlRxEnable:
        rv = _bcm_esw_mac_rx_control(unit, port, 1, value);
        break;
    case bcmPortControlTxEnable:
        if (SOC_REG_IS_VALID(unit, COSMASKr) || 
            (SOC_REG_IS_VALID(unit, EGRMETERINGCONFIGr) &&
            SOC_REG_IS_VALID(unit, EGRMETERINGBUCKETr))) {
            *value = 1;
            if (PORT(unit, port).flags & _PORT_INFO_STOP_TX) {
                *value = 0;
            }
            rv = SOC_E_NONE;
        }
        break;

    case bcmPortControlMplsEntropyHashSet:
#if defined(BCM_KATANA_SUPPORT)
         if (soc_feature(unit, soc_feature_mpls)) {
              if (SOC_MEM_FIELD_VALID(unit, PORT_TABm, RTAG7_HASH_CFG_SEL_ENTROPY_LABELf)) {
                   rv = _bcm_esw_port_tab_get(unit, port, RTAG7_HASH_CFG_SEL_ENTROPY_LABELf, value);
              }
         }
#endif /* BCM_KATANA_SUPPORT */
         break;

    default:
        break;
    }

    SOC_DEBUG_PRINT((DK_PORT,
                   "bcm_port_control_get: u=%d p=%d v=%d\n",
                    unit, port, *value));
    return rv;
}


/*
 * Function:
 *      _bcm_esw_port_policer_get
 * Purpose:
 *       Internal function to Retrieve the Policer ID accociated 
 *       with specified physical port. 
 * Parameters:
 *     Unit                  - (IN) unit number 
 *     port                  - (IN) Port Number 
 *     policer_id            - (OUT) policer Id 
 * Returns:
 *     BCM_E_XXX 
 */
int _bcm_esw_port_policer_get(
    int unit,
    bcm_port_t port,
    bcm_policer_t *policer_id)
{
    int rv = BCM_E_NONE;
#ifdef BCM_KATANA_SUPPORT
    int index = 0;
    int offset_mode = 0;

    BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_get(unit, port,
                                      _bcmPortSvcMeterIndex, &index));
    BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_get(unit, port,
                                    _bcmPortSvcMeterOffsetMode, &offset_mode));
    _bcm_esw_get_policer_id_from_index_offset(unit, index, 
                                                    offset_mode, policer_id);
#endif
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_policer_get
 * Purpose:
 *       Retrieve the Policer ID accociated for the specified physical port. 
 * Parameters:
 *     Unit                  - (IN) unit number 
 *     port                  - (IN) Port Number 
 *     policer_id            - (OUT) policer Id 
 * Returns:
 *     BCM_E_XXX 
 */
int bcm_esw_port_policer_get(
    int unit,
    bcm_port_t port,
    bcm_policer_t *policer_id)
{
#ifdef BCM_KATANA_SUPPORT
    int rv = BCM_E_NONE;
    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    BCM_IF_ERROR_RETURN(_check_global_meter_init(unit));
    rv = _bcm_esw_port_gport_validate(unit, port, &port);
    BCM_IF_ERROR_RETURN(rv);
    rv = _bcm_esw_port_policer_get(unit, port, policer_id); 
    return rv;
#else 
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_port_policer_set
 * Purpose:
 *      Set the Policer ID accociated for the specified physical port. 
 * Parameters:
 *     Unit                  - (IN) unit number 
 *     port                  - (IN) Port Number 
 *     policer_id            - (IN) policer Id 
 * Returns:
 *     BCM_E_XXX 
 */
int bcm_esw_port_policer_set(
    int unit,
    bcm_port_t port,
    bcm_policer_t policer)
{
#ifdef BCM_KATANA_SUPPORT
    int rv = BCM_E_NONE;
    int index=0;
    int offset_mode=0;
    bcm_policer_t current_policer = 0;
    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    BCM_IF_ERROR_RETURN(_check_global_meter_init(unit));
    rv = _bcm_esw_port_gport_validate(unit, port, &port);
    BCM_IF_ERROR_RETURN(rv);
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer);
    if (BCM_FAILURE(rv)) {
        SOC_DEBUG_PRINT((DK_VERBOSE, "%s : Invalid policer id passed: %x \n", \
                                      FUNCTION_NAME(), policer));
        return (rv);
    }
    /* get the policer id that is currently configured */
    rv = _bcm_esw_port_policer_get(unit, port, &current_policer); 
    if (BCM_FAILURE(rv)) {
        SOC_DEBUG_PRINT((DK_VERBOSE, "%s : Unabel to get the current policer \
                                   configured \n", FUNCTION_NAME()));
        return (rv);
    }
    rv = BCM_E_UNAVAIL;
    /* Set policer id */
    _bcm_esw_get_policer_table_index(unit, policer, &index); 
    offset_mode = (((policer) & BCM_POLICER_GLOBAL_METER_MODE_MASK) >> 
                                    BCM_POLICER_GLOBAL_METER_MODE_SHIFT);
    if (offset_mode >= 1) {
        offset_mode = offset_mode - 1;
    } 
    if (index >= soc_mem_index_count(unit,SVM_METER_TABLEm))  {
        SOC_DEBUG_PRINT((DK_VERBOSE, "%s : Invalid table index\n", \
                                                    FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    if (offset_mode >= (soc_mem_index_count(unit,SVM_OFFSET_TABLEm) >>
                               BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE))  {
        SOC_DEBUG_PRINT((DK_VERBOSE, "%s : Invalid offset mode\n", \
                                                         FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(
            _bcm_esw_port_config_set(unit, port,
                                      _bcmPortSvcMeterIndex, index));
    BCM_IF_ERROR_RETURN(
             _bcm_esw_port_config_set(unit, port,
                                     _bcmPortSvcMeterOffsetMode, offset_mode));
    /* decrement current policer if any */
    if ((current_policer & BCM_POLICER_GLOBAL_METER_INDEX_MASK) > 0) {
        rv = _bcm_esw_policer_decrement_ref_count(unit, current_policer);
        BCM_IF_ERROR_RETURN(rv);
    }
    /* increment policer reference count */
    if ((policer & BCM_POLICER_GLOBAL_METER_INDEX_MASK) > 0 ) {
        rv = _bcm_esw_policer_increment_ref_count(unit, policer);
        BCM_IF_ERROR_RETURN(rv);
    }
    return rv;
#else 
    return BCM_E_UNAVAIL;
#endif
} 

/* 
 * Function:
 *      bcm_port_vlan_inner_tag_set
 * Purpose:
 *      Set the inner tag to be added to the packet.
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      inner_tag  - (IN) Inner tag. 
 *                    Priority[15:13] CFI[12] VLAN ID [11:0]
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_port_vlan_inner_tag_set(int unit, bcm_port_t port, uint16 inner_tag)
{
    int rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = BCM_E_UNAVAIL;

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        bcm_vlan_action_set_t action;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        PORT_LOCK(unit);
        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_default_action_get(unit, port, &action));
        action.new_inner_vlan = BCM_VLAN_CTRL_ID(inner_tag);
        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_default_action_set(unit, port, &action));
        PORT_UNLOCK(unit); 
        return BCM_E_NONE;
    }
#endif

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT) 
    if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit) || SOC_IS_HAWKEYE(unit)) {
        PORT_LOCK(unit);

        rv = soc_reg_field32_modify(unit, EGR_SRC_PORTr, port, 
                                    INNER_TAGf, inner_tag);
        PORT_UNLOCK(unit); 
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT */

    return rv;
}

/* 
 * Function:
 *      bcm_port_vlan_inner_tag_get
 * Purpose:
 *      Get the inner tag to be added to the packet.
 * Parameters:
 *      unit       - (IN) BCM unit.
 *      port       - (IN) Port number.
 *      inner_tag  - (OUT) Inner tag. 
 *                    Priority[15:13] CFI[12] VLAN ID [11:0]
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_esw_port_vlan_inner_tag_get(int unit, bcm_port_t port, uint16 *inner_tag)
{
    int rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    rv = BCM_E_UNAVAIL;

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        bcm_vlan_action_set_t action;

        if (!soc_feature(unit, soc_feature_vlan_action)) {
            return BCM_E_UNAVAIL;
        }

        PORT_LOCK(unit);
        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_default_action_get(unit, port, &action));
        *inner_tag = action.new_inner_vlan;
        PORT_UNLOCK(unit);
        return BCM_E_NONE;
    }
#endif

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_RAVEN_SUPPORT)
        if (SOC_IS_FIREBOLT2(unit) || SOC_IS_RAVEN(unit) || SOC_IS_HAWKEYE(unit)) {
        uint32 value;

        rv = READ_EGR_SRC_PORTr(unit, port, &value);
        *inner_tag = soc_reg_field_get(unit, EGR_SRC_PORTr, value,
                                       INNER_TAGf);
    } 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_RAVEN_SUPPORT */
    return rv;
}

/* 
 * Function    : 
 *     bcm_port_class_set
 * Description : 
 *     Set the ports class ID. Ports with the
 *     same class ID can be treated as a group in
 *     field processing and VLAN translation.
 * Parameters  : 
 *     (IN) unit      - BCM device number
 *     (IN) port      - Device or logical port number
 *     (IN) pclass    - Classification type 
 *     (IN) pclass_id - New class ID of the port.
 * Returns     : 
 *     BCM_E_NONE     - Success
 *     BCM_E_XXX      - Failed
 */
int 
bcm_esw_port_class_set(int unit, bcm_port_t port, 
                       bcm_port_class_t pclass, uint32 pclass_id)
{
    int        rv = BCM_E_UNAVAIL;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        int        port_class_id;
        soc_reg_t egr_port_reg;
        egr_port_reg = (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                        SOC_IS_VALKYRIE2(unit)) ? EGR_PORT_64r : EGR_PORTr;
#endif /* BCM_FIREBOLT_SUPPORT || BCM_TRX_SUPPORT */

        switch (pclass) {
        case bcmPortClassFieldLookup:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
            if (soc_mem_field_valid(unit, PORT_TABm, VFP_PORT_GROUP_IDf)) {
                rv = soc_mem_field_pbmp_fit(unit, PORT_TABm, VFP_PORT_GROUP_IDf,
                                           &pclass_id);
                if (rv == BCM_E_NONE) {
                    port_class_id = (int) pclass_id;
                    rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                               VFP_PORT_GROUP_IDf, 
                                               port_class_id);
                }
            }
#endif /* BCM_FIREBOLT_SUPPORT || BCM_TRX_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
            if ((SOC_MEM_FIELD_VALID(unit, SOURCE_TRUNK_MAP_TABLEm, 
                                     VFP_PORT_GROUP_IDf)) &&
                (BCM_SUCCESS(rv))) {
                rv = _bcm_trx_source_trunk_map_set(unit, port,
                                                   VFP_PORT_GROUP_IDf, 
                                                   pclass_id);
            }
#endif /* BCM_TRX_SUPPORT */
            break;

        case bcmPortClassFieldIngress:
#if defined(BCM_TRX_SUPPORT)
            if (SOC_MEM_FIELD_VALID(unit, SOURCE_TRUNK_MAP_TABLEm, CLASS_IDf)) {
                rv = _bcm_trx_source_trunk_map_set(unit, port, CLASS_IDf, pclass_id);
            }
#endif /* BCM_TRX_SUPPORT */
            break;
        case bcmPortClassFieldEgress:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
            if (soc_reg_field_valid(unit, egr_port_reg, EGR_PORT_GROUP_IDf)) {
                rv = soc_reg_field32_modify(unit, egr_port_reg, port,
                                            EGR_PORT_GROUP_IDf, pclass_id);
            }
#ifdef BCM_TRIDENT_SUPPORT
            else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm,
                                         EGR_PORT_GROUP_IDf)) {
                rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                                            EGR_PORT_GROUP_IDf, pclass_id);
            }
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_FIREBOLT_SUPPORT || BCM_TRX_SUPPORT */
            break;
        case bcmPortClassVlanTranslateEgress:
#if defined(BCM_TRX_SUPPORT)
            if (soc_reg_field_valid(unit, egr_port_reg, VT_PORT_GROUP_IDf)) {
                soc_control_t   *sc = SOC_CONTROL(unit);

                if (sc->soc_flags & SOC_F_PORT_CLASS_BLOCKED) {
                    return BCM_E_CONFIG;
                }
                rv = soc_reg_field32_modify(unit, egr_port_reg, port,
                                            VT_PORT_GROUP_IDf, pclass_id);
                if (BCM_SUCCESS(rv)) {
                    /* Once port class is changed system will not allow */
                    /* To use bcm_vlan_translate_egress */
                    sc->soc_flags |= SOC_F_XLATE_EGR_BLOCKED;
                }
            }
#ifdef BCM_TRIDENT_SUPPORT
            else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm, VT_PORT_GROUP_IDf)) {
                rv = soc_mem_field32_modify(unit, EGR_PORTm, port,
                                            VT_PORT_GROUP_IDf, pclass_id);
            }
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_TRX_SUPPORT */
            break;
        default:
            rv = BCM_E_PARAM;
        }
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return rv;
}
  
/* 
 * Function    : 
 *     bcm_port_class_get
 * Description : 
 *     Get the ports class ID. Ports with the
 *     same class ID can be treated as a group in
 *     field processing and VLAN translation.
 * Parameters  : 
 *     (IN) unit       - BCM device number
 *     (IN) port       - Device or logical port number
 *     (IN) pclass     - Classification type 
 *     (OUT) pclass_id - New class ID of the port.
 * Returns     : 
 *     BCM_E_NONE      - Success
 *     BCM_E_XXX       - Failed
 */
int 
bcm_esw_port_class_get(int unit, bcm_port_t port, 
                       bcm_port_class_t pclass, uint32 *pclass_id)
{
    int        rv = BCM_E_UNAVAIL;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (NULL == pclass_id) {
        return BCM_E_PARAM;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        uint32 reg_val;
        uint64 reg_val64;
        int    port_class_id;
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        switch (pclass) {
        case bcmPortClassFieldLookup:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
            if (soc_mem_field_valid(unit, PORT_TABm, VFP_PORT_GROUP_IDf)) {
                rv = _bcm_esw_port_tab_get(unit, port, VFP_PORT_GROUP_IDf, 
                                           &port_class_id);
                *pclass_id = (uint32) port_class_id; 
            }
#endif /* BCM_FIREBOLT_SUPPORT || BCM_TRX_SUPPORT */
            break;
        case bcmPortClassFieldIngress:
#if defined(BCM_TRX_SUPPORT)
            if (SOC_MEM_FIELD_VALID(unit, SOURCE_TRUNK_MAP_TABLEm, CLASS_IDf)) {
                rv = _bcm_trx_source_trunk_map_get(unit, port, CLASS_IDf, pclass_id);
            }
#endif /* BCM_TRX_SUPPORT */
            break;
        case bcmPortClassFieldEgress:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
            if (soc_reg_field_valid(unit, EGR_PORT_64r, EGR_PORT_GROUP_IDf)) {
                rv = READ_EGR_PORT_64r(unit, port, &reg_val64); 
                if (BCM_SUCCESS(rv)) {
                    *pclass_id = soc_reg64_field32_get(unit, EGR_PORT_64r, 
                                                   reg_val64, EGR_PORT_GROUP_IDf);
                }
            } else if (soc_reg_field_valid(unit, EGR_PORTr, EGR_PORT_GROUP_IDf)) {
                rv = READ_EGR_PORTr(unit, port, &reg_val); 
                if (BCM_SUCCESS(rv)) {
                    *pclass_id = soc_reg_field_get(unit, EGR_PORTr, 
                                                   reg_val, EGR_PORT_GROUP_IDf);
                }
            }
#ifdef BCM_TRIDENT_SUPPORT
            else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm,
                                         EGR_PORT_GROUP_IDf)) {
                egr_port_entry_t entry;

                rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &entry);
                if (BCM_SUCCESS(rv)) {
                    *pclass_id = soc_mem_field32_get
                        (unit, EGR_PORTm, &entry, EGR_PORT_GROUP_IDf);
                }

            }
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_FIREBOLT_SUPPORT || BCM_TRX_SUPPORT */
            break;
        case bcmPortClassVlanTranslateEgress:
#if defined(BCM_TRX_SUPPORT)
            if (soc_reg_field_valid(unit, EGR_PORT_64r, VT_PORT_GROUP_IDf)) {
                if (SOC_REG_IS_VALID(unit, EGR_PORT_64r)) {
                    rv = READ_EGR_PORT_64r(unit, port, &reg_val64);
                    if (BCM_SUCCESS(rv)) {
                        *pclass_id = soc_reg64_field32_get(unit, EGR_PORT_64r,
                                                       reg_val64, VT_PORT_GROUP_IDf);
                    }
                } 
            } else if (soc_reg_field_valid(unit, EGR_PORTr, VT_PORT_GROUP_IDf)) {
                    rv = READ_EGR_PORTr(unit, port, &reg_val);
                    if (BCM_SUCCESS(rv)) {
                        *pclass_id = soc_reg_field_get(unit, EGR_PORTr,
                                                       reg_val, VT_PORT_GROUP_IDf);
                    }
            }
#ifdef BCM_TRIDENT_SUPPORT
            else if (SOC_MEM_FIELD_VALID(unit, EGR_PORTm, VT_PORT_GROUP_IDf)) {
                egr_port_entry_t entry;

                rv = READ_EGR_PORTm(unit, MEM_BLOCK_ANY, port, &entry);
                if (BCM_SUCCESS(rv)) {
                    *pclass_id = soc_mem_field32_get
                        (unit, EGR_PORTm, &entry, VT_PORT_GROUP_IDf);
                }

            }
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_TRX_SUPPORT */
            break;
        default:
            rv = BCM_E_PARAM;
        }
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return rv;
}


/*
 * Function    : _bcm_esw_port_config_set
 * Description : Internal function to set port configuration.
 * Parameters  : (IN)unit  - BCM device number.
 *               (IN)port  - Port number.
 *               (IN)type  - Port property.   
 *               (IN)value - New property value.
 * Returns     : BCM_E_XXX
 */
int 
_bcm_esw_port_config_set(int unit, bcm_port_t port, 
                         _bcm_port_config_t type, int value)
{
    int rv = BCM_E_UNAVAIL;    /* Operation return status. */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    switch (type) { 
    case _bcmPortL3UrpfMode:       
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   URPF_MODEf, value);
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortL3UrpfDefaultRoute:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   URPF_DEFAULTROUTECHECKf, 
                                   (value) ? 0 : 1);
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortVlanTranslate:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   VT_ENABLEf, value);
#ifdef BCM_TRX_SUPPORT
        if (SOC_IS_TRX(unit) && (rv == SOC_E_NONE)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       VT_KEY_TYPE_USE_GLPf, value);
            if (rv == SOC_E_NONE) {
                rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                           VT_KEY_TYPE_2_USE_GLPf, value);
            }
        }
#endif
        break; 
    case _bcmPortVlanPrecedence:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   VLAN_PRECEDENCEf, value);
        break;
    case _bcmPortVTMissDrop:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   VT_MISS_DROPf, value);
        break;
    case _bcmPortLookupMACEnable:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   MAC_BASED_VID_ENABLEf, value);
        break;
    case _bcmPortLookupIPEnable:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   SUBNET_BASED_VID_ENABLEf,
                                   value);
        break;
    case _bcmPortUseInnerPri:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   USE_INNER_PRIf, value);
#endif /* BCM_RAVEN_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortUseOuterPri:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   TRUST_OUTER_DOT1Pf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVerifyOuterTpid:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   OUTER_TPID_VERIFYf, value);
#endif /* BCM_RAVEN_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break; 
    case _bcmPortVTKeyTypeFirst:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   VT_KEY_TYPEf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVTKeyPortFirst:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   VT_KEY_TYPE_USE_GLPf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVTKeyTypeSecond:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   VT_KEY_TYPE_2f, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVTKeyPortSecond:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   VT_KEY_TYPE_2_USE_GLPf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortIpmcEnable:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   IPMC_ENABLEf, value);
        break;
    case _bcmPortIpmcV4Enable:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   V4IPMC_ENABLEf, value);
        break;
    case _bcmPortIpmcV6Enable:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   V6IPMC_ENABLEf, value);
        break;
    case _bcmPortIpmcVlanKey:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   IPMC_DO_VLANf, value);
        break;
    case _bcmPortCfiAsCng:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   CFI_AS_CNGf, value);
        break;
    case _bcmPortNni:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   NNI_PORTf, value);
        break;
    case _bcmPortHigigTrunkId:
#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRX_SUPPORT)
        if (value < 0) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       HIGIG_TRUNKf, 0);
            if (BCM_SUCCESS(rv)) {
                rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                           HIGIG_TRUNK_IDf, 0);
            }
        } else {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                       HIGIG_TRUNK_IDf, value);
            if (BCM_SUCCESS(rv)) {
                rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                           HIGIG_TRUNKf, 1);
            }
        }
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortModuleLoopback:
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   ALLOW_SRC_MODf, value);
        if (BCM_SUCCESS(rv) && SOC_MEM_IS_VALID(unit, LPORT_TABm)) {
            rv = _bcm_esw_lport_tab_set(unit, port, ALLOW_SRC_MODf, value);
        }
        break;
    case _bcmPortOuterTpidEnables:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   OUTER_TPID_ENABLEf, value);
#endif /* BCM_RAVEN_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
#if defined(BCM_KATANA_SUPPORT)
    case _bcmPortSvcMeterIndex:
        if (SOC_IS_KATANA(unit)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   SVC_METER_INDEXf, value);
        }
        break;
    case _bcmPortSvcMeterOffsetMode:
        if (SOC_IS_KATANA(unit)) {
            rv = _bcm_esw_port_tab_set(unit, port, _BCM_CPU_TABS_BOTH,
                                   SVC_METER_OFFSET_MODEf, value);
        }
        break;
#endif /* BCM_KATANA_SUPPORT */
    default:
        rv = BCM_E_INTERNAL;
    }

    PORT_UNLOCK(unit);
    return (rv);
}

/*
 * Function    : _bcm_esw_port_config_get
 * Description : Internal function to get port configuration.
 * Parameters  : (IN)unit  - BCM device number.
 *               (IN)port  - Port number.
 *               (IN)type  - Port property.   
 *               (OUT)value -Port property value.
 * Returns     : BCM_E_XXX
 */
int 
_bcm_esw_port_config_get(int unit, bcm_port_t port, 
                     _bcm_port_config_t type, int *value)
{
    int rv = BCM_E_UNAVAIL;    /* Operation return status. */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
    int tmp_value; /* Temporary value.     */
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);

    switch (type) { 
    case _bcmPortL3UrpfMode:       
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, URPF_MODEf, value);
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortL3UrpfDefaultRoute:
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, URPF_DEFAULTROUTECHECKf,
                                   &tmp_value);
        *value = (tmp_value) ? FALSE : TRUE; 
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortVlanTranslate:
        rv = _bcm_esw_port_tab_get(unit, port, VT_ENABLEf, value);
        break;
    case _bcmPortVlanPrecedence:
        rv = _bcm_esw_port_tab_get(unit, port, VLAN_PRECEDENCEf, value);
        break; 
    case _bcmPortVTMissDrop:
        rv = _bcm_esw_port_tab_get(unit, port, VT_MISS_DROPf, value);
        break;
    case _bcmPortLookupMACEnable:
        rv = _bcm_esw_port_tab_get(unit, port, MAC_BASED_VID_ENABLEf, value);
        break;
    case _bcmPortLookupIPEnable:
        rv = _bcm_esw_port_tab_get(unit, port, SUBNET_BASED_VID_ENABLEf,
                                   value);
        break;
    case _bcmPortUseInnerPri:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, USE_INNER_PRIf, value);
#endif /* BCM_RAVEN_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortUseOuterPri:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, TRUST_OUTER_DOT1Pf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVerifyOuterTpid:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, OUTER_TPID_VERIFYf, value);
#endif /* BCM_RAVEN_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortVTKeyTypeFirst:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, VT_KEY_TYPEf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVTKeyPortFirst:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, VT_KEY_TYPE_USE_GLPf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVTKeyTypeSecond:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, VT_KEY_TYPE_2f, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortVTKeyPortSecond:
#if defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, VT_KEY_TYPE_2_USE_GLPf, value);
#endif /* BCM_TRX_SUPPORT */
        break;
    case _bcmPortIpmcEnable:
        rv = _bcm_esw_port_tab_get(unit, port, IPMC_ENABLEf, value);
        break;
    case _bcmPortIpmcV4Enable:
        rv = _bcm_esw_port_tab_get(unit, port, V4IPMC_ENABLEf, value);
        break;
    case _bcmPortIpmcV6Enable:
        rv = _bcm_esw_port_tab_get(unit, port, V6IPMC_ENABLEf, value);
        break;
    case _bcmPortIpmcVlanKey:
        rv = _bcm_esw_port_tab_get(unit, port, IPMC_DO_VLANf, value);
        break;
    case _bcmPortCfiAsCng:
        rv = _bcm_esw_port_tab_get(unit, port, CFI_AS_CNGf, value);
        break;
    case _bcmPortNni:
        rv = _bcm_esw_port_tab_get(unit, port, NNI_PORTf, value);
        break;
    case _bcmPortHigigTrunkId:
#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, HIGIG_TRUNKf, value);
        if (BCM_SUCCESS(rv)) {
            if (*value == 0) {
                *value = -1;
            } else {
                rv = _bcm_esw_port_tab_get(unit, port,
                                           HIGIG_TRUNK_IDf, value);
            }
        }
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */
        break;
    case _bcmPortModuleLoopback:
        rv = _bcm_esw_port_tab_get(unit, port, ALLOW_SRC_MODf, value);
        break;
    case _bcmPortOuterTpidEnables:
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_FIREBOLT2_SUPPORT) || \
    defined(BCM_TRX_SUPPORT)
        rv = _bcm_esw_port_tab_get(unit, port, OUTER_TPID_ENABLEf, value);
#endif /* BCM_RAVEN_SUPPORT || BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT */
        break;
#if defined(BCM_KATANA_SUPPORT)
    case _bcmPortSvcMeterIndex:
        if (SOC_IS_KATANA(unit)) {
            rv = _bcm_esw_port_tab_get(unit, port, SVC_METER_INDEXf, value);
        }
        break;
    case _bcmPortSvcMeterOffsetMode:
        if (SOC_IS_KATANA(unit)) {
            rv = _bcm_esw_port_tab_get(unit, port, SVC_METER_OFFSET_MODEf, value);
        }
        break;
#endif /* BCM_KATANA_SUPPORT */
    default:
        rv = BCM_E_INTERNAL;
    }
    PORT_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *     bcm_port_force_vlan_set
 * Description:
 *     To set the force vlan attribute of a port
 * Parameters:
 *     unit        device number
 *     port        port number
 *     vlan        vlan identifier
 *                 (0 - 4095) - use this VLAN id if egress packet is tagged
 *     pkt_prio    egress packet priority (-1, 0..7)
 *                 any negative priority value disable the priority
 *                 override if the egress packet is tagged
 *     flags       bit fields
 *                 BCM_PORT_FORCE_VLAN_ENABLE - enable force vlan on this
 *                                              port
 *                 BCM_PORT_FORCE_VLAN_UNTAG - egress untagged when force
 *                                             vlan is enabled on this port
 *
 * Return:
 *     BCM_E_NONE
 *     BCM_E_UNIT
 *     BCM_E_PORT
 *     BCM_E_PARAM
 *     BCM_E_UNAVAIL
 *     BCM_E_XXX
 */
int
bcm_esw_port_force_vlan_set(int unit, bcm_port_t port, bcm_vlan_t vlan,
                            int pkt_prio, uint32 flags)
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_TRX_SUPPORT)
    if (SOC_REG_FIELD_VALID(unit, EGR_PVLAN_EPORT_CONTROLr, PVLAN_UNTAGf)) {
        rv = _bcm_trx_port_force_vlan_set(unit, port, vlan, pkt_prio, flags);
    }
#endif /* defined(BCM_TRX_SUPPORT) */

    return rv;
}

/*
 * Function:
 *     bcm_port_force_vlan_get
 * Description:
 *     To get the force vlan attribute of a port
 * Parameters:
 *     unit        device number
 N_UNTAGf*     port        port number
 *     vlan        pointer to vlan identifier
 *                 (0 - 4095) - use this VLAN id if egress packet is tagged
 *                 valid only when BCM_PORT_FORCE_VLAN_ENABLE is set and
 *                 BCM_PORT_FORCE_VLAN_UNTAG is clear
 *     pkt_prio    egress packet priority (-1, 0 - 7)
 *                 valid only when BCM_PORT_FORCE_VLAN_ENABLE is set and
 *                 BCM_PORT_FORCE_VLAN_UNTAG is clear
 *     flags       bit fields
 *                 BCM_PORT_FORCE_VLAN_ENABLE - enable force vlan on this
 *                                              port
 *                 BCM_PORT_FORCE_VLAN_UNTAG - egress untagged when force
 *                                             vlan is enabled on this port
 *
 * Return:
 *     BCM_E_XXX
 */
int
bcm_esw_port_force_vlan_get(int unit, bcm_port_t port, bcm_vlan_t *vlan,
                            int *pkt_prio, uint32 *flags)
{
    int rv = BCM_E_UNAVAIL;

#if defined(BCM_TRX_SUPPORT)
    if (SOC_REG_FIELD_VALID(unit, EGR_PVLAN_EPORT_CONTROLr, PVLAN_UNTAGf)) {
        rv = _bcm_trx_port_force_vlan_get(unit, port, vlan, pkt_prio, flags);
    }
#endif /* defined(BCM_TRX_SUPPORT) */

    return rv;
}

/*
 * Function:
 *     bcm_port_phy_control_set
 * Description:
 *     Set PHY specific properties 
 * Parameters:
 *     unit        device number
 *     port        port number
 *     type        configuration type
 *     value       new value for the configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcm_esw_port_phy_control_set(int unit, bcm_port_t port, 
                             bcm_port_phy_control_t type, uint32 value)
{
    int rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    PORT_LOCK(unit);
    rv = soc_phyctrl_control_set(unit, port, type, value);
    PORT_UNLOCK(unit);

    return rv; 
}

/*
 * Function:
 *     bcm_port_phy_control_get
 * Description:
 *     Set PHY specific properties 
 * Parameters:
 *     unit        device number
 *     port        port number
 *     type        configuration type
 *     value       value for the configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcm_esw_port_phy_control_get(int unit, bcm_port_t port,
                             bcm_port_phy_control_t type, uint32 *value)
{
    int rv;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
    if (NULL == value) {
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);
    rv = soc_phyctrl_control_get(unit, port, type, value); 
    PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_port_phy_firmware_set
 * Purpose:
 *      Write the firmware to the PHY device's non-volatile storage.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port number
 *      flags - (IN) PHY spcific flags, such as BCM_PORT_PHY_INTERNAL 
 *      offset - (IN) Offset to the firmware data array 
 *      array - (IN)  The firmware data array 
 *      length - (IN) The length of the firmware data array 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_esw_port_phy_firmware_set(int unit, bcm_port_t port, uint32 flags, 
                              int offset, uint8 *array, int length)
{
    int rv;
                                                                                
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
                                                                                
    PORT_LOCK(unit);
    rv = soc_phyctrl_firmware_set(unit, port, flags,offset,array,length);
    PORT_UNLOCK(unit);
                                                                                
    return rv;
}

/*
 * Function    : bcm_esw_port_local_get
 * Description : Get the local port from the given GPORT ID.
 *
 * Parameters  : (IN)  unit         - BCM device number
 *               (IN)  gport        - global port identifier
 *               (OUT) local_port   - local port encoded in gport
 * Returns     : BCM_E_XXX
 */
int
bcm_esw_port_local_get(int unit, bcm_gport_t gport, bcm_port_t *local_port)
{
    bcm_module_t my_mod, encap_mod;
    bcm_port_t encap_port;
    int num_modid, isLocal;

    BCM_IF_ERROR_RETURN(bcm_esw_stk_modid_count(unit, &num_modid));
    if ((!num_modid) && !BCM_GPORT_IS_DEVPORT(gport) ){
        /* Only devport gport can be supported on device with no modid */
        return BCM_E_UNAVAIL;
    }

    if (BCM_GPORT_IS_LOCAL(gport)) {
         *local_port = BCM_GPORT_LOCAL_GET(gport);
    } else if (BCM_GPORT_IS_LOCAL_CPU(gport)) {
        *local_port = CMIC_PORT(unit);
    } else if (BCM_GPORT_IS_DEVPORT(gport)) {
        *local_port = BCM_GPORT_DEVPORT_PORT_GET(gport);
        if (unit != BCM_GPORT_DEVPORT_DEVID_GET(gport)) {
            return BCM_E_PORT;
        }
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_stk_my_modid_get(unit, &my_mod));
        encap_mod = BCM_GPORT_MODPORT_MODID_GET(gport);
        encap_port = BCM_GPORT_MODPORT_PORT_GET(gport);
        if (encap_mod == my_mod){
            *local_port = encap_port;
        } else if (num_modid > 1) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_modid_is_local(unit, encap_mod, &isLocal));
            if (isLocal) {
                *local_port = encap_port +
                    (encap_mod - my_mod) * (SOC_MODPORT_MAX(unit) + 1);
            } else {
                return BCM_E_PORT;
            }
        } else {
            return BCM_E_PORT;
        }
        if (soc_feature(unit, soc_feature_sysport_remap)) {
            BCM_XLATE_SYSPORT_S2P(unit, local_port);
        }
        if (!SOC_PORT_VALID(unit, *local_port)) {
            return BCM_E_PORT;
        }
    } else {
        return BCM_E_PORT;
    }

    return BCM_E_NONE;
}

/*
 * Function    : _bcm_esw_modid_is_local
 * Description : Identifies if given modid is local on a given unit
 *
 * Parameters  : (IN)   unit      - BCM device number
 *               (IN)   modnd     - Module ID 
 *               (OUT)  result    - TRUE if modid is local, FALSE otherwise
 * Returns     : BCM_E_XXX
 */
int 
_bcm_esw_modid_is_local(int unit, bcm_module_t modid, int *result)
{
    bcm_module_t    mymodid;    
    int             rv;

    /* Input parameters check. */
    if (NULL == result) {
        return (BCM_E_PARAM);
    }

    /* Get local module id. */
    rv = bcm_esw_stk_my_modid_get(unit, &mymodid);
    if (BCM_E_UNAVAIL == rv) {
        if (BCM_MODID_INVALID == modid) {
            *result = TRUE;
        } else {
            *result = FALSE;
        }
        return (BCM_E_NONE);
    }

    if (mymodid == modid) {
        *result = TRUE;
    } else if ((modid > mymodid) && (modid < mymodid + NUM_MODID(unit))) {
            *result = TRUE;
        } else {
            *result = FALSE;
        }

    return (BCM_E_NONE);
}


/*
 * Function    : _bcm_gport_modport_hw2api_map
 * Description : Remaps module and port from HW space to API space 
 *
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  mod_in    - Module ID to map   
 *               (IN)  port_in   - Physical port to map   
 *               (OUT)  mod_out  - Module ID after mapping
 *               (OUT)  port_out - Port number after mapping 
 * Returns     : BCM_E_XXX
 * Notes       : If mod_out == NULL then port_out will be local physical port.
 */
int 
_bcm_gport_modport_hw2api_map(int unit, bcm_module_t mod_in, bcm_port_t port_in,
                              bcm_module_t *mod_out, bcm_port_t *port_out)
{
    if (port_out == NULL) {
        return (BCM_E_PARAM);
    }

    if (NUM_MODID(unit) == 1) { /* HW device has single modid */
        if (mod_out != NULL) {
            *mod_out = mod_in;
        }
        *port_out = port_in;

        return (BCM_E_NONE);
    }
    /* Here only for devices with multiple modid NUM_MODID(unit) > 1 */
    if (mod_out == NULL) {  /* physical port requested */
        int             isLocal;
        bcm_module_t    mymodid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_modid_is_local(unit, mod_in, &isLocal));
        if (isLocal != TRUE) {
            return (BCM_E_PARAM);
        }
        BCM_IF_ERROR_RETURN (bcm_esw_stk_my_modid_get(unit, &mymodid));
        *port_out = port_in +
            (mod_in - mymodid) * (SOC_MODPORT_MAX(unit) + 1);
    } else {    /* NUM_MODID(unit) > 1 and not local physical port */
        *port_out = port_in % (SOC_MODPORT_MAX(unit) + 1);
        *mod_out = mod_in + port_in / (SOC_MODPORT_MAX(unit) + 1);
    }

    return (BCM_E_NONE);
}

/*
 * Function    : bcm_port_gport_get
 * Description : Get the GPORT ID for the specified physical port.
 *
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  port      - Port number
 *               (OUT) gport     - GPORT ID
 * Returns     : BCM_E_XXX
 * Notes:
 *     Always returns a MODPORT gport or an error.
 */
int
bcm_esw_port_gport_get(int unit, bcm_port_t port, bcm_gport_t *gport)
{
    int                 rv;
    _bcm_gport_dest_t   dest;

    _bcm_gport_dest_t_init(&dest);

    PORT_PARAM_CHECK(unit, port);

    rv = bcm_esw_stk_my_modid_get(unit, &dest.modid);
    
    if (BCM_FAILURE(rv)) {
        return BCM_E_UNAVAIL;
    }

    if (soc_feature(unit, soc_feature_sysport_remap)) {
        BCM_XLATE_SYSPORT_P2S(unit, &port);
    }

    dest.port = port;
    dest.gport_type = _SHR_GPORT_TYPE_MODPORT;

    /* In this case we can safely assume that a port is in hw format */
    /* since it is a physical port number */
    BCM_IF_ERROR_RETURN
        (_bcm_gport_modport_hw2api_map(unit, dest.modid, dest.port, 
                                       &(dest.modid), &(dest.port)));

    return _bcm_esw_gport_construct(unit, &dest, gport); 
}

#if defined(BCM_TRIUMPH2_SUPPORT)
int
_bcm_esw_port_flex_stat_index_set(int unit, bcm_gport_t port, int fs_idx)
{
    bcm_port_t loc_port;
    int rv;

    rv = bcm_esw_port_local_get(unit, port, &loc_port);
    if (BCM_FAILURE(rv)) {
        return BCM_E_NOT_FOUND;  /* Local port disabled */
    }

    PORT_LOCK(unit); /* Keep port tables in sync */
    rv = soc_mem_field32_modify(unit, PORT_TABm, loc_port, VINTF_CTR_IDXf, 
                                fs_idx);
    if (BCM_SUCCESS(rv)) {
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            rv = soc_mem_field32_modify(unit, EGR_PORTm, loc_port,
                                        VINTF_CTR_IDXf,  fs_idx);
        } else 
#endif /* BCM_TRIDENT_SUPPORT */
        {
            rv = soc_reg_field32_modify(unit, EGR_PORT_64r, loc_port,
                                        VINTF_CTR_IDXf, fs_idx);
        }
    }
    PORT_UNLOCK(unit);
    return rv;
}

STATIC int
_bcm_esw_port_stat_param_verify(int unit, bcm_gport_t port)
{
    int vp;

    if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT) && defined(INCLUDE_L3)
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
            return BCM_E_PARAM;
        }
#endif
    } else if (BCM_GPORT_IS_SUBPORT_GROUP(port) ||
               BCM_GPORT_IS_SUBPORT_PORT(port)) {
#if defined(INCLUDE_L3)
        if (_bcm_tr2_subport_gport_used(unit, port) == BCM_E_NOT_FOUND) {
            return BCM_E_PARAM;
        }
    } else if (BCM_GPORT_IS_MIM_PORT(port)) {
        vp = BCM_GPORT_MIM_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
            return BCM_E_PARAM;
        }
    } else if (BCM_GPORT_IS_WLAN_PORT(port)) {
        vp = BCM_GPORT_WLAN_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeWlan)) {
            return BCM_E_PARAM;
        }
#endif
    } else {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &vp));
    }

    return BCM_E_NONE;
}
STATIC int
_bcm_esw_port_stat_param_valid(int unit, bcm_gport_t port)
{
    PORT_INIT(unit);

    if (!soc_feature(unit, soc_feature_gport_service_counters)) {
           return BCM_E_UNAVAIL;
    }
    return _bcm_esw_port_stat_param_verify(unit, port);
}

STATIC _bcm_flex_stat_t
_bcm_esw_port_stat_to_flex_stat(bcm_port_stat_t stat)
{
    _bcm_flex_stat_t flex_stat;

    switch (stat) {
    case bcmPortStatIngressPackets:
        flex_stat = _bcmFlexStatIngressPackets;
        break;
    case bcmPortStatIngressBytes:
        flex_stat = _bcmFlexStatIngressBytes;
        break;
    case bcmPortStatEgressPackets:
        flex_stat = _bcmFlexStatEgressPackets;
        break;
    case bcmPortStatEgressBytes:
        flex_stat = _bcmFlexStatEgressBytes;
        break;
    default:
        flex_stat = _bcmFlexStatNum;
    }

    return flex_stat;
}

/* Requires "idx" variable */
#define BCM_PORT_VALUE_ARRAY_VALID(unit, nstat, value_arr) \
    for (idx = 0; idx < nstat; idx++) { \
        if (NULL == value_arr + idx) { \
            return (BCM_E_PARAM); \
        } \
    }

STATIC int
_bcm_port_stat_array_convert(int unit, int nstat, bcm_port_stat_t *stat_arr, 
                             _bcm_flex_stat_t *fs_arr)
{
    int idx;

    if ((nstat <= 0) || (nstat > _bcmFlexStatNum)) {
        return BCM_E_PARAM;
    }

    for (idx = 0; idx < nstat; idx++) {
        if (NULL == stat_arr + idx) {
            return (BCM_E_PARAM);
        }
        fs_arr[idx] = _bcm_esw_port_stat_to_flex_stat(stat_arr[idx]);
    }
    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      bcm_esw_port_stat_enable_set
 * Purpose:
 *      Enable/disable packet and byte counters for the selected
 *      gport.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      enable - (IN) Non-zero to enable counter collection, zero to disable.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_enable_set(int unit, bcm_gport_t port, int enable)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
#if defined(BCM_KATANA_SUPPORT)
    uint32                     num_of_tables=0;
    uint32                     num_stat_counter_ids=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    bcm_stat_object_t          object=bcmStatObjectIngPort;
    uint32                     stat_counter_id[
                                       BCM_STAT_FLEX_COUNTER_MAX_DIRECTION]={0};
    uint32                     num_entries=0;
    int                        index=0;

#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    return _bcm_esw_flex_stat_enable_set(unit, _bcmFlexStatTypeGport,
                             _bcm_esw_port_flex_stat_hw_index_set,
                                         NULL, port, enable);
  } 
#if defined(BCM_KATANA_SUPPORT)
    BCM_IF_ERROR_RETURN(_bcm_esw_port_stat_get_table_info(
                        unit,port,&num_of_tables,&table_info[0]));
    if (enable ) {
        for(index=0;index < num_of_tables ;index++) {
            if(table_info[index].direction == bcmStatFlexDirectionIngress) {
               BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_ingress_object(
                                   table_info[index].table,&object));
            } else {
                BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_egress_object(
                                    table_info[index].table,&object));
            }
            BCM_IF_ERROR_RETURN(bcm_esw_stat_group_create(
                                unit,object,bcmStatGroupModeSingle,
                                &stat_counter_id[table_info[index].direction],
                                &num_entries));
            BCM_IF_ERROR_RETURN(bcm_esw_port_stat_attach(
                                unit,port,
                                stat_counter_id[table_info[index].direction]));
        }
        return BCM_E_NONE;
    } else {
        BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_counter_id(
                            unit, num_of_tables,&table_info[0],
                            &num_stat_counter_ids,&stat_counter_id[0]));
        if ((stat_counter_id[bcmStatFlexDirectionIngress] == 0) &&
            (stat_counter_id[bcmStatFlexDirectionEgress] == 0)) {
             return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN(bcm_esw_port_stat_detach(unit,port));
        if (stat_counter_id[bcmStatFlexDirectionIngress] != 0) {
            BCM_IF_ERROR_RETURN(bcm_esw_stat_group_destroy(
                                unit,
                                stat_counter_id[bcmStatFlexDirectionIngress]));
        }
        if (stat_counter_id[bcmStatFlexDirectionEgress] != 0) {
            BCM_IF_ERROR_RETURN(bcm_esw_stat_group_destroy(
                                unit,
                                stat_counter_id[bcmStatFlexDirectionEgress]));
        }
        return BCM_E_NONE;
    }
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif
}

#if defined(BCM_KATANA_SUPPORT)
/*
 * Function:
 *      _bcm_esw_port_stat_get_table_info
 * Description:
 *      Provides relevant flex table information(table-name,index with 
 *      direction)  for given gport.
 *      
 * Parameters:
 *      unit             - (IN) unit number
 *      port             - (IN) GPORT ID  
 *      num_of_tables    - (OUT) Number of flex counter tables
 *      table_info       - (OUT) Flex counter tables information
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
STATIC
bcm_error_t _bcm_esw_port_stat_get_table_info(
                   int                        unit,
                   bcm_gport_t                port,
                   uint32                     *num_of_tables,
                   bcm_stat_flex_table_info_t *table_info)
{
#if defined(INCLUDE_L3)
    int                     vp=0;
    int                     rv=0;
    int                     nh_index=0;
    int                     vp_base=0;
    int                     vt_index =-1;
    bcm_vlan_t              ovid = 0;
    uint32                  port_class = 0;
    bcm_port_t              egress_port = 0;
    ing_l3_next_hop_entry_t ing_nh={{0}};
    egr_vlan_xlate_entry_t  egr_vent={{0}};
    ing_dvp_table_entry_t   dvp={{0}};
    egr_l3_next_hop_entry_t egr_nh={{0}};
    source_vp_entry_t       svp;
    int                     l3_idx=0;
    egr_l3_intf_entry_t     l3_intf;

#endif
    bcm_port_t  loc_port=0;

    (*num_of_tables) = 0;

    if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
        return BCM_E_UNAVAIL;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_port_stat_param_verify(unit, port));
    if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT) && defined(INCLUDE_L3)
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
            return BCM_E_NOT_FOUND;
        }
        /* Ingress Side */
        table_info[*num_of_tables].table=SOURCE_VPm;
        table_info[*num_of_tables].index=vp;
        table_info[*num_of_tables].direction=bcmStatFlexDirectionIngress;
        (*num_of_tables)++;
        /* Egress Side */
        if (BCM_SUCCESS(READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp))) {
            nh_index = soc_mem_field32_get(unit, ING_DVP_TABLEm, &dvp,
                                           NEXT_HOP_INDEXf);
	    if (BCM_SUCCESS(soc_mem_read(unit, EGR_L3_NEXT_HOPm, MEM_BLOCK_ANY, 
                                         nh_index,&egr_nh))) {
                table_info[*num_of_tables].table= EGR_L3_NEXT_HOPm;
                table_info[*num_of_tables].index=nh_index;
                table_info[*num_of_tables].direction=
                bcmStatFlexDirectionEgress;
                (*num_of_tables)++;
            }
        }
        return BCM_E_NONE;
#endif
    } else if (BCM_GPORT_IS_SUBPORT_GROUP(port) || 
               BCM_GPORT_IS_SUBPORT_PORT(port)) {
#if defined(INCLUDE_L3)
               if (BCM_GPORT_IS_SUBPORT_PORT(port)) {
                   l3_idx = BCM_GPORT_SUBPORT_PORT_GET(port) & 0xfff;
                   BCM_IF_ERROR_RETURN(soc_mem_read(
                                       unit, EGR_L3_INTFm, MEM_BLOCK_ALL, 
                                       l3_idx, &l3_intf));
                   vp = soc_mem_field32_get(unit,EGR_L3_INTFm, &l3_intf, IVIDf);
                   BCM_IF_ERROR_RETURN(READ_SOURCE_VPm(
                                       unit,MEM_BLOCK_ALL,vp,&svp));
                   vp_base = soc_SOURCE_VPm_field32_get(unit, &svp, DVPf);
                   /* vp = BCM_GPORT_SUBPORT_PORT_GET(port); */
                   /* Get the base group VP */
                   /* vp_base = vp & ~(0x7); */
                   /* Get the group's next-hop index */
                   BCM_IF_ERROR_RETURN(READ_ING_DVP_TABLEm(
                                       unit, MEM_BLOCK_ALL, vp_base, &dvp));
                   nh_index = soc_ING_DVP_TABLEm_field32_get(
                                       unit, &dvp, NEXT_HOP_INDEXf);
                   /*Get the group's OVID from the group's egress NextHopEntry*/
                   BCM_IF_ERROR_RETURN(soc_mem_read(
                                       unit, EGR_L3_NEXT_HOPm,
                                       MEM_BLOCK_ALL, nh_index, &egr_nh));
                   ovid = soc_mem_field32_get(
                                  unit, EGR_L3_NEXT_HOPm, &egr_nh, OVIDf);
                   /* Get the egress port class */
                   BCM_IF_ERROR_RETURN (soc_mem_read(
                                        unit, ING_L3_NEXT_HOPm, 
                                        MEM_BLOCK_ANY,nh_index, &ing_nh));
                   if (!soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, Tf)) {
			egress_port = soc_ING_L3_NEXT_HOPm_field32_get(
                                          unit, &ing_nh, PORT_NUMf);
                        BCM_IF_ERROR_RETURN(bcm_esw_port_class_get(
                                    unit, egress_port, 
                                    bcmPortClassVlanTranslateEgress,
                                    &port_class));
                   }
                   sal_memset(&egr_vent, 0, sizeof(egr_vent));
                   soc_EGR_VLAN_XLATEm_field32_set(unit, &egr_vent, VALIDf, 1);
                   soc_EGR_VLAN_XLATEm_field32_set(unit, &egr_vent, OVIDf,ovid);
                   if (soc_feature(unit, soc_feature_subport_enhanced)) {
                       soc_EGR_VLAN_XLATEm_field32_set(unit,&egr_vent,IVIDf,vp);
                   }
                   soc_EGR_VLAN_XLATEm_field32_set(
                                       unit,&egr_vent,ENTRY_TYPEf, 0x01);
		   soc_EGR_VLAN_XLATEm_field32_set(
                                      unit,&egr_vent,PORT_GROUP_IDf,port_class);
                   MEM_LOCK(unit, EGR_VLAN_XLATEm);
		   rv = soc_mem_search(unit,EGR_VLAN_XLATEm,MEM_BLOCK_ALL, 
                                       &vt_index, &egr_vent, &egr_vent, 0);
                   if (rv < 0) {
                       MEM_UNLOCK(unit, EGR_VLAN_XLATEm);
                       return rv;
                   }
                   BCM_IF_ERROR_RETURN(soc_mem_write(unit, EGR_VLAN_XLATEm, 
                                           MEM_BLOCK_ALL, vt_index,&egr_vent));
                   table_info[*num_of_tables].table= EGR_VLAN_XLATEm;
                   table_info[*num_of_tables].index=vt_index;
                   table_info[*num_of_tables].direction=
                                              bcmStatFlexDirectionEgress;
                   (*num_of_tables)++;
                   return BCM_E_NONE;
               } else { /*BCM_GPORT_IS_SUBPORT_GROUP(port) */
#if defined(BCM_TRIUMPH2_SUPPORT)
                   vp_base = BCM_GPORT_SUBPORT_GROUP_GET(port);
                   BCM_IF_ERROR_RETURN(_bcm_tr2_subport_gport_used(unit,port));
                   table_info[*num_of_tables].table= SOURCE_VPm;
                   table_info[*num_of_tables].index=vp_base;
                   table_info[*num_of_tables].direction=
                                              bcmStatFlexDirectionIngress;
                   (*num_of_tables)++;
                   return BCM_E_NONE; 
#endif
               }
    } else if (BCM_GPORT_IS_MIM_PORT(port)) {
               vp = BCM_GPORT_MIM_PORT_ID_GET(port);
               /*MIM_LOCK(unit); */
               if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
                   /*MIM_UNLOCK(unit); */
        	   return BCM_E_NOT_FOUND;
               }	
               /* Ingress side */
               table_info[*num_of_tables].table= SOURCE_VPm;
               table_info[*num_of_tables].index=vp;
               table_info[*num_of_tables].direction=
                                          bcmStatFlexDirectionIngress;
               (*num_of_tables)++;
               /* Egress side */
               if (BCM_SUCCESS(READ_ING_DVP_TABLEm(
                                    unit, MEM_BLOCK_ANY, vp, &dvp))) {
                   nh_index = soc_mem_field32_get(unit, ING_DVP_TABLEm, &dvp,
                                                  NEXT_HOP_INDEXf);
                   if (BCM_SUCCESS(soc_mem_read(
                                       unit, EGR_L3_NEXT_HOPm, MEM_BLOCK_ANY, 
                                       nh_index,&egr_nh))) {
                       table_info[*num_of_tables].table= EGR_L3_NEXT_HOPm;
                       table_info[*num_of_tables].index=nh_index;
                       table_info[*num_of_tables].direction=
                                                  bcmStatFlexDirectionEgress;
                       (*num_of_tables)++;
                   }
               }
               return BCM_E_NONE;
    } else if (BCM_GPORT_IS_WLAN_PORT(port)) {
               vp = BCM_GPORT_WLAN_PORT_ID_GET(port);
               /*WLAN_LOCK(unit); */
               if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeWlan)) {
                   /*WLAN_UNLOCK(unit); */
                   return BCM_E_NOT_FOUND;
               }
               /*Ingress Side */
               table_info[*num_of_tables].table= SOURCE_VPm;
               table_info[*num_of_tables].index=vp;
               table_info[*num_of_tables].direction=bcmStatFlexDirectionIngress;
               (*num_of_tables)++;
               BCM_IF_ERROR_RETURN(READ_ING_DVP_TABLEm(
                                        unit, MEM_BLOCK_ANY, vp, &dvp));
               nh_index = soc_ING_DVP_TABLEm_field32_get(
                                  unit, &dvp, NEXT_HOP_INDEXf);
               table_info[*num_of_tables].table= EGR_L3_NEXT_HOPm;
               table_info[*num_of_tables].index=nh_index;
               table_info[*num_of_tables].direction=bcmStatFlexDirectionEgress;
               (*num_of_tables)++;
               return BCM_E_NONE;
#endif
    } else {
        if(BCM_FAILURE(bcm_esw_port_local_get(unit, port, &loc_port))) {
           return BCM_E_NOT_FOUND;  /* Local port disabled */
        }
        /*PORT_LOCK(unit); */
        table_info[*num_of_tables].table= PORT_TABm;
        table_info[*num_of_tables].index=loc_port;
        table_info[*num_of_tables].direction= bcmStatFlexDirectionIngress;
        (*num_of_tables)++;
        table_info[*num_of_tables].table= EGR_PORTm;
        table_info[*num_of_tables].index=loc_port;
        table_info[*num_of_tables].direction=bcmStatFlexDirectionEgress;
        (*num_of_tables)++;
        return BCM_E_NONE; 
    }
    return BCM_E_NOT_FOUND;
}
#endif
/*
 * Function:
 *      bcm_esw_port_stat_attach
 * Description:
 *      Attach counter entries to the given GPORT
 *      
 * Parameters:
 *      unit             - (IN) unit number
 *      port             - (IN) GPORT ID  
 *      stat_counter_id  - (IN) Stat Counter ID.
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      
 */
bcm_error_t bcm_esw_port_stat_attach(
            int	        unit,
            bcm_gport_t	port,
            uint32      stat_counter_id)
{
#if defined(BCM_KATANA_SUPPORT)
    soc_mem_t                 table=0;
    bcm_stat_flex_direction_t direction=bcmStatFlexDirectionIngress;
    uint32                    pool_number=0;
    uint32                    base_index=0;
    bcm_stat_flex_mode_t      offset_mode=0;
    bcm_stat_object_t         object=bcmStatObjectIngPort;
    bcm_stat_group_mode_t     group_mode= bcmStatGroupModeSingle;
    uint32                    count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
   
    _bcm_esw_stat_get_counter_id_info(
                  stat_counter_id,
                  &group_mode,&object,&offset_mode,&pool_number,&base_index);

        /* Validate object id first */
    if (!((object >= bcmStatObjectIngPort) &&
          (object <= bcmStatObjectEgrL3Intf))) {
           SOC_DEBUG_PRINT((DK_PORT,
                            "Invalid bcm_stat_object_t passed %d \n",object));
           return BCM_E_PARAM;
    }
    /* Validate group_mode */
    if(!((group_mode >= bcmStatGroupModeSingle) &&
         (group_mode <= bcmStatGroupModeDvpType))) {
          SOC_DEBUG_PRINT((DK_PORT,
                           "Invalid bcm_stat_group_mode_t passed %d \n",
                           group_mode));
          return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_stat_flex_get_table_info(
                        object,&table,&direction));

    BCM_IF_ERROR_RETURN(_bcm_esw_port_stat_get_table_info(
                   unit, port,&num_of_tables,&table_info[0]));
    for (count=0; count < num_of_tables ; count++) {
      if ( (table_info[count].direction == direction) &&
           (table_info[count].table == table) ) {
           if(direction == bcmStatFlexDirectionIngress) {
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
 *      bcm_esw_port_stat_detach
 * Description:
 *      Detach counter entries to the given GPORT
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      port             - (IN) GPORT ID
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_port_stat_detach(
            int	        unit,
            bcm_gport_t	port)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32                     count=0;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    bcm_error_t                rv[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = 
                                  {BCM_E_FAIL};
    uint32                     flag[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION] = {0};

    BCM_IF_ERROR_RETURN(_bcm_esw_port_stat_get_table_info(
                        unit, port,&num_of_tables,&table_info[0]));

    for (count=0; count < num_of_tables ; count++) {
      if (table_info[count].direction == bcmStatFlexDirectionIngress) {
           rv[bcmStatFlexDirectionIngress]= 
                    _bcm_esw_stat_flex_detach_ingress_table_counters(
                         unit,
                         table_info[count].table,
                         table_info[count].index);
           flag[bcmStatFlexDirectionIngress] = 1;
      } else {
           rv[bcmStatFlexDirectionEgress] = 
                     _bcm_esw_stat_flex_detach_egress_table_counters(
                          unit,
                          table_info[count].table,
                          table_info[count].index);
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
 *      bcm_esw_port_stat_counter_get
 * Description:
 *      retrieve set of counter statistic values for the given GPORT 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      port             - (IN) GPORT ID
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
bcm_error_t bcm_esw_port_stat_counter_get( 
            int	             unit, 
            bcm_gport_t      port, 
            bcm_port_stat_t  stat,
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

    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatIngressBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         direction = bcmStatFlexDirectionEgress;
    }
    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatEgressPackets)) {
         byte_flag=0;
    } else {
         byte_flag=1;
    }
    
    BCM_IF_ERROR_RETURN(_bcm_esw_port_stat_get_table_info(
                        unit, port,&num_of_tables,&table_info[0]));

    for (table_count=0; table_count < num_of_tables ; table_count++) {
         if (table_info[table_count].direction == direction) {
             for (index_count=0; index_count < num_entries ; index_count++) {
               /*ctr_offset_info.offset_index = counter_indexes[index_count];*/
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
 *      bcm_esw_port_stat_counter_set
 * Description:
 *      set counter statistic values for the given GPORT 
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      port             - (IN) GPORT ID
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      num_entries      - (IN) Number of counter Entries
 *      counter_indexes  - (IN) Pointer to Counter indexes entries
 *      counter_values   - (IN) Pointer to counter values
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_port_stat_counter_set( 
            int	             unit, 
            bcm_gport_t      port, 
            bcm_port_stat_t  stat,
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

    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatIngressBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         direction = bcmStatFlexDirectionEgress;
    }
    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatEgressPackets)) {
        byte_flag=0;
    } else {
        byte_flag=1;
    }
    
    BCM_IF_ERROR_RETURN(_bcm_esw_port_stat_get_table_info(
                   unit, port,&num_of_tables,&table_info[0]));

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
 *      bcm_esw_port_stat_id_get
 * Description:
 *      Get stat counter id associated with given gport
 *
 * Parameters:
 *      unit             - (IN) unit number
 *      port             - (IN) GPORT ID
 *      stat             - (IN) Type of the counter to retrieve
 *                              I.e. ingress/egress byte/packet)
 *      Stat_counter_id  - (OUT) Stat Counter ID
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
bcm_error_t bcm_esw_port_stat_id_get(
            int              unit, 
            bcm_gport_t      port, 
            bcm_port_stat_t  stat, 
            uint32           *stat_counter_id)	 
{
#if defined(BCM_KATANA_SUPPORT)
    bcm_stat_flex_direction_t  direction=bcmStatFlexDirectionIngress;
    uint32                     num_of_tables=0;
    bcm_stat_flex_table_info_t table_info[BCM_STAT_FLEX_COUNTER_MAX_DIRECTION];
    uint32                     index=0;
    uint32                     num_stat_counter_ids=0;

    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatIngressBytes)) {
         direction = bcmStatFlexDirectionIngress;
    } else {
         direction = bcmStatFlexDirectionEgress;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_port_stat_get_table_info(
                        unit,port,&num_of_tables,&table_info[0]));
    for (index=0; index < num_of_tables ; index++) {
         if (table_info[index].direction == direction)
             return _bcm_esw_stat_flex_get_counter_id(
                             unit, 1, &table_info[index],
                             &num_stat_counter_ids, stat_counter_id);
    }
    return BCM_E_NOT_FOUND;
#else
    return BCM_E_UNAVAIL;
#endif
}


/*
 * Function:
 *      bcm_esw_port_stat_get
 * Purpose:
 *      Get 64-bit counter value for specified port statistic type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (OUT) Pointer to a counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_get(int unit, bcm_gport_t port, bcm_port_stat_t stat, 
                      uint64 *val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    return _bcm_esw_flex_stat_get(unit, _bcmFlexStatTypeGport, port,
                           _bcm_esw_port_stat_to_flex_stat(stat), val);
  }
#if defined(BCM_KATANA_SUPPORT)
    BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_get( 
                        unit,port,stat, 1, &counter_indexes, &counter_values));
    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatEgressPackets)) {
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
#endif
}

/*
 * Function:
 *      bcm_esw_port_stat_get32
 * Purpose:
 *      Get lower 32-bit counter value for specified port statistic
 *      type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (OUT) Pointer to a counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_get32(int unit, bcm_gport_t port, 
                        bcm_port_stat_t stat, uint32 *val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    return _bcm_esw_flex_stat_get32(unit, _bcmFlexStatTypeGport, port,
                           _bcm_esw_port_stat_to_flex_stat(stat), val);
  }
#if defined(BCM_KATANA_SUPPORT)
    BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_get( 
                        unit,port,stat, 1, &counter_indexes, &counter_values));
    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatEgressPackets)) {
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
#endif
}

/*
 * Function:
 *      bcm_esw_port_stat_set
 * Purpose:
 *      Set 64-bit counter value for specified port statistic type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (IN) New counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_set(int unit, bcm_gport_t port, bcm_port_stat_t stat, 
                      uint64 val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    return _bcm_esw_flex_stat_set(unit, _bcmFlexStatTypeGport, port,
                           _bcm_esw_port_stat_to_flex_stat(stat), val);
  }
#if defined(BCM_KATANA_SUPPORT)
    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatEgressPackets)) {
         counter_values.packets = COMPILER_64_LO(val);
    } else {
        COMPILER_64_SET(counter_values.bytes,
                        COMPILER_64_HI(val),
                        COMPILER_64_LO(val));
    }
    BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_set( 
                        unit,port,stat, 1, &counter_indexes, &counter_values));
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_port_stat_set32
 * Purpose:
 *      Set lower 32-bit counter value for specified port statistic
 *      type.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      stat - (IN) Type of the counter to retrieve.
 *      val - (IN) New counter value.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_set32(int unit, bcm_gport_t port, 
                        bcm_port_stat_t stat, uint32 val)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    return _bcm_esw_flex_stat_set32(unit, _bcmFlexStatTypeGport, port,
                           _bcm_esw_port_stat_to_flex_stat(stat), val);
  }
#if defined(BCM_KATANA_SUPPORT)
    if ((stat == bcmPortStatIngressPackets) ||
        (stat == bcmPortStatEgressPackets)) {
         counter_values.packets = val;
    } else {
        /* Ignoring high value */
        COMPILER_64_SET(counter_values.bytes,0,val);
    }
    BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_set( 
                        unit,port,stat, 1, &counter_indexes, &counter_values));
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_port_stat_multi_get
 * Purpose:
 *      Get 64-bit counter value for multiple port statistic types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (OUT) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_multi_get(int unit, bcm_gport_t port, int nstat, 
                            bcm_port_stat_t *stat_arr, 
                            uint64 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int idx;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    BCM_IF_ERROR_RETURN
        (_bcm_port_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_PORT_VALUE_ARRAY_VALID(unit, nstat, value_arr);

    return _bcm_esw_flex_stat_multi_get(unit, _bcmFlexStatTypeGport, port,
                                        nstat, fs_arr, value_arr);
  }
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_get( 
                             unit, port, stat_arr[idx], 
                             1, &counter_indexes, &counter_values));
         if ((stat_arr[idx] == bcmPortStatIngressPackets) ||
             (stat_arr[idx] == bcmPortStatEgressPackets)) {
              COMPILER_64_SET(value_arr[idx],0,counter_values.packets);
         } else {
             COMPILER_64_SET(value_arr[idx],
                             COMPILER_64_HI(counter_values.bytes),
                             COMPILER_64_LO(counter_values.bytes));
         }
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_port_stat_multi_get32
 * Purpose:
 *      Get lower 32-bit counter value for multiple port statistic
 *      types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (OUT) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_multi_get32(int unit, bcm_gport_t port, int nstat, 
                              bcm_port_stat_t *stat_arr, 
                              uint32 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int idx;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    BCM_IF_ERROR_RETURN
        (_bcm_port_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_PORT_VALUE_ARRAY_VALID(unit, nstat, value_arr);

    return _bcm_esw_flex_stat_multi_get32(unit, _bcmFlexStatTypeGport, port,
                                          nstat, fs_arr, value_arr);
  }
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_get( 
                             unit, port, stat_arr[idx], 
                             1, &counter_indexes, &counter_values));
         if ((stat_arr[idx] == bcmPortStatIngressPackets) ||
             (stat_arr[idx] == bcmPortStatEgressPackets)) {
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
#endif
}

/*
 * Function:
 *      bcm_esw_port_stat_multi_set
 * Purpose:
 *      Set 64-bit counter value for multiple port statistic types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (IN) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_multi_set(int unit, bcm_gport_t port, int nstat, 
                            bcm_port_stat_t *stat_arr, 
                            uint64 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int idx;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    BCM_IF_ERROR_RETURN
        (_bcm_port_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_PORT_VALUE_ARRAY_VALID(unit, nstat, value_arr);

    return _bcm_esw_flex_stat_multi_set(unit, _bcmFlexStatTypeGport, port,
                                          nstat, fs_arr, value_arr);
  }
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         if ((stat_arr[idx] == bcmPortStatIngressPackets) ||
             (stat_arr[idx] == bcmPortStatEgressPackets)) {
              counter_values.packets = COMPILER_64_LO(value_arr[idx]);
         } else {
             COMPILER_64_SET(counter_values.bytes,
                             COMPILER_64_HI(value_arr[idx]),
                             COMPILER_64_LO(value_arr[idx]));
         }
         BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_set( 
                             unit, port, stat_arr[idx], 
                             1, &counter_indexes, &counter_values));
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_port_stat_multi_set32
 * Purpose:
 *      Set lower 32-bit counter value for multiple port statistic
 *      types.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) GPORT ID
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Collected statistics descriptors array
 *      value_arr - (IN) Collected counters values
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_stat_multi_set32(int unit, bcm_gport_t port, int nstat, 
                              bcm_port_stat_t *stat_arr, 
                              uint32 *value_arr)
{
#if defined(BCM_TRIUMPH2_SUPPORT)
    _bcm_flex_stat_t fs_arr[_bcmFlexStatNum]; /* Normalize stats */
    int idx;
#if defined(BCM_KATANA_SUPPORT)
    uint32           counter_indexes=0;
    bcm_stat_value_t counter_values={0};
#endif

  if (!soc_feature(unit,soc_feature_advanced_flex_counter)) {
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_stat_param_valid(unit, port));
    BCM_IF_ERROR_RETURN
        (_bcm_port_stat_array_convert(unit, nstat, stat_arr, fs_arr));
    BCM_PORT_VALUE_ARRAY_VALID(unit, nstat, value_arr);

    return _bcm_esw_flex_stat_multi_set32(unit, _bcmFlexStatTypeGport, port,
                                          nstat, fs_arr, value_arr);
  }
#if defined(BCM_KATANA_SUPPORT)
    for (idx=0;idx < nstat ; idx ++) {
         if ((stat_arr[idx] == bcmPortStatIngressPackets) ||
             (stat_arr[idx] == bcmPortStatEgressPackets)) {
              counter_values.packets = value_arr[idx];
         } else {
             COMPILER_64_SET(counter_values.bytes,0,value_arr[idx]);
         }
         BCM_IF_ERROR_RETURN(bcm_esw_port_stat_counter_set( 
                             unit, port, stat_arr[idx], 
                             1, &counter_indexes, &counter_values));
    }
    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function    : _bcm_gport_dest_t_init
 * Description : Initialize gport_dest structure
 * Parameters  : (IN/OUT)  gport_dest - Structure to initialize
 * Returns     : None
 */

void
_bcm_gport_dest_t_init(_bcm_gport_dest_t *gport_dest)
{
    sal_memset(gport_dest, 0, sizeof (_bcm_gport_dest_t));
}


/*
 * Function    : _bcm_esw_gport_construct
 * Description : Internal function to construct a gport from 
 *                given parameters 
 * Parameters  : (IN)  unit       - BCM device number
 *               (IN)  gport_dest - Structure that contains destination
 *                                   to encode into a gport
 *               (OUT) gport      - Global port identifier
 * Returns     : BCM_E_XXX
 * Notes       : The modid and port are translated from the
 *               local modid/port space to application space
 */
int 
_bcm_esw_gport_construct(int unit, _bcm_gport_dest_t *gport_dest, bcm_gport_t *gport)
{

    bcm_gport_t     l_gport = 0;
    bcm_module_t    mod_out;
    bcm_port_t      port_out;
    
    if ((NULL == gport_dest) || (NULL == gport) ){
        return BCM_E_PARAM;
    }

    switch (gport_dest->gport_type) {
        case _SHR_GPORT_TYPE_TRUNK:
            SOC_GPORT_TRUNK_SET(l_gport, gport_dest->tgid);
            break;
        case _SHR_GPORT_TYPE_LOCAL_CPU:
            l_gport = BCM_GPORT_LOCAL_CPU;
            break;
        case _SHR_GPORT_TYPE_BLACK_HOLE:
            l_gport = BCM_GPORT_BLACK_HOLE;
            break;
        case _SHR_GPORT_TYPE_LOCAL:
            SOC_GPORT_LOCAL_SET(l_gport, gport_dest->port);
            break;
        case _SHR_GPORT_TYPE_SUBPORT_GROUP:
            SOC_GPORT_SUBPORT_GROUP_SET(l_gport, gport_dest->subport_id);
            break;
        case _SHR_GPORT_TYPE_MPLS_PORT:
            BCM_GPORT_MPLS_PORT_ID_SET(l_gport, gport_dest->mpls_id);
            break;
        case _SHR_GPORT_TYPE_MIM_PORT:
            BCM_GPORT_MIM_PORT_ID_SET(l_gport, gport_dest->mim_id);
            break;
        case _SHR_GPORT_TYPE_WLAN_PORT:
            BCM_GPORT_WLAN_PORT_ID_SET(l_gport, gport_dest->wlan_id);
            break;
        case _SHR_GPORT_TYPE_TRILL_PORT:
            BCM_GPORT_TRILL_PORT_ID_SET(l_gport, gport_dest->trill_id);
            break;
        case _SHR_GPORT_TYPE_VLAN_PORT:
            BCM_GPORT_VLAN_PORT_ID_SET(l_gport, gport_dest->vlan_vp_id);
            break;
        case _SHR_GPORT_TYPE_DEVPORT: 
            BCM_GPORT_DEVPORT_SET(l_gport, unit, gport_dest->port);
            break;
        case _SHR_GPORT_TYPE_MODPORT:
            BCM_IF_ERROR_RETURN (
                _bcm_gport_modport_hw2api_map(unit, gport_dest->modid, 
                                              gport_dest->port, &mod_out,
                                              &port_out));
            SOC_GPORT_MODPORT_SET(l_gport, mod_out, port_out);
            break;
        default:    
            return BCM_E_PARAM;
    }

    *gport = l_gport;
    return BCM_E_NONE;
}

/*
 * Function    : _bcm_esw_gport_resolve
 * Description : Internal function to get modid, port, and trunk_id
 *               from a bcm_gport_t (global port)
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  gport     - Global port identifier
 *               (OUT) modid     - Module ID
 *               (OUT) port      - Port number
 *               (OUT) trunk_id  - Trunk ID
 *               (OUT) id        - HW ID
 * Returns     : BCM_E_XXX
 * Notes       : The modid and port are translated from the
 *               application space to local modid/port space if applicable, 
 *               on units without modid (Fabric) modid will be -1 and port 
 *               will be a local physical port.
 */
int 
_bcm_esw_gport_resolve(int unit, bcm_gport_t gport,
                       bcm_module_t *modid, bcm_port_t *port, 
                       bcm_trunk_t *trunk_id, int *id)
{
    int             local_id, rv = BCM_E_NONE;
    bcm_module_t    mod_in, local_modid;
    bcm_port_t      port_in, local_port;
    bcm_trunk_t     local_tgid;
    
    local_modid = -1;
    local_port = -1;
    local_id = -1;
    local_tgid = BCM_TRUNK_INVALID;

    if (SOC_GPORT_IS_TRUNK(gport)) {
        local_tgid = SOC_GPORT_TRUNK_GET(gport);
    } else if (SOC_GPORT_IS_LOCAL_CPU(gport)) {
        rv = bcm_esw_stk_my_modid_get(unit, &local_modid);
        if (BCM_FAILURE(rv) && !SOC_IS_XGS_FABRIC(unit)) {
            return rv;
        }
        local_port = CMIC_PORT(unit);
    } else if (SOC_GPORT_IS_LOCAL(gport)) {
        BCM_IF_ERROR_RETURN 
            (bcm_esw_stk_my_modid_get(unit, &local_modid));
        local_port = SOC_GPORT_LOCAL_GET(gport);
        if (soc_feature(unit, soc_feature_sysport_remap)) {
            BCM_XLATE_SYSPORT_P2S(unit, &local_port);
        }
        if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
            return BCM_E_PORT;
        }
    } else if (SOC_GPORT_IS_DEVPORT(gport)) {
        rv = bcm_esw_stk_my_modid_get(unit, &local_modid);
        if (BCM_FAILURE(rv)) {
            if (!SOC_IS_XGS_FABRIC(unit)) {
                return rv;
            }
        }
        if (unit != SOC_GPORT_DEVPORT_DEVID_GET(gport)) {
            return BCM_E_PORT;
        }
        local_port = SOC_GPORT_DEVPORT_PORT_GET(gport);

        if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
            return BCM_E_PORT;
        }
    } else if (SOC_GPORT_IS_MODPORT(gport)) {
        mod_in = SOC_GPORT_MODPORT_MODID_GET(gport);
        port_in = SOC_GPORT_MODPORT_PORT_GET(gport);
        PORT_DUALMODID_VALID(unit, port_in);
        rv = _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in, &local_modid, &local_port);

        if (!SOC_MODID_ADDRESSABLE(unit, local_modid)) {
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, local_port)) {
            return BCM_E_PORT;
        }
    }
#if defined(BCM_TRIUMPH_SUPPORT) && defined(BCM_MPLS_SUPPORT) && \
    defined(INCLUDE_L3)
    else if (SOC_GPORT_IS_MPLS_PORT(gport)) {
        if (SOC_IS_TR_VL(unit)) {
            rv = _bcm_tr_mpls_port_resolve(unit, gport, &local_modid,
                                           &local_port, &local_tgid, 
                                           &local_id);
        }
    }
#endif
#if defined (BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    else if (SOC_GPORT_IS_MIM_PORT(gport)) {
        if (soc_feature(unit, soc_feature_mim)) {
            rv = _bcm_tr2_mim_port_resolve(unit, gport, &local_modid,
                                          &local_port, &local_tgid, 
                                          &local_id);
        }
    } else if (SOC_GPORT_IS_WLAN_PORT(gport)) {
        if (soc_feature(unit, soc_feature_wlan)) {
            rv = _bcm_tr2_wlan_port_resolve(unit, gport, &local_modid,
                                           &local_port, &local_tgid, 
                                           &local_id);
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT && INCLUDE_L3 */
#if defined (BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    else if (SOC_GPORT_IS_TRILL_PORT(gport)) {
        if (soc_feature(unit, soc_feature_trill)) {
            rv = _bcm_td_trill_port_resolve(unit, gport, -1, &local_modid,
                                          &local_port, &local_tgid, 
                                          &local_id);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
#if defined (BCM_TRX_SUPPORT) && defined(INCLUDE_L3)
    else if (SOC_GPORT_IS_SUBPORT_GROUP(gport)) {
        if (SOC_IS_TRX(unit)) {
            if (soc_feature(unit, soc_feature_subport_enhanced)) {
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
                rv = _bcm_tr2_subport_group_resolve(unit, gport, &local_modid,
                                                    &local_port, &local_tgid, 
                                                    &local_id);
#endif /* defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3) */
            } else if (soc_feature(unit, soc_feature_subport)) {
                rv = _bcm_tr_subport_group_resolve(unit, gport, &local_modid,
                                                   &local_port, &local_tgid, 
                                                   &local_id);
            }
        }
    } else if (SOC_GPORT_IS_SUBPORT_PORT(gport)) {
        if (SOC_IS_TRX(unit)) {
            if (soc_feature(unit, soc_feature_subport_enhanced)) {
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
                rv = _bcm_tr2_subport_port_resolve(unit, gport, &local_modid,
                                                   &local_port, &local_tgid, 
                                                   &local_id);
#endif /* defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3) */
            } else if (soc_feature(unit, soc_feature_subport)) {
                rv = _bcm_tr_subport_port_resolve(unit, gport, &local_modid,
                                                  &local_port, &local_tgid, 
                                                  &local_id);
            }
        }
    }
#endif /* BCM_TRX_SUPPORT && INCLUDE_L3 */
#ifdef BCM_TRIUMPH_SUPPORT
    else if (SOC_GPORT_IS_SCHEDULER(gport)) {
#if defined (BCM_TRIDENT_SUPPORT)
        if (SOC_IS_TD_TT(unit)) {
            rv = _bcm_td_cosq_port_resolve(unit, gport, &local_modid,
                                           &local_port, &local_tgid, 
                                           &local_id);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#if defined (BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit)) {
            rv = _bcm_tr2_cosq_port_resolve(unit, gport, &local_modid,
                                            &local_port, &local_tgid, 
                                            &local_id);
        } else 
#endif /* BCM_TRIUMPH2_SUPPORT */
        if (SOC_IS_TR_VL(unit)) {
            rv = _bcm_tr_cosq_port_resolve(unit, gport, &local_modid,
                                           &local_port, &local_tgid, 
                                           &local_id);
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined (BCM_TRIDENT_SUPPORT)
    else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
             BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
        if (SOC_IS_TD_TT(unit)) {
            rv = _bcm_td_cosq_port_resolve(unit, gport, &local_modid,
                                           &local_port, &local_tgid, 
                                           &local_id);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */
#if (defined(BCM_TRIDENT_SUPPORT) || defined(BCM_ENDURO_SUPPORT)) && defined(INCLUDE_L3)
    else if (SOC_GPORT_IS_VLAN_PORT(gport)) {
        if (soc_feature(unit, soc_feature_vlan_vp)) {
#if defined(BCM_ENDURO_SUPPORT)
            if (SOC_IS_ENDURO(unit)) {
                rv = _bcm_enduro_vlan_port_resolve(unit, gport, &local_modid,
                                          &local_port, &local_tgid, 
                                          &local_id);
            } else 
#endif /* BCM_ENDURO_SUPPORT */
            {
#if defined(BCM_TRIDENT_SUPPORT)
                rv = _bcm_trident_vlan_port_resolve(unit, gport, &local_modid,
                                          &local_port, &local_tgid, 
                                          &local_id);
#endif /* BCM_TRIDENT_SUPPORT */
            }
        }
    }
#endif /* (BCM_TRIDENT_SUPPORT || BCM_ENDURO_SUPPORT) && INCLUDE_L3 */
#if defined(BCM_TRIDENT_SUPPORT) && defined(INCLUDE_L3)
    else if (SOC_GPORT_IS_NIV_PORT(gport)) {
        if (soc_feature(unit, soc_feature_niv)) {
            rv = _bcm_trident_niv_port_resolve(unit, gport, &local_modid,
                    &local_port, &local_tgid, 
                    &local_id);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
    else if (BCM_GPORT_IS_BLACK_HOLE(gport)) {
         local_modid = BCM_MODID_INVALID;
         local_tgid = BCM_TRUNK_INVALID;
         local_port = BCM_GPORT_INVALID;
         local_id = -1;
         rv = BCM_E_NONE;
    } else {
         /* BCM_GPORT_INVALID should return an error */
         return BCM_E_PORT;
    }
    *modid = local_modid;
    *port = local_port;
    *trunk_id = local_tgid;
    *id = local_id; 
    return (rv);
}

/* Check if a given port is a valid controlling port for the Flex-port
   feature.
*/

unsigned 
_bcm_esw_valid_flex_port_controlling_port(int unit, bcm_port_t port)
{
    uint16 dev_id;
    uint8  rev_id;
    
    if (!SOC_PORT_VALID(unit, port))  return (FALSE);
    
    soc_cm_get_id(unit, &dev_id, &rev_id);
    
    if (dev_id == BCM56636_DEVICE_ID) {
        /* Device is a BCM56636 */

        /* Only valid controlling ports are 42 and 50
	   (ports 30, 34 and 38 come up in gigE, and
	   therefore are not valid Flex-port controlling ports).
	*/
      
        if (port != 42 && port != 50)  return (FALSE);
    } else {
        if ((port != 30) && (port != 34) && (port != 38)
	    && (port != 42) && (port != 46) && (port != 50)
	    ) {
	    return (FALSE);
	}
    }	
    
    return (TRUE);
}


/*
 * Function:
 *      bcm_port_subsidiary_ports_get
 * Purpose:
 *      Given a controlling port, this API returns the set of ancillary ports
 *      belonging to the group (port block) that can be configured to operate
 *      either as a single high-speed port or multiple GE ports. If the input
 *      port is not a controlling port, BCM_E_PORT error will be returned.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      pbmp - (OUT) Ports associated with the hot-swap group
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_esw_port_subsidiary_ports_get(int unit, bcm_port_t port, bcm_pbmp_t *pbmp)
{
    int rv = BCM_E_UNAVAIL;
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_flex_port) &&
         !SOC_IS_ENDURO(unit) && !SOC_IS_HURRICANE(unit) ) {
        int i;

	if (!_bcm_esw_valid_flex_port_controlling_port(unit, port)) {
            return (BCM_E_PORT);
        }

        BCM_PBMP_CLEAR(*pbmp);
        for (i = 0; i < 4; i++) {
            if (SOC_PORT_VALID(unit, port + i)) {
                BCM_PBMP_PORT_ADD(*pbmp, port + i);
            }
        }
        rv = BCM_E_NONE;
    }
#endif
    return rv;
}

/*
 * Function:
 *      _bcm_esw_port_e2ecc_hg_pbm_convert
 * Purpose:
 *      Convert Higig port to E2ECC Higig port bitmap.
 * Parameters:
 *      port - (IN) Higig port number.
 *      converted_hg_pbm - (OUT) Converted Higig port bitmap.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int     
_bcm_esw_port_e2ecc_hg_pbm_convert(bcm_port_t port, uint32 *converted_hg_pbm)
{
    int    hg_pbm_index;

    /* In Triumph2, the mapping between HG_PBM[9:0] and physical port number is:
     *
     * HG_PBM index:  |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
     * Physical Port: | 50 | 46 | 42 | 38 | 34 | 30 | 29 | 28 | 27 | 26 | 
     */
    switch (port) 
    {
        case 26: 
        case 27: 
        case 28: 
        case 29: 
        case 30: 
            hg_pbm_index = port - 26; 
            break;
        case 34: 
        case 38: 
        case 42: 
        case 46: 
        case 50: 
            hg_pbm_index = 5 + (port - 34) / 4; 
            break;
        default: 
            soc_cm_debug(DK_ERR, "Error: Port %d is an invalid Higig port", port);
            return BCM_E_PARAM;
    }

    *converted_hg_pbm = (1 << hg_pbm_index);

    return BCM_E_NONE;
}

STATIC int
_bcm_esw_port_drop_status_enable_set(int unit, bcm_port_t port, int enable)
{
    uint32 rval;
    int idx;

#if defined(BCM_TRIDENT_SUPPORT)
    if (SOC_IS_TD_TT(unit)) {
        return bcm_td_cosq_drop_status_enable_set(unit, port, enable);
    }
#endif /* BCM_TRIDENT_SUPPORT */

    for (idx = 0; idx < 8; idx++) {
        if (SOC_IS_SC_CQ(unit)) {
            SOC_IF_ERROR_RETURN(READ_OP_QUEUE_CONFIGr(unit, port, idx, &rval));
            soc_reg_field_set(unit, OP_QUEUE_CONFIGr, &rval,
                              Q_E2E_DS_ENABLEf, enable ? 1 : 0);
            SOC_IF_ERROR_RETURN(WRITE_OP_QUEUE_CONFIGr(unit, port, idx, rval));
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                   SOC_IS_VALKYRIE2(unit)) {
            SOC_IF_ERROR_RETURN
                (READ_OP_QUEUE_CONFIG1_CELLr(unit, port, idx, &rval));
            soc_reg_field_set(unit, OP_QUEUE_CONFIG1_CELLr, &rval,
                              Q_E2E_DS_EN_CELLf, enable ? 1 : 0);
            SOC_IF_ERROR_RETURN
                (WRITE_OP_QUEUE_CONFIG1_CELLr(unit, port, idx, rval));

            SOC_IF_ERROR_RETURN
                (READ_OP_QUEUE_CONFIG1_PACKETr(unit, port, idx, &rval));
            soc_reg_field_set(unit, OP_QUEUE_CONFIG1_PACKETr, &rval,
                              Q_E2E_DS_EN_PACKETf, enable ? 1 : 0);
            SOC_IF_ERROR_RETURN
                (WRITE_OP_QUEUE_CONFIG1_PACKETr(unit, port, idx, rval));
        }
    }

    SOC_IF_ERROR_RETURN(READ_OP_THR_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, OP_THR_CONFIGr, &rval, EARLY_E2E_SELECTf,
                      enable ? 1 : 0);
    SOC_IF_ERROR_RETURN(WRITE_OP_THR_CONFIGr(unit, rval));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_port_e2ecc_tx
 * Purpose:
 *      Configure E2ECC message transmission.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Local port number.
 *      config - (IN) Congestion configuration.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int      
_bcm_esw_port_e2ecc_tx(int unit, bcm_port_t port,
                       bcm_port_congestion_config_t *config)      
{
    uint32 regval;
    uint32 hg_pbm, converted_hg_pbm, new_hg_pbm;
    int    value_a, value_b;
    int    time_unit_sel, time_units;
    int    src_modid, src_pid;
    soc_higig_e2ecc_hdr_t e2ecc_hdr;
    int blk, blk_num = -1, bindex = 0;

    if (config->flags & BCM_PORT_CONGESTION_CONFIG_TX) {
        /* If BCM_PORT_CONGESTION_CONFIG_TX flag is set, enable E2ECC transmission. */
   
        /* Set chip-wide E2ECC enable bit */ 
        if (SOC_IS_SC_CQ(unit)) {
            SOC_IF_ERROR_RETURN(READ_E2E_HOL_ENr(unit, &regval));
            if (soc_reg_field_get(unit, E2E_HOL_ENr, regval, ENf) == 0) {
                soc_reg_field_set(unit, E2E_HOL_ENr, &regval, ENf, 1);
                SOC_IF_ERROR_RETURN(WRITE_E2E_HOL_ENr(unit, regval));
            }
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                   SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
                   SOC_IS_KATANA(unit)) { 
            SOC_IF_ERROR_RETURN(READ_E2ECC_HOL_ENr(unit, &regval));
            if (soc_reg_field_get(unit, E2ECC_HOL_ENr, regval, ENf) == 0) {
                soc_reg_field_set(unit, E2ECC_HOL_ENr, &regval, ENf, 1);
                SOC_IF_ERROR_RETURN(WRITE_E2ECC_HOL_ENr(unit, regval));
            }
        }

         
        /* In Trident E2E_HOL_ENf is not present. MMY E2ECC_HOL_EN is used */
        /* (Checked with Architecture) */ 
        if ((!SOC_IS_TD_TT(unit)) && (!SOC_IS_KATANA(unit))) {
            /* Set per-port E2ECC TX enable bit */
            SOC_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &regval));
            if (soc_reg_field_get(unit, XPORT_CONFIGr, regval, E2E_HOL_ENf) == 0) {
                soc_reg_field_set(unit, XPORT_CONFIGr, &regval, E2E_HOL_ENf, 1);
                SOC_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, regval));
            }
        }

        /* Set per-queue E2ECC enable bits */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_drop_status_enable_set(unit, port, TRUE));

        /* Set the Higig port bitmap used to transmit E2ECC messages */
        if (SOC_IS_SC_CQ(unit)) {
            SOC_IF_ERROR_RETURN(READ_E2E_HOL_PBMr(unit, &regval));
            hg_pbm = soc_reg_field_get(unit, E2E_HOL_PBMr, regval, PORT_BITMAPf);

            converted_hg_pbm = 1 << port;

            if (!(hg_pbm & converted_hg_pbm)) {
                new_hg_pbm = hg_pbm | converted_hg_pbm;
                soc_reg_field_set(unit, E2E_HOL_PBMr, &regval, PORT_BITMAPf, new_hg_pbm);
                SOC_IF_ERROR_RETURN(WRITE_E2E_HOL_PBMr(unit, regval));
            }
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || SOC_IS_VALKYRIE2(unit)) { 
            SOC_IF_ERROR_RETURN(READ_E2ECC_HOL_PBMr(unit, &regval));
            hg_pbm = soc_reg_field_get(unit, E2ECC_HOL_PBMr, regval, HG_PBMf);

            SOC_IF_ERROR_RETURN(_bcm_esw_port_e2ecc_hg_pbm_convert(port, &converted_hg_pbm));

            if (!(hg_pbm & converted_hg_pbm)) {
                new_hg_pbm = hg_pbm | converted_hg_pbm;
                soc_reg_field_set(unit, E2ECC_HOL_PBMr, &regval, HG_PBMf, new_hg_pbm);
                SOC_IF_ERROR_RETURN(WRITE_E2ECC_HOL_PBMr(unit, regval));
            }
        } else if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                blk = SOC_PORT_BLOCK(unit, port);
                blk_num = SOC_BLOCK_INFO(unit, blk).number;
                bindex = SOC_PORT_BINDEX(unit, port);
                SOC_IF_ERROR_RETURN(READ_E2ECC_TX_ENABLE_BMPr(unit, blk_num, 
                                                              &regval));
                if (!(regval & (1 << bindex))) {
                    new_hg_pbm = regval | (1 << bindex);
                    soc_reg_field_set(unit, E2ECC_TX_ENABLE_BMPr, &regval, 
                                      TX_ENABLE_BMPf, new_hg_pbm);
                    SOC_IF_ERROR_RETURN(WRITE_E2ECC_TX_ENABLE_BMPr(unit,
                                                              blk_num, regval));
                }
        }

        /* Configuring the E2ECC min timer so that during congestion, back-to-back
         * E2ECC messages are generated as fast as hardware allows.
         */
        if (SOC_IS_SC_CQ(unit)) {

            SOC_IF_ERROR_RETURN(READ_E2E_MIN_TX_TIMERr(unit, &regval));
            if (soc_reg_field_get(unit, E2E_MIN_TX_TIMERr, regval, TIMERf) != 0) {
                soc_reg_field_set(unit, E2E_MIN_TX_TIMERr, &regval, TIMERf, 0);
                soc_reg_field_set(unit, E2E_MIN_TX_TIMERr, &regval, LGf, 0);
                SOC_IF_ERROR_RETURN(WRITE_E2E_MIN_TX_TIMERr(unit, regval));
            }
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                   SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
                   SOC_IS_KATANA(unit)) { 

            SOC_IF_ERROR_RETURN(READ_E2ECC_MIN_TX_TIMERr(unit, &regval));
            if (soc_reg_field_get(unit, E2ECC_MIN_TX_TIMERr, regval, TIMERf) != 0) {
                soc_reg_field_set(unit, E2ECC_MIN_TX_TIMERr, &regval, TIMERf, 0);
                soc_reg_field_set(unit, E2ECC_MIN_TX_TIMERr, &regval, LGf, 0);
                SOC_IF_ERROR_RETURN(WRITE_E2ECC_MIN_TX_TIMERr(unit, regval));
            }
        }
        
        /* In the absence of congestion, an E2ECC message is still generated to refresh 
         * congestion status when the E2ECC max timer expires. This timer is specified 
         * in units of 250ns or 25us, and up to 1023 time units can be specified. 
         * The selection of time unit and the number of time units is computed as follows:
         *
         * Let E2ECC message refresh rate = N messages/sec, and N != 0,
         * then A = # of 250ns time units = 1 sec / N / 250ns = 4000000 / N, 
         * and B = # of 25us time units = 1 sec / N / 25us = 40000 / N = A / 100.
         * 
         * If A < 1, configure the minimum allowed timer value: time unit = 250ns, 
         * # of time units = 1. 
         *
         * If A is between 1 and 1023, configure the timer as: time unit = 250ns,
         * # of time units = A. 
         *
         * If A > 1023 and B <= 1023, configure the timer as: time unit = 25us,
         * # of time units = B.  
         *
         * If B > 1023, configure the maximum allowed timer value: time unit = 25us, 
         * # of time units = 1023.
         *
         * Note: If N = 0, # of time units will be set to 0, and E2ECC message refresh
         *       is effectively disabled. E2ECC message will be generated only when
         *       there is a change in congestion status.
         */

        if (config->packets_per_sec == 0) {
            time_unit_sel = 0;
            time_units = 0;
        } else {
            value_a = 4000000 / config->packets_per_sec;
            value_b = value_a / 100;       
            if (value_a < 1) {
                time_unit_sel = 0;
                time_units = 1;
            } else if (value_a <= 1023) {
                time_unit_sel = 0;
                time_units = value_a;
            } else if ((value_a > 1023) && (value_b <= 1023)) {
                time_unit_sel = 1;
                time_units = value_b;
            } else { /* value_b > 1023 */
                time_unit_sel = 1;
                time_units = 1023;
            }
        }

        if (SOC_IS_SC_CQ(unit)) {
            SOC_IF_ERROR_RETURN(READ_E2E_MAX_TX_TIMERr(unit, &regval));
            soc_reg_field_set(unit, E2E_MAX_TX_TIMERr, &regval, LGf,
                              time_unit_sel);
            soc_reg_field_set(unit, E2E_MAX_TX_TIMERr, &regval, TIMERf,
                              time_units);
            SOC_IF_ERROR_RETURN(WRITE_E2E_MAX_TX_TIMERr(unit, regval));
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                   SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) ||
                   SOC_IS_KATANA(unit)) { 
            SOC_IF_ERROR_RETURN(READ_E2ECC_MAX_TX_TIMERr(unit, &regval));
            soc_reg_field_set(unit, E2ECC_MAX_TX_TIMERr, &regval, LGf,
                              time_unit_sel);
            soc_reg_field_set(unit, E2ECC_MAX_TX_TIMERr, &regval, TIMERf,
                              time_units);
            SOC_IF_ERROR_RETURN(WRITE_E2ECC_MAX_TX_TIMERr(unit, regval));
        }

        /* Configure per-port E2ECC message header */  

        if (BCM_GPORT_IS_MODPORT(config->src_port)) {
            src_modid = BCM_GPORT_MODPORT_MODID_GET(config->src_port);
            src_pid = BCM_GPORT_MODPORT_PORT_GET(config->src_port);
        } else {
            soc_cm_debug(DK_ERR, "Error: src_port %d is not of type modid:port.\n", config->src_port);
            return BCM_E_PARAM;
        }

        sal_memset(&e2ecc_hdr, 0, sizeof(soc_higig_e2ecc_hdr_t));

        e2ecc_hdr.overlay1.words[0] =
                    (SOC_HIGIG_START << 24) |                              /* K.SOP[7:0] */
                    (((1 << 4) | (config->traffic_class & 0x0F)) << 16) |  /* MC and TC[3:0] */ 
                    (config->multicast_id & 0xFFFF);                       /* MGID[15:0] */

        e2ecc_hdr.overlay1.words[1] = 
                    ((src_modid & 0xFF) << 24) |                           /* SRC_MODID[7:0] */
                    ((src_pid & 0xFF) << 16) |                             /* SRC_PID[7:0] */
                    ((config->color & 0x03) << 6);                         /* DP[1:0] */

        /* e2ecc_hdr.overlay1.words[2] is all zeroes. */

        e2ecc_hdr.overlay1.words[3] =
            ((config->pri & 0x07) << 29) |   /* VID_HIGH[7:5] */
            ((config->cfi & 0x01) << 28) |   /* VID_HIGH[4] */
            ((config->vlan & 0xfff) << 16) | /* VID_HIGH[3:0], VID_LOW[7:0] */
            ((SOC_HIGIG_OP_BC & 0x07) << 8);  /* OPCODE[2:0] */

        e2ecc_hdr.overlay1.words[4] = 
                    (config->dest_mac[0] << 24) |                          /* MAC-DA[47:40] */
                    (config->dest_mac[1] << 16) |                          /* MAC-DA[39:32] */
                    (config->dest_mac[2] << 8) |                           /* MAC-DA[31:24] */
                    (config->dest_mac[3]);                                 /* MAC-DA[23:16] */

        e2ecc_hdr.overlay1.words[5] = 
                    (config->dest_mac[4] << 24) |                          /* MAC-DA[15:8] */
                    (config->dest_mac[5] << 16) |                          /* MAC-DA[7:0] */
                    (config->src_mac[0] << 8) |                            /* MAC-SA[47:40] */
                    (config->src_mac[1]);                                  /* MAC-SA[39:32] */

        e2ecc_hdr.overlay1.words[6] = 
                    (config->src_mac[2] << 24) |                           /* MAC-SA[31:24] */
                    (config->src_mac[3] << 16) |                           /* MAC-SA[23:16] */
                    (config->src_mac[4] << 8) |                            /* MAC-SA[15:8] */
                    (config->src_mac[5]);                                  /* MAC-SA[7:0] */

        e2ecc_hdr.overlay1.words[7] = 
                    ((config->ethertype & 0xFFFF) << 16) |                 /* Ethertype[15:0] */
                    (config->opcode & 0xFFFF);                             /* Opcode[7:0] */

        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH0r(unit, port, e2ecc_hdr.overlay1.words[0]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH1r(unit, port, e2ecc_hdr.overlay1.words[1]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH2r(unit, port, e2ecc_hdr.overlay1.words[2]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH3r(unit, port, e2ecc_hdr.overlay1.words[3]));

        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D0r(unit, port, e2ecc_hdr.overlay1.words[4]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D1r(unit, port, e2ecc_hdr.overlay1.words[5]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D2r(unit, port, e2ecc_hdr.overlay1.words[6]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D3r(unit, port, e2ecc_hdr.overlay1.words[7]));

        /* Enable QE interop mode */
        if (SOC_IS_SC_CQ(unit)) {
            SOC_IF_ERROR_RETURN(READ_GLOBAL_SHARED_FILL_STATE_CONFIGr(unit, &regval));
            if (soc_reg_field_get(unit, GLOBAL_SHARED_FILL_STATE_CONFIGr, regval, GLOBAL_SHARED_FILL_STATE_ENf) == 0) {
                soc_reg_field_set(unit, GLOBAL_SHARED_FILL_STATE_CONFIGr, &regval, GLOBAL_SHARED_FILL_STATE_ENf, 1);
                SOC_IF_ERROR_RETURN(WRITE_GLOBAL_SHARED_FILL_STATE_CONFIGr(unit, regval));
            }
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || SOC_IS_VALKYRIE2(unit)) { 
            SOC_IF_ERROR_RETURN(READ_QE_INTEROP_CONFIGr(unit, &regval));
            if (soc_reg_field_get(unit, QE_INTEROP_CONFIGr, regval, QE_INTEROP_ENf) == 0) {
                soc_reg_field_set(unit, QE_INTEROP_CONFIGr, &regval, QE_INTEROP_ENf, 1);
                SOC_IF_ERROR_RETURN(WRITE_QE_INTEROP_CONFIGr(unit, regval));
            }
        } else if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            SOC_IF_ERROR_RETURN(READ_THDO_INTEROP_CONFIGr(unit, &regval));
            if (soc_reg_field_get(unit, THDO_INTEROP_CONFIGr, regval, GLOBAL_CNG_STATE_REPORT_ENABLEf) == 0) {
                soc_reg_field_set(unit, THDO_INTEROP_CONFIGr, &regval, GLOBAL_CNG_STATE_REPORT_ENABLEf, 1);
                SOC_IF_ERROR_RETURN(WRITE_THDO_INTEROP_CONFIGr(unit, regval));
            }
        }

    } else {
        /* If BCM_PORT_CONGESTION_CONFIG_TX flag is not set, disable E2ECC transmission. */
        
        if ((!SOC_IS_TD_TT(unit)) && (!SOC_IS_KATANA(unit))) {
            /* Clear per-port E2ECC TX enable bit */
            SOC_IF_ERROR_RETURN(READ_XPORT_CONFIGr(unit, port, &regval));
            if (soc_reg_field_get(unit, XPORT_CONFIGr, regval, E2E_HOL_ENf) == 1) {
                soc_reg_field_set(unit, XPORT_CONFIGr, &regval, E2E_HOL_ENf, 0);
                SOC_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, regval));
            }
        }
 
        /* Clear per-queue E2ECC enable bits */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_drop_status_enable_set(unit, port, FALSE));

        /* Clear the bit corresponding to the given Higig port in the Higig port bitmap 
         * used to transmit E2ECC messages 
         */
        if (SOC_IS_SC_CQ(unit)) {
            SOC_IF_ERROR_RETURN(READ_E2E_HOL_PBMr(unit, &regval));
            hg_pbm = soc_reg_field_get(unit, E2E_HOL_PBMr, regval, PORT_BITMAPf);

            converted_hg_pbm = 1 << port;

            if (hg_pbm & converted_hg_pbm) {
                new_hg_pbm = hg_pbm & ~converted_hg_pbm;
                soc_reg_field_set(unit, E2E_HOL_PBMr, &regval, PORT_BITMAPf, new_hg_pbm);
                SOC_IF_ERROR_RETURN(WRITE_E2E_HOL_PBMr(unit, regval));
            }
        } else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || SOC_IS_VALKYRIE2(unit)) { 
            SOC_IF_ERROR_RETURN(READ_E2ECC_HOL_PBMr(unit, &regval));
            hg_pbm = soc_reg_field_get(unit, E2ECC_HOL_PBMr, regval, HG_PBMf);

            SOC_IF_ERROR_RETURN(_bcm_esw_port_e2ecc_hg_pbm_convert(port, &converted_hg_pbm));

            if (hg_pbm & converted_hg_pbm) {
                new_hg_pbm = hg_pbm & ~converted_hg_pbm;
                soc_reg_field_set(unit, E2ECC_HOL_PBMr, &regval, HG_PBMf, new_hg_pbm);
                SOC_IF_ERROR_RETURN(WRITE_E2ECC_HOL_PBMr(unit, regval));
            }
        } else if (SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
                blk = SOC_PORT_BLOCK(unit, port);
                blk_num = SOC_BLOCK_INFO(unit, blk).number;
                bindex = SOC_PORT_BINDEX(unit, port);
                SOC_IF_ERROR_RETURN(READ_E2ECC_TX_ENABLE_BMPr(unit, blk_num, 
                                                              &regval));
                if (regval & (1 << bindex)) {
                    new_hg_pbm = regval & ~(1 << bindex);
                    soc_reg_field_set(unit, E2ECC_TX_ENABLE_BMPr, &regval, 
                                      TX_ENABLE_BMPf, new_hg_pbm);
                    SOC_IF_ERROR_RETURN(WRITE_E2ECC_TX_ENABLE_BMPr(unit,
                                                              blk_num, regval));
                }
        }

        /* Clear per-port registers holding the E2ECC message header */  

        sal_memset(&e2ecc_hdr, 0, sizeof(soc_higig_e2ecc_hdr_t));

        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH0r(unit, port, e2ecc_hdr.overlay1.words[0]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH1r(unit, port, e2ecc_hdr.overlay1.words[1]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH2r(unit, port, e2ecc_hdr.overlay1.words[2]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_MH3r(unit, port, e2ecc_hdr.overlay1.words[3]));

        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D0r(unit, port, e2ecc_hdr.overlay1.words[4]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D1r(unit, port, e2ecc_hdr.overlay1.words[5]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D2r(unit, port, e2ecc_hdr.overlay1.words[6]));
        SOC_IF_ERROR_RETURN
            (WRITE_XHOL_D3r(unit, port, e2ecc_hdr.overlay1.words[7]));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_port_e2ecc_rx
 * Purpose:
 *      Configure E2ECC message reception.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Local port number.
 *      config - (IN) Congestion configuration.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int      
_bcm_esw_port_e2ecc_rx(int unit, bcm_port_t port,
                       bcm_port_congestion_config_t *config)      
{      
    uint32 regval;
    uint64 regval64;
    uint32 field_val;

    if (config->flags & BCM_PORT_CONGESTION_CONFIG_RX) {
        /* If BCM_PORT_CONGESTION_CONFIG_RX flag is set, enable E2ECC reception. */
   
        /* Make sure chip-wide E2ECC reception disable bit is cleared */ 
        SOC_IF_ERROR_RETURN(READ_ING_CONFIG_64r(unit, &regval64));
        if (soc_reg64_field32_get(unit, ING_CONFIG_64r, regval64,
                                  DISABLE_E2E_HOL_CHECKf) == 1) {
            soc_reg64_field32_set(unit, ING_CONFIG_64r, &regval64,
                                  DISABLE_E2E_HOL_CHECKf, 0);
            SOC_IF_ERROR_RETURN(WRITE_ING_CONFIG_64r(unit, regval64));
        }

        /* Set per-port E2ECC RX enable bit */
        SOC_IF_ERROR_RETURN(READ_IE2E_CONTROLr(unit, port, &regval));
        if (soc_reg_field_get(unit, IE2E_CONTROLr, regval,
                              HOL_ENABLEf) == 0) {
            soc_reg_field_set(unit, IE2E_CONTROLr, &regval, HOL_ENABLEf, 1);
            SOC_IF_ERROR_RETURN(WRITE_IE2E_CONTROLr(unit, port, regval));
        }

        /* Configure E2ECC receive MAC-DA, Length/Type, and Opcode */

        field_val = (config->dest_mac[0] << 8) |    /* MAC-DA[47:40] */
                    (config->dest_mac[1]);          /* MAC-DA[39:32] */
        SOC_IF_ERROR_RETURN(READ_E2E_HOL_RX_DA_MSr(unit, &regval)); 
        soc_reg_field_set(unit, E2E_HOL_RX_DA_MSr, &regval, DAf, field_val);
        SOC_IF_ERROR_RETURN(WRITE_E2E_HOL_RX_DA_MSr(unit, regval)); 

        field_val = (config->dest_mac[2] << 24) |    /* MAC-DA[31:24] */
                    (config->dest_mac[3] << 16) |    /* MAC-DA[23:16] */
                    (config->dest_mac[4] << 8) |     /* MAC-DA[15:8] */
                    (config->dest_mac[5]);           /* MAC-DA[7:0] */
        SOC_IF_ERROR_RETURN(READ_E2E_HOL_RX_DA_LSr(unit, &regval)); 
        soc_reg_field_set(unit, E2E_HOL_RX_DA_LSr, &regval, DAf, field_val);
        SOC_IF_ERROR_RETURN(WRITE_E2E_HOL_RX_DA_LSr(unit, regval)); 

        SOC_IF_ERROR_RETURN(READ_E2E_HOL_RX_LENGTH_TYPEr(unit, &regval)); 
        soc_reg_field_set(unit, E2E_HOL_RX_LENGTH_TYPEr, &regval, LENGTH_TYPEf, config->ethertype);
        SOC_IF_ERROR_RETURN(WRITE_E2E_HOL_RX_LENGTH_TYPEr(unit, regval)); 

        SOC_IF_ERROR_RETURN(READ_E2E_HOL_RX_OPCODEr(unit, &regval)); 
        soc_reg_field_set(unit, E2E_HOL_RX_OPCODEr, &regval, OPCODEf, config->opcode);
        SOC_IF_ERROR_RETURN(WRITE_E2E_HOL_RX_OPCODEr(unit, regval)); 

    } else {
        /* If BCM_PORT_CONGESTION_CONFIG_RX flag is not set, disable E2ECC reception. */
        
        /* Clear per-port E2ECC RX enable bit */
        SOC_IF_ERROR_RETURN(READ_IE2E_CONTROLr(unit, port, &regval));
        if (soc_reg_field_get(unit, IE2E_CONTROLr, regval, HOL_ENABLEf) == 1) {
            soc_reg_field_set(unit, IE2E_CONTROLr, &regval, HOL_ENABLEf, 0);
            SOC_IF_ERROR_RETURN(WRITE_IE2E_CONTROLr(unit, port, regval));
        }
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_esw_port_hcfc(int unit, bcm_port_t port,
                   bcm_port_congestion_config_t *config)
{
    soc_info_t *si;
    soc_reg_t reg;
    int enable, port_base;
    uint64 rval64, fval64;
    uint32 rval, fval_hi, fval_lo;

    si = &SOC_INFO(unit);

    enable = config->flags & BCM_PORT_CONGESTION_CONFIG_TX ? 1 : 0;
    BCM_IF_ERROR_RETURN
        (_bcm_esw_port_drop_status_enable_set(unit, port, enable));

    /* Configure egress status reporting */
    if (port < 33) {
        /* logical ports (not mmu port) 0-32 are in OOBFC_ENG_PORT_EN_0_64 */
        reg = OOBFC_ENG_PORT_EN_0_64r;
        port_base = 0;
    } else {
        /* logical ports (not mmu port) 33-65 are in OOBFC_ENG_PORT_EN_0_64 */
        reg = OOBFC_ENG_PORT_EN_1_64r;
        port_base = 33;
    }
    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, REG_PORT_ANY, 0, &rval64));
    fval64 = soc_reg64_field_get(unit, reg, rval64, ENG_PORT_ENf);
    fval_hi = COMPILER_64_HI(fval64);
    fval_lo = COMPILER_64_LO(fval64);
    if (enable) {
        if (port - port_base < 32) {
            fval_lo |= (1 << (port - port_base));
        } else {
            fval_hi |= (1 << (port - port_base - 32));
        }
    } else {
        if (port - port_base < 32) {
            fval_lo &= ~(1 << (port - port_base));
        } else {
            fval_hi &= ~(1 << (port - port_base - 32));
        }
    }
    COMPILER_64_SET(fval64, fval_hi, fval_lo);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, REG_PORT_ANY, 0, rval64));

    SOC_IF_ERROR_RETURN(READ_OOBFC_GCSr(unit, &rval));
    soc_reg_field_set(unit, OOBFC_GCSr, &rval, GCS_ENf, 0);
    SOC_IF_ERROR_RETURN(WRITE_OOBFC_GCSr(unit, rval));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_port_dmvoqfc_rx
 * Purpose:
 *      Configure VOQFC message reception.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Local port number.
 *      config - (IN) Congestion configuration.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int      
_bcm_esw_port_dmvoqfc_rx(int unit, bcm_port_t port,
                       bcm_port_congestion_config_t *config)      
{      
    uint32 regval;
    uint32 field_val;
    int     en, cos_mode;

    en = config->flags & BCM_PORT_CONGESTION_CONFIG_RX ? 1 : 0;
    cos_mode = config->flags & BCM_PORT_CONGESTION_CONFIG_RX ? 1 : 0;
    SOC_IF_ERROR_RETURN(READ_IE2E_CONTROLr(unit, port, &regval));
    if (en != soc_reg_field_get(unit, IE2E_CONTROLr, regval, 
                                VOQFC_ENABLEf)) {
        soc_reg_field_set(unit, IE2E_CONTROLr, &regval, VOQFC_ENABLEf, en);
        SOC_IF_ERROR_RETURN(WRITE_IE2E_CONTROLr(unit, port, regval));
    }

    SOC_IF_ERROR_RETURN(READ_ING_COS_MODEr(unit, port, &regval));
    if (cos_mode != soc_reg_field_get(unit, ING_COS_MODEr, regval, 
                               QUEUE_MODEf)) {
        soc_reg_field_set(unit, ING_COS_MODEr, &regval, QUEUE_MODEf, cos_mode);
        SOC_IF_ERROR_RETURN(WRITE_ING_COS_MODEr(unit, port, regval));
    }

    if (en) {
        /* Configure DMVOQFC receive MAC-DA, Length/Type, and Opcode */
        field_val = (config->dest_mac[0] << 8) |    /* MAC-DA[47:40] */
                    (config->dest_mac[1]);          /* MAC-DA[39:32] */
        SOC_IF_ERROR_RETURN(READ_ING_VOQFC_MACDA_MSr(unit, &regval));
        soc_reg_field_set(unit, ING_VOQFC_MACDA_MSr, &regval, DAf, field_val);
        SOC_IF_ERROR_RETURN(WRITE_ING_VOQFC_MACDA_MSr(unit, regval));

        field_val = (config->dest_mac[2] << 24) |    /* MAC-DA[31:24] */
                    (config->dest_mac[3] << 16) |    /* MAC-DA[23:16] */
                    (config->dest_mac[4] << 8) |     /* MAC-DA[15:8] */
                    (config->dest_mac[5]);           /* MAC-DA[7:0] */

        SOC_IF_ERROR_RETURN(READ_ING_VOQFC_MACDA_LSr(unit, &regval));
        soc_reg_field_set(unit, ING_VOQFC_MACDA_LSr, &regval, DAf, field_val);
        SOC_IF_ERROR_RETURN(WRITE_ING_VOQFC_MACDA_LSr(unit, regval));


        SOC_IF_ERROR_RETURN(READ_ING_VOQFC_IDr(unit, &regval));
        soc_reg_field_set(unit, ING_VOQFC_IDr, &regval, 
                          LENGTH_TYPEf, config->ethertype);

        soc_reg_field_set(unit, ING_VOQFC_IDr, &regval, OPCODEf, config->opcode);
        SOC_IF_ERROR_RETURN(WRITE_ING_VOQFC_IDr(unit, regval));
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_port_dmvoqfc_tx
 * Purpose:
 *      Configure VOQFC message reception.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Local port number.
 *      config - (IN) Congestion configuration.
 * Returns:
 *      BCM_E_xxx
 */
STATIC int      
_bcm_esw_port_dmvoqfc_tx(int unit, bcm_port_t port,
                       bcm_port_congestion_config_t *config)      
{      
    /*
     * Trident/Titan VOQFC message generation is same as E2ECC, except for
     * ethertype and opcode.
     */
    if (SOC_IS_TD_TT(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_port_e2ecc_tx(unit, port, config));
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_port_congestion_config_set
 * Purpose:
 *      Set end-to-end congestion control.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Gport number.
 *      config - (IN) Structure containing the congestion configuration.
 * Returns:
 *      BCM_E_xxx
 */
int      
bcm_esw_port_congestion_config_set(int unit, bcm_gport_t port,       
                                   bcm_port_congestion_config_t *config)      
{      
    bcm_port_t local_port;

    /* Currently, E2ECC API is only implemented for Scorpion/Conqueror, 
     * Triumph2/Apollo/Valkyrie2, for the purpose of interop with Sirius. 
     */
    if (SOC_IS_SC_CQ(unit) || SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {

        /* Check for parameters */

        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &local_port));

        if (!IS_HG_PORT(unit, local_port)) {
            soc_cm_debug(DK_ERR, "Error: unit %d port %d is not a Higig port. "
                         "E2ECC messages can only be transmitted to or received from "
                         "Higig ports.\n", unit, local_port);
            return BCM_E_PARAM;
        }
 
        if (config == NULL) {
            return BCM_E_PARAM;
        }

        if (!((config->flags & BCM_PORT_CONGESTION_CONFIG_E2ECC) ||
            (config->flags & BCM_PORT_CONGESTION_CONFIG_HCFC) || 
            (config->flags & BCM_PORT_CONGESTION_CONFIG_DESTMOD_FLOW_CONTROL) || 
            (config->flags & BCM_PORT_CONGESTION_CONFIG_MAX_SPLIT4))) {
            return BCM_E_PARAM;
        }

        if (config->packets_per_sec < 0) {
            return BCM_E_PARAM;
        }

        /* Make sure port module is initialized. */
        PORT_INIT(unit);

        /* Lock is needed to modify bcm_port_info */
        PORT_LOCK(unit);

        /* Allocate per-port congestion configuration state on demand */
        if (PORT(unit, local_port).e2ecc_config == NULL) {
            PORT(unit, local_port).e2ecc_config = sal_alloc(sizeof(bcm_port_congestion_config_t),
                                                            "bcm_port_congestion_config");
            if (PORT(unit, local_port).e2ecc_config == NULL) {
                soc_cm_debug(DK_ERR, "Error: unable to allocate memory for bcm_port_congestion_config.\n");
                PORT_UNLOCK(unit);
                return BCM_E_MEMORY;
            }
        }

        /* Store E2ECC configuration */
        *(PORT(unit, local_port).e2ecc_config) = *(config);

        PORT_UNLOCK(unit);

        switch (config->flags &
                (BCM_PORT_CONGESTION_CONFIG_E2ECC |
                 BCM_PORT_CONGESTION_CONFIG_HCFC |
                 BCM_PORT_CONGESTION_CONFIG_DESTMOD_FLOW_CONTROL |
                 BCM_PORT_CONGESTION_CONFIG_MAX_SPLIT4)) {
        case BCM_PORT_CONGESTION_CONFIG_E2ECC:
        case BCM_PORT_CONGESTION_CONFIG_MAX_SPLIT4:
            /* Handle E2ECC message transmission */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_e2ecc_tx(unit, local_port, config));

            /* Handle E2ECC message reception */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_e2ecc_rx(unit, local_port, config));
            break;

        case BCM_PORT_CONGESTION_CONFIG_HCFC:
            BCM_IF_ERROR_RETURN(_bcm_esw_port_hcfc(unit, local_port, config));
            break;

        case BCM_PORT_CONGESTION_CONFIG_DESTMOD_FLOW_CONTROL:
            if (!SOC_IS_TD_TT(unit)) {
                return BCM_E_PARAM;
            }

            /* Handle VOQFC message reception */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_dmvoqfc_rx(unit, local_port, config));

            /* Handle E2ECC message transmission */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_port_dmvoqfc_tx(unit, local_port, config));
            break;
        default:
            return BCM_E_PARAM;
        }
        return BCM_E_NONE;

    } else {
        return BCM_E_UNAVAIL;
    }

}

/*
 * Function:
 *      bcm_port_congestion_config_get
 * Purpose:
 *      Get end-to-end congestion control configuration.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Gport number.
 *      config - (OUT) Structure containing the congestion configuration.
 * Returns:
 *      BCM_E_xxx
 */

int      
bcm_esw_port_congestion_config_get(int unit, bcm_gport_t port,       
                                   bcm_port_congestion_config_t *config)      
{      
    bcm_port_t local_port;

    /* Currently, E2ECC API is only implemented for Scorpion/Conqueror, 
     * Triumph2/Apollo/Valkyrie2, for the purpose of interop with Sirius. 
     */
    if (SOC_IS_SC_CQ(unit) || SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {

        /* Check for parameters */

        BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &local_port));

        if (!IS_HG_PORT(unit, local_port)) {
            soc_cm_debug(DK_ERR, "Error: unit %d port %d is not a Higig port. "
                         "E2ECC messages can only be transmitted to or received from "
                         "Higig ports.\n", unit, local_port);
            return BCM_E_PARAM;
        }
 
        if (config == NULL) {
            return BCM_E_PARAM;
        }

        /* Make sure port module is initialized. */
        PORT_INIT(unit);

        if (PORT(unit, local_port).e2ecc_config == NULL) {
            soc_cm_debug(DK_ERR, "Error: Port %d bcm_port_congestion_config was not set.\n", local_port);
            return BCM_E_NOT_FOUND;
        }

        /* Retrieve E2ECC configuration */
        *config = *(PORT(unit, local_port).e2ecc_config);

        return BCM_E_NONE;

    } else {
        return BCM_E_UNAVAIL;
    }
}      

/*
 * Function:
 *      bcm_port_match_add
 * Purpose:
 *      Add a match to an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port or gport
 *      match - (IN) Match criteria
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_match_add(
    int unit, 
    bcm_gport_t port, 
    bcm_port_match_info_t *match)
{
#if defined(BCM_TRIUMPH_SUPPORT) && defined(INCLUDE_L3)
    int vp=0, rv = BCM_E_NONE;
    bcm_gport_t gp;

    if (match == NULL) {
        return BCM_E_PARAM;
    }
    if (BCM_GPORT_IS_MIM_PORT(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (!soc_feature(unit, soc_feature_mim)) {
            return BCM_E_UNAVAIL;
        }
        vp = BCM_GPORT_MIM_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
            return BCM_E_NOT_FOUND;
        }
        BCM_IF_ERROR_RETURN(_bcm_tr2_mim_port_phys_gport_get(unit, vp, &gp));
#endif
    } else if  (BCM_GPORT_IS_MPLS_PORT(port)) {
        if (!soc_feature(unit, soc_feature_mpls)) {
            return BCM_E_UNAVAIL;
        }
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
            return BCM_E_NOT_FOUND;
        }
#if defined(BCM_MPLS_SUPPORT)
        BCM_IF_ERROR_RETURN(bcm_tr_mpls_port_phys_gport_get(unit, vp, &gp));
#endif
    } else {
        return BCM_E_PORT;
    }

    switch (match->match) {
    case BCM_PORT_MATCH_NONE:
        rv = BCM_E_NONE;
        break;
    case BCM_PORT_MATCH_PORT_VLAN:
        /* Add an entry in the VLAN_XLATE table */
        /* Tag actions need to be adjusted by calling the 
           bcm_vlan_translate_action_add API for the entry we are creating */
        VLAN_CHK_ID(unit, match->match_vlan);
        if (BCM_GPORT_IS_MODPORT(match->port) || 
            BCM_GPORT_IS_TRUNK(match->port)) {
            /* Use the port from bcm_port_match_info_t instead of the physical 
               port from the VP */
            gp = match->port;
        }
        rv = _bcm_tr_vlan_translate_vp_add(unit, gp,
                                            bcmVlanTranslateKeyPortOuter,
                                            match->match_vlan,
                                            BCM_VLAN_INVALID,
                                            vp);
        if (BCM_GPORT_IS_MIM_PORT(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT)
           _bcm_tr2_mim_port_match_count_adjust(unit, vp, 1);
#endif
        } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT)
           _bcm_tr_mpls_port_match_count_adjust(unit, vp, 1);
#endif
        }
        break;
    case BCM_PORT_MATCH_PORT_VLAN_STACKED:
        /* Add an entry in the VLAN_XLATE table */
        /* Tag actions need to be adjusted by calling the 
           bcm_vlan_translate_action_add API for the entry we are creating */
        VLAN_CHK_ID(unit, match->match_vlan);
        VLAN_CHK_ID(unit, match->match_inner_vlan);
        if (BCM_GPORT_IS_MODPORT(match->port) || 
            BCM_GPORT_IS_TRUNK(match->port)) {
            /* Use the port from bcm_port_match_info_t instead of the physical 
               port from the VP */
            gp = match->port;
        }
        rv = _bcm_tr_vlan_translate_vp_add(unit, gp,
                                            bcmVlanTranslateKeyPortDouble,
                                            match->match_vlan,
                                            match->match_inner_vlan,
                                            vp);
        if (BCM_GPORT_IS_MIM_PORT(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT)
        _bcm_tr2_mim_port_match_count_adjust(unit, vp, 1);
#endif
        } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT)
           _bcm_tr_mpls_port_match_count_adjust(unit, vp, 1);
#endif
        }
        break;
    case BCM_PORT_MATCH_PORT_VLAN_RANGE:
#if defined(BCM_TRIUMPH2_SUPPORT)
        /* Add entries in the ING_VLAN_RANGE and VLAN_XLATE tables */
        /* Tag actions need to be adjusted by calling the 
           bcm_vlan_translate_action_add API for the entry we are creating */
        VLAN_CHK_ID(unit, match->match_vlan);
        VLAN_CHK_ID(unit, match->match_vlan_max);
        if (BCM_GPORT_IS_MODPORT(match->port) || 
            BCM_GPORT_IS_TRUNK(match->port)) {
            /* Use the port from bcm_port_match_info_t instead of the physical 
               port from the VP */
            gp = match->port;
        }
        rv = _bcm_tr2_vlan_translate_range_vp_add(unit, gp,
                                                  match->match_vlan,
                                                  match->match_vlan_max,
                                                  vp);
        if (BCM_GPORT_IS_MIM_PORT(port)) {
            _bcm_tr2_mim_port_match_count_adjust(unit, vp, 1);
        } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT)
            _bcm_tr_mpls_port_match_count_adjust(unit, vp, 1);
#endif
        }
#endif
        break;
    default:
        rv = BCM_E_PARAM;
        break;
    }
    return rv; 
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_port_match_delete
 * Purpose:
 *      Remove a match from an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port or gport
 *      match - (IN) Match criteria
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_match_delete(
    int unit, 
    bcm_gport_t port, 
    bcm_port_match_info_t *match)
{
#if defined(BCM_TRIUMPH_SUPPORT) && defined(INCLUDE_L3)
    int vp=0, rv = BCM_E_NONE;
    bcm_gport_t gp;

    if (match == NULL) {
        return BCM_E_PARAM;
    }
    if (BCM_GPORT_IS_MIM_PORT(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (!soc_feature(unit, soc_feature_mim)) {
            return BCM_E_UNAVAIL;
        }
        vp = BCM_GPORT_MIM_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
            return BCM_E_NOT_FOUND;
        }
        BCM_IF_ERROR_RETURN(_bcm_tr2_mim_port_phys_gport_get(unit, vp, &gp));
#endif
    } else if  (BCM_GPORT_IS_MPLS_PORT(port)) {
        if (!soc_feature(unit, soc_feature_mpls)) {
            return BCM_E_UNAVAIL;
        }
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
            return BCM_E_NOT_FOUND;
        }
#if defined(BCM_MPLS_SUPPORT)
        BCM_IF_ERROR_RETURN(bcm_tr_mpls_port_phys_gport_get(unit, vp, &gp));
#endif
    } else {
        return BCM_E_PORT;
    }

    switch (match->match) {
    case BCM_PORT_MATCH_NONE:
        rv = BCM_E_NONE;
        break;
    case BCM_PORT_MATCH_PORT_VLAN:
        /* Add an entry in the VLAN_XLATE table */
        /* Tag actions need to be adjusted by calling the 
           bcm_vlan_translate_action_add API for the entry we are creating */
        VLAN_CHK_ID(unit, match->match_vlan);
        if (BCM_GPORT_IS_MODPORT(match->port) || 
            BCM_GPORT_IS_TRUNK(match->port)) {
            /* Use the port from bcm_port_match_info_t instead of the physical 
               port from the VP */
            gp = match->port;
        }
        rv = _bcm_tr_vlan_translate_vp_delete(unit, gp,
                                               bcmVlanTranslateKeyPortOuter,
                                               match->match_vlan,
                                               BCM_VLAN_INVALID,
                                               vp);

        if (BCM_GPORT_IS_MIM_PORT(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT)
           _bcm_tr2_mim_port_match_count_adjust(unit, vp, -1);
#endif
        } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT)
           _bcm_tr_mpls_port_match_count_adjust(unit, vp, -1);
#endif
        }
        break;
    case BCM_PORT_MATCH_PORT_VLAN_STACKED:
        /* Add an entry in the VLAN_XLATE table */
        /* Tag actions need to be adjusted by calling the 
           bcm_vlan_translate_action_add API for the entry we are creating */
        VLAN_CHK_ID(unit, match->match_vlan);
        VLAN_CHK_ID(unit, match->match_inner_vlan);
        if (BCM_GPORT_IS_MODPORT(match->port) || 
            BCM_GPORT_IS_TRUNK(match->port)) {
            /* Use the port from bcm_port_match_info_t instead of the physical 
               port from the VP */
            gp = match->port;
        }
        rv = _bcm_tr_vlan_translate_vp_delete(unit, gp,
                                               bcmVlanTranslateKeyPortDouble,
                                               match->match_vlan,
                                               match->match_inner_vlan,
                                               vp);

        if (BCM_GPORT_IS_MIM_PORT(port)) {
#if defined(BCM_TRIUMPH2_SUPPORT)
           _bcm_tr2_mim_port_match_count_adjust(unit, vp, -1);
#endif
        } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT)
           _bcm_tr_mpls_port_match_count_adjust(unit, vp, -1);
#endif
        }
        break;
    case BCM_PORT_MATCH_PORT_VLAN_RANGE:
#if defined(BCM_TRIUMPH2_SUPPORT)
        /* Add entries in the ING_VLAN_RANGE and VLAN_XLATE tables */
        /* Tag actions need to be adjusted by calling the 
           bcm_vlan_translate_action_add API for the entry we are creating */
        VLAN_CHK_ID(unit, match->match_vlan);
        VLAN_CHK_ID(unit, match->match_vlan_max);
        if (BCM_GPORT_IS_MODPORT(match->port) || 
            BCM_GPORT_IS_TRUNK(match->port)) {
            /* Use the port from bcm_port_match_info_t instead of the physical 
               port from the VP */
            gp = match->port;
        }
        rv = _bcm_tr2_vlan_translate_range_vp_delete(unit, gp,
                                                     match->match_vlan,
                                                     match->match_vlan_max,
                                                     vp);
        if (BCM_GPORT_IS_MIM_PORT(port)) {
            _bcm_tr2_mim_port_match_count_adjust(unit, vp, -1);
        } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
#if defined(BCM_MPLS_SUPPORT)
            _bcm_tr_mpls_port_match_count_adjust(unit, vp, -1);
#endif
        }
#endif
        break;
    default:
        rv = BCM_E_PARAM;
        break;
    }
    return rv; 
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_port_match_replace
 * Purpose:
 *      Replace an old match with a new one for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port or gport
 *      old_match - (IN) Old match criteria
 *      new_match - (IN) New match criteria
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_match_replace(
    int unit, 
    bcm_gport_t port, 
    bcm_port_match_info_t *old_match, 
    bcm_port_match_info_t *new_match)
{
#if defined(BCM_TRIUMPH_SUPPORT) && defined(INCLUDE_L3)
    int rv = BCM_E_NONE;
    rv = bcm_esw_port_match_add(unit, port, new_match);
    if (BCM_SUCCESS(rv)) {
        rv = bcm_esw_port_match_delete(unit, port, old_match);
    }
    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

extern void
_bcm_trx_vlan_range_profile_entry_get(int unit, bcm_vlan_t *min_vlan,
                                     bcm_vlan_t *max_vlan, uint32 index);

/*
 * Function:
 *      bcm_port_match_multi_get
 * Purpose:
 *      Get all the matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port or gport
 *      size - (IN) Number of elements in match array
 *      match_array - (OUT) Match array
 *      count - (OUT) Match count
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_match_multi_get(
    int unit, 
    bcm_gport_t port, 
    int size, 
    bcm_port_match_info_t *match_array, 
    int *count)
{
#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)
    int id, vp, prof_idx, i, j, index_min, index_max, ctr, rv = BCM_E_NONE;
    int stm_idx = 0;
    uint32 key_type, matched;
    uint8 *vlan_xlate_buf = NULL; /* VLAN_XLATE DMA buffer */
    vlan_xlate_entry_t *vlan_xlate;
    uint8 *source_trunk_map_buf = NULL; /* SOURCE_TRUNK_MAP DMA buffer */
    source_trunk_map_table_entry_t *source_trunk_map;
    bcm_gport_t gp;
    bcm_vlan_t min_vlan[8], max_vlan[8];
    bcm_module_t mod_out, mod_in;
    bcm_port_t port_out, port_in;
    bcm_trunk_t trunk_id;

    /* Parameter checks */
    if ((size < 0) || (count == NULL)) {
        return BCM_E_PARAM;
    }
    if ((size > 0) && (match_array == NULL)) {
        return BCM_E_PARAM;
    }
    if (BCM_GPORT_IS_MIM_PORT(port)) {
        if (!soc_feature(unit, soc_feature_mim)) {
            return BCM_E_UNAVAIL;
        }
        vp = BCM_GPORT_MIM_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
            return BCM_E_NOT_FOUND;
        }
        BCM_IF_ERROR_RETURN(_bcm_tr2_mim_port_phys_gport_get(unit, vp, &gp));
    } else if  (BCM_GPORT_IS_MPLS_PORT(port)) {
        if (!soc_feature(unit, soc_feature_mpls)) {
            return BCM_E_UNAVAIL;
        }
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
        if (!_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
            return BCM_E_NOT_FOUND;
        }
#if defined(BCM_MPLS_SUPPORT)
        BCM_IF_ERROR_RETURN(bcm_tr_mpls_port_phys_gport_get(unit, vp, &gp));
#endif

    } else {
        return BCM_E_PORT;
    }

    rv = _bcm_esw_gport_resolve(unit, gp, &mod_out, &port_out, &trunk_id, &id);
    if (BCM_FAILURE(rv)) {
        goto cleanup;
    }

    /* DMA the relevant tables for traversal */
    vlan_xlate_buf = soc_cm_salloc(unit, SOC_MEM_TABLE_BYTES
                                  (unit, VLAN_XLATEm), 
                                  "VLAN_XLATE buffer");
    if (NULL == vlan_xlate_buf) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }
    index_min = soc_mem_index_min(unit, VLAN_XLATEm);
    index_max = soc_mem_index_max(unit, VLAN_XLATEm);
    if ((rv = soc_mem_read_range(unit, VLAN_XLATEm, MEM_BLOCK_ANY,
                                 index_min, index_max, 
                                 vlan_xlate_buf)) < 0 ) {
        goto cleanup;
    }
    source_trunk_map_buf = soc_cm_salloc(unit, SOC_MEM_TABLE_BYTES
                                 (unit, SOURCE_TRUNK_MAP_TABLEm), 
                                 "SOURCE_TRUNK_MAP_TABLE buffer");
    if (NULL == source_trunk_map_buf) {
        rv = BCM_E_MEMORY;
        goto cleanup;
    }
    index_min = soc_mem_index_min(unit, SOURCE_TRUNK_MAP_TABLEm);
    index_max = soc_mem_index_max(unit, SOURCE_TRUNK_MAP_TABLEm);
    if ((rv = soc_mem_read_range(unit, SOURCE_TRUNK_MAP_TABLEm, MEM_BLOCK_ANY,
                                 index_min, index_max, 
                                 source_trunk_map_buf)) < 0 ) {
        goto cleanup;
    }

    ctr = 0;
    /* Get all VLAN_XLATE matches */
    index_min = soc_mem_index_min(unit, VLAN_XLATEm);
    index_max = soc_mem_index_max(unit, VLAN_XLATEm);
    for (i = index_min; i <= index_max; i++) {
        vlan_xlate = soc_mem_table_idx_to_pointer
                         (unit, VLAN_XLATEm, vlan_xlate_entry_t *, 
                         vlan_xlate_buf, i);
        if (soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, VALIDf) == 0) {
            continue;
        }
        if (soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, MPLS_ACTIONf) != 1) {
            continue; /* Entry is not for an SVP */
        }
        key_type = soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, KEY_TYPEf);
        vp = soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, SOURCE_VPf);
        matched = 0;
        if (BCM_GPORT_IS_MIM_PORT(port)) {
            if (vp == BCM_GPORT_MIM_PORT_ID_GET(port)) {
                matched = 1;
            }
        } else {
            if (vp == BCM_GPORT_MPLS_PORT_ID_GET(port)) {
                matched = 1;
            }
        }
        if (matched) {
            ctr++;
            if (size > 0) {
                switch (key_type) {
                case TR_VLXLT_HASH_KEY_TYPE_OVID:
                    match_array->match_vlan = 
                        soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, OVIDf);
                    /* Check if this match is a VLAN range match */
                    if (!BCM_GPORT_IS_TRUNK(gp)) {
                        if(SOC_IS_ENDURO(unit)) {
                            stm_idx = (mod_out * 64) + port_out;
                        }else {
                            stm_idx = (mod_out * (SOC_PORT_ADDR_MAX(unit) + 1)) 
                                       + port_out;
                        }    
                        source_trunk_map = soc_mem_table_idx_to_pointer
                                              (unit, SOURCE_TRUNK_MAP_TABLEm, 
                                               source_trunk_map_table_entry_t *, 
                                               source_trunk_map_buf, stm_idx);
                        prof_idx = soc_SOURCE_TRUNK_MAP_TABLEm_field32_get(unit, 
                                             source_trunk_map, VLAN_RANGE_IDXf);
                        _bcm_trx_vlan_range_profile_entry_get(unit, min_vlan, 
                                                            max_vlan, prof_idx);
                        for (j = 0; j < 8; j++) {
                            if (min_vlan[j] == match_array->match_vlan) {
                                match_array->match_vlan_max = max_vlan[j];
                                match_array->match = 
                                           BCM_PORT_MATCH_PORT_VLAN_RANGE;
                                break; /* Found a VLAN range match */
                            }
                        }
                        if (j == 8) {
                            match_array->match = BCM_PORT_MATCH_PORT_VLAN;
                        }
                    } else { /* Trunk Port */
                        match_array->match = BCM_PORT_MATCH_PORT_VLAN;
                    }
                    if (soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, Tf)) {
                        trunk_id = soc_VLAN_XLATEm_field32_get
                                       (unit, vlan_xlate, TGIDf);
                        BCM_GPORT_TRUNK_SET(gp, trunk_id);
                    } else {
                        mod_in = soc_VLAN_XLATEm_field32_get
                                     (unit, vlan_xlate, MODULE_IDf);
                        port_in = soc_VLAN_XLATEm_field32_get
                                      (unit, vlan_xlate, PORT_NUMf);

                        rv = _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in, &mod_out, &port_out);
                        BCM_GPORT_MODPORT_SET(gp, mod_out, port_out);
                    }
                    match_array->port = gp;
                    match_array++;
                    break;
                case TR_VLXLT_HASH_KEY_TYPE_IVID_OVID:
                    match_array->match = BCM_PORT_MATCH_PORT_VLAN_STACKED;
                    match_array->match_vlan = 
                        soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, OVIDf);
                    match_array->match_inner_vlan = 
                        soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, IVIDf);
                    if (soc_VLAN_XLATEm_field32_get(unit, vlan_xlate, Tf)) {
                        trunk_id = soc_VLAN_XLATEm_field32_get
                                       (unit, vlan_xlate, TGIDf);
                        BCM_GPORT_TRUNK_SET(gp, trunk_id);
                    } else {
                        mod_in = soc_VLAN_XLATEm_field32_get
                                     (unit, vlan_xlate, MODULE_IDf);
                        port_in = soc_VLAN_XLATEm_field32_get
                                      (unit, vlan_xlate, PORT_NUMf);

                        rv = _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in, &mod_out, &port_out);
                        BCM_GPORT_MODPORT_SET(gp, mod_out, port_out);
                    }
                    match_array->port = gp;
                    match_array++;
                    break;
                default:
                    break;
                }
            }
        }
    }
    *count = ctr;
cleanup:
    if (vlan_xlate_buf) {
        soc_cm_sfree(unit, vlan_xlate_buf);
    }
    if (source_trunk_map_buf) {
        soc_cm_sfree(unit, source_trunk_map_buf);
    }
    return rv; 
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_port_match_set
 * Purpose:
 *      Assign a set of matches to an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port or gport
 *      size - (IN) Number of elements in match array
 *      match_array - (IN) Match array
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_match_set(
    int unit, 
    bcm_gport_t port, 
    int size, 
    bcm_port_match_info_t *match_array)
{
#if defined(BCM_TRIUMPH_SUPPORT) && defined(INCLUDE_L3)
    int i, rv = BCM_E_NONE;
    if ((size <= 0) || (match_array == NULL)) {
        return BCM_E_PARAM;
    }

    /* Add every single match */
    for (i = 0; i < size; i++) {
        rv = bcm_esw_port_match_add(unit, port, match_array + i);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_port_match_delete_all
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port or gport
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_match_delete_all(
    int unit, 
    bcm_gport_t port)
{
#if defined(BCM_TRIUMPH_SUPPORT) && defined(INCLUDE_L3)
    int i, size, count, rv = BCM_E_NONE;
    bcm_port_match_info_t *match_array = NULL;

    /* First get number of entries */
    rv = bcm_esw_port_match_multi_get(unit, port, 0, match_array, &count);
    if (BCM_FAILURE(rv) || (count == 0)) {
        return rv;
    }
    match_array = sal_alloc(sizeof(bcm_port_match_info_t) * count, 
                            "match_array");
    if (match_array == NULL) {
        return BCM_E_MEMORY;
    }

    /* Get all matches */
    size = count;
    rv = bcm_esw_port_match_multi_get(unit, port, size, match_array, &count);
    if (BCM_FAILURE(rv)) {
        sal_free(match_array);
        return rv;
    }

    /* Delete every single match */
    for (i = 0; i < count; i++) {
        rv = bcm_esw_port_match_delete(unit, port, match_array + i);
        if (BCM_FAILURE(rv)) {
            sal_free(match_array);
            return rv;
        }
    }
    sal_free(match_array);
    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      bcm_esw_port_timesync_config_get
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      conf - (IN) Configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */

extern int
bcm_esw_port_timesync_config_get(int unit, bcm_port_t port, int array_size,
                                  bcm_port_timesync_config_t *config_array, 
                                  int *array_count)
{
    int rv = BCM_E_UNAVAIL;
                                                                                
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    /* 
     * Internal timesync function handles port lock and retrieves 
     * timesync config profiles
     */                                                                                
    if (SOC_IS_KATANA(unit) &&
        (soc_feature(unit,soc_feature_timesync_support))) {
#if defined(BCM_KATANA_SUPPORT)
        rv = _bcm_esw_port_timesync_config_get(unit, port, array_size,
                                           config_array, array_count);
#endif /*defined(BCM_KATANA_SUPPORT) */
    } else {
        return BCM_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_port_timesync_config_set
 * Purpose:
 *      Set Timesync Configuration profiles for the port
 * Parameters:
 *      unit            - (IN) bcm device
 *      port            - (IN) port
 *      config_count    - (IN)Count of timesync profile
 *      *config_array   - (IN/OUT) Pointer to array of timesync profiles
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_esw_port_timesync_config_set(int unit, bcm_port_t port, int config_count,
                                  bcm_port_timesync_config_t *config_array)
{
    int rv = BCM_E_UNAVAIL;
                                                                                
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    /* 
     * Internal timesync function handles port lock and adds 
     * timesync config profiles
     */                                                                                

    if (SOC_IS_KATANA(unit) &&
        (soc_feature(unit,soc_feature_timesync_support))) {
#if defined(BCM_KATANA_SUPPORT)
        rv = _bcm_esw_port_timesync_config_set(unit, port, config_count, 
                                           config_array);
#endif /*defined(BCM_KATANA_SUPPORT) */
    } else {
        return BCM_E_UNAVAIL;
    }
                                                                                
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_phy_timesync_config_set
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      conf - (IN) Configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_esw_port_phy_timesync_config_set(int unit, bcm_port_t port, bcm_port_phy_timesync_config_t *conf)
{
    int rv;
                                                                                
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
                                                                                
    PORT_LOCK(unit);
    rv = soc_port_phy_timesync_config_set(unit, port, (soc_port_phy_timesync_config_t *)conf);
    PORT_UNLOCK(unit);
                                                                                
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_phy_timesync_config_get
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      conf - (OUT) Configuration
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_esw_port_phy_timesync_config_get(int unit, bcm_port_t port, bcm_port_phy_timesync_config_t *conf)
{
    int rv;
                                                                                
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
                                                                                
    PORT_LOCK(unit);
    rv = soc_port_phy_timesync_config_get(unit, port, (soc_port_phy_timesync_config_t *)conf);
    PORT_UNLOCK(unit);
                                                                                
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_control_phy_timesync_set
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      type - (IN) Operation
 *      value- (IN) Arg to operation
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_esw_port_control_phy_timesync_set(int unit, bcm_port_t port, bcm_port_control_phy_timesync_t type, uint64 value)
{
    int rv;
                                                                                
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
                                                                                
    PORT_LOCK(unit);
    rv = soc_port_control_phy_timesync_set(unit, port, type, value);
    PORT_UNLOCK(unit);
                                                                                
    return rv;
}

/*
 * Function:
 *      bcm_esw_port_control_phy_timesync_get
 * Purpose:
 *      Delete all matches for an existing port
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port
 *      type - (IN) Operation
 *      value- (OUT) Pointer to arg for operation
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_esw_port_control_phy_timesync_get(int unit, bcm_port_t port, bcm_port_control_phy_timesync_t type, uint64 *value)
{
    int rv;
                                                                                
    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));
                                                                                
    PORT_LOCK(unit);
    rv = soc_port_control_phy_timesync_get(unit, port, type, value);
    PORT_UNLOCK(unit);
                                                                                
    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_port_cleanup(int unit)
{
#if defined(BCM_DRACO15_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    if (SOC_IS_DRACO15(unit) || SOC_IS_FB_FX_HX(unit)) {
        bcm_port_t port;

        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            if (PORT(unit, port).p_vd_pbvl != NULL) {
                sal_free(PORT(unit, port).p_vd_pbvl);
                PORT(unit, port).p_vd_pbvl = NULL;
            }

            if (PORT(unit, port).e2ecc_config != NULL) {
                sal_free(PORT(unit, port).e2ecc_config);
                PORT(unit, port).e2ecc_config = NULL;
            }
        }

        if (bcm_port_info[unit] != NULL) {
            sal_free(bcm_port_info[unit]);
            bcm_port_info[unit] = NULL;
        }
    }
#endif /* BCM_DRACO15_SUPPORT || BCM_FIREBOLT_SUPPORT */

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_port_sw_dump
 * Purpose:
 *     Displays port information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 * Note:
 *    Skip following structure fields: p_phy, p_mac
 */
void
_bcm_port_sw_dump(int unit)
{
    _bcm_port_info_t *info;
    int              i, j;
    int              idxmax, entries;
    bcm_port_t       port;

    info = bcm_port_info[unit];
    idxmax = soc_mem_index_max(unit, VLAN_PROTOCOLm) + 1;
    entries = (idxmax + (_BCM_PORT_VD_PBVL_ESIZE -  1)) / _BCM_PORT_VD_PBVL_ESIZE;

    soc_cm_print("\nSW Information Port - Unit %d\n", unit);
    
    if (info != NULL) {
        for (i = 0; i < SOC_MAX_NUM_PORTS; i++) {
            soc_cm_print("       Port %2d : ", i);
            soc_cm_print("     MAC driver - %s\n", 
                         (info[i].p_mac != NULL) ?
                         (info[i].p_mac)->drv_name : "none");
            soc_cm_print("     Untagged priority - %d\n",
                         info[i].p_ut_prio);
            soc_cm_print("     Double-tag mode - ");
            if (0 != info[i].dtag_mode) {
                if (0 != (info[i].dtag_mode &
                          BCM_PORT_DTAG_MODE_INTERNAL)) {
                    soc_cm_print(" Internal");
                }
                if (0 != (info[i].dtag_mode &
                          BCM_PORT_DTAG_MODE_EXTERNAL)) {
                    soc_cm_print(" External");
                }
                if (0 != (info[i].dtag_mode &
                          BCM_PORT_DTAG_ADD_EXTERNAL_TAG)) {
                    soc_cm_print(" AddTag");
                }
                if (0 != (info[i].dtag_mode &
                          BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG)) {
                    soc_cm_print(" RemoveTag");
                }
            } else {
                soc_cm_print(" None");
            }
            soc_cm_print("\n");
            soc_cm_print("     VLAN_PROTOCOL_DATA index - %d\n",
                         info[i].vlan_prot_ptr);
            soc_cm_print("     Number of VPs - %d\n",
                         info[i].vp_count);
        }

        soc_cm_print("\n");

        soc_cm_print("     Protocol based VLAN - \n");
        soc_cm_print("       Total Entries : %d\n", entries);

        PBMP_ALL_ITER(unit, port) {
            soc_cm_print("       Port %2d : ", port);
            if (info[port].p_vd_pbvl == NULL) {
                soc_cm_print("\n");
                continue;
            }

            for (j = 0; j < entries; j++) {
                if ((j > 0) && !(j % 10)) {
                    soc_cm_print("\n                 ");
                }
                soc_cm_print(" 0x%2.2x", info[port].p_vd_pbvl[j]);
            }
            soc_cm_print("\n");
        }

        if (SOC_IS_SC_CQ(unit) || SOC_IS_TRIUMPH2(unit) ||
            SOC_IS_APOLLO(unit) || SOC_IS_VALKYRIE2(unit) ||
            SOC_IS_TD_TT(unit) || SOC_IS_KATANA(unit)) {
            bcm_port_congestion_config_t *e2ep;

            soc_cm_print("\n");
            soc_cm_print("     E2E Congestion Control - \n");
            PBMP_ALL_ITER(unit, port) {
                e2ep = info[port].e2ecc_config;
                if ((NULL == e2ep) ||
                    (0 == (e2ep->flags & (BCM_PORT_CONGESTION_CONFIG_E2ECC | 
                        BCM_PORT_CONGESTION_CONFIG_DESTMOD_FLOW_CONTROL)))) {
                    continue;
                }

                soc_cm_print("\n       Port : %d\n", port);
                soc_cm_print("         Flags : %s %s %s\n",
                      (e2ep->flags & BCM_PORT_CONGESTION_CONFIG_E2ECC) ? "E2ECC" : "DestMod Flow control",
                             (e2ep->flags & BCM_PORT_CONGESTION_CONFIG_TX) ?
                             "TX" : "",
                             (e2ep->flags & BCM_PORT_CONGESTION_CONFIG_RX) ?
                             "RX" : "");
                if (e2ep->flags & BCM_PORT_CONGESTION_CONFIG_TX) {
                    soc_cm_print("         Port bits : 0x%02x\n",
                                 e2ep->port_bits);
                    soc_cm_print("         Packets/sec : %d\n",
                                 e2ep->packets_per_sec);
                    soc_cm_print("         Source gport : 0x%08x\n",
                                 e2ep->src_port);
                    soc_cm_print("         Multicast ID : 0x%04x\n",
                                 e2ep->multicast_id);
                    soc_cm_print("         Traffic class : %d\n",
                                 e2ep->traffic_class);
                    soc_cm_print("         Color : %d\n",
                                 e2ep->color);
                    soc_cm_print
                        ("         Source MAC : 0x%02x%02x%02x%02x%02x%02x\n",
                         e2ep->src_mac[0], e2ep->src_mac[1],
                         e2ep->src_mac[2], e2ep->src_mac[3],
                         e2ep->src_mac[4], e2ep->src_mac[5]);
                }
                soc_cm_print
                    ("         Destination MAC : "
                     "0x%02x%02x%02x%02x%02x%02x\n",
                             e2ep->dest_mac[0], e2ep->dest_mac[1],
                             e2ep->dest_mac[2], e2ep->dest_mac[3],
                             e2ep->dest_mac[4], e2ep->dest_mac[5]);
                soc_cm_print("         Ethertype : 0x%04x\n",
                             e2ep->ethertype);
                soc_cm_print("         Opcode : 0x%04x\n",
                             e2ep->opcode);
            }
        }

#ifdef BCM_TRIUMPH2_SUPPORT
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                SOC_IS_VALKYRIE2(unit) || SOC_IS_TD_TT(unit)) {

            soc_cm_print("\n");
            soc_cm_print("     Source Modid Egress Blocking Profiles - \n");
            PBMP_ALL_ITER(unit, port) {
                if (-1 == PORT_SRC_MOD_EGR_PROF_PTR(unit, port)) {
                    continue;
                }
                soc_cm_print("       Port %2d : ", port);
                soc_cm_print("Profile index %d\n",
                        PORT_SRC_MOD_EGR_PROF_PTR(unit, port));
            }
            for (i = 0; i < 8; i++) {
                soc_cm_print("       Profile %d : ", i);
                soc_cm_print("Reference Count %2d\n",
                        SRC_MOD_EGR_REF_COUNT(unit, i));
            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */

    }

    soc_cm_print("\n");

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#ifdef INCLUDE_CES

/**
 * Function:
 *      bcm_port_tdm_config_t_init()
 * Purpose:
 *      Initialize TDM port control struct
 *      
 * Parameters:
 *      unit - Unit
 *      port - TDM port
 *
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
void bcm_port_tdm_config_t_init(bcm_tdm_port_config_t *config) {

    if (config != NULL) {
        sal_memset(config, 0, sizeof(bcm_tdm_port_config_t));
    }
    return;
}



/**
 * Function:
 *      bcm_esw_port_tdm_find_port();
 * Purpose:
 *      Finds the port control structure or record.
 *      
 * Parameters:
 *      unit - Unit
 *      port - TDM port
 *
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
bcm_tdm_port_record_t *bcm_esw_port_tdm_find_port(int unit, bcm_port_t port) {
    int i;
    bcm_tdm_port_record_t *record = (bcm_tdm_port_record_t *)SOC_CONTROL(unit)->tdm_ctrl;
    uint32 count = SOC_CONTROL(unit)->tdm_count;
 
    for (i = 0;i < count;i++)
	{
	    if (record->port == port)
		return record;
	    record++;
	}

    return NULL;
}


/**
 * Function:
 *      bcm_esw_port_tdm_add_service()
 * Purpose:
 *      Associates a CES service with the TDM port.
 *      
 * Parameters:
 *      *record - Port record
 *      *serice - CES service record for service using this port.
 *      num_slots - number of timeslots used.
 *      slots - array of slots
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int bcm_esw_port_tdm_add_service(int unit,
				 bcm_tdm_port_record_t *record, 
				 bcm_ces_service_record_t *service,
				 int num_slots,
				 uint16 *slots)
{
    int i;

    /*
     * If unstructured then there can be only one service.
     * The first entry in the service table is used for
     * this service.
     */
    if (!record->config.b_structured) {
	if (record->serviceList[0] != NULL)
	    return BCM_E_INTERNAL;

	record->serviceList[0] = service;
    } else {
	/*
	 * Check that the requested slots are unused 
	 * and assign them to this service
	 */
	for (i = 0;i < num_slots;i++)
	{
	    if (record->serviceList[slots[i]] != NULL &&
		record->serviceList[slots[i]] != service)
	    {
		return BCM_E_INTERNAL;
	    }
	    else
		record->serviceList[slots[i]] = service;
	}
    }

    bcm_esw_port_tdm_cas_replacement_set(unit, record);
    return BCM_E_NONE;
}


/**
 * Function:
 *      bcm_esw_port_tdm_delete_service()
 * Purpose:
 *      Removes a CES service reference from the TDM port. Called only
 *      from the CES control code.
 * Parameters:
 *      *record - Port record
 *      *serice - CES service record for service using this port.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int bcm_esw_port_tdm_delete_service(int unit,
				    bcm_tdm_port_record_t *record, 
				    bcm_ces_service_record_t *service)
{
    int i;

    /*
     * Remove service entry(s)
     */
    for (i = 0;i < BCM_CES_SLOT_MAX;i++)
	{
	    if (record->serviceList[i] == service)
		record->serviceList[i] = NULL;
	}

    bcm_esw_port_tdm_cas_replacement_set(unit, record);

    return BCM_E_NONE;
}

/**
 * Function:
 *      bcm_esw_port_tdm_has_services()
 * Purpose:
 *      Returns TRUE if there are any CES services using this port
 * Parameters:
 *      unit - (IN) Unit number.
 *      record - Port record
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int bcm_esw_port_tdm_has_services(int unit, bcm_tdm_port_record_t *record) 
{
    int i;

    /*
     * Find if there are any services on this port 
     */
    for (i = 0;i < BCM_CES_SLOT_MAX;i++) {
	if (record->serviceList[i] != NULL) {
	    return TRUE;
	}
    }

    return FALSE;
}

/**
 * Function:
 *      bcm_esw_port_tdm_init()
 * Purpose:
 *      Initialize TDM ports
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int bcm_esw_port_tdm_init(int unit)
{
    soc_port_t port;
    uint32 count;
    char *str;
    bcm_ces_tdm_proto_t protocol;
    bcm_tdm_port_record_t **tdm_ctrl = (bcm_tdm_port_record_t **)&SOC_CONTROL(unit)->tdm_ctrl;
    bcm_tdm_port_record_t *record;

    /*
     * Find number of TDM ports
     */
    BCM_PBMP_COUNT(PBMP_TDM_ALL(unit), count);
    SOC_CONTROL(unit)->tdm_count = count;

    /*
     * Allocate control structs
     */
    *tdm_ctrl = (bcm_tdm_port_record_t*)sal_alloc(sizeof(bcm_tdm_port_record_t) * count, "TDM port record");
    if ( *tdm_ctrl == NULL)
	return BCM_E_MEMORY;

    sal_memset(*tdm_ctrl, 0, sizeof(bcm_tdm_port_record_t) * count);

    /*
     * Initialize control structs
     */

    str = soc_property_get_str(unit, spn_CES_PORT_TDM_PROTO);
    if (str != NULL)
    {
	if ( sal_strcmp(str, "T1") == 0 )
	    protocol = bcmCesTdmProtocolT1;
	else if ( sal_strcmp(str, "E1") == 0 )
	    protocol = bcmCesTdmProtocolE1;
	else
	{
	    /* TODO cleanup */
	    return BCM_E_PARAM;
	}
    }
    else
	protocol = bcmCesTdmProtocolT1;


    record = *tdm_ctrl;

    PBMP_TDM_ITER(unit, port) 
    {
	record->port = port;
	record->config.e_protocol = protocol;
	record->config.b_structured    = FALSE;
	record->config.b_octet_aligned = FALSE;
	record->config.b_T1_D4_framing = FALSE;
	record->config.b_signaling_enable = FALSE;
	record->config.e_clk_rx_select = bcmCesRxClkSelectIndependent;
	record->config.e_clk_tx_select = bcmCesTxClkSelectCclk;
	record->config.n_cas_idle_timeslots = 0xffffffff;
	record->config.n_step_size = 1;
	if (protocol == bcmCesTdmProtocolT1) {
	    record->config.b_txcrc = TRUE;
	    record->config.b_rxcrc = TRUE;
	} else {
	    record->config.b_txcrc = FALSE;
	    record->config.b_rxcrc = FALSE;
	}
	record->config.n_signaling_format = 0;
	record->config.b_master = TRUE;

	BCM_CES_SET_MODIFIED(record);

	/*
	 * Next
	 */
	record++;
    }

    return BCM_E_NONE;
}

/**
 * Function:
 *      bcm_esw_port_tdm_cas_replacement_set
 * Purpose:
 *      Set TDM port CAS replacement mask
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      config - (IN) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
void bcm_esw_port_tdm_cas_replacement_set(int unit, bcm_tdm_port_record_t *record) {
    int i;
    AgNdMsgConfigCasDataReplace ndCasDataReplace;
    AgResult n_ret;
    bcm_ces_service_global_config_t *ctrl = SOC_CONTROL(unit)->ces_ctrl;

    if (record->config.b_signaling_enable) {
	/*
	 * Find all unused time slots
	 */
	record->config.n_cas_idle_timeslots = 0x00000000;

	for (i = 0;i < BCM_CES_SLOT_MAX;i++) {
	    if (record->serviceList[i] == NULL) {
		record->config.n_cas_idle_timeslots |= (1 << i);
	    }
	}
    } else {
	record->config.n_cas_idle_timeslots = 0xffffffff;
    }

    /*
     * Set CAS data replace mask
     */
    ndCasDataReplace.n_circuit_id = bcm_ces_port_to_circuit_id(unit, record->port);
    ndCasDataReplace.n_cas_idle_timeslots = record->config.n_cas_idle_timeslots;
    n_ret = ag_nd_device_write(ctrl->ndHandle, AG_ND_OPCODE_CONFIG_CAS_DATA_REPLACE, &ndCasDataReplace);
}


/**
 * Function:
 *      bcm_esw_port_tdm_config_set
 * Purpose:
 *      Set TDM port attributes.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      config - (IN) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_config_set(int unit, 
			    bcm_port_t tdm_port, 
			    bcm_tdm_port_config_t *config)
{
    bcm_tdm_port_record_t *record;
    int i;
    int ret;
    int16 modServices[BCM_CES_SLOT_MAX];
    int j;

    memset(modServices, -1, sizeof(modServices));

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    /*
     * Are the configs different?
     */
    if (sal_memcmp(config, &record->config, sizeof(bcm_tdm_port_config_t)) != 0) {
	/*
	 * Has the structure changed? If it has then we need to check to see
	 * if there are any services using this port, if there are then 
	 * the structure can not be changed.
	 */
	if (config->b_structured != record->config.b_structured &&
	    bcm_esw_port_tdm_has_services(unit, record)) {
	    return BCM_E_BUSY;
	}


	sal_memcpy(&record->config, config, sizeof(bcm_tdm_port_config_t));

	/*
	 * Mark as modified
	 */
	BCM_CES_SET_MODIFIED(record);

	/*
	 * Apply the config to all CES services using this port
	 */
	record->config.n_cas_idle_timeslots = 0x00000000;

	for (i = 0;i < BCM_CES_SLOT_MAX;i++) {
	    if (record->serviceList[i] != NULL) {
		/*
		 * Has the service already been modified?
		 */
		j = 0;
		while(modServices[j] != -1 && j < BCM_CES_SLOT_MAX) {
		    if (modServices[j] == (int16)record->serviceList[i]->ces_service)
			break;
		    j++;
		}

		if (modServices[j] != (int16)record->serviceList[i]->ces_service) {
		    ret = bcm_esw_ces_service_create(unit, 
						     BCM_CES_TDM_UPDATE_WITH_ID, 
						     NULL,
						     &record->serviceList[i]->ces_service);
		    if (ret != BCM_E_NONE)
			return ret;

		    modServices[j] = (int16)record->serviceList[i]->ces_service;
		}
	    } 
	}
    }


    /*
     * Set configured (will clear modified)
     */
    BCM_CES_SET_CONFIGURED(record);
    return BCM_E_NONE; 
}

/**
 * Function:
 *      bcm_esw_port_tdm_config_get
 * Purpose:
 *      Get TDM port attributes.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      config - (IN) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_config_get(
    int unit, 
    bcm_port_t tdm_port, 
    bcm_tdm_port_config_t *config)
{
    bcm_tdm_port_record_t *record;

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    sal_memcpy(config, &record->config, sizeof(bcm_tdm_port_config_t));
    return BCM_E_NONE; 
}

/**
 * Function:
 *      bcm_esw_port_tdm_cas_status_get
 * Purpose:
 *      Get TDM port CAS change bits.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      cas_changed - (OUT) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_cas_status_get(
    int unit, 
    bcm_port_t tdm_port, 
    uint32 *cas_changed)
{
    AgResult n_ret;
    bcm_tdm_port_record_t *record;
    AgNdMsgStatusCasChange ndStatusCasChange;
    bcm_ces_service_global_config_t *ctrl = SOC_CONTROL(unit)->ces_ctrl;

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    ndStatusCasChange.n_circuit_id = bcm_ces_port_to_circuit_id(unit, record->port);

    n_ret = ag_nd_device_read(ctrl->ndHandle, AG_ND_OPCODE_STATUS_CAS_CHANGE, &ndStatusCasChange);
    if (!AG_SUCCEEDED(n_ret))
    {
	return BCM_E_INTERNAL;
    }

    *cas_changed = ndStatusCasChange.n_changed;
    return BCM_E_NONE; 
}

/**
 * Function:
 *      bcm_esw_port_tdm_cas_abcd_get
 * Purpose:
 *      Get TDM port CAS ABCD bits.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      status - (OUT) Ingress or Egress
 *      cas_abcd - (OUT) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_cas_abcd_get(
    int unit, 
    bcm_port_t tdm_port, 
    bcm_tdm_port_cas_status_t status, 
    uint8 *cas_abcd)
{
    AgResult n_ret;
    bcm_tdm_port_record_t *record;
    AgNdMsgStatusCasData ndStatusCasData;
    bcm_ces_service_global_config_t *ctrl = SOC_CONTROL(unit)->ces_ctrl;

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    sal_memset(&ndStatusCasData, 0, sizeof(AgNdMsgStatusCasData));
    ndStatusCasData.n_circuit_id = bcm_ces_port_to_circuit_id(unit, record->port);
    ndStatusCasData.e_path = status;

    n_ret = ag_nd_device_read(ctrl->ndHandle, AG_ND_OPCODE_STATUS_CAS_DATA, &ndStatusCasData);
    if (!AG_SUCCEEDED(n_ret))
    {
	return BCM_E_INTERNAL;
    }

    sal_memcpy(cas_abcd, ndStatusCasData.a_abcd, BCM_CES_SLOT_MAX);
  
    return BCM_E_NONE; 
}

/**
 * Function:
 *      bcm_esw_port_tdm_ces_ports_get
 * Purpose:
 *      Get associated CES ports.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      n_ports - (OUT) <UNDEF>
 *      ces_ports - (OUT) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_ces_ports_get(
    int unit, 
    bcm_port_t tdm_port, 
    uint32     *n_ports,
    uint32     ces_ports[BCM_CES_SLOT_MAX])
{
    bcm_tdm_port_record_t *record;
    int i;

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    /*
     * Return a pointer to the ces services
     */
    *n_ports = 0;
    for (i = 0;i < BCM_CES_SLOT_MAX;i++)
    {
	if (record->serviceList[i] != NULL)
	{
	    ces_ports[*n_ports] = record->serviceList[i]->ces_service;
	    *n_ports += 1;
	}
    }

    return BCM_E_NONE;
}


/**
 * Function:
 *      bcm_esw_port_tdm_framer_port_loopback_set
 * Purpose:
 *      Configures framer port loopback setting.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      enable - Enable or disable loopback
 *      type - Loopback type
 *      slot_mask - Mask of slots to be looped
 *      activation_code - 
 *      deactivation_code - 
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_framer_port_loopback_set(
    int unit, 
    bcm_port_t tdm_port, 
    int enable,
    int type,
    uint32 slot_mask,
    int activation_code,
    int deactivation_code)
{
    AgResult n_ret;
    bcm_tdm_port_record_t *record;
    AgFramerPortLoopbackControl ndLoopbackControl;
    bcm_ces_service_global_config_t *ctrl = SOC_CONTROL(unit)->ces_ctrl;

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    ndLoopbackControl.n_circuit_id = bcm_ces_port_to_circuit_id(unit, record->port);
    ndLoopbackControl.b_loop_enable = enable;
    ndLoopbackControl.e_type = type;
    ndLoopbackControl.n_mask = slot_mask;
    ndLoopbackControl.b_loopback_activation_code = activation_code;
    ndLoopbackControl.b_loopback_deactivation_code = deactivation_code;

    n_ret = ag_nd_device_write(ctrl->ndHandle, AG_ND_OPCODE_FRAMER_PORT_LOOPBACK_CONTROL, &ndLoopbackControl);
    if (!AG_SUCCEEDED(n_ret))
    {
	return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}

/**
 * Function:
 *      bcm_esw_port_tdm_framer_port_status
 * Purpose:
 *      Get the framer port status.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      ndFramerPortStatus - (OUT)
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_framer_port_status(int unit,
				      bcm_port_t tdm_port, 
				      AgFramerPortStatus *ndFramerPortStatus)
{
    AgResult n_ret;
    bcm_tdm_port_record_t *record;
    bcm_ces_service_global_config_t *ctrl = SOC_CONTROL(unit)->ces_ctrl;

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    ndFramerPortStatus->n_circuit_id = bcm_ces_port_to_circuit_id(unit, record->port);
    n_ret = ag_nd_device_read(ctrl->ndHandle, AG_ND_OPCODE_FRMER_PORT_STATUS, ndFramerPortStatus);
    if (!AG_SUCCEEDED(n_ret))
    {
	return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}

/**
 * Function:
 *      bcm_esw_port_tdm_framer_port_pm
 * Purpose:
 *      Get the framer port pm counters.
 * Parameters:
 *      unit - (IN) Unit number.
 *      tdm_port - (IN) <UNDEF>
 *      ndFramerPortPm - (OUT)
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_esw_port_tdm_framer_port_pm(int unit,
				      bcm_port_t tdm_port, 
				      AgFramerPortPm *ndFramerPortPm)
{
    AgResult n_ret;
    bcm_tdm_port_record_t *record;
    bcm_ces_service_global_config_t *ctrl = SOC_CONTROL(unit)->ces_ctrl;

    /*
     * Find record.
     */
    if ((record = bcm_esw_port_tdm_find_port(unit, tdm_port)) == NULL) {
	return BCM_E_PORT;
    }

    ndFramerPortPm->n_circuit_id = bcm_ces_port_to_circuit_id(unit, record->port);
    n_ret = ag_nd_device_read(ctrl->ndHandle, AG_ND_OPCODE_FRAMER_PORT_PM_READ, ndFramerPortPm);
    if (!AG_SUCCEEDED(n_ret))
    {
	return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}

#endif /* INCLUDE_CES */
