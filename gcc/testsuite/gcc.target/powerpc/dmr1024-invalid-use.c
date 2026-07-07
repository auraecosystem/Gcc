/* { dg-do compile } */
/* { dg-require-effective-target powerpc_future_compile_ok } */
/* { dg-options "-mdejagnu-cpu=future -O2 -std=gnu17" } */

/* The __dmr1024 opaque type, like __vector_quad and __vector_pair
   (see pr96506-1.c/pr96506-2.c), may not be used as a function
   parameter or return value.  Verify we flag errors on these uses
   rather than ICE.  */

extern void bar0 ();

typedef __dmr1024 dmr_t;

void
foo0 (void)
{
  __dmr1024 v;
  bar0 (v); /* { dg-error "invalid use of Dense Math operand of type .__dmr1024. as a function parameter" } */
}

void
foo1 (void)
{
  dmr_t v;
  bar0 (v); /* { dg-error "invalid use of Dense Math operand of type .__dmr1024. as a function parameter" } */
}

__dmr1024
foo2 (__dmr1024 *src)
{ /* { dg-error "invalid use of Dense Math type .__dmr1024. as a function return value" } */
  return *src;
}

dmr_t
foo3 (dmr_t *src)
{ /* { dg-error "invalid use of Dense Math type .__dmr1024. as a function return value" } */
  return *src;
}
