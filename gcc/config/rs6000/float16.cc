/* Subroutines for the C front end on the PowerPC architecture.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.

   Contributed by Zack Weinberg <zack@codesourcery.com>
   and Paolo Bonzini <bonzini@gnu.org>

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

/* 16-bit floating point support.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "rtl.h"
#include "tree.h"
#include "memmodel.h"
#include "tm_p.h"
#include "stringpool.h"
#include "expmed.h"
#include "optabs.h"
#include "regs.h"
#include "insn-attr.h"
#include "flags.h"
#include "attribs.h"
#include "explow.h"
#include "expr.h"
#include "emit-rtl.h"
#include "common/common-target.h"
#include "rs6000-internal.h"

/* Expand a 16-bit vector operation:

   ICODE:   Operation to perform.
   RESULT:  Result of the operation.
   OP1:     Input operand1.
   OP2:     Input operand2.
   OP3:     Input operand3 or NULL_RTX.
   SUBTYPE: Describe the operation.  */
	
void
fp16_vectorization (enum rtx_code icode,
		    rtx result,
		    rtx op1,
		    rtx op2,
		    rtx op3,
		    enum fp16_operation subtype)
{
  gcc_assert (can_create_pseudo_p ());

  machine_mode result_mode = GET_MODE (result);
  rtx op_orig[3] = { op1, op2, op3 };
  rtx op_hi[3];
  rtx op_lo[3];
  rtx result_hi;
  rtx result_lo;
  size_t n_opts;

  switch (subtype)
    {
    case FP16_UNARY:
      n_opts = 1;
      break;

    case FP16_BINARY:
      n_opts = 2;
      break;

    case FP16_FMA:
    case FP16_FMS:
    case FP16_NFMA:
    case FP16_NFMS:
      n_opts = 3;
      break;

    default:
      gcc_unreachable ();
    }

  /* Allocate 2 temporaries for the results and the input operands.  */
  result_hi = gen_reg_rtx (V4SFmode);
  result_lo = gen_reg_rtx (V4SFmode);

  for (size_t i = 0; i < n_opts; i++)
    {
      gcc_assert (op_orig[i] != NULL_RTX);
      op_hi[i] = gen_reg_rtx (V4SFmode);	/* high register.  */
      op_lo[i] = gen_reg_rtx (V4SFmode);	/* low register.  */

      rtx interleave_hi = gen_reg_rtx (result_mode);
      rtx interleave_lo = gen_reg_rtx (result_mode);
      rtx orig = op_orig[i];

      rs6000_expand_interleave (interleave_hi, orig, orig, !BYTES_BIG_ENDIAN);
      rs6000_expand_interleave (interleave_lo, orig, orig,  BYTES_BIG_ENDIAN);

      if (result_mode == V8HFmode)
	{
	  emit_insn (gen_xvcvhpsp_v8hf (op_hi[i], interleave_hi));
	  emit_insn (gen_xvcvhpsp_v8hf (op_lo[i], interleave_lo));
	}

      else if (result_mode == V8BFmode)
	{
	  emit_insn (gen_xvcvbf16spn_v8bf (op_hi[i], interleave_hi));
	  emit_insn (gen_xvcvbf16spn_v8bf (op_lo[i], interleave_lo));
	}

      else
	gcc_unreachable ();
    }

