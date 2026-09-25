/* Software floating-point emulation.
   Return an IEEE half converted to IEEE single
   Copyright (C) 2021-2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file into
   combinations with other programs, and to distribute those
   combinations without any restriction coming from the use of this
   file.  (The Lesser General Public License restrictions do apply in
   other respects; for example, they cover modification of the file,
   and distribution when not linked into a combine executable.)

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This is a clone of soft-fp/extendhfsf2.c for the PowerPC.  It
   provides both __extendhfsf2 and __extendhfdf2, since both types
   return 64-bit floating point formats.

   We provide power8, and power9 versiosns of the conversion function and we
   use an ifunc resolver to select the appropriate conversion to use.

   We provide an alternative version of the conversion that takes the value in
   a GPR passed as an unsigned short.  This is to allow the calling sequence
   where arguments are converted to the 32/64-bit scalar format.  If we don't
   have the hardware support, we can't use the standard __extend function.  */

#define FP_NO_EXACT_UNDERFLOW
#include "soft-fp.h"
#include "half.h"
#include "single.h"
#include "double.h"

union ushort_hf {
  unsigned short us;
  HFtype hf;
};

typedef SFtype hf_fpr_to_sf_function_t (HFtype a);
typedef SFtype hf_gpr_to_sf_function_t (unsigned short a);

typedef hf_fpr_to_sf_function_t *hf_fpr_to_sf_ptr_function_t;
typedef hf_gpr_to_sf_function_t *hf_gpr_to_sf_ptr_function_t;

typedef DFtype hf_fpr_to_df_function_t (HFtype a);
typedef DFtype hf_gpr_to_df_function_t (unsigned short a);

typedef hf_fpr_to_df_function_t *hf_fpr_to_df_ptr_function_t;
typedef hf_gpr_to_df_function_t *hf_gpr_to_df_ptr_function_t;

static hf_fpr_to_sf_ptr_function_t __extendhfsf2_ifunc (void);
static hf_gpr_to_sf_ptr_function_t __extendhfsf2_gpr_ifunc (void);

static hf_fpr_to_df_ptr_function_t __extendhfdf2_ifunc (void);
static hf_gpr_to_df_ptr_function_t __extendhfdf2_gpr_ifunc (void);

static hf_fpr_to_sf_function_t __extendhfsf2_p8
  __attribute__ ((__target__("cpu=power8")));

static hf_gpr_to_sf_function_t __extendhfsf2_gpr_p8
  __attribute__ ((__target__("cpu=power8")));

static hf_fpr_to_sf_function_t __extendhfsf2_p9
  __attribute__ ((__target__("cpu=power9")));

static hf_gpr_to_sf_function_t __extendhfsf2_gpr_p9
  __attribute__ ((__target__("cpu=power9")));

extern hf_fpr_to_sf_function_t __extendhfsf2
  __attribute__ ((__ifunc__ ("__extendhfsf2_ifunc")));

extern hf_gpr_to_sf_function_t __extendhfsf2_gpr
  __attribute__ ((__ifunc__ ("__extendhfsf2_gpr_ifunc")));

/* Power8 implementation, HFtype passed in FPR register, no hardware
   support.  */
static SFtype
__extendhfsf2_p8 (HFtype a)
{
  FP_DECL_EX;
  FP_DECL_H (A);
  FP_DECL_S (R);
  SFtype r;

  FP_INIT_EXCEPTIONS;
  FP_UNPACK_RAW_H (A, a);
  FP_EXTEND (S, H, 1, 1, R, A);
  FP_PACK_RAW_S (r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

/* Power8 implementation, HFtype passed in GPR register, no hardware
   support.  */
static SFtype
__extendhfsf2_gpr_p8 (unsigned short us)
{
  union ushort_hf u;
  HFtype a;

  u.us = us;
  a = u.hf;

  FP_DECL_EX;
  FP_DECL_H (A);
  FP_DECL_S (R);
  SFtype r;

  FP_INIT_EXCEPTIONS;
  FP_UNPACK_RAW_H (A, a);
  FP_EXTEND (S, H, 1, 1, R, A);
  FP_PACK_RAW_S (r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

/* Power9 implementation, HFtype passed in FPR register, use XVCVHPSP.  */
static SFtype
__extendhfsf2_p9 (HFtype a)
{
  return (SFtype) a;
}

/* Power9 implementation, HFtype passed in GPR register, use XVCVHPSP.  */
static SFtype
__extendhfsf2_gpr_p9 (unsigned short a)
{
  union ushort_hf u;
  u.us = a;
  return (SFtype) u.hf;
}

/* Determine if we are on power8 or power9.  */
static hf_fpr_to_sf_ptr_function_t
__extendhfsf2_ifunc (void)
{
  return (__builtin_cpu_supports ("arch_3_00")
	  ? __extendhfsf2_p9
	  : __extendhfsf2_p8);
}

static hf_gpr_to_sf_ptr_function_t
__extendhfsf2_gpr_ifunc (void)
{
  return (__builtin_cpu_supports ("arch_3_00")
	  ? __extendhfsf2_gpr_p9
	  : __extendhfsf2_gpr_p8);
}

/* DFtype functions.  */
static hf_fpr_to_df_ptr_function_t
__extendhfdf2_ifunc (void)
{
  return (hf_fpr_to_df_ptr_function_t) (__builtin_cpu_supports ("arch_3_00")
					? __extendhfsf2_p9
					: __extendhfsf2_p8);
}

static hf_gpr_to_df_ptr_function_t
__extendhfdf2_gpr_ifunc (void)
{
  return (hf_gpr_to_df_ptr_function_t) (__builtin_cpu_supports ("arch_3_00")
					? __extendhfsf2_gpr_p9
					: __extendhfsf2_gpr_p8);
}
