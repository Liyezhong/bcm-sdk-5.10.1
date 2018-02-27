/*
 * $Id: flex_ctr_egress.c 1.1.2.2 Broadcom SDK $
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
 * File:        flex_ctr_egress.c
 * Purpose:     Manage flex counter ingress group creation and deletion
 */

#include <bcm_int/esw/flex_ctr.h>
#include <bcm/debug.h>

static soc_reg_t _egress_pkt_selector_key_reg[8] = {
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_0r,
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_1r,
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_2r,
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_3r,
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_4r,
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_5r,
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_6r,
                 EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_7r
                 };
/*
 * Function:
 *      _bcm_esw_stat_flex_update_egress_selector_keys
 * Description:
 *      Update flex egress selector keys
 * Parameters:
 *      unit                  - (IN) unit number
 *      pkt_attr_type         - (IN) Flex Packet Attribute Type
 *      pkt_selector_key_reg  - (IN) Flex Packet Attribute selector key register
 *      pkt_attr_bits         - (IN) Flex Packet Attribute Bits
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */
static bcm_error_t 
_bcm_esw_stat_flex_update_egress_selector_keys(
    int                               unit,
    bcm_stat_flex_packet_attr_type_t  pkt_attr_type,
    soc_reg_t                         pkt_selector_key_reg,
    bcm_stat_flex_egr_pkt_attr_bits_t pkt_attr_bits)
{
    uint64 pkt_selector_key_reg_value, val64;
    uint8  current_bit_position=0;
    uint8         index=0;

    COMPILER_64_ZERO(pkt_selector_key_reg_value);
    COMPILER_64_ZERO(val64);
    
    for (index=0;index<8;index++) {
         if (pkt_selector_key_reg == _egress_pkt_selector_key_reg[index]) {
            break;
         }
    }
    if (index == 8) {
           return BCM_E_PARAM;
    }

    /* bcmStatFlexPacketAttrTypeUdf not supported here */
    if (!((pkt_attr_type==bcmStatFlexPacketAttrTypeUncompressed) ||
          (pkt_attr_type==bcmStatFlexPacketAttrTypeCompressed))) {
           return BCM_E_PARAM;
    }
    /* Valid register value go ahead for setting
       EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r:SELECTOR_KEY field */

    /* First Get complete value of EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r value*/
    SOC_IF_ERROR_RETURN(soc_reg_get(unit,
                                    pkt_selector_key_reg,
                                    REG_PORT_ANY,
                                    0,
                                    &pkt_selector_key_reg_value));

    /* Next set field value for
       EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r:SELECTOR_KEY field */
    soc_reg64_field_set(unit,
                        pkt_selector_key_reg,
                        &pkt_selector_key_reg_value,
                        USE_UDF_KEYf,
                        val64);
    soc_reg64_field_set(unit,
                        pkt_selector_key_reg,
                        &pkt_selector_key_reg_value,
                        USER_SPECIFIED_UDF_VALIDf,
                        val64);
    if (pkt_attr_type==bcmStatFlexPacketAttrTypeCompressed) {
        COMPILER_64_SET(val64, 0, 1);
        soc_reg64_field_set(unit,
                            pkt_selector_key_reg,
                            &pkt_selector_key_reg_value,
                            USE_COMPRESSED_PKT_KEYf,
                            val64);
    }else {
        soc_reg64_field_set(unit,
                            pkt_selector_key_reg,
                            &pkt_selector_key_reg_value,
                            USE_COMPRESSED_PKT_KEYf,
                            val64);
    }
    if (pkt_attr_bits.ip_pkt != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /*IP_PKT bit position */
                           0,
                           /* 1:IP_PKT total bits */
                           pkt_attr_bits.ip_pkt,
                           pkt_attr_bits.ip_pkt_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.drop != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                             unit,
                             pkt_selector_key_reg,
                             &pkt_selector_key_reg_value,
                             /* DROP bit position */
                             1,
                             pkt_attr_bits.drop,
                             /* 1:DROP total bits */
                             pkt_attr_bits.drop_mask,
                             &current_bit_position));
    }
    if (pkt_attr_bits.dvp_type != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* DVP_TYPE bit position */
                           2,
                           pkt_attr_bits.dvp_type,
                           /* 1:DVP_TYPE total bits  */
                           pkt_attr_bits.dvp_type_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.svp_type != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* SVP_TYPE bit position */
                           3,
                           /* 1:SVP_TYPE total bits */
                           pkt_attr_bits.svp_type,
                           pkt_attr_bits.svp_type_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.pkt_resolution != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* PKT_RESOLUTION bit position */
                           4,
                           /* 1:PKT_RESOLUTION total bits */
                           pkt_attr_bits.pkt_resolution,
                           pkt_attr_bits.pkt_resolution_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.tos != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* TOS bit position */
                           5,
                           /* 8:TOS total bits */
                           pkt_attr_bits.tos,
                           pkt_attr_bits.tos_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.egr_port != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* EGR_PORT bit position */
                           13,
                           /* 6:EGR_PORT total bits */
                           pkt_attr_bits.egr_port,
                           pkt_attr_bits.egr_port_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.inner_dot1p != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* INNER_DOT1P bit position */
                           19,
                           /* 3:INNER_DOT1P total bits */
                           pkt_attr_bits.inner_dot1p,
                           pkt_attr_bits.inner_dot1p_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.outer_dot1p != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* OUTER_DOT1P bit position */
                           22,
                           /* 3:OUTER_DOT1P total bits */
                           pkt_attr_bits.outer_dot1p,
                           pkt_attr_bits.outer_dot1p_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.vlan_format != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* VLAN_FORMAT bit position */
                           25,
                           /* 2:VLAN_FORMAT total bits  */
                           pkt_attr_bits.vlan_format,
                           pkt_attr_bits.vlan_format_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.int_pri != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* INT_PRI bit position */
                           27,
                           /* 4:INT_PRI total bits */
                           pkt_attr_bits.int_pri,
                           pkt_attr_bits.int_pri_mask,
                           &current_bit_position));
    }
    if (pkt_attr_bits.cng != 0) {
        BCM_IF_ERROR_RETURN(
        _bcm_esw_stat_flex_update_selector_keys_enable_fields(
                           unit,
                           pkt_selector_key_reg,
                           &pkt_selector_key_reg_value,
                           /* CNG bit position */
                           31,
                           /* 2:CNG total bits */
                           pkt_attr_bits.cng,
                           pkt_attr_bits.cng_mask,
                           &current_bit_position));
    }
    /* Finally set value for EGR_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r */
    SOC_IF_ERROR_RETURN(soc_reg_set(        unit,
                        pkt_selector_key_reg,
                        REG_PORT_ANY,
                        0,
                        pkt_selector_key_reg_value));
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_create_egress_uncompress_mode
 * Description:
 *      Creates New Flex Egress Uncompressed Mode
 *
 * Parameters:
 *      unit                     - (IN) unit number
 *      uncmprsd_attr_selectors  - (IN) Flex attributes
 *      mode                     - (OUT) Flex mode
 *      total_counters           - (OUT) Total Counters associated with new
 *                                       flex uncompressed mode
 *
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

