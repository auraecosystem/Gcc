/* { dg-do compile } */
/* { dg-require-effective-target powerpc_future_compile_ok } */
/* { dg-require-effective-target lp64 } */
/* { dg-options "-mdejagnu-cpu=future -mdense-math -O2" } */

void
test1 (__vector_quad *dst)
{
  __vector_quad acc;
  __builtin_mma_xxsetaccz (&acc);
  *dst = acc;
}

/* { dg-final { scan-assembler-times {\mdmxxsetaccz\M} 1 } } */