  /* Do 2 sets of V4SFmode operations.  */
  switch (subtype)
    {
    case FP16_UNARY:
      emit_insn (gen_rtx_SET (result_hi,
			      gen_rtx_fmt_e (icode, V4SFmode,
					     op_hi[0])));

      emit_insn (gen_rtx_SET (result_lo,
			      gen_rtx_fmt_e (icode, V4SFmode,
					     op_lo[0])));
      break;

    case FP16_BINARY:
      emit_insn (gen_rtx_SET (result_hi,
			      gen_rtx_fmt_ee (icode, V4SFmode,
					      op_hi[0],
					      op_hi[1])));

      emit_insn (gen_rtx_SET (result_lo,
			      gen_rtx_fmt_ee (icode, V4SFmode,
					      op_lo[0],
					      op_lo[1])));
      break;

    case FP16_FMA:
    case FP16_FMS:
    case FP16_NFMA:
    case FP16_NFMS:
      {
	rtx op1_hi = op_hi[0];
	rtx op2_hi = op_hi[1];
	rtx op3_hi = op_hi[2];

	rtx op1_lo = op_lo[0];
	rtx op2_lo = op_lo[1];
	rtx op3_lo = op_lo[2];

	if (subtype == FP16_FMS || subtype == FP16_NFMS)
	  {
	    op3_hi = gen_rtx_NEG (V4SFmode, op3_hi);
	    op3_lo = gen_rtx_NEG (V4SFmode, op3_lo);
	  }

	rtx op_fma_hi = gen_rtx_FMA (V4SFmode, op1_hi, op2_hi, op3_hi);
	rtx op_fma_lo = gen_rtx_FMA (V4SFmode, op1_lo, op2_lo, op3_lo);

	if (subtype == FP16_NFMA || subtype == FP16_NFMS)
	  {
	    op_fma_hi = gen_rtx_NEG (V4SFmode, op_fma_hi);
	    op_fma_lo = gen_rtx_NEG (V4SFmode, op_fma_lo);
	  }

	emit_insn (gen_rtx_SET (result_hi, op_fma_hi));
	emit_insn (gen_rtx_SET (result_lo, op_fma_lo));
      }
      break;

    default:
      gcc_unreachable ();
    }

  /* Combine the 2 V4SFmode operations into one V8HFmode/V8BFmode vector.  */
  if (result_mode == V8HFmode)
    emit_insn (gen_vec_pack_trunc_v4sf_v8hf (result, result_hi, result_lo));

  else if (result_mode == V8BFmode)
    emit_insn (gen_vec_pack_trunc_v4sf_v8bf (result, result_hi, result_lo));

  else
    gcc_unreachable ();

  return;
}

/* Expand a bfloat16 scalar floating point operation:

   ICODE:   Operation to perform.
   OP1:     Input operand1.
   OP2:     Input operand2.
   OP3:     Input operand3 or NULL_RTX.
   SUBTYPE: Describe the operation.

   The operation is done as a V4SFmode vector operation.  This is because
   converting BFmode from a scalar BFmode to SFmode to do the operation and
   back again takes quite a bit of time.  GCC will only generate the native
   operation if -Ofast is used.  The float16.md code that calls this function
   adds various combine operations to do the operation in V4SFmode instead of
   SFmode.

   Inner part that does the operation in V4SFmode, and returns the pseudo
   with the results.  */

static rtx
bfloat16_operation_as_v4sf_inner (enum rtx_code icode,
				  rtx op1,
				  rtx op2,
				  rtx op3,
				  enum fp16_operation subtype)
{
  gcc_assert (can_create_pseudo_p ());

  rtx result_v4sf = gen_reg_rtx (V4SFmode);
  rtx ops_orig[3] = { op1, op2, op3 };
  rtx ops_v4sf[3];
  size_t n_opts;

  switch (subtype)
    {
    case FP16_UNARY:
      n_opts = 1;
      gcc_assert (op2 == NULL_RTX && op3 == NULL_RTX);
      break;

    case FP16_BINARY:
      n_opts = 2;
      gcc_assert (op3 == NULL_RTX);
      break;

    case FP16_FMA:
    case FP16_FMS:
    case FP16_NFMA:
    case FP16_NFMS:
      gcc_assert (icode == FMA);
      n_opts = 3;
      break;

    default:
      gcc_unreachable ();
    }

