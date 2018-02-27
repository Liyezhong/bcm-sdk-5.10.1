/* $Id: stat.c 1.9.112.1 Broadcom SDK $
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
 * Broadcom QE2000 Statistics API.
 */

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>

#include <soc/sbx/counter.h>
#include <soc/sbx/qe2000_counter.h>
#include <soc/sbx/qe2000_scoreboard.h>

#include <bcm/error.h>
#include <bcm/stat.h>
#include <bcm/debug.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stat.h>


#define BCMSIM_STAT_INTERVAL            25000000

static int bcm_qe2000_stat_custom_compute(int unit,
					  bcm_port_t port,
					  bcm_stat_val_t type,
					  uint64 *count);
static int  qe2000_counter_enable_flag;


/*
 * Function:
 *     _bcm_qe2000_sw_counter_init
 * Description:
 *     Initialize and start the counter collection, software
 *     accumulation process, on given unit.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_qe2000_sw_counter_init(int unit)
{
    pbmp_t       pbmp;
    sal_usecs_t  interval;
    uint32       flags;
     
    SOC_PBMP_CLEAR(pbmp);

    interval = (SAL_BOOT_BCMSIM) ? BCMSIM_STAT_INTERVAL : 1000000;
    interval = soc_property_get(unit, spn_BCM_STAT_INTERVAL, interval);

    flags = soc_property_get(unit, spn_BCM_STAT_FLAGS, 0);

    SOC_IF_ERROR_RETURN(soc_sbx_qe2000_counter_init(unit, flags, interval, pbmp));

    qe2000_counter_enable_flag = 0;

    return BCM_E_NONE;

}




int
bcm_qe2000_stat_init(int unit)
{
    bcm_stat_info_t            *si = NULL;
    int                         result = BCM_E_NONE;

    BCM_STAT_VVERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || unit >= BCM_MAX_NUM_UNITS) {
        return BCM_E_UNIT;
    }

    si = &STAT_CNTL(unit);

    si->init = TRUE;

    result = _bcm_qe2000_sw_counter_init(unit);


    BCM_STAT_VERB(("bcm_stat_init: unit=%d rv=%d(%s)\n",
                unit, result, bcm_errmsg(result)));

    return result;
}


int
bcm_qe2000_stat_sync(int unit)
{
    return soc_sbx_counter_sync(unit);
}

/*
 * Function:
 *	bcm_stat_get
 * Description:
 *	Get the specified statistic
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - zero-based port number
 *	type - SNMP statistics type (see stat.h)
 * Returns:
 *	BCM_E_NONE - Success.
 *	BCM_E_PARAM - Illegal parameter.
 *	BCM_E_INTERNAL - Chip access failure.
 */

int
bcm_qe2000_stat_get(int unit,
                 bcm_port_t port,
                 bcm_stat_val_t type,
                 uint64 *val)
{
  uint64 count;
  int rv = BCM_E_NONE;

  if (!SOC_PORT_VALID(unit, port)) {
      return BCM_E_PORT;
  }

    COMPILER_REFERENCE(&count);  /* Work around PPC compiler bug */
    COMPILER_64_ZERO(count);

    switch (type) {
    case snmpBcmCustomReceive0:
    case snmpBcmCustomReceive1:
    case snmpBcmCustomReceive2:
    case snmpBcmCustomReceive3:
    case snmpBcmCustomReceive4:
    case snmpBcmCustomReceive5:
    case snmpBcmCustomReceive6:
    case snmpBcmCustomReceive7:
    case snmpBcmCustomReceive8:
	rv = bcm_qe2000_stat_custom_compute(unit, port, type, &count);
        break;
    default:
	BCM_DEBUG(BCM_DBG_WARN+BCM_DBG_VERBOSE,
		  ("bcm_stat_get: Statistic not supported: %d\n", type));
	return BCM_E_PARAM;
    }

    *val = count;

    return rv;
}

