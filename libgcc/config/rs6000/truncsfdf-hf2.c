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

   This is a clone of soft-fp/truncsfhf2.c for the PowerPC.

   We provide an alternative version of the conversion that retunrs the HFtype
   value as unsigned short.  This is to allow the calling sequence where
   arguments are converted to the 32/64-bit scalar format.  If we don't have
   the hardware support, we can't use the standard __trunc function.  */

#include "soft-fp.h"
#include "half.h"
#include "single.h"
#include "double.h"

/* Return HFtype in a FPR register.  */
HFtype
__truncsfhf2 (SFtype a)
{
  FP_DECL_EX;
  FP_DECL_S (A);
  FP_DECL_H (R);
  HFtype r;

  FP_INIT_ROUNDMODE;
  FP_UNPACK_SEMIRAW_S (A, a);
  FP_TRUNC (H, S, 1, 1, R, A);
  FP_PACK_SEMIRAW_H (r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

/* Return HFtype in a GPR register.  */
unsigned short
__truncsfhf2_gpr (SFtype a)
{
  union ushort_hf u;

  FP_DECL_EX;
  FP_DECL_S (A);
  FP_DECL_H (R);
  HFtype r;

  FP_INIT_ROUNDMODE;
  FP_UNPACK_SEMIRAW_S (A, a);
  FP_TRUNC (H, S, 1, 1, R, A);
  FP_PACK_SEMIRAW_H (r, R);
  FP_HANDLE_EXCEPTIONS;

  u.hf = r;
  return u.us;
}

/* On the PowerPC DFtype scalars use the same encoding as SFtype, so use the
   same function for both.  */
__asm__ ("\n"
	 "\t.globl __truncdfhf2\n"
	 "__truncdfhf2 = __truncsfhf2\n"
	 "\n"
	 "\t.globl __truncdfhf2_gpr\n"
	 "__truncdfhf2_gpr = __truncsfhf2_gpr\n");
