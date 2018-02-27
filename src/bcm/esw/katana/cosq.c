/*
 * $Id: cosq.c 1.13.2.12 Broadcom SDK $
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
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 */

#include <sal/core/libc.h>

#include <soc/defs.h>
#if defined(BCM_KATANA_SUPPORT)
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/katana.h>
#include <soc/profile_mem.h>
#include <soc/debug.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
	
#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/katana.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/cosq.h>

#include <bcm_int/esw_dispatch.h>

#include <bcm/types.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#define KT_CELL_FIELD_MAX       0xffff

/* MMU cell size in bytes */
#define _BCM_KT_COSQ_CELLSIZE   192

#define _BCM_KT_NUM_UCAST_QUEUE_GROUP            8
#define _BCM_KT_NUM_VLAN_UCAST_QUEUE_GROUP       16

#define _BCM_KT_NUM_PORT_SCHEDULERS              36
#define _BCM_KT_NUM_L0_SCHEDULER                 256
#define _BCM_KT_NUM_L1_SCHEDULER                 1024
#define _BCM_KT_NUM_L2_QUEUES                    4096
#define _BCM_KT_NUM_TOTAL_SCHEDULERS             (_BCM_KT_NUM_PORT_SCHEDULERS + \
                                                  _BCM_KT_NUM_L0_SCHEDULER +    \
                                                  _BCM_KT_NUM_L1_SCHEDULER)

#define _BCM_KT_COSQ_LIST_NODE_INIT(list, node)         \
    list[node].in_use            = FALSE;               \
    list[node].wrr_in_use        = 0;                   \
    list[node].gport             = -1;                  \
    list[node].base_index        = -1;                  \
    list[node].numq              = 0;                   \
    list[node].hw_index          = -1;                  \
    list[node].level             = -1;                  \
    list[node].cosq_attached_to  = -1;                  \
    list[node].num_child         = 0;                   \
    list[node].first_child       = 4095;                \
    list[node].parent            = NULL;                \
    list[node].sibling           = NULL;                \
    list[node].child             = NULL;

#define _BCM_KT_COSQ_NODE_INIT(node)                    \
    node->in_use            = FALSE;                    \
    node->wrr_in_use        = 0;                        \
    node->gport             = -1;                       \
    node->base_index        = -1;                       \
    node->numq              = 0;                        \
    node->hw_index          = -1;                       \
    node->level             = -1;                       \
    node->cosq_attached_to  = -1;                       \
    node->num_child         = 0;                        \
    node->first_child       = 4095;                     \
    node->parent            = NULL;                     \
    node->sibling           = NULL;                     \
    node->child             = NULL;

typedef enum {
    _BCM_KT_COSQ_INDEX_STYLE_BUCKET,
    _BCM_KT_COSQ_INDEX_STYLE_WRED,
    _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
    _BCM_KT_COSQ_INDEX_STYLE_UCAST_QUEUE,
    _BCM_KT_COSQ_INDEX_STYLE_PERQ_XMT
}_bcm_kt_cosq_index_style_t;

typedef enum {
    _BCM_KT_COSQ_NODE_LEVEL_ROOT,
    _BCM_KT_COSQ_NODE_LEVEL_L0,
    _BCM_KT_COSQ_NODE_LEVEL_L1,
    _BCM_KT_COSQ_NODE_LEVEL_L2,
    _BCM_KT_COSQ_NODE_LEVEL_MAX
}_bcm_kt_cosq_node_level_t;

typedef struct _bcm_kt_cosq_node_s{
    struct _bcm_kt_cosq_node_s *parent;
    struct _bcm_kt_cosq_node_s *sibling;
    struct _bcm_kt_cosq_node_s *child;
    bcm_gport_t gport;
    int in_use;
    int wrr_in_use;
    int base_index;
    int numq;
    int hw_index;
    _bcm_kt_cosq_node_level_t level;
    int cosq_attached_to;
    int num_child;
    int first_child;
}_bcm_kt_cosq_node_t;

typedef struct _bcm_kt_cosq_list_s {
    int count;
    SHR_BITDCL *bits;
}_bcm_kt_cosq_list_t;

typedef struct _bcm_kt_cosq_port_info {
    int q_offset;
    int q_limit;
}_bcm_kt_cosq_port_info_t;

typedef struct _bcm_kt_mmu_info_s {
    int num_base_queues;   /* Number of classical queues */
    int num_ext_queues;    /* Number of extended queues */
    int qset_size;         /* subscriber queue set size */
    uint32 num_queues;     /* total number of queues */
    uint32 num_nodes;      /* max number of sched nodes */
    uint32 max_nodes[_BCM_KT_COSQ_NODE_LEVEL_MAX]; /* max nodes in each level */

    _bcm_kt_cosq_port_info_t *port;     /* list of base queues */
    _bcm_kt_cosq_list_t ext_qlist;      /* list of extended queues */
    _bcm_kt_cosq_list_t sched_list;     /* list of sched nodes */
    _bcm_kt_cosq_list_t l0_sched_list;  /* list of l0 sched nodes */
    _bcm_kt_cosq_list_t l1_sched_list;  /* list of l1 sched nodes */
    _bcm_kt_cosq_list_t l2_base_qlist;  /* list of l2  base queue index list */
    _bcm_kt_cosq_list_t l2_ext_qlist;   /* list of l2  base queue index list */

    _bcm_kt_cosq_node_t sched_node[_BCM_KT_NUM_TOTAL_SCHEDULERS];     /* sched nodes */
    _bcm_kt_cosq_node_t queue_node[_BCM_KT_NUM_L2_QUEUES];            /* queue nodes */
}_bcm_kt_mmu_info_t;

STATIC _bcm_kt_mmu_info_t *_bcm_kt_mmu_info[BCM_MAX_NUM_UNITS];  /* MMU info */

STATIC soc_profile_mem_t *_bcm_kt_cos_map_profile[BCM_MAX_NUM_UNITS];
STATIC soc_profile_mem_t *_bcm_kt_wred_profile[BCM_MAX_NUM_UNITS];

/* Number of COSQs configured for this device */
STATIC int _bcm_kt_num_cosq[SOC_MAX_NUM_DEVICES];

/*
 * Function:
 *     _bcm_kt_cosq_localport_resolve
 * Purpose:
 *     Resolve GRPOT if it is a local port
 * Parameters:
 *     unit       - (IN) unit number
 *     gport      - (IN) GPORT identifier
 *     local_port - (OUT) local port number
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_localport_resolve(int unit, bcm_gport_t gport,
                               bcm_port_t *local_port)
{
    bcm_module_t module;
    bcm_port_t port;
    bcm_trunk_t trunk;
    int id, result;

    if (!BCM_GPORT_IS_SET(gport)) {
        if (!SOC_PORT_VALID(unit, gport)) {
            return BCM_E_PORT;
        }
        *local_port = gport;
        return BCM_E_NONE;
    } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_SCHEDULER(gport)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_esw_gport_resolve(unit, gport, &module, &port, &trunk, &id));

    BCM_IF_ERROR_RETURN(_bcm_esw_modid_is_local(unit, module, &result));
    if (result == FALSE) {
        return BCM_E_PORT;
    }

    *local_port = port;

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_node_index_get
 * Purpose:
 *     Allocate free index from the given list
 * Parameters:
 *     list       - (IN) bit list
 *     start      - (IN) start index
 *     end        - (IN) end index
 *     qset       - (IN) size of queue set
 *     range      - (IN) range bits
 *     id         - (OUT) start index
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_node_index_get(SHR_BITDCL *list, int start, int end,
                       int qset, int range, int *id)
{
    int i, j, inc;

    *id = -1;

    if (range != qset) {
        inc = 1;
        for (i = start; i < end; i = (i + inc)) {
            for (j = i; j < (i + range); j++, inc++) {
                if (SHR_BITGET(list, j) != 0) {
                    break;
                }
            }

            if (j == (i + range)) {
                *id = i;
                return BCM_E_NONE;
            }
        }
        if (i == end) {
            return BCM_E_RESOURCE;
         }
    }

    for (i = start; i < end;  i = i + range) {
        for (j = i; j < (i + range); j++) {
            if (SHR_BITGET(list, j) != 0) {
                break;
            }
        }

        if (j == (i + range)) {
            *id  =  i;
            return BCM_E_NONE;
           }
    }

    if (i == end) {
        return BCM_E_RESOURCE;
      }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_node_index_set
 * Purpose:
 *     Mark indices as allocated
 * Parameters:
 *     list       - (IN) bit list
 *     start      - (IN) start index
 *     range      - (IN) range bits
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_node_index_set(_bcm_kt_cosq_list_t *list, int start, int range)
{
    list->count += range;
    SHR_BITSET_RANGE(list->bits, start, range);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_node_index_clear
 * Purpose:
 *     Mark indices as free
 * Parameters:
 *     list       - (IN) bit list
 *     start      - (IN) start index
 *     range      - (IN) range bits
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_node_index_clear(_bcm_kt_cosq_list_t *list, int start, int range)
{
    list->count -= range;
    SHR_BITCLR_RANGE(list->bits, start, range);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_alloc_clear
 * Purpose:
 *     Allocate cosq memory and clear
 * Parameters:
 *     size       - (IN) size of memory required
 *     description- (IN) description about the structure
 * Returns:
 *     BCM_E_XXX
 */
STATIC void *
_bcm_kt_cosq_alloc_clear(unsigned int size, char *description)
{
    void *block_p;

    block_p = sal_alloc(size, description);

    if (block_p != NULL) {
        sal_memset(block_p, 0, size);
    }

    return block_p;
}

/*
 * Function:
 *     _bcm_kt_cosq_free_memory
 * Purpose:
 *     Free memory allocated for mmu info members
 * Parameters:
 *     mmu_info_p       - (IN) MMU info pointer
 * Returns:
 *     BCM_E_XXX
 */
STATIC void
_bcm_kt_cosq_free_memory(_bcm_kt_mmu_info_t *mmu_info_p)
{
    sal_free(mmu_info_p->port);
    sal_free(mmu_info_p->ext_qlist.bits);
    sal_free(mmu_info_p->sched_list.bits);
    sal_free(mmu_info_p->l0_sched_list.bits);
    sal_free(mmu_info_p->l1_sched_list.bits);
    sal_free(mmu_info_p->l2_base_qlist.bits);
    sal_free(mmu_info_p->l2_ext_qlist.bits);
}

