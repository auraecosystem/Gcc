/* { dg-do run { target bitint } } */
/* { dg-additional-options "-std=c23 -O0" } */

/* Test _BitInt size calculations for PowerPC (uniform DImode ABI).
   ppc64: DImode (8-byte) limbs for all widths > 32 bits.
   ppc32: SImode (4-byte) limbs throughout.  */

static long unsigned int
calc_size (int n)
{
#ifdef __powerpc64__
/* 64-bit PowerPC: uniform DImode (8-byte) limbs for all n > 32.
   _BitInt(65..128) = 2 limbs = 16 bytes (same on LE and BE).
   _BitInt(>128)    = ceil(n/64) * 8 bytes.  */
  if (n > 32)
    return ((n - 1) / 64 + 1) * sizeof (long long);
#else
  /* 32-bit PowerPC: SImode (4-byte) limbs throughout.  */
  if (n > 32)
    return ((n - 1) / 32 + 1) * sizeof (int);
#endif
  if (n > 16)
    return sizeof (int);
  if (n > 8)
    return sizeof (short);
  else
    return sizeof (char);
}

#define CHECK_SIZE(N) \
  if (sizeof(_BitInt(N)) != calc_size(N)) \
    __builtin_abort ()

#define CHECK_USIZE(N) \
  if (sizeof(unsigned _BitInt(N)) != calc_size(N)) \
    __builtin_abort ();

int main (void)
{
  /* Small sizes */
  CHECK_USIZE(1);
  CHECK_SIZE(2);
  CHECK_SIZE(3);
  CHECK_SIZE(7);
  CHECK_SIZE(8);

  /* 9-16 bits */
  CHECK_SIZE(9);
  CHECK_SIZE(13);
  CHECK_SIZE(15);
  CHECK_SIZE(16);

  /* 17-32 bits */
  CHECK_SIZE(17);
  CHECK_SIZE(24);
  CHECK_SIZE(31);
  CHECK_SIZE(32);

  /* 33-64 bits */
  CHECK_SIZE(33);
  CHECK_SIZE(42);
  CHECK_SIZE(53);
  CHECK_SIZE(63);
  CHECK_SIZE(64);

#ifdef __powerpc64__
  /* 65-128 bits (64-bit mode only) */
  CHECK_SIZE(65);
  CHECK_SIZE(79);
  CHECK_SIZE(96);
  CHECK_SIZE(113);
  CHECK_SIZE(127);
  CHECK_SIZE(128);

  /* Large sizes (64-bit mode) */
  CHECK_SIZE(129);
  CHECK_SIZE(153);
  CHECK_SIZE(255);
  CHECK_SIZE(256);
  CHECK_SIZE(257);
  CHECK_SIZE(353);
  CHECK_SIZE(512);
  CHECK_SIZE(620);
  CHECK_SIZE(1024);
  CHECK_SIZE(2048);
#else
  /* 65-128 bits on 32-bit mode use multiple 32-bit limbs.  */
  CHECK_SIZE(65);
  CHECK_SIZE(79);
  CHECK_SIZE(96);
  CHECK_SIZE(113);
  CHECK_SIZE(127);
  CHECK_SIZE(128);
#endif

  return 0;
}
