/* { dg-do compile } */
/* { dg-skip-if "" { powerpc*-*-aix* } } */
/* { dg-options "-O2 -mdejagnu-cpu=power10 -mno-pcrel" } */

/* Verify that the test case from PR108018 won't fail.  */

static int foo, bar;

unsigned long test(void)
{
        return (unsigned long)&foo + (unsigned long)&bar;
}

/* { dg-final { scan-assembler-times {\maddis[^\n]*\n\taddi\M} 3 } } */