/*
 * Function:
 *     _bcm_kt_cosq_node_get
 * Purpose:
 *     Get internal information of queue group or scheduler type GPORT
 * Parameters:
 *     unit       - (IN) unit number
 *     gport      - (IN) queue group/scheduler GPORT identifier
 *     modid      - (OUT) module identifier
 *     port       - (OUT) port number
 *     id         - (OUT) queue group or scheduler identifier
 *     node       - (OUT) node structure for the specified GPORT
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_node_get(int unit, bcm_gport_t gport, bcm_module_t *modid,
                      bcm_port_t *port, int *id, _bcm_kt_cosq_node_t **node)
{
    soc_info_t *si;
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *node_base;
    bcm_module_t modid_out = 0;
    bcm_port_t port_out = 0;
    uint32 encap_id;
    int index;

    si = &SOC_INFO(unit);

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &modid_out));
        port_out = BCM_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(gport);
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &modid_out));
        port_out = (gport >> 16) & 0xff;
    } else if (BCM_GPORT_IS_SCHEDULER(gport)) {
        port_out = BCM_GPORT_SCHEDULER_GET(gport) & 0xff;
    } else if (BCM_GPORT_IS_LOCAL(gport)) {
        encap_id = BCM_GPORT_LOCAL_GET(gport);
        port_out = encap_id;
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
        encap_id = BCM_GPORT_MODPORT_PORT_GET(gport);
        port_out = encap_id;
    } else {
        return BCM_E_PORT;
    }

    if (!SOC_PORT_VALID(unit, port_out)) {
        return BCM_E_PORT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        encap_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
        index = encap_id;
        if (index < mmu_info->num_base_queues) {
            node_base = mmu_info->queue_node;
        } else {
            if (si->port_num_ext_cosq[port_out] == 0) {
                return BCM_E_PORT;
            }
            node_base = mmu_info->queue_node;
        }
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        encap_id = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);
        index = encap_id;
        node_base = mmu_info->queue_node;
    }
    else { /* scheduler */
        encap_id = (BCM_GPORT_SCHEDULER_GET(gport) >> 8) & 0x7ff;
        if (encap_id == 0) {
            encap_id = (BCM_GPORT_SCHEDULER_GET(gport) & 0xff);
        }
        index = encap_id;
        if (index >= _BCM_KT_NUM_TOTAL_SCHEDULERS) {
            return BCM_E_PORT;
        }
        node_base = mmu_info->sched_node;
    }

    if (!(BCM_GPORT_IS_LOCAL(gport) || BCM_GPORT_IS_MODPORT(gport)) &&
         node_base[index].numq == 0) {
        return BCM_E_NOT_FOUND;
    }

    if (modid != NULL) {
        *modid = modid_out;
    }
    if (port != NULL) {
        *port = port_out;
    }
    if (id != NULL) {
        *id = encap_id;
    }
    if (node != NULL) {
        *node = &node_base[index];
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_gport_traverse
 * Purpose:
 *     Walks through the valid COSQ GPORTs and calls
 *     the user supplied callback function for each entry.
 * Parameters:
 *     unit       - (IN) unit number
 *     gport      - (IN)
 *     cb         - (IN) Callback function.
 *     user_data  - (IN) User data to be passed to callback function
 * Returns:
 *     BCM_E_NONE - Success.
 *     BCM_E_XXX
 */
int
_bcm_kt_cosq_gport_traverse(int unit, bcm_gport_t gport,
                            bcm_cosq_gport_traverse_cb cb,
                            void *user_data)
{
    _bcm_kt_cosq_node_t *node;
    uint32 flags = BCM_COSQ_GPORT_SCHEDULER;
    int rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, gport, NULL, NULL, NULL, &node));

    if (node != NULL) {
       if (node->level == _BCM_KT_COSQ_NODE_LEVEL_L2) {
          flags = BCM_GPORT_IS_UCAST_QUEUE_GROUP(node->gport) ?
                  BCM_COSQ_GPORT_UCAST_QUEUE_GROUP :
                  BCM_COSQ_GPORT_SUBSCRIBER;
       }
       rv = cb(unit, gport, node->numq, flags,
                  node->gport, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
       if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
           return rv;
       }
#endif
    } else {
            return BCM_E_NONE;
    }

    if (node->sibling != NULL) {
        _bcm_kt_cosq_gport_traverse(unit, node->sibling->gport, cb, user_data);
    }

    if (node->child != NULL) {
        _bcm_kt_cosq_gport_traverse(unit, node->child->gport, cb, user_data);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_index_resolve
 * Purpose:
 *     Find hardware index for the given port/cosq
 * Parameters:
 *     unit       - (IN) unit number
 *     port       - (IN) port number or GPORT identifier
 *     cosq       - (IN) COS queue number
 *     style      - (IN) index style (_BCM_KA_COSQ_INDEX_STYLE_XXX)
 *     local_port - (OUT) local port number
 *     index      - (OUT) result index
 *     count      - (OUT) number of index
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_index_resolve(int unit, bcm_port_t port, bcm_cos_queue_t cosq, \
                           _bcm_kt_cosq_index_style_t style,                 \
                           bcm_port_t *local_port, int *index, int *count)    \
{
    _bcm_kt_cosq_node_t *node, *cur_node;
    bcm_port_t resolved_port;
    int resolved_index;
    int id, startq, numq = 0;
    _bcm_kt_mmu_info_t *mmu_info;

    if (cosq < -1) {
        return BCM_E_PARAM;
    } else if (cosq == -1) {
        startq = 0;
    } else {
        startq = cosq;
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
        BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) || 
        BCM_GPORT_IS_SCHEDULER(port)) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_node_get(unit, port, NULL, &resolved_port, &id,
                                   &node));
        if (node->cosq_attached_to < 0) { /* Not resolved yet */
            return BCM_E_NOT_FOUND;
        }
        if (BCM_GPORT_IS_SCHEDULER(port)) {
            numq = node->numq;
            if (startq >= numq) {
                return BCM_E_PARAM;
            }
        }    
    } else {
        /* optionally validate port */
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_localport_resolve(unit, port, &resolved_port));
        if (resolved_port < 0) {
            return BCM_E_PORT;
        }
        node = NULL;
        numq = 0;
    }

    switch (style) {
    case _BCM_KT_COSQ_INDEX_STYLE_BUCKET:
        if (node != NULL) {
            if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
                BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port)) {
                resolved_index = node->hw_index;
            } else if (BCM_GPORT_IS_SCHEDULER(port)) {
                if (node->level > _BCM_KT_COSQ_NODE_LEVEL_L1 || startq >= numq) {
                    return BCM_E_PARAM;
                }

                for (cur_node = node->child; cur_node != NULL;
                    cur_node = cur_node->sibling) {
                    if (cur_node->cosq_attached_to == startq) {
                        break;
                    }
                }

                if (cur_node == NULL) {
                    /* this cosq is not yet attached */
                    return BCM_E_INTERNAL;
                }

                if (node->numq != -1 && node->numq <= 8 ) {
                    resolved_index = node->base_index + startq;
                } else if (node->numq == -1) {
                    resolved_index = cur_node->hw_index;
                } else {
                    return BCM_E_INTERNAL;
                }
            } else {
                    return BCM_E_PARAM;
            }
        } else { /* node == NULL */
            mmu_info = _bcm_kt_mmu_info[unit];
            numq = mmu_info->port[resolved_port].q_limit - 
                    mmu_info->port[resolved_port].q_offset;
            
            if (cosq >= numq) {
                return BCM_E_PARAM;
            }
            resolved_index = mmu_info->port[resolved_port].q_offset + startq;
        }
        break;

    case _BCM_KT_COSQ_INDEX_STYLE_WRED:
        if (node != NULL) {
            if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
                BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port)) {
                resolved_index = node->base_index + startq;
            } else if (BCM_GPORT_IS_SCHEDULER(port)) {
                if (node->level != _BCM_KT_COSQ_NODE_LEVEL_L1 || startq >= numq) {
                    return BCM_E_PARAM;
                }
                resolved_index = node->hw_index;
            } else {
                    return BCM_E_PARAM;
            }
        } else { /* node == NULL */
            mmu_info = _bcm_kt_mmu_info[unit];
            numq = mmu_info->port[resolved_port].q_limit - 
                   mmu_info->port[resolved_port].q_offset;

            if (cosq >= numq) {
                return BCM_E_PARAM;
            }
            resolved_index = mmu_info->port[resolved_port].q_offset + startq;
        }
        break;

    case _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER:
        if (node != NULL) {
            if (BCM_GPORT_IS_SCHEDULER(port)) {
                for (cur_node = node->child; cur_node != NULL;
                    cur_node = cur_node->sibling) {
                    if (cur_node->cosq_attached_to == startq) {
                        break;
                    }
                }

                if (cur_node == NULL) {
                    /* this cosq is not yet attached */
                    return BCM_E_INTERNAL;
                }

                if (node->numq != -1 && node->numq <= 8) {
                    resolved_index = node->base_index + startq;
                } else if (node->numq == -1) {
                    resolved_index = cur_node->hw_index;
                } else {
                    return BCM_E_INTERNAL;
                }
            } else { /* unicast queue group or multicast queue group */
                return BCM_E_PARAM;
            }
        } else { /* node == NULL */
           return BCM_E_PARAM;
        }
        break;
    case _BCM_KT_COSQ_INDEX_STYLE_UCAST_QUEUE:
        if (node != NULL) {
            if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
                BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port)) {
                resolved_index = node->base_index + startq;
            } else if (BCM_GPORT_IS_SCHEDULER(port)) {
                if (node->level != _BCM_KT_COSQ_NODE_LEVEL_L1 || startq >= numq) {
                    return BCM_E_PARAM;
                }
                resolved_index = node->hw_index;
            } else {
                    return BCM_E_PARAM;
            }
        } else { /* node == NULL */
            return BCM_E_PARAM;
        }
        break;

    default:
        return BCM_E_INTERNAL;
    }

    if (local_port != NULL) {
        *local_port = resolved_port;
    }
    if (index != NULL) {
        *index = resolved_index;
    }
    if (count != NULL) {
        *count = (cosq == -1) ? numq : 1;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_node_resolve
 * Purpose:
 *     Resolve queue number in the specified scheduler tree and its subtree
 * Parameters:
 *     unit       - (IN) unit number
 *     node       - (IN) node structure for the specified scheduler node
 *     cosq       - (IN) COS queue to attach to
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_node_resolve(int unit, _bcm_kt_cosq_node_t *node,
                          bcm_cos_queue_t cosq)
{
    _bcm_kt_cosq_node_t *cur_node, *parent;
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_list_t *list;
    bcm_port_t local_port;
    uint32 map;
    int i;
    int numq, id;
    int offset, limit;

    if (node->parent == NULL) {
        /* Not attached, should never happen */
        return BCM_E_NOT_FOUND;
    }

    if (node->parent->numq == 0 || node->parent->numq < -1 || node->parent->numq > 8) {
        return BCM_E_PARAM;
    }

    if (node->parent->numq == -1 && cosq < 0) {
        return BCM_E_PARAM;
    }

    parent = node->parent;
    if (parent->numq != -1 &&  parent->numq <= 8) {
         /* find the current cosq_attached_to usage */
        map = 0;
        for (cur_node = parent->child; cur_node != NULL;
             cur_node = cur_node->sibling) {
            if (cur_node->cosq_attached_to >= 0) {
                map |= 1 << cur_node->cosq_attached_to;
            }
        }

        if (cosq < 0) {
            for (i = 0; i < parent->numq; i++) {
                if ((map & (1 << i)) == 0) {
                    node->cosq_attached_to = i;
                    break;
                }
            }
            if (i == parent->numq) {
                return BCM_E_PARAM;
            }
            cosq = i;
        } else {
            if (map & (1 << cosq)) {
                return BCM_E_EXISTS;
            }
            node->cosq_attached_to = cosq;
        }
    }

    mmu_info = _bcm_kt_mmu_info[unit];
    numq = (parent->numq == -1) ? 1 : parent->numq;

    switch (parent->level) {
        case _BCM_KT_COSQ_NODE_LEVEL_ROOT:
            if (parent->base_index < 0) {
                list = &mmu_info->l0_sched_list;
                BCM_IF_ERROR_RETURN
                    (_bcm_kt_node_index_get(list->bits, 0, mmu_info->max_nodes[node->level],
                                    mmu_info->qset_size, numq, &id));
                _bcm_kt_node_index_set(list, id, numq);
                parent->base_index = id;
            }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L0:
            if (parent->base_index < 0) {
                list = &mmu_info->l1_sched_list;
                BCM_IF_ERROR_RETURN
                    (_bcm_kt_node_index_get(list->bits, 0, mmu_info->max_nodes[node->level],
                                    mmu_info->qset_size, numq, &id));
                _bcm_kt_node_index_set(list, id, numq);
                parent->base_index = id;
            }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L1:
            if (parent->base_index < 0) {
                if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(node->gport)) {
                    local_port = BCM_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(node->gport);
                    offset = mmu_info->port[local_port].q_offset;
                    limit =  mmu_info->port[local_port].q_limit;
                    list = &mmu_info->l2_base_qlist;
                    BCM_IF_ERROR_RETURN
                        (_bcm_kt_node_index_get(list->bits, offset, limit,
                                    mmu_info->qset_size, numq, &id));
                    _bcm_kt_node_index_set(list, id, numq);
                }else {
                    list = &mmu_info->l2_ext_qlist;
                    BCM_IF_ERROR_RETURN
                        (_bcm_kt_node_index_get(list->bits, 0, mmu_info->num_ext_queues,
                                    mmu_info->qset_size, numq, &id));
                    _bcm_kt_node_index_set(list, id, numq);
                    id += mmu_info->num_base_queues;
                }
                parent->base_index = id;
                node->base_index = id;
            }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L2:
            /* passthru */
        default:
            return BCM_E_PARAM;

    }

    node->hw_index = parent->base_index + node->cosq_attached_to;

    if (parent->numq != -1 &&  parent->numq <= 8) {
        if (parent->level < _BCM_KT_COSQ_NODE_LEVEL_L1) {
            node->hw_index = parent->base_index + cosq;
        }

        parent->num_child++;
        /* get the lowest cosq as a first child */
        if (node->hw_index < parent->first_child) {
            parent->first_child = node->hw_index;
        }
    } else if (parent->numq == -1) {
            /* check duplicate entry */
        for (cur_node = node->sibling; cur_node != NULL;
              cur_node = cur_node->sibling) {
            if (cur_node->cosq_attached_to == cosq) {
                break;
            }
        }

        if (cur_node != NULL) {
            return BCM_E_PARAM;
        }

        if (parent->level < _BCM_KT_COSQ_NODE_LEVEL_L1) {
            node->hw_index = id;
        }
        node->cosq_attached_to = cosq;
        node->parent->num_child++;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_node_unresolve
 * Purpose:
 *     Unresolve queue number in the specified scheduler tree and its subtree
 * Parameters:
 *     unit       - (IN) unit number
 *     node       - (IN) node structure for the specified scheduler node
 *     cosq       - (IN) COS queue to attach to
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_node_unresolve(int unit, _bcm_kt_cosq_node_t *node, bcm_cos_queue_t cosq)
{
    _bcm_kt_cosq_node_t *cur_node, *parent;
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_list_t *list;
    uint32 map;
    int min_index, start_index;
    int id , numq;
    int min_flag = 0;

    if (node->cosq_attached_to < 0) {
        /* Has been unresolved already */
        return BCM_E_NONE;
    }

    parent = node->parent;

    if (parent->numq == 0 || parent->numq < -1 || parent->numq > 8) {
        return BCM_E_PARAM;
    }

    if (parent->numq != -1 &&  parent->numq <= 8) {
         /* find the current cosq_attached_to usage */
        map = 0;
        if (parent->first_child == node->hw_index) {
            min_flag = 1;
            min_index = (node->level == _BCM_KT_COSQ_NODE_LEVEL_L2) ? 4095 :
                        ((node->level == _BCM_KT_COSQ_NODE_LEVEL_L1) ? 1023 : 255);
        } else {
            min_index = parent->first_child;
        }

        for (cur_node = parent->child; cur_node != NULL;
            cur_node = cur_node->sibling) {
            if (cur_node->cosq_attached_to >= 0) {
                map |= 1 << cur_node->cosq_attached_to;
                /* dont update the min node is not this node */
                if (min_flag && cur_node->hw_index != node->hw_index) {
                if (cur_node->hw_index < min_index ) {
                    min_index = cur_node->hw_index;
                }
               }
            }
        }

        if (!(map & (1 << node->cosq_attached_to))) {
            return BCM_E_PARAM;
        }

        parent->first_child = min_index;

    } else if (parent->numq == -1) {

        /* check whether the entry is available */
        for (cur_node = node->sibling; cur_node != NULL;
            cur_node = cur_node->sibling) {
            if (cur_node->gport == node->gport) {
                break;
            }
        }

        if (cur_node == NULL) {
            return BCM_E_PARAM;
        }

    }

    mmu_info = _bcm_kt_mmu_info[unit];

    switch (parent->level) {
        case _BCM_KT_COSQ_NODE_LEVEL_ROOT:
            list = &mmu_info->l0_sched_list;
            node->cosq_attached_to = -1;
            parent->num_child--;
            if (parent->num_child > 0) {
                _bcm_kt_node_index_clear(list, node->hw_index, 1);
            } else {    
                numq = parent->numq;
                start_index = parent->base_index; 
                _bcm_kt_node_index_clear(list, start_index, numq);
                parent->base_index = -1;
            }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L0:
            list = &mmu_info->l1_sched_list;
            node->cosq_attached_to = -1;
            parent->num_child--;
            if (parent->num_child > 0) {
                _bcm_kt_node_index_clear(list, node->hw_index, 1);
            } else {    
                numq = parent->numq;
                start_index = parent->base_index; 
                _bcm_kt_node_index_clear(list, start_index, numq);
                parent->base_index = -1;
            }            
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L1:
            list = (BCM_GPORT_IS_UCAST_QUEUE_GROUP(node->gport)) ?
                   &mmu_info->l2_base_qlist : &mmu_info->l2_ext_qlist;
            if (cosq >= 0) { /* unresolve the specific node */
                for (cur_node = node; cur_node != NULL; cur_node = cur_node->sibling) {
                    if (cur_node->cosq_attached_to == cosq) {
                        break;
                    }
                }

                if (cur_node == NULL) {
                    return BCM_E_INTERNAL;
                }

                id = (BCM_GPORT_IS_UCAST_QUEUE_GROUP(node->gport)) ? cur_node->hw_index :
                     (cur_node->hw_index - mmu_info->num_base_queues);

                node->cosq_attached_to = -1;
                parent->num_child--;

                if (parent->num_child > 0) {
                    _bcm_kt_node_index_clear(list, node->hw_index, 1);
                } else {    
                    numq = parent->numq;
                    start_index = parent->base_index; 
                    _bcm_kt_node_index_clear(list, start_index, numq);
                    parent->base_index = -1;
                }                        

            } else { /* unresolve all the nodes which are connected */
                for (cur_node = node; cur_node != NULL; cur_node = cur_node->sibling) {
                    if (cur_node->cosq_attached_to >= 0) {
                        id = (BCM_GPORT_IS_UCAST_QUEUE_GROUP(node->gport)) ? cur_node->hw_index :
                             (cur_node->hw_index - mmu_info->num_base_queues);

                         if (!BCM_GPORT_IS_UCAST_QUEUE_GROUP(node->gport)) {
                             _bcm_kt_node_index_clear(list, id, 1);
                         }

                        node->cosq_attached_to = -1;
                        parent->num_child--;
                    }
                }

                if (parent->num_child != 0) {
                    return BCM_E_INTERNAL;
                } else {
                    numq = parent->numq;
                    start_index = parent->base_index; 
                    _bcm_kt_node_index_clear(list, start_index, numq);
                    parent->base_index = -1;
                }

            }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L2:
            /* passthru */
        default:
            return BCM_E_PARAM;

        }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_sched_node_set
 * Purpose:
 *     Set LLS scheduler registers based on GPORT
 * Parameters:
 *     unit       - (IN) unit number
 *     gport      - (IN) queue group/scheduler GPORT identifier
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_sched_node_set(int unit, bcm_gport_t gport)
{
    _bcm_kt_cosq_node_t *node, *parent;
    lls_port_config_entry_t port_cfg;
    lls_l0_parent_entry_t l0_parent;
    lls_l0_config_entry_t l0_cfg;
    lls_l1_config_entry_t l1_cfg;
    lls_l1_parent_entry_t l1_parent;
    lls_l2_parent_entry_t l2_parent;
    uint32 num_pri, wrr_use, start_pri;
    int port;
	uint32 rval = 0;

    /*  parent ->level == root
     * config LLS_PORT_CONFIG, LLS_L0_PARENT points to port.

     * parent->level == L0
     * config LLS_L0_CONFIG and LLS_L1_PARENT points to L0 queue

     * parent->level == L1
     * config LLS_L1_CONFIG and LLS_L2_PARENT points to L1 queue
    */

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, gport, NULL, &port, NULL,
                               &node));

    /* read lls_config[level], if num_pri and num_child is diffent reprogram
     * write lls_parent[level with node->parent->hw_index indexed on node_hw_index
     */

    parent = node->parent;

    if (parent->level == _BCM_KT_COSQ_NODE_LEVEL_ROOT) {
        SOC_IF_ERROR_RETURN
                (READ_LLS_PORT_CONFIGm(unit, MEM_BLOCK_ALL, parent->hw_index, &port_cfg));

        num_pri = soc_mem_field32_get(unit, LLS_PORT_CONFIGm, &port_cfg, P_NUM_SPRIf);
        wrr_use = soc_mem_field32_get(unit, LLS_PORT_CONFIGm, &port_cfg, P_WRR_IN_USEf);
        start_pri = soc_mem_field32_get(unit, LLS_PORT_CONFIGm, &port_cfg, P_START_SPRIf);

        sal_memset(&l0_parent, 0, sizeof(lls_l0_parent_entry_t));
        if (parent->num_child < num_pri) {
            soc_mem_field32_set(unit, LLS_L0_PARENTm, &l0_parent, C_PARENTf, 0x3f);
        }
        else {
            soc_mem_field32_set(unit, LLS_L0_PARENTm, &l0_parent, C_PARENTf, parent->hw_index);
        }
        SOC_IF_ERROR_RETURN
            (WRITE_LLS_L0_PARENTm(unit, MEM_BLOCK_ALL, node->hw_index, &l0_parent));

        if (parent->numq > 0 && wrr_use == 0) { /* strict priority configured */
            if (num_pri != parent->num_child || start_pri != parent->first_child) {
            soc_mem_field32_set(unit, LLS_PORT_CONFIGm, &port_cfg, P_NUM_SPRIf, parent->num_child);
            soc_mem_field32_set(unit, LLS_PORT_CONFIGm, &port_cfg, P_WRR_IN_USEf, parent->wrr_in_use);
            soc_mem_field32_set(unit, LLS_PORT_CONFIGm, &port_cfg, P_START_SPRIf, parent->first_child);

            SOC_IF_ERROR_RETURN
                (WRITE_LLS_PORT_CONFIGm(unit, MEM_BLOCK_ALL, parent->hw_index, &port_cfg));
            }
        }

    } else if (parent->level == _BCM_KT_COSQ_NODE_LEVEL_L0) {
        SOC_IF_ERROR_RETURN
                (READ_LLS_L0_CONFIGm(unit, MEM_BLOCK_ALL, parent->hw_index, &l0_cfg));

        num_pri = soc_mem_field32_get(unit, LLS_L0_CONFIGm, &l0_cfg, P_NUM_SPRIf);
        wrr_use = soc_mem_field32_get(unit, LLS_L0_CONFIGm, &l0_cfg, P_WRR_IN_USEf);
        start_pri = soc_mem_field32_get(unit, LLS_L0_CONFIGm, &l0_cfg, P_START_SPRIf);

        sal_memset(&l1_parent, 0, sizeof(lls_l1_parent_entry_t));
        if (parent->num_child < num_pri) {
            soc_mem_field32_set(unit, LLS_L1_PARENTm, &l1_parent, C_PARENTf, 0xff);
        }
        else {
            soc_mem_field32_set(unit, LLS_L1_PARENTm, &l1_parent, C_PARENTf, parent->hw_index);
        }
        SOC_IF_ERROR_RETURN
            (WRITE_LLS_L1_PARENTm(unit, MEM_BLOCK_ALL, node->hw_index, &l1_parent));

        if (parent->numq > 0 && wrr_use == 0) { /* strict priority configured */
            if (num_pri != parent->num_child || start_pri != parent->first_child) {
            soc_mem_field32_set(unit, LLS_L0_CONFIGm, &l0_cfg, P_NUM_SPRIf, parent->num_child);
            soc_mem_field32_set(unit, LLS_L0_CONFIGm, &l0_cfg, P_WRR_IN_USEf, parent->wrr_in_use);
            soc_mem_field32_set(unit, LLS_L0_CONFIGm, &l0_cfg, P_START_SPRIf, parent->first_child);

            SOC_IF_ERROR_RETURN
                (WRITE_LLS_L0_CONFIGm(unit, MEM_BLOCK_ALL, parent->hw_index, &l0_cfg));
            }
        }

    } else if (parent->level == _BCM_KT_COSQ_NODE_LEVEL_L1) {
        SOC_IF_ERROR_RETURN
                (READ_LLS_L1_CONFIGm(unit, MEM_BLOCK_ALL, parent->hw_index, &l1_cfg));

        num_pri = soc_mem_field32_get(unit, LLS_L1_CONFIGm, &l1_cfg, P_NUM_SPRIf);
        wrr_use = soc_mem_field32_get(unit, LLS_L1_CONFIGm, &l1_cfg, P_WRR_IN_USEf);
        start_pri = soc_mem_field32_get(unit, LLS_L1_CONFIGm, &l1_cfg, P_START_SPRIf);

        sal_memset(&l2_parent, 0, sizeof(lls_l2_parent_entry_t));
        if (parent->num_child < num_pri) {
             /* deletion case , flush the queue  */
            soc_reg_field_set(unit, TOQ_QUEUE_FLUSH0r, &rval, Q_FLUSH_ID0f, node->hw_index);
            soc_reg_field_set(unit, TOQ_QUEUE_FLUSH0r, &rval, Q_FLUSH_NUMf, 1);
            soc_reg_field_set(unit, TOQ_QUEUE_FLUSH0r, &rval, Q_FLUSH_ACTIVEf, 1);
            BCM_IF_ERROR_RETURN(WRITE_TOQ_QUEUE_FLUSH0r(unit, rval));
            
            soc_mem_field32_set(unit, LLS_L2_PARENTm, &l2_parent, C_PARENTf, 0x3ff);
        }
        else {
            soc_mem_field32_set(unit, LLS_L2_PARENTm, &l2_parent, C_PARENTf, parent->hw_index);
        }
        SOC_IF_ERROR_RETURN
            (WRITE_LLS_L2_PARENTm(unit, MEM_BLOCK_ALL, node->hw_index, &l2_parent));

        if (parent->numq > 0 && wrr_use == 0) { /* strict priority configured */
            if (num_pri != parent->num_child || start_pri != parent->first_child) {

            soc_mem_field32_set(unit, LLS_L1_CONFIGm, &l1_cfg, P_NUM_SPRIf, parent->num_child);
            soc_mem_field32_set(unit, LLS_L1_CONFIGm, &l1_cfg, P_WRR_IN_USEf, parent->wrr_in_use);
            soc_mem_field32_set(unit, LLS_L1_CONFIGm, &l1_cfg, P_START_SPRIf, parent->first_child);

            SOC_IF_ERROR_RETURN
                (WRITE_LLS_L1_CONFIGm(unit, MEM_BLOCK_ALL, parent->hw_index, &l1_cfg));
            }
        }
    } else {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_sched_set
 * Purpose:
 *     Set scheduling mode
 * Parameters:
 *     unit                - (IN) unit number
 *     port                - (IN) port number or GPORT identifier
 *     cosq                - (IN) COS queue number
 *     mode                - (IN) scheduling mode (BCM_COSQ_XXX)
 *     num_weights         - (IN) number of entries in weights array
 *     weights             - (IN) weights array
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_sched_set(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                       int mode, int num_weights, const int weights[])
{
    _bcm_kt_cosq_node_t *node;
    bcm_port_t local_port;
    int index;
    int i;
    int wrr_in_use;
    int max_weight;

    lls_port_config_entry_t port_cfg;
    lls_l0_config_entry_t l0_cfg;
    lls_l1_config_entry_t l1_cfg;
    lls_l0_child_weight_cfg_cnt_entry_t l0_weight_cfg;
    lls_l1_child_weight_cfg_cnt_entry_t l1_weight_cfg;
    lls_l2_child_weight_cfg_cnt_entry_t l2_weight_cfg;

    if (cosq < 0) { /* caller needs to resolve the wild card value */
        return BCM_E_INTERNAL;
    }
    /* Validate weight values */
    if (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN ||
        mode == BCM_COSQ_DEFICIT_ROUND_ROBIN ||
        mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) {
           max_weight =
               (1 << soc_mem_field_length(unit, LLS_L0_CHILD_WEIGHT_CFG_CNTm, C_WEIGHTf)) - 1;
           for (i = 0; i < num_weights; i++) {
               if (weights[i] < 0 || weights[i] > max_weight) {
                   return BCM_E_PARAM;
               }
           }
    }

    if (BCM_GPORT_IS_SCHEDULER(port)) { /* ETS style scheduler */
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_node_get(unit, port, NULL, &local_port, NULL,
                                   &node));

        if ((mode != BCM_COSQ_STRICT) && (cosq + num_weights) > node->numq) {
            return BCM_E_PARAM;
        }

        if (node->cosq_attached_to < 0) { /* Not resolved yet */
            return BCM_E_NOT_FOUND;
        }

        switch (node->level) {
        case 0: /* port scheduler */
            SOC_IF_ERROR_RETURN
                (READ_LLS_PORT_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &port_cfg));
            wrr_in_use = soc_mem_field32_get(unit, LLS_PORT_CONFIGm, &port_cfg, P_WRR_IN_USEf);

            if (mode == BCM_COSQ_STRICT) {
                if (wrr_in_use == 1) {
                    soc_mem_field32_set(unit, LLS_PORT_CONFIGm, &port_cfg, P_WRR_IN_USEf, 0);
                    SOC_IF_ERROR_RETURN
                        (WRITE_LLS_PORT_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &port_cfg));
                }
                return BCM_E_NONE;
            } else {
                if (wrr_in_use == 0) {
                    soc_mem_field32_set(unit, LLS_PORT_CONFIGm, &port_cfg, P_WRR_IN_USEf, 1);
                    SOC_IF_ERROR_RETURN
                        (WRITE_LLS_PORT_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &port_cfg));
                }
            }

            for (i = 0; i < num_weights; i++) {
                BCM_IF_ERROR_RETURN
                       (_bcm_kt_cosq_index_resolve(unit, port, cosq + i,
                                           _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                            NULL, &index, NULL));
                SOC_IF_ERROR_RETURN
                       (READ_LLS_L0_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l0_weight_cfg));
                soc_mem_field32_set(unit, LLS_L0_CHILD_WEIGHT_CFG_CNTm, (uint32 *)&l0_weight_cfg, C_WEIGHTf, weights[i]);
                SOC_IF_ERROR_RETURN
                       (WRITE_LLS_L0_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l0_weight_cfg));

            }
            break;

        case 1: /* L0 scheduler */
            SOC_IF_ERROR_RETURN
                (READ_LLS_L0_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l0_cfg));
            wrr_in_use = soc_mem_field32_get(unit, LLS_L0_CONFIGm, &l0_cfg, P_WRR_IN_USEf);

            if (mode == BCM_COSQ_STRICT) {
                if (wrr_in_use == 1) {
                    soc_mem_field32_set(unit, LLS_L0_CONFIGm, &l0_cfg, P_WRR_IN_USEf, 0);
                      SOC_IF_ERROR_RETURN
                        (WRITE_LLS_L0_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l0_cfg));
                }
                return BCM_E_NONE;
            } else {
                if (wrr_in_use == 0) {
                    soc_mem_field32_set(unit, LLS_L0_CONFIGm, &l0_cfg, P_WRR_IN_USEf, 1);
                      SOC_IF_ERROR_RETURN
                        (WRITE_LLS_L0_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l0_cfg));
                }
            }

            for (i = 0; i < num_weights; i++) {
                BCM_IF_ERROR_RETURN
                       (_bcm_kt_cosq_index_resolve(unit, port, cosq + i,
                                           _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                            NULL, &index, NULL));
                SOC_IF_ERROR_RETURN
                       (READ_LLS_L1_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l1_weight_cfg));
                soc_mem_field32_set(unit, LLS_L1_CHILD_WEIGHT_CFG_CNTm, (uint32 *)&l1_weight_cfg, C_WEIGHTf, weights[i]);
                SOC_IF_ERROR_RETURN
                       (WRITE_LLS_L1_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l1_weight_cfg));

            }
            break;

        case 2: /* L1 scheduler */
            SOC_IF_ERROR_RETURN
                (READ_LLS_L1_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l1_cfg));
            wrr_in_use = soc_mem_field32_get(unit, LLS_L1_CONFIGm, &l1_cfg, P_WRR_IN_USEf);

            if (mode == BCM_COSQ_STRICT) {
                if (wrr_in_use == 1) {
                    soc_mem_field32_set(unit, LLS_L1_CONFIGm, &l1_cfg, P_WRR_IN_USEf, 0);
                    SOC_IF_ERROR_RETURN
                        (WRITE_LLS_L1_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l1_cfg));
                }
                return BCM_E_NONE;
            } else {
                if (wrr_in_use == 0) {
                    soc_mem_field32_set(unit, LLS_L1_CONFIGm, &l1_cfg, P_WRR_IN_USEf, 1);
                    SOC_IF_ERROR_RETURN
                        (WRITE_LLS_L1_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l1_cfg));
                }
            }

            for (i = 0; i < num_weights; i++) {
                BCM_IF_ERROR_RETURN
                       (_bcm_kt_cosq_index_resolve(unit, port, cosq + i,
                                           _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                            NULL, &index, NULL));
                SOC_IF_ERROR_RETURN
                       (READ_LLS_L2_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l2_weight_cfg));
                soc_mem_field32_set(unit, LLS_L2_CHILD_WEIGHT_CFG_CNTm, (uint32 *)&l2_weight_cfg, C_WEIGHTf, weights[i]);
                SOC_IF_ERROR_RETURN
                       (WRITE_LLS_L2_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l2_weight_cfg));

            }
            break;

        default:
            return BCM_E_INTERNAL;
        }
    } else {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_sched_get
 * Purpose:
 *     Get scheduling mode setting
 * Parameters:
 *     unit                - (IN) unit number
 *     port                - (IN) port number or GPORT identifier
 *     cosq                - (IN) COS queue number
 *     mode                - (OUT) scheduling mode (BCM_COSQ_XXX)
 *     num_weights         - (IN) number of entries in weights array
 *     weights             - (OUT) weights array
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_sched_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                       int *mode, int num_weights, int weights[])
{
    _bcm_kt_cosq_node_t *node;
    bcm_port_t local_port;
    int index;
    int i;

    lls_port_config_entry_t port_cfg;
    lls_l0_config_entry_t l0_cfg;
    lls_l1_config_entry_t l1_cfg;
    lls_l0_child_weight_cfg_cnt_entry_t l0_weight_cfg;
    lls_l1_child_weight_cfg_cnt_entry_t l1_weight_cfg;
    lls_l2_child_weight_cfg_cnt_entry_t l2_weight_cfg;
    int wrr_in_use = 0;

    if (cosq < 0) { /* caller needs to resolve the wild card value */
        return BCM_E_INTERNAL;
    }

    if (BCM_GPORT_IS_SCHEDULER(port)) { /* ETS style scheduler */
           BCM_IF_ERROR_RETURN
               (_bcm_kt_cosq_node_get(unit, port, NULL, &local_port, NULL,
                                      &node));

           if (cosq + num_weights > node->numq) {
               return BCM_E_PARAM;
           }

           if (node->cosq_attached_to < 0) { /* Not resolved yet */
               return BCM_E_NOT_FOUND;
           }

            switch (node->level) {
            case 0: /* port scheduler */
                SOC_IF_ERROR_RETURN
                    (READ_LLS_PORT_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &port_cfg));
                wrr_in_use = soc_mem_field32_get(unit, LLS_PORT_CONFIGm, &port_cfg, P_WRR_IN_USEf);

                for (i = 0; i < num_weights; i++) {
                    BCM_IF_ERROR_RETURN
                        (_bcm_kt_cosq_index_resolve(unit, port, cosq + i,
                                                _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                                NULL, &index, NULL));
                    SOC_IF_ERROR_RETURN
                        (READ_LLS_L0_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l0_weight_cfg));
                    weights[i] = soc_mem_field32_get(unit, LLS_L0_CHILD_WEIGHT_CFG_CNTm, (uint32 *)&l0_weight_cfg, C_WEIGHTf);

                }
                break;

            case 1: /* L0 scheduler */
                SOC_IF_ERROR_RETURN
                    (READ_LLS_L0_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l0_cfg));
                wrr_in_use = soc_mem_field32_get(unit, LLS_L0_CONFIGm, &l0_cfg, P_WRR_IN_USEf);

                for (i = 0; i < num_weights; i++) {
                    BCM_IF_ERROR_RETURN
                        (_bcm_kt_cosq_index_resolve(unit, port, cosq + i,
                                                _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                                NULL, &index, NULL));
                    SOC_IF_ERROR_RETURN
                        (READ_LLS_L1_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l1_weight_cfg));
                    weights[i] = soc_mem_field32_get(unit, LLS_L1_CHILD_WEIGHT_CFG_CNTm, (uint32 *)&l1_weight_cfg, C_WEIGHTf);

                }
                break;

            case 2: /* L1 scheduler */
                SOC_IF_ERROR_RETURN
                    (READ_LLS_L1_CONFIGm(unit, MEM_BLOCK_ALL, node->hw_index, &l1_cfg));
                wrr_in_use = soc_mem_field32_get(unit, LLS_L1_CONFIGm, &l1_cfg, P_WRR_IN_USEf);

                for (i = 0; i < num_weights; i++) {
                    BCM_IF_ERROR_RETURN
                        (_bcm_kt_cosq_index_resolve(unit, port, cosq + i,
                                                _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                                NULL, &index, NULL));
                    SOC_IF_ERROR_RETURN
                        (READ_LLS_L2_CHILD_WEIGHT_CFG_CNTm(unit, MEM_BLOCK_ALL, index, &l2_weight_cfg));
                    weights[i] = soc_mem_field32_get(unit, LLS_L2_CHILD_WEIGHT_CFG_CNTm, (uint32 *)&l2_weight_cfg, C_WEIGHTf);

                }
                break;

                default:
                    return BCM_E_INTERNAL;
    }

    *mode = (wrr_in_use == 1) ? BCM_COSQ_WEIGHTED_FAIR_QUEUING : BCM_COSQ_STRICT;
    } else {
        return BCM_E_PARAM;
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_port_sched_set
 * Purpose:
 *     Set scheduling mode setting for a port
 * Parameters:
 *     unit                - (IN) unit number
 *     port                - (IN) port number or GPORT identifier
 *     cosq                - (IN) COS queue number
 *     mode                - (IN) scheduling mode (BCM_COSQ_XXX)
 *     num_weights         - (IN) number of entries in weights array
 *     weights             - (IN) weights array
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_port_sched_set(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                            int mode, int num_weights, int *weights)
{
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *port_node;
    _bcm_kt_cosq_node_t *l0_node, *l1_node;

    if ((mode != BCM_COSQ_STRICT) && 
        (mode != BCM_COSQ_WEIGHTED_ROUND_ROBIN)) {
        return BCM_E_PARAM;
    }

    /* Each port has L0 and L1 nodes; L1 node has 8 children (L2 queues) */
    /* Set port and L0 nodes as strict priority, Appy the sched parameters
       to L1 node */
    /* get the root node */
    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }
    mmu_info = _bcm_kt_mmu_info[unit];

    port_node = &mmu_info->sched_node[port];

    if (port_node->gport < 0) {
        return BCM_E_PARAM;
    }

    for (l0_node = port_node->child; l0_node != NULL;
        l0_node = l0_node->sibling) {

        if (l0_node->cosq_attached_to < 0) {
            /* this cosq is not yet attached */
            return BCM_E_INTERNAL;
        }

        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_sched_set(unit, port_node->gport,
                        l0_node->cosq_attached_to, BCM_COSQ_STRICT, 1, weights));

        for (l1_node = l0_node->child; l1_node != NULL;
            l1_node = l1_node->sibling) {

            if (l1_node->cosq_attached_to < 0) {
                /* this cosq is not yet attached */
                return BCM_E_INTERNAL;
            }

            BCM_IF_ERROR_RETURN
                (_bcm_kt_cosq_sched_set(unit, l0_node->gport,
                            l1_node->cosq_attached_to, BCM_COSQ_STRICT, 1, weights));
            BCM_IF_ERROR_RETURN
                (_bcm_kt_cosq_sched_set(unit, l1_node->gport,
                            0, mode, num_weights, weights));

        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_port_sched_get
 * Purpose:
 *     Set scheduling mode setting for a port
 * Parameters:
 *     unit                - (IN) unit number
 *     port                - (IN) port number or GPORT identifier
 *     cosq                - (IN) COS queue number
 *     mode                - (OUT) scheduling mode (BCM_COSQ_XXX)
 *     num_weights         - (IN) number of entries in weights array
 *     weights             - (OUT) weights array
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_port_sched_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                            int *mode, int num_weights, int *weights)
{
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *port_node;
    _bcm_kt_cosq_node_t *l0_node, *l1_node;

    /* Each port has L0 and L1 nodes; L1 node has 8 children (L2 queues) */
    /* get the root node */
    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }
    mmu_info = _bcm_kt_mmu_info[unit];

    port_node = &mmu_info->sched_node[port];

    if (port_node->gport < 0) {
        return BCM_E_PARAM;
    }

    if ((l0_node = port_node->child) == NULL) {
        return BCM_E_PARAM;
    }

    if ((l1_node = l0_node->child) == NULL) {
        return BCM_E_PARAM;
    }

    /* get the weitghts configured for L2 children */
    BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_sched_get(unit, l1_node->gport, cosq, mode, num_weights, weights));


    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_mapping_set_regular
 * Purpose:
 *     Determine which COS queue a given priority currently maps to.
 * Parameters:
 *     unit     - (IN) unit number
 *     gport    - (IN) queue group GPORT identifier
 *     priority - (IN) priority value to map
 *     cosq     - (IN) COS queue to map to
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_kt_cosq_mapping_set_regular(int unit, bcm_port_t gport, bcm_cos_t priority,
                        bcm_cos_queue_t cosq)
{
	_bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *node;
    bcm_port_t port;
    int id, numq;
    bcm_cos_queue_t hw_cosq;
    soc_field_t field = COSf;
    cos_map_sel_entry_t cos_map_sel_entry;
    port_cos_map_entry_t cos_map_entries[16];
    void *entries[1];     uint32 old_index, new_index;
    int rv;
	
    if (priority < 0 || priority >= 16) {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, gport, NULL, &port, &id, &node));

    mmu_info = _bcm_kt_mmu_info[unit];
    numq = mmu_info->port[port].q_limit - 
           mmu_info->port[port].q_offset;
	
    if (cosq >= node->numq) {
        return BCM_E_PARAM;
    }

    if (node->base_index < 0) {
        return BCM_E_NOT_FOUND;
    }

    hw_cosq = node->cosq_attached_to;

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        field = numq < 8 ? COSf : HG_COSf;
    }

    entries[0] = &cos_map_entries;

    BCM_IF_ERROR_RETURN
        (READ_COS_MAP_SELm(unit, MEM_BLOCK_ANY, port, &cos_map_sel_entry));
    old_index = soc_mem_field32_get(unit, COS_MAP_SELm, &cos_map_sel_entry,
                                    SELECTf);
    old_index *= 16;

    BCM_IF_ERROR_RETURN
        (soc_profile_mem_get(unit, _bcm_kt_cos_map_profile[unit],
                             old_index, 16, entries));
    soc_mem_field32_set(unit, PORT_COS_MAPm, &cos_map_entries[priority], field,
                        hw_cosq);

    soc_mem_lock(unit, PORT_COS_MAPm);

    rv = soc_profile_mem_delete(unit, _bcm_kt_cos_map_profile[unit],
                                old_index);