int
bcm_qe2000_stat_get32(int unit,
                   bcm_port_t port,
                   bcm_stat_val_t type,
                   uint32 *val)
{
    int     rv = BCM_E_NONE;
    uint64  val64;

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    rv = bcm_qe2000_stat_get(unit, port, type, &val64);

    COMPILER_64_TO_32_LO(*val, val64);

    return(rv);
}

#define ALLPORTS                 (SB_FAB_DEVICE_QE2000_SFI_LINKS + SB_FAB_DEVICE_QE2000_SCI_LINKS)

int 
bcm_qe2000_stat_clear(int unit, bcm_port_t port)
{
    soc_sbx_config_t *sbx = SOC_SBX_CFG(unit);
    uint32 *bp = NULL;
    int base = 0;

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_PORT_VALID(unit, port)){
        return BCM_E_PORT;
    }

    /*  
     * If SFI or SCI, then clear custom stats
     */

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
	for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
	    bp = (uint32 *) &sbx->custom_stats[base * ALLPORTS];
	    bp[port - SOC_PORT_MIN(unit, sfi)] = 0;
	}
    }
    
    return(BCM_E_NONE);
}

/*
 * Function:
 *      bcm_qe2000_stat_custom_set 
 * Description:
 *      No h/w register support for custom set.
 *      Handled in the software.
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number
 *      type  - SNMP statistics type.
 *      flags - The counter select value (see stat.h for bit definitions). 
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_qe2000_stat_custom_set(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        uint32 flags)
{
    int rv = BCM_E_UNAVAIL;

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    /*  valid port  */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    return rv;
}

int
bcm_qe2000_stat_custom_get(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        uint32 *flags)
{
    int rv = BCM_E_UNAVAIL;

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (flags == NULL) {
	return BCM_E_RESOURCE;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }


    /* valid port */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    return rv;
}

int 
bcm_qe2000_stat_custom_add(int unit,
                        bcm_port_t port,
                        bcm_stat_val_t type,
                        bcm_custom_stat_trigger_t trigger)
{
     int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx = SOC_SBX_CFG(unit);
    int	base = 0;

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    if ((trigger < bcmDbgCntRX_SYMBOL) || (trigger > bcmDbgCntBIT_INTERLEAVED_PARITY_ODD)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;
    base = trigger - bcmDbgCntRX_SYMBOL;

    if (port == -1) {
	SHR_BITSET_RANGE(sbx->soc_sbx_dbg_cntr_rx[type].ports, base * SHR_BITWID, ALLPORTS);
	sbx->soc_sbx_dbg_cntr_rx[type].trigger |= (1 << base);
    } else {
	sbx->soc_sbx_dbg_cntr_rx[type].trigger |= (1 << base);
	SHR_BITSET(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port));
    }
    
    return rv;
}

int 
bcm_qe2000_stat_custom_delete(int unit,
                           bcm_port_t port,
                           bcm_stat_val_t type, 
                           bcm_custom_stat_trigger_t trigger)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    int	base = 0;

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    if ((trigger < bcmDbgCntRX_SYMBOL) || (trigger > bcmDbgCntBIT_INTERLEAVED_PARITY_ODD)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;
    base = trigger - bcmDbgCntRX_SYMBOL;

    if (port == -1) {
	SHR_BITCLR_RANGE(sbx->soc_sbx_dbg_cntr_rx[type].ports, base * SHR_BITWID, ALLPORTS);
	sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
    } else {
	SHR_BITCLR(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port));
	if (sbx->soc_sbx_dbg_cntr_rx[type].ports[base] == 0)
	    sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
    }
    
    return rv;
}

