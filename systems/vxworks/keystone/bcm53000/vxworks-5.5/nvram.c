/*
 * Stubs for NVRAM functions for platforms without flash
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
 * $Id: nvram.c 1.2 Broadcom SDK $
 */

#include <bcmnvram.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <sbsdram.h>

extern struct nvram_tuple *_nvram_realloc(struct nvram_tuple *t, const char *name,
                                          const char *value);
extern void _nvram_free(struct nvram_tuple *t);
extern int _nvram_read(void *buf);

extern char *_nvram_get(const char *name);
extern int _nvram_set(const char *name, const char *value);
extern int _nvram_unset(const char *name);
extern int _nvram_getall(char *buf, int count);
extern int _nvram_commit(struct nvram_header *header);
extern int _nvram_init(void *sih);
extern void _nvram_exit(void);
uint8 nvram_calc_crc(struct nvram_header *nvh);

static struct nvram_tuple *BCMINITDATA(nvram_hash)[257];
static struct nvram_tuple *nvram_dead;

/* Free all tuples. Should be locked. */
static void
BCMINITFN(nvram_free)(void)
{
    uint i;
    struct nvram_tuple *t, *next;
    
    /* Free hash table */
    for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
        for (t = nvram_hash[i]; t; t = next) {
            next = t->next;
            _nvram_free(t);
        }
        nvram_hash[i] = NULL;
    }
    
    /* Free dead table */
    for (t = nvram_dead; t; t = next) {
        next = t->next;
        _nvram_free(t);
    }
    nvram_dead = NULL;
    
    /* Indicate to per-port code that all tuples have been freed */
    _nvram_free(NULL);
}

/* String hash */
static INLINE uint
hash(const char *s)
{
    uint hash = 0;
    
    while (*s)
        hash = 31 *hash + *s++;
    
    return hash;
}

/* (Re)initialize the hash table. Should be locked. */
static int
BCMINITFN(nvram_rehash)(struct nvram_header *header)
{
    char *name, *value, *end, *eq;
    
    /* (Re)initialize hash table */
    nvram_free();
    
    /* Parse and set "name=value\0 ... \0\0" */
    name = (char *) &header[1];
    end = (char *) header + NVRAM_SPACE - 2;
    end[0] = end[1] = '\0';
    for (; *name; name = value + strlen(value) + 1) {
        if (!(eq = strchr(name, '=')))
            break;
        *eq = '\0';
        value = eq + 1;
        _nvram_set(name, value);
        *eq = '=';
    }
    
    return 0;
}

/* Get the value of an NVRAM variable. Should be locked. */
char *
_nvram_get(const char *name)
{
    uint i;
    struct nvram_tuple *t;
    char *value;
    
    if (!name)
        return NULL;
    
    /* Hash the name */
    i = hash(name) % ARRAYSIZE(nvram_hash);
    
    /* Find the associated tuple in the hash table */
    for (t = nvram_hash[i]; t && strcmp(t->name, name); t = t->next);
    
    value = t ? t->value : NULL;
    
    return value;
}

/* Set the value of an NVRAM variable. Should be locked. */
int
BCMINITFN(_nvram_set)(const char *name, const char *value)
{
    uint i;
    struct nvram_tuple *t, *u, **prev;
    
    /* Hash the name */
    i = hash(name) % ARRAYSIZE(nvram_hash);
    
    /* Find the associated tuple in the hash table */
    for (prev = &nvram_hash[i], t = *prev; t && strcmp(t->name, name);
        prev = &t->next, t = *prev);
    
    /* (Re)allocate tuple */
    if (!(u = _nvram_realloc(t, name, value)))
        return -12; /* -ENOMEM */
    
    /* Value reallocated */
    if (t && t == u)
        return 0;
    
    /* Move old tuple to the dead table */
    if (t) {
        *prev = t->next;
        t->next = nvram_dead;
        nvram_dead = t;
    }
    
    /* Add new tuple to the hash table */
    u->next = nvram_hash[i];
    nvram_hash[i] = u;
    
    return 0;
}

