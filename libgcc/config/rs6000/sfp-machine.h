/* Decide whether to use 64 or 32-bit types to do the emulation.  If we are
   doing IEEE-128 with VSX, use 64-bit emulation even if we are compiling for a
   32-bit target.  */

#if defined(_ARCH_PPC64) || defined(__VSX__) || defined(__FLOAT128__)
#define _FP_W_TYPE_SIZE		64
#define _FP_W_TYPE		unsigned long long
#define _FP_WS_TYPE		signed long long
#define _FP_I_TYPE		long long

#ifdef _ARCH_PPC64
typedef int TItype __attribute__ ((mode (TI)));
typedef unsigned int UTItype __attribute__ ((mode (TI)));

#define TI_BITS (__CHAR_BIT__ * (int)sizeof(TItype))
#endif

#else	/* 32-bits  */
#define _FP_W_TYPE_SIZE		32
#define _FP_W_TYPE		unsigned int
#define _FP_WS_TYPE		signed int
#define _FP_I_TYPE		int
#endif	/* 32-bits  */

#define _FP_NANFRAC_H           _FP_QNANBIT_H
#define _FP_NANFRAC_B           _FP_QNANBIT_B

/* The type of the result of a floating point comparison.  This must
   match `__libgcc_cmp_return__' in GCC for the target.  */
typedef int __gcc_CMPtype __attribute__ ((mode (__libgcc_cmp_return__)));
#define CMPtype __gcc_CMPtype

#define _FP_MUL_MEAT_S(R,X,Y)				\
  _FP_MUL_MEAT_1_wide(_FP_WFRACBITS_S,R,X,Y,umul_ppmm)

#if (_FP_W_TYPE_SIZE==64)
#define _FP_MUL_MEAT_D(R,X,Y)				\
  _FP_MUL_MEAT_1_wide(_FP_WFRACBITS_D,R,X,Y,umul_ppmm)
#define _FP_MUL_MEAT_Q(R,X,Y)				\
  _FP_MUL_MEAT_2_wide(_FP_WFRACBITS_Q,R,X,Y,umul_ppmm)
#else
#define _FP_MUL_MEAT_D(R,X,Y)				\
  _FP_MUL_MEAT_2_wide(_FP_WFRACBITS_D,R,X,Y,umul_ppmm)
#define _FP_MUL_MEAT_Q(R,X,Y)				\
  _FP_MUL_MEAT_4_wide(_FP_WFRACBITS_Q,R,X,Y,umul_ppmm)
#endif

#define _FP_DIV_MEAT_S(R,X,Y)	_FP_DIV_MEAT_1_loop(S,R,X,Y)

#if (_FP_W_TYPE_SIZE==64)
#define _FP_DIV_MEAT_D(R,X,Y)	_FP_DIV_MEAT_1_udiv(D,R,X,Y)
#define _FP_DIV_MEAT_Q(R,X,Y)   _FP_DIV_MEAT_2_udiv(Q,R,X,Y)
#else
#define _FP_DIV_MEAT_D(R,X,Y)	_FP_DIV_MEAT_2_udiv(D,R,X,Y)
#define _FP_DIV_MEAT_Q(R,X,Y)	_FP_DIV_MEAT_4_udiv(Q,R,X,Y)
#endif

#define _FP_NANFRAC_S		((_FP_QNANBIT_S << 1) - 1)

#if (_FP_W_TYPE_SIZE==64)
#define _FP_NANFRAC_D		((_FP_QNANBIT_D << 1) - 1)
#define _FP_NANFRAC_Q		((_FP_QNANBIT_Q << 1) - 1), -1
#else
#define _FP_NANFRAC_D		((_FP_QNANBIT_D << 1) - 1), -1
#define _FP_NANFRAC_Q		((_FP_QNANBIT_Q << 1) - 1), -1, -1, -1
#endif

#define _FP_NANSIGN_H		1
#define _FP_NANSIGN_B		1
#define _FP_NANSIGN_S		0
#define _FP_NANSIGN_D		0
#define _FP_NANSIGN_Q		0

#define _FP_KEEPNANFRACP 1
#define _FP_QNANNEGATEDP 0

