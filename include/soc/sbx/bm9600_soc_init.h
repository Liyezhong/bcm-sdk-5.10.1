/*
 * $Id: bm9600_soc_init.h 1.20 Broadcom SDK $
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
 * ============================================================
 * == bm9600_soc_init.h - BM9600 Initialization defines          ==
 * ============================================================
 */

#ifndef _BM9600_SOC_INIT_H
#define _BM9600_SOC_INIT_H

#include "sbTypesGlue.h"
#include <soc/sbx/sbx_drv.h>


int
soc_bm9600_init(int unit,
		soc_sbx_config_t *cfg);

void
soc_bm9600_isr(void *_unit);

int
soc_bm9600_port_info_config(int unit);

int
soc_bm9600_bwp_read(int unit,
		    int32 queue,
		    uint32 *pgamma,
		    uint32 *psigma);

int
soc_bm9600_bw_mem_write(int unit,
			int32 bw_table_id,
			int32 repository,
			int32 addr,
			uint32 write_data);

int
soc_bm9600_eset_set(int nUnit,
		    uint32 uEset,
		    uint64 uLowNodesMask,
                    uint32 uHiNodesMask,
		    uint32 uMcFullEvalMin,
                    uint32 uEsetFullStatusMode);

int
soc_bm9600_eset_get(int nUnit,
		    uint32 uEset,
		    uint64 *pLowNodesMask,
                    uint32 *pHiNodesMask,
		    uint32 *pMcFullEvalMin,
                    uint32 *pEsetFullStatusMode);

void
soc_bm9600_config_all_linkdriver(int unit);

int
soc_bm9600_config_linkdriver(int unit,
			     int port);

int
soc_bm9600_bwp_write(int unit,
                     int queue,
                     uint gamma,
                     uint sigma);

int
soc_bm9600_bag_write(int unit,
		     int bw_group,
		     uint32 num_queues_in_bag,
		     uint32 base_queue,
		     uint32 bag_rate_bytes_per_epoch);

int
soc_bm9600_bag_read(int unit,
		    int bw_group,
		    uint32 *pnum_queues_in_bag,
		    uint32 *pbase_queue,
		    uint32 *pbag_rate_bytes_per_epoch);

int
soc_bm9600_nm_sysport_array_table_write(uint32 unit,
					int32 portset,
					int32 offset,
					int32 sysport,
					int32 new_row);

int
soc_bm9600_nm_sysport_array_table_read(uint32 unit,
				       int32 portset,
				       int32 offset,
				       int32 *sysport);

int
soc_bm9600_portset_link_table_write(uint32 unit,
				    int32 portset,
				    int32 next);

int
soc_bm9600_portset_link_table_read(uint32 unit,
				   int32 portset,
				   int32 *next);
int
soc_bm9600_portset_info_table_write(uint32 unit,
				    int32 portset,
				    int32 virtual,
				    int32 eopp,
				    int32 start_port,
				    int32 eg_node);

int
soc_bm9600_portset_info_table_read(uint32 unit,
				    int32 portset,
				    int32 *p_virtual,
				    int32 *p_eopp,
				    int32 *p_start_port,
				   int32 *p_eg_node);

int
soc_bm9600_ina_sysport_map_table_write_all(uint32 unit,
					   int32 sysport,
					   int32 portset,
					   int32 offset);

int
soc_bm9600_ina_sysport_map_table_read(uint32 unit,
				      int32 sysport,
				      int32 *portset,
				      int32 *offset);

int
soc_bm9600_move_sysport_pri(uint32 unit,
			    int32  sysport,
                            int32  old_portset,
                            int32  old_offset,
                            int32  new_portset,
                            int32  new_offset);
int
soc_bm9600_portpri_init(uint32 unit,
			int32  portset);


int soc_bm9600_mdio_hc_write(int unit,
			     uint32 uPhyAddr,
			     uint32 uLane,
			     uint32 uRegAddr,
			     uint32 uData);

int soc_bm9600_mdio_hc_read(int unit,
			    uint32 uPhyAddr,
			    uint32 uLane,
			    uint32 uRegAddr,
			    uint32 *pReadData);

int soc_bm9600_hc_speed_set(int unit, 
			    int32 nSi,
			    uint32 uSerdesSpeed);

int soc_bm9600_hc_speed_get(int unit, 
			    int32 nSi, 
			    uint32 *pSerdesSpeed);

int soc_bm9600_hc_encoding_set(int unit,
			       int32 nSi,
			       int32 bSerdesEncoding);

int soc_bm9600_features(int unit, soc_feature_t feature);

extern int soc_bm9600_mdio_hc_cl22_read(int unit, uint32 uPhyAddr, 
                                        uint32 uRegAddr, uint32 *pReadData);
extern int soc_bm9600_mdio_hc_cl22_write(int unit, uint32 uPhyAddr, 
                                         uint32 uRegAddr, uint32 uData);

extern int soc_bm9600_mdio_hc_cl22_write_easy_reload(int unit, uint32 uPhyAddr, 
						     uint32 uRegAddr, uint32 uData);


extern int soc_bm9600_xb_test_pkt_get(int unit, int egress, int xb_port, int *cnt);

extern int soc_bm9600_xb_test_pkt_clear(int unit, int egress, int xb_port);

extern int soc_bm9600_port_check_policing_errors_reset_xb_port(int unit, uint32 *xb_iport_channel_ptr);
extern soc_driver_t soc_driver_bm9600_a0;
extern soc_driver_t soc_driver_bm9600_b0;




/* status */
#define HW_BM9600_STATUS_OK_K                            (0)
#define HW_BM9600_STATUS_INIT_BM9600_BAD_CHIP_REV_K      (1)
#define HW_BM9600_STATUS_INIT_BIST_TIMEOUT_K             (2)
#define HW_BM9600_STATUS_INIT_BM9600_BIST_BW_UNREPAIR_K  (3)
#define HW_BM9600_STATUS_INIT_BM9600_BIST_BW_TIMEOUT_K   (4)
#define HW_BM9600_STATUS_INDIRECT_ACCESS_TIMEOUT_K       (5)
#define HW_BM9600_STATUS_INIT_BM9600_SER_TIMEOUT_K       (6)
#define HW_BM9600_STATUS_INIT_BM9600_BW_TIMEOUT_K        (7)
#define HW_BM9600_STATUS_INIT_EPOCH_LENGTH_INVALID_K     (8)

/* Fix for bug 23436, resolution was wrong */
#define HW_BM9600_10_USEC_K       (10000)
#define HW_BM9600_100_USEC_K     (100000)
#define HW_BM9600_1_MSEC_K      (1000000)
#define HW_BM9600_10_MSEC_K    (10000000)
#define HW_BM9600_100_MSEC_K  (100000000)
#define HW_BM9600_500_MSEC_K  (500000000)
#define HW_BM9600_1_SEC_K    (1000000000)

#define HW_BM9600_CLOCK_SPEED_IN_HZ     (250000000)

#endif /* _BM9600_SOC_INIT_H */
