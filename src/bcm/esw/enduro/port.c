/*
 * $Id: port.c 1.4.122.1 Broadcom SDK $
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
 */


#include <soc/defs.h>

#include <assert.h>

#include <sal/core/libc.h>
#if defined(BCM_ENDURO_SUPPORT)

#include <shared/util.h>
#include <soc/mem.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <bcm/error.h>
#include <bcm_int/esw/enduro.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/xgs3.h>

#include <bcm_int/esw_dispatch.h>

/* Helper functions for bcmPortControlLanes 
 * Used for flex-port support
 */
int 
_bcm_en_port_lanes_get(int unit, bcm_port_t port, int *value)
{
    int speed = 0;
    int rv = BCM_E_NONE;
    /*    coverity[new_values]    */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }
    if ((port != 26) && (port != 27) && (port != 28) && (port != 29)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(bcm_esw_port_speed_max(unit, port, &speed));
    
    if (speed >= 10000) {
        *value = 4;
    } else {
        *value = 1;
    }
    return rv;
}

static const uint16 _soc_en_int_phy_addr[] = {
        0x00, /* Port  0 (cmic)         N/A */
        0x00, /* Port  1 (N/A)                */
        0x81, /* Port  2 (8SERDES_0)    IntBus=0 Addr=0x01 */
        0x82, /* Port  3 (8SERDES_0)    IntBus=0 Addr=0x02 */
        0x83, /* Port  4 (8SERDES_0)    IntBus=0 Addr=0x03 */
        0x84, /* Port  5 (8SERDES_0)    IntBus=0 Addr=0x04 */
        0x85, /* Port  6 (8SERDES_0)    IntBus=0 Addr=0x05 */
        0x86, /* Port  7 (8SERDES_0)    IntBus=0 Addr=0x06 */
        0x87, /* Port  8 (8SERDES_0)    IntBus=0 Addr=0x07 */
        0x88, /* Port  9 (8SERDES_0)    IntBus=0 Addr=0x08 */
        0x89, /* Port 10 (8SERDES_1)    IntBus=0 Addr=0x09 */
        0x8a, /* Port 11 (8SERDES_1)    IntBus=0 Addr=0x0a */
        0x8b, /* Port 12 (8SERDES_1)    IntBus=0 Addr=0x0b */
        0x8c, /* Port 13 (8SERDES_1)    IntBus=0 Addr=0x0c */
        0x8d, /* Port 14 (8SERDES_1)    IntBus=0 Addr=0x0d */
        0x8e, /* Port 15 (8SERDES_1)    IntBus=0 Addr=0x0e */
        0x8f, /* Port 16 (8SERDES_1)    IntBus=0 Addr=0x0f */
        0x90, /* Port 17 (8SERDES_1)    IntBus=0 Addr=0x10 */
        0x91, /* Port 18 (9SERDES)      IntBus=0 Addr=0x11 */
        0x92, /* Port 19 (9SERDES)      IntBus=0 Addr=0x12 */
        0x93, /* Port 20 (9SERDES)      IntBus=0 Addr=0x13 */
        0x94, /* Port 21 (9SERDES)      IntBus=0 Addr=0x14 */
        0x95, /* Port 22 (9SERDES)      IntBus=0 Addr=0x15 */
        0x96, /* Port 23 (9SERDES)      IntBus=0 Addr=0x16 */
        0x97, /* Port 24 (9SERDES)      IntBus=0 Addr=0x17 */
        0x98, /* Port 25 (9SERDES)      IntBus=0 Addr=0x18 */
        0x99, /* Port 26 (HC0)          IntBus=0 Addr=0x19 */
        0x9a, /* Port 27 (HC1)          IntBus=0 Addr=0x1a */
        0x9b, /* Port 28 (HC2)          IntBus=0 Addr=0x1b */
        0x9c, /* Port 29 (HC3)          IntBus=0 Addr=0x1c */
};

/* Port table field programming - assumes PORT_LOCK is taken */
int
_bcm_en_port_tab_set(int unit, bcm_port_t port, 
                      soc_field_t field, int value)