  for (size_t i = 0; i < n_opts; i++)
    {
      rtx op = ops_orig[i];

      gcc_assert (op != NULL_RTX);

      switch (GET_CODE (op))
	{
	case NEG:
	case ABS:
	  ops_v4sf[i] = bfloat16_operation_as_v4sf_inner (GET_CODE (op),
							  XEXP (op, 0),
							  NULL_RTX,
							  NULL_RTX,
							  FP16_UNARY);
	  continue;

	  /* Handle chained operations.  */
	case PLUS:
	case MINUS:
	case MULT:
	  ops_v4sf[i] = bfloat16_operation_as_v4sf_inner (GET_CODE (op),
							  XEXP (op, 0),
							  XEXP (op, 1),
							  NULL_RTX,
							  FP16_BINARY);
	  continue;

	  /* Remove truncation/extend added.  */
	case FLOAT_EXTEND:
	case FLOAT_TRUNCATE:
	  op = XEXP (op, 0);
	  break;

	default:
	  break;
	}

      /* Convert operands to V4SFmode format.  We use SPLAT for registers to
	 get the value into the upper 32-bits.  We can use XXSPLTW to splat
	 words instead of VSPLTIH since the XVCVBF16SPN instruction ignores the
	 odd half-words, and XXSPLTW can operate on all VSX registers instead
	 of just the Altivec registers.  Using SPLAT instead of a shift also
	 insure that other bits are not a signalling NaN.  If we are using
	 XXSPLTIW or XXSPLTIB to load the constant the other bits are
	 duplicated.  */

      rtx tmp = ops_v4sf[i] = gen_reg_rtx (V4SFmode);
      if (op == CONST0_RTX (SFmode) || op == CONST0_RTX (BFmode))
	{
	  rtx zero = CONST0_RTX (V4SFmode);
	  rtx insn = emit_insn (gen_rtx_SET (tmp, zero));
	  set_unique_reg_note (insn, REG_EQUIV, zero);
	}

      else if (CONST_DOUBLE_P (op))
	{
	  rtx sf_op;

	  if (GET_MODE (op) == SFmode)
	    sf_op = op;
	  else
	    {
	      const REAL_VALUE_TYPE *rtype = CONST_DOUBLE_REAL_VALUE (op);
	      sf_op = const_double_from_real_value (*rtype, SFmode);
	    }

	  rtvec v = gen_rtvec (4, sf_op, sf_op, sf_op, sf_op);
	  rtx cv = gen_rtx_CONST_VECTOR (V4SFmode, v);
	  rtx insn = emit_insn (gen_rtx_SET (tmp, cv));
	  set_unique_reg_note (insn, REG_EQUIV, cv);
	}

      else if (GET_MODE (op) == BFmode)
	{
	  rtx tmp2 = gen_reg_rtx (V4SFmode);
	  emit_insn (gen_xxspltw_bf (tmp2, force_reg (BFmode, op)));
	  emit_insn (gen_xvcvbf16spn_bf (tmp, tmp2));
	}

      else if (GET_MODE (op) == SFmode)
	emit_insn (gen_vsx_splat_v4sf (tmp, force_reg (SFmode, op)));

      else
	gcc_unreachable ();
    }

  /* Do the operation in V4SFmode.  */
  switch (subtype)
    {
    case FP16_UNARY:
      emit_insn (gen_rtx_SET (result_v4sf,
			      gen_rtx_fmt_e (icode, V4SFmode,
					     ops_v4sf[0])));

      break;

    case FP16_BINARY:
      emit_insn (gen_rtx_SET (result_v4sf,
			      gen_rtx_fmt_ee (icode, V4SFmode,
					      ops_v4sf[0],
					      ops_v4sf[1])));
      break;

    case FP16_FMA:
    case FP16_FMS:
    case FP16_NFMA:
    case FP16_NFMS:
      {
	rtx op1 = ops_v4sf[0];
	rtx op2 = ops_v4sf[1];
	rtx op3 = ops_v4sf[2];

	if (subtype == FP16_FMS || subtype == FP16_NFMS)
	  op3 = gen_rtx_NEG (V4SFmode, op3);

	rtx op_fma = gen_rtx_FMA (V4SFmode, op1, op2, op3);

	if (subtype == FP16_NFMA || subtype == FP16_NFMS)
	  op_fma = gen_rtx_NEG (V4SFmode, op_fma);

	emit_insn (gen_rtx_SET (result_v4sf, op_fma));
      }
      break;

    default:
      gcc_unreachable ();
    }

