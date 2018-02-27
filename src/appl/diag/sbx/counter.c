/*
 * $Id: counter.c 1.17 Broadcom SDK $
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
 * SBX Counter CLI support
 */
#include <appl/diag/system.h>

#include <bcm/stat.h>
#include <bcm/error.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/counter.h>

#if defined(BCM_SIRIUS_SUPPORT)
/*
 * Data structure for saving the previous value of counters so we can
 * tell which counters that have changed since last shown.
 */

STATIC uint64 *sbx_counter_val[SOC_MAX_NUM_DEVICES];

STATIC void
sbx_counter_val_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                int ar_idx, uint64 val)
{
    int			ind;
    soc_counter_non_dma_t *non_dma;

    if (sbx_counter_val[unit] == NULL) {
	int		n;

	n = SOC_CONTROL(unit)->counter_n32 + SOC_CONTROL(unit)->counter_n64 +
            SOC_CONTROL(unit)->counter_n64_non_dma;
	sbx_counter_val[unit] = sal_alloc(n * sizeof (uint64), "save_ctrs");

	if (sbx_counter_val[unit] == NULL) {
	    return;
	}

	sal_memset(sbx_counter_val[unit], 0, n * sizeof (uint64));
    }

    ind = soc_counter_idx_get(unit, ctr_reg, ar_idx, port);

    if (ind >= 0) {

	if (ctr_reg >= SOC_COUNTER_NON_DMA_END) {
	    debugk(DK_COUNTER + DK_VERBOSE,
		   "cval_set: Illegal counter index -- "
		   "ar_idx=%d p=%d idx=%d vh=%d vl=%d\n",
		   ar_idx, port, ind,
		   COMPILER_64_HI(val),
		   COMPILER_64_LO(val));
	    return;
    
	} else if (ctr_reg >= SOC_COUNTER_NON_DMA_START &&
		   ctr_reg < SOC_COUNTER_NON_DMA_END) {
	    non_dma =
		&SOC_CONTROL(unit)->counter_non_dma[ctr_reg -
						    SOC_COUNTER_NON_DMA_START];
	    debugk(DK_COUNTER + DK_VERBOSE,
		   "cval_set: %s ar_idx=%d p=%d idx=%d vh=%d vl=%d\n",
		   non_dma->cname,
		   ar_idx, port, ind,
		   COMPILER_64_HI(val),
		   COMPILER_64_LO(val));
	} else {
	    debugk(DK_COUNTER + DK_VERBOSE,
		   "cval_set: %s ar_idx=%d p=%d idx=%d vh=%d vl=%d\n",
		   SOC_REG_NAME(unit, ctr_reg),
		   ar_idx, port, ind,
		   COMPILER_64_HI(val),
		   COMPILER_64_LO(val));
	}
	
	sbx_counter_val[unit][ind] = val;
    }
}

void
sbx_counter_val_set_by_port(int unit, pbmp_t pbmp, uint64 val)
{
    int			i;
    soc_port_t		port;
    soc_reg_t		ctr_reg;
    soc_cmap_t          *cmap;

    SOC_PBMP_ITER(pbmp, port) {
      cmap = soc_port_cmap_get(unit, port);
      if (cmap == NULL) {
	continue;
      }
      for (i = 0; i < cmap->cmap_size; i++) {
	ctr_reg = cmap->cmap_base[i].reg;
	if (ctr_reg != INVALIDr) {
	  sbx_counter_val_set(unit, port, ctr_reg, -1, val);
	}
      }
    }
}