{
    port_tab_entry_t pent;
    soc_mem_t mem;
    int rv, cur_val;

    mem = SOC_PORT_MEM_TAB(unit, port);
    if (!SOC_MEM_FIELD_VALID(unit, mem, field)) {
        return (BCM_E_UNAVAIL);
    }
    sal_memset(&pent, 0, sizeof(port_tab_entry_t));
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                      SOC_PORT_MOD_OFFSET(unit, port), &pent);

    if (BCM_SUCCESS(rv)) {
        cur_val = soc_PORT_TABm_field32_get(unit, &pent, field);
        if (value != cur_val) {
            soc_PORT_TABm_field32_set(unit, &pent, field, value);
            rv = soc_mem_write(unit, mem,
                               MEM_BLOCK_ALL, port, &pent);
        }
    }
    return rv;
}

int 
_bcm_en_port_lanes_set(int unit, bcm_port_t port, int value)
{
    int okay, old_value, rv = BCM_E_NONE;
    uint32 rval, val2, to_usec, mode, flags;
    uint64 rval64;
    soc_info_t *si;
    soc_field_t xq_rst2 = INVALIDf;
    soc_field_t ep_port_rst = INVALIDf;
    soc_field_t ep_mmu_rst = INVALIDf;
    uint16 phy0;
    bcm_port_t it_port;
    mac_driver_t *macd;

    si = &SOC_INFO(unit);

    to_usec = SAL_BOOT_QUICKTURN ? (250 * MILLISECOND_USEC) :
                                   (10 * MILLISECOND_USEC);

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }
    if ((port != 26) && (port != 27) && (port != 28) && (port != 29)) {
        return BCM_E_PORT;
    }
    if ((value != 1) && (value != 4)) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_bcm_en_port_lanes_get(unit, port, &old_value));

#define RECONFIGURE_PORT_TYPE_INFO(ptype) \
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
    if ((value == 1) && (old_value == 4)) {
        /* The block was originally a single high-speed port */
        SOC_CONTROL_LOCK(unit);
        if (IS_HG_PORT(unit, port)) {
            SOC_PBMP_PORT_REMOVE(si->st.bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->hg.bitmap, port);
            SOC_PBMP_PORT_ADD(si->ether.bitmap, port);
        } else {
            SOC_PBMP_PORT_REMOVE(si->xe.bitmap, port);
        }
        SOC_PBMP_PORT_ADD(si->ge.bitmap, port);
        si->port_speed_max[port] = 2500;
        soc_port_cmap_set(unit, port, SOC_CTR_TYPE_GE);
        
        RECONFIGURE_PORT_TYPE_INFO(ether);
        RECONFIGURE_PORT_TYPE_INFO(st);
        RECONFIGURE_PORT_TYPE_INFO(hg);
        RECONFIGURE_PORT_TYPE_INFO(xe);
        RECONFIGURE_PORT_TYPE_INFO(ge);

        SOC_CONTROL_UNLOCK(unit);
        BCM_IF_ERROR_RETURN(_bcm_en_port_tab_set(unit, port, PORT_TYPEf, 0));
        BCM_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, EGR_PORTr, port, PORT_TYPEf, 0));

    } else if ((value == 4) && (old_value == 1)) {
        /* The block originally had 4 GE ports */
        SOC_CONTROL_LOCK(unit);
        SOC_PBMP_PORT_ADD(si->st.bitmap, port);
        SOC_PBMP_PORT_ADD(si->hg.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->ether.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->ge.bitmap, port);
        si->port_speed_max[port] = 13000;
        soc_port_cmap_set(unit, port, SOC_CTR_TYPE_XE);
        RECONFIGURE_PORT_TYPE_INFO(ether);
        RECONFIGURE_PORT_TYPE_INFO(st);
        RECONFIGURE_PORT_TYPE_INFO(hg);
        RECONFIGURE_PORT_TYPE_INFO(ge);
        SOC_CONTROL_UNLOCK(unit);
        BCM_IF_ERROR_RETURN(_bcm_en_port_tab_set(unit, port, PORT_TYPEf, 1));
        BCM_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, EGR_PORTr, port, PORT_TYPEf, 1));
    }
