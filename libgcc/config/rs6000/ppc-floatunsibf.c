/* Software floating-point emulation.
   Convert a 32bit unsigned integer to IEEE half.
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

   This is a clone of softfp/floatunssibf.c.

   We provide 2 conversions, one that returns the BFtype in a floating point
   register and the other returns it in a general purpose register that is used
   when BFtype must be converted to the 32/64-bit scalar type.  */

#include "soft-fp.h"
#include "brain.h"

BFtype
__floatunsibf (USItype i)
{
  FP_DECL_EX;
  FP_DECL_B (A);
  BFtype a;

  FP_INIT_ROUNDMODE;
  FP_FROM_INT_B (A, i, SI_BITS, USItype);
  FP_PACK_RAW_B (a, A);
  FP_HANDLE_EXCEPTIONS;

  return a;
}

unsigned short
__floatunsibf_gpr (USItype i)
{
  FP_DECL_EX;
  FP_DECL_B (A);
  BFtype a;
  union ushort_bf u;

  FP_INIT_ROUNDMODE;
  FP_FROM_INT_B (A, i, SI_BITS, USItype);
  FP_PACK_RAW_B (a, A);
  FP_HANDLE_EXCEPTIONS;

  u.bf = a;
  return u.us;
}