#ifndef BCM_COSQ_HIGIG_MAP_DISABLE
    if (BCM_SUCCESS(rv) && IS_CPU_PORT(unit, port)) {
        rv = soc_profile_mem_delete(unit, _bcm_kt_cos_map_profile[unit],
                                    old_index);
    }
#endif /* BCM_COSQ_HIGIG_MAP_DISABLE */
    if (BCM_FAILURE(rv)) {
        soc_mem_unlock(unit, PORT_COS_MAPm);
        return rv;
    }

    rv = soc_profile_mem_add(unit, _bcm_kt_cos_map_profile[unit], entries,
                             16, &new_index);
#ifndef BCM_COSQ_HIGIG_MAP_DISABLE
    if (BCM_SUCCESS(rv) && IS_CPU_PORT(unit, port)) {
        rv = soc_profile_mem_reference(unit, _bcm_kt_cos_map_profile[unit],
                                       new_index, 16);
    }
#endif /* BCM_COSQ_HIGIG_MAP_DISABLE */

    if (rv == SOC_E_RESOURCE) {
        (void)soc_profile_mem_reference(unit, _bcm_kt_cos_map_profile[unit],
                                        old_index, 16);
#ifndef BCM_COSQ_HIGIG_MAP_DISABLE
        if (IS_CPU_PORT(unit, port)) {
            (void)soc_profile_mem_reference(unit,
                                            _bcm_kt_cos_map_profile[unit],
                                            old_index, 16);
        }
#endif /* BCM_COSQ_HIGIG_MAP_DISABLE */
    }

    soc_mem_unlock(unit, PORT_COS_MAPm);

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    soc_mem_field32_set(unit, COS_MAP_SELm, &cos_map_sel_entry, SELECTf,
                        new_index / 16);
    BCM_IF_ERROR_RETURN
        (WRITE_COS_MAP_SELm(unit, MEM_BLOCK_ANY, port, &cos_map_sel_entry));

#ifndef BCM_COSQ_HIGIG_MAP_DISABLE
    if (IS_CPU_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN
            (soc_mem_field32_modify(unit, COS_MAP_SELm,
                                    SOC_INFO(unit).cpu_hg_index, SELECTf,
                                    new_index / 16));
    }
#endif /* BCM_COSQ_HIGIG_MAP_DISABLE */

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_bucket_set
 * Purpose:
 *     Configure COS queue bandwidth control bucket
 * Parameters:
 *     unit          - (IN) unit number
 *     port          - (IN) port number or GPORT identifier
 *     cosq          - (IN) COS queue number
 *     min_quantum   - (IN) kbps or packet/second
 *     max_quantum   - (IN) kbps or packet/second
 *     burst_quantum - (IN) kbps or packet/second
 *     flags         - (IN)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
STATIC int
_bcm_kt_cosq_bucket_set(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                        uint32 min_quantum, uint32 max_quantum,
                        uint32 burst_quantum, uint32 flags)
{
    int rv;
    _bcm_kt_cosq_node_t *node;
    bcm_port_t local_port;
    int index;
    soc_mem_t config_mem[2];
    lls_port_shaper_config_c_entry_t port_entry;
    lls_l0_shaper_config_c_entry_t  l0_entry;
    lls_l1_shaper_config_c_entry_t  l1_entry;
    lls_l2_shaper_config_lower_entry_t l2_entry;
    uint32 rate_exp[2], rate_mantissa[2];
    uint32 burst_exp, burst_mantissa;
    int i, idx;
	int level = _BCM_KT_COSQ_NODE_LEVEL_L2;
    soc_field_t rate_exp_f[] = {
        C_MAX_REF_RATE_EXPf, C_MIN_REF_RATE_EXPf
    };
    soc_field_t rate_mant_f[] = {
        C_MAX_REF_RATE_MANTf, C_MIN_REF_RATE_MANTf
    };
    soc_field_t burst_exp_f[] = {
        C_MAX_THLD_EXPf, C_MIN_THLD_EXPf
    };
    soc_field_t burst_mant_f[] = {
        C_MAX_THLD_MANTf, C_MIN_THLD_MANTf
    };
    static const soc_field_t rate_exp_fields[] = {
       C_MAX_REF_RATE_EXP_0f, C_MAX_REF_RATE_EXP_1f,
       C_MAX_REF_RATE_EXP_2f, C_MAX_REF_RATE_EXP_3f,
       C_MIN_REF_RATE_EXP_0f, C_MIN_REF_RATE_EXP_1f,
       C_MIN_REF_RATE_EXP_2f, C_MIN_REF_RATE_EXP_3f
    };
    static const soc_field_t rate_mant_fields[] = {
       C_MAX_REF_RATE_MANT_0f, C_MAX_REF_RATE_MANT_1f,
       C_MAX_REF_RATE_MANT_2f, C_MAX_REF_RATE_MANT_3f,
       C_MIN_REF_RATE_MANT_0f, C_MIN_REF_RATE_MANT_1f,
       C_MIN_REF_RATE_MANT_2f, C_MIN_REF_RATE_MANT_3f
    };
    static const soc_field_t burst_exp_fields[] = {
       C_MAX_THLD_EXP_0f, C_MAX_THLD_EXP_1f,
       C_MAX_THLD_EXP_2f, C_MAX_THLD_EXP_3f,
       C_MIN_THLD_EXP_0f, C_MIN_THLD_EXP_1f,
       C_MIN_THLD_EXP_2f, C_MIN_THLD_EXP_3f
    };
    static const soc_field_t burst_mant_fields[] = {
       C_MAX_THLD_MANT_0f, C_MAX_THLD_MANT_1f,
       C_MAX_THLD_MANT_2f, C_MAX_THLD_MANT_3f,
       C_MIN_THLD_MANT_0f, C_MIN_THLD_MANT_1f,
       C_MIN_THLD_MANT_2f, C_MIN_THLD_MANT_3f
    };

    if (cosq < 0) {
        if (cosq == -1) {
            /* caller needs to resolve the wild card value (-1) */
            return BCM_E_INTERNAL;
        } else { /* reject all other negative value */
            return BCM_E_PARAM;
        }
    }


    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_index_resolve(unit, port, cosq,
                                    _BCM_KT_COSQ_INDEX_STYLE_BUCKET,
                                    &local_port, &index, NULL));
    
    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
        BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
        BCM_GPORT_IS_SCHEDULER(port)) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_node_get(unit, port, NULL, NULL, NULL, &node));
        level = node->level;    
    }     

    /* compute exp and mantissa and program the registers */
    rv = soc_katana_get_shaper_rate_info(unit, max_quantum,
                                         &rate_mantissa[0], &rate_exp[0]);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    rv = soc_katana_get_shaper_rate_info(unit, min_quantum,
                                         &rate_mantissa[1], &rate_exp[1]);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    rv = soc_katana_get_shaper_burst_info(unit, burst_quantum,
                                          &burst_mantissa, &burst_exp);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    switch (level) {
        case _BCM_KT_COSQ_NODE_LEVEL_ROOT:
            config_mem[0] = LLS_PORT_SHAPER_CONFIG_Cm;
            SOC_IF_ERROR_RETURN
               (soc_mem_read(unit, config_mem[0], MEM_BLOCK_ALL,
                             index, &port_entry));
            soc_mem_field32_set(unit, config_mem[0], &port_entry,
                                rate_exp_f[0], rate_exp[0]);
            soc_mem_field32_set(unit, config_mem[0], &port_entry,
                                rate_mant_f[0], rate_mantissa[0]);
            soc_mem_field32_set(unit, config_mem[0], &port_entry,
                                burst_exp_f[0], burst_exp);
            soc_mem_field32_set(unit, config_mem[0], &port_entry,
                                burst_mant_f[0], burst_mantissa);
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, config_mem[0],
                               MEM_BLOCK_ALL, index, &port_entry));
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L0:
            config_mem[0] = LLS_L0_SHAPER_CONFIG_Cm;
            config_mem[1] = LLS_L0_MIN_CONFIG_Cm;

            for (i = 0; i < 2; i++) {
                SOC_IF_ERROR_RETURN
                       (soc_mem_read(unit, config_mem[i], MEM_BLOCK_ALL,
                                   index, &l0_entry));
                soc_mem_field32_set(unit, config_mem[i], &l0_entry,
                                    rate_exp_f[i], rate_exp[i]);
                soc_mem_field32_set(unit, config_mem[i], &l0_entry,
                                    rate_mant_f[i], rate_mantissa[i]);
                soc_mem_field32_set(unit, config_mem[i], &l0_entry,
                                    burst_exp_f[i], burst_exp);
                soc_mem_field32_set(unit, config_mem[i], &l0_entry,
                                    burst_mant_f[i], burst_mantissa);
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, config_mem[i],
                                     MEM_BLOCK_ALL, index, &l0_entry));
             }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L1:
            config_mem[0] = LLS_L1_SHAPER_CONFIG_Cm;
            config_mem[1] = LLS_L1_MIN_CONFIG_Cm;

            for (i = 0; i < 2; i++) {
                idx = (i * 4) + (index % 4);
                SOC_IF_ERROR_RETURN
                       (soc_mem_read(unit, config_mem[i], MEM_BLOCK_ALL,
                                   index/4, &l1_entry));
                soc_mem_field32_set(unit, config_mem[i], &l1_entry,
                                    rate_exp_fields[idx], rate_exp[i]);
                soc_mem_field32_set(unit, config_mem[i], &l1_entry,
                                    rate_mant_fields[idx], rate_mantissa[i]);
                soc_mem_field32_set(unit, config_mem[i], &l1_entry,
                                    burst_exp_fields[idx], burst_exp);
                soc_mem_field32_set(unit, config_mem[i], &l1_entry,
                                    burst_mant_fields[idx], burst_mantissa);
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, config_mem[i],
                                   MEM_BLOCK_ALL, index/4, &l1_entry));
            }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L2:
            if ((index % 8) < 4) {
                config_mem[0] = LLS_L2_SHAPER_CONFIG_LOWERm;
                config_mem[1] = LLS_L2_MIN_CONFIG_LOWER_Cm;
            } else {
                config_mem[0] = LLS_L2_SHAPER_CONFIG_UPPERm;
                config_mem[1] = LLS_L2_MIN_CONFIG_UPPER_Cm;
            }

            for (i = 0; i < 2; i++) {
                idx = (i * 4) + (index % 4);
                SOC_IF_ERROR_RETURN
                       (soc_mem_read(unit, config_mem[i], MEM_BLOCK_ALL,
                                 index/8, &l2_entry));
                soc_mem_field32_set(unit, config_mem[i], &l2_entry,
                                    rate_exp_fields[idx], rate_exp[i]);
                soc_mem_field32_set(unit, config_mem[i], &l2_entry,
                                    rate_mant_fields[idx], rate_mantissa[i]);
                soc_mem_field32_set(unit, config_mem[i], &l2_entry,
                                    burst_exp_fields[idx], burst_exp);
                soc_mem_field32_set(unit, config_mem[i], &l2_entry,
                                    burst_mant_fields[idx], burst_mantissa);
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, config_mem[i],
                                   MEM_BLOCK_ALL, index/8, &l2_entry));
            }
            break;

        default:
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_bucket_get
 * Purpose:
 *     Get COS queue bandwidth control bucket setting
 * Parameters:
 *     unit          - (IN) unit number
 *     port          - (IN) port number or GPORT identifier
 *     cosq          - (IN) COS queue number
 *     min_quantum   - (OUT) kbps or packet/second
 *     max_quantum   - (OUT) kbps or packet/second
 *     burst_quantum - (OUT) kbps or packet/second
 *     flags         - (IN)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
