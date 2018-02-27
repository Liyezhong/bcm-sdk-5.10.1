#ifndef _SB_EML_COMP_H_
#define _SB_EML_COMP_H_

/******************************************************************************
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
 * sbEMLComp.h: Exact Match Legacy compiler, internal definitions.
 *
 * $Id: sbEMLComp.h 1.6.110.1 Broadcom SDK $
 *
 *****************************************************************************/

#include <soc/sbx/fe2k_common/sbEML.h>
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include <soc/sbx/fe2k_common/sbPayloadMgr.h>
#include <sal/core/sync.h>

/*
 * Note - sbEMLTable2_t definition in public API header sbEML.h
 *        as it needs to be exposed to clients. What follows here is
 *        just a comment.
 *
 * Base struct for a table 2 entry, must be first in a table 2 entry.
 * It can be followed by bit fields as long as these together with
 * the bit fields below are all contained in one word.
 * Immediately after the base entry there must be a word array for the
 * packed payload.
 * E.g.
 * struct sbEMLTable2_s {
 *   SB_EML_TABLE2_BASE;
 *   uint32_t payload[PAYLOAD_SIZE];
 * };
 *
 * *next is the link to the next t2 element
 * valid    -
 * slotNo   - indicates the order of a t2 element with respect to the usemap.
 *            i.e. if it represents the right most bit set in the use map the
 *            slotNo will be 0, for the 3rd rightmost bit set slotNo is 2 etc.
 * usage    -
 * clsDirty -
 *
 */


#define BITSPERUINT32 32
#define MAXSLABSIZE 0x10000



/*
 * The sbEML_t is used by a client of EML to pass various information
 * to the EML.
 * Fields:
 *  fe			the fe the client init got
 *  cParams		the cParams the client init got
 *  ctx			an arbitrary value that will be passed to all
 *			functions called via function pointers in this struct.
 *  table1_count	number of table 1 entries (power of 2)
 *  table2_max		maximum number of table 2 entries
 *  table2_size		size (in bytes) of the packed key and payload
 *  slab_size		slab size (DEF)
 *  key_size		size (in bits) of the key
 *  seed_bits		number of bits in the seed
 *  maxDirty		maximum number of entries in the clsid array
 *
 *  getKey		function that extracts the key from tha packed
 *			key&payload.  The extracted keys in a number of words.
 *
 *  packKey		function that takes an unpacked key and packs it.
 *
 *  unPackKey		function that takes an packed key and unpacks it.
 *
 *  packKeyPayload	function that takes an unpacked key payload and packs
 *			 them.
 *
 *  updatePacked	function that takes an unpacked key, an unpacked
 *			payload, and an already packed key&payload an updates
 *			the latter.
 *			The difference between the packKeyPayload and
 *			updatePacked is that the latter will not touch any
 *			private fields in the packed payload (e.g. classifier
 *			ids).
 *
 *  table2size		function that computes the size (in words) of a level 2
 *			hash table given the number of entries in it.
 *
 *  slabTable2entry	function that makes slabs for writing a single entry in
 *			a level 2 table.
 *
 *  slabGetTable2entry	function that makes slabs for reading a single entry in
 *			a level 2 table.
 *
 *  unslabTable2Entry	function that takes slabs (originally from
 * 			slabGetTable2entry) and turns it into a packed payload.
 *
 *  slabTable2		function that creates slabs for an entire level 2 hash
 *			table, including the bitmap.
 *
 *  slabSetTable1ptr	function that creates a slab for changing a pointer in
 *			the level 1 hash table.
 *
 *  commitDone		callback function run when a commit is done.
 *  updateDone		callback function run when an update is done.
 *  getDone		callback function run when a get is done.
 *
 *  updClsId		function that updates a classifier id in a packed
 *			payload.  Returns 1 if these was a change, 0 if not.
 *
 *  sharedFreeList	if non-null, this points to the struct that this
 *			EML will share its free list with.
 */