static bcm_error_t 
_bcm_esw_stat_flex_create_egress_uncompress_mode(
    int                                         unit,
    bcm_stat_flex_egr_uncmprsd_attr_selectors_t *uncmprsd_attr_selectors,
    bcm_stat_flex_mode_t                        *mode,
    uint32                                      *total_counters)
{
    bcm_stat_flex_mode_t                        mode_l=0;
    bcm_stat_flex_egr_pkt_attr_bits_t           pkt_attr_bits={0};
    uint32                                      total_egress_bits=0;
    uint32                                      index=0;
    bcm_stat_flex_egress_mode_t                 flex_egress_mode;

    bcm_stat_flex_egr_uncmprsd_attr_selectors_t uncmprsd_attr_selectors_l;
    uint32                                      num_flex_egress_pools;

    num_flex_egress_pools = SOC_INFO(unit).num_flex_egress_pools;

    for (index =0; index < BCM_STAT_FLEX_COUNTER_MAX_MODE ; index++) {
         if (_bcm_esw_stat_flex_get_egress_mode_info(
             unit,
             index,
             &flex_egress_mode) == BCM_E_NONE) {
             if (flex_egress_mode.egr_attr.packet_attr_type !=
                 bcmStatFlexPacketAttrTypeUncompressed) {
                 continue;
             }
             uncmprsd_attr_selectors_l = flex_egress_mode.egr_attr.
                                         uncmprsd_attr_selectors;
             if (sal_memcmp(&uncmprsd_attr_selectors_l,uncmprsd_attr_selectors,
                            sizeof(uncmprsd_attr_selectors_l)) == 0) {
                 *total_counters = flex_egress_mode.total_counters;
                 *mode=index;
                 return BCM_E_EXISTS;
             }
         }
    }

    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_available_mode(
                        unit,
                        bcmStatFlexDirectionEgress,
                        &mode_l));
    /* Step2: Packet Attribute selection */
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_CNG_ATTR_BITS) {
        pkt_attr_bits.cng = 2;
        pkt_attr_bits.cng_mask = (1 << pkt_attr_bits.cng) - 1;
        total_egress_bits +=2;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS) {
        pkt_attr_bits.int_pri = 4;
        pkt_attr_bits.int_pri_mask = (1 << pkt_attr_bits.int_pri) - 1;
        total_egress_bits +=4;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS) {
        pkt_attr_bits.vlan_format = 2;
        pkt_attr_bits.vlan_format_mask=(1<<pkt_attr_bits.vlan_format)-1;
        total_egress_bits +=2;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS) {
        pkt_attr_bits.outer_dot1p = 3;
        pkt_attr_bits.outer_dot1p_mask=(1<<pkt_attr_bits.outer_dot1p)-1;
        total_egress_bits +=3;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS) {
        pkt_attr_bits.inner_dot1p = 3;
        pkt_attr_bits.inner_dot1p_mask = (1 << pkt_attr_bits.inner_dot1p) - 1;
        total_egress_bits +=3;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_EGRESS_PORT_ATTR_BITS) {
        pkt_attr_bits.egr_port = 6;
        pkt_attr_bits.egr_port_mask = (1 << pkt_attr_bits.egr_port) - 1;
        total_egress_bits +=6;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_TOS_ATTR_BITS) {
        pkt_attr_bits.tos = 8;
        pkt_attr_bits.tos_mask = (1 << pkt_attr_bits.tos) - 1;
        total_egress_bits +=8;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
        pkt_attr_bits.pkt_resolution = 1;
        pkt_attr_bits.pkt_resolution_mask=(1 << pkt_attr_bits.pkt_resolution)-1;
        total_egress_bits +=1;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS) {
        pkt_attr_bits.svp_type = 1;
        pkt_attr_bits.svp_type_mask = (1 << pkt_attr_bits.svp_type) - 1;
        total_egress_bits +=1;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DVP_TYPE_ATTR_BITS) {
        pkt_attr_bits.dvp_type = 1;
        pkt_attr_bits.dvp_type_mask = (1 << pkt_attr_bits.dvp_type) - 1;
        total_egress_bits +=1;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_DROP_ATTR_BITS) {
        pkt_attr_bits.drop = 1;
        pkt_attr_bits.drop_mask = (1 << pkt_attr_bits.drop) - 1;
        total_egress_bits +=1;
    }
    if (uncmprsd_attr_selectors->uncmprsd_attr_bits_selector &
        BCM_STAT_FLEX_EGR_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS) {
        pkt_attr_bits.ip_pkt = 1;
        pkt_attr_bits.ip_pkt_mask = (1 << pkt_attr_bits.ip_pkt) - 1;
        total_egress_bits +=1;
    }
    if (total_egress_bits > 8) {
        return BCM_E_PARAM;
    }

    /* Step3: Packet Attribute filling */
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_egress_selector_keys(unit,
                        bcmStatFlexPacketAttrTypeUncompressed,
                        _egress_pkt_selector_key_reg[mode_l],
                        pkt_attr_bits));
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_egress_selector_keys(unit,
                        bcmStatFlexPacketAttrTypeUncompressed,
                        _egress_pkt_selector_key_reg[mode_l+4],
                        pkt_attr_bits));
    /* Step4: Offset table filling */
    for (index=0;index < num_flex_egress_pools ; index++) {
         BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_offset_table(unit,
                             bcmStatFlexDirectionEgress,
                             EGR_FLEX_CTR_OFFSET_TABLE_0m+index,
                             mode_l,
                             256,
                             uncmprsd_attr_selectors->offset_table_map));
    }
    /* Step5: Final: reserver mode and return */
    *total_counters=uncmprsd_attr_selectors->total_counters;
    *mode=mode_l;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_create_egress_compress_mode
 * Description:
 *      Creates New Flex Egress Compressed Mode
 * Parameters:
 *      unit                   - (IN) unit number
 *      cmprsd_attr_selectors  - (IN) Flex Compressed Attribute Selector
 *      mode                   - (OUT) Flex mode
 *      total_counters         - (OUT) Total Counters associated with new
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

