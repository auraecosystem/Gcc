/* Software floating-point emulation.
   Truncate IEEE single into IEEE half.
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
   <http://www.gnu.org/licenses/>.

   This is a clone of soft-fp/truncdfbf2.c for the PowerPC.

   We provide power8, and power10 versiosns of the conversion function and we
   use an ifunc resolver to select the appropriate conversion to use.

   We provide an alternative version of the conversion that retunrs the HFtype
   value as unsigned short.  This is to allow the calling sequence where
   arguments are converted to the 32/64-bit scalar format.  If we don't have
   the hardware support, we can't use the standard __trunc function.  */

#include "soft-fp.h"
#include "brain.h"
#include "single.h"
#include "double.h"

/* Union to convert between BFtype and unsigned short.  */
union ushort_bf {
  unsigned short us;
  BFtype bf;
};

/* Function and pointer to function prototypes.  */
typedef BFtype sf_to_bf_fpr_function_t (SFtype a);
typedef unsigned short sf_to_bf_gpr_function_t (SFtype a);

typedef sf_to_bf_fpr_function_t *sf_to_bf_fpr_ptr_function_t;
typedef sf_to_bf_gpr_function_t *sf_to_bf_gpr_ptr_function_t;

static sf_to_bf_fpr_ptr_function_t __truncsfbf2_ifunc (void);
static sf_to_bf_gpr_ptr_function_t __truncsfbf2_gpr_ifunc (void);

static sf_to_bf_fpr_function_t __truncsfbf2_p8
  __attribute__ ((__target__ ("cpu=power8")));

static sf_to_bf_gpr_function_t __truncsfbf2_gpr_p8
  __attribute__ ((__target__ ("cpu=power8")));

static sf_to_bf_fpr_function_t __truncsfbf2_p10
  __attribute__ ((__target__ ("cpu=power10")));

static sf_to_bf_gpr_function_t __truncsfbf2_gpr_p10
  __attribute__ ((__target__ ("cpu=power10")));

extern sf_to_bf_fpr_function_t __truncsfbf2
  __attribute__ ((__ifunc__ ("__truncsfbf2_ifunc")));

extern sf_to_bf_gpr_function_t __truncsfbf2_gpr
  __attribute__ ((__ifunc__ ("__truncsfbf2_gpr_ifunc")));

/* Power8, return BFtype in a FPR register, no hardware support.  */
static BFtype
__truncsfbf2_p8 (SFtype a)
{
  FP_DECL_EX;
  FP_DECL_S (A);
  FP_DECL_B (R);
  BFtype r;

  FP_INIT_ROUNDMODE;
  FP_UNPACK_SEMIRAW_D (A, a);
#if _FP_W_TYPE_SIZE < _FP_FRACBITS_D
  FP_TRUNC (B, D, 1, 2, R, A);
#else
  FP_TRUNC (B, D, 1, 1, R, A);
#endif
  FP_PACK_SEMIRAW_B (r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

/* Power8, return BFtype in a GPR register, no hardware support.  */
static unsigned short
__truncsfbf2_gpr_p8 (SFtype a)
{
  union ushort_bf u;

  FP_DECL_EX;
  FP_DECL_S (A);
  FP_DECL_B (R);
  BFtype r;

  FP_INIT_ROUNDMODE;
  FP_UNPACK_SEMIRAW_D (A, a);
#if _FP_W_TYPE_SIZE < _FP_FRACBITS_D
  FP_TRUNC (B, D, 1, 2, R, A);
#else
  FP_TRUNC (B, D, 1, 1, R, A);
#endif
  FP_PACK_SEMIRAW_B (r, R);
  FP_HANDLE_EXCEPTIONS;

  u.bf = r;
  return u.us;
}

/* Power10, return BFtype in a FPR register, use XVCVSPBF16.  */
static BFtype
__truncsfbf2_p10 (SFtype a)
{
  return (BFtype) a;
}
  
/* Power9, return BFtype in a GPR register, use XVCVSPBF16.  */
static unsigned short
__truncsfbf2_gpr_p10 (SFtype a)
{
  union ushort_bf u;
  u.bf = a;
  return u.us;
}

/* Resolve ifunc declarations for SFtype to BFtype.  */
static sf_to_bf_fpr_ptr_function_t
__truncsfbf2_ifunc (void)
{
  return (__builtin_cpu_supports ("arch_3_1")
	  ? __truncsfbf2_p10
	  : __truncsfbf2_p8);
}

static sf_to_bf_gpr_ptr_function_t
__truncsfbf2_gpr_ifunc (void)
{
  return (__builtin_cpu_supports ("arch_3_1")
	  ? __truncsfbf2_gpr_p10
	  : __truncsfbf2_gpr_p8);
}

/* On the PowerPC DFtype scalars use the same encoding as SFtype, so use the
   same function for both.  */
__asm__ ("\n"
	 "\t.globl __truncdfbf2\n"
	 "__truncdfbf2 = __truncsfbf2\n"
	 "\n"
	 "\t.globl __truncdfbf2_gpr\n"
	 "__truncdfbf2_gpr = __truncsfbf2_gpr\n");
