/*
 * $Id: cint_wrappers.h 1.3 Broadcom SDK $
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
 * File:        cint_wrappers.h
 * Purpose:     CINT wrapper interfaces
 *
 */

#ifndef __CINT_WRAPPERS_H__
#define __CINT_WRAPPERS_H__

#define CINT_FARGS0() \


#define CINT_FARGS1(pt0,p0) \
    pt0 p0; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \


#define CINT_FARGS2(pt0,p0,pt1,p1) \
    pt0 p0; \
    pt1 p1; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \


#define CINT_FARGS3(pt0,p0,pt1,p1,pt2,p2) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \


#define CINT_FARGS4(pt0,p0,pt1,p1,pt2,p2,pt3,p3) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \


#define CINT_FARGS5(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    pt4 p4; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \
    CINT_MEMCPY(&p4, fparams->args[4], sizeof(p4)); \


#define CINT_FARGS6(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    pt4 p4; \
    pt5 p5; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \
    CINT_MEMCPY(&p4, fparams->args[4], sizeof(p4)); \
    CINT_MEMCPY(&p5, fparams->args[5], sizeof(p5)); \


#define CINT_FARGS7(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    pt4 p4; \
    pt5 p5; \
    pt6 p6; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \
    CINT_MEMCPY(&p4, fparams->args[4], sizeof(p4)); \
    CINT_MEMCPY(&p5, fparams->args[5], sizeof(p5)); \
    CINT_MEMCPY(&p6, fparams->args[6], sizeof(p6)); \


#define CINT_FARGS8(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    pt4 p4; \
    pt5 p5; \
    pt6 p6; \
    pt7 p7; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \
    CINT_MEMCPY(&p4, fparams->args[4], sizeof(p4)); \
    CINT_MEMCPY(&p5, fparams->args[5], sizeof(p5)); \
    CINT_MEMCPY(&p6, fparams->args[6], sizeof(p6)); \
    CINT_MEMCPY(&p7, fparams->args[7], sizeof(p7)); \


#define CINT_FARGS9(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    pt4 p4; \
    pt5 p5; \
    pt6 p6; \
    pt7 p7; \
    pt8 p8; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \
    CINT_MEMCPY(&p4, fparams->args[4], sizeof(p4)); \
    CINT_MEMCPY(&p5, fparams->args[5], sizeof(p5)); \
    CINT_MEMCPY(&p6, fparams->args[6], sizeof(p6)); \
    CINT_MEMCPY(&p7, fparams->args[7], sizeof(p7)); \
    CINT_MEMCPY(&p8, fparams->args[8], sizeof(p8)); \


#define CINT_FARGS10(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    pt4 p4; \
    pt5 p5; \
    pt6 p6; \
    pt7 p7; \
    pt8 p8; \
    pt9 p9; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \
    CINT_MEMCPY(&p4, fparams->args[4], sizeof(p4)); \
    CINT_MEMCPY(&p5, fparams->args[5], sizeof(p5)); \
    CINT_MEMCPY(&p6, fparams->args[6], sizeof(p6)); \
    CINT_MEMCPY(&p7, fparams->args[7], sizeof(p7)); \
    CINT_MEMCPY(&p8, fparams->args[8], sizeof(p8)); \
    CINT_MEMCPY(&p9, fparams->args[9], sizeof(p9)); \


#define CINT_FARGS11(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9,pt10,p10) \
    pt0 p0; \
    pt1 p1; \
    pt2 p2; \
    pt3 p3; \
    pt4 p4; \
    pt5 p5; \
    pt6 p6; \
    pt7 p7; \
    pt8 p8; \
    pt9 p9; \
    pt10 p10; \
    CINT_MEMCPY(&p0, fparams->args[0], sizeof(p0)); \
    CINT_MEMCPY(&p1, fparams->args[1], sizeof(p1)); \
    CINT_MEMCPY(&p2, fparams->args[2], sizeof(p2)); \
    CINT_MEMCPY(&p3, fparams->args[3], sizeof(p3)); \
    CINT_MEMCPY(&p4, fparams->args[4], sizeof(p4)); \
    CINT_MEMCPY(&p5, fparams->args[5], sizeof(p5)); \
    CINT_MEMCPY(&p6, fparams->args[6], sizeof(p6)); \
    CINT_MEMCPY(&p7, fparams->args[7], sizeof(p7)); \
    CINT_MEMCPY(&p8, fparams->args[8], sizeof(p8)); \
    CINT_MEMCPY(&p9, fparams->args[9], sizeof(p9)); \
    CINT_MEMCPY(&p10, fparams->args[10], sizeof(p10)); \


