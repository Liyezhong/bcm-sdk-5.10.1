/* $Id: fe2k-asm2-md5.c 1.5.60.2 Broadcom SDK $ 
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
#include <soc/sbx/fe2k_common/fe2k-asm2-md5.h>
#ifndef __KERNEL__
#include <string.h>
#endif

/* 32-bit integer manipulation macros (little endian) */
#ifndef GET_ULONG_LE
#define GET_ULONG_LE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ]       )        \
        | ( (unsigned long) (b)[(i) + 1] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 2] << 16 )        \
        | ( (unsigned long) (b)[(i) + 3] << 24 );       \
}
#endif

#ifndef PUT_ULONG_LE
#define PUT_ULONG_LE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n)       );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 3] = (unsigned char) ( (n) >> 24 );       \
}
#endif

/* md5 init of the context structure */
int Md5c__init( Md5c *ctx )
{
  if (ctx == NULL) return 1;

  ctx->m_count[0] = 0;
  ctx->m_count[1] = 0;
  ctx->m_state[0] = 0x67452301;
  ctx->m_state[1] = 0xEFCDAB89;
  ctx->m_state[2] = 0x98BADCFE;
  ctx->m_state[3] = 0x10325476;
  MEMSET_VN (ctx->m_pad, 0, sizeof (ctx->m_pad));
  ctx->m_pad[0] = 0x80;

  return 0;
}