typedef struct sbEML_s {
/* Public fields: (filled by caller) */
        sbFe2000DmaMgr_t *pDmaCtxt;
        struct sbPayloadMgr *payloadMgr;
	sbCommonConfigParams_p_t cParams;

	sbEMLCtx_p ctx;

	uint32_t table1_count;

	uint32_t table2_max;
	uint32_t table2_size;

	uint32_t payload_size;

	uint32_t slab_size;

	uint32_t key_size;
	uint32_t seed_bits;

	uint32_t maxDirty;
        uint8_t ipv6;
        uint8_t new_hash;

	/* extract packed key from packed payload */
	void (*getKey)(sbEMLCtx_p ctx,
		       sbEMLPackedKeyPayload_p_t pkp,
		       sbEMLPackedKey_t keyData);
	/* pack key */
	void (*packKey)(sbEMLCtx_p ctx,
			sbEMLKey_p_t key,
			sbEMLPackedKey_t pkey);
	void (*unPackKey)(sbEMLCtx_p ctx,
			sbEMLKey_p_t key,
			sbEMLPackedKey_t pkey);
	/* pack key and payload */
	void (*packKeyPayload)(sbEMLCtx_p ctx,
			       sbEMLKey_p_t key,
			       sbEMLPayload_p_t payload,
			       sbEMLPackedKeyPayload_p_t pkp);

	/* update packed payload */
	void (*updatePacked)(sbEMLCtx_p ctx,
			     sbEMLPackedKeyPayload_p_t pkp,
			     sbEMLKey_p_t key,
			     sbEMLPayload_p_t payload);

	/* returns size in double words */
	uint32_t (*table2size)(sbEMLCtx_p ctx, uint32_t nentries);
	sbFe2000DmaSlab_t* (*slabTable2entry)(sbEMLCtx_p ctx,
                                               sbPHandle_t phdl,
                                               sbEMLTable2_t *entry,
                                               sbFe2000DmaSlab_t* slab);

	sbFe2000DmaSlab_t* (*slabGetTable2entry)(sbEMLCtx_p ctx,
						  sbPHandle_t phdl,
						   sbEMLTable2_t *entry,
						   sbFe2000DmaSlab_t* slab);
	void (*unslabTable2Entry)(sbEMLCtx_p ctx,
				  sbEMLPackedKeyPayload_p_t pkp,
				  struct sbFe2000DmaSlab_s * slab,
				  unsigned int slotNo);

	sbFe2000DmaSlab_t* (*slabTable2)(sbEMLCtx_p ctx,
                                          sbPHandle_t phdl,
                                          uint32_t usemap,
                                          sbEMLTable2_t *entries,
                                          sbFe2000DmaSlab_t* slab);

	sbFe2000DmaSlab_t* (*slabSetTable1ptr)(sbEMLCtx_p ctx,
						 uint32_t ix,
						 uint32_t seed,
						 sbPHandle_t phdl,
                                                 uint32_t usemap,
						 sbFe2000DmaSlab_t* slab);

    uint32 (*table1Addr)(sbEMLCtx_p cstate, int u);
    void (*table1EntryParse) (sbEMLCtx_p cstate, uint32 *s, 
                              uint32 *p, uint32 *m, uint8 *b);


	void (*commitDone)(sbEMLCtx_p ctx, sbStatus_t status);
	void (*updateDone)(sbEMLCtx_p ctx, sbStatus_t status);
	void (*getDone)(sbEMLCtx_p ctx, sbStatus_t status);

	unsigned int (*updClsId)(sbEMLCtx_p ctx, sbEMLPackedKeyPayload_p_t pkp,
			 uint8_t b, unsigned int clsId, uint8_t src);

	/* If this is set, the table 2 free list is taken from this EML */
	struct sbEML_s *sharedFreeList;

        uint32_t nbank;

/* Filled by Init */
	sbEMLTable2_t *table2array;
	sbEMLTable2_p_t table2arrayP;

/* Private fields: */
	struct emcomp *emp;
    sal_mutex_t eml_mutex;
} sbEML_t, *sbEML_p_t;

struct slabTransfer {
	sbEML_p_t slabEm;
	void *slabMemory;
	uint32_t slabSize;
	uint32_t *slabEnd;
	sbFe2000DmaSlabOp_t slabOp;
	uint32_t *slabRunLengthP;
	unsigned int slabState;
};

struct table2 {
  SB_EML_TABLE2_BASE;
	uint32_t packedPayload[1];	/* really bigger */
};

/* struct table2entry in our .h file */
/*	uint usage : 3; values */
#define USE_ADD 1		/* regular add */
#define USE_CLS_A 2		/* used in classifier A set */
#define USE_CLS_B 4		/* used in classifier B set */