STATIC int
_bcm_kt_cosq_bucket_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                        uint32 *min_quantum, uint32 *max_quantum,
                        uint32 *burst_quantum, uint32 flags)
{
    int rv;
    _bcm_kt_cosq_node_t *node;
    bcm_port_t local_port;
    int index;
    soc_mem_t config_mem[2];
    lls_port_shaper_config_c_entry_t port_entry;
    lls_l0_shaper_config_c_entry_t  l0_entry;
    lls_l1_shaper_config_c_entry_t  l1_entry;
    lls_l2_shaper_config_lower_entry_t l2_entry;
    uint32 rate_exp[2], rate_mantissa[2];
    uint32 burst_exp, burst_mantissa;
    int i, idx;
	int level = _BCM_KT_COSQ_NODE_LEVEL_L2;
    soc_field_t rate_exp_f[] = {
        C_MAX_REF_RATE_EXPf, C_MIN_REF_RATE_EXPf
    };
    soc_field_t rate_mant_f[] = {
        C_MAX_REF_RATE_MANTf, C_MIN_REF_RATE_MANTf
    };
    soc_field_t burst_exp_f[] = {
        C_MAX_THLD_EXPf, C_MIN_THLD_EXPf
    };
    soc_field_t burst_mant_f[] = {
        C_MAX_THLD_MANTf, C_MIN_THLD_MANTf
    };
    static const soc_field_t rate_exp_fields[] = {
       C_MAX_REF_RATE_EXP_0f, C_MAX_REF_RATE_EXP_1f,
       C_MAX_REF_RATE_EXP_2f, C_MAX_REF_RATE_EXP_3f,
       C_MIN_REF_RATE_EXP_0f, C_MIN_REF_RATE_EXP_1f,
       C_MIN_REF_RATE_EXP_2f, C_MIN_REF_RATE_EXP_3f
    };
    static const soc_field_t rate_mant_fields[] = {
       C_MAX_REF_RATE_MANT_0f, C_MAX_REF_RATE_MANT_1f,
       C_MAX_REF_RATE_MANT_2f, C_MAX_REF_RATE_MANT_3f,
       C_MIN_REF_RATE_MANT_0f, C_MIN_REF_RATE_MANT_1f,
       C_MIN_REF_RATE_MANT_2f, C_MIN_REF_RATE_MANT_3f
    };
    static const soc_field_t burst_exp_fields[] = {
       C_MAX_THLD_EXP_0f, C_MAX_THLD_EXP_1f,
       C_MAX_THLD_EXP_2f, C_MAX_THLD_EXP_3f,
       C_MIN_THLD_EXP_0f, C_MIN_THLD_EXP_1f,
       C_MIN_THLD_EXP_2f, C_MIN_THLD_EXP_3f       
    };
    static const soc_field_t burst_mant_fields[] = {
       C_MAX_THLD_MANT_0f, C_MAX_THLD_MANT_1f,
       C_MAX_THLD_MANT_2f, C_MAX_THLD_MANT_3f,
       C_MIN_THLD_MANT_0f, C_MIN_THLD_MANT_1f,
       C_MIN_THLD_MANT_2f, C_MIN_THLD_MANT_3f
    };

    if (cosq < 0) {
        if (cosq == -1) {
            /* caller needs to resolve the wild card value (-1) */
            return BCM_E_INTERNAL;
        } else { /* reject all other negative value */
            return BCM_E_PARAM;
        }
    }

    if (min_quantum == NULL || max_quantum == NULL || burst_quantum == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_index_resolve(unit, port, cosq,
                                    _BCM_KT_COSQ_INDEX_STYLE_BUCKET,
                                    &local_port, &index, NULL));
    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
        BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
        BCM_GPORT_IS_SCHEDULER(port)) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_node_get(unit, port, NULL, NULL, NULL, &node));
        level = node->level;    
    } 

    switch (level) {
        case _BCM_KT_COSQ_NODE_LEVEL_ROOT:
            config_mem[0] = LLS_PORT_SHAPER_CONFIG_Cm;
            SOC_IF_ERROR_RETURN
               (soc_mem_read(unit, config_mem[0], MEM_BLOCK_ALL,
                             index, &port_entry));
            rate_exp[0] = soc_mem_field32_get(unit, config_mem[0], &port_entry,
                                rate_exp_f[0]);
            rate_mantissa[0] = soc_mem_field32_get(unit, config_mem[0], &port_entry,
                                rate_mant_f[0]);
            rate_exp[1] = rate_exp[0];
            rate_mantissa[1] = rate_mantissa[0];
            burst_exp = soc_mem_field32_get(unit, config_mem[0], &port_entry,
                                burst_exp_f[0]);
            burst_mantissa = soc_mem_field32_get(unit, config_mem[0], &port_entry,
                                burst_mant_f[0]);
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L0:
            config_mem[0] = LLS_L0_SHAPER_CONFIG_Cm;
            config_mem[1] = LLS_L0_MIN_CONFIG_Cm;

            for (i = 0; i < 2; i++) {
                SOC_IF_ERROR_RETURN
                       (soc_mem_read(unit, config_mem[i], MEM_BLOCK_ALL,
                                 index, &l0_entry));
                rate_exp[i] = soc_mem_field32_get(unit, config_mem[i], &l0_entry,
                                    rate_exp_f[i]);
                rate_mantissa[i] = soc_mem_field32_get(unit, config_mem[i], &l0_entry,
                                    rate_mant_f[i]);
                burst_exp = soc_mem_field32_get(unit, config_mem[i], &l0_entry,
                                    burst_exp_f[i]);
                burst_mantissa = soc_mem_field32_get(unit, config_mem[i], &l0_entry,
                                    burst_mant_f[i]);
            }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L1:
            config_mem[0] = LLS_L1_SHAPER_CONFIG_Cm;
            config_mem[1] = LLS_L1_MIN_CONFIG_Cm;

            for (i = 0; i < 2; i++) {
                idx = (i * 4) + (index % 4);
                SOC_IF_ERROR_RETURN
                       (soc_mem_read(unit, config_mem[i], MEM_BLOCK_ALL,
                                     index/4, &l1_entry));
                rate_exp[i] = soc_mem_field32_get(unit, config_mem[i], &l1_entry,
                                    rate_exp_fields[idx]);
                rate_mantissa[i] = soc_mem_field32_get(unit, config_mem[i], &l1_entry,
                                    rate_mant_fields[idx]);
                burst_exp = soc_mem_field32_get(unit, config_mem[i], &l1_entry,
                                    burst_exp_fields[idx]);
                burst_mantissa = soc_mem_field32_get(unit, config_mem[i], &l1_entry,
                                    burst_mant_fields[idx]);
             }
            break;

        case _BCM_KT_COSQ_NODE_LEVEL_L2:
            if ((index % 8) < 4) {
                config_mem[0] = LLS_L2_SHAPER_CONFIG_LOWERm;
                config_mem[1] = LLS_L2_MIN_CONFIG_LOWER_Cm;
            } else {
                config_mem[0] = LLS_L2_SHAPER_CONFIG_UPPERm;
                config_mem[1] = LLS_L2_MIN_CONFIG_UPPER_Cm;
            }

            for (i = 0; i < 2; i++) {
                idx = (i * 4) + (index % 4);
                SOC_IF_ERROR_RETURN
                       (soc_mem_read(unit, config_mem[i], MEM_BLOCK_ALL,
                                 index/8, &l2_entry));
                rate_exp[i] = soc_mem_field32_get(unit, config_mem[i], &l2_entry,
                                    rate_exp_fields[idx]);
                rate_mantissa[i] = soc_mem_field32_get(unit, config_mem[i], &l2_entry,
                                    rate_mant_fields[idx]);
                burst_exp = soc_mem_field32_get(unit, config_mem[i], &l2_entry,
                                    burst_exp_fields[idx]);
                burst_mantissa = soc_mem_field32_get(unit, config_mem[i], &l2_entry,
                                    burst_mant_fields[idx]);
            }
            break;

        default:
            return BCM_E_PARAM;
    }

    /* convert exp and mantissa to bps */
    rv = soc_katana_compute_shaper_rate(unit, rate_mantissa[0], rate_exp[0],
                                        max_quantum);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    rv = soc_katana_compute_shaper_rate(unit, rate_mantissa[1], rate_exp[1],
                                        min_quantum);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    rv = soc_katana_compute_shaper_burst(unit, burst_mantissa, burst_exp,
                                         burst_quantum);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    return BCM_E_NONE;
}

/*
 *  Convert HW drop probability to percent value
 */
STATIC int
_bcm_kt_hw_drop_prob_to_percent[] = {
    0,     /* 0  */
    1,     /* 1  */
    2,     /* 2  */
    3,     /* 3  */
    4,     /* 4  */
    5,     /* 5  */
    6,     /* 6  */
    7,     /* 7  */
    8,     /* 8  */
    9,     /* 9  */
    10,    /* 10 */
    25,    /* 11 */
    50,    /* 12 */
    75,    /* 13 */
    100,   /* 14 */
    -1     /* 15 */
};

STATIC int
_bcm_kt_percent_to_drop_prob(int percent)
{
   int i;

   for (i = 14; i > 0 ; i--) {
      if (percent >= _bcm_kt_hw_drop_prob_to_percent[i]) {
          break;
      }
   }
   return i;
}

STATIC int
_bcm_kt_drop_prob_to_percent(int drop_prob) {
   return (_bcm_kt_hw_drop_prob_to_percent[drop_prob]);
}

/*
 * Function:
 *     _bcm_kt_cosq_wred_set
 * Purpose:
 *     Configure unicast queue WRED setting
 * Parameters:
 *     unit                - (IN) unit number
 *     port                - (IN) port number or GPORT identifier
 *     cosq                - (IN) COS queue number
 *     flags               - (IN) BCM_COSQ_DISCARD_XXX
 *     min_thresh          - (IN)
 *     max_thresh          - (IN)
 *     drop_probablity     - (IN)
 *     gain                - (IN)
 *     ignore_enable_flags - (IN)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
STATIC int
_bcm_kt_cosq_wred_set(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                      uint32 flags, uint32 min_thresh, uint32 max_thresh,
                      int drop_probability, int gain, int ignore_enable_flags)
{
    soc_reg_t reg = 0;
    soc_field_t field;
    uint32 rval = 0;
    bcm_port_t local_port;
    int index;
    uint32 profile_index, old_profile_index;
    mmu_wred_drop_curve_profile_0_entry_t entry_tcp_green;
    mmu_wred_drop_curve_profile_1_entry_t entry_tcp_yellow;
    mmu_wred_drop_curve_profile_2_entry_t entry_tcp_red;
    mmu_wred_drop_curve_profile_3_entry_t entry_nontcp_green;
    mmu_wred_drop_curve_profile_4_entry_t entry_nontcp_yellow;
    mmu_wred_drop_curve_profile_5_entry_t entry_nontcp_red;
    void *entries[6];
    soc_mem_t mems[6];
    _bcm_kt_cosq_node_t *node;
    soc_mem_t wred_mem = MMU_WRED_QUEUE_CONFIG_BUFFERm;
    mmu_wred_queue_config_buffer_entry_t qentry;
    int rate, i;

    reg = WRED_CONFIGr;
    if (port == -1) {
        reg = GLOBAL_WRED_CONFIG_QENTRYr;
        field = WRED_ENABLEf;
        local_port = 0;
        index = 0; /* mmu init code uses service pool 0 only */
    } else {
        if (cosq == -1) {
           return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, port, cosq,
                                        _BCM_KT_COSQ_INDEX_STYLE_WRED,
                                        &local_port, &index, NULL));
        if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
            BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
            BCM_GPORT_IS_SCHEDULER(port)) {
                BCM_IF_ERROR_RETURN
                    (_bcm_kt_cosq_node_get(unit, port, NULL, &local_port, NULL,
                                       &node));
                if (node->level == _BCM_KT_COSQ_NODE_LEVEL_L1) {
                    wred_mem = MMU_WRED_OPN_CONFIG_BUFFERm;
                }
            }
        field = WRED_ENf;
    }

    old_profile_index = 0xffffffff;
    if (reg == GLOBAL_WRED_CONFIG_QENTRYr) {
        BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg, local_port, index, &rval));
    } else {
        SOC_IF_ERROR_RETURN
           (soc_mem_read(unit, wred_mem, MEM_BLOCK_ALL,
                         index, &qentry));
    }
    if (flags & (BCM_COSQ_DISCARD_NONTCP | BCM_COSQ_DISCARD_COLOR_ALL)) {
        if (reg == GLOBAL_WRED_CONFIG_QENTRYr) {
            old_profile_index = soc_reg_field_get(unit, reg, rval, PROFILE_INDEXf);
        } else {
            old_profile_index = soc_mem_field32_get(unit, wred_mem,
                                                    &qentry, PROFILE_INDEXf);
        } 
        entries[0] = &entry_tcp_green;
        entries[1] = &entry_tcp_yellow;
        entries[2] = &entry_tcp_red;
        entries[3] = &entry_nontcp_green;
        entries[4] = &entry_nontcp_yellow;
        entries[5] = &entry_nontcp_red;
        BCM_IF_ERROR_RETURN
            (soc_profile_mem_get(unit, _bcm_kt_wred_profile[unit],
                                 old_profile_index, 1, entries));
        for (i = 0; i < 6; i++) {
            mems[i] = INVALIDm;
        }
        if (!(flags & BCM_COSQ_DISCARD_NONTCP)) {
            if (flags & BCM_COSQ_DISCARD_COLOR_GREEN) {
                mems[0] = MMU_WRED_DROP_CURVE_PROFILE_0m;
            }
            if (flags & BCM_COSQ_DISCARD_COLOR_YELLOW) {
                mems[1] = MMU_WRED_DROP_CURVE_PROFILE_1m;
            }
            if (flags & BCM_COSQ_DISCARD_COLOR_RED) {
                mems[2] = MMU_WRED_DROP_CURVE_PROFILE_2m;
            }
        } else if (!(flags & (BCM_COSQ_DISCARD_COLOR_GREEN |
                              BCM_COSQ_DISCARD_COLOR_YELLOW |
                              BCM_COSQ_DISCARD_COLOR_RED))) {
            mems[3] = MMU_WRED_DROP_CURVE_PROFILE_3m;
            mems[4] = MMU_WRED_DROP_CURVE_PROFILE_4m;
            mems[5] = MMU_WRED_DROP_CURVE_PROFILE_5m;

        } else {
            if (flags & BCM_COSQ_DISCARD_COLOR_GREEN) {
                mems[3] = MMU_WRED_DROP_CURVE_PROFILE_3m;
            }
            if (flags & BCM_COSQ_DISCARD_COLOR_YELLOW) {
                mems[4] = MMU_WRED_DROP_CURVE_PROFILE_4m;
            }
            if (flags & BCM_COSQ_DISCARD_COLOR_RED) {
                mems[5] = MMU_WRED_DROP_CURVE_PROFILE_5m;
            }
        }
        rate = _bcm_kt_percent_to_drop_prob(drop_probability);
        for (i = 0; i < 6; i++) {
            if (mems[i] == INVALIDm) {
                continue;
            }
            soc_mem_field32_set(unit, mems[i], entries[i], MIN_THDf,
                                min_thresh);
            soc_mem_field32_set(unit, mems[i], entries[i], MAX_THDf,
                                max_thresh);
            soc_mem_field32_set(unit, mems[i], entries[i], MAX_DROP_RATEf,
                                rate);
        }
        BCM_IF_ERROR_RETURN
            (soc_profile_mem_add(unit, _bcm_kt_wred_profile[unit], entries, 1,
                                 &profile_index));
        if (reg == GLOBAL_WRED_CONFIG_QENTRYr) {
            soc_reg_field_set(unit, reg, &rval, PROFILE_INDEXf, profile_index);
            soc_reg_field_set(unit, reg, &rval, WEIGHTf, gain);
        } else {
            soc_mem_field32_set(unit, wred_mem, &qentry, PROFILE_INDEXf, profile_index);
            soc_mem_field32_set(unit, wred_mem, &qentry, WEIGHTf, gain);
        }
    }

    /* Some APIs only modify profiles */
    if (!ignore_enable_flags) {
        if ( reg == GLOBAL_WRED_CONFIG_QENTRYr) {
            soc_reg_field_set(unit, reg, &rval, CAP_AVERAGEf,
                              flags & BCM_COSQ_DISCARD_CAP_AVERAGE ? 1 : 0);
            soc_reg_field_set(unit, reg, &rval, field,
                              flags & BCM_COSQ_DISCARD_ENABLE ? 1 : 0);
        } else {
            soc_mem_field32_set(unit, wred_mem, &qentry, CAP_AVERAGEf,
                              flags & BCM_COSQ_DISCARD_CAP_AVERAGE ? 1 : 0);
            soc_mem_field32_set(unit, wred_mem, &qentry, field,
                              flags & BCM_COSQ_DISCARD_ENABLE ? 1 : 0);
            if (wred_mem == MMU_WRED_QUEUE_CONFIG_BUFFERm) {
                /* ECN marking is applicable only for queues */
                soc_mem_field32_set(unit, wred_mem, &qentry, ECN_MARKING_ENf,
                                    flags & BCM_COSQ_DISCARD_MARK_CONGESTION ?
                                    1 : 0);
            }
        }
    }

    if (reg == GLOBAL_WRED_CONFIG_QENTRYr) {
        BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, local_port, index, rval));
    } else {
        SOC_IF_ERROR_RETURN
            (soc_mem_write(unit, wred_mem, MEM_BLOCK_ALL,
                           index, &qentry));
        /* write the same into MMU_WRED_QUEUE_CONFIG_QENTRYm and
           MMU_WRED_OPN_CONFIG_QENTRYm */

        if (wred_mem == MMU_WRED_QUEUE_CONFIG_BUFFERm) {
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, MMU_WRED_QUEUE_CONFIG_QENTRYm,
                               MEM_BLOCK_ALL, index, &qentry));
        } else {
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, MMU_WRED_OPN_CONFIG_QENTRYm,
                               MEM_BLOCK_ALL, index, &qentry));
        }
    }

    if (old_profile_index != 0xffffffff) {
        BCM_IF_ERROR_RETURN
            (soc_profile_mem_delete(unit, _bcm_kt_wred_profile[unit],
                                    old_profile_index));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_kt_cosq_wred_get
 * Purpose:
 *     Get unicast queue WRED setting
 * Parameters:
 *     unit                - (IN) unit number
 *     port                - (IN) port number or GPORT identifier
 *     cosq                - (IN) COS queue number
 *     flags               - (IN/OUT) BCM_COSQ_DISCARD_XXX
 *     min_thresh          - (OUT)
 *     max_thresh          - (OUT)
 *     drop_probablity     - (OUT)
 *     gain                - (OUT)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
STATIC int
_bcm_kt_cosq_wred_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                      uint32 *flags, uint32 *min_thresh, uint32 *max_thresh,
                      int *drop_probability, int *gain)
{
    soc_reg_t reg = 0;
    soc_field_t field;
    uint32 rval;
    bcm_port_t local_port;
    int index, profile_index;
    mmu_wred_drop_curve_profile_0_entry_t entry_tcp_green;
    mmu_wred_drop_curve_profile_1_entry_t entry_tcp_yellow;
    mmu_wred_drop_curve_profile_2_entry_t entry_tcp_red;
    mmu_wred_drop_curve_profile_3_entry_t entry_nontcp_green;
    mmu_wred_drop_curve_profile_4_entry_t entry_nontcp_yellow;
    mmu_wred_drop_curve_profile_5_entry_t entry_nontcp_red;
    void *entries[6];
    void *entry_p;
    soc_mem_t mem;
    _bcm_kt_cosq_node_t *node;
    soc_mem_t wred_mem = MMU_WRED_QUEUE_CONFIG_BUFFERm;
    mmu_wred_queue_config_buffer_entry_t qentry;
    int rate;

    if (port == -1) {
        reg = GLOBAL_WRED_CONFIG_QENTRYr;
        field = WRED_ENABLEf;
        local_port = 0;
        index = 0; /* mmu init code uses service pool 0 only */
    } else {
        if (cosq == -1) {
           return BCM_E_PARAM;
        }
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, port, cosq,
                                        _BCM_KT_COSQ_INDEX_STYLE_WRED,
                                        &local_port, &index, NULL));
        if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
            BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
            BCM_GPORT_IS_SCHEDULER(port)) {        
                BCM_IF_ERROR_RETURN
                    (_bcm_kt_cosq_node_get(unit, port, NULL, &local_port, NULL,
                                       &node));
                if (node->level == _BCM_KT_COSQ_NODE_LEVEL_L1) {
                    wred_mem = MMU_WRED_OPN_CONFIG_BUFFERm;
                }
            }
         
        field = WRED_ENf;
    }

    if (reg == GLOBAL_WRED_CONFIG_QENTRYr) {
        BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg, local_port, index, &rval));
        profile_index = soc_reg_field_get(unit, reg, rval, PROFILE_INDEXf);
    } else {
            SOC_IF_ERROR_RETURN
               (soc_mem_read(unit, wred_mem, MEM_BLOCK_ALL,
                             index, &qentry));
            profile_index = soc_mem_field32_get(unit, wred_mem,
                                                &qentry, PROFILE_INDEXf);
    }

    if (!(*flags & BCM_COSQ_DISCARD_NONTCP)) {
        if (*flags & BCM_COSQ_DISCARD_COLOR_GREEN) {
            mem = MMU_WRED_DROP_CURVE_PROFILE_0m;
            entry_p = &entry_tcp_green;
        } else if (*flags & BCM_COSQ_DISCARD_COLOR_YELLOW) {
            mem = MMU_WRED_DROP_CURVE_PROFILE_1m;
            entry_p = &entry_tcp_yellow;
        } else if (*flags & BCM_COSQ_DISCARD_COLOR_RED) {
            mem = MMU_WRED_DROP_CURVE_PROFILE_2m;
            entry_p = &entry_tcp_red;
        } else {
           mem = MMU_WRED_DROP_CURVE_PROFILE_0m;
           entry_p = &entry_tcp_green;
        }
    } else {
        if (*flags & BCM_COSQ_DISCARD_COLOR_GREEN) {
            mem = MMU_WRED_DROP_CURVE_PROFILE_3m;
            entry_p = &entry_nontcp_green;
        } else if (*flags & BCM_COSQ_DISCARD_COLOR_YELLOW) {
            mem = MMU_WRED_DROP_CURVE_PROFILE_4m;
            entry_p = &entry_nontcp_yellow;
        } else if (*flags & BCM_COSQ_DISCARD_COLOR_RED) {
            mem = MMU_WRED_DROP_CURVE_PROFILE_5m;
            entry_p = &entry_nontcp_red;
        } else {
           mem = MMU_WRED_DROP_CURVE_PROFILE_3m;
           entry_p = &entry_nontcp_green;
        }
    }
    entries[0] = &entry_tcp_green;
    entries[1] = &entry_tcp_yellow;
    entries[2] = &entry_tcp_red;
    entries[3] = &entry_nontcp_green;
    entries[4] = &entry_nontcp_yellow;
    entries[5] = &entry_nontcp_red;
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_get(unit, _bcm_kt_wred_profile[unit],
                             profile_index, 1, entries));
    if (min_thresh != NULL) {
        *min_thresh = soc_mem_field32_get(unit, mem, entry_p, MIN_THDf);
    }
    if (max_thresh != NULL) {
        *max_thresh = soc_mem_field32_get(unit, mem, entry_p, MAX_THDf);
    }
    if (drop_probability != NULL) {
        rate = soc_mem_field32_get(unit, mem, entry_p, MAX_DROP_RATEf);
        *drop_probability = _bcm_kt_drop_prob_to_percent(rate);
    }

    if (gain != NULL && reg == GLOBAL_WRED_CONFIG_QENTRYr) {
        *gain = soc_reg_field_get(unit, reg, rval, WEIGHTf);
    }
    if (gain != NULL && wred_mem != 0) {
        *gain = soc_mem_field32_get(unit, wred_mem, &qentry, WEIGHTf);
    }

    *flags &= ~(BCM_COSQ_DISCARD_CAP_AVERAGE | BCM_COSQ_DISCARD_ENABLE);
    if ((reg == GLOBAL_WRED_CONFIG_QENTRYr) &&
        soc_reg_field_get(unit, reg, rval, CAP_AVERAGEf)) {
        *flags |= BCM_COSQ_DISCARD_CAP_AVERAGE;
    }
    if ((wred_mem != 0) &&
         soc_mem_field32_get(unit, wred_mem, &qentry, CAP_AVERAGEf)) {
        *flags |= BCM_COSQ_DISCARD_CAP_AVERAGE;
    }

    if ((reg == GLOBAL_WRED_CONFIG_QENTRYr) &&
        soc_reg_field_get(unit, reg, rval, field)) {
        *flags |= BCM_COSQ_DISCARD_ENABLE;
    }
    if ((wred_mem != 0) &&
         soc_mem_field32_get(unit, wred_mem, &qentry, field)) {
        *flags |= BCM_COSQ_DISCARD_ENABLE;
    }

    if (wred_mem == MMU_WRED_QUEUE_CONFIG_BUFFERm) {
        if (soc_mem_field32_get(unit, wred_mem, &qentry, ECN_MARKING_ENf)) {
            *flags |= BCM_COSQ_DISCARD_MARK_CONGESTION;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_add
 * Purpose:
 *     Create a cosq gport structure
 * Parameters:
 *     unit  - (IN) unit number
 *     port  - (IN) port number
 *     numq  - (IN) number of COS queues
 *     flags - (IN) flags (BCM_COSQ_GPORT_XXX)
 *     gport - (OUT) GPORT identifier
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_add(int unit, bcm_gport_t port, int numq, uint32 flags,
                      bcm_gport_t *gport)
{
    soc_info_t *si;
    _bcm_kt_mmu_info_t *mmu_info;
	_bcm_kt_cosq_port_info_t *port_info;
    _bcm_kt_cosq_node_t *node = NULL;
    bcm_module_t local_modid, modid_out;
    bcm_port_t local_port, port_out;
    int id;
    uint32 sched_encap;
    _bcm_kt_cosq_list_t *list;
    int max_queues;

    BCM_DEBUG(BCM_DBG_COSQ,
              ("bcm_kt_cosq_gport_add: unit=%d port=0x%x numq=%d flags=0x%x\n",
               unit, port, numq, flags));

    if (gport == NULL) {
        return BCM_E_PARAM;
    }

    si = &SOC_INFO(unit);

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_localport_resolve(unit, port, &local_port));

    if (local_port < 0) {
        return BCM_E_PORT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];
    switch (flags) {
    case BCM_COSQ_GPORT_UCAST_QUEUE_GROUP:
    case BCM_COSQ_GPORT_VLAN_UCAST_QUEUE_GROUP:
        if (numq != 1) {
            return BCM_E_PARAM;
        }

        if ((flags ==  BCM_COSQ_GPORT_VLAN_UCAST_QUEUE_GROUP) &&
            (si->port_num_ext_cosq[local_port] == 0)) {
            return BCM_E_PARAM;
        }    

        port_info = &mmu_info->port[local_port];
        
        for (id = port_info->q_offset; id < port_info->q_limit; id++) {
            if (mmu_info->queue_node[id].numq == 0) {
                break;
            }
        }
        
        if (id == port_info->q_limit) {
            return BCM_E_RESOURCE;
        }            

        BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(*gport, local_port, id);
        node = &mmu_info->queue_node[id];
        node->gport = *gport;
        node->numq = numq;
        break;

    case BCM_COSQ_GPORT_SUBSCRIBER:
        if (numq != 1) {
            return BCM_E_PARAM;
        }

        list = &mmu_info->ext_qlist;
        max_queues = mmu_info->num_ext_queues;
        BCM_IF_ERROR_RETURN
            (_bcm_kt_node_index_get(list->bits, 0, max_queues,
                                mmu_info->qset_size, 1, &id));
        _bcm_kt_node_index_set(list, id, 1);
        id += mmu_info->num_base_queues;
        BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*gport, id);
        *gport |= (local_port << 16);
        node = &mmu_info->queue_node[id];
        node->gport = *gport;
        node->numq = numq;
        break;

    case BCM_COSQ_GPORT_SCHEDULER:
        /* passthru */
    case 0:  
        /*
         * Can not validate actual number of cosq until attach time.
         * Maximum number of input is 8 for SP or WRR node
         * expect -1 if number of inputs are unknown at this stage(WRR only)
         */
        if (numq == 0 || numq < -1 || numq > 8) {
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &local_modid));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET, local_modid,
                                     local_port, &modid_out, &port_out));

        if ( flags == 0) {
            /* this is a port level scheduler */
            id = port_out;

            if ( id < 0 || id >= _BCM_KT_NUM_PORT_SCHEDULERS) {
                return BCM_E_PARAM;
            }

            _bcm_kt_node_index_set(&mmu_info->sched_list, id, 1);
            node = &mmu_info->sched_node[id];
            sched_encap = (id << 8) | port_out;
            BCM_GPORT_SCHEDULER_SET(*gport, sched_encap);
            node->gport = *gport;
            node->level = _BCM_KT_COSQ_NODE_LEVEL_ROOT;
            node->hw_index = id;
            node->numq = numq;
            node->cosq_attached_to = 0;
        } else {
             /* allocate from sched_list, index will start from 36,
              * 0-35 is used for port(root) schedulers. */
            BCM_IF_ERROR_RETURN
                (_bcm_kt_node_index_get(mmu_info->sched_list.bits,
                                                _BCM_KT_NUM_PORT_SCHEDULERS,
                                mmu_info->num_nodes, 1, 1, &id));
            _bcm_kt_node_index_set(&mmu_info->sched_list, id, 1);
            node = &mmu_info->sched_node[id];
            sched_encap = (id << 8) | port_out;
            BCM_GPORT_SCHEDULER_SET(*gport, sched_encap);
            node->gport = *gport;
            node->numq = numq;
            node->cosq_attached_to = -1;
       }
        break;
        
    case BCM_COSQ_GPORT_WITH_ID:
        if (BCM_GPORT_IS_SCHEDULER(*gport)) {
            if (numq == 0 || numq < -1 || numq > 8) {
                return BCM_E_PARAM;
            }
            id = BCM_GPORT_SCHEDULER_GET(*gport);

            if (id != local_port) {
                return BCM_E_PARAM;
            }    
            
            /* allow only port level scheduler with id */
            if ( id < 0 || id >= _BCM_KT_NUM_PORT_SCHEDULERS) {
                return BCM_E_PARAM;
            }

            _bcm_kt_node_index_set(&mmu_info->sched_list, id, 1);
            node = &mmu_info->sched_node[id];
            node->gport = *gport;
            node->level = _BCM_KT_COSQ_NODE_LEVEL_ROOT;
            node->hw_index = id;
            node->numq = numq;
            node->cosq_attached_to = 0;
        } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(*gport)) {
            if (numq != 1) {
                return BCM_E_PARAM;
            }

            if (BCM_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(*gport) != local_port) {
                return BCM_E_PARAM;
            }    
            
            id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(*gport);
            /* allow only base queues */
            if (id > mmu_info->num_base_queues) {
                return BCM_E_PARAM;
            }    
            node = &mmu_info->queue_node[id];

            if (node->numq != 0) {
                return BCM_E_PARAM;
            } else {    
                node->gport = *gport;
                node->numq = numq;
            }    
        } else {
            return BCM_E_PARAM;
        }
        break;
        
    default:
        return BCM_E_PARAM;
    }

    BCM_DEBUG(BCM_DBG_COSQ,
              ("                       gport=0x%x\n",
               *gport));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_delete
 * Purpose:
 *     Destroy a cosq gport structure
 * Parameters:
 *     unit  - (IN) unit number
 *     gport - (IN) GPORT identifier
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_delete(int unit, bcm_gport_t gport)
{
    _bcm_kt_cosq_node_t *node;
    _bcm_kt_mmu_info_t *mmu_info;
    int encap_id;

    BCM_DEBUG(BCM_DBG_COSQ,
              ("bcm_kt_cosq_gport_delete: unit=%d gport=0x%x\n",
               unit, gport));

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, gport, NULL, NULL, NULL, &node));

    if (node->child != NULL) {
        bcm_kt_cosq_gport_delete(unit, node->child->gport);
    }

    if (node->sibling != NULL) {
        bcm_kt_cosq_gport_delete(unit, node->sibling->gport);
    }

    if (node->level != _BCM_KT_COSQ_NODE_LEVEL_ROOT && node->cosq_attached_to >= 0) {
        BCM_IF_ERROR_RETURN
            (bcm_kt_cosq_gport_detach(unit, node->gport, node->parent->gport, node->cosq_attached_to));
    }

    mmu_info = _bcm_kt_mmu_info[unit];

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) { /* unicast queue group */
        encap_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        /* subscriber queue group */
        encap_id = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);
        _bcm_kt_node_index_clear(&mmu_info->ext_qlist,
                                (encap_id - mmu_info->num_base_queues) , 1);
    } else {/* scheduler node */
        encap_id = (BCM_GPORT_SCHEDULER_GET(gport) >> 8) & 0x7ff;

        if (encap_id == 0) {
            encap_id = (BCM_GPORT_SCHEDULER_GET(gport) & 0xff);
        }

        _bcm_kt_node_index_clear(&mmu_info->sched_list, encap_id, 1);

   }

    _BCM_KT_COSQ_NODE_INIT(node);

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_get
 * Purpose:
 *     Get a cosq gport structure
 * Parameters:
 *     unit  - (IN) unit number
 *     gport - (IN) GPORT identifier
 *     port  - (OUT) port number
 *     numq  - (OUT) number of COS queues
 *     flags - (OUT) flags (BCM_COSQ_GPORT_XXX)
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_get(int unit, bcm_gport_t gport, bcm_gport_t *port,
                      int *numq, uint32 *flags)
{
    _bcm_kt_cosq_node_t *node;
    bcm_module_t modid;
    bcm_port_t local_port;
    int id;
    _bcm_gport_dest_t dest;
    _bcm_kt_mmu_info_t *mmu_info;

    if (port == NULL || numq == NULL || flags == NULL) {
        return BCM_E_PARAM;
    }

    BCM_DEBUG(BCM_DBG_COSQ,
              ("bcm_kt_cosq_gport_get: unit=%d gport=0x%x\n",
               unit, gport));

    mmu_info = _bcm_kt_mmu_info[unit];
    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, gport, NULL, &local_port, &id, &node));

    if (SOC_USE_GPORT(unit)) {
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &modid));
        dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
        dest.modid = modid;
        dest.port = local_port;
        BCM_IF_ERROR_RETURN(_bcm_esw_gport_construct(unit, &dest, port));
    } else {
        *port = local_port;
    }

    *numq = node->numq;

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        *flags = BCM_COSQ_GPORT_UCAST_QUEUE_GROUP;
            /* TODO for vlan qos */
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        *flags = BCM_COSQ_GPORT_SUBSCRIBER;
    } else if (BCM_GPORT_IS_SCHEDULER(gport)) {
        *flags = BCM_COSQ_GPORT_SCHEDULER;
    } else {
        *flags = 0;
    }

    BCM_DEBUG(BCM_DBG_COSQ,
              ("                       port=0x%x numq=%d flags=0x%x\n",
               *port, *numq, *flags));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_traverse
 * Purpose:
 *     Walks through the valid COSQ GPORTs and calls
 *     the user supplied callback function for each entry.
 * Parameters:
 *     unit       - (IN) unit number
 *     trav_fn    - (IN) Callback function.
 *     user_data  - (IN) User data to be passed to callback function
 * Returns:
 *     BCM_E_NONE - Success.
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_traverse(int unit, bcm_cosq_gport_traverse_cb cb,
                           void *user_data)
{
    _bcm_kt_cosq_node_t *port_info;
    _bcm_kt_mmu_info_t *mmu_info;
    bcm_module_t my_modid, modid_out;
    bcm_port_t port, port_out;
    bcm_gport_t gport;

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }
    mmu_info = _bcm_kt_mmu_info[unit];

    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));
    PBMP_ALL_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    my_modid, port, &modid_out, &port_out));
       /* BCM_GPORT_MODPORT_SET(gport, modid_out, port_out); */
       BCM_GPORT_LOCAL_SET(gport, port_out);

        /* root node */
        port_info = &mmu_info->sched_node[port_out];

        if (port_info->gport >= 0) {
            _bcm_kt_cosq_gport_traverse(unit, port_info->gport, cb, user_data);
        }
      }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_mapping_set
 * Purpose:
 *     Determine which COS queue a given priority currently maps to.
 * Parameters:
 *     unit     - (IN) unit number
 *     gport    - (IN) queue group GPORT identifier
 *     priority - (IN) priority value to map
 *     cosq     - (IN) COS queue to map to
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_mapping_set(int unit, bcm_port_t gport, bcm_cos_t priority,
                        bcm_cos_queue_t cosq)
{ 
    _bcm_kt_cosq_node_t *node;
    bcm_port_t port;
    int id;

    if (gport == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }
		
    if (priority < 0 || priority >= 16) {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, gport, NULL, &port, &id, &node));
	
    if (cosq >= node->numq) {
        return BCM_E_PARAM;
    }

    if (node->base_index < 0) {
        return BCM_E_NOT_FOUND;
    }

    return _bcm_kt_cosq_mapping_set_regular(unit, gport, priority, cosq);
}


