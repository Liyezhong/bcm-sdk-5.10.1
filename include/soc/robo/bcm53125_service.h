/*
 * $Id: bcm53125_service.h 1.2 Broadcom SDK $
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
 
#ifndef _BCM53125_SERVICE_H
#define _BCM53125_SERVICE_H

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>

int drv_bcm53125_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_bcm53125_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_bcm53125_eav_control_set(int unit, uint32 type, uint32 param);
int drv_bcm53125_eav_control_get(int unit, uint32 type, uint32 *param);
int drv_bcm53125_eav_enable_set(int unit, uint32 port, uint32 enable);
int drv_bcm53125_eav_enable_get(int unit, uint32 port, uint32 *enable);
int drv_bcm53125_eav_link_status_set(int unit, uint32 port, uint32 link);
int drv_bcm53125_eav_link_status_get(int unit, uint32 port, uint32 *link);
int drv_bcm53125_eav_egress_timestamp_get(int unit, uint32 port,
    uint32 *timestamp);
int drv_bcm53125_eav_time_sync_set(int unit, uint32 type, uint32 p0, uint32 p1);
int drv_bcm53125_eav_time_sync_get(int unit, uint32 type, uint32 *p0, uint32 *p1);
int drv_bcm53125_eav_queue_control_set(int unit, uint32 port, uint32 type, uint32 param);
int drv_bcm53125_eav_queue_control_get(int unit, uint32 port, uint32 type, uint32 *param);
int drv_bcm53125_eav_time_sync_mac_set(int unit, uint8 * mac, uint16 ethertype);
int drv_bcm53125_eav_time_sync_mac_get(int unit, uint8 * mac, uint16 *ethertype);
int drv_bcm53125_fp_tcam_parity_check(int unit,
                    drv_fp_tcam_checksum_t *drv_fp_tcam_chksum);
                    

extern drv_if_t drv_bcm53125_services;

#endif