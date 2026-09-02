/* Convert an IBM long double (IBM double-double, TF mode) to a _BitInt.

   Copyright (C) 2026 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/* Compiled with -mabi=ibmlongdouble (long double = IBM double-double)
   plus FP128_CFLAGS_SW (-mfloat128 for __float128 / KF mode access).

   Route through the IEEE 128-bit (__float128 / KF) representation:
     TF (IBM DD) -> KF  (via __extendkftf2; exact, KF has more precision)
     KF          -> _BitInt (via __fixkfbitint from fp128_softfp_funcs).  */

typedef unsigned long long UBILtype;
typedef int SItype;

extern void __fixkfbitint (UBILtype *, SItype, __float128);

void
__fixtfbitint (UBILtype *r, SItype rprec, long double a)
{
  __fixkfbitint (r, rprec, (__float128) a);
}