/*
 * Function:
 *     bcm_kt_cosq_mapping_get
 * Purpose:
 *     Determine which COS queue a given priority currently maps to.
 * Parameters:
 *     unit     - (IN) unit number
 *     gport    - (IN) queue group GPORT identifier
 *     priority - (IN) priority value to map
 *     cosq     - (OUT) COS queue to map to
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_mapping_get(int unit, bcm_port_t gport, bcm_cos_t priority,
                        bcm_cos_queue_t *cosq)
{
	_bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *node;
    bcm_port_t port;
    int id, numq;
    bcm_cos_queue_t hw_cosq;
    soc_field_t field = COSf;
    int index;
    cos_map_sel_entry_t cos_map_sel_entry;
    void *entry_p;

    if (priority < 0 || priority >= 16) {
        return BCM_E_PARAM;
    }

    if (!BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, gport, NULL, &port, &id, &node));

    if (node->base_index < 0) {
        return BCM_E_NOT_FOUND;
    }

    mmu_info = _bcm_kt_mmu_info[unit];
    numq = mmu_info->port[port].q_limit - 
		   mmu_info->port[port].q_offset;
	
    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        field = numq < 8 ? COSf : HG_COSf;
    }

    BCM_IF_ERROR_RETURN
        (READ_COS_MAP_SELm(unit, MEM_BLOCK_ANY, port, &cos_map_sel_entry));
    index = soc_mem_field32_get(unit, COS_MAP_SELm, &cos_map_sel_entry,
                                SELECTf);
    index *= 16;

    entry_p = SOC_PROFILE_MEM_ENTRY(unit, _bcm_kt_cos_map_profile[unit],
                                    port_cos_map_entry_t *,
                                    index + priority);
    hw_cosq = soc_mem_field32_get(unit, PORT_COS_MAPm, entry_p, field);

    if (hw_cosq >= numq) {
        return BCM_E_NOT_FOUND;
    }
    *cosq = hw_cosq;

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_port_sched_set
 * Purpose:
 *     Set up class-of-service policy and corresponding weights and delay
 * Parameters:
 *     unit    - (IN) unit number
 *     pbm     - (IN) port bitmap
 *     mode    - (IN) Scheduling mode (BCM_COSQ_xxx)
 *     weights - (IN) Weights for each COS queue
 *     delay   - This parameter is not used
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     For non-ETS style scheduler (CPU port) only.
 */
