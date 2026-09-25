/* Software floating-point emulation.
   Return an bfloat16 converted to IEEE single
   Copyright (C) 2022 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   This is a clone of soft-fp/extendbfsf2.c for the PowerPC.

   We provide power8, power9, and power10 versiosns of the conversion function
   and we use an ifunc resolver to select the appropriate conversion to
   use.

   We provide an alternative version of the conversion that takes the value in
   a GPR passed as an unsigned short.  This is to allow the calling sequence
   where arguments are converted to the 32/64-bit scalar format.  If we don't
   have the hardware support, we can't use the standard __extend function.  */

#define FP_NO_EXACT_UNDERFLOW
#include "soft-fp.h"
#include "brain.h"
#include "single.h"
#include "double.h"

union ushort_bf {
  unsigned short us;
  BFtype bf;
};

typedef SFtype bf_fpr_to_sf_function_t (BFtype a);
typedef SFtype bf_gpr_to_sf_function_t (unsigned short a);

typedef bf_fpr_to_sf_function_t *bf_fpr_to_sf_ptr_function_t;
typedef bf_gpr_to_sf_function_t *bf_gpr_to_sf_ptr_function_t;

static bf_fpr_to_sf_ptr_function_t __extendbfsf2_ifunc (void);
static bf_gpr_to_sf_ptr_function_t __extendbfsf2_gpr_ifunc (void);

static bf_fpr_to_sf_function_t __extendbfsf2_p8
  __attribute__ ((__target__("cpu=power8")));

static bf_gpr_to_sf_function_t __extendbfsf2_gpr_p8
  __attribute__ ((__target__("cpu=power8")));

static bf_fpr_to_sf_function_t __extendbfsf2_p9
  __attribute__ ((__target__("cpu=power9")));

static bf_gpr_to_sf_function_t __extendbfsf2_gpr_p9
  __attribute__ ((__target__("cpu=power9")));

static bf_fpr_to_sf_function_t __extendbfsf2_p10
  __attribute__ ((__target__("cpu=power10")));

static bf_gpr_to_sf_function_t __extendbfsf2_gpr_p10
  __attribute__ ((__target__("cpu=power10")));

extern bf_fpr_to_sf_function_t __extendbfsf2
  __attribute__ ((__ifunc__ ("__extendbfsf2_ifunc")));

extern bf_gpr_to_sf_function_t __extendbfsf2_gpr
  __attribute__ ((__ifunc__ ("__extendbfsf2_gpr_ifunc")));

/* Power8 implementation, BFtype passed in FPR register, no hardware
   support.  */
static SFtype
__extendbfsf2_p8 (BFtype a)
{
  FP_DECL_EX;
  FP_DECL_B (A);
  FP_DECL_S (R);
  SFtype r;

  FP_INIT_EXCEPTIONS;
  FP_UNPACK_RAW_B (A, a);
  FP_EXTEND (S, B, 1, 1, R, A);
  FP_PACK_RAW_S (r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

/* Power8 implementation, BFtype passed in GPR register, no hardware
   support.  */
static SFtype
__extendbfsf2_gpr_p8 (unsigned short us)
{
  union ushort_bf u;
  BFtype a;

  u.us = us;
  a = u.bf;

  FP_DECL_EX;
  FP_DECL_B (A);
  FP_DECL_S (R);
  SFtype r;

  FP_INIT_EXCEPTIONS;
  FP_UNPACK_RAW_B (A, a);
  FP_EXTEND (S, B, 1, 1, R, A);
  FP_PACK_RAW_S (r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

/* Power9 implementation, BFtype passed in FPR register, use shift and
   XSCVSPDPN.  */
static SFtype
__extendbfsf2_p9 (BFtype a)
{
  return (SFtype) a;
}

/* Power9 implementation, BFtype passed in GPR register, use shift and
   XSCVSPDPN.  */
static SFtype
__extendbfsf2_gpr_p9 (unsigned short a)
{
  union ushort_bf u;
  u.us = a;
  return (SFtype) u.bf;
}

/* Power10 implementtion, BFtype passed in FPR register, use XXSPLTW,
   XVCVBF16SPN, and XSCVSPDPN.  */
static SFtype
__extendbfsf2_p10 (BFtype a)
{
  return (SFtype) a;
}

/* Power10 implementtion, BFtype passed in GPR register, use XXSPLTW,
   XVCVBF16SPN, and XSCVSPDPN.  */
static SFtype
__extendbfsf2_gpr_p10 (unsigned short a)
{
  union ushort_bf u;
  u.us = a;
  return (SFtype) u.bf;
}

/* Determine if we are on power8, power9, or power10.  */
static bf_fpr_to_sf_ptr_function_t
__extendbfsf2_ifunc (void)
{
  return (__builtin_cpu_supports ("arch_3_1")
	  ? __extendbfsf2_p10
	  : (__builtin_cpu_supports ("arch_3_00")
	     ? __extendbfsf2_p9
	     : __extendbfsf2_p8));
}

static bf_gpr_to_sf_ptr_function_t
__extendbfsf2_gpr_ifunc (void)
{
  return (__builtin_cpu_supports ("arch_3_1")
	  ? __extendbfsf2_gpr_p10
	  : (__builtin_cpu_supports ("arch_3_00")
	     ? __extendbfsf2_gpr_p9
	     : __extendbfsf2_gpr_p8));
}

/* On the PowerPC DFtype scalars use the same encoding as SFtype, so use the
   same function for both.  */
__asm__ ("\t.globl __extendbfdf2\n"
	 "__extendbfdf2 = __extendbfsf2\n"
	 "\n"
	 "\t.globl __extendbfdf2_gpr\n"
	 "__extendbfdf2_gpr = __extendbfsf2_gpr");