static bcm_error_t 
_bcm_esw_stat_flex_create_egress_compress_mode(
    int                                       unit,
    bcm_stat_flex_egr_cmprsd_attr_selectors_t *cmprsd_attr_selectors,
    bcm_stat_flex_mode_t                      *mode,
    uint32                                    *total_counters)
{
    bcm_stat_flex_mode_t              mode_l=0;
    bcm_stat_flex_egr_pkt_attr_bits_t pkt_attr_bits={0};
    uint32                            total_egress_bits=0;
    uint32                            index=0;
    bcm_stat_flex_egress_mode_t       *flex_egress_mode=NULL;
    bcm_stat_flex_egr_pkt_attr_bits_t pkt_attr_bits_exist;

    /*soc_mem_info_t                  *mem_info; */
    uint32                            pri_cnf_map_used=0;
    uint32                            pkt_pri_map_used=0;
    uint32                            port_map_used=0;
    uint32                            tos_map_used=0;
    uint32                            pkt_res_map_used=0;
    uint32                            num_flex_egress_pools;


    /* MAP ENTRY Structure */
    egr_flex_ctr_pkt_pri_map_entry_t *pkt_pri_map_dma=NULL;
    uint32                           pkt_pri_map_value=0;
    egr_flex_ctr_pkt_res_map_entry_t *pkt_res_map_dma=NULL;
    uint32                           pkt_res_map_value=0;
    egr_flex_ctr_port_map_entry_t    *port_map_dma=NULL;
    uint32                           port_map_value=0;
    egr_flex_ctr_pri_cng_map_entry_t *pri_cng_map_dma=NULL;
    uint32                           pri_cng_map_value=0;
    egr_flex_ctr_tos_map_entry_t     *tos_map_dma=NULL;
    uint32                           tos_map_value=0;

    num_flex_egress_pools = SOC_INFO(unit).num_flex_egress_pools;
    flex_egress_mode = sal_alloc(sizeof(bcm_stat_flex_egress_mode_t),
                                "flex_egress_mode");
    if (flex_egress_mode == NULL) {
        return BCM_E_MEMORY;
    }


    for (index =0 ; index < BCM_STAT_FLEX_COUNTER_MAX_MODE ; index++) {
         sal_memset(flex_egress_mode,0,sizeof(bcm_stat_flex_egress_mode_t));
         if (_bcm_esw_stat_flex_get_egress_mode_info(
             unit,
             index,
             flex_egress_mode) == BCM_E_NONE) {
             if (flex_egress_mode->egr_attr.packet_attr_type !=
                 bcmStatFlexPacketAttrTypeCompressed) {
                 continue;
             }
             pkt_attr_bits_exist = flex_egress_mode->egr_attr.
                                   cmprsd_attr_selectors.pkt_attr_bits;
             if ((pkt_attr_bits_exist.cng ==
                  cmprsd_attr_selectors->pkt_attr_bits.cng) &&
                 (pkt_attr_bits_exist.int_pri ==
                  cmprsd_attr_selectors->pkt_attr_bits.int_pri) &&
                 (pkt_attr_bits_exist.vlan_format ==
                  cmprsd_attr_selectors->pkt_attr_bits.vlan_format) &&
                 (pkt_attr_bits_exist.outer_dot1p ==
                  cmprsd_attr_selectors->pkt_attr_bits.outer_dot1p) &&
                 (pkt_attr_bits_exist.inner_dot1p ==
                  cmprsd_attr_selectors->pkt_attr_bits.inner_dot1p) &&
                 (pkt_attr_bits_exist.egr_port ==
                  cmprsd_attr_selectors->pkt_attr_bits.egr_port) &&
                 (pkt_attr_bits_exist.tos ==
                  cmprsd_attr_selectors->pkt_attr_bits.tos) &&
                 (pkt_attr_bits_exist.pkt_resolution ==
                  cmprsd_attr_selectors->pkt_attr_bits.pkt_resolution) &&
                 (pkt_attr_bits_exist.svp_type ==
                  cmprsd_attr_selectors->pkt_attr_bits.svp_type) &&
                 (pkt_attr_bits_exist.dvp_type ==
                  cmprsd_attr_selectors->pkt_attr_bits.dvp_type) &&
                 (pkt_attr_bits_exist.drop ==
                  cmprsd_attr_selectors->pkt_attr_bits.drop) &&
                 (pkt_attr_bits_exist.ip_pkt ==
                  cmprsd_attr_selectors->pkt_attr_bits.ip_pkt)) {
                  *total_counters = flex_egress_mode->total_counters;
                  *mode=index;
                  sal_free(flex_egress_mode);
                  return BCM_E_EXISTS;
             }
             if ((pkt_attr_bits_exist.cng != 0) ||
                 (pkt_attr_bits_exist.int_pri)) {
                  pri_cnf_map_used=1;
             }
             if ((pkt_attr_bits_exist.vlan_format != 0) ||
                 (pkt_attr_bits_exist.outer_dot1p) ||
                 (pkt_attr_bits_exist.inner_dot1p)){
                  pkt_pri_map_used=1;
             }
             if (pkt_attr_bits_exist.egr_port != 0){
                 port_map_used=1;
             }
             if (pkt_attr_bits_exist.tos != 0){
                 tos_map_used=1;
             }
             if ((pkt_attr_bits_exist.pkt_resolution!=0) ||
                 (pkt_attr_bits_exist.svp_type) ||
                 (pkt_attr_bits_exist.dvp_type) ||
                 (pkt_attr_bits_exist.drop)){
                  pkt_res_map_used=1;
             }
         }
    }
    sal_free(flex_egress_mode);
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_available_mode(
                        unit,
                        bcmStatFlexDirectionEgress,&mode_l));

    /* Step2: Packet Attribute selection */
    if (cmprsd_attr_selectors->pkt_attr_bits.cng !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.cng > 2) {
            return BCM_E_PARAM;
        }
        if (pri_cnf_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.cng;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.int_pri !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.int_pri > 4) {
            return BCM_E_PARAM;
        }
        if (pri_cnf_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.int_pri;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.vlan_format !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.vlan_format >2) {
            return BCM_E_PARAM;
        }
        if (pkt_pri_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.vlan_format;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.outer_dot1p !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.outer_dot1p  > 3) {
            return BCM_E_PARAM;
        }
        if (pkt_pri_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.outer_dot1p ;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.inner_dot1p !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.inner_dot1p  >3) {
            return BCM_E_PARAM;
        }
        if (pkt_pri_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.inner_dot1p ;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.egr_port !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.egr_port  > 6) {
            return BCM_E_PARAM;
        }
        if (port_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.egr_port ;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.tos !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.tos  > 8) {
            return BCM_E_PARAM;
        }
        if (tos_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.tos ;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.pkt_resolution !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.pkt_resolution  > 1) {
            return BCM_E_PARAM;
        }
        if (pkt_res_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.
                              pkt_resolution;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.svp_type !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.svp_type  > 1) {
            return BCM_E_PARAM;
        }
        if (pkt_res_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.svp_type ;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.dvp_type !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.dvp_type  > 1) {
            return BCM_E_PARAM;
        }
        if (pkt_res_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.dvp_type ;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.drop        !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.drop > 1) {
            return BCM_E_PARAM;
        }
        if (pkt_res_map_used==1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.drop ;
    }
    if (cmprsd_attr_selectors->pkt_attr_bits.ip_pkt        !=0) {
        if (cmprsd_attr_selectors->pkt_attr_bits.ip_pkt > 1) {
            return BCM_E_PARAM;
        }
        total_egress_bits +=cmprsd_attr_selectors->pkt_attr_bits.ip_pkt ;
    }
    if (total_egress_bits > 8) {
        return BCM_E_PARAM;
    }
    pkt_attr_bits =cmprsd_attr_selectors->pkt_attr_bits;
    /* Step3: Fill up map array */
    if ((pkt_attr_bits.cng != 0) ||
        (pkt_attr_bits.int_pri)) {
         /* Sanity Check */
         if (soc_mem_index_count(unit,EGR_FLEX_CTR_PRI_CNG_MAPm) !=
             sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t)) {
             return BCM_E_INTERNAL;
         }
         pri_cng_map_dma=soc_cm_salloc(
                           unit,
                           sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t)*
                           sizeof(egr_flex_ctr_pri_cng_map_entry_t),
                           "pri_cng_map");
         if (pri_cng_map_dma == NULL) {
             return BCM_E_MEMORY;
         }
         sal_memset(pri_cng_map_dma,
                    0,sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t));
         if (soc_mem_read_range(
                     unit,
                     EGR_FLEX_CTR_PRI_CNG_MAPm,
                     MEM_BLOCK_ANY,
                     soc_mem_index_min(unit,EGR_FLEX_CTR_PRI_CNG_MAPm),
                     soc_mem_index_max(unit,EGR_FLEX_CTR_PRI_CNG_MAPm),
                     pri_cng_map_dma) != BCM_E_NONE){
             soc_cm_sfree(unit,pri_cng_map_dma);
             return BCM_E_INTERNAL;
         }
         for (index=0;
              index< sizeof(bcm_stat_flex_egr_cmprsd_pri_cnf_attr_map_t);
              index++) {
              pri_cng_map_value=cmprsd_attr_selectors->pri_cnf_attr_map[index];
              soc_mem_field_set(
                      unit,
                      EGR_FLEX_CTR_PRI_CNG_MAPm,
                      (uint32 *)&pri_cng_map_dma[index],
                      PRI_CNG_FNf,
                      &pri_cng_map_value);
         }
         if (soc_mem_write_range(unit,
                     EGR_FLEX_CTR_PRI_CNG_MAPm,
                     MEM_BLOCK_ALL,
                     soc_mem_index_min(unit,EGR_FLEX_CTR_PRI_CNG_MAPm),
                     soc_mem_index_max(unit,EGR_FLEX_CTR_PRI_CNG_MAPm),
                     pri_cng_map_dma) != BCM_E_NONE) {
             soc_cm_sfree(unit,pri_cng_map_dma);
             return BCM_E_INTERNAL;
         }
         soc_cm_sfree(unit,pri_cng_map_dma);
    }
    if ((pkt_attr_bits.vlan_format != 0) ||
        (pkt_attr_bits.outer_dot1p) ||
        (pkt_attr_bits.inner_dot1p)){
         /* Sanity Check */
         if (soc_mem_index_count(unit,EGR_FLEX_CTR_PKT_PRI_MAPm)
             != sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t)) {
                  return BCM_E_INTERNAL;
         }
         pkt_pri_map_dma = soc_cm_salloc(
                           unit,
                           sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t)*
                           sizeof(egr_flex_ctr_pkt_pri_map_entry_t),
                           "pkt_pri_map");
         if (pkt_pri_map_dma == NULL) {
             return BCM_E_MEMORY;
         }
         sal_memset(pkt_pri_map_dma,
                    0,sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t));
         if (soc_mem_read_range(
                     unit,
                     EGR_FLEX_CTR_PKT_PRI_MAPm,
                     MEM_BLOCK_ANY,
                     soc_mem_index_min(unit,EGR_FLEX_CTR_PKT_PRI_MAPm),
                     soc_mem_index_max(unit,EGR_FLEX_CTR_PKT_PRI_MAPm),
                     pkt_pri_map_dma) != BCM_E_NONE) {
             soc_cm_sfree(unit,pkt_pri_map_dma);
             return BCM_E_INTERNAL;
         }
         for (index=0;
              index< sizeof(bcm_stat_flex_egr_cmprsd_pkt_pri_attr_map_t);
              index++) {
              pkt_pri_map_value= cmprsd_attr_selectors->pkt_pri_attr_map[index];
              soc_mem_field_set(
                      unit,
                      EGR_FLEX_CTR_PKT_PRI_MAPm,
                      (uint32 *)&pkt_pri_map_dma[index],
                      PKT_PRI_FNf,
                      &pkt_pri_map_value);
         }
         if (soc_mem_write_range(unit,
                     EGR_FLEX_CTR_PKT_PRI_MAPm,
                     MEM_BLOCK_ALL,
                     soc_mem_index_min(unit,EGR_FLEX_CTR_PKT_PRI_MAPm),
                     soc_mem_index_max(unit,EGR_FLEX_CTR_PKT_PRI_MAPm),
                     pkt_pri_map_dma) != BCM_E_NONE){
             soc_cm_sfree(unit,pkt_pri_map_dma);
             return BCM_E_INTERNAL;
         }
         soc_cm_sfree(unit,pkt_pri_map_dma);
    }
    if (pkt_attr_bits.egr_port != 0){
        /* Sanity Check */
        if (soc_mem_index_count(unit,EGR_FLEX_CTR_PORT_MAPm)
            != sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t)) {
            return BCM_E_INTERNAL;
        }
        port_map_dma = soc_cm_salloc(
                           unit,
                           sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t)*
                           sizeof(egr_flex_ctr_port_map_entry_t),
                           "port_map");
        if (port_map_dma == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(port_map_dma,
                   0,sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t));
        if (soc_mem_read_range(
                    unit,
                    EGR_FLEX_CTR_PORT_MAPm,
                    MEM_BLOCK_ANY,
                    soc_mem_index_min(unit,EGR_FLEX_CTR_PORT_MAPm),
                    soc_mem_index_max(unit,EGR_FLEX_CTR_PORT_MAPm),
                    port_map_dma) != BCM_E_NONE) {
            soc_cm_sfree(unit,port_map_dma);
            return BCM_E_INTERNAL;
        }
        for(index=0;
            index< sizeof(bcm_stat_flex_egr_cmprsd_port_attr_map_t);
            index++) {
            port_map_value = cmprsd_attr_selectors->port_attr_map[index];
            soc_mem_field_set(
                    unit,
                    EGR_FLEX_CTR_PORT_MAPm,
                    (uint32 *)&port_map_dma[index],
                    PORT_FNf,
                    &port_map_value);
        }
        if (soc_mem_write_range(
                    unit,
                    EGR_FLEX_CTR_PORT_MAPm,
                    MEM_BLOCK_ALL,
                    soc_mem_index_min(unit,EGR_FLEX_CTR_PORT_MAPm),
                    soc_mem_index_max(unit,EGR_FLEX_CTR_PORT_MAPm),
                    port_map_dma) != BCM_E_NONE){
            soc_cm_sfree(unit,port_map_dma);
            return BCM_E_INTERNAL;
        }
        soc_cm_sfree(unit,port_map_dma);
    }
    if (pkt_attr_bits.tos != 0){
        /* Sanity Check */
        if (soc_mem_index_count(unit,EGR_FLEX_CTR_TOS_MAPm)
            != sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t)) {
            return BCM_E_INTERNAL;
        }
        tos_map_dma = soc_cm_salloc(
                          unit,
                          sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t)*
                          sizeof(egr_flex_ctr_tos_map_entry_t),
                          "tos_map");
        if (tos_map_dma == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(tos_map_dma,
                   0,sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t));
        if (soc_mem_read_range(
                    unit,
                    EGR_FLEX_CTR_TOS_MAPm,
                    MEM_BLOCK_ANY,
                    soc_mem_index_min(unit,EGR_FLEX_CTR_TOS_MAPm),
                    soc_mem_index_max(unit,EGR_FLEX_CTR_TOS_MAPm),
                    tos_map_dma) != BCM_E_NONE){
            soc_cm_sfree(unit,tos_map_dma); 
            return BCM_E_INTERNAL;
        }
        for(index=0;
            index< sizeof(bcm_stat_flex_egr_cmprsd_tos_attr_map_t);
            index++) {
            tos_map_value = cmprsd_attr_selectors->tos_attr_map[index];
            soc_mem_field_set(
                    unit,
                    EGR_FLEX_CTR_PORT_MAPm,
                    (uint32 *)&tos_map_dma[index],
                    TOS_FNf,
                    &tos_map_value);
        }
        if (soc_mem_write_range(unit,
                    EGR_FLEX_CTR_TOS_MAPm,
                    MEM_BLOCK_ALL,
                    soc_mem_index_min(unit,EGR_FLEX_CTR_TOS_MAPm),
                    soc_mem_index_max(unit,EGR_FLEX_CTR_TOS_MAPm),
                    tos_map_dma) != BCM_E_NONE){
            soc_cm_sfree(unit,tos_map_dma); 
            return BCM_E_INTERNAL;
        }
        soc_cm_sfree(unit,tos_map_dma); 
    }
    if ((pkt_attr_bits.pkt_resolution != 0) ||
        (pkt_attr_bits.svp_type) ||
        (pkt_attr_bits.dvp_type) ||
        (pkt_attr_bits.drop)){
         /* Sanity Check */
         if (soc_mem_index_count(unit,EGR_FLEX_CTR_PKT_RES_MAPm)
             != sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t)) {
             return BCM_E_INTERNAL;
         }
         pkt_res_map_dma= soc_cm_salloc(
                            unit,
                            sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t)*
                            sizeof(egr_flex_ctr_pkt_res_map_entry_t),
                            "pkt_res_map");
         if (pkt_res_map_dma == NULL) {
             return BCM_E_MEMORY;
         }
         sal_memset(pkt_res_map_dma,
                    0,sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t));
         if (soc_mem_read_range(
                     unit,
                     EGR_FLEX_CTR_PKT_RES_MAPm,
                     MEM_BLOCK_ANY,
                     soc_mem_index_min(unit,EGR_FLEX_CTR_PKT_RES_MAPm),
                     soc_mem_index_max(unit,EGR_FLEX_CTR_PKT_RES_MAPm),
                     pkt_res_map_dma) != BCM_E_NONE){
             soc_cm_sfree(unit,pkt_res_map_dma); 
             return BCM_E_INTERNAL;
         }
         for (index=0;
              index< sizeof(bcm_stat_flex_egr_cmprsd_pkt_res_attr_map_t);
              index++) {
              pkt_res_map_value= cmprsd_attr_selectors->pkt_res_attr_map[index];
              soc_mem_field_set(
                      unit,
                      EGR_FLEX_CTR_PKT_RES_MAPm,
                      (uint32 *)&pkt_res_map_dma[index],
                      PKT_RES_FNf,
                      &pkt_res_map_value);
         }
         if (soc_mem_write_range(unit,
                     EGR_FLEX_CTR_PKT_RES_MAPm,
                     MEM_BLOCK_ALL,
                     soc_mem_index_min(unit,EGR_FLEX_CTR_PKT_RES_MAPm),
                     soc_mem_index_max(unit,EGR_FLEX_CTR_PKT_RES_MAPm),
                     pkt_res_map_dma) != BCM_E_NONE){
             soc_cm_sfree(unit,pkt_res_map_dma); 
             return BCM_E_INTERNAL;
         }
         soc_cm_sfree(unit,pkt_res_map_dma); 
    }
    /* Step4: Packet Attribute filling */
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_egress_selector_keys(
                        unit,
                        bcmStatFlexPacketAttrTypeCompressed,
                        _egress_pkt_selector_key_reg[mode_l],
                        pkt_attr_bits));
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_egress_selector_keys(
                        unit,
                        bcmStatFlexPacketAttrTypeCompressed,
                        _egress_pkt_selector_key_reg[mode_l+4],
                        pkt_attr_bits));
    /* Step5: Offset table filling */
    for (index=0;index < num_flex_egress_pools ; index++) {
         BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_offset_table(
                             unit,
                             bcmStatFlexDirectionEgress,
                             EGR_FLEX_CTR_OFFSET_TABLE_0m+index,
                             mode_l,
                             256,
                             cmprsd_attr_selectors->offset_table_map));
    }
    /* Step6: Final: reserve mode and return */
    *total_counters=cmprsd_attr_selectors->total_counters;
    *mode=mode_l;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_create_egress_udf_mode
 * Description:
 *      Creates New Flex Ingress UDF(User Defined Field) Mode
 * Parameters:
 *      unit                    - (IN) unit number
 *      udf_pkt_attr_selectors  - (IN) Flex attributes
 *      mode                    - (OUT) Flex mode
 *      total_counters          - (OUT) Total Counters associated with new
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *       Not being used currently
 */