STATIC void
sbx_counter_val_get(int unit, soc_port_t port, soc_reg_t ctr_reg,
                int ar_idx, uint64 *val)
{
    int			ind;
    soc_counter_non_dma_t *non_dma;

    if (sbx_counter_val[unit] == NULL) {
	COMPILER_64_SET(*val, 0, 0);
    } else {
	ind = soc_counter_idx_get(unit, ctr_reg, ar_idx, port);

	if (ind >= 0) {
	    
	    if (ctr_reg >= SOC_COUNTER_NON_DMA_END) {
		debugk(DK_COUNTER + DK_VERBOSE,
		       "cval_get: Illegal counter index -- "
		       "ar_idx=%d p=%d idx=%d vh=%d vl=%d\n",
		       ar_idx, port, ind,
		       COMPILER_64_HI(*val),
		       COMPILER_64_LO(*val));
		return;
		
	    } else if (ctr_reg >= SOC_COUNTER_NON_DMA_START &&
		       ctr_reg < SOC_COUNTER_NON_DMA_END) {
		non_dma =
		    &SOC_CONTROL(unit)->counter_non_dma[ctr_reg -
							SOC_COUNTER_NON_DMA_START];
		debugk(DK_COUNTER + DK_VERBOSE,
		       "cval_get: %s ar_idx=%d p=%d idx=%d vh=%d vl=%d\n",
		       non_dma->cname,
		       ar_idx, port, ind,
		       COMPILER_64_HI(*val),
		       COMPILER_64_LO(*val));
	    } else {
		debugk(DK_COUNTER + DK_VERBOSE,
		       "cval_get: %s ar_idx=%d p=%d idx=%d vh=%d vl=%d\n",
		       SOC_REG_NAME(unit, ctr_reg),
		       ar_idx, port, ind,
		       COMPILER_64_HI(*val),
		       COMPILER_64_LO(*val));
	    }
	    *val = sbx_counter_val[unit][ind];

	} else {
	    *val = 0;
	}
    }
}

/*
 * Function:
 *	sbx_do_resync_counters
 * Purpose:
 *	Resynchronize the previous counter values (stored in sbx_counter_val)
 *	with the actual counters.
 */

int
sbx_do_resync_counters(int unit, pbmp_t pbmp)
{
    soc_port_t		port;
    int			i;
    soc_cmap_t          *cmap;
    soc_reg_t           reg;
    uint64		val;
    int                 ar_idx;

    SOC_PBMP_ITER(pbmp, port) {
        cmap = soc_port_cmap_get(unit, port);

	for (i = 0; i < cmap->cmap_size; i++) {
	    reg = cmap->cmap_base[i].reg;
	    if (reg != INVALIDr) {
                if (SOC_REG_INFO(unit, reg).flags & SOC_REG_FLAG_ARRAY) {
                    for (ar_idx = 0;
                         ar_idx < SOC_REG_INFO(unit, reg).numels;
                         ar_idx++) {
			if (soc_counter_get(unit, port, reg, ar_idx, &val) == SOC_E_NONE) {
			    sbx_counter_val_set(unit, port, reg, ar_idx, val);
			}
                    }
                } else {
                    if (soc_counter_get(unit, port, reg, 0, &val) == SOC_E_NONE )
                          sbx_counter_val_set(unit, port, reg, 0, val);
                }
	    }
	}
    }

    return 0;
}

/*
 * sbx_do_show_counter
 *
 *   Displays counters for specified register(s) and port(s).
 *
 *	SHOW_CTR_HEX displays values in hex instead of decimal.
 *
 *	SHOW_CTR_RAW displays only the counter value, not the register
 *	name or delta.
 *
 *	SHOW_CTR_CHANGED: includes counters that have changed since the
 *	last call to do_show_counters().
 *
 *	SHOW_CTR_NOT_CHANGED: includes counters that have not changed since
 *	the last call to do_show_counters().
 *
 *   NOTE: to get any output, you need at least one of the above two flags.
 *
 *	SHOW_CTR_ZERO: includes counters that are zero.
 *
 *	SHOW_CTR_NOT_ZERO: includes counters that are not zero.
 *
 *   NOTE: to get any output, you need at least one of the above two flags.
 *
 *   SHOW_CTR_ED:
 *      Show only those registers marked as Error/Discard.
 *
 *   Columns will remain lined up properly until the following maximum
 *   limits are exceeded:
 *
 *	Current count: 99,999,999,999,999,999
 *	Increment: +99,999,999,999,999
 *	Rate: 999,999,999,999/s
 */