/* Base struct for a table 1 entry, must be first in a table 1 entry */
typedef struct sbEMLTable1_s {
  sbEMLTable2_p_t entries;    /* linked list of T2 entries in this bucket */
  sbEMLTable2_p_t remEntries; /* linked list of removed T2 entries in
				   * this bucket that are waiting for a commit
				   * to delete them */
  uint32_t l1hash;                /* L1 hash value */
  uint32_t nused;                 /* number of t2 entries in this bucket */
  uint32_t usemap;                /* current usemap */
  uint32_t nextSeed;              /* keep track of seeds used so we don't
				   * repeat seed search */
  uint32_t usedSeed;              /* seed currently used */

  sbPayloadHandle_t phdl;    /* ptr to the payload buffer - keep so we
				   * can free */
  uint32_t isDirty;               /* set when a block has been modified,
				   * indicating it needs dmaing to the FE */
  uint32_t dataSize;              /* total size of t2 block in words */
  sbEMLTable2_p_t nextCls;    /* linked list of T2 entries that have been
				   * deleted but are still used by the
				   * clasifier */
  struct sbEMLTable1_s *nextDirty; /* link ptr of T1 entries that are
					* 'dirty' */
} sbEMLTable1_t, *sbEMLTable1_p_t;




struct emcomp {
  struct sbEMLTable1_s *table1; /* ptr to an array of (table1_count) T1
				     * entries */
  sbEMLTable2_p_t *t2freelistp; /* where to find free list */
  sbEMLTable2_p_t _t2freelist; /* free list for table 2 */

  unsigned int keySize;		/* in words */
  uint32_t table1mask;
  unsigned int nseeds;
  uint32_t table2mask;

  struct sbEMLTable1_s *headDirty;
  struct sbEMLTable1_s **tailDirty;

  struct slabTransfer commitSlab;
  unsigned int committing;
  struct slabTransfer updateSlab;
  struct slabTransfer getSlab;
  unsigned int getSlotNo;
  sbEMLPackedKeyPayload_p_t getPkp;

  sbPayloadHandle_t zeroHdl;

  /* classifier commit storage */
  unsigned int clsCommitting;
  unsigned int clsNext;
  unsigned int clsRunL;
  sbEMLTable2_p_t *tmpMacs;
  uint8_t *tmpAlloc;
  sbEMLTable2_p_t *tmpDirty;
  sbEMLTable1_p_t *tmpDirtyT1;
  unsigned int numDirty;

  /* Mulitcast classifier commit storage */
  int32_t currentT1;  /* index of current T1 - save in case we reschedule */
  sbEMLTable2_p_t currentT2; /* ptr to current T2 - again, save */

#ifdef RJZ
  /* folowing fields keep track of the cls tables sent to us by ClsComp */
  int clsSrcACount;
  sbIpv4ClassifierIdInfo_p_t  clsSrcA;
  int clsDstACount;
  sbIpv4ClassifierIdInfo_p_t  clsDstA;
  int clsSrcBCount;
  sbIpv4ClassifierIdInfo_p_t  clsSrcB;
  int clsDstBCount;
  sbIpv4ClassifierIdInfo_p_t  clsDstB;
#endif


  /* Init stuff */
  sbFeInitAsyncCallback_f_t initCb;
  void *initId;
  uint32_t initOffs;
};

enum { UIdle = 0, UWriting };
enum { GIdle = 0, GReading };
enum { CFIdle = 0, CFlushingDirty };

sbStatus_t
sbEMLDelPackedKey(sbEML_p_t em, sbEMLPackedKey_t pkey);

sbStatus_t 
sbEMLUpdateLocal(sbEML_p_t em,
                  sbEMLKey_p_t key,
                  sbEMLPayload_p_t payload,
                  sbEMLTable2_p_t *at2p);

sbEMLTable2_p_t*
lookupPKeyCls(sbEML_p_t em,
              sbEMLPackedKey_t key,
              sbEMLTable1_p_t *t1p);


void 
freeT2(struct emcomp *emp, 
       sbEMLTable2_p_t p);

sbEMLTable2_p_t 
getT2(struct emcomp *emp);

sbStatus_t 
flushDirty(sbEML_p_t em, 
           uint32_t *runLP);

typedef void (*sbEMLKeyCompare_f_t)
  (sbEML_t *emp, sbEMLTable2_t *pT2, 
   void *minrtrv, void *rtrv, void **match, void *lukey, 
   int *found);

int
sbEMLWalkTree(sbEMLCtx_p emCtxt, sbEMLKeyCompare_f_t pfCompare, 
	      void *minrtrv, void *rtrv, void **match, void *lukey, 
	      uint32_t keysize);

#endif
