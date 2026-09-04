/* { dg-do compile } */
/* { dg-options "-std=c99 -O2 -fno-tree-vrp -fno-tree-dominator-opts" } */

/* PR tree-optimization/126532
   The following code used to cause an ICE as the logical and reverse operator
   range calculation was accessing a range's type without checking it was
   non-undefined.  */

int f_b, f_r;

__attribute__((noipa)) void
f (int a)
{
  _Bool t1 = f_b;
  if (t1)
    {
      _Bool t3 = a && t1;
      _Bool t39 = t3 ^ t1;
      if (t3)
        ;
      else if (!a)
        ;
      else
        f_r += 7;
      if (t39)
        f_r += 3;
    }
}

int
main (void)
{
  for (int b = 0; b < 2; b++)
    for (int a = -1; a < 2; a++)
      {
        int e = 0;
        f_b = b;
        f_r = 0;
        f (a);
        if (b)
          {
            int t3 = (a != 0);
            int t39 = t3 ^ 1;
            if (!t3 && a)
              e += 7;
            if (t39)
              e += 3;
          }
        if (f_r != e)
          __builtin_abort ();
      }
  return 0;
}
