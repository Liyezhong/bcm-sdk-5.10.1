/*
 * $Id: bcm53600_service.h 1.2 Broadcom SDK $
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
 */
     
#ifndef _BCM53600_SERVICE_H
#define _BCM53600_SERVICE_H

extern drv_if_t drv_bcm53600_services;



int drv_bcm53600_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags);
int drv_bcm53600_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base);
int drv_bcm53600_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base);
int drv_bcm53600_cfp_range_set(int unit, uint32 type, uint32 id, uint32 param1, uint32 param2);
int drv_bcm53600_cfp_range_get(int unit, uint32 type, uint32 id, uint32 *param1, uint32 *param2);



int _vo_cfp_udf_value_set(int unit, uint32 udf_idx, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask);
int _vo_cfp_udf_value_get(int unit, uint32 udf_idx, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask);
int _vo_cfp_qual_value_set(int unit, drv_field_qualify_t qual, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask);
int _vo_cfp_qual_value_get(int unit, drv_field_qualify_t qual, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask);

int _vo_qset_to_cfp(int unit, drv_field_qset_t qset, drv_cfp_entry_t * drv_entry, int mode);
int _vo_fp_entry_tcam_chain_mode_get(int unit, int stage_id, void *drv_entry, int sliceId, void *mode);
int _vo_fp_entry_tcam_slice_id_set(int unit, int stage_id, void *entry, int sliceId, void *slice_map);

int
_vo_cfp_data_mask_write(int unit, uint32 ram_type, uint32 index, drv_cfp_entry_t *cfp_entry);
int
_vo_cfp_data_mask_read(int unit, uint32 ram_type, uint32 index, drv_cfp_entry_t *cfp_entry);
int
_vo_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, drv_cfp_entry_t* entry, uint32* fld_val);
int
_vo_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int
_vo_fp_entry_cfp_tcam_policy_install(int unit, void *entry, int tcam_idx, 
        int tcam_chain_idx);

int _vo_cfp_mem_hw2sw(int unit, uint32 *hw_entry, uint32 *sw_entry);
int _vo_cfp_mem_sw2hw(int unit, uint32 *sw_entry, uint32 *hw_entry);
int _vo_fp_cfp_qualify_support(int unit, drv_field_qset_t qset);
#endif
