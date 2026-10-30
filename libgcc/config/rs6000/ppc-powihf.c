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

   The code is cloned from the code in libgcc2.c (which handles the standard
   SF, DF, TF, and XF types).

   We provide 2 conversions, one that returns the BFtype in a floating point
   register and the other returns it in a general purpose register that is used
   when BFtype must be converted to the 32/64-bit scalar type.

   In addition, we use the target_clones attribute so that if the user is
   running on a power9 or power10, they get a version that is compiled to use
   the HF conversions.  If the default for the compiler is power10 or greater,
   we don't use target_clones.  */

#include "soft-fp.h"
#include "half.h"

#if defined(_ARCH_PWR10)
#define ATTRIBUTE_TARGET_CLONES

#elif defined(_ARCH_PWR9)
#define ATTRIBUTE_TARGET_CLONES	\
  __attribute__((target_clones("cpu=power10,default")))

#else
#define ATTRIBUTE_TARGET_CLONES \
  __attribute__((target_clones("cpu=power10,power9,default")))
#endif

#define ONE	((HFtype) 1.0)


ATTRIBUTE_TARGET_CLONES
HFtype
__powihf2 (HFtype x, SItype m)
{
  unsigned int n = m < 0 ? -m : m;
  HFtype y = n % 2 ? x : ONE;

  while (n >>= 1)
    {
      x = x * x;
      if (n % 2)
	y = y * x;
    }

  return m < 0 ? ONE/y : y;
}

/* Version of the powi function that passes and return _Float16 values in
   gprs.  */


ATTRIBUTE_TARGET_CLONES
unsigned short
__powihf2_gpr (unsigned short us_x, SItype m)
{
  union ushort_hf u_x, u_r;
  unsigned int n;
  HFtype x, y;

  u_x.us = us_x;
  n = m < 0 ? -m : m;
  x = u_x.hf;
  y = n % 2 ? x : ONE;

  while (n >>= 1)
    {
      x = x * x;
      if (n % 2)
	y = y * x;
    }

  u_r.hf = m < 0 ? ONE/y : y;
  return u_r.us;
}
