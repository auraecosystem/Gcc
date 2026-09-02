/* { dg-do run { target bitint } } */
/* { dg-additional-options "-std=c23 -O0" } */

/* Test _BitInt endianness handling for PowerPC.
   This test verifies that _BitInt works correctly on both
   big-endian and little-endian PowerPC systems. */

#include <string.h>

static void
check_bytes (const unsigned char *bytes, const unsigned char *expected, int n)
{
  for (int i = 0; i < n; ++i)
    if (bytes[i] != expected[i])
      __builtin_abort ();
}

int main(void)
{
  /* Verify actual multi-limb byte layout.  */
  {
    _BitInt(96) val = 0x010203040506070809101112wb;
    unsigned char bytes[12];

    memcpy (bytes, &val, 12);

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    if (bytes[0] != 0x01 || bytes[1] != 0x02 || bytes[10] != 0x11
        || bytes[11] != 0x12)
      __builtin_abort ();
#else
    if (bytes[0] != 0x12 || bytes[1] != 0x11 || bytes[10] != 0x02
        || bytes[11] != 0x01)
      __builtin_abort ();
#endif
  }

  /* Verify byte layout across a non-power-of-two multi-limb width.  */
  {
    unsigned _BitInt(65) val
      = ((unsigned _BitInt(65)) 0x01u << 64)
      | (unsigned _BitInt(65)) 0x02030405u;
    unsigned char bytes[16] = { 0 };

    memcpy (bytes, &val, sizeof (val));

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    if (bytes[7] != 0x01 || bytes[8] != 0x00 || bytes[12] != 0x02
        || bytes[13] != 0x03 || bytes[14] != 0x04 || bytes[15] != 0x05)
      __builtin_abort ();
#else
    if (bytes[0] != 0x05 || bytes[1] != 0x04 || bytes[2] != 0x03
        || bytes[3] != 0x02 || bytes[8] != 0x01)
      __builtin_abort ();
#endif
  }

  /* Also verify limb ordering across a larger multi-limb object.  */
  {
    unsigned _BitInt(160) value
      = ((unsigned _BitInt(160)) 0x01020304u << 128)
      | ((unsigned _BitInt(160)) 0x11121314u << 96)
      | ((unsigned _BitInt(160)) 0x21222324u << 64)
      | ((unsigned _BitInt(160)) 0x31323334u << 32)
      | (unsigned _BitInt(160)) 0x41424344u;
    const unsigned char *bytes = (const unsigned char *) &value;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    const unsigned char expected[20]
      = { 0x44, 0x43, 0x42, 0x41, 0x34, 0x33, 0x32, 0x31,
          0x24, 0x23, 0x22, 0x21, 0x14, 0x13, 0x12, 0x11,
          0x04, 0x03, 0x02, 0x01 };
#else
    const unsigned char expected[20]
      = { 0x01, 0x02, 0x03, 0x04, 0x11, 0x12, 0x13, 0x14,
          0x21, 0x22, 0x23, 0x24, 0x31, 0x32, 0x33, 0x34,
          0x41, 0x42, 0x43, 0x44 };
#endif
    check_bytes (bytes, expected, 20);
  }

  /* Test that arithmetic works correctly regardless of endianness */
  {
    _BitInt(64) a = 0x123456789ABCDEF0LL;
    _BitInt(64) b = 0x0FEDCBA987654321LL;
    _BitInt(64) sum = a + b;

    /* The sum should be correct regardless of byte order */
    if (sum != 0x2222222222222211LL)
      __builtin_abort();
  }

#ifdef __powerpc64__
  /* Test 128-bit values on 64-bit systems */
  {
    _BitInt(128) value = 1;

    /* Shift left by 100 bits */
    value = value << 100;

    /* Shift right by 100 bits */
    value = value >> 100;

    /* Should get back to 1 */
    if (value != 1)
      __builtin_abort();
  }
#endif

  /* Test that comparisons work correctly */
  {
    _BitInt(32) a = 100;
    _BitInt(32) b = 200;

    if (!(a < b))
      __builtin_abort();
    if (!(b > a))
      __builtin_abort();
    if (a >= b)
      __builtin_abort();
    if (b <= a)
      __builtin_abort();
  }

  return 0;
}

/* Test passes if endianness handling is correct */