#define CINT_FWRAPPER_FUNCTION_VP0(fname)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
\
  fname (); \
\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP0(fname)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP0(fname)  \
CINT_FWRAPPER_FUNCTION_VP0(fname)   \
CINT_FWRAPPER_PARAMS_VP0(fname)  

#define CINT_FWRAPPER_FUNCTION_VP1(fname,pt0,p0)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS1(pt0,p0);  \
\
  fname (p0); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP1(fname,pb0,p0,pp0,pa0)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP1(fname,pt0,pb0,p0,pp0,pa0)  \
CINT_FWRAPPER_FUNCTION_VP1(fname,pt0,p0)   \
CINT_FWRAPPER_PARAMS_VP1(fname,pb0,p0,pp0,pa0)  

#define CINT_FWRAPPER_FUNCTION_VP2(fname,pt0,p0,pt1,p1)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS2(pt0,p0,pt1,p1);  \
\
  fname (p0,p1); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP2(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP2(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1)  \
CINT_FWRAPPER_FUNCTION_VP2(fname,pt0,p0,pt1,p1)   \
CINT_FWRAPPER_PARAMS_VP2(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1)  

#define CINT_FWRAPPER_FUNCTION_VP3(fname,pt0,p0,pt1,p1,pt2,p2)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS3(pt0,p0,pt1,p1,pt2,p2);  \
\
  fname (p0,p1,p2); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP3(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP3(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2)  \
CINT_FWRAPPER_FUNCTION_VP3(fname,pt0,p0,pt1,p1,pt2,p2)   \
CINT_FWRAPPER_PARAMS_VP3(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2)  

#define CINT_FWRAPPER_FUNCTION_VP4(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS4(pt0,p0,pt1,p1,pt2,p2,pt3,p3);  \
\
  fname (p0,p1,p2,p3); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP4(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP4(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3)  \
CINT_FWRAPPER_FUNCTION_VP4(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3)   \
CINT_FWRAPPER_PARAMS_VP4(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3)  

#define CINT_FWRAPPER_FUNCTION_VP5(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS5(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4);  \
\
  fname (p0,p1,p2,p3,p4); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP5(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP5(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4)  \
CINT_FWRAPPER_FUNCTION_VP5(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4)   \
CINT_FWRAPPER_PARAMS_VP5(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4)  

#define CINT_FWRAPPER_FUNCTION_VP6(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS6(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5);  \
\
  fname (p0,p1,p2,p3,p4,p5); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP6(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP6(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5)  \
CINT_FWRAPPER_FUNCTION_VP6(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5)   \
CINT_FWRAPPER_PARAMS_VP6(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5)  

#define CINT_FWRAPPER_FUNCTION_VP7(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS7(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6);  \
\
  fname (p0,p1,p2,p3,p4,p5,p6); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP7(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP7(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6)  \
CINT_FWRAPPER_FUNCTION_VP7(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6)   \
CINT_FWRAPPER_PARAMS_VP7(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6)  

#define CINT_FWRAPPER_FUNCTION_VP8(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS8(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7);  \
\
  fname (p0,p1,p2,p3,p4,p5,p6,p7); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP8(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP8(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7)  \
CINT_FWRAPPER_FUNCTION_VP8(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7)   \
CINT_FWRAPPER_PARAMS_VP8(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7)  

#define CINT_FWRAPPER_FUNCTION_VP9(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS9(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8);  \
\
  fname (p0,p1,p2,p3,p4,p5,p6,p7,p8); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP9(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  { #pb8,#p8,pp8,pa8 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP9(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7,pt8,pb8,p8,pp8,pa8)  \
CINT_FWRAPPER_FUNCTION_VP9(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8)   \
CINT_FWRAPPER_PARAMS_VP9(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8)  

#define CINT_FWRAPPER_FUNCTION_VP10(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS10(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9);  \
\
  fname (p0,p1,p2,p3,p4,p5,p6,p7,p8,p9); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP10(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  { #pb8,#p8,pp8,pa8 }, \
  { #pb9,#p9,pp9,pa9 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP10(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7,pt8,pb8,p8,pp8,pa8,pt9,pb9,p9,pp9,pa9)  \
CINT_FWRAPPER_FUNCTION_VP10(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9)   \
CINT_FWRAPPER_PARAMS_VP10(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9)  

#define CINT_FWRAPPER_FUNCTION_VP11(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9,pt10,p10)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS11(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9,pt10,p10);  \
\
  fname (p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_VP11(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9,pb10,p10,pp10,pa10)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { "void", "r", 0, 0 }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  { #pb8,#p8,pp8,pa8 }, \
  { #pb9,#p9,pp9,pa9 }, \
  { #pb10,#p10,pp10,pa10 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_VP11(fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7,pt8,pb8,p8,pp8,pa8,pt9,pb9,p9,pp9,pa9,pt10,pb10,p10,pp10,pa10)  \
CINT_FWRAPPER_FUNCTION_VP11(fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9,pt10,p10)   \
CINT_FWRAPPER_PARAMS_VP11(fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9,pb10,p10,pp10,pa10)  

#define CINT_FWRAPPER_FUNCTION_RP0(rt,fname)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
\
  CINT_FRET(rt) = fname (); \
\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP0(rb,rp,ra,fname)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP0(rt,rb,rp,ra,fname)  \
CINT_FWRAPPER_FUNCTION_RP0(rt,fname)   \
CINT_FWRAPPER_PARAMS_RP0(rb,rp,ra,fname)  

#define CINT_FWRAPPER_FUNCTION_RP1(rt,fname,pt0,p0)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS1(pt0,p0);  \
\
  CINT_FRET(rt) = fname (p0); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP1(rb,rp,ra,fname,pb0,p0,pp0,pa0)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP1(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0)  \
CINT_FWRAPPER_FUNCTION_RP1(rt,fname,pt0,p0)   \
CINT_FWRAPPER_PARAMS_RP1(rb,rp,ra,fname,pb0,p0,pp0,pa0)  

#define CINT_FWRAPPER_FUNCTION_RP2(rt,fname,pt0,p0,pt1,p1)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS2(pt0,p0,pt1,p1);  \
\
  CINT_FRET(rt) = fname (p0,p1); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP2(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP2(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1)  \
CINT_FWRAPPER_FUNCTION_RP2(rt,fname,pt0,p0,pt1,p1)   \
CINT_FWRAPPER_PARAMS_RP2(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1)  

#define CINT_FWRAPPER_FUNCTION_RP3(rt,fname,pt0,p0,pt1,p1,pt2,p2)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS3(pt0,p0,pt1,p1,pt2,p2);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP3(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP3(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2)  \
CINT_FWRAPPER_FUNCTION_RP3(rt,fname,pt0,p0,pt1,p1,pt2,p2)   \
CINT_FWRAPPER_PARAMS_RP3(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2)  

#define CINT_FWRAPPER_FUNCTION_RP4(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS4(pt0,p0,pt1,p1,pt2,p2,pt3,p3);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP4(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP4(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3)  \
CINT_FWRAPPER_FUNCTION_RP4(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3)   \
CINT_FWRAPPER_PARAMS_RP4(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3)  

#define CINT_FWRAPPER_FUNCTION_RP5(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS5(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3,p4); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP5(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP5(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4)  \
CINT_FWRAPPER_FUNCTION_RP5(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4)   \
CINT_FWRAPPER_PARAMS_RP5(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4)  

#define CINT_FWRAPPER_FUNCTION_RP6(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS6(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3,p4,p5); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP6(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP6(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5)  \
CINT_FWRAPPER_FUNCTION_RP6(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5)   \
CINT_FWRAPPER_PARAMS_RP6(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5)  

#define CINT_FWRAPPER_FUNCTION_RP7(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS7(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3,p4,p5,p6); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP7(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP7(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6)  \
CINT_FWRAPPER_FUNCTION_RP7(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6)   \
CINT_FWRAPPER_PARAMS_RP7(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6)  

#define CINT_FWRAPPER_FUNCTION_RP8(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS8(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3,p4,p5,p6,p7); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP8(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP8(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7)  \
CINT_FWRAPPER_FUNCTION_RP8(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7)   \
CINT_FWRAPPER_PARAMS_RP8(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7)  

#define CINT_FWRAPPER_FUNCTION_RP9(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS9(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3,p4,p5,p6,p7,p8); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP9(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  { #pb8,#p8,pp8,pa8 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP9(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7,pt8,pb8,p8,pp8,pa8)  \
CINT_FWRAPPER_FUNCTION_RP9(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8)   \
CINT_FWRAPPER_PARAMS_RP9(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8)  

#define CINT_FWRAPPER_FUNCTION_RP10(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS10(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3,p4,p5,p6,p7,p8,p9); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP10(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  { #pb8,#p8,pp8,pa8 }, \
  { #pb9,#p9,pp9,pa9 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP10(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7,pt8,pb8,p8,pp8,pa8,pt9,pb9,p9,pp9,pa9)  \
CINT_FWRAPPER_FUNCTION_RP10(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9)   \
CINT_FWRAPPER_PARAMS_RP10(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9)  

#define CINT_FWRAPPER_FUNCTION_RP11(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9,pt10,p10)  \
static int __cint_fwrapper__##fname (cint_fparams_t* fparams) \
{ \
  CINT_FARGS11(pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9,pt10,p10);  \
\
  CINT_FRET(rt) = fname (p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); \
\
  CINT_FWRAPPER_END;\
  return 0; \
}

#define CINT_FWRAPPER_PARAMS_RP11(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9,pb10,p10,pp10,pa10)  \
static cint_parameter_desc_t __cint_parameters__##fname [] = \
{ \
  { #rb, "r", rp, ra }, \
  { #pb0,#p0,pp0,pa0 }, \
  { #pb1,#p1,pp1,pa1 }, \
  { #pb2,#p2,pp2,pa2 }, \
  { #pb3,#p3,pp3,pa3 }, \
  { #pb4,#p4,pp4,pa4 }, \
  { #pb5,#p5,pp5,pa5 }, \
  { #pb6,#p6,pp6,pa6 }, \
  { #pb7,#p7,pp7,pa7 }, \
  { #pb8,#p8,pp8,pa8 }, \
  { #pb9,#p9,pp9,pa9 }, \
  { #pb10,#p10,pp10,pa10 }, \
  CINT_ENTRY_LAST \
}

#define CINT_FWRAPPER_CREATE_RP11(rt,rb,rp,ra,fname,pt0,pb0,p0,pp0,pa0,pt1,pb1,p1,pp1,pa1,pt2,pb2,p2,pp2,pa2,pt3,pb3,p3,pp3,pa3,pt4,pb4,p4,pp4,pa4,pt5,pb5,p5,pp5,pa5,pt6,pb6,p6,pp6,pa6,pt7,pb7,p7,pp7,pa7,pt8,pb8,p8,pp8,pa8,pt9,pb9,p9,pp9,pa9,pt10,pb10,p10,pp10,pa10)  \
CINT_FWRAPPER_FUNCTION_RP11(rt,fname,pt0,p0,pt1,p1,pt2,p2,pt3,p3,pt4,p4,pt5,p5,pt6,p6,pt7,p7,pt8,p8,pt9,p9,pt10,p10)   \
CINT_FWRAPPER_PARAMS_RP11(rb,rp,ra,fname,pb0,p0,pp0,pa0,pb1,p1,pp1,pa1,pb2,p2,pp2,pa2,pb3,p3,pp3,pa3,pb4,p4,pp4,pa4,pb5,p5,pp5,pa5,pb6,p6,pp6,pa6,pb7,p7,pp7,pa7,pb8,p8,pp8,pa8,pb9,p9,pp9,pa9,pb10,p10,pp10,pa10)  

#endif /* __CINT_WRAPPERS_H__ */
