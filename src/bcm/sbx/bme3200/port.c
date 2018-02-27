/*
 * $Id: port.c 1.7 Broadcom SDK $
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
 * BME3200 Port API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pt_auto.h>
#include <soc/sbx/bme3200.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/link.h>

#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/state.h>

#define BM3200_PORT_CHECK(unit, port)           \
  do {                                          \
      if ((!SOC_PBMP_PORT_VALID(port)) ||       \
          (!(IS_SFI_PORT(unit, port) ||         \
             IS_SCI_PORT(unit, port)))) {       \
            return BCM_E_PARAM;                 \
      }                                         \
  } while(0)


int
bcm_bm3200_port_init(int unit)
{
    return BCM_E_NONE;
}

int
bcm_bm3200_port_enable_set(int unit, bcm_port_t port, int enable)
{
    BM3200_PORT_CHECK(unit, port);

    if (enable) {
        SAND_HAL_RMW_FIELD_STRIDE(unit, PT, IF, SOC_PORT_BLOCK_INDEX(unit, port),
				  IF0_SI_DEBUG1,
                                  FORCE_SERDES_TX_LOW, 0);
    } else {
        SAND_HAL_RMW_FIELD_STRIDE(unit, PT, IF, SOC_PORT_BLOCK_INDEX(unit, port),
				  IF0_SI_DEBUG1,
                                  FORCE_SERDES_TX_LOW, 1);
    }

    return BCM_E_NONE;
}

int
bcm_bm3200_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    uint32 val;

    BM3200_PORT_CHECK(unit, port);

    /* Determine whether link transmitter is enabled */
    val = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, SOC_PORT_BLOCK_INDEX(unit, port), IF0_SI_DEBUG1);

    if (SAND_HAL_GET_FIELD(PT, IF0_SI_DEBUG1, FORCE_SERDES_TX_LOW, val)) {
        *enable = FALSE;
    } else {
        *enable = TRUE;
    }

    return BCM_E_NONE;
}

int
bcm_bm3200_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_bm3200_port_speed_get(int unit, bcm_port_t port, int *speed)
{
    int rv = BCM_E_UNAVAIL;

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
 	if (SOC_SBX_CFG_BM3200(unit)->bSv2_5GbpsLinks) {
	    *speed = 2500;         /* 2.5Gbps link   */
	} else {
	    *speed = 3125;         /* 3.125Gbps link */
	}
        rv = BCM_E_NONE;
    }

    return rv;
}

int
bcm_bm3200_port_link_status_get(int unit, bcm_port_t port, int *up)
{
    uint32 status = 0;
    int    timeAligned         = FALSE;
    int    byteAligned         = FALSE;

    BM3200_PORT_CHECK(unit, port);

    *up = FALSE;

    status = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF,
				  SOC_PORT_BLOCK_INDEX(unit, port), IF0_SI_STATUS);

    if (SAND_HAL_GET_FIELD(PT, IF0_SI_STATUS, TIME_ALIGNED, status) == 1) {
        timeAligned = TRUE;
    }

    if (SAND_HAL_GET_FIELD(PT, IF0_SI_STATUS, BYTE_ALIGNED, status) == 1) {
        byteAligned = TRUE;
    }

    if (timeAligned && byteAligned) {
        *up = TRUE;
    }

    return BCM_E_NONE;
}

int
bcm_bm3200_port_loopback_set(int unit, bcm_port_t port, int loopback)
{
    uint32 uData;

    BM3200_PORT_CHECK(unit, port);

    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF,
				 SOC_PORT_BLOCK_INDEX(unit, port), IF0_SI_DEBUG0);

    uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_DEBUG0, LPBK_EN, uData,
			       (loopback!=BCM_PORT_LOOPBACK_NONE)?1:0 );

    SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF,
			  SOC_PORT_BLOCK_INDEX(unit, port), IF0_SI_DEBUG0, uData);

    return BCM_E_NONE;
}

int
bcm_bm3200_port_loopback_get(int unit, bcm_port_t port, int *loopback)
{
    uint32 uData;

    BM3200_PORT_CHECK(unit, port);

    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF,
				 SOC_PORT_BLOCK_INDEX(unit, port), IF0_SI_DEBUG0);
    *loopback = SAND_HAL_GET_FIELD(PT, IF0_SI_DEBUG0, LPBK_EN, uData);

    return BCM_E_NONE;
}