static int Md5c__process( Md5c *ctx, unsigned char data[64] )
{
  unsigned long X[16], A, B, C, D;

  if (ctx == NULL) return 1;

  GET_ULONG_LE( X[ 0], data,  0 );
  GET_ULONG_LE( X[ 1], data,  4 );
  GET_ULONG_LE( X[ 2], data,  8 );
  GET_ULONG_LE( X[ 3], data, 12 );
  GET_ULONG_LE( X[ 4], data, 16 );
  GET_ULONG_LE( X[ 5], data, 20 );
  GET_ULONG_LE( X[ 6], data, 24 );
  GET_ULONG_LE( X[ 7], data, 28 );
  GET_ULONG_LE( X[ 8], data, 32 );
  GET_ULONG_LE( X[ 9], data, 36 );
  GET_ULONG_LE( X[10], data, 40 );
  GET_ULONG_LE( X[11], data, 44 );
  GET_ULONG_LE( X[12], data, 48 );
  GET_ULONG_LE( X[13], data, 52 );
  GET_ULONG_LE( X[14], data, 56 );
  GET_ULONG_LE( X[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a,b,c,d,k,s,t)                                \
{                                                       \
    a += F(b,c,d) + X[k] + t; a = S(a,s) + b;           \
}

  A = ctx->m_state[0];
  B = ctx->m_state[1];
  C = ctx->m_state[2];
  D = ctx->m_state[3];

#define F(x,y,z) (z ^ (x & (y ^ z)))

  P( A, B, C, D,  0,  7, 0xD76AA478 );
  P( D, A, B, C,  1, 12, 0xE8C7B756 );
  P( C, D, A, B,  2, 17, 0x242070DB );
  P( B, C, D, A,  3, 22, 0xC1BDCEEE );
  P( A, B, C, D,  4,  7, 0xF57C0FAF );
  P( D, A, B, C,  5, 12, 0x4787C62A );
  P( C, D, A, B,  6, 17, 0xA8304613 );
  P( B, C, D, A,  7, 22, 0xFD469501 );
  P( A, B, C, D,  8,  7, 0x698098D8 );
  P( D, A, B, C,  9, 12, 0x8B44F7AF );
  P( C, D, A, B, 10, 17, 0xFFFF5BB1 );
  P( B, C, D, A, 11, 22, 0x895CD7BE );
  P( A, B, C, D, 12,  7, 0x6B901122 );
  P( D, A, B, C, 13, 12, 0xFD987193 );
  P( C, D, A, B, 14, 17, 0xA679438E );
  P( B, C, D, A, 15, 22, 0x49B40821 );

#undef F

#define F(x,y,z) (y ^ (z & (x ^ y)))

  P( A, B, C, D,  1,  5, 0xF61E2562 );
  P( D, A, B, C,  6,  9, 0xC040B340 );
  P( C, D, A, B, 11, 14, 0x265E5A51 );
  P( B, C, D, A,  0, 20, 0xE9B6C7AA );
  P( A, B, C, D,  5,  5, 0xD62F105D );
  P( D, A, B, C, 10,  9, 0x02441453 );
  P( C, D, A, B, 15, 14, 0xD8A1E681 );
  P( B, C, D, A,  4, 20, 0xE7D3FBC8 );
  P( A, B, C, D,  9,  5, 0x21E1CDE6 );
  P( D, A, B, C, 14,  9, 0xC33707D6 );
  P( C, D, A, B,  3, 14, 0xF4D50D87 );
  P( B, C, D, A,  8, 20, 0x455A14ED );
  P( A, B, C, D, 13,  5, 0xA9E3E905 );
  P( D, A, B, C,  2,  9, 0xFCEFA3F8 );
  P( C, D, A, B,  7, 14, 0x676F02D9 );
  P( B, C, D, A, 12, 20, 0x8D2A4C8A );

#undef F
    
#define F(x,y,z) (x ^ y ^ z)

  P( A, B, C, D,  5,  4, 0xFFFA3942 );
  P( D, A, B, C,  8, 11, 0x8771F681 );
  P( C, D, A, B, 11, 16, 0x6D9D6122 );
  P( B, C, D, A, 14, 23, 0xFDE5380C );
  P( A, B, C, D,  1,  4, 0xA4BEEA44 );
  P( D, A, B, C,  4, 11, 0x4BDECFA9 );
  P( C, D, A, B,  7, 16, 0xF6BB4B60 );
  P( B, C, D, A, 10, 23, 0xBEBFBC70 );
  P( A, B, C, D, 13,  4, 0x289B7EC6 );
  P( D, A, B, C,  0, 11, 0xEAA127FA );
  P( C, D, A, B,  3, 16, 0xD4EF3085 );
  P( B, C, D, A,  6, 23, 0x04881D05 );
  P( A, B, C, D,  9,  4, 0xD9D4D039 );
  P( D, A, B, C, 12, 11, 0xE6DB99E5 );
  P( C, D, A, B, 15, 16, 0x1FA27CF8 );
  P( B, C, D, A,  2, 23, 0xC4AC5665 );

#undef F

#define F(x,y,z) (y ^ (x | ~z))

  P( A, B, C, D,  0,  6, 0xF4292244 );
  P( D, A, B, C,  7, 10, 0x432AFF97 );
  P( C, D, A, B, 14, 15, 0xAB9423A7 );
  P( B, C, D, A,  5, 21, 0xFC93A039 );
  P( A, B, C, D, 12,  6, 0x655B59C3 );
  P( D, A, B, C,  3, 10, 0x8F0CCC92 );
  P( C, D, A, B, 10, 15, 0xFFEFF47D );
  P( B, C, D, A,  1, 21, 0x85845DD1 );
  P( A, B, C, D,  8,  6, 0x6FA87E4F );
  P( D, A, B, C, 15, 10, 0xFE2CE6E0 );
  P( C, D, A, B,  6, 15, 0xA3014314 );
  P( B, C, D, A, 13, 21, 0x4E0811A1 );
  P( A, B, C, D,  4,  6, 0xF7537E82 );
  P( D, A, B, C, 11, 10, 0xBD3AF235 );
  P( C, D, A, B,  2, 15, 0x2AD7D2BB );
  P( B, C, D, A,  9, 21, 0xEB86D391 );

#undef F

  ctx->m_state[0] += A;
  ctx->m_state[1] += B;
  ctx->m_state[2] += C;
  ctx->m_state[3] += D;

  return 0;
}

/* MD5 process buffer */
int Md5c__update( Md5c *ctx, unsigned char *input, int ilen )
{
  int fill;
  unsigned long left;

  if (ctx == NULL) return 1;
  if (input == NULL) return 2;
  if( ilen <= 0 ) return 3;

  left = ctx->m_count[0] & 0x3F;
  fill = 64 - left;

  ctx->m_count[0] += ilen;
  ctx->m_count[0] &= 0xFFFFFFFF;

  if( ctx->m_count[0] < (unsigned long) ilen ) ctx->m_count[1]++;

  if( left && ilen >= fill ) {
    memcpy( (void *) (ctx->m_buf + left), (void *) input, fill );
    Md5c__process( ctx, ctx->m_buf );
    input += fill;
    ilen  -= fill;
    left = 0;
  }

  while( ilen >= 64 ) {
    Md5c__process( ctx, input );
    input += 64;
    ilen  -= 64;
  }

  if( ilen > 0 ) memcpy( (void *) (ctx->m_buf + left), (void *) input, ilen );

  return 0;
}

/* MD5 final digest */
int Md5c__finish( Md5c *ctx, unsigned char output[16] )
{
  unsigned long last, padn, high, low;
  unsigned char msglen[8];

  if (ctx == NULL) return 1;

  high = ( ctx->m_count[0] >> 29 ) | ( ctx->m_count[1] <<  3 );
  low  = ( ctx->m_count[0] <<  3 );

  PUT_ULONG_LE( low,  msglen, 0 );
  PUT_ULONG_LE( high, msglen, 4 );

  last = ctx->m_count[0] & 0x3F;
  padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

  Md5c__update( ctx, (unsigned char *) ctx->m_pad, padn );
  /* coverity[overrun-buffer-val] */
  Md5c__update( ctx, msglen, 8 );

  PUT_ULONG_LE( ctx->m_state[0], output,  0 );
  PUT_ULONG_LE( ctx->m_state[1], output,  4 );
  PUT_ULONG_LE( ctx->m_state[2], output,  8 );
  PUT_ULONG_LE( ctx->m_state[3], output, 12 );

  MEMSET_VN( ctx, 0, sizeof( Md5c ) );

  return 0;
}
