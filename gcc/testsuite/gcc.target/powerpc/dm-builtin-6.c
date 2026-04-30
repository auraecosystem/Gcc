/* { dg-do compile } */
/* { dg-require-effective-target powerpc_future_ok } */
/* { dg-options "-Wno-psabi -mdejagnu-cpu=future -O2" } */

void
foo (__vector_quad *dst)
{
  __vector_quad acc0, acc1;
  __builtin_mma_xxsetaccz (&acc0);
  __builtin_mma_xxsetaccz (&acc1);
  dst[0] = acc0;
  dst[1] = acc1;
}

/* { dg-final { scan-assembler-not   {\mlxv\M}           } } */
/* { dg-final { scan-assembler-not   {\mlxvp\M}          } } */
/* { dg-final { scan-assembler-not   {\mxxmtacc\M}       } } */
/* { dg-final { scan-assembler-not   {\mxxmfacc\M}       } } */
/* { dg-final { scan-assembler-times {\mdmxxsetaccz\M} 2 } } */
/* { dg-final { scan-assembler-times {\mstxvp\M}       4 } } */
