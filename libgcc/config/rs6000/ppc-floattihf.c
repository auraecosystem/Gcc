/* Software floating-point emulation.
   Convert a 128bit signed integer to hfloat16
   Copyright (C) 2007-2023 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.

   This is a clone of softfp/floattihf.c.

   We provide 2 conversions, one that returns the HFtype in a floating point
   register and the other returns it in a general purpose register that is used
   when HFtype must be converted to the 32/64-bit scalar type.  */

#include "soft-fp.h"
#include "half.h"

HFtype
__floattihf (TItype i)
{
  FP_DECL_EX;
  FP_DECL_H (A);
  HFtype a;

  FP_INIT_ROUNDMODE;
  FP_FROM_INT_H (A, i, TI_BITS, UTItype);
  FP_PACK_RAW_H (a, A);
  FP_HANDLE_EXCEPTIONS;

  return a;
}

unsigned short
__floattihf_gpr (TItype i)
{
  union ushort_hf u;

  FP_DECL_EX;
  FP_DECL_H (A);
  HFtype a;

  FP_INIT_ROUNDMODE;
  FP_FROM_INT_H (A, i, TI_BITS, UTItype);
  FP_PACK_RAW_H (a, A);
  FP_HANDLE_EXCEPTIONS;

  u.hf = a;
  return u.us;
}