void
sbx_do_show_counter(int unit, soc_port_t port, soc_reg_t ctr_reg,
                int ar_idx, int flags)
{
    soc_counter_non_dma_t *non_dma;
    uint64		val, prev_val=0, diff, rate;
    int			changed;
    int                 is_ed_cntr;
    int			tabwidth = soc_property_get(unit, spn_DIAG_TABS, 8);
    int			commachr = soc_property_get(unit, spn_DIAG_COMMA, ',');

    if (ctr_reg >= SOC_COUNTER_NON_DMA_START &&
        ctr_reg < SOC_COUNTER_NON_DMA_END) {
        if (SOC_CONTROL(unit)->counter_non_dma == NULL) {
            return;
        }
        is_ed_cntr = FALSE;
        if (ar_idx < 0) {
            non_dma =
                &SOC_CONTROL(unit)->counter_non_dma[ctr_reg -
                                                    SOC_COUNTER_NON_DMA_START];
            for (ar_idx = 0; ar_idx < non_dma->entries_per_port; ar_idx++) {
                sbx_do_show_counter(unit, port, ctr_reg, ar_idx, flags);
            }
            return;
        }
    } else {
        is_ed_cntr = SOC_REG_INFO(unit, ctr_reg).flags & SOC_REG_FLAG_ED_CNTR;

        if (!(SOC_REG_INFO(unit, ctr_reg).flags & SOC_REG_FLAG_ARRAY)) {
            ar_idx = 0;
        } else {
            if (ar_idx < 0) { /* Set all elts of array */
                for (ar_idx = 0; ar_idx < SOC_REG_INFO(unit, ctr_reg).numels;
                     ar_idx++) {
                    sbx_do_show_counter(unit, port, ctr_reg, ar_idx, flags);
                }
                return;
            }
        }
    }

    if (soc_counter_get(unit, port, ctr_reg, ar_idx, &val) < 0) {
        return;
    }
    sbx_counter_val_get(unit, port, ctr_reg, ar_idx, &prev_val);

    soc_counter_get_rate(unit, port, ctr_reg, ar_idx, &rate);

    diff = val;
    COMPILER_64_SUB_64(diff, prev_val);

    if (COMPILER_64_IS_ZERO(diff)) {
	changed = 0;
    } else {
	sbx_counter_val_set(unit, port, ctr_reg, ar_idx, val);
	changed = 1;
    }

    if ((( changed && (flags & SHOW_CTR_CHANGED)) ||
	 (!changed && (flags & SHOW_CTR_SAME))) &&
	(( COMPILER_64_IS_ZERO(val) && (flags & SHOW_CTR_Z)) ||
	 (!COMPILER_64_IS_ZERO(val) && (flags & SHOW_CTR_NZ))) &&
        ((!(flags & SHOW_CTR_ED) ||
          ((flags & SHOW_CTR_ED) && is_ed_cntr)))
        ) {

    /* size of line must be equal or more then sum of cname, buf_val, 
      buf_diff and buf_rate. */
    /* The size of tabby must be greater than size of line */

	char		tabby[120], line[120];
	char		cname[20];
	char		buf_val[32], buf_diff[32], buf_rate[32];

        if (ctr_reg >= SOC_COUNTER_NON_DMA_START &&
            ctr_reg < SOC_COUNTER_NON_DMA_END) {
            non_dma =
                &SOC_CONTROL(unit)->counter_non_dma[ctr_reg -
                                                    SOC_COUNTER_NON_DMA_START];
            sal_sprintf(cname, "%s(%d).%s", non_dma->cname, ar_idx,
                        SOC_PORT_NAME(unit, port));
        } else if (strlen(SOC_REG_NAME(unit, ctr_reg)) > 10) {
            sal_memcpy(cname, SOC_REG_NAME(unit, ctr_reg), 10);
            sal_sprintf(&cname[10], ".%s", SOC_PORT_NAME(unit, port));
        } else {
            if (SOC_REG_INFO(unit, ctr_reg).flags & SOC_REG_FLAG_ARRAY) {
                sal_sprintf(cname, "%s(%d).%s",
                        SOC_REG_NAME(unit, ctr_reg),
                        ar_idx,
                        SOC_PORT_NAME(unit, port));
            } else {
		sal_sprintf(cname, "%s.%s",
			SOC_REG_NAME(unit, ctr_reg),
			SOC_PORT_NAME(unit, port));
	    }
        }

	if (flags & SHOW_CTR_RAW) {
	    if (flags & SHOW_CTR_HEX) {
		sal_sprintf(line, "0x%08x%08x",
			COMPILER_64_HI(val), COMPILER_64_LO(val));
	    } else {
		format_uint64_decimal(buf_val, val, 0);
		sal_sprintf(line, "%s", buf_val);
	    }
	} else {
	    if (flags & SHOW_CTR_HEX) {
		sal_sprintf(line, "%-15s : 0x%08x%08x +0x%08x%08x 0x%08x%08x/s",
			cname,
			COMPILER_64_HI(val), COMPILER_64_LO(val),
			COMPILER_64_HI(diff), COMPILER_64_LO(diff),
			COMPILER_64_HI(rate), COMPILER_64_LO(rate));
	    } else {
		format_uint64_decimal(buf_val, val, commachr);
		buf_diff[0] = '+';
		format_uint64_decimal(buf_diff + 1, diff, commachr);
		sal_sprintf(line, "%-16s:%22s%20s",
			cname, buf_val, buf_diff);
		if (!COMPILER_64_IS_ZERO(rate)) {
		    format_uint64_decimal(buf_rate, rate, commachr);
		    sal_sprintf(line + strlen(line), "%16s/s", buf_rate);
		}
	    }
	}

	/*
	 * Display is tremendously faster with tabify.
	 * Set diag_tab property to desired value, or 0 to turn it off.
	 */

	tabify_line(tabby, line, tabwidth);

	printk("%s\n", tabby);
    }
}