/* Unset the value of an NVRAM variable. Should be locked. */
int
BCMINITFN(_nvram_unset)(const char *name)
{
    uint i;
    struct nvram_tuple *t, **prev;
    
    if (!name)
        return 0;
    
    /* Hash the name */
    i = hash(name) % ARRAYSIZE(nvram_hash);
    
    /* Find the associated tuple in the hash table */
    for (prev = &nvram_hash[i], t = *prev; t && strcmp(t->name, name);
        prev = &t->next, t = *prev);
    
    /* Move it to the dead table */
    if (t) {
        *prev = t->next;
        t->next = nvram_dead;
        nvram_dead = t;
    }
    
    return 0;
}

/* Get all NVRAM variables. Should be locked. */
int
_nvram_getall(char *buf, int count)
{
    uint i;
    struct nvram_tuple *t;
    int len = 0;
    
    bzero(buf, count);
    
    /* Write name=value\0 ... \0\0 */
    for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
        for (t = nvram_hash[i]; t; t = t->next) {
            if ((count - len) > (strlen(t->name) + 1 + strlen(t->value) + 1))
                len += sprintf(buf + len, "%s=%s", t->name, t->value) + 1;
            else
                break;
        }
    }
    
    return 0;
}

/* Regenerate NVRAM. Should be locked. */
/* XXX: not marking as RECLAIMTEXT allows this fucntion to be fc-sectioned out when not referenced
 */
int
BCMINITFN(_nvram_commit)(struct nvram_header *header)
{
    char *ptr, *end;
    int i;
    struct nvram_tuple *t;
    
    /* Regenerate header */
    header->magic = NVRAM_MAGIC;
    header->crc_ver_init = (NVRAM_VERSION << 8);
	header->config_refresh = 0; /* for backward compatibility */
	header->config_ncdl = 0; /* for backward compatibility */
    
    /* Clear data area */
    ptr = (char *) header + sizeof(struct nvram_header);
    bzero(ptr, NVRAM_SPACE - sizeof(struct nvram_header));
    
    /* Leave space for a double NUL at the end */
    end = (char *) header + NVRAM_SPACE - 2;
    
    /* Write out all tuples */
    for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
        for (t = nvram_hash[i]; t; t = t->next) {
            if ((ptr + strlen(t->name) + 1 + strlen(t->value) + 1) > end)
                break;
            ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
        }
    }
    
    /* End with a double NUL */
    ptr += 2;
    
    /* Set new length */
    header->len = ROUNDUP(ptr - (char *) header, 4);
    
    /* Set new CRC8 */
    header->crc_ver_init |= nvram_calc_crc(header);
    
    /* Reinitialize hash table */
    return nvram_rehash(header);
}

/* Initialize hash table. Should be locked. */
int
BCMINITFN(_nvram_init)(void *sih)
{
    struct nvram_header *header;
    int ret;
    
    
    if (!(header = (struct nvram_header *) MALLOC(si_osh(sih), NVRAM_SPACE))) {
        printf("nvram_init: out of memory\n");
        return -12; /* -ENOMEM */
    }
    
    if ((ret = _nvram_read(header)) == 0 &&
        header->magic == NVRAM_MAGIC)
        nvram_rehash(header);
    
    MFREE(si_osh(sih), header, NVRAM_SPACE);
    return ret;
}

 /* Free hash table. Should be locked. */
void
BCMINITFN(_nvram_exit)(void)
{
    nvram_free();
}

/* returns the CRC8 of the nvram */
uint8
BCMINITFN(nvram_calc_crc)(struct nvram_header *nvh)
{
    struct nvram_header tmp;
    uint8 crc;
    
    /* Little-endian CRC8 over the last 11 bytes of the header */
    tmp.crc_ver_init = htol32((nvh->crc_ver_init & NVRAM_CRC_VER_MASK));
    tmp.config_refresh = htol32(nvh->config_refresh);
    tmp.config_ncdl = htol32(nvh->config_ncdl);
    
    crc = hndcrc8((uint8 *) &tmp + NVRAM_CRC_START_POSITION,
    sizeof(struct nvram_header) - NVRAM_CRC_START_POSITION,
    CRC8_INIT_VALUE);
    
    /* Continue CRC8 over data bytes */
    crc = hndcrc8((uint8 *) &nvh[1], nvh->len - sizeof(struct nvram_header), crc);
    
    return crc;
}