static bcm_error_t 
_bcm_esw_stat_flex_create_egress_udf_mode(
    int                                    unit,
    bcm_stat_flex_udf_pkt_attr_selectors_t *udf_pkt_attr_selectors,
    bcm_stat_flex_mode_t                   *mode,
    uint32                                 *total_counters)
{
    bcm_stat_flex_mode_t                   mode_l=0;
    uint32                                 total_udf_bits=0;
    uint32                                 index=0;
    bcm_stat_flex_egress_mode_t            flex_egress_mode;
    bcm_stat_flex_udf_pkt_attr_selectors_t udf_pkt_attr_selectors_l;
    bcm_stat_flex_udf_pkt_attr_bits_t      udf_pkt_attr_bits_l;
    uint32                                 num_flex_egress_pools;

    num_flex_egress_pools = SOC_INFO(unit).num_flex_egress_pools;

    for (index =0 ; index < BCM_STAT_FLEX_COUNTER_MAX_MODE ; index++) {
         if (_bcm_esw_stat_flex_get_egress_mode_info(
             unit,
             index,
             &flex_egress_mode) == BCM_E_NONE) {
             if (flex_egress_mode.egr_attr.packet_attr_type !=
                 bcmStatFlexPacketAttrTypeUdf) {
                 continue;
             }
             udf_pkt_attr_selectors_l = flex_egress_mode.egr_attr.
                                        udf_pkt_attr_selectors;
             udf_pkt_attr_bits_l = udf_pkt_attr_selectors_l.udf_pkt_attr_bits;
             if ((udf_pkt_attr_bits_l.udf0==
                  udf_pkt_attr_selectors->udf_pkt_attr_bits.udf0) &&
                 (udf_pkt_attr_bits_l.udf1==
                  udf_pkt_attr_selectors->udf_pkt_attr_bits.udf1)) {
                  *total_counters = flex_egress_mode.total_counters;
                  *mode=index;
                  return BCM_E_EXISTS;
             }
         }
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_get_available_mode(
                        unit,
                        bcmStatFlexDirectionEgress,&mode_l));
    /* Step2: Packet Attribute filling */
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_udf_selector_keys(
                        unit,
                        bcmStatFlexDirectionEgress,
                        _egress_pkt_selector_key_reg[mode_l],
                        udf_pkt_attr_selectors,
                        &total_udf_bits));
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_udf_selector_keys(
                        unit,
                        bcmStatFlexDirectionEgress,
                        _egress_pkt_selector_key_reg[mode_l+4],
                        udf_pkt_attr_selectors,
                        &total_udf_bits));
    /* Step3: Offset table filling */
    for (index=0;index < num_flex_egress_pools ; index++) {
         BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_update_offset_table(
                             unit,
                             bcmStatFlexDirectionEgress,
                             ING_FLEX_CTR_OFFSET_TABLE_0m+index,
                             mode_l,
                             256,
                             NULL));
    }
    /* Step4: Final: reserve mode and return */
    *total_counters = (1 << total_udf_bits);
    *mode=mode_l;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_create_egress_mode
 * Description:
 *      Creates New Flex Egress Mode(Either uncompressd or compressed or udf)
 *
 * Parameters:
 *      unit      - (IN) unit number
 *      ing_attr  - (IN) Flex Ingress attributes
 *      mode      - (OUT) Flex mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

