/*
 * $Id: diffserv.h 1.8.6.2 Broadcom SDK $
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
 * 
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */

#ifndef __BCMX_DIFFSERV_H__
#define __BCMX_DIFFSERV_H__

#include <bcm/types.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>
#include <bcm/diffserv.h>

/* Initialize the diffserv subsystem. */
extern int bcmx_ds_init(void);

/* Create a diffserv datapath. */
extern int bcmx_ds_datapath_create(
    int dpid, 
    uint32 flags, 
    bcmx_lplist_t plist);

/* Create a diffserv datapath. */
extern int bcmx_ds_datapath_alloc_create(
    int *dpid, 
    uint32 flags, 
    bcmx_lplist_t plist);

/* Returns the number of units associated with the given datapath. */
extern int bcmx_ds_datapath_unit_count(
    int dpid);

/* Delete a diffserv datapath and all associated classifiers. */
extern int bcmx_ds_datapath_delete(
    int dpid);

/* Install the datapath and all associated classifiers onto the hardware. */
extern int bcmx_ds_datapath_install(
    int dpid);

/* Create a classifier within a datapath. */
extern int bcmx_ds_classifier_create(
    int dpid, 
    int cfid, 
    bcm_ds_clfr_t *clfr, 
    bcm_ds_inprofile_actn_t *inp_actn, 
    bcm_ds_outprofile_actn_t *outp_actn, 
    bcm_ds_nomatch_actn_t *nm_actn);

/* Create a classifier within a datapath. */
extern int bcmx_ds_classifier_alloc_create(
    int dpid, 
    int *cfid, 
    bcm_ds_clfr_t *clfr, 
    bcm_ds_inprofile_actn_t *inp_actn, 
    bcm_ds_outprofile_actn_t *outp_actn, 
    bcm_ds_nomatch_actn_t *nm_actn);

/* Update an existing classifier. */
extern int bcmx_ds_classifier_update(
    int dpid, 
    int cfid, 
    uint32 flags, 
    bcm_ds_inprofile_actn_t *inp_actn, 
    bcm_ds_outprofile_actn_t *outp_actn);

/* Delete a classifier from a datapath. */
extern int bcmx_ds_classifier_delete(
    int dpid, 
    int cfid);

/* Get a classifiers from a datapath. */
extern int bcmx_ds_classifier_get(
    int dpid, 
    int cfid, 
    bcm_ds_clfr_t *clfr, 
    bcm_ds_inprofile_actn_t *inp_actn, 
    bcm_ds_outprofile_actn_t *outp_actn, 
    bcm_ds_nomatch_actn_t *nm_actn);

/* Add a scheduler to a diffserv datapath. */
extern int bcmx_ds_scheduler_add(
    int dpid, 
    bcm_ds_scheduler_t *scheduler);

/* Get packet counters for a classifier. */
extern int bcmx_ds_counter_get(
    int dpid, 
    int cfid, 
    bcm_ds_counters_t *counter);

#if defined(BROADCOM_DEBUG)
extern int bcmx_ds_dump(void);
#endif

#endif /* __BCMX_DIFFSERV_H__ */
