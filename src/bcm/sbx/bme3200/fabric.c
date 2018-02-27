/*
 * $Id: fabric.c 1.16 Broadcom SDK $
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
 * BM3200 Fabric Control API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pt_auto.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm3200_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/cosq.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/fabric.h>


#define BM3200_NS_PER_CLOCK (4) /* 250MHz clock, 4 ns, per clock */


int
bcm_bm3200_fabric_crossbar_connection_set(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t dst_xbport)
{
    return BCM_E_UNAVAIL;
}

int
bcm_bm3200_fabric_crossbar_connection_get(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport)
{
    return BCM_E_UNAVAIL;
}

int
bcm_bm3200_fabric_crossbar_mapping_set(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_bm3200_fabric_crossbar_mapping_get(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t *port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_bm3200_fabric_crossbar_enable_set(int unit,
                                      uint64 xbars)
{
    uint32 i;
    uint32 uData, uTmp;
    int32  nTsSizeNormNs, nTsSizeDegrNs;
    int32  nTsSizeNormClocks, nTsSizeDegrClocks;
    int32  nSfiCount;
    int32  nNewTsLarger;
    int32  nHalfBus;
    int32  nMaxFailedLinks;
    int32  nOldTsSizeNormNs;
    int32  bw_group;
    int32_t num_sp_queues, num_queues_in_bag, base_queue, bag_rate_bytes_per_epoch;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate_start;
    int32  nOldDemandScale, nDemandScale;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int    gamma = 0, sigma = 0;
    int32  queue;
#ifndef __KERNEL__
    int64  uuTmp;
#endif
    int32  delayUs;
    sal_usecs_t stime;
    int    rv = BCM_E_NONE;

    nHalfBus = SOC_SBX_CFG(unit)->bHalfBus;
    nMaxFailedLinks = SOC_SBX_CFG(unit)->uMaxFailedLinks;

    /* set the link enables on the BME */
    nSfiCount = 0;
    for (i=0; i<64; i++) {
        /* Count number of enabled links */
        nSfiCount += ((xbars >> i) & 0x1);
    }
    if (nSfiCount < nMaxFailedLinks) {
	BCM_WARN(("WARNING: the number of SFI enabled(%d) is less than the maximum number of failed links configured(%d)\n",
		 nSfiCount, nMaxFailedLinks));

        if (nSfiCount > 0 ) {
	    nMaxFailedLinks = nSfiCount - 1;
	}else {
	    nMaxFailedLinks = 0;
	}

	BCM_WARN(("WARNING: override will write maxFailedLinks(%d) xbars(%d)\n", nMaxFailedLinks, nSfiCount));
    }

    SAND_HAL_RMW_FIELD(unit, PT, FO_CONFIG0, MAX_DIS_LINKS, nMaxFailedLinks);

    nTsSizeNormNs = soc_sbx_fabric_get_timeslot_size(unit, nSfiCount, nHalfBus, soc_feature(unit, soc_feature_hybrid) );
    nTsSizeDegrNs = soc_sbx_fabric_get_timeslot_size(unit, nSfiCount - nMaxFailedLinks,
						      nHalfBus, soc_feature(unit, soc_feature_hybrid) );

    nOldTsSizeNormNs = SOC_SBX_STATE(unit)->fabric_state->timeslot_size;
    SOC_SBX_STATE(unit)->fabric_state->timeslot_size = nTsSizeNormNs;

    /* The timeslot_size and deg_timeslot_size fields in the ac_config1
     * register are set to one less than the value you wish to be used.
     * See the BM3200 spec
     */
    nTsSizeNormClocks = (nTsSizeNormNs / BM3200_NS_PER_CLOCK) - 1;
    nTsSizeDegrClocks = (nTsSizeDegrNs / BM3200_NS_PER_CLOCK) - 1;

    /* When adjusting the Timeslot size, the following must be done:
     * 1. Force the arbiter to generate null grants to avoid drops
     * 2. if the TS gets smaller, update grant offset first
     * 3. update timeslot sizes (normal & degraded)
     * 4. if TS get larger, update grant offset last
     * 5. update link_en last
     * 6. disable force null grants
     */

    stime = sal_time_usecs();
    SAND_HAL_RMW_FIELD(unit, PT, AC_CONFIG0, FORCE_NULL_GRANT, 1);

    /* if TS size decreases, update the Grant offset first */
    uData = SAND_HAL_READ(unit, PT, AC_CONFIG1);
    if (SAND_HAL_GET_FIELD(PT, AC_CONFIG1, TIMESLOT_SIZE, uData) >
        nTsSizeNormClocks)
    {
        nNewTsLarger = 0;

        uTmp = SAND_HAL_READ(unit, PT, AC_CONFIG2);
        uTmp = SAND_HAL_MOD_FIELD(PT, AC_CONFIG2, SEND_GRANT_OFFSET,
                                  uTmp, nTsSizeNormClocks - 0xf);
        SAND_HAL_WRITE(unit, PT, AC_CONFIG2, uTmp);
    } else {
        nNewTsLarger = 1;
    }

    /* update the normal and degraded timeslot sizes */
    uData = SAND_HAL_MOD_FIELD(PT, AC_CONFIG1, TIMESLOT_SIZE, uData, nTsSizeNormClocks);
    uData = SAND_HAL_MOD_FIELD(PT, AC_CONFIG1, DEG_TIMESLOT_SIZE, uData, nTsSizeDegrClocks);
    SAND_HAL_WRITE(unit, PT, AC_CONFIG1, uData);

    /* if TS size increases, update the Grant offset last */
    if ( nNewTsLarger ) {
        uTmp = SAND_HAL_READ(unit, PT, AC_CONFIG2);
        uTmp = SAND_HAL_MOD_FIELD(PT, AC_CONFIG2, SEND_GRANT_OFFSET, uTmp, nTsSizeNormClocks - 0xf);
        SAND_HAL_WRITE(unit, PT, AC_CONFIG2, uTmp);
    }

    /* configure the fabric data plane */
    SAND_HAL_RMW_FIELD(unit, PT, FO_CONFIG0, LINK_EN, xbars);

    /*
     * The above configuration (link enable) results in also forcing
     * NULL Grants in case gracefull degradation is enabled.
     * Add delay so that the state machine is not effected by disabling
     * of NULL Grants.
     *
     * NOTE: Currently this is commented as the state machine is not effected.
     *   
     */
    delayUs = (nOldTsSizeNormNs / 1000) + ((nOldTsSizeNormNs % 1000) ? 1 : 0);
    delayUs = (delayUs == 0) ? 1 : delayUs;
    delayUs = (delayUs > SB_FAB_DEVICE_BM3200_MAX_TS) ? SB_FAB_DEVICE_BM3200_MAX_TS : delayUs;
    delayUs = delayUs * HW_BM3200_NULL_CYCLE_COUNT;
#if 0
    sal_udelay(delayUs);
#endif /* 0 */

    /* Done: re-enable the BME */
    SAND_HAL_RMW_FIELD(unit, PT, AC_CONFIG0, FORCE_NULL_GRANT, 0);

    BCM_DEBUG(BCM_DBG_VERBOSE, (" bcm_bm3200_fabric_crossbar_enable_set: took %d usec\n",
                                              SAL_USECS_SUB(sal_time_usecs(), stime)));
    /*
     * Do Not re-configure the bags in the critical region.
     * As this configuration occurs rates will converge.
     */

    /* reconfigure bags */
    nOldDemandScale = SOC_SBX_STATE(unit)->fabric_state->old_demand_scale;
    nDemandScale = SOC_SBX_CFG(unit)->demand_scale;

    if ( (nOldTsSizeNormNs != nTsSizeNormNs) || (nOldDemandScale != nDemandScale) ) {
	/* go through all bags and reconfig all non-zero bag rate */
	p_bwstate_start = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
	
	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {
	    p_bwstate = &p_bwstate_start[bw_group];
	    
	    if ( ( p_bwstate->in_use == FALSE ) || (p_bwstate->path.bag_rate_kbps == 0) ) {
		continue;
	    }
	    
	    rv = soc_bm3200_prt_read(unit, bw_group, &num_sp_queues, &num_queues_in_bag,
				     &base_queue, &bag_rate_bytes_per_epoch);
	    
	    if (rv != SOC_E_NONE) {
		BCM_ERR(("ERROR: BM3200 read of PRT table failed for bw_group(%d)\n", bw_group));
		return BCM_E_FAIL;
	    }
	    
	    if (bag_rate_bytes_per_epoch == 0) {
		continue;
	    }
#ifndef __KERNEL__
	    uuTmp = bag_rate_bytes_per_epoch * nTsSizeNormNs;
	    uuTmp /= nOldTsSizeNormNs;
	    uuTmp >>= (nDemandScale - nOldDemandScale);
	    bag_rate_bytes_per_epoch = (uint32_t)uuTmp;
#endif
	    rv = soc_bm3200_prt_write(unit, bw_group, num_sp_queues, num_queues_in_bag,
				      base_queue, bag_rate_bytes_per_epoch);
	    
	    if (rv != SOC_E_NONE) {
		BCM_ERR(("ERROR: BM3200 Write to PRT table failed for bw_group(%d)\n", bw_group));
		return BCM_E_FAIL;
	    }
	}
	
	/* go through all queue indexed BWP table and update non-zero sigma */
	p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
	for (queue = 0; queue < SOC_SBX_CFG(unit)->num_queues; queue++) {
	    if ( (p_qstate[queue].state == BCM_INT_SBX_QUEUE_STATE_IN_USE) &&
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_AF) ) {

		rv = soc_bm3200_bwp_read(unit, queue, &gamma, &sigma);
		
		if (rv != SOC_E_NONE) {
		    BCM_ERR(("ERROR: BM3200 Read to BWP table failed for queue(%d)\n", queue));
		    return BCM_E_FAIL;
		}
#ifndef __KERNEL__
		uuTmp = sigma * nTsSizeNormNs;
		uuTmp /= nOldTsSizeNormNs;
		uuTmp >>= (nDemandScale - nOldDemandScale);
		sigma = (uint32_t)uuTmp;
#endif
		rv = soc_bm3200_bwp_write(unit, queue, gamma, sigma);
		
		if (rv != SOC_E_NONE) {
		    BCM_ERR(("ERROR: BM3200 Write to BWP table failed for queue(%d)\n", queue));
		    return BCM_E_FAIL;
		}
		
	    }
	}
    }

    return BCM_E_NONE;
}

int
bcm_bm3200_fabric_crossbar_enable_get(int unit,
                                      uint64 *xbars)
{
    uint32 uData;

    uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
    *xbars = SAND_HAL_GET_FIELD(PT, FO_CONFIG0, LINK_EN, uData);

    return BCM_E_NONE;
}

int
bcm_bm3200_fabric_crossbar_status_get(int unit,
                                      uint64 *xbars)
{
    uint32 uData;

    uData = SAND_HAL_READ(unit, PT, FO_STATUS);
    *xbars = SAND_HAL_GET_FIELD(PT, FO_STATUS, ENABLED_LINKS, uData);

    return BCM_E_NONE;
}

int
bcm_bm3200_fabric_distribution_create(int unit,
                                      bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_UNAVAIL;


    return rv;
}

int
bcm_bm3200_fabric_distribution_destroy(int unit,
                                       bcm_fabric_distribution_t  ds_id)
{
    int rv = BCM_E_UNAVAIL;

    rv = soc_bm3200_eset_set(unit, ds_id, 0, 0);
    return rv;
}

int
bcm_bm3200_fabric_distribution_set(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int modid_count,
                                   int *dist_modids,
				   int  mc_full_eval_min)
{
    int rv = BCM_E_UNAVAIL;
    int i;
    uint32 eset_value;


    for (i = 0, eset_value = 0; i < modid_count; i++) {
        eset_value |= (1 << BCM_STK_MOD_TO_NODE((*(dist_modids + i))));
    }

    rv = soc_bm3200_eset_set(unit, ds_id, eset_value, (uint32)mc_full_eval_min);

    return rv;
}

int
bcm_bm3200_fabric_distribution_get(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int max_count,
                                   int *dist_modids,
                                   int *count,
				   int *mc_full_eval_min)
{
    int rv = BCM_E_UNAVAIL;
    int node, num_members;
    uint32 eset_value;

    rv = soc_bm3200_eset_get(unit, ds_id, &eset_value, (uint32*)mc_full_eval_min);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    /* It is the BCM layer resonsibility to convert node to modId */
    for (node = 0, num_members = 0; ((node < 32) & (num_members < max_count)); node++) {
        if (eset_value & (1 << node)) {
            *(dist_modids + num_members++) = node;
        }
    }
    *count = num_members;

    return rv;
}

int
bcm_bm3200_fabric_distribution_control_set(int unit,
					   bcm_fabric_distribution_t ds_id,
					   bcm_fabric_distribution_control_t type,
					   int value)
{
    int rv = BCM_E_NONE;
    uint32 eset_value;
    uint32 mc_full_eval_min;

    rv = soc_bm3200_eset_get(unit, ds_id, &eset_value, &mc_full_eval_min);

    if (rv) {
        return rv;
    }
    mc_full_eval_min = value;

    rv = soc_bm3200_eset_set(unit, ds_id, eset_value, mc_full_eval_min);
    if (rv) {
        return rv;
    }

    return rv;
}
int
bcm_bm3200_fabric_distribution_control_get(int unit,
					   bcm_fabric_distribution_t ds_id,
					   bcm_fabric_distribution_control_t type,
					   int *value)
{
    int rv = BCM_E_NONE;
    uint32 eset_value;
    uint32 mc_full_eval_min;

    rv = soc_bm3200_eset_get(unit, ds_id, &eset_value, &mc_full_eval_min);

    if (rv) {
        return rv;
    }
    *value = mc_full_eval_min;

    return rv;
}

int
bcm_bm3200_fabric_control_set(int unit,
                              bcm_fabric_control_t type,
                              int arg)
{
    int bEnableAutoFailover;
    int bEnableAutoLinkDisable;
    int rv = BCM_E_NONE;
    uint32 uData, i;
    int32  nHalfBus;
    int32  nMaxFailedLinks;
    int32  nTsSizeDegrNs;
    int32  nTsSizeDegrClocks;
    int32  nSfiCount;
    int32  xbars;

    switch (type) {
        case bcmFabricArbiterId:
            uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, LOCAL_BM_ID, uData, arg);
            SAND_HAL_WRITE(unit, PT, FO_CONFIG0, uData);
            break;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, DEFAULT_BM_ID, uData, arg);
            SAND_HAL_WRITE(unit, PT, FO_CONFIG0, uData);
            break;
        case bcmFabricArbiterConfig:
            SAND_HAL_RMW_FIELD(unit, PT, AC_CONFIG0, FORCE_NULL_GRANT, (arg == 0));
            break;
        case bcmFabricMaximumFailedLinks:

            /* GNATS 20156, need to set max failed links global var for use in crossbar_enable_set */
            SOC_SBX_CFG(unit)->uMaxFailedLinks = arg;

           /* update the degraded timeslot size */
            nHalfBus = SOC_SBX_CFG(unit)->bHalfBus;
            nMaxFailedLinks = SOC_SBX_CFG(unit)->uMaxFailedLinks;

            /* update the max failed link configuration */
            uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
            xbars = SAND_HAL_GET_FIELD(PT, FO_CONFIG0, LINK_EN, uData);

            nSfiCount = 0;
            for (i=0; i<32; i++) {
                /* Count number of enabled links */
                nSfiCount += ((xbars >> i) & 0x1);
            }

            if (nSfiCount < nMaxFailedLinks) {
                BCM_WARN(("WARNING: the number of SFI enabled(%d) is less than the maximum number of failed links(%d)\n",
                          nSfiCount, nMaxFailedLinks));

                if (nSfiCount > 0) {
                    nMaxFailedLinks = nSfiCount - 1;
                } else {
                    nMaxFailedLinks = 0;
                }
                BCM_WARN(("WARNING: override maxFailedLinks(%d) total xbars(%d)\n", nMaxFailedLinks, nSfiCount));
            }

            /* update the max failed link configuration */
            uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
            xbars = SAND_HAL_GET_FIELD(PT, FO_CONFIG0, LINK_EN, uData);
            uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, MAX_DIS_LINKS, uData, nMaxFailedLinks);
            SAND_HAL_WRITE(unit, PT, FO_CONFIG0, uData);


            nTsSizeDegrNs = soc_sbx_fabric_get_timeslot_size(unit, nSfiCount - nMaxFailedLinks, nHalfBus, soc_feature(unit, soc_feature_hybrid) );
            nTsSizeDegrClocks = (nTsSizeDegrNs / BM3200_NS_PER_CLOCK) - 1;
            uData = SAND_HAL_READ(unit, PT, AC_CONFIG1);
            uData = SAND_HAL_MOD_FIELD(PT, AC_CONFIG1, DEG_TIMESLOT_SIZE, uData, nTsSizeDegrClocks);
            SAND_HAL_WRITE(unit, PT, AC_CONFIG1, uData);
            break;
        case bcmFabricRedundancyMode:
            /* clear interrupts */
            uData = (SAND_HAL_PT_FO_ERROR_AUTO_SWITCHOVER_EVENT_MASK |
                     SAND_HAL_PT_FO_ERROR_MASK_AUTO_QE_DIS_EVENT_DISINT_MASK |
                     SAND_HAL_PT_FO_ERROR_MASK_AUTO_DIS_EVENT_DISINT_MASK);
            SAND_HAL_WRITE(unit, PT, FO_ERROR, uData);

            /* configure redundancy mode */
            bEnableAutoFailover = 0;
            bEnableAutoLinkDisable = 0;
            switch (arg) {
                case bcmFabricRed1Plus1Both:
                    bEnableAutoFailover = 1;
                    break;
                case bcmFabricRed1Plus1LS:
                    bEnableAutoFailover = 1;
                    bEnableAutoLinkDisable = 1;
                    break;
                case bcmFabricRedLS:
                    bEnableAutoLinkDisable = 1;
                    break;
                case bcmFabricRedManual:
                    break;
                case bcmFabricRed1Plus1ELS:
                case bcmFabricRedELS:
                default:
                    soc_cm_debug(DK_ERR, "bcmFabricRedundancyMode %d not supported by bm3200\n", arg);
                    rv = BCM_E_PARAM;
                    break;
            }
            uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, ENABLE_AUTO_SWITCHOVER, uData, bEnableAutoFailover);
            uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, ENABLE_AUTO_LINK_DIS, uData, bEnableAutoLinkDisable);
            uData = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, ENABLE_MORE_LINK_DIS, uData, bEnableAutoLinkDisable);
            SAND_HAL_WRITE(unit, PT, FO_CONFIG0, uData);

            if (bEnableAutoFailover || bEnableAutoLinkDisable){
                /* arm interrupts */
                uData = 0xFFFFFFFF;
                if (bEnableAutoFailover) {
                    uData &= ~(SAND_HAL_PT_FO_ERROR_AUTO_SWITCHOVER_EVENT_MASK);
                }
                if (bEnableAutoLinkDisable) {
                    uData &= ~(SAND_HAL_PT_FO_ERROR_MASK_AUTO_DIS_EVENT_DISINT_MASK);
                    uData &= ~(SAND_HAL_PT_FO_ERROR_MASK_AUTO_QE_DIS_EVENT_DISINT_MASK);
                }
                SAND_HAL_WRITE(unit, PT, FO_ERROR_MASK, uData);
                /* need to unmask top level too */
                SAND_HAL_RMW_FIELD(unit, PT, PI_PT_ERROR0_MASK, FO_INT_DISINT, 0);
            }
            break;
        case bcmFabricMode:
            rv = BCM_E_UNAVAIL;
            break;
        case bcmFabricMcGroupSourceKnockout:
            rv = BCM_E_UNAVAIL;
            break;

        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            rv = BCM_E_UNAVAIL;
            break;

        case bcmFabricQueueMin:
            if (arg) {
                BCM_ERR(("Unit %d: Invalid value specified (%d) for "
                         "bcmFabricQueueMin. Only 0 is supported. \n", unit, arg));
                rv = BCM_E_PARAM;
                break;
            } 
            /* nothing to do. Already set to 0. */
            break;
    
        case bcmFabricQueueMax:
            if ((arg < 0) || (arg > HW_BM3200_PT_MAX_DMODE_QUEUES)) {
                BCM_ERR(("%d: Invalid Queue Max (%d) specified. Valid range: "
                         "0-%d \n", unit, arg, HW_BM3200_PT_MAX_DMODE_QUEUES));
                rv = BCM_E_PARAM;
                break;
            }
            rv = soc_bm3200_epoch_in_timeslot_config_get(unit, arg, &uData);
            if (rv == SOC_E_NONE) {
                SOC_SBX_CFG(unit)->num_queues = arg;
                SOC_SBX_CFG(unit)->epoch_length_in_timeslots = uData;

                /* Adjust the sw state to reflect this new settings */
                rv = _bcm_sbx_cosq_queue_regions_set(unit);
                if (rv != BCM_E_NONE) {
                    BCM_ERR(("%d: Could not set queue_regions to reflect new "
                             "Max VOQs (%d) \n", unit, arg));
                    break;
                }

                /* set the HW config registers */
                uData = SAND_HAL_READ(unit, PT, BW_EPOCH_CONFIG);
                uData = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, NUM_TIMESLOTS, 
                          uData, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
                SAND_HAL_WRITE(unit, PT, BW_EPOCH_CONFIG, uData);
                SAND_HAL_WRITE(unit, PT, BW_GROUPS, 
                               (SOC_SBX_CFG(unit)->num_queues -1));
            } else {
                BCM_ERR(("%d: Could not calculate Epoch length for specified "
                         "bcmFabricQueueMax value (%d) \n", unit, arg));
            }
            break;

        case bcmFabricEgressQueueMin: /* intentional fall thru */
        case bcmFabricEgressQueueMax:
            rv = BCM_E_UNAVAIL;
            break;

        default:
            rv = BCM_E_UNAVAIL;
            break;
    }

    return rv;
}