int 
bcm_qe2000_stat_custom_delete_all(int unit,
                               bcm_port_t port,
                               bcm_stat_val_t type)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    int	base = 0;

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port) && (port != -1)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;

    for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
	if (port == -1) {
	    SHR_BITCLR_RANGE(sbx->soc_sbx_dbg_cntr_rx[type].ports, base * SHR_BITWID, ALLPORTS);
	    sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
	} else {
	    SHR_BITCLR(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port));
		    if (sbx->soc_sbx_dbg_cntr_rx[type].ports[base] == 0)
			sbx->soc_sbx_dbg_cntr_rx[type].trigger &= ~(1<<base);
	}
    }
    
    return rv;
}

int 
bcm_qe2000_stat_custom_check(int unit,
			     bcm_port_t port,
			     bcm_stat_val_t type, 
			     bcm_custom_stat_trigger_t trigger,
			     int *result)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    int	base = 0;
   
    if (result == NULL) {
	return BCM_E_RESOURCE;
    }

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if ((type < snmpBcmCustomReceive0) || (type > snmpBcmCustomReceive8)) {
        return BCM_E_PARAM;
    }

    if ((trigger < bcmDbgCntRX_SYMBOL) || (trigger > bcmDbgCntBIT_INTERLEAVED_PARITY_ODD)) {
        return BCM_E_PARAM;
    }

    /* port = -1 is valid port in that case */
    if (!IS_SFI_PORT(unit, port) && !IS_SCI_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    /* Decrement base SFI port to 0 base */
    if (port > 0)
	port -= SOC_PORT_MIN(unit, sfi);

    type -= snmpBcmCustomReceive0;
    base = trigger - bcmDbgCntRX_SYMBOL;

    if (SHR_BITGET(sbx->soc_sbx_dbg_cntr_rx[type].ports, ((base * SHR_BITWID) + port))) {
	*result = 0;
    } else {
	*result = -1;
    }

    return rv;
}

/*
 * Function:
 *      bcm_qe2000_stat_custom_compute
 * Description:
 *      Calculate debug counter value.
 * Parameters:
 *      unit  - StrataSwitch PCI device unit number.
 *      port  - Port number
 *      type  - SNMP statistics type.
 *      flags - (OUT) The counter select value
 *		      (see stat.h for bit definitions).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_qe2000_stat_custom_compute(int unit,
			       bcm_port_t port,
			       bcm_stat_val_t type,
			       uint64 *count)
{
    int rv = BCM_E_NONE;
    soc_sbx_config_t *sbx  = SOC_SBX_CFG(unit);
    uint32 *bp = NULL;
    int base = 0, ports = 0, offset = 0, begin = 0, end = 0;
    uint32 value = 0;

    if (!SOC_IS_SBX_QE2000(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (count == NULL) {
	return BCM_E_RESOURCE;
    }

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
	begin = port - SOC_PORT_MIN(unit, sfi);
	end = begin + 1;
    } else {
	return BCM_E_PORT;
    }

    offset = type - snmpBcmCustomReceive0;

    for (base=0; base <= (bcmDbgCntBIT_INTERLEAVED_PARITY_ODD - bcmDbgCntRX_SYMBOL); base++) {
	if ((sbx->soc_sbx_dbg_cntr_rx[offset].trigger & (1<<base)) == 0) {
	    continue;
	}
	bp = (uint32 *) &sbx->custom_stats[base * ALLPORTS];
	for (ports = begin; ports < end; ports++) {
	    if (SHR_BITGET(sbx->soc_sbx_dbg_cntr_rx[offset].ports, ((base * SHR_BITWID) + ports))) {
		value += bp[ports];
	    }
	}
    }

    COMPILER_64_ADD_32(*count, value);

    return rv;
}

int
bcm_qe2000_stat_scoreboard_get(int unit,
                    uint32 *pBufLost,
                    uint32 *pBufFrees,
                    uint32 *pWatchdogErrs,
                    uint32 *pShortIntervals,
                    uint32 *pScoreboardTicks)

{

      return (soc_qe2000_scoreboard_stats_get(unit, pBufLost, pBufFrees, pWatchdogErrs, pShortIntervals, pScoreboardTicks));

}