int
bcm_kt_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
                           int mode, const int *weights, int delay)
{
    bcm_port_t port;
    int num_weights;

    BCM_PBMP_ITER(pbm, port) {
        num_weights = IS_CPU_PORT(unit, port) ? NUM_CPU_COSQ(unit) : 8;
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_port_sched_set(unit, port, 0, mode, num_weights,
                                    (int *)weights));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_port_sched_get
 * Purpose:
 *     Retrieve class-of-service policy and corresponding weights and delay
 * Parameters:
 *     unit     - (IN) unit number
 *     pbm      - (IN) port bitmap
 *     mode     - (OUT) Scheduling mode (BCM_COSQ_XXX)
 *     weights  - (OUT) Weights for each COS queue
 *     delay    - This parameter is not used
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     For non-ETS style scheduler (CPU port) only.
 */
int
bcm_kt_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
                           int *mode, int *weights, int *delay)
{
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *port_node;
    bcm_port_t port;

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }
    mmu_info = _bcm_kt_mmu_info[unit];

    BCM_PBMP_ITER(pbm, port) {
        if (IS_CPU_PORT(unit, port) && SOC_PBMP_NEQ(pbm, PBMP_CMIC(unit))) {
            continue;
        }
        /* root node */
        port_node = &mmu_info->sched_node[port];

        if (port_node->gport >= 0) {
            return _bcm_kt_cosq_port_sched_get(unit, port, 0, mode, 8, weights);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_sched_weight_max_get
 * Purpose:
 *     Retrieve maximum weights for given COS policy
 * Parameters:
 *     unit    - (IN) unit number
 *     mode    - (IN) Scheduling mode (BCM_COSQ_xxx)
 *     weight_max - (OUT) Maximum weight for COS queue.
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_sched_weight_max_get(int unit, int mode, int *weight_max)
{
    switch (mode) {
    case BCM_COSQ_STRICT:
        *weight_max = BCM_COSQ_WEIGHT_STRICT;
        break;
    case BCM_COSQ_ROUND_ROBIN:
        *weight_max = BCM_COSQ_WEIGHT_MIN;
        break;
    case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
    case BCM_COSQ_DEFICIT_ROUND_ROBIN:
        *weight_max =
            (1 << soc_mem_field_length(unit, LLS_L0_CHILD_WEIGHT_CFG_CNTm, C_WEIGHTf)) - 1;
        break;
    default:
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_bandwidth_set
 * Purpose:
 *     Configure COS queue bandwidth control
 * Parameters:
 *     unit          - (IN) unit number
 *     port          - (IN) port number or GPORT identifier
 *     cosq          - (IN) COS queue number
 *     min_quantum   - (IN)
 *     max_quantum   - (IN)
 *     burst_quantum - (IN)
 *     flags         - (IN)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
int
bcm_kt_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                               bcm_cos_queue_t cosq,
                               uint32 min_quantum, uint32 max_quantum,
                               uint32 burst_quantum, uint32 flags)
{
    if (cosq < 0) {
        return BCM_E_PARAM;
    }

    return _bcm_kt_cosq_bucket_set(unit, port, cosq, min_quantum,
                                   max_quantum, burst_quantum, flags);
}

/*
 * Function:
 *     bcm_kt_cosq_bandwidth_get
 * Purpose:
 *     Get COS queue bandwidth control configuration
 * Parameters:
 *     unit          - (IN) unit number
 *     port          - (IN) port number or GPORT identifier
 *     cosq          - (IN) COS queue number
 *     min_quantum   - (OUT)
 *     max_quantum   - (OUT)
 *     burst_quantum - (OUT)
 *     flags         - (OUT)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
int
bcm_kt_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                               bcm_cos_queue_t cosq,
                               uint32 *min_quantum, uint32 *max_quantum,
                               uint32 *burst_quantum, uint32 *flags)
{

    if (cosq < 0) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_bcm_kt_cosq_bucket_get(unit, port, cosq, min_quantum,
                                                max_quantum, burst_quantum,
                                                *flags));
    *flags = 0;

    return BCM_E_NONE;
}

int
bcm_kt_cosq_port_pps_set(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                         int pps)
{
    uint32 min, max, burst;

    if (!IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    } else if (cosq < 0 || cosq >= NUM_CPU_COSQ(unit)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_bucket_get(unit, port, cosq, &min, &max, &burst,
                                 _BCM_XGS_METER_FLAG_PACKET_MODE));

    return _bcm_kt_cosq_bucket_set(unit, port, cosq, min, pps, burst,
                                   _BCM_XGS_METER_FLAG_PACKET_MODE);
}

int
bcm_kt_cosq_port_pps_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                         int *pps)
{
    uint32 min, max, burst;

    if (!IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    } else if (cosq < 0 || cosq >= NUM_CPU_COSQ(unit)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_bucket_get(unit, port, cosq, &min, &max, &burst,
                                 _BCM_XGS_METER_FLAG_PACKET_MODE));
    *pps = max;

    return BCM_E_NONE;
}

int
bcm_kt_cosq_port_burst_set(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                           int burst)
{
    uint32 min, max, cur_burst;

    if (!IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    } else if (cosq < 0 || cosq >= NUM_CPU_COSQ(unit)) {
        return BCM_E_PARAM;
    }

    /* Get the current PPS and BURST settings */
    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_bucket_get(unit, port, cosq, &min, &max, &cur_burst,
                                 _BCM_XGS_METER_FLAG_PACKET_MODE));

    /* Replace the current BURST setting, keep PPS the same */
    return _bcm_kt_cosq_bucket_set(unit, port, cosq, min, max, burst,
                                   _BCM_XGS_METER_FLAG_PACKET_MODE);
}

int
bcm_kt_cosq_port_burst_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                           int *burst)
{
    uint32 min, max, cur_burst;

    if (!IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    } else if (cosq < 0 || cosq >= NUM_CPU_COSQ(unit)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_bucket_get(unit, port, cosq, &min, &max, &cur_burst,
                                 _BCM_XGS_METER_FLAG_PACKET_MODE));
    *burst = cur_burst;

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_sched_set
 * Purpose:
 *     Configure COS queue scheduler setting
 * Parameters:
 *      unit   - (IN) unit number
 *      gport  - (IN) GPORT identifier
 *      cosq   - (IN) COS queue to configure, -1 for all COS queues
 *      mode   - (IN) Scheduling mode, one of BCM_COSQ_xxx
 *      weight - (IN) queue weight
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_kt_cosq_gport_sched_set(int unit, bcm_gport_t gport,
                            bcm_cos_queue_t cosq, int mode, int weight)
{
    int rv, numq, i, count;

    if (cosq == -1) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, gport, cosq,
                                        _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                        NULL, NULL, &numq));
        cosq = 0;
    } else {
        numq = 1;
    }

    count = 0;
    for (i = 0; i < numq; i++) {
        rv = _bcm_kt_cosq_sched_set(unit, gport, cosq + i, mode, 1, &weight);
        if (rv == BCM_E_NOT_FOUND) {
            continue;
        } else if (BCM_FAILURE(rv)) {
            return rv;
        } else {
            count++;
        }
    }

    return count > 0 ? BCM_E_NONE : BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_sched_get
 * Purpose:
 *     Get COS queue scheduler setting
 * Parameters:
 *     unit   - (IN) unit number
 *     gport  - (IN) GPORT identifier
 *     cosq   - (IN) COS queue to get, -1 for any queue
 *     mode   - (OUT) Scheduling mode, one of BCM_COSQ_xxx
 *     weight - (OUT) queue weight
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_sched_get(int unit, bcm_gport_t gport, bcm_cos_queue_t cosq,
                            int *mode, int *weight)
{
    int rv, numq, i;

    if (cosq == -1) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, gport, cosq,
                                        _BCM_KT_COSQ_INDEX_STYLE_SCHEDULER,
                                        NULL, NULL, &numq));
        cosq = 0;
    } else {
        numq = 1;
    }

    for (i = 0; i < numq; i++) {
        rv = _bcm_kt_cosq_sched_get(unit, gport, cosq + i, mode, 1, weight);
        if (rv == BCM_E_NOT_FOUND) {
            continue;
        } else {
            return rv;
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_bandwidth_set
 * Purpose:
 *     Configure COS queue bandwidth control
 * Parameters:
 *     unit   - (IN) unit number
 *     gport  - (IN) GPORT identifier
 *     cosq   - (IN) COS queue to configure, -1 for all COS queues
 *     kbits_sec_min - (IN) minimum bandwidth, kbits/sec
 *     kbits_sec_max - (IN) maximum bandwidth, kbits/sec
 *     flags  - (IN) BCM_COSQ_BW_*
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_bandwidth_set(int unit, bcm_gport_t gport,
                                bcm_cos_queue_t cosq, uint32 kbits_sec_min,
                                uint32 kbits_sec_max, uint32 flags)
{
    int numq, i;

    if (cosq == -1) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, gport, cosq,
                                        _BCM_KT_COSQ_INDEX_STYLE_BUCKET,
                                        NULL, NULL, &numq));
        cosq = 0;
    } else {
        numq = 1;
    }

    for (i = 0; i < numq; i++) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_bucket_set(unit, gport, cosq + i, kbits_sec_min,
                                     kbits_sec_max, kbits_sec_max, flags));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_bandwidth_get
 * Purpose:
 *     Get COS queue bandwidth control configuration
 * Parameters:
 *     unit   - (IN) unit number
 *     gport  - (IN) GPORT identifier
 *     cosq   - (IN) COS queue to get, -1 for any COS queue
 *     kbits_sec_min - (OUT) minimum bandwidth, kbits/sec
 *     kbits_sec_max - (OUT) maximum bandwidth, kbits/sec
 *     flags  - (OUT) BCM_COSQ_BW_*
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_bandwidth_get(int unit, bcm_gport_t gport,
                                bcm_cos_queue_t cosq, uint32 *kbits_sec_min,
                                uint32 *kbits_sec_max, uint32 *flags)
{
    uint32 kbits_sec_burst;

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_bucket_get(unit, gport, cosq == -1 ? 0 : cosq,
                                 kbits_sec_min, kbits_sec_max,
                                 &kbits_sec_burst, *flags));
    *flags = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_bandwidth_burst_set
 * Purpose:
 *     Configure COS queue bandwidth burst setting
 * Parameters:
 *      unit        - (IN) unit number
 *      gport       - (IN) GPORT identifier
 *      cosq        - (IN) COS queue to configure, -1 for all COS queues
 *      kbits_burst - (IN) maximum burst, kbits
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_kt_cosq_gport_bandwidth_burst_set(int unit, bcm_gport_t gport,
                                      bcm_cos_queue_t cosq,
                                      uint32 kbits_burst)
{
    int numq, i;
    uint32 kbits_sec_min, kbits_sec_max, kbits_sec_burst;

    if (cosq == -1) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, gport, cosq,
                                        _BCM_KT_COSQ_INDEX_STYLE_BUCKET,
                                        NULL, NULL, &numq));
        cosq = 0;
    } else {
        numq = 1;
    }

    for (i = 0; i < numq; i++) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_bucket_get(unit, gport, cosq + i, &kbits_sec_min,
                                     &kbits_sec_max, &kbits_sec_burst, 0));
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_bucket_set(unit, gport, cosq + i, kbits_sec_min,
                                     kbits_sec_max, kbits_burst, 0));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_bandwidth_burst_get
 * Purpose:
 *     Get COS queue bandwidth burst setting
 * Parameters:
 *     unit        - (IN) unit number
 *     gport       - (IN) GPORT identifier
 *     cosq        - (IN) COS queue to get, -1 for any queue
 *     kbits_burst - (OUT) maximum burst, kbits
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_bandwidth_burst_get(int unit, bcm_gport_t gport,
                                       bcm_cos_queue_t cosq,
                                       uint32 *kbits_burst)
{
    uint32 kbits_sec_min, kbits_sec_max;

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_bucket_get(unit, gport, cosq == -1 ? 0 : cosq,
                                 &kbits_sec_min, &kbits_sec_max, kbits_burst,
                                 0));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_discard_set
 * Purpose:
 *     Configure gport WRED setting
 * Parameters:
 *     unit    - (IN) unit number
 *     port    - (IN) GPORT identifier
 *     cosq    - (IN) COS queue to configure, -1 for all COS queues
 *     discard - (IN) discard settings
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_discard_set(int unit, bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               bcm_cosq_gport_discard_t *discard)
{
    int numq, i;
    uint32 min_thresh, max_thresh;
    int cell_size, cell_field_max;

    if (discard == NULL ||
        discard->gain < 0 || discard->gain > 15 ||
        discard->drop_probability < 0 || discard->drop_probability > 100) {
        return BCM_E_PARAM;
    }

    if (cosq < -1) {
        return BCM_E_PARAM;
    }

    cell_size = _BCM_KT_COSQ_CELLSIZE;
    cell_field_max = KT_CELL_FIELD_MAX;

    min_thresh = discard->min_thresh;
    max_thresh = discard->max_thresh;
    if (discard->flags & BCM_COSQ_DISCARD_BYTES) {
        /* Convert bytes to cells */
        min_thresh += (cell_size - 1);
        min_thresh /= cell_size;

        max_thresh += (cell_size - 1);
        max_thresh /= cell_size;

        if ((min_thresh > cell_field_max) ||
            (max_thresh > cell_field_max)) {
            return BCM_E_PARAM;
        }
    } else { /* BCM_COSQ_DISCARD_PACKETS */
        if ((min_thresh > KT_CELL_FIELD_MAX) ||
            (max_thresh > KT_CELL_FIELD_MAX)) {
            return BCM_E_PARAM;
        }
    }

    if (cosq == -1) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, gport, cosq,
                                        _BCM_KT_COSQ_INDEX_STYLE_WRED,
                                        NULL, NULL, &numq));
        cosq = 0;
    } else {
        numq = 1;
    }

    for (i = 0; i < numq; i++) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_wred_set(unit, gport, cosq + i, discard->flags,
                                   min_thresh, max_thresh,
                                   discard->drop_probability, discard->gain,
                                   FALSE));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_discard_get
 * Purpose:
 *     Get port WRED setting
 * Parameters:
 *     unit    - (IN) unit number
 *     port    - (IN) GPORT identifier
 *     cosq    - (IN) COS queue to get, -1 for any queue
 *     discard - (OUT) discard settings
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_discard_get(int unit, bcm_gport_t gport,
                              bcm_cos_queue_t cosq,
                              bcm_cosq_gport_discard_t *discard)
{
    uint32 min_thresh, max_thresh;

    if (discard == NULL) {
        return BCM_E_PARAM;
    }

    if (cosq < -1) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_wred_get(unit, gport, cosq == -1 ? 0 : cosq,
                               &discard->flags, &min_thresh, &max_thresh,
                               &discard->drop_probability, &discard->gain));

    /* Convert number of cells to number of bytes */
    discard->min_thresh = min_thresh * _BCM_KT_COSQ_CELLSIZE;
    discard->max_thresh = max_thresh * _BCM_KT_COSQ_CELLSIZE;

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_discard_set
 * Purpose:
 *     Get port WRED setting
 * Parameters:
 *     unit    - (IN) unit number
 *     flags   - (IN) flags
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_discard_set(int unit, uint32 flags)
{
    bcm_port_t port;
    bcm_cos_queue_t cosq;
    int numq;

    flags &= ~(BCM_COSQ_DISCARD_NONTCP | BCM_COSQ_DISCARD_COLOR_ALL);

    PBMP_PORT_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, port, -1,
                                        _BCM_KT_COSQ_INDEX_STYLE_WRED, NULL,
                                        NULL, &numq));
        for (cosq = 0; cosq < numq; cosq++) {
            BCM_IF_ERROR_RETURN
                (_bcm_kt_cosq_wred_set(unit, port, cosq, flags, 0, 0, 0, 0,
                                       FALSE));
        }
    }

    return BCM_E_NONE;

}

/*
 * Function:
 *     bcm_kt_cosq_discard_get
 * Purpose:
 *     Get port WRED setting
 * Parameters:
 *     unit    - (IN) unit number
 *     flags   - (OUT) flags
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_discard_get(int unit, uint32 *flags)
{
    bcm_port_t port;

    PBMP_PORT_ITER(unit, port) {
        *flags = 0;
        /* use setting from hardware cosq index 0 of the first port */
        return _bcm_kt_cosq_wred_get(unit, port, 0, flags, NULL, NULL, NULL,
                                     NULL);
    }

    return BCM_E_NOT_FOUND;

}

/*
 * Function:
 *     bcm_kt_cosq_discard_port_set
 * Purpose:
 *     Configure port WRED setting
 * Parameters:
 *     unit          - (IN) unit number
 *     port          - (IN) port number or GPORT identifier
 *     cosq          - (IN) COS queue number
 *     color         - (IN)
 *     drop_start    - (IN)
 *     drop_slot     - (IN)
 *     average_time  - (IN)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
int
bcm_kt_cosq_discard_port_set(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                             uint32 color, int drop_start, int drop_slope,
                             int average_time)
{
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_kt_cosq_discard_port_get
 * Purpose:
 *     Get port WRED setting
 * Parameters:
 *     unit          - (IN) unit number
 *     port          - (IN) port number or GPORT identifier
 *     cosq          - (IN) COS queue number
 *     color         - (OUT)
 *     drop_start    - (OUT)
 *     drop_slot     - (OUT)
 *     average_time  - (OUT)
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If port is any form of local port, cosq is the hardware queue index.
 */
int
bcm_kt_cosq_discard_port_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                             uint32 color, int *drop_start, int *drop_slope,
                             int *average_time)
{
    return SOC_E_UNAVAIL;
}

int
bcm_kt_cosq_stat_set(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                     bcm_cosq_stat_t stat, uint64 value)
{
    bcm_port_t local_port;
    int startq, numq, i;
    uint64 value64;

    if (port == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }

    if (!(BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
         BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
         BCM_GPORT_IS_SCHEDULER(port))) {
        return BCM_E_PARAM;
    }    

    if (BCM_GPORT_IS_SCHEDULER(port) && cosq != BCM_COS_INVALID) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_index_resolve
        (unit, port, cosq, _BCM_KT_COSQ_INDEX_STYLE_UCAST_QUEUE,
         &local_port, &startq, &numq));


    switch (stat) {
    case bcmCosqStatDroppedPackets:
        if (!BCM_GPORT_IS_SCHEDULER(port)) {      
            BCM_IF_ERROR_RETURN
                 (soc_counter_set(unit, local_port,
                                  SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC, startq,
                                  value));
        } else {
            COMPILER_64_ZERO(value64);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_set(unit, local_port,
                                     SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC,
                                     startq + i, value64));
            }
        }
        break;
    case bcmCosqStatDroppedBytes:
        if (!BCM_GPORT_IS_SCHEDULER(port)) {      
            BCM_IF_ERROR_RETURN
                (soc_counter_set(unit, local_port,
                                 SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC, startq,
                                 value));
        } else {
            COMPILER_64_ZERO(value64);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_set(unit, local_port,
                                     SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC,
                                     startq + i, value64));
            }
        }
        break;
    case bcmCosqStatRedDiscardDroppedPackets:
        if (!BCM_GPORT_IS_SCHEDULER(port)) {      
            BCM_IF_ERROR_RETURN
                (soc_counter_set(unit, local_port,
                                 SOC_COUNTER_NON_DMA_COSQ_WRED_PKT_RED, startq, 
                                 value));
        } else {
            COMPILER_64_ZERO(value64);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_set(unit, local_port,
                                     SOC_COUNTER_NON_DMA_COSQ_WRED_PKT_RED,
                                     startq + i, value64));
            }
        }
        break;
    case bcmCosqStatOutPackets:
        if (!BCM_GPORT_IS_SCHEDULER(port)) {      
            BCM_IF_ERROR_RETURN
                (soc_counter_set(unit, local_port,
                                 SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT,
                                 startq, value));
        } else {
            COMPILER_64_ZERO(value64);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_set(unit, local_port,
                                     SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT,
                                     startq + i, value64));
            }        
        }
        break;
    case bcmCosqStatOutBytes:
        if (!BCM_GPORT_IS_SCHEDULER(port)) {      
            BCM_IF_ERROR_RETURN
                (soc_counter_set(unit, local_port,
                                 SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE,
                                 startq, value));
        } else {
            COMPILER_64_ZERO(value64);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_set(unit, local_port,
                                     SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE,
                                     startq + i, value64));
            }            
        }
        break;
    default:
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int
bcm_kt_cosq_stat_get(int unit, bcm_port_t port, bcm_cos_queue_t cosq,
                     bcm_cosq_stat_t stat, uint64 *value)
{
    bcm_port_t local_port;
    int startq, numq, i;
    uint64 sum, value64;

    if (port == BCM_GPORT_INVALID) {
        return (BCM_E_PORT);
    }

    if (!(BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
         BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
         BCM_GPORT_IS_SCHEDULER(port))) {
        return BCM_E_PARAM;
    }    

    if (value == NULL) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SCHEDULER(port) && cosq != BCM_COS_INVALID) {
        return BCM_E_PARAM;
    }    

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_index_resolve
        (unit, port, cosq, _BCM_KT_COSQ_INDEX_STYLE_UCAST_QUEUE,
         &local_port, &startq, &numq));

    switch (stat) {
    case bcmCosqStatDroppedPackets:
        if (!BCM_GPORT_IS_SCHEDULER(port)) { 
            BCM_IF_ERROR_RETURN
                 (soc_counter_get(unit, local_port,
                                  SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC, startq,
                                  value));
        } else {
            COMPILER_64_ZERO(sum);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_get(unit, local_port,
                                     SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC,
                                     startq + i, &value64));
                COMPILER_64_ADD_64(sum, value64);
            }
            *value = sum;
        }
        break;
    case bcmCosqStatDroppedBytes:
        if (!BCM_GPORT_IS_SCHEDULER(port)) { 
            BCM_IF_ERROR_RETURN
                (soc_counter_get(unit, local_port,
                                 SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC, startq,
                                 value));
        } else {
            COMPILER_64_ZERO(sum);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_get(unit, local_port,
                                     SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC,
                                     startq + i, &value64));
                COMPILER_64_ADD_64(sum, value64);
            }
            *value = sum;
        }
        break;
    case bcmCosqStatRedDiscardDroppedPackets:
        if (!BCM_GPORT_IS_SCHEDULER(port)) { 
            BCM_IF_ERROR_RETURN
                (soc_counter_get(unit, local_port,
                                 SOC_COUNTER_NON_DMA_COSQ_WRED_PKT_RED, startq, 
                                 value));
        } else {
            COMPILER_64_ZERO(sum);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_get(unit, local_port,
                                     SOC_COUNTER_NON_DMA_COSQ_WRED_PKT_RED,
                                     startq + i, &value64));
                COMPILER_64_ADD_64(sum, value64);
            }
            *value = sum;
        }
        break;
    case bcmCosqStatOutPackets:
        if (!BCM_GPORT_IS_SCHEDULER(port)) { 
            BCM_IF_ERROR_RETURN
                (soc_counter_get(unit, local_port,
                                 SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT,
                                 startq, value));
        } else {
            COMPILER_64_ZERO(sum);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_get(unit, local_port,
                                     SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT,
                                     startq + i, &value64));
                COMPILER_64_ADD_64(sum, value64);
            }
            *value = sum;
        }
        break;
    case bcmCosqStatOutBytes:
        if (!BCM_GPORT_IS_SCHEDULER(port)) { 
            BCM_IF_ERROR_RETURN
                (soc_counter_get(unit, local_port,
                                 SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE,
                                 startq, value));
        }else {
            COMPILER_64_ZERO(sum);
            for (i = 0; i < numq; i++) {
                BCM_IF_ERROR_RETURN
                    (soc_counter_get(unit, local_port,
                                     SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE,
                                     startq + i, &value64));
                COMPILER_64_ADD_64(sum, value64);
            }
            *value = sum;    
        }
        break;
    default:
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int bcm_kt_cosq_port_get(int unit, int encap_id, bcm_port_t *port) 
{
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *node;
    lls_l0_parent_entry_t l0_parent;
    lls_l1_parent_entry_t l1_parent;
    lls_l2_parent_entry_t l2_parent;
    int index;

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];
    
    if (encap_id >= mmu_info->num_queues) {
        return BCM_E_PARAM;
    }    

    node = &mmu_info->queue_node[encap_id];

    if (node->cosq_attached_to < 0) {
        return BCM_E_PARAM;
    }

    SOC_IF_ERROR_RETURN
            (READ_LLS_L2_PARENTm(unit, MEM_BLOCK_ALL, node->hw_index, &l2_parent));
    index = soc_mem_field32_get(unit, LLS_L2_PARENTm, &l2_parent, C_PARENTf);

    SOC_IF_ERROR_RETURN
            (READ_LLS_L1_PARENTm(unit, MEM_BLOCK_ALL, index, &l1_parent));
    index = soc_mem_field32_get(unit, LLS_L1_PARENTm, &l1_parent, C_PARENTf);

    SOC_IF_ERROR_RETURN
            (READ_LLS_L0_PARENTm(unit, MEM_BLOCK_ALL, index, &l0_parent));
    
    *port = soc_mem_field32_get(unit, LLS_L0_PARENTm, &l0_parent, C_PARENTf);;

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT

#define BCM_WB_VERSION_1_0                     SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION                 BCM_WB_VERSION_1_0

#define _BCM_KT_COSQ_WB_END_NODE_VALUE         0xffffffff

#define _BCM_KT_COSQ_WB_NODE_COSQ_MASK         0x3f    /* LW-1 [5:0] */
#define _BCM_KT_COSQ_WB_NODE_COSQ_SHIFT        0
#define _BCM_KT_COSQ_WB_NODE_NUMQ_MASK         0x3f    /* LW-1 [11:6] */
#define _BCM_KT_COSQ_WB_NODE_NUMQ_SHIFT        6
#define _BCM_KT_COSQ_WB_NODE_HW_INDEX_MASK     0xfff   /* LW-1 [23:12] */
#define _BCM_KT_COSQ_WB_NODE_HW_INDEX_SHIFT    12
#define _BCM_KT_COSQ_WB_NODE_LEVEL_MASK        0x3     /* LW-1 [25:24 */
#define _BCM_KT_COSQ_WB_NODE_LEVEL_SHIFT       24