  return result_v4sf;
}

/* Expand a bfloat16 scalar floating point operation:

   ICODE:   Operation to perform.
   RESULT:  Result of the operation.
   OP1:     Input operand1.
   OP2:     Input operand2.
   OP3:     Input operand3 or NULL_RTX.
   SUBTYPE: Describe the operation.

   The operation is done as a V4SFmode vector operation.  This is because
   converting BFmode from a scalar BFmode to SFmode to do the operation and
   back again takes quite a bit of time.  GCC will only generate the native
   operation if -Ofast is used.  The float16.md code that calls this function
   adds various combine operations to do the operation in V4SFmode instead of
   SFmode.

   External part that calls bfloat16_operation_as_v4sf_inner to do the
   calculation in V4SFmode, and then converts the result to SFmode or
   BFmode.  */

void
bfloat16_operation_as_v4sf (enum rtx_code icode,
			    rtx result,
			    rtx op1,
			    rtx op2,
			    rtx op3,
			    enum fp16_operation subtype)
{
  rtx result_v4sf
    = bfloat16_operation_as_v4sf_inner (icode, op1, op2, op3, subtype);

  /* Convert V4SF result back to scalar mode.  */
  if (GET_MODE (result) == BFmode)
    emit_insn (gen_xvcvspbf16_bf (result, result_v4sf));

  else if (GET_MODE (result) == SFmode)
    {
      rtx element = GEN_INT (WORDS_BIG_ENDIAN ? 2 : 3);
      emit_insn (gen_vsx_extract_v4sf (result, result_v4sf, element));
    }

  else
    gcc_unreachable ();
}


/*
 * Recognize if can BFmode operations in V4SFmode.  If the operation mostly
 * involves BFmode values with some SFmode values. it is faster to convert the
 * scalar code to vector and do the operation using one element of the vector,
 * due to the lack of instructions to convert between BFmode and SFmode/DFmode
 * scalars on power10.
 *
 * We allow multiple operations (a + b + c) to be folded together to try to do
 * as much possible in the vector unit without having to convert back to scalar
 * mode.
 *
 * We also fold in loads from memory to simplify the conversion.
 *
 * The expression that is combined is split before register allocation.
 */

/*
 * Stats for whole expression.
 */

struct bfloat16_to_v4sf_stats
{
  unsigned num_bf_regs;		// # of BF registers used
  unsigned num_bf_loads;		// # of BF values loaded from memory
  unsigned num_sf_regs;		// # of SF registers used
  unsigned num_const;		// # of constants used
  unsigned num_binary;		// # of binary operations done
  unsigned num_fma;		// # of FMA (fused multiply-adds) done
  unsigned num_unary;		// # of unary ops not folded into FMA/ABS
  unsigned num_neg_folded;	// # of negates folded into a FMA/ABS insn
  unsigned num_extend;		// # of FLOAT_EXTENDs
};

/* Initialize the bfloat16_to_v4sf_stats structure.  */
static struct bfloat16_to_v4sf_stats	zero_bfloat16_to_v4sf_stats;

/*
 * Recursively evaluate whether a RTX expression can be optimized to use
 * V4SFmode operations.  Return true if the expression might be able to be
 * optimized, and update the stats.
 */