int
sbx_do_show_counters(int unit, soc_reg_t ctr_reg, pbmp_t pbmp, int flags)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    soc_port_t		port;
    int			i;
    soc_cmap_t          *cmap;
    soc_reg_t           reg;
    int                 numregs;
    char		pfmt[SOC_PBMP_FMT_LEN];

    SOC_PBMP_ITER(pbmp, port) {
        cmap = soc_port_cmap_get(unit, port);
        numregs = cmap->cmap_size;

	if (ctr_reg != INVALIDr) {
	    sbx_do_show_counter(unit, port, ctr_reg, -1, flags);
	} else {
            for (i = 0; i < numregs; i++) {
                reg = cmap->cmap_base[i].reg;
                if (reg != INVALIDr) {
                    sbx_do_show_counter(unit, port, reg,
                                    cmap->cmap_base[i].index, flags);
                }
            }
            for (i = SOC_COUNTER_NON_DMA_START; i < SOC_COUNTER_NON_DMA_END;
                 i++) {
                sbx_do_show_counter(unit, port, i, -1, flags);
            }
        }
    }

    SOC_PBMP_REMOVE(pbmp, soc->counter_pbmp);
    if (soc->counter_interval == 0) {
	printk("NOTE: counter collection is not running\n");
    } else if (SOC_PBMP_NOT_NULL(pbmp)) {
	printk("NOTE: counter collection is not active for ports %s\n",
	       SOC_PBMP_FMT(pbmp, pfmt));
    }

    return 0;
}

char cmd_sbx_counter_usage[] =
    "\nParameters: [off] [sync] [Interval=<usec>]\n\t"
    "\t[PortBitMap=<pbm>] [DMA=<true|false>]\n\t"
    "Starts the counter collection task and/or DMA running every <usec>\n\t"
    "microseconds.  The task tallies software counters based on hardware\n\t"
    "values and must run often enough to avoid counter overflow.\n\t"
    "If <interval> is 0, stops the task.  If <interval> is omitted, prints\n\t"
    "the current INTERVAL.  sync reads in all the counters to synchronize\n\t"
    "'show counters' for the first time, and must be used alone.\n";