int
bcm_bm3200_fabric_control_get(int unit,
                              bcm_fabric_control_t type,
                              int *arg)
{
    int     rv = BCM_E_NONE;
    uint32  uData;

    switch (type) {
        case bcmFabricArbiterId:
            uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PT, FO_CONFIG0, LOCAL_BM_ID, uData);
            break;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PT, FO_CONFIG0, DEFAULT_BM_ID, uData);
            break;
        case bcmFabricArbiterConfig:
            uData = SAND_HAL_READ(unit, PT, AC_CONFIG0);
            *arg = (SAND_HAL_GET_FIELD(PT, AC_CONFIG0, FORCE_NULL_GRANT, uData) == 0);
            break;
        case bcmFabricMaximumFailedLinks:
            *arg = SOC_SBX_CFG(unit)->uMaxFailedLinks;
            break;
        case bcmFabricActiveId:
            uData = SAND_HAL_READ(unit, PT, FO_STATUS);
            *arg = SAND_HAL_GET_FIELD(PT, FO_STATUS, ACTIVE_BM, uData);
            break;
        case bcmFabricRedundancyMode:
            /* return cached value */
            *arg = SOC_SBX_CFG(unit)->uRedMode;
            break;
        case bcmFabricMcGroupSourceKnockout:
            rv = BCM_E_UNAVAIL;
            break;

        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            rv = BCM_E_UNAVAIL;
            break;

        case bcmFabricQueueMin:
            *arg = 0; /* always starts at 0 */
            break;
        case bcmFabricQueueMax:
            *arg = SOC_SBX_CFG(unit)->num_queues;
            break;
        case bcmFabricEgressQueueMin: /* intentional fall thru */
        case bcmFabricEgressQueueMax: /* intentional fall thru */
        default:
            rv = BCM_E_UNAVAIL;
            break;
    }

    return rv;
}
