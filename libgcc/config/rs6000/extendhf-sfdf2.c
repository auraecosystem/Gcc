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
   <http://www.gnu.org/licenses/>.

   This is a clone of soft-fp/extendhfsf2.c for the PowerPC.

   We provide an alternative version of the conversion that takes the value in
   a GPR passed as an unsigned short.  This is to allow the calling sequence
   where arguments are converted to the 32/64-bit scalar format.  If we don't
   have the hardware support, we can't use the standard __extend function.  */

#define FP_NO_EXACT_UNDERFLOW
#include "soft-fp.h"
#include "half.h"
#include "single.h"
#include "double.h"

/* HFtype passed in FPR register.  */
SFtype
__extendhfsf2 (HFtype a)
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

/* HFtype passed in GPR register.  */
SFtype
__extendhfsf2_gpr (unsigned short us)
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

/* On the PowerPC DFtype scalars use the same encoding as SFtype, so use the
   same function for both.  */
__asm__ ("\t.globl __extendhfdf2\n"
	 "__extendhfdf2 = __extendhfsf2\n"
	 "\n"
	 "\t.globl __extendhfdf2_gpr\n"
	 "__extendhfdf2_gpr = __extendhfsf2_gpr");