int
bcm_bm3200_port_control_set(int unit, bcm_port_t port,
                            bcm_port_control_t type, int value)
{
    uint32 uData;
    int nLink;

    BM3200_PORT_CHECK(unit, port);

    nLink = SOC_PORT_BLOCK_INDEX(unit, port);
    switch (type) {
        case bcmPortControlPrbsPolynomial:
            if ((value != BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1) &&
                (value != BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1)) {
                return BCM_E_PARAM;
            }
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG0, PRBS_POLY_SEL, uData, value);
            SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0, uData);
            break;
        case bcmPortControlPrbsTxInvertData:
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG0, PRBS_INVERT, uData, (value)?1:0);
            SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0, uData);
            break;
        case bcmPortControlPrbsForceTxError:
	    /* Read the IF0_SI_PRBS_STATUS register will clear the error count.
	       We retrieve the state from the stored software state. This is
	       Assuming all other fields are read only
	    */
	    uData = 0;
            uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_PRBS_STATUS, FORCE_ERROR, uData, (value)?1:0);
            SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_PRBS_STATUS, uData);
	    /* store the force tx state */
	    SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[nLink] = (value)?1:0;

            break;
        case bcmPortControlPrbsTxEnable:
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG0, PRBS_GENERATOR_EN, uData, (value)?1:0);
            SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0, uData);
            break;
        case bcmPortControlPrbsRxEnable:
            /* Read to clear prbs_err_cnt */
            SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_PRBS_STATUS);
            if (value) {
                /* Enable PRBS, disable normal rx path */
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
                uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG0, ENABLE, uData, 0 /* disable */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0, uData);

                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
                uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG0, PRBS_MONITOR_EN, uData, 1 /* enable */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0, uData);
            }
            else {
                /* Disable PRBS, enable normal rx path */
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
                uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG0, PRBS_MONITOR_EN, uData, 0 /* disable */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0, uData);

                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
                uData = SAND_HAL_MOD_FIELD(PT, IF0_SI_CONFIG0, ENABLE, uData, 1 /* enable */);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0, uData);
            }
            break;
        case bcmPortControlSerdesDriverStrength:
            if ( (value > 100) || (value < 0)) {
                return BCM_E_PARAM;
            }
            SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[nLink].uDriverStrength = value;
            soc_bm3200_config_linkdriver(unit, nLink, &(SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[nLink]));
            break;
        case bcmPortControlSerdesDriverEqualization:
            if ( (value > 100) || (value < 0)) {
                return BCM_E_PARAM;
            }
            SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[nLink].uDriverEqualization = value;
            soc_bm3200_config_linkdriver(unit, nLink, &(SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[nLink]));
            break;
        case bcmPortControlAbility:
            /* Not supported by BM3200 */
            return BCM_E_UNAVAIL;
        default:
            return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

int
bcm_bm3200_port_control_get(int unit, bcm_port_t port,
                            bcm_port_control_t type, int *value)
{
    uint32 uData;
    int nLink;

    BM3200_PORT_CHECK(unit, port);

    nLink = SOC_PORT_BLOCK_INDEX(unit, port);
    switch (type) {
        case bcmPortControlPrbsPolynomial:
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
            *value = SAND_HAL_GET_FIELD(PT, IF0_SI_CONFIG0, PRBS_POLY_SEL, uData);
            break;
        case bcmPortControlPrbsTxInvertData:
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
            *value = SAND_HAL_GET_FIELD(PT, IF0_SI_CONFIG0, PRBS_INVERT, uData);
            break;
        case bcmPortControlPrbsForceTxError:
	    /* Read the IF0_SI_PRBS_STATUS register will clear the error count.
	       We retrieve the state from the stored software state
	       uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_PRBS_STATUS);
	       *value = SAND_HAL_GET_FIELD(PT, IF0_SI_PRBS_STATUS, FORCE_ERROR, uData);
	     */
	    *value = SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[nLink];
            break;
        case bcmPortControlPrbsTxEnable:
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
            *value = SAND_HAL_GET_FIELD(PT, IF0_SI_CONFIG0, PRBS_GENERATOR_EN, uData);
            break;
        case bcmPortControlPrbsRxEnable:
            uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_CONFIG0);
            *value = SAND_HAL_GET_FIELD(PT, IF0_SI_CONFIG0, PRBS_MONITOR_EN, uData);
            break;
	case bcmPortControlPrbsRxStatus:
	    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, PT, IF, nLink, IF0_SI_PRBS_STATUS);
	    *value = SAND_HAL_GET_FIELD(PT, IF0_SI_PRBS_STATUS, PRBS_ERR_CNT, uData);
	    break;
	case bcmPortControlSerdesDriverStrength:
            *value = SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[nLink].uDriverStrength;
            break;
        case bcmPortControlSerdesDriverEqualization:
            *value = SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[nLink].uDriverEqualization;
            break;
        case bcmPortControlAbility:
            /* Not supported by BM3200 */
            return BCM_E_UNAVAIL;
        default:
            return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}
