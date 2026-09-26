/* Software floating-point emulation.
   Return 0 iff a == b, 1 otherwise
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

   This is a clone of softfp/eqhf2.c.

   We provide 2 conversions, one that returns the BFtype in a floating point
   register and the other returns it in a general purpose register that is used
   when BFtype must be converted to the 32/64-bit scalar type.  */

#include "soft-fp.h"
#include "half.h"

/* When HFtype is passed in FPR registers.  */
CMPtype
__eqhf2 (HFtype a, HFtype b)
{
  FP_DECL_EX;
  FP_DECL_H (A);
  FP_DECL_H (B);
  CMPtype r;

  FP_INIT_EXCEPTIONS;
  FP_UNPACK_RAW_H (A, a);
  FP_UNPACK_RAW_H (B, b);
  FP_CMP_EQ_H (r, A, B, 1);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

strong_alias (__eqhf2, __nehf2);

/* When HFtype is passed in GPR registers.  */
CMPtype
__eqhf2_gpr (unsigned short us_a, unsigned short us_b)
{
  FP_DECL_EX;
  FP_DECL_H (A);
  FP_DECL_H (B);
  CMPtype r;
  union ushort_hf u_a, u_b;
  HFtype a, b;

  u_a.us = us_a;
  u_b.us = us_b;
  a = u_a.hf;
  b = u_b.hf;

  FP_INIT_EXCEPTIONS;
  FP_UNPACK_RAW_H (A, a);
  FP_UNPACK_RAW_H (B, b);
  FP_CMP_EQ_H (r, A, B, 1);
  FP_HANDLE_EXCEPTIONS;

  return r;
}

strong_alias (__eqhf2_gpr, __nehf2_gpr);