/*
 * Counter collection configuration
 */

cmd_result_t
cmd_sbx_counter(int unit, args_t *a)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    pbmp_t		pbmp;
    int			usec, dma, r = 0;
    parse_table_t	pt;
    uint32		flags;
    int			sync = 0;
    pbmp_t              temp_pbmp;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }


    flags = soc->counter_flags;
    usec = soc->counter_interval;
    BCM_PBMP_ASSIGN(pbmp, soc->counter_pbmp);
    dma = (flags & SOC_COUNTER_F_DMA) != 0;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Interval", 	PQ_DFL|PQ_INT,
		    (void *)( 0), &usec, NULL);
    parse_table_add(&pt, "PortBitMap", 	PQ_DFL|PQ_PBMP,
		    (void *)(0), &pbmp, NULL);
    parse_table_add(&pt, "DMA", 	PQ_DFL|PQ_BOOL,
		    INT_TO_PTR(dma), &dma, NULL);

    if (!ARG_CNT(a)) {			/* Display settings */
	printk("Current settings:\n");
	parse_eq_format(&pt);
	parse_arg_eq_done(&pt);
	return(CMD_OK);
    }

    if (parse_arg_eq(a, &pt) < 0) {
	printk("%s: Error: Unknown option: %s\n", ARG_CMD(a), ARG_CUR(a));
	parse_arg_eq_done(&pt);
	return(CMD_FAIL);
    }
    parse_arg_eq_done(&pt);

    if (ARG_CNT(a) > 0 && !sal_strcasecmp(_ARG_CUR(a), "off")) {
	ARG_NEXT(a);
	usec = 0;
    }

    if (ARG_CNT(a) > 0 && !sal_strcasecmp(_ARG_CUR(a), "sync")) {
	ARG_NEXT(a);
	sync = 1;
    }

    if (sync) {
        /* Sync driver info to HW */
	if ((r = soc_sbx_counter_sync(unit)) < 0) {
	    printk("%s: Error: Could not sync counters: %s\n",
		   ARG_CMD(a), soc_errmsg(r));
	    return CMD_FAIL;
	}
        /* Sync diag info with driver */
	sbx_do_resync_counters(unit, pbmp);
	return CMD_OK;
    }

    SOC_PBMP_CLEAR(temp_pbmp);
    SOC_PBMP_OR(temp_pbmp, SOC_PORT_BITMAP(unit, hg));
    SOC_PBMP_OR(temp_pbmp, SOC_PORT_BITMAP(unit, xe));
    SOC_PBMP_OR(temp_pbmp, SOC_PORT_BITMAP(unit, ge));
    if (soc_feature(unit, soc_feature_cpuport_stat_dma)) {
        SOC_PBMP_OR(temp_pbmp, PBMP_CMIC(unit));
        BCM_PBMP_AND(pbmp, temp_pbmp);
    } else {
        BCM_PBMP_AND(pbmp, temp_pbmp);
    }

    if (dma) {
	flags |= SOC_COUNTER_F_DMA;
    } else {
	flags &= ~SOC_COUNTER_F_DMA;
    }

    if (usec > 0) {
      r = soc_sbx_counter_start(unit, flags, usec, pbmp); 
    } else {
      if (soc->counterMutex) {
	COUNTER_LOCK(unit);
	r = soc_sbx_counter_stop(unit);
	COUNTER_UNLOCK(unit);
      }
    }

    if (r < 0) {
	printk("%s: Error: Could not set counter mode: %s\n",
	       ARG_CMD(a), soc_errmsg(r));
	return CMD_FAIL;
    }

    return CMD_OK;
}
#endif /* BCM_SIRIUS_SUPPORT */