#define SET_FIELD(_field, _value)                       \
    (((_value) & _BCM_KT_COSQ_WB_## _field## _MASK) <<  \
     _BCM_KT_COSQ_WB_## _field## _SHIFT)
#define GET_FIELD(_field, _byte)                        \
    (((_byte) >> _BCM_KT_COSQ_WB_## _field## _SHIFT) &  \
     _BCM_KT_COSQ_WB_## _field## _MASK)

STATIC int
_bcm_kt_cosq_wb_alloc(int unit)
{
    soc_scache_handle_t scache_handle;
    uint8 *scache_ptr;
    _bcm_kt_mmu_info_t *mmu_info;
    int rv, alloc_size;

    mmu_info = _bcm_kt_mmu_info[unit];

    alloc_size = 0;
    alloc_size += sizeof(mmu_info->num_base_queues);
    alloc_size += sizeof(mmu_info->num_ext_queues);
    alloc_size += sizeof(mmu_info->qset_size);
    alloc_size += sizeof(mmu_info->num_queues);
    alloc_size += sizeof(mmu_info->num_nodes);
    alloc_size += sizeof(mmu_info->max_nodes);
    alloc_size += (_BCM_KT_NUM_TOTAL_SCHEDULERS * sizeof(uint32) * 3);
    alloc_size += (_BCM_KT_NUM_L2_QUEUES * sizeof(uint32) * 2);
    alloc_size += sizeof(uint32);

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_COSQ, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, TRUE, alloc_size,
                                 &scache_ptr, BCM_WB_DEFAULT_VERSION, NULL);
    if (rv == BCM_E_NOT_FOUND) {
        rv = BCM_E_NONE;
    }

    return rv;
}

STATIC uint32
_bcm_kt_cosq_wb_encode_node(int unit, _bcm_kt_cosq_node_t *node)
{
    uint32 data = 0;

    data  = SET_FIELD(NODE_COSQ, node->cosq_attached_to);
    data |= SET_FIELD(NODE_NUMQ, node->numq);
    data |= SET_FIELD(NODE_LEVEL, node->level);
    data |= SET_FIELD(NODE_HW_INDEX, node->hw_index);
    
    return data;
}

int
bcm_kt_cosq_sync(int unit)
{
    soc_scache_handle_t scache_handle;
    uint8 *scache_ptr, *ptr;
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *port_node, *l0_node, *l1_node, *l2_node;
    bcm_port_t port;
    int i;
    uint32 node_data;

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_COSQ, 0);
    BCM_IF_ERROR_RETURN
        (_bcm_esw_scache_ptr_get(unit, scache_handle, FALSE, 0,
                                 &scache_ptr, BCM_WB_DEFAULT_VERSION, NULL));

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];

    ptr = scache_ptr;

    *(((uint32 *)ptr)) = mmu_info->num_base_queues;
    ptr += sizeof(uint32);

    *(((uint32 *)ptr)) = mmu_info->num_ext_queues;
    ptr += sizeof(uint32);

    *(((uint32 *)ptr)) = mmu_info->qset_size;
    ptr += sizeof(uint32);

    *(((uint32 *)ptr)) = mmu_info->num_queues;
    ptr += sizeof(uint32);

    *(((uint32 *)ptr)) = mmu_info->num_nodes;
    ptr += sizeof(uint32);

    for (i = 0; i < _BCM_KT_COSQ_NODE_LEVEL_MAX; i++) {
        *(((uint32 *)ptr)) = mmu_info->max_nodes[i];
        ptr += sizeof(uint32);
    }

    PBMP_ALL_ITER(unit, port) {
        port_node = &mmu_info->sched_node[port];

        if (port_node->cosq_attached_to < 0) {
            continue;
        }

        if (port_node->child != NULL) {
            *(((uint32 *)ptr)) = port_node->gport;
            ptr += sizeof(uint32);
            
            node_data = _bcm_kt_cosq_wb_encode_node(unit, port_node);
            *(((uint32 *)ptr)) = node_data;
            ptr += sizeof(uint32);

            *(((uint32 *)ptr)) = port_node->base_index;
            ptr += sizeof(uint32);
        }

        for (l0_node = port_node->child; l0_node != NULL;
             l0_node = l0_node->sibling) {
            *(((uint32 *)ptr)) = l0_node->gport;
            ptr += sizeof(uint32);
            
            node_data = _bcm_kt_cosq_wb_encode_node(unit, l0_node);
            *(((uint32 *)ptr)) = node_data;
            ptr += sizeof(uint32);

            *(((uint32 *)ptr)) = l0_node->base_index;
            ptr += sizeof(uint32);

            for (l1_node = l0_node->child; l1_node != NULL;
                 l1_node = l1_node->sibling) {
                *(((uint32 *)ptr)) = l1_node->gport;
                ptr += sizeof(uint32);

                node_data = _bcm_kt_cosq_wb_encode_node(unit, l1_node);
                *(((uint32 *)ptr)) = node_data;
                ptr += sizeof(uint32);

                *(((uint32 *)ptr)) = l1_node->base_index;
                ptr += sizeof(uint32);
                
                for (l2_node = l1_node->child; l2_node != NULL;
                     l2_node = l2_node->sibling) {
                    *(((uint32 *)ptr)) = l2_node->gport;
                    ptr += sizeof(uint32);
                     
                    node_data = _bcm_kt_cosq_wb_encode_node(unit, l2_node);
                    *(((uint32 *)ptr)) = node_data;
                    ptr += sizeof(uint32);

                }
            }

        }

    }

    *(((uint32 *)ptr)) = _BCM_KT_COSQ_WB_END_NODE_VALUE;

    return BCM_E_NONE; 
}

int
bcm_kt_cosq_reinit(int unit)
{
    soc_scache_handle_t scache_handle;
    uint8 *scache_ptr, *ptr;
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *port_node = NULL;
    _bcm_kt_cosq_node_t *queue_node = NULL;
    _bcm_kt_cosq_node_t *node = NULL;
    _bcm_kt_cosq_node_t *l0_node = NULL;
    _bcm_kt_cosq_node_t *l1_node = NULL;
    int rv, stable_size = 0;
    bcm_port_t port = 0;
    bcm_gport_t gport;
    int i, index, numq;
    uint32 node_data;
    uint32 encap_id;
    int set_index, cosq;
    uint32 entry[SOC_MAX_MEM_WORDS];
    soc_profile_mem_t *profile_mem;
    _bcm_kt_cosq_list_t *list;
    soc_mem_t mem;
    mmu_wred_queue_config_buffer_entry_t qentry;

    SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
    if ((stable_size == 0) || (SOC_WARM_BOOT_SCACHE_IS_LIMITED(unit))) {
        /* COSQ warmboot requires extended scache support i.e. level2 warmboot*/
        return BCM_E_NONE;
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_COSQ, 0);
    rv = _bcm_esw_scache_ptr_get(unit, scache_handle, FALSE, 0, &scache_ptr,
                                 BCM_WB_DEFAULT_VERSION, NULL);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];

    ptr = scache_ptr;

    mmu_info->num_base_queues = *(((uint32 *)ptr));
    ptr += sizeof(uint32);

    mmu_info->num_ext_queues = *(((uint32 *)ptr));
    ptr += sizeof(uint32);

    mmu_info->qset_size = *(((uint32 *)ptr));
    ptr += sizeof(uint32);

    mmu_info->num_queues = *(((uint32 *)ptr));
    ptr += sizeof(uint32);

    mmu_info->num_nodes = *(((uint32 *)ptr));
    ptr += sizeof(uint32);

    for (i = 0; i < _BCM_KT_COSQ_NODE_LEVEL_MAX; i++) {
        mmu_info->max_nodes[i] = *(((uint32 *)ptr));
        ptr += sizeof(uint32);
    }

    while (*(((uint32 *)ptr)) != _BCM_KT_COSQ_WB_END_NODE_VALUE) {
        gport = *(((uint32 *)ptr));
        ptr += sizeof(uint32);
        node_data = *(((uint32 *)ptr));
        ptr += sizeof(uint32);
                     
        if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
            list = &mmu_info->ext_qlist;
            encap_id = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);
            node = &mmu_info->queue_node[encap_id]; 
            encap_id -= mmu_info->num_base_queues;
            _bcm_kt_node_index_set(list, encap_id, 1);
        } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
            encap_id = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
            node = &mmu_info->queue_node[encap_id]; 
        } else {
            encap_id = (BCM_GPORT_SCHEDULER_GET(gport) >> 8) & 0x7ff;
            if (encap_id == 0) {
                encap_id = (BCM_GPORT_SCHEDULER_GET(gport) & 0xff);
            }
            list = &mmu_info->sched_list;
            _bcm_kt_node_index_set(list, encap_id, 1);
            node = &mmu_info->sched_node[encap_id]; 
            node->first_child = 4095;
            node->base_index = *(((uint32 *)ptr));
            ptr += sizeof(uint32);
        }

        node->gport = gport;
        node->numq  = GET_FIELD(NODE_NUMQ, node_data);
        node->cosq_attached_to = GET_FIELD(NODE_COSQ, node_data);
        node->level = GET_FIELD(NODE_LEVEL, node_data);
        node->hw_index = GET_FIELD(NODE_HW_INDEX, node_data);

        switch (node->level) {
            case _BCM_KT_COSQ_NODE_LEVEL_ROOT:
                port_node = node;
                if (port_node != NULL) {
                    port_node->parent = NULL;
                    port_node->sibling = NULL;
                    port_node->child = NULL;
                }
                break;

            case _BCM_KT_COSQ_NODE_LEVEL_L0:
                l0_node = node;
                list = &mmu_info->l0_sched_list;
                
                if (port_node != NULL && l0_node != NULL) {
                    l0_node->parent = port_node;
                    l0_node->sibling = port_node->child;
                    
                    port_node->child = l0_node;
                    port_node->num_child++;
                    
                    if (port_node->numq > 0) {
                        if (l0_node->hw_index < port_node->first_child) {
                            port_node->first_child = l0_node->hw_index;
                        }
                        if (port_node->num_child == 1) {
                            _bcm_kt_node_index_set(list, port_node->base_index, port_node->numq);
                        }
                    } else {
                        _bcm_kt_node_index_set(list, l0_node->hw_index, 1);
                    }
                }
                break;

            case _BCM_KT_COSQ_NODE_LEVEL_L1:
                l1_node = node;
                list = &mmu_info->l1_sched_list;
                if (l0_node != NULL && l1_node != NULL) {
                    l1_node->parent = l0_node;
                    l1_node->sibling = l0_node->child;
                    
                    l0_node->child = l1_node;
                    l0_node->num_child++;

                    if (l0_node->numq > 0) {
                        if (l1_node->hw_index < l0_node->first_child) {
                            l0_node->first_child = l1_node->hw_index;
                        }
                        if (l0_node->num_child == 1) {
                            _bcm_kt_node_index_set(list, l0_node->base_index, l0_node->numq);
                        }
                    } else {
                        _bcm_kt_node_index_set(list, l1_node->hw_index, 1);
                    }
                }
                break;

            case _BCM_KT_COSQ_NODE_LEVEL_L2:
                queue_node = node;
                if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
                    list = &mmu_info->l2_base_qlist;
                } else {
                    list = &mmu_info->l2_ext_qlist;
                }
                if (l1_node != NULL && queue_node != NULL) {
                    queue_node->parent = l1_node;
                    queue_node->sibling = l1_node->child;
                    queue_node->hw_index = GET_FIELD(NODE_HW_INDEX, node_data);
                    l1_node->child = queue_node;
                    l1_node->num_child++;

                    if (l1_node->numq > 0) {    
                        if (queue_node->hw_index < l1_node->first_child) {
                            l1_node->first_child = queue_node->hw_index;
                        }
                        if (l1_node->num_child == 1) {
                            _bcm_kt_node_index_set(list, l1_node->base_index, l1_node->numq);
                        }    
                    } else {
                        _bcm_kt_node_index_set(list, queue_node->hw_index, 1);
                    }
                }
                break;

            default:
               break;
        }
    }

    /* Update PORT_COS_MAP memory profile reference counter */
    profile_mem = _bcm_kt_cos_map_profile[unit];
    PBMP_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, COS_MAP_SELm, MEM_BLOCK_ALL, port, &entry));
        set_index = soc_mem_field32_get(unit, COS_MAP_SELm, &entry, SELECTf);
        SOC_IF_ERROR_RETURN
            (soc_profile_mem_reference(unit, profile_mem, set_index * 16, 16));
    }
    if (SOC_INFO(unit).cpu_hg_index != -1) {
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, COS_MAP_SELm, MEM_BLOCK_ALL,
                                         SOC_INFO(unit).cpu_hg_index, &entry));
        set_index = soc_mem_field32_get(unit, COS_MAP_SELm, &entry, SELECTf);
        SOC_IF_ERROR_RETURN
            (soc_profile_mem_reference(unit, profile_mem, set_index, 1));
    }

    /* Update MMU_WRED_DROP_CURVE_PROFILE_x memory profile reference counter */
    profile_mem = _bcm_kt_wred_profile[unit];
    PBMP_PORT_ITER(unit, port) {
        mem = MMU_WRED_QUEUE_CONFIG_BUFFERm;
        
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, port, -1,
                                        _BCM_KT_COSQ_INDEX_STYLE_WRED, NULL,
                                        NULL, &numq));
        for (cosq = 0; cosq < numq; cosq++) {
            BCM_IF_ERROR_RETURN
                (_bcm_kt_cosq_index_resolve(unit, port, cosq,
                                            _BCM_KT_COSQ_INDEX_STYLE_WRED,
                                            NULL, &index, NULL));
            SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, mem, MEM_BLOCK_ALL, index, &qentry));
            set_index = soc_mem_field32_get(unit, mem, &qentry, PROFILE_INDEXf);
            SOC_IF_ERROR_RETURN
                (soc_profile_mem_reference(unit, profile_mem, set_index, 1));
        }
    }

    return BCM_E_NONE;
}

