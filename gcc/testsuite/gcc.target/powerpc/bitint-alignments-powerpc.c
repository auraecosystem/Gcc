/* { dg-do run { target bitint } } */
/* { dg-additional-options "-std=c23 -O2" } */

/* Test _BitInt alignment for PowerPC (uniform DImode ABI).
   _BitInt(33..N) uses DImode (8-byte) limbs on ppc64 and SImode
   (4-byte) limbs on ppc32.  All multi-limb types are 8-byte aligned
   on ppc64 and 4-byte aligned on ppc32, regardless of endianness.  */

int main(void)
{
  /* Test basic alignments */
  if (__alignof__(_BitInt(8)) != 1)
    __builtin_abort();

  if (__alignof__(_BitInt(16)) != 2)
    __builtin_abort();

  if (__alignof__(_BitInt(32)) != 4)
    __builtin_abort();

#ifdef __powerpc64__
  /* 64-bit PowerPC: DImode alignment (8 bytes) for 33..N bit _BitInt */
  if (__alignof__(_BitInt(64)) != 8)
    __builtin_abort();

  /* _BitInt(65..128) = two DImode limbs: 8-byte aligned */
  if (__alignof__(_BitInt(128)) != 8)
    __builtin_abort();
#else
  /* 32-bit PowerPC: SImode alignment (4 bytes) for 33..N bit _BitInt */
  if (__alignof__(_BitInt(64)) != 4)
    __builtin_abort();
#endif

  /* Test unsigned variants */
  if (__alignof__(unsigned _BitInt(8)) != 1)
    __builtin_abort();

  if (__alignof__(unsigned _BitInt(32)) != 4)
    __builtin_abort();

#ifdef __powerpc64__
  if (__alignof__(unsigned _BitInt(64)) != 8)
    __builtin_abort();

  if (__alignof__(unsigned _BitInt(128)) != 8)
    __builtin_abort();
#endif

  /* Large (> 128-bit) _BitInt: DImode limbs => 8-byte aligned on ppc64 */
#ifdef __powerpc64__
  /* Uniform DImode: 8 bytes on all ppc64, both endiannesses.  */
  if (__alignof__(_BitInt(200)) != 8)
    __builtin_abort();

  if (__alignof__(_BitInt(256)) != 8)
    __builtin_abort();
#else
  /* 32-bit PowerPC: SImode limbs => 4-byte aligned */
  if (__alignof__(_BitInt(200)) != 4)
    __builtin_abort();

  if (__alignof__(_BitInt(256)) != 4)
    __builtin_abort();
#endif

  return 0;
}

/* Test passes if all alignment checks succeed */