static bool
bfloat16_to_v4sf_inner (rtx op, struct bfloat16_to_v4sf_stats *stats)
{
  machine_mode mode = GET_MODE (op);
  rtx arg;
  rtx_code code;
  bool ret = true;
  bool update_stats = false;

#ifdef TARGET_DEBUG_EXTRA1
  if (TARGET_DEBUG_EXTRA1)
    update_stats = true;
#endif

  if (mode != BFmode && mode != SFmode)
    ret = false;

  else
    {
      switch (GET_CODE (op))
	{
	default:
	  ret = false;
	  break;

	  // Update register counts.
	case REG:
	case SUBREG:
	  if (mode == BFmode)
	    stats->num_bf_regs++;
	  else
	    stats->num_sf_regs++;

	  break;

	  // Update memory counts.
	case MEM:
	  if (mode != BFmode)
	    ret = false;
	  else
	    stats->num_bf_loads++;

	  break;

	  // Update constants
	case CONST_DOUBLE:
	  stats->num_const++;
	  break;

	  // Process absolute value
	case ABS:
	  stats->num_unary++;
	  if (!bfloat16_to_v4sf_inner (XEXP (op, 0), stats))
	    ret = false;

	  break;

	  // Process negation, deal with negate being folded into FMA or ABS.
	case NEG:
	  arg = XEXP (op, 0);
	  code = GET_CODE (arg);

	  if (code == FMA || code == ABS)
	    stats->num_neg_folded++;
	  else
	    stats->num_unary++;

	  if (!bfloat16_to_v4sf_inner (arg, stats))
	    ret = false;

	  break;

	  // Process binary operations
	case PLUS:
	case MINUS:
	case MULT:
	case SMIN:
	case SMAX:
	  stats->num_binary++;

	  if (update_stats)
	    {
	      bool ret1 = bfloat16_to_v4sf_inner (XEXP (op, 0), stats);
	      bool ret2 = bfloat16_to_v4sf_inner (XEXP (op, 1), stats);
	      if (!ret1 || !ret2)
		ret = false;
	    }
	  else if (!bfloat16_to_v4sf_inner (XEXP (op, 0), stats)
		   || !bfloat16_to_v4sf_inner (XEXP (op, 1), stats))
	    ret = false;

	  break;

	  // Process fused multiply-adds
	case FMA:
	  stats->num_fma++;

	  if (update_stats)
	    {
	      bool ret1 = bfloat16_to_v4sf_inner (XEXP (op, 0), stats);
	      bool ret2 = bfloat16_to_v4sf_inner (XEXP (op, 1), stats);
	      bool ret3 = bfloat16_to_v4sf_inner (XEXP (op, 2), stats);
	      if (!ret1 || !ret2 || !ret3)
		ret = false;
	    }
	  else
	    if (!bfloat16_to_v4sf_inner (XEXP (op, 0), stats)
		|| !bfloat16_to_v4sf_inner (XEXP (op, 1), stats)
		|| !bfloat16_to_v4sf_inner (XEXP (op, 2), stats))
	      ret = false;

	  break;

	  // Process extend BF -> SF
	case FLOAT_EXTEND:
	  stats->num_extend++;
	  arg = XEXP (op, 0);
	  if (mode != SFmode || GET_MODE (arg) != BFmode)
	    ret = false;;

	  if (!bfloat16_to_v4sf_inner (arg, stats))
	    ret = false;
	}
    }

  return ret;
}

/*
 * Decide whether an expression can optimize BFmode to use V4SFmode.
 */

