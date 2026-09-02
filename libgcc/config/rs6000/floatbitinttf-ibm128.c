/* Convert a _BitInt to IBM long double (IBM double-double, TF mode).

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
     _BitInt -> KF  (via __floatbitintkf from fp128_softfp_funcs)
     KF      -> TF  (implicit narrowing; IBM DD has ~106 bits
     effective precision vs KF's 113 bits, so no precision is
     lost for values representable in IBM double-double).  */

typedef unsigned long long UBILtype;
typedef int SItype;

extern __float128 __floatbitintkf (const UBILtype *, SItype);

long double
__floatbitinttf (const UBILtype *i, SItype iprec)
{
  return (long double) __floatbitintkf (i, iprec);
}