/* Someone please check this.  */
#define _FP_CHOOSENAN(fs, wc, R, X, Y, OP)			\
  do {								\
    if ((_FP_FRAC_HIGH_RAW_##fs(X) & _FP_QNANBIT_##fs)		\
	&& !(_FP_FRAC_HIGH_RAW_##fs(Y) & _FP_QNANBIT_##fs))	\
      {								\
	R##_s = Y##_s;						\
	_FP_FRAC_COPY_##wc(R,Y);				\
      }								\
    else							\
      {								\
	R##_s = X##_s;						\
	_FP_FRAC_COPY_##wc(R,X);				\
      }								\
    R##_c = FP_CLS_NAN;						\
  } while (0)

#define _FP_TININESS_AFTER_ROUNDING 0

#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321

#if defined __BIG_ENDIAN__ || defined _BIG_ENDIAN
# if defined __LITTLE_ENDIAN__ || defined _LITTLE_ENDIAN
#  error "Both BIG_ENDIAN and LITTLE_ENDIAN defined!"
# endif
# define __BYTE_ORDER __BIG_ENDIAN
#else
# if defined __LITTLE_ENDIAN__ || defined _LITTLE_ENDIAN
#  define __BYTE_ORDER __LITTLE_ENDIAN
# else
#  error "Cannot determine current byte order"
# endif
#endif

/* Only provide exception support if we have hardware floating point using
   floating point registers and we can execute the mtfsf instruction.  This
   would only be true if we are using the emulation routines for IEEE 128-bit
   floating point on pre-ISA 3.0 machines without the IEEE 128-bit floating
   point support.  */

#ifdef __FLOAT128__
#define ISA_BIT(x) (1LL << (63 - x))

/* Use the same bits of the FPSCR.  */
# define FP_EX_INVALID		ISA_BIT(34)
# define FP_EX_OVERFLOW		ISA_BIT(35)
# define FP_EX_UNDERFLOW	ISA_BIT(36)
# define FP_EX_DIVZERO		ISA_BIT(37)
# define FP_EX_INEXACT		ISA_BIT(38)
# define FP_EX_ALL		(FP_EX_INVALID | FP_EX_OVERFLOW 	\
				 | FP_EX_UNDERFLOW | FP_EX_DIVZERO	\
				 | FP_EX_INEXACT)

void __sfp_handle_exceptions (int);

# define FP_HANDLE_EXCEPTIONS			\
  do {						\
    if (__builtin_expect (_fex, 0))		\
      __sfp_handle_exceptions (_fex);		\
  } while (0)

/* The FP_EX_* bits track whether the exception has occurred.  This macro
   must set the FP_EX_* bits of those exceptions which are configured to
   trap.  The FPSCR bit which indicates this is 22 ISA bits above the
   respective FP_EX_* bit.  Note, the ISA labels bits from msb to lsb,
   so 22 ISA bits above is 22 bits below when counted from the lsb.  */
# define FP_TRAPPING_EXCEPTIONS ((_fpscr.i << 22) & FP_EX_ALL)

# define FP_RND_NEAREST	0x0
# define FP_RND_ZERO	0x1
# define FP_RND_PINF	0x2
# define FP_RND_MINF	0x3
# define FP_RND_MASK	0x3

# define _FP_DECL_EX \
  union { unsigned long long i; double d; } _fpscr __attribute__ ((unused)) = \
        { .i = FP_RND_NEAREST }

#define FP_INIT_ROUNDMODE			\
  do {						\
    __asm__ __volatile__ ("mffs %0"		\
			  : "=f" (_fpscr.d));	\
  } while (0)

# define FP_ROUNDMODE	(_fpscr.i & FP_RND_MASK)
#endif	/* !__FLOAT128__ */

/* Define ALIASNAME as a strong alias for NAME.  */
# define strong_alias(name, aliasname) _strong_alias(name, aliasname)
# define _strong_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)));

/* Add prototypes for the HFmode and BFmode functions.  */
typedef double DFtype2;
typedef float SFtype2;
typedef int HItype2 __attribute__ ((mode (HI)));
typedef unsigned int UHItype2 __attribute__ ((mode (HI)));
typedef int SItype2 __attribute__ ((mode (SI)));
typedef unsigned int USItype2 __attribute__ ((mode (SI)));
typedef int DItype2 __attribute__ ((mode (DI)));
typedef unsigned int UDItype2 __attribute__ ((mode (DI)));
typedef int TItype2 __attribute__ ((mode (TI)));
typedef unsigned int UTItype2 __attribute__ ((mode (TI)));

/* We have 2 different conversion functions.  One passes or returns the 16-bit
   floating point type in a floating point register, and the other passes or
   returns it in a general purpose register.  If 16-bit floating point is
   normally converted to the 32/64-bit scalar represention, we need a function
   that does not do this conversion.  */
#define PASS_FP16(NAME, RET, FP16)					\
extern RET NAME (FP16);							\
extern RET NAME ## _gpr (UHItype2)

#define PASS_FP16_2(NAME, RET, FP16_1, FP16_2)				\
extern RET NAME (FP16_1, FP16_2);					\
extern RET NAME ## _gpr (UHItype2, UHItype2)

#define RETURN_FP16(NAME, FP16, ARG)					\
extern FP16 NAME (ARG);							\
extern UHItype2 NAME ## _gpr (ARG)

#define CONVERT_FP16(NAME, FP16_RET, FP16_ARG)				\
extern FP16_RET NAME (FP16_ARG);					\
extern UHItype2 NAME ## _gpr (UHItype2)

#ifdef __FLOAT16__
typedef float HFtype2 __attribute__ ((mode (HF)));

/* Move between HFtype and unsigned short.  */
union ushort_hf {
  unsigned short us;
  HFtype2 hf;
};

PASS_FP16_2 (__eqhf2, CMPtype, HFtype2, HFtype2);

PASS_FP16 (__extendhfsf2, SFtype2, HFtype2);
PASS_FP16 (__extendhfdf2, DFtype2, HFtype2);

PASS_FP16 (__fixhfsi, SItype2, HFtype2);
PASS_FP16 (__fixhfdi, DItype2, HFtype2);
PASS_FP16 (__fixhfti, TItype2, HFtype2);

PASS_FP16 (__fixunshfsi, USItype2, HFtype2);
PASS_FP16 (__fixunshfdi, UDItype2, HFtype2);
PASS_FP16 (__fixunshfti, UTItype2, HFtype2);

RETURN_FP16 (__floatsihf, HFtype2, SItype2);
RETURN_FP16 (__floatdihf, HFtype2, DItype2);
RETURN_FP16 (__floattihf, HFtype2, TItype2);

RETURN_FP16 (__floatunsihf, HFtype2, USItype2);
RETURN_FP16 (__floatundihf, HFtype2, UDItype2);
RETURN_FP16 (__floatuntihf, HFtype2, UTItype2);

RETURN_FP16 (__truncsfhf2, HFtype2, SFtype2);
RETURN_FP16 (__truncdfhf2, HFtype2, DFtype2);

extern HFtype2 powf16 (HFtype2, HFtype2);
extern UHItype2 powf16_gpr (UHItype2, UHItype2);

extern HFtype2 __powihf2 (HFtype2, SItype2);
extern UHItype2 __powihf2_gpr (UHItype2, SItype2);
#endif

#ifdef __BFLOAT16__
typedef float BFtype2 __attribute__ ((mode (BF)));

/* Move between BFtype and unsigned short.  */
union ushort_bf {
  unsigned short us;
  BFtype2 bf;
};

PASS_FP16_2 (__eqbf2, CMPtype, BFtype2, BFtype2);

PASS_FP16 (__extendbfsf2, SFtype2, BFtype2);
PASS_FP16 (__extendbfdf2, DFtype2, BFtype2);

PASS_FP16 (__fixbfsi, SItype2, BFtype2);
PASS_FP16 (__fixbfdi, DItype2, BFtype2);
PASS_FP16 (__fixbfti, TItype2, BFtype2);

PASS_FP16 (__fixunsbfsi, USItype2, BFtype2);
PASS_FP16 (__fixunsbfdi, UDItype2, BFtype2);
PASS_FP16 (__fixunsbfti, UTItype2, BFtype2);

RETURN_FP16 (__floatsibf, BFtype2, SItype2);
RETURN_FP16 (__floatdibf, BFtype2, DItype2);
RETURN_FP16 (__floattibf, BFtype2, TItype2);

RETURN_FP16 (__floatunsibf, BFtype2, USItype2);
RETURN_FP16 (__floatundibf, BFtype2, UDItype2);
RETURN_FP16 (__floatuntibf, BFtype2, UTItype2);

RETURN_FP16 (__truncsfbf2, BFtype2, SFtype2);
RETURN_FP16 (__truncdfbf2, BFtype2, DFtype2);

extern BFtype2 powbf16 (BFtype2, BFtype2);
extern UHItype2 powbf16_gpr (UHItype2, UHItype2);

extern BFtype2 __powibf2 (BFtype2, SItype2);
extern UHItype2 __powibf2_gpr (UHItype2, SItype2);
#endif

#if defined(__FLOAT16__) && defined(__BFLOAT16__)
CONVERT_FP16 (__truncbfhf2, HFtype2, BFtype2);
CONVERT_FP16 (__trunchfbf2, BFtype2, HFtype2);
#endif
