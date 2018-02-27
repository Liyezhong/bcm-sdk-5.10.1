/*
 * $Id: meter.c 1.2 Broadcom SDK $
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
 * File: 	meter.c
 * Purpose: 	API for metering, ffppacketcounters and ffpcounters.
 *
 * BCM5690A0 errata (GNATS 2997): when using BCM5690 metering to
 * rate-limit a 10Mb stream, for refresh rate values 6 or less the
 * amount of metered data will come out 10% less than expected.
 * This is fixed in BCM5690A1.
 */

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/meter.h>

int
bcm_sbx_meter_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_meter_create(int unit,
                     bcm_port_t port,
                     int *mid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_meter_delete(int unit,
                     bcm_port_t port,
                     int mid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_meter_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_meter_set(int unit,
                  bcm_port_t port,
                  int mid, 
                  uint32 kbits_sec,
                  uint32 kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_meter_get(int unit,
                  bcm_port_t port,
                  int mid, 
                  uint32 *kbits_sec,
                  uint32 *kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffppacketcounter_set(int unit,
                             bcm_port_t port,
                             int mid,
                             uint64 val)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffppacketcounter_get(int unit,
                             bcm_port_t port,
                             int mid,
                             uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffpcounter_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffpcounter_create(int unit,
                          bcm_port_t port,
                          int *ffpcounterid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffpcounter_delete(int unit,
                          bcm_port_t port,
                          int ffpcounterid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffpcounter_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffpcounter_set(int unit,
                       bcm_port_t port,
                       int ffpcounterid,
                       uint64 val)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_ffpcounter_get(int unit,
                       bcm_port_t port,
                       int ffpcounterid,
                       uint64 *val)
{
    return BCM_E_UNAVAIL;
}
