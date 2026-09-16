/* { dg-do run } */
/* { dg-require-effective-target ppc_bf16_hw } */
/* { dg-options "-mfloat16 -O2" } */

#include <stdlib.h>

/* On a power10/power11 system, test whether we can do __bf16 calculations
   with both software emulation (i.e. running the code with -mcpu=power9) and
   hardware conversions (i.e. with -mcpu=power10 or -mcpu=power11).

   Power10 supports the XVCVBF16SPN and XVCVSPBF16 instructions that convert between
   a vector __bf16 and a vector float. */

extern void do_power9  (void) __attribute__ ((noinline,target("cpu=power9")));

volatile __bf16 two = 2.0;
volatile __bf16 three = 3.0;
volatile __bf16 p9_add, p9_sub, p9_mul, p9_div;
volatile __bf16 p10_add, p10_sub, p10_mul, p10_div;

void
do_power9 (void)
{
  p9_add = three + two;
  p9_sub = three - two;
  p9_mul = three * two;
  p9_div = three / two;
}

void
do_power10 (void)
{
  p10_add = three + two;
  p10_sub = three - two;
  p10_mul = three * two;
  p10_div = three / two;
}

int
main (int argc, char *argv[])
{
  do_power9 ();

  if (((double)p9_add) != 5.0)
    abort ();

  if (((double)p9_sub) != 1.0)
    abort ();

  if (((double)p9_mul) != 6.0)
    abort ();

  if (((double)p9_div) != 1.5)
    abort ();

  do_power10 ();

  if (((double)p10_add) != 5.0)
    abort ();

  if (((double)p10_sub) != 1.0)
    abort ();

  if (((double)p10_mul) != 6.0)
    abort ();

  if (((double)p10_div) != 1.5)
    abort ();

  return 0;
}
