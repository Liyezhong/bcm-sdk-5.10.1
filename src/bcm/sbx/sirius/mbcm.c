/*
 * $Id: mbcm.c 1.48.2.3 Broadcom SDK $
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
 * File:        mbcm.c
 */

#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/sirius.h>

mbcm_sbx_functions_t mbcm_sirius_driver = {
    /* modid/nodeid functions */
    bcm_sirius_stk_modid_set,
    bcm_sirius_stk_modid_get,
    bcm_sirius_stk_my_modid_set,
    bcm_sirius_stk_my_modid_get,
    bcm_sirius_stk_module_enable,
    bcm_sirius_stk_module_protocol_set,
    bcm_sirius_stk_module_protocol_get,
    bcm_sirius_stk_fabric_map_set,
    bcm_sirius_stk_fabric_map_get,

    /* fabric control functions */
    bcm_sirius_fabric_crossbar_connection_set,
    bcm_sirius_fabric_crossbar_connection_get,
    bcm_sirius_fabric_tdm_enable_set,
    bcm_sirius_fabric_tdm_enable_get,
    bcm_sirius_fabric_calendar_max_get,
    bcm_sirius_fabric_calendar_size_set,
    bcm_sirius_fabric_calendar_size_get,
    bcm_sirius_fabric_calendar_set,
    bcm_sirius_fabric_calendar_get,
    bcm_sirius_fabric_calendar_multi_set,
    bcm_sirius_fabric_calendar_multi_get,
    NULL, /* bcm_sirius_fabric_calendar_active */
    bcm_sirius_fabric_crossbar_mapping_set,
    bcm_sirius_fabric_crossbar_mapping_get,
    bcm_sirius_fabric_crossbar_enable_set,
    bcm_sirius_fabric_crossbar_enable_get,
    bcm_sirius_fabric_crossbar_status_get,
    bcm_sirius_fabric_control_set,
    bcm_sirius_fabric_control_get,
    bcm_sirius_fabric_port_create,
    bcm_sirius_fabric_port_destroy,
    bcm_sirius_fabric_congestion_size_set,
    bcm_sirius_fabric_congestion_size_get,

    /* voq functions */
    bcm_sirius_cosq_init,
    bcm_sirius_cosq_detach,
    bcm_sirius_cosq_add_queue,
    bcm_sirius_cosq_delete_queue,
    bcm_sirius_cosq_enable_queue,
    bcm_sirius_cosq_disable_queue,
    bcm_sirius_cosq_enable_fifo,
    bcm_sirius_cosq_disable_fifo,
    bcm_sirius_cosq_enable_get,
    bcm_sirius_cosq_overlay_queue,
    bcm_sirius_cosq_delete_overlay_queue,
    bcm_sirius_cosq_set_ingress_params,
    bcm_sirius_cosq_set_ingress_shaper,
    bcm_sirius_cosq_set_template_gain,
    NULL,
    bcm_sirius_cosq_set_template_pfc,
    bcm_sirius_cosq_get_template_pfc,
    bcm_sirius_cosq_gport_discard_set,
    bcm_sirius_cosq_gport_discard_get,
    bcm_sirius_cosq_gport_stat_enable_set,
    bcm_sirius_cosq_gport_stat_enable_get,
    bcm_sirius_cosq_gport_stat_set,
    bcm_sirius_cosq_gport_stat_get,
    bcm_sirius_cosq_gport_stat_config_set,
    bcm_sirius_cosq_gport_stat_config_get,
    bcm_sirius_cosq_gport_statistic_set,
    bcm_sirius_cosq_gport_statistic_get,
    bcm_sirius_cosq_gport_statistic_multi_set,
    bcm_sirius_cosq_gport_statistic_multi_get,
    bcm_sirius_cosq_attach_scheduler,
    bcm_sirius_cosq_detach_scheduler,
    bcm_sirius_cosq_scheduler_attach_get,
    bcm_sirius_cosq_set_egress_scheduler_params,
    bcm_sirius_cosq_get_egress_scheduler_params,
    bcm_sirius_cosq_set_egress_shaper_params,
    bcm_sirius_cosq_get_egress_shaper_params,
    bcm_sirius_cosq_set_ingress_scheduler_params,
    bcm_sirius_cosq_get_ingress_scheduler_params,
    bcm_sirius_cosq_set_ingress_shaper_params,
    bcm_sirius_cosq_get_ingress_shaper_params,
    bcm_sirius_cosq_control_set,
    bcm_sirius_cosq_control_get,
    bcm_sirius_cosq_target_set,
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    bcm_sirius_cosq_state_get,
#endif
#endif
    bcm_sirius_cosq_egress_size_set,
    bcm_sirius_cosq_egress_size_get,
    bcm_sirius_cosq_flow_control_set,
    bcm_sirius_cosq_flow_control_get,
    bcm_sirius_cosq_pfc_config_set,
    bcm_sirius_cosq_pfc_config_get,
    bcm_sirius_cosq_port_congestion_set,
    bcm_sirius_cosq_port_congestion_get,
    NULL, /* bcm_sirius_cosq_gport_sched_config_set */
    NULL, /* bcm_sirius_cosq_gport_sched_config_get */
    bcm_sirius_cosq_scheduler_allocate,
    bcm_sirius_cosq_scheduler_free,
    bcm_sirius_cosq_gport_queue_attach,
    bcm_sirius_cosq_gport_queue_attach_get,
    bcm_sirius_cosq_gport_queue_detach,
    bcm_sirius_cosq_mapping_set,
    bcm_sirius_cosq_mapping_get,
    bcm_sirius_cosq_multipath_allocate,
    bcm_sirius_cosq_multipath_free,
    bcm_sirius_cosq_multipath_add,
    bcm_sirius_cosq_multipath_delete,
    bcm_sirius_cosq_multipath_get,
  
    /* multicast functions */
    bcm_sirius_fabric_distribution_create,
    bcm_sirius_fabric_distribution_destroy,
    bcm_sirius_fabric_distribution_set,
    bcm_sirius_fabric_distribution_get,
    NULL /* bcm_sirius_fabric_distribution_control_set */,
    NULL /* bcm_sirius_fabric_distribution_control_get */,
    bcm_sirius_fabric_packet_adjust_set,
    bcm_sirius_fabric_packet_adjust_get,
    bcm_sirius_vlan_control_vlan_set,
    bcm_sirius_vlan_init,
    bcm_sirius_vlan_create,
    bcm_sirius_vlan_port_add,
    bcm_sirius_vlan_port_remove,
    bcm_sirius_vlan_destroy,
    bcm_sirius_vlan_destroy_all,
    bcm_sirius_vlan_port_get,
    bcm_sirius_vlan_list,
    bcm_sirius_vlan_list_by_pbmp,
    bcm_sirius_vlan_list_destroy,
    bcm_sirius_vlan_default_set,
    bcm_sirius_vlan_default_get,
    bcm_sirius_multicast_init,
    bcm_sirius_multicast_detach,
    bcm_sirius_multicast_create,
    bcm_sirius_multicast_destroy,
    bcm_sirius_multicast_group_get,
    bcm_sirius_multicast_group_traverse,
    bcm_sirius_multicast_egress_add,
    bcm_sirius_multicast_egress_delete,
    bcm_sirius_multicast_egress_subscriber_add,
    bcm_sirius_multicast_egress_subscriber_delete,
    bcm_sirius_multicast_egress_delete_all,
    bcm_sirius_multicast_egress_set,
    bcm_sirius_multicast_egress_get,
    bcm_sirius_multicast_egress_subscriber_set,
    bcm_sirius_multicast_egress_subscriber_get,
    bcm_sirius_multicast_fabric_distribution_set,
    bcm_sirius_multicast_fabric_distribution_get,
#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
    bcm_sirius_multicast_state_get,
#endif
#endif

    /* port functions */
    bcm_sirius_port_init,
    bcm_sirius_port_enable_set,
    bcm_sirius_port_enable_get,
    bcm_sirius_port_speed_set,
    bcm_sirius_port_speed_get,
    NULL /* bcm_bm3200_port_frame_max_set */,
    NULL /* bcm_bm3200_port_frame_max_get */,
    bcm_sirius_port_link_status_get,
    bcm_sirius_port_loopback_set,
    bcm_sirius_port_loopback_get,
    bcm_sirius_port_control_set,
    bcm_sirius_port_control_get,
    NULL /* bcm_sirius_port_linkscan_set */,
    NULL /* bcm_sirius_port_linkscan_get */,
    NULL /* bcm_sirius_port_rate_egress_shaper_set */,
    NULL /* bcm_sirius_port_rate_egress_shaper_get */,
    NULL /* bcm_sirius_port_rate_egress_traffic_set */,
    NULL /* bcm_sirius_port_rate_egress_traffic_get */,
    bcm_sirius_port_probe,
    bcm_sirius_port_ability_get,
    bcm_sirius_port_congestion_config_set,
    bcm_sirius_port_congestion_config_get,
    bcm_sirius_port_scheduler_get,
    bcm_sirius_port_is_egress_multicast,
    bcm_sirius_port_egress_multicast_scheduler_get,
    bcm_sirius_port_egress_multicast_group_get,

    bcm_sirius_trunk_init,
    bcm_sirius_trunk_create,
    bcm_sirius_trunk_create_id,
    bcm_sirius_trunk_destroy,
    bcm_sirius_trunk_detach,
    bcm_sirius_trunk_find,
    bcm_sirius_trunk_get,
    bcm_sirius_trunk_chip_info_get,
    bcm_sirius_trunk_set,

    /* stat functions */
    bcm_sirius_stat_init,
    bcm_sirius_stat_sync,
    bcm_sirius_stat_get,
    bcm_sirius_stat_get32,
    bcm_sirius_stat_multi_get,
    bcm_sirius_stat_multi_get32,
    bcm_sirius_stat_clear,
    NULL,  /*bcm_sirius_stat_scoreboard_get */
    bcm_sirius_stat_custom_set,
    bcm_sirius_stat_custom_get,
    bcm_sirius_stat_custom_add,
    bcm_sirius_stat_custom_delete,
    bcm_sirius_stat_custom_delete_all,
    bcm_sirius_stat_custom_check,

    /* switch functions */

    bcm_sirius_switch_control_set,
    bcm_sirius_switch_control_get,
    NULL, /* bcm_sirius_switch_event_register */
    NULL, /* bcm_sirius_switch_event_unregister */

    /* subscriber map functions */

    bcm_sirius_cosq_subscriber_map_add,   
    bcm_sirius_cosq_subscriber_map_delete,
    bcm_sirius_cosq_subscriber_map_delete_all,
    bcm_sirius_cosq_subscriber_map_get,
    bcm_sirius_cosq_subscriber_traverse,

    /* failover functions */
    bcm_sirius_failover_enable,
    bcm_sirius_failover_set,
    bcm_sirius_failover_destroy,

    /* frame steering functions */
    bcm_sirius_stk_steering_unicast_set,
    bcm_sirius_stk_steering_multicast_set,
    bcm_sirius_stk_steering_clear,
    bcm_sirius_stk_steering_clear_all,

    /* predicate control */
    bcm_sirius_fabric_predicate_create,
    bcm_sirius_fabric_predicate_destroy,
    bcm_sirius_fabric_predicate_destroy_all,
    bcm_sirius_fabric_predicate_get,
    bcm_sirius_fabric_predicate_traverse,

    /* parser (action) control */
    bcm_sirius_fabric_action_create,
    bcm_sirius_fabric_action_destroy,
    bcm_sirius_fabric_action_destroy_all,
    bcm_sirius_fabric_action_get,
    bcm_sirius_fabric_action_traverse,

    /* type resolution (predicate_action) control */
    bcm_sirius_fabric_predicate_action_create,
    bcm_sirius_fabric_predicate_action_get,
    bcm_sirius_fabric_predicate_action_destroy,
    bcm_sirius_fabric_predicate_action_destroy_all,
    bcm_sirius_fabric_predicate_action_traverse,

    /* QUEUE_MAP (qsel) control */
    bcm_sirius_fabric_qsel_create,
    bcm_sirius_fabric_qsel_destroy,
    bcm_sirius_fabric_qsel_destroy_all,
    bcm_sirius_fabric_qsel_get,
    bcm_sirius_fabric_qsel_traverse,
    bcm_sirius_fabric_qsel_entry_set,
    bcm_sirius_fabric_qsel_entry_get,
    bcm_sirius_fabric_qsel_entry_multi_set,
    bcm_sirius_fabric_qsel_entry_multi_get,
    bcm_sirius_fabric_qsel_entry_traverse,

    /* COS_MAP (qsel_offset) control */
    bcm_sirius_fabric_qsel_offset_create,
    bcm_sirius_fabric_qsel_offset_destroy,
    bcm_sirius_fabric_qsel_offset_destroy_all,
    bcm_sirius_fabric_qsel_offset_traverse,
    bcm_sirius_fabric_qsel_offset_entry_set,
    bcm_sirius_fabric_qsel_offset_entry_get,
    bcm_sirius_fabric_qsel_offset_entry_traverse,

    /* Flow Control */
    bcm_sirius_fd_fct_get,
};