bcm_error_t _bcm_esw_stat_flex_create_egress_mode (
            int unit,
            bcm_stat_flex_egr_attr_t *egr_attr,
            bcm_stat_flex_mode_t *mode)
{
    uint32        total_counters=0;
    switch(egr_attr->packet_attr_type) {
    case bcmStatFlexPacketAttrTypeUncompressed:
         FLEXCTR_VVERB(("Creating Egress uncompressed mode \n"));
         BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_create_egress_uncompress_mode(
                             unit,
                             &(egr_attr->uncmprsd_attr_selectors),
                             mode,
                             &total_counters));
         break;
    case bcmStatFlexPacketAttrTypeCompressed:
         FLEXCTR_VVERB(("Creating Egress compressed mode \n"));
         BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_create_egress_compress_mode(
                             unit,
                             &(egr_attr->cmprsd_attr_selectors),
                             mode,
                             &total_counters));
         break;
    case bcmStatFlexPacketAttrTypeUdf:
         FLEXCTR_VVERB(("Creating Egress udf mode \n"));
         BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_create_egress_udf_mode(
                             unit,
                             &(egr_attr->udf_pkt_attr_selectors),
                             mode,
                             &total_counters));
         break;
    default:
         return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(_bcm_esw_stat_flex_egress_reserve_mode(
                        unit,
                        *mode,
                        total_counters,
                        egr_attr));
    /* *mode |= 0x80; */
    FLEXCTR_VVERB(("\n Done %d \n",*mode));
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_stat_flex_delete_egress_mode
 * Description:
 *      Destroys New Flex Egress Mode(Either uncompressd or compressed or udf)
 *
 * Parameters:
 *      unit  - (IN) unit number
 *      mode  - (IN) Flex mode
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *
 */

bcm_error_t _bcm_esw_stat_flex_delete_egress_mode(
            int                  unit,
            bcm_stat_flex_mode_t mode)
{
    if (mode > (BCM_STAT_FLEX_COUNTER_MAX_MODE-1)) {
        /* Exceeding max allowed value : (BCM_STAT_FLEX_COUNTER_MAX_MODE-1) */
        return BCM_E_PARAM;
    }
    return _bcm_esw_stat_flex_unreserve_mode(
           unit,
           bcmStatFlexDirectionEgress,
           mode);
}
