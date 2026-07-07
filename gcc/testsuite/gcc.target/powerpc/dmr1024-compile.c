/* { dg-do compile } */
/* { dg-require-effective-target powerpc_future_compile_ok } */
/* { dg-options "-mdejagnu-cpu=future -mdense-math -O2" } */

/* Verify that __dmr1024 is accepted as a type with the expected size
   and alignment, and that it can be used through pointers without
   triggering any diagnostics.  There is no __dmr1024 move support
   (movtdo) yet, so this deliberately never copies a __dmr1024 value;
   only pointers to it are passed around.  */

typedef __dmr1024 dmr_t;

_Static_assert (sizeof (__dmr1024) == 128, "__dmr1024 must be 128 bytes");
_Static_assert (__alignof__ (__dmr1024) == 64,
		 "__dmr1024 alignment must be 64 bytes");

struct dmr_holder
{
  __dmr1024 dmr;
};

extern __dmr1024 global_dmr;
extern const dmr_t *global_dmr_ptr;

__dmr1024 *
identity (__dmr1024 *p)
{
  return p;
}

const dmr_t *
identity_const (const dmr_t *p)
{
  return p;
}

void *
as_void_ptr (__dmr1024 *p)
{
  return (void *) p;
}

__dmr1024 *
holder_dmr (struct dmr_holder *h)
{
  return &h->dmr;
}