#undef RECONFIGURE_PORT_TYPE_INFO

    soc_dport_map_update(unit);

    /* Step 2: Perform necessary block-level reset and initialization */
    /* Egress disable */
    COMPILER_64_ZERO(rval64);
    soc_reg64_field32_set(unit, EGR_ENABLEr, &rval64, PRT_ENABLEf, 0);
    SOC_IF_ERROR_RETURN(WRITE_EGR_ENABLEr(unit, port, rval64));

    /* Hold the XQPORT hotswap reset */
    switch (port) {
    case 26:
        ep_port_rst = EGR_XQP0_PORT_INT_RESETf;
        ep_mmu_rst = EGR_XQP0_MMU_INT_RESETf;
        xq_rst2 = XQ0_HOTSWAP_RST_Lf;
        break;
    case 27:
        ep_port_rst = EGR_XQP1_PORT_INT_RESETf;
        ep_mmu_rst = EGR_XQP1_MMU_INT_RESETf;
        xq_rst2 = XQ1_HOTSWAP_RST_Lf;
        break;
    case 28:
        ep_port_rst = EGR_XQP2_PORT_INT_RESETf;
        ep_mmu_rst = EGR_XQP2_MMU_INT_RESETf;
        xq_rst2 = XQ2_HOTSWAP_RST_Lf;
        break;
    case 29:
        ep_port_rst = EGR_XQP3_PORT_INT_RESETf;
        ep_mmu_rst = EGR_XQP3_MMU_INT_RESETf;
        xq_rst2 = XQ3_HOTSWAP_RST_Lf;
        break;
    default:
        return SOC_E_PORT;
    }

    SOC_IF_ERROR_RETURN(READ_CMIC_SOFT_RESET_REGr(unit, &val2));
    soc_reg_field_set(unit, CMIC_SOFT_RESET_REGr, &val2, 
                      xq_rst2, 0);
    SOC_IF_ERROR_RETURN(WRITE_CMIC_SOFT_RESET_REGr(unit, val2));
    sal_usleep(to_usec);


#if 0 /* Enduro need this? */
    /* Program the ingress and MMU TDMs */
    count = -1;
    for (i = 0; i < soc_mem_index_count(unit, ARB_TDM_TABLEm); i++) {
        BCM_IF_ERROR_RETURN
            (READ_ARB_TDM_TABLEm(unit, MEM_BLOCK_ANY, i, &arb_tdm));
        BCM_IF_ERROR_RETURN
            (READ_IARB_TDM_TABLEm(unit, MEM_BLOCK_ANY, i, &iarb_tdm));
        it_port = soc_ARB_TDM_TABLEm_field32_get(unit, &arb_tdm, PORT_NUMf);
        /* First set to disabled - 63 */
        if (old_value == value) {
            continue;
        } else if (old_value == 4 && value == 1) {
            if (port != it_port) {
                continue;
            }
        } else {
            if ((it_port != port) && (it_port != port + 1) && 
                (it_port != port + 2) && (it_port != port + 3)) {
                continue;
            }
        }
        soc_ARB_TDM_TABLEm_field32_set(unit, &arb_tdm, PORT_NUMf, 63);
        soc_IARB_TDM_TABLEm_field32_set(unit, &iarb_tdm, PORT_NUMf, 63);
        BCM_IF_ERROR_RETURN(WRITE_IARB_TDM_TABLEm(unit, MEM_BLOCK_ALL, i, 
                                                  &iarb_tdm));
        BCM_IF_ERROR_RETURN(WRITE_ARB_TDM_TABLEm(unit, MEM_BLOCK_ALL, i, 
                                                 &arb_tdm));
        /* Then program the desired value */
        if (old_value == 4 && value == 1) {
            count++;
            soc_ARB_TDM_TABLEm_field32_set(unit, &arb_tdm, PORT_NUMf, 
                                           port + (count % 4));
            soc_IARB_TDM_TABLEm_field32_set(unit, &iarb_tdm, PORT_NUMf, 
                                            port + (count % 4));
        } else {
            soc_ARB_TDM_TABLEm_field32_set(unit, &arb_tdm, PORT_NUMf, port);
            soc_IARB_TDM_TABLEm_field32_set(unit, &iarb_tdm, PORT_NUMf, port);
        }
        BCM_IF_ERROR_RETURN(WRITE_IARB_TDM_TABLEm(unit, MEM_BLOCK_ALL, i, 
                                                  &iarb_tdm));
        BCM_IF_ERROR_RETURN(WRITE_ARB_TDM_TABLEm(unit, MEM_BLOCK_ALL, i, 
                                                 &arb_tdm));
        if (soc_ARB_TDM_TABLEm_field32_get(unit, &arb_tdm, WRAP_ENf)) {
            break;
        }
    }
