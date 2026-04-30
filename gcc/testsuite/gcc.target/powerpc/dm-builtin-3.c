/* { dg-do compile } */
/* { dg-require-effective-target powerpc_future_ok } */
/* { dg-options "-Wno-psabi -mdejagnu-cpu=future -O2" } */

void
foo0 (void)
{
  __vector_quad acc;
  asm ("#..." : "=d" (acc));
  __builtin_mma_xxmtacc (&acc);
  __builtin_mma_xxmfacc (&acc);
  asm ("#..." :: "d" (acc));
}

typedef unsigned char  vec_t __attribute__((vector_size(16)));

void
foo1 (vec_t *vec)
{
  vec[1] = __builtin_vsx_xvcvspbf16 (vec[0]);
  vec[3] = __builtin_vsx_xvcvbf16spn (vec[2]);
}

/* { dg-final { scan-assembler-not   {\mxxmtacc\M}         } } */
/* { dg-final { scan-assembler-not   {\mxxmfacc\M}         } } */
/* { dg-final { scan-assembler-not   {\mlxvp\M}            } } */
/* { dg-final { scan-assembler-not   {\mstxvp\M}           } } */
/* { dg-final { scan-assembler-times {\mlxv\M}           2 } } */
/* { dg-final { scan-assembler-times {\mstxv\M}          2 } } */
/* { dg-final { scan-assembler-times {\mdmxvcvspbf16\M}  1 } } */
/* { dg-final { scan-assembler-times {\mdmxvcvbf16spn\M} 1 } } */