#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *     bcm_kt_cosq_init
 * Purpose:
 *     Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *     unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_init(int unit)
{
    soc_info_t *si;
    STATIC int _kt_max_cosq = -1;
    int cosq, numq, alloc_size;
    bcm_port_t port;
    soc_reg_t mem;
    STATIC soc_mem_t wred_mems[6] = {
        MMU_WRED_DROP_CURVE_PROFILE_0m, MMU_WRED_DROP_CURVE_PROFILE_1m,
        MMU_WRED_DROP_CURVE_PROFILE_2m, MMU_WRED_DROP_CURVE_PROFILE_3m,
        MMU_WRED_DROP_CURVE_PROFILE_4m, MMU_WRED_DROP_CURVE_PROFILE_5m
    };
    int entry_words[6];
    mmu_wred_drop_curve_profile_0_entry_t entry_tcp_green;
    mmu_wred_drop_curve_profile_1_entry_t entry_tcp_yellow;
    mmu_wred_drop_curve_profile_2_entry_t entry_tcp_red;
    mmu_wred_drop_curve_profile_3_entry_t entry_nontcp_green;
    mmu_wred_drop_curve_profile_4_entry_t entry_nontcp_yellow;
    mmu_wred_drop_curve_profile_5_entry_t entry_nontcp_red;
    void *entries[6];
    uint32 profile_index, rval;
    _bcm_kt_mmu_info_t *mmu_info_p;
    int i, prev_port, cmc;
    bcm_gport_t port_sched, l0_sched, l1_sched, queue;

    if (_kt_max_cosq < 0) {
        _kt_max_cosq = NUM_COS(unit);
        NUM_COS(unit) = 8;
    }

    if (!SOC_WARM_BOOT(unit)) {    /* Cold Boot */
        BCM_IF_ERROR_RETURN (bcm_kt_cosq_detach(unit, 0));
    }

    numq = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_DEFAULT);

    if (numq < 1) {
        numq = 1;
    } else if (numq > 8) {
        numq = 8;
    }

    /* Create profile for PORT_COS_MAP table */
    if (_bcm_kt_cos_map_profile[unit] == NULL) {
        _bcm_kt_cos_map_profile[unit] = sal_alloc(sizeof(soc_profile_mem_t),
                                                  "COS_MAP Profile Mem");
        if (_bcm_kt_cos_map_profile[unit] == NULL) {
            return BCM_E_MEMORY;
        }
        soc_profile_mem_t_init(_bcm_kt_cos_map_profile[unit]);
    }
    mem = PORT_COS_MAPm;
    entry_words[0] = sizeof(port_cos_map_entry_t) / sizeof(uint32);
    BCM_IF_ERROR_RETURN(soc_profile_mem_create(unit, &mem, entry_words, 1,
                                               _bcm_kt_cos_map_profile[unit]));

    /* Create profile for MMU_WRED_DROP_CURVE_PROFILE_x tables */
    if (_bcm_kt_wred_profile[unit] == NULL) {
        _bcm_kt_wred_profile[unit] = sal_alloc(sizeof(soc_profile_mem_t),
                                               "WRED Profile Mem");
        if (_bcm_kt_wred_profile[unit] == NULL) {
            return BCM_E_MEMORY;
        }
        soc_profile_mem_t_init(_bcm_kt_wred_profile[unit]);
    }
    entry_words[0] =
        sizeof(mmu_wred_drop_curve_profile_0_entry_t) / sizeof(uint32);
    entry_words[1] =
        sizeof(mmu_wred_drop_curve_profile_1_entry_t) / sizeof(uint32);
    entry_words[2] =
        sizeof(mmu_wred_drop_curve_profile_2_entry_t) / sizeof(uint32);
    entry_words[3] =
        sizeof(mmu_wred_drop_curve_profile_3_entry_t) / sizeof(uint32);
    entry_words[4] =
        sizeof(mmu_wred_drop_curve_profile_4_entry_t) / sizeof(uint32);
    entry_words[5] =
        sizeof(mmu_wred_drop_curve_profile_5_entry_t) / sizeof(uint32);
    BCM_IF_ERROR_RETURN(soc_profile_mem_create(unit, wred_mems, entry_words, 6,
                                               _bcm_kt_wred_profile[unit]));

    alloc_size = sizeof(_bcm_kt_mmu_info_t) * 1;
    if (_bcm_kt_mmu_info[unit] == NULL) {
        _bcm_kt_mmu_info[unit] =
            sal_alloc(alloc_size, "_bcm_kt_mmu_info");

        if (_bcm_kt_mmu_info[unit] == NULL) {
            return BCM_E_MEMORY;
        }
    } else {
        _bcm_kt_cosq_free_memory(_bcm_kt_mmu_info[unit]);
    }

    sal_memset(_bcm_kt_mmu_info[unit], 0, alloc_size);

    mmu_info_p = _bcm_kt_mmu_info[unit];
    if (soc_feature(unit, soc_feature_ddr3)) {
        mmu_info_p->num_queues = soc_mem_index_count(unit, LLS_L2_PARENTm);
    } else {
        /* restrict to maximum 1K queues */
        mmu_info_p->num_queues = 1024;
    }
    mmu_info_p->num_base_queues = soc_property_get(unit, spn_MMU_MAX_CLASSIC_QUEUES, 792);
    mmu_info_p->num_ext_queues  = mmu_info_p->num_queues - mmu_info_p->num_base_queues;
    mmu_info_p->qset_size       = soc_property_get(unit, spn_MMU_SUBSCRIBER_NUM_COS_LEVEL, 4);

    mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_ROOT] = soc_mem_index_count(unit, LLS_PORT_CONFIGm);
    mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_L0]   = soc_mem_index_count(unit, LLS_L0_CONFIGm);
    mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_L1]   = soc_mem_index_count(unit, LLS_L1_CONFIGm);
    mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_L2]   = mmu_info_p->num_queues;

    mmu_info_p->num_nodes = 0;

    if (!soc_feature(unit, soc_feature_ddr3)) {
        mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_L0] /= 2; /* 512 */
        mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_L1] /= 2; /* 128 */
    }    

    for (i=0; i<_BCM_KT_COSQ_NODE_LEVEL_L2; i++) {
        mmu_info_p->num_nodes += mmu_info_p->max_nodes[i];
    }

    mmu_info_p->port = _bcm_kt_cosq_alloc_clear((_BCM_KT_NUM_PORT_SCHEDULERS * 
                                              sizeof(_bcm_kt_cosq_port_info_t)),
                                              "port_info");
    if (mmu_info_p->port == NULL) {
        _bcm_kt_cosq_free_memory(mmu_info_p);
        return BCM_E_MEMORY;
    }

    mmu_info_p->ext_qlist.bits  = _bcm_kt_cosq_alloc_clear(SHR_BITALLOCSIZE(mmu_info_p->num_ext_queues),
                                              "ext_qlist");
    if (mmu_info_p->ext_qlist.bits == NULL) {
        _bcm_kt_cosq_free_memory(mmu_info_p);
        return BCM_E_MEMORY;
    }

    mmu_info_p->sched_list.bits = _bcm_kt_cosq_alloc_clear(SHR_BITALLOCSIZE(mmu_info_p->num_nodes),
                                              "sched_list");
    if (mmu_info_p->sched_list.bits == NULL) {
        _bcm_kt_cosq_free_memory(mmu_info_p);
        return BCM_E_MEMORY;
    }

    mmu_info_p->l0_sched_list.bits = _bcm_kt_cosq_alloc_clear(SHR_BITALLOCSIZE
                                            (mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_L0]),
                                            "l0_sched_list");
    if (mmu_info_p->l0_sched_list.bits == NULL) {
        _bcm_kt_cosq_free_memory(mmu_info_p);
        return BCM_E_MEMORY;
    }

    mmu_info_p->l1_sched_list.bits = _bcm_kt_cosq_alloc_clear(SHR_BITALLOCSIZE
                                             (mmu_info_p->max_nodes[_BCM_KT_COSQ_NODE_LEVEL_L1]),
                                             "l1_sched_list");
    if (mmu_info_p->l1_sched_list.bits == NULL) {
        _bcm_kt_cosq_free_memory(mmu_info_p);
        return BCM_E_MEMORY;
    }

    mmu_info_p->l2_base_qlist.bits = _bcm_kt_cosq_alloc_clear(SHR_BITALLOCSIZE(mmu_info_p->num_base_queues),
                                             "l2_base_qlist");
    if (mmu_info_p->l2_base_qlist.bits == NULL) {
        _bcm_kt_cosq_free_memory(mmu_info_p);
        return BCM_E_MEMORY;
    }

    mmu_info_p->l2_ext_qlist.bits = _bcm_kt_cosq_alloc_clear(SHR_BITALLOCSIZE(mmu_info_p->num_ext_queues),
                                             "l2_ext_qlist");
    if (mmu_info_p->l2_ext_qlist.bits == NULL) {
        _bcm_kt_cosq_free_memory(mmu_info_p);
        return BCM_E_MEMORY;
    }

    for (i = 0; i < _BCM_KT_NUM_TOTAL_SCHEDULERS; i++) {
        _BCM_KT_COSQ_LIST_NODE_INIT(mmu_info_p->sched_node, i);
    }

    for (i = 0; i < _BCM_KT_NUM_L2_QUEUES; i++) {
        _BCM_KT_COSQ_LIST_NODE_INIT(mmu_info_p->queue_node, i);
    }

    si = &SOC_INFO(unit);
    prev_port = 0;
    PBMP_ALL_ITER(unit, port) {
        if (IS_CPU_PORT(unit, port)) {
            mmu_info_p->port[port].q_offset = 0;
            mmu_info_p->port[port].q_limit = 48;
        } else {
            mmu_info_p->port[port].q_offset = mmu_info_p->port[prev_port].q_limit; 
            if (si->port_num_ext_cosq[port] != 0) {
                mmu_info_p->port[port].q_limit = 
                            mmu_info_p->port[port].q_offset + si->port_num_ext_cosq[port];
            } else {
                if (IS_HG_PORT(unit, port)) {   
                    mmu_info_p->port[port].q_limit = 
                                    mmu_info_p->port[port].q_offset + 24;
                } else {
                    mmu_info_p->port[port].q_limit = 
                                    mmu_info_p->port[port].q_offset + 8;
                }    
            }
        }

        rval = 0;
        soc_reg_field_set(unit, ING_COS_MODEr, &rval, BASE_QUEUE_NUMf,
                          mmu_info_p->port[port].q_offset);
        SOC_IF_ERROR_RETURN(WRITE_ING_COS_MODEr(unit, port, rval));

        SOC_IF_ERROR_RETURN(READ_RQE_PORT_CONFIGr(unit, port, &rval));
        soc_reg_field_set(unit, RQE_PORT_CONFIGr, &rval, BASE_QUEUEf,
                          mmu_info_p->port[port].q_offset);
        SOC_IF_ERROR_RETURN(WRITE_RQE_PORT_CONFIGr(unit, port, rval));

        prev_port = port;
    }        

    if (!SOC_WARM_BOOT(unit)) {    /* Cold Boot */
        
    PBMP_ALL_ITER(unit, port) {
        if (IS_CPU_PORT(unit, port)) {
            BCM_IF_ERROR_RETURN
                (bcm_kt_cosq_gport_add(unit, port, SOC_CMCS_NUM(unit), 
                               0, &port_sched));
            for (cmc = 0; cmc < SOC_CMCS_NUM(unit); cmc++) {
                if (NUM_CPU_ARM_COSQ(unit, cmc) == 0) {
                    continue;
                } 
                cosq = (NUM_CPU_ARM_COSQ(unit, cmc) + 7) / 8; 
                BCM_IF_ERROR_RETURN
                    (bcm_kt_cosq_gport_add(unit, port, cosq, 
                                   BCM_COSQ_GPORT_SCHEDULER, &l0_sched));
                BCM_IF_ERROR_RETURN
                    (bcm_kt_cosq_gport_attach(unit, l0_sched, port_sched, cmc));  
                cosq = 0;
                for (i = 0; i < NUM_CPU_ARM_COSQ(unit, cmc); i++) {
                    if (i % 8 == 0) {
                        BCM_IF_ERROR_RETURN
                            (bcm_kt_cosq_gport_add(unit, port, 8, 
                                           BCM_COSQ_GPORT_SCHEDULER, &l1_sched));
                        BCM_IF_ERROR_RETURN
                             (bcm_kt_cosq_gport_attach(unit, l1_sched, l0_sched, cosq));  
                        cosq++;
                    } 
                    BCM_IF_ERROR_RETURN
                        (bcm_kt_cosq_gport_add(unit, port, 1,
                                       BCM_COSQ_GPORT_UCAST_QUEUE_GROUP, &queue));
                    BCM_IF_ERROR_RETURN
                        (bcm_kt_cosq_gport_attach(unit, queue, l1_sched, (i % 8)));  

                }
            } 
        } else {
            BCM_IF_ERROR_RETURN
                (bcm_kt_cosq_gport_add(unit, port, 1, 0, &port_sched));
            BCM_IF_ERROR_RETURN
                (bcm_kt_cosq_gport_add(unit, port, 1, BCM_COSQ_GPORT_SCHEDULER, 
                               &l0_sched));
            BCM_IF_ERROR_RETURN
                (bcm_kt_cosq_gport_attach(unit, l0_sched, port_sched, 0));  
            BCM_IF_ERROR_RETURN
                (bcm_kt_cosq_gport_add(unit, port, numq, BCM_COSQ_GPORT_SCHEDULER, 
                               &l1_sched));
            BCM_IF_ERROR_RETURN
                (bcm_kt_cosq_gport_attach(unit, l1_sched, l0_sched, 0));  
            for (cosq = 0; cosq < numq; cosq++) {
                BCM_IF_ERROR_RETURN
                    (bcm_kt_cosq_gport_add(unit, port, 1,
                                   BCM_COSQ_GPORT_UCAST_QUEUE_GROUP, &queue));
                BCM_IF_ERROR_RETURN
                     (bcm_kt_cosq_gport_attach(unit, queue, l1_sched, cosq));  

            }
        }
    }
    
   }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        return bcm_kt_cosq_reinit(unit);
    } else {
        BCM_IF_ERROR_RETURN(_bcm_kt_cosq_wb_alloc(unit));
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    /* Add default entries for PORT_COS_MAP memory profile */
    BCM_IF_ERROR_RETURN(bcm_kt_cosq_config_set(unit, numq));
    
    sal_memset(&entry_tcp_green, 0, sizeof(entry_tcp_green));
    sal_memset(&entry_tcp_yellow, 0, sizeof(entry_tcp_yellow));
    sal_memset(&entry_tcp_red, 0, sizeof(entry_tcp_red));
    sal_memset(&entry_nontcp_green, 0, sizeof(entry_nontcp_green));
    sal_memset(&entry_nontcp_yellow, 0, sizeof(entry_nontcp_yellow));
    sal_memset(&entry_nontcp_red, 0, sizeof(entry_nontcp_red));
    entries[0] = &entry_tcp_green;
    entries[1] = &entry_tcp_yellow;
    entries[2] = &entry_tcp_red;
    entries[3] = &entry_nontcp_green;
    entries[4] = &entry_nontcp_yellow;
    entries[5] = &entry_nontcp_red;
    profile_index = 0xffffffff;
    PBMP_PORT_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_index_resolve(unit, port, -1,
                                        _BCM_KT_COSQ_INDEX_STYLE_WRED, NULL,
                                        NULL, &numq));
        for (cosq = 0; cosq < numq; cosq++) {
            if (profile_index == 0xffffffff) {
                BCM_IF_ERROR_RETURN
                    (soc_profile_mem_add(unit, _bcm_kt_wred_profile[unit],
                                         entries, 1, &profile_index));
            } else {
                BCM_IF_ERROR_RETURN
                    (soc_profile_mem_reference(unit,
                                               _bcm_kt_wred_profile[unit],
                                               profile_index, 0));
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_detach
 * Purpose:
 *     Discard all COS schedule/mapping state.
 * Parameters:
 *     unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_detach(int unit, int software_state_only)
{
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_config_set
 * Purpose:
 *     Set the number of COS queues
 * Parameters:
 *     unit - unit number
 *     numq - number of COS queues (1-8).
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_config_set(int unit, int numq)
{
    port_cos_map_entry_t cos_map_entries[16];
    void *entries[1], *hg_entries[1];
    uint32 index, hg_index;
    int count, hg_count;
    bcm_port_t port;
    int cos, prio;
    uint32 i;
#ifdef BCM_COSQ_HIGIG_MAP_DISABLE
    port_cos_map_entry_t hg_cos_map_entries[16];
    uint32 rval;
#endif

    if (numq < 1 || numq > 8) {
        return BCM_E_PARAM;
    }

    /* Distribute first 8 internal priority levels into the specified number
     * of cosq, map remaining internal priority levels to highest priority
     * cosq */
    sal_memset(cos_map_entries, 0, sizeof(cos_map_entries));
    entries[0] = &cos_map_entries;
    hg_entries[0] = &cos_map_entries;
    prio = 0;
    for (cos = 0; cos < numq; cos++) {
        for (i = 8 / numq + (cos < 8 % numq ? 1 : 0); i > 0; i--) {
            soc_mem_field32_set(unit, PORT_COS_MAPm, &cos_map_entries[prio],
                                COSf, cos);
            prio++;
        }
    }
    for (prio = 8; prio < 16; prio++) {
        soc_mem_field32_set(unit, PORT_COS_MAPm, &cos_map_entries[prio], COSf,
                            numq - 1);
    }

#ifdef BCM_COSQ_HIGIG_MAP_DISABLE
    /* Use identical mapping for Higig port */
    sal_memset(hg_cos_map_entries, 0, sizeof(hg_cos_map_entries));
    hg_entries[0] = &hg_cos_map_entries;
    prio = 0;
    for (prio = 0; prio < 8; prio++) {
        soc_mem_field32_set(unit, PORT_COS_MAPm, &hg_cos_map_entries[prio],
                            COSf, prio);
    }
    for (prio = 8; prio < 16; prio++) {
        soc_mem_field32_set(unit, PORT_COS_MAPm, &hg_cos_map_entries[prio],
                            COSf, 7);
    }

#endif /* BCM_COSQ_HIGIG_MAP_DISABLE */

    BCM_IF_ERROR_RETURN
        (soc_profile_mem_add(unit, _bcm_kt_cos_map_profile[unit], entries, 16,
                             &index));
    BCM_IF_ERROR_RETURN
        (soc_profile_mem_add(unit, _bcm_kt_cos_map_profile[unit], hg_entries,
                             16, &hg_index));
    count = 0;
    hg_count = 0;
    PBMP_ALL_ITER(unit, port) {
        if (IS_HG_PORT(unit, port)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_field32_modify(unit, COS_MAP_SELm, port, SELECTf,
                                        hg_index / 16));
            hg_count++;
        } else {
            BCM_IF_ERROR_RETURN
                (soc_mem_field32_modify(unit, COS_MAP_SELm, port, SELECTf,
                                        index / 16));
            count++;
        }
    }
    if (SOC_INFO(unit).cpu_hg_index != -1) {
        BCM_IF_ERROR_RETURN
            (soc_mem_field32_modify(unit, COS_MAP_SELm,
                                    SOC_INFO(unit).cpu_hg_index, SELECTf,
                                    hg_index / 16));
        hg_count++;
    }


    for (i = 1; i < count; i++) {
        soc_profile_mem_reference(unit, _bcm_kt_cos_map_profile[unit], index,
                                  0);
    }
    for (i = 1; i < hg_count; i++) {
        soc_profile_mem_reference(unit, _bcm_kt_cos_map_profile[unit],
                                  hg_index, 0);
    }

    _bcm_kt_num_cosq[unit] = numq;

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_kt_cosq_config_get
 * Purpose:
 *     Get the number of cos queues
 * Parameters:
 *     unit - unit number
 *     numq - (Output) number of cosq
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_config_get(int unit, int *numq)
{
    if (_bcm_kt_num_cosq[unit] == 0) {
        return BCM_E_INIT;
    }

    if (numq != NULL) {
        *numq = _bcm_kt_num_cosq[unit];
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_attach
 * Purpose:
 *     Attach sched_port to the specified index (cosq) of input_port
 * Parameters:
 *     unit       - (IN) unit number
 *     sched_port - (IN) scheduler GPORT identifier
 *     input_port - (IN) GPORT to attach to
 *     cosq       - (IN) COS queue to attach to (-1 for first available cosq)
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_attach(int unit, bcm_gport_t sched_gport,
                         bcm_gport_t input_gport, bcm_cos_queue_t cosq)
{
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *sched_node, *input_node;
    bcm_port_t sched_port, input_port, local_port;
    int rv;

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(input_gport)) {
        return BCM_E_PORT;
    }

    if(!(BCM_GPORT_IS_UCAST_QUEUE_GROUP(sched_gport) ||
         BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(sched_gport) ||
         BCM_GPORT_IS_SCHEDULER(sched_gport))) {
        return BCM_E_PORT;
     }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, sched_gport, NULL, &sched_port, NULL,
                               &sched_node));

    if (sched_node->cosq_attached_to >= 0) {
        /* Has already attached */
        return BCM_E_EXISTS;
    }

    if (BCM_GPORT_IS_SCHEDULER(input_gport) ||
        BCM_GPORT_IS_LOCAL(input_gport) ||
        BCM_GPORT_IS_MODPORT(input_gport)) {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_node_get(unit, input_gport, NULL, &input_port, NULL,
                                   &input_node));
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_kt_cosq_localport_resolve(unit, input_gport, &input_port));
        input_node = NULL;
    }

    if (sched_port != input_port) {
        return BCM_E_PORT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];
      /* Identify the levels of schedulers
       * input_port == phy_port && sched_gport == scheduler => sched_gport = L0
       * input_port == schduler && sched_gport == scheduler => input_port = L0 and sched_port = L1
       * input_port == scheduler && sched_port == queue_group => input_port = L1 and sched_port = L2
       */
    if (input_node != NULL) {
        if (!BCM_GPORT_IS_SCHEDULER(input_gport)) {
            if (input_node->numq == 0) {
                local_port = (BCM_GPORT_IS_LOCAL(input_gport)) ?
                              BCM_GPORT_LOCAL_GET(input_gport) :
                              BCM_GPORT_MODPORT_PORT_GET(input_gport);
                input_node->gport = input_gport;
                input_node->hw_index = local_port;
                input_node->level = _BCM_KT_COSQ_NODE_LEVEL_ROOT;
                input_node->cosq_attached_to = 0;
                input_node->numq =  8; /* todo decide based on hg or vlan */
                input_node->base_index = -1;
            }

            if (!BCM_GPORT_IS_SCHEDULER(sched_gport)) {
                return BCM_E_PARAM;
            }

            sched_node->level = _BCM_KT_COSQ_NODE_LEVEL_L0;
        } else {
             if (input_node->level == _BCM_KT_COSQ_NODE_LEVEL_ROOT) {
                 sched_node->level = _BCM_KT_COSQ_NODE_LEVEL_L0;
             }

             if ((input_node->level == -1)) {
            	input_node->level = (BCM_GPORT_IS_SCHEDULER(sched_gport)) ?
                                     _BCM_KT_COSQ_NODE_LEVEL_L0 : _BCM_KT_COSQ_NODE_LEVEL_L1;
             }

             if (sched_node->level == -1) {
                 sched_node->level = (BCM_GPORT_IS_UCAST_QUEUE_GROUP(sched_gport) ||
                                      BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(sched_gport)) ?
                                      _BCM_KT_COSQ_NODE_LEVEL_L2 : _BCM_KT_COSQ_NODE_LEVEL_L1;
             }
        }

        if (input_node->cosq_attached_to < 0) {
            /* Only allow to attach to a node that has already attached */
            return BCM_E_PARAM;
        }
        if (cosq < -1 || cosq >= input_node->numq) {
            return BCM_E_PARAM;
        }
    }

    if (BCM_GPORT_IS_SCHEDULER(input_gport) || BCM_GPORT_IS_LOCAL(input_gport) ||
        BCM_GPORT_IS_MODPORT(input_gport)) {
        if (input_node->cosq_attached_to < 0) {
            /* dont allow to attach to a node that has already attached */
            return BCM_E_PARAM;
        }

        sched_node->parent = input_node;
        sched_node->sibling = input_node->child;
        input_node->child = sched_node;
        /* resolve the nodes */
        rv = _bcm_kt_cosq_node_resolve(unit, sched_node, cosq);
        if (BCM_FAILURE(rv)) {
            input_node->child = sched_node->sibling;
            return rv;
        }
        BCM_IF_ERROR_RETURN(_bcm_kt_cosq_sched_node_set(unit, sched_gport));

        BCM_DEBUG(BCM_DBG_COSQ,
                  ("                         hw_cosq=%d\n",
                   sched_node->cosq_attached_to));
    } else {
            return BCM_E_PORT;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_detach
 * Purpose:
 *     Detach sched_port to the specified index (cosq) of input_port
 * Parameters:
 *     unit       - (IN) unit number
 *     sched_port - (IN) scheduler GPORT identifier
 *     input_port - (IN) GPORT to attach to
 *     cosq       - (IN) COS queue to attach to (-1 for first available cosq)
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_kt_cosq_gport_detach(int unit, bcm_gport_t sched_gport,
                         bcm_gport_t input_gport, bcm_cos_queue_t cosq)
{
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *sched_node, *input_node, *prev_node;
    bcm_port_t sched_port, input_port;
    int rv;

    if (_bcm_kt_mmu_info[unit] == NULL) {
        return BCM_E_INIT;
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(input_gport)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, sched_gport, NULL, &sched_port, NULL,
                               &sched_node));

    if (sched_node->cosq_attached_to < 0) {
        /* Not attached yet */
        return BCM_E_PORT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];

    if (input_gport != BCM_GPORT_INVALID) {

        if (BCM_GPORT_IS_SCHEDULER(input_gport) ||
            BCM_GPORT_IS_LOCAL(input_gport) ||
            BCM_GPORT_IS_MODPORT(input_gport)) {
               BCM_IF_ERROR_RETURN
                (_bcm_kt_cosq_node_get(unit, input_gport, NULL, &input_port, NULL,
                                       &input_node));

        }
        else {

            if (!(BCM_GPORT_IS_SCHEDULER(sched_gport) || BCM_GPORT_IS_UCAST_QUEUE_GROUP(sched_gport))) {
                return BCM_E_PARAM;
            }
            else {
                BCM_IF_ERROR_RETURN
                (_bcm_kt_cosq_localport_resolve(unit, input_gport,
                                                &input_port));
                input_node = NULL;
            }
        }
    }

    if (sched_port != input_port) {
        return BCM_E_PORT;
    }

    if (sched_node->parent != input_node) {
        return BCM_E_PORT;
    }

    if (cosq < -1 || cosq >= input_node->numq) {
        return BCM_E_PARAM;
    }

     if (cosq != -1) {
        if (sched_node->cosq_attached_to != cosq) {
            return BCM_E_PARAM;
        }
    }

    /* unresolve the node - delete this node from parent's child list */
    rv = _bcm_kt_cosq_node_unresolve(unit, sched_node, cosq);
    /* update the hw accordingly */
    BCM_IF_ERROR_RETURN(_bcm_kt_cosq_sched_node_set(unit, sched_gport));
    /* now remove from the sw tree */
    if (sched_node->parent != NULL) {
        if (sched_node->parent->child == sched_node) {
            sched_node->parent->child = sched_node->sibling;
        } else {
            prev_node = sched_node->parent->child;
            while (prev_node != NULL && prev_node->sibling != sched_node) {
                prev_node = prev_node->sibling;
            }
            if (prev_node == NULL) {
                return BCM_E_INTERNAL;
            }
            prev_node->sibling = sched_node->sibling;
       }
        sched_node->parent = NULL;
        sched_node->sibling = NULL;
     }

      BCM_DEBUG(BCM_DBG_COSQ,
             ("                         hw_cosq=%d\n",
             sched_node->cosq_attached_to));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_gport_attach_get
 * Purpose:
 *     Get attached status of a scheduler port
 * Parameters:
 *     unit       - (IN) unit number
 *     sched_port - (IN) scheduler GPORT identifier
 *     input_port - (OUT) GPORT to attach to
 *     cosq       - (OUT) COS queue to attached to
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcm_kt_cosq_gport_attach_get(int unit, bcm_gport_t sched_gport,
                             bcm_gport_t *input_gport, bcm_cos_queue_t *cosq)
{
    _bcm_kt_cosq_node_t *sched_node;
    bcm_module_t modid, modid_out;
    bcm_port_t port, port_out;

    if (input_gport == NULL || cosq == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_kt_cosq_node_get(unit, sched_gport, &modid, &port, NULL,
                               &sched_node));

    if (sched_node->cosq_attached_to < 0) {
        /* Not attached yet */
        return BCM_E_NOT_FOUND;
    }

    if (sched_node->parent != NULL) { 
        *input_gport = sched_node->parent->gport;
    } else {  /* sched_node is in L2 level */
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET, modid, port,
                                    &modid_out, &port_out));
        BCM_GPORT_MODPORT_SET(*input_gport, modid_out, port_out);
    }
    *cosq = sched_node->cosq_attached_to;

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP

STATIC int
_bcm_kt_cosq_port_info_dump(int unit, bcm_port_t port) 
{
    _bcm_kt_mmu_info_t *mmu_info;
    _bcm_kt_cosq_node_t *port_node = NULL;
    _bcm_kt_cosq_node_t *queue_node = NULL;
    _bcm_kt_cosq_node_t *l0_node = NULL;
    _bcm_kt_cosq_node_t *l1_node = NULL;

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    mmu_info = _bcm_kt_mmu_info[unit];

    /* get the root node */
    port_node = &mmu_info->sched_node[port];

    if (port_node && port_node->cosq_attached_to < 0) {
        return BCM_E_NONE;
    }    

    soc_cm_print("=== port %d\n", port);
    soc_cm_print("base = %d numq = %d hw_index = %d level = %d cosq_attach = %d \n",
                 port_node->base_index, port_node->numq, port_node->hw_index,
                 port_node->level, port_node->cosq_attached_to);
    soc_cm_print("num_child = %d first_child = %d \n", port_node->num_child,
                 port_node->first_child);
    if (port_node->parent != NULL) {
        soc_cm_print("  parent gport = 0x%08x\n", port_node->parent->gport);
    }
    if (port_node->sibling != NULL) {
        soc_cm_print("  sibling gport = 0x%08x\n", port_node->sibling->gport);
    }
    if (port_node->child != NULL) {
        soc_cm_print("  child  gport = 0x%08x\n", port_node->child->gport);
    }

    for (l0_node = port_node->child; l0_node != NULL;
         l0_node = l0_node->sibling) {

         if (l0_node->cosq_attached_to < 0) {
            return BCM_E_NONE;
         }    
         soc_cm_print("=== L0 gport 0x%08x\n", l0_node->gport);
         soc_cm_print("base = %d numq = %d hw_index = %d level = %d cosq_attach = %d \n",
                      l0_node->base_index, l0_node->numq, l0_node->hw_index,
                      l0_node->level, l0_node->cosq_attached_to);
         soc_cm_print("num_child = %d first_child = %d \n", l0_node->num_child,
                      l0_node->first_child);
         if (l0_node->parent != NULL) {
             soc_cm_print("  parent gport = 0x%08x\n", l0_node->parent->gport);
         }
         if (l0_node->sibling != NULL) {
             soc_cm_print("  sibling gport = 0x%08x\n", l0_node->sibling->gport);
         }
         if (l0_node->child != NULL) {
             soc_cm_print("  child    gport = 0x%08x\n", l0_node->child->gport);
         }

         for (l1_node = l0_node->child; l1_node != NULL;
              l1_node = l1_node->sibling) {
              if (l1_node->cosq_attached_to < 0) {
                  return BCM_E_NONE;
              }
              soc_cm_print("=== L1 gport 0x%08x\n", l1_node->gport);
              soc_cm_print("base = %d numq = %d hw_index = %d level = %d cosq_attach = %d \n",
                            l1_node->base_index, l1_node->numq, l1_node->hw_index,
                            l1_node->level, l1_node->cosq_attached_to);
              soc_cm_print("num_child = %d first_child = %d \n", l1_node->num_child,
                                    l1_node->first_child);
              if (l1_node->parent != NULL) {
                  soc_cm_print("  parent gport = 0x%08x\n", l1_node->parent->gport);
              }
              if (l1_node->sibling != NULL) {
                  soc_cm_print("  sibling gport = 0x%08x\n", l1_node->sibling->gport);
              }
              if (l1_node->child != NULL) {
                  soc_cm_print("  child  gport = 0x%08x\n", l1_node->child->gport);
              }

              for (queue_node = l1_node->child; queue_node != NULL;
                   queue_node = queue_node->sibling) {
                   if (queue_node->cosq_attached_to < 0) {
                       return BCM_E_NONE;
                   }
                   soc_cm_print("=== L2 gport 0x%08x\n", queue_node->gport);
                   soc_cm_print("base = %d numq = %d hw_index = %d level = %d cosq_attach = %d \n",
                                 queue_node->base_index, queue_node->numq, queue_node->hw_index,
                                 queue_node->level, queue_node->cosq_attached_to);
                   soc_cm_print("num_child = %d first_child = %d \n", queue_node->num_child,
                                 queue_node->first_child);
                   if (queue_node->parent != NULL) {
                       soc_cm_print("  parent gport = 0x%08x\n", queue_node->parent->gport);
                   }
                   if (queue_node->sibling != NULL) {
                       soc_cm_print("  sibling gport = 0x%08x\n", queue_node->sibling->gport);
                   }
                   if (queue_node->child != NULL) {
                       soc_cm_print("  child  gport = 0x%08x\n", queue_node->child->gport);
                   }
              }        

         }        
    }        

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_kt_cosq_sw_dump
 * Purpose:
 *     Displays COS Queue information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
bcm_kt_cosq_sw_dump(int unit)
{
    _bcm_kt_mmu_info_t *mmu_info;
    bcm_port_t port;

    mmu_info = _bcm_kt_mmu_info[unit];

    soc_cm_print("\nSW Information COSQ - Unit %d\n", unit);

    PBMP_ALL_ITER(unit, port) {
        (void)_bcm_kt_cosq_port_info_dump(unit, port);
    }

    soc_cm_print("ext_qlist = %d \n", mmu_info->ext_qlist.count);
    soc_cm_print("sched_list = %d \n", mmu_info->sched_list.count);
    soc_cm_print("l0_sched_list = %d \n", mmu_info->l0_sched_list.count);
    soc_cm_print("l1_sched_list = %d \n", mmu_info->l1_sched_list.count);
    soc_cm_print("l2_base_qlist = %d \n", mmu_info->l2_base_qlist.count);
    soc_cm_print("l2_ext_qlist = %d \n", mmu_info->l2_ext_qlist.count);
    
    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#else /* BCM_KATANA_SUPPORT */
int _kt_cosq_not_empty;
#endif  /* BCM_KATANA_SUPPORT */