#endif

    /* Clear the ECRC register */
    if (SOC_REG_IS_VALID(unit,ECRCr)) {
        SOC_IF_ERROR_RETURN(WRITE_ECRCr(unit, port, 0));
    } else if (SOC_REG_IS_VALID(unit,EGRESSCELLREQUESTCOUNTr)) {
		SOC_IF_ERROR_RETURN(WRITE_EGRESSCELLREQUESTCOUNTr(unit, port, 0));
	}
    /* Reset the EP */
    SOC_IF_ERROR_RETURN(READ_EDATABUF_XQP_FLEXPORT_CONFIGr(unit, &val2));
    soc_reg_field_set(unit, EDATABUF_XQP_FLEXPORT_CONFIGr, &val2, 
                      ep_port_rst, 1);
    soc_reg_field_set(unit, EDATABUF_XQP_FLEXPORT_CONFIGr, &val2, 
                      ep_mmu_rst, 1);
    SOC_IF_ERROR_RETURN(WRITE_EDATABUF_XQP_FLEXPORT_CONFIGr(unit, val2));
    sal_usleep(to_usec);

    SOC_IF_ERROR_RETURN(READ_EDATABUF_XQP_FLEXPORT_CONFIGr(unit, &val2));
    soc_reg_field_set(unit, EDATABUF_XQP_FLEXPORT_CONFIGr, &val2, 
                      ep_port_rst, 0);
    soc_reg_field_set(unit, EDATABUF_XQP_FLEXPORT_CONFIGr, &val2, 
                      ep_mmu_rst, 0);
    SOC_IF_ERROR_RETURN(WRITE_EDATABUF_XQP_FLEXPORT_CONFIGr(unit, val2));
    sal_usleep(to_usec);

    /* Bring the XQPORT block out of reset */
    SOC_IF_ERROR_RETURN(READ_CMIC_SOFT_RESET_REGr(unit, &val2));
    soc_reg_field_set(unit, CMIC_SOFT_RESET_REGr, &val2, 
                      xq_rst2, 1);
    SOC_IF_ERROR_RETURN(WRITE_CMIC_SOFT_RESET_REGr(unit, val2));
    sal_usleep(to_usec);

    /* Bring the hyperlite out of reset */
    soc_xgxs_reset(unit, port);
    SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit, XQPORT_XGXS_NEWCTL_REGr, 
                                               port, TXD1G_FIFO_RSTBf, 0xf));

    /* Change the XQPORT block mode */
    mode = (value == 4) ? 2 : 1; /* 2 = high-speed, 1 = GE */
    SOC_IF_ERROR_RETURN(READ_XQPORT_MODE_REGr(unit, port, &rval));
    soc_reg_field_set(unit, XQPORT_MODE_REGr, &rval, 
                      XQPORT_MODE_BITSf, mode);
    SOC_IF_ERROR_RETURN(WRITE_XQPORT_MODE_REGr(unit, port, rval));

    if (value == 4) {
        rval = 0;
        soc_reg_field_set(unit, XPORT_CONFIGr, &rval, XPORT_ENf, 1);
        soc_reg_field_set(unit, XPORT_CONFIGr, &rval, HIGIG_MODEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_XPORT_CONFIGr(unit, port, rval));
    }

    /* Egress Enable */
    COMPILER_64_ZERO(rval64);
    soc_reg64_field32_set(unit, EGR_ENABLEr, &rval64, PRT_ENABLEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_EGR_ENABLEr(unit, port, rval64));

    /* Step 3: Port probe and initialization */
    phy0 = _soc_en_int_phy_addr[port];
    flags = _SHR_PORT_PHY_INTERNAL;
#if 0    
    SOC_IF_ERROR_RETURN(soc_phyctrl_detach(unit, port));
    SOC_IF_ERROR_RETURN(soc_phy_cfg_addr_set(unit, port, flags, phy0));
#endif
    BCM_IF_ERROR_RETURN(_bcm_port_probe(unit, port, &okay));
    if (!okay) {
        return BCM_E_INTERNAL;
    }

    BCM_IF_ERROR_RETURN(_bcm_port_mode_setup(unit, port, TRUE));

    if (value == 4) {
        /* Need to reset the Bigmac to make registers accessible */
        SOC_IF_ERROR_RETURN(soc_mac_probe(unit, port, &macd));
        SOC_IF_ERROR_RETURN(READ_MAC_CTRLr(unit, port, &rval64));
        soc_reg64_field32_set(unit, MAC_CTRLr, &rval64, TXRESETf, 0);
        soc_reg64_field32_set(unit, MAC_CTRLr, &rval64, RXRESETf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MAC_CTRLr(unit, port, rval64));
        SOC_IF_ERROR_RETURN(MAC_INIT(macd, unit, port));
        BCM_IF_ERROR_RETURN(_bcm_port_mode_setup(unit, port, TRUE));
    }

    return rv;
}
#endif