bool
bfloat16_to_v4sf_p (rtx op, machine_mode mode)
{
  if (mode != BFmode && mode != SFmode)
    return false;

#ifdef TARGET_DEBUG_EXTRA1
  rtx orig_op = op;
#endif

  struct bfloat16_to_v4sf_stats stats = zero_bfloat16_to_v4sf_stats;

  // Handle top level FLOAT_TRUNCATE
  if (GET_CODE (op) == FLOAT_TRUNCATE
      && mode == BFmode
      && GET_MODE (XEXP (op, 0)) == SFmode)
    op = XEXP (op, 0);

  bool ret = bfloat16_to_v4sf_inner (op, &stats);

  // Make sure we had enough BFmode operations to do the optimization
  if (ret)
    {
      unsigned num_bf = stats.num_bf_regs + stats.num_bf_loads;

      // If we had no BFmode values, don't do the optimization.
      if (!num_bf)
	ret = false;

      // If we had no binary or fma opertors, don't do the optimization.
      else if (!stats.num_binary && !stats.num_fma)
	ret = false;

      // If the expression has much more SFmode elements than BFmode, prefer
      // converting BFmode to SFmode instead of using V4SFmode.
      else if (stats.num_sf_regs > (num_bf + 2))
	ret = false;
    }

  // Debugging.
#ifdef TARGET_DEBUG_EXTRA1
  if (TARGET_DEBUG_EXTRA1)
    {
      fprintf (stderr, "\n========== bfloat16_to_v4sf_p, return = %s",
	       ret ? "true" : "false");

      if (stats.num_bf_regs)
	fprintf (stderr, ", # BF regs = %u", stats.num_bf_regs);

      if (stats.num_bf_loads)
	fprintf (stderr, ", # BF values loaded = %u", stats.num_bf_loads);

      if (stats.num_sf_regs)
	fprintf (stderr, ", # SF regs = %u", stats.num_sf_regs);

      if (stats.num_const)
	fprintf (stderr, ", # constants = %u", stats.num_const);

      if (stats.num_extend)
	fprintf (stderr, ", # extends = %u", stats.num_extend);

      if (stats.num_unary)
	fprintf (stderr, ", # unary = %u", stats.num_unary);

      if (stats.num_binary)
	fprintf (stderr, ", # binary = %u", stats.num_binary);

      if (stats.num_fma)
	fprintf (stderr, ", # fma = %u", stats.num_fma);

      if (stats.num_neg_folded)
	fprintf (stderr, ", # neg fold  = %u", stats.num_neg_folded);

      fprintf (stderr, "\n");
      debug_rtx (orig_op);
    }
#endif

  return ret;
}

/*
 * Split the isnsns tht optimize BFmode to use V4SFmode.
 */

void
bfloat16_to_v4sf_split (rtx dest, rtx src)
{
  rtx orig_src = src;
  enum rtx_code code = GET_CODE (src);
  rtx arg, op0, op1, op2;

#ifdef TARGET_DEBUG_EXTRA2
  if (TARGET_DEBUG_EXTRA2)
    debug_rtx (gen_rtx_SET (dest, src));
#endif

  if (code == FLOAT_EXTEND || code == FLOAT_TRUNCATE)
    {
      src = XEXP (src, 0);
      code = GET_CODE (src);
    }

  switch (code)
    {
    case ABS:
      bfloat16_operation_as_v4sf (code, dest,
				  XEXP (src, 0),
				  NULL_RTX,
				  NULL_RTX,
				  FP16_UNARY);
      break;

    case NEG:
      arg = XEXP (src, 0);
      if (GET_CODE (arg) == FMA)
	{
	  op0 = XEXP (arg, 0);
	  op1 = XEXP (arg, 1);
	  op2 = XEXP (arg, 2);
	  if (GET_CODE (op2) == NEG)
	    {
	      op2 = XEXP (op2, 0);
	      bfloat16_operation_as_v4sf (FMA, dest,
					  op0,
					  op1,
					  op2,
					  FP16_NFMS);
	    }

	  else
	    bfloat16_operation_as_v4sf (FMA, dest,
					op0,
					op1,
					op2,
					FP16_NFMA);
	}

      else
	bfloat16_operation_as_v4sf (GET_CODE (arg), dest,
				    XEXP (arg, 0),
				    NULL_RTX,
				    NULL_RTX,
				    FP16_UNARY);

      break;

    case PLUS:
    case MINUS:
    case MULT:
    case SMIN:
    case SMAX:
      bfloat16_operation_as_v4sf (code, dest,
				  XEXP (src, 0),
				  XEXP (src, 1),
				  NULL_RTX,
				  FP16_BINARY);
      break;

    case FMA:
      op0 = XEXP (src, 0);
      op1 = XEXP (src, 1);
      op2 = XEXP (src, 2);
      if (GET_CODE (op2) == NEG)
	bfloat16_operation_as_v4sf (FMA, dest,
				    op0,
				    op1,
				    XEXP (op2, 0),
				    FP16_FMS);

      else
	bfloat16_operation_as_v4sf (FMA, dest,
				    op0,
				    op1,
				    op2,
				    FP16_FMA);

      break;

    default:
      fatal_insn_not_found (gen_rtx_SET (dest, orig_src));
    }

  return;
}
