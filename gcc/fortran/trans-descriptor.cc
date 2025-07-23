/* Copyright (C) 2002-2025 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */


#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "fold-const.h"
#include "gfortran.h"
#include "trans.h"
#include "trans-const.h"
#include "trans-types.h"
#include "trans-array.h"
#include "trans-descriptor.h"


/* Array descriptor low level access routines.
 ******************************************************************************/

/* Build expressions to access the members of an array descriptor.
   It's surprisingly easy to mess up here, so never access
   an array descriptor by "brute force", always use these
   functions.  This also avoids problems if we change the format
   of an array descriptor.

   To understand these magic numbers, look at the comments
   before gfc_build_array_type() in trans-types.cc.

   The code within these defines should be the only code which knows the format
   of an array descriptor.

   Any code just needing to read obtain the bounds of an array should use
   gfc_conv_array_* rather than the following functions as these will return
   know constant values, and work with arrays which do not have descriptors.

   Don't forget to #undef these!  */

#define DATA_FIELD 0
#define OFFSET_FIELD 1
#define DTYPE_FIELD 2
#define SPAN_FIELD 3
#define DIMENSION_FIELD 4
#define CAF_TOKEN_FIELD 5

#define STRIDE_SUBFIELD 0
#define LBOUND_SUBFIELD 1
#define UBOUND_SUBFIELD 2

#define GFC_DTYPE_ELEM_LEN 0
#define GFC_DTYPE_VERSION 1
#define GFC_DTYPE_RANK 2
#define GFC_DTYPE_TYPE 3
#define GFC_DTYPE_ATTRIBUTE 4


/* Get FIELD_IDX'th field in struct TYPE.  */

static tree
get_type_field (tree type, unsigned field_idx)
{
  tree field = gfc_advance_chain (TYPE_FIELDS (type), field_idx);
  gcc_assert (field != NULL_TREE);

  return field;
}


/* Return a reference to the FIELD_IDX-th field of the descriptor DESC.  */

static tree
gfc_get_descriptor_field (tree desc, unsigned field_idx)
{
  tree type = TREE_TYPE (desc);
  gcc_assert (GFC_DESCRIPTOR_TYPE_P (type));

  tree field = get_type_field (type, field_idx);

  return fold_build3_loc (input_location, COMPONENT_REF, TREE_TYPE (field),
			  desc, field, NULL_TREE);
}


/* Return a reference to the data field of the array descriptor DESC.  */

static tree
conv_descriptor_data (tree desc)
{
  return gfc_get_descriptor_field (desc, DATA_FIELD);
}

/* This provides READ-ONLY access to the data field.  The field itself
   doesn't have the proper type.  */

tree
gfc_conv_descriptor_data_get (tree desc)
{
  tree type = TREE_TYPE (desc);
  if (TREE_CODE (type) == REFERENCE_TYPE)
    gcc_unreachable ();

  tree data = conv_descriptor_data (desc);
  return fold_convert (GFC_TYPE_ARRAY_DATAPTR_TYPE (type), data);
}

/* This provides WRITE access to the data field.  */

void
gfc_conv_descriptor_data_set (stmtblock_t *block, tree desc, tree value)
{
  tree data = conv_descriptor_data (desc);
  gfc_add_modify (block, data, fold_convert (TREE_TYPE (data), value));
}


/* Return a reference to the offset field of the array descriptor DESC.  */

static tree
conv_descriptor_offset (tree desc)
{
  tree field = gfc_get_descriptor_field (desc, OFFSET_FIELD);
  gcc_assert (TREE_TYPE (field) == gfc_array_index_type);
  return field;
}

/* Return the offset value of the array descriptor DESC.  */

tree
gfc_conv_descriptor_offset_get (tree desc)
{
  return conv_descriptor_offset (desc);
}

/* Add code to BLOCK assigning VALUE to the offset field of the array descriptor
   DESC.  */

void
gfc_conv_descriptor_offset_set (stmtblock_t *block, tree desc, tree value)
{
  tree t = conv_descriptor_offset (desc);
  gfc_add_modify (block, t, fold_convert (TREE_TYPE (t), value));
}


/* Return a reference to the dtype field of the array descriptor DESC.  */

static tree
conv_descriptor_dtype (tree desc)
{
  tree field = gfc_get_descriptor_field (desc, DTYPE_FIELD);
  gcc_assert (TREE_TYPE (field) == get_dtype_type_node ());
  return field;
}

/* Return the dtype value of the array descriptor DESC.  */

tree
gfc_conv_descriptor_dtype_get (tree desc)
{
  return conv_descriptor_dtype (desc);
}

/* Add code to BLOCK assigning VALUE to the dtype field of the array descriptor
   DESC.  */

void
gfc_conv_descriptor_dtype_set (stmtblock_t *block, tree desc, tree value)
{
  location_t loc = input_location;
  tree t = conv_descriptor_dtype (desc);
  gfc_add_modify_loc (loc, block, t,
		      fold_convert_loc (loc, TREE_TYPE (t), value));
}


/* Return a reference to the span field of the array descriptor DESC.  */

static tree
conv_descriptor_span (tree desc)
{
  tree field = gfc_get_descriptor_field (desc, SPAN_FIELD);
  gcc_assert (TREE_TYPE (field) == gfc_array_index_type);
  return field;
}

/* Return the span value of the array descriptor DESC.  */

tree
gfc_conv_descriptor_span_get (tree desc)
{
  return conv_descriptor_span (desc);
}

/* Add code to BLOCK assigning VALUE to the span field of the array descriptor
   DESC.  */

void
gfc_conv_descriptor_span_set (stmtblock_t *block, tree desc, tree value)
{
  tree t = conv_descriptor_span (desc);
  gfc_add_modify (block, t, fold_convert (TREE_TYPE (t), value));
}


/* Return a reference to the rank field of the array descriptor DESC.  */

static tree
conv_descriptor_rank (tree desc)
{
  tree tmp;
  tree dtype;

  dtype = conv_descriptor_dtype (desc);
  tmp = gfc_advance_chain (TYPE_FIELDS (TREE_TYPE (dtype)), GFC_DTYPE_RANK);
  gcc_assert (tmp != NULL_TREE
	      && TREE_TYPE (tmp) == gfc_array_dim_rank_type);
  return fold_build3_loc (input_location, COMPONENT_REF, TREE_TYPE (tmp),
			  dtype, tmp, NULL_TREE);
}

/* Return the rank value of the array descriptor DESC.  */

tree
gfc_conv_descriptor_rank_get (tree desc)
{
  return conv_descriptor_rank (desc);
}

/* Add code to BLOCK assigning VALUE to the rank field of the array descriptor
   DESC.  */

void
gfc_conv_descriptor_rank_set (stmtblock_t *block, tree desc, tree value)
{
  location_t loc = input_location;
  tree t = conv_descriptor_rank (desc);
  gfc_add_modify_loc (loc, block, t,
		      fold_convert_loc (loc, TREE_TYPE (t), value));
}

/* Add code to BLOCK assigning VALUE to the rank field of the array descriptor
   DESC.  */

void
gfc_conv_descriptor_rank_set (stmtblock_t *block, tree desc, int value)
{
  gfc_conv_descriptor_rank_set (block, desc, gfc_rank_cst[value]);
}


/* Return a reference to the format version field of the array descriptor
   DESC.  */

static tree
conv_descriptor_version (tree desc)
{
  tree tmp;
  tree dtype;

  dtype = conv_descriptor_dtype (desc);
  tmp = gfc_advance_chain (TYPE_FIELDS (TREE_TYPE (dtype)), GFC_DTYPE_VERSION);
  gcc_assert (tmp != NULL_TREE
	      && TREE_TYPE (tmp) == integer_type_node);
  return fold_build3_loc (input_location, COMPONENT_REF, TREE_TYPE (tmp),
			  dtype, tmp, NULL_TREE);
}

/* Return the format version value of the array descriptor DESC.  */

tree
gfc_conv_descriptor_version_get (tree desc)
{
  return conv_descriptor_version (desc);
}

/* Add code to BLOCK assigning VALUE to the format version field of the array
   descriptor DESC.  */

void
gfc_conv_descriptor_version_set (stmtblock_t *block, tree desc, tree value)
{
  location_t loc = input_location;
  tree t = conv_descriptor_version (desc);
  gfc_add_modify_loc (loc, block, t,
		      fold_convert_loc (loc, TREE_TYPE (t), value));
}


/* Return a reference to the element length field of the array descriptor
   DESC.  */

static tree
conv_descriptor_elem_len (tree desc)
{
  tree tmp;
  tree dtype;

  dtype = conv_descriptor_dtype (desc);
  tmp = gfc_advance_chain (TYPE_FIELDS (TREE_TYPE (dtype)),
			   GFC_DTYPE_ELEM_LEN);
  gcc_assert (tmp != NULL_TREE
	      && TREE_TYPE (tmp) == size_type_node);
  return fold_build3_loc (input_location, COMPONENT_REF, TREE_TYPE (tmp),
			  dtype, tmp, NULL_TREE);
}

/* Return the element length value of the array descriptor DESC.  */

tree
gfc_conv_descriptor_elem_len_get (tree desc)
{
  return conv_descriptor_elem_len (desc);
}

/* Add code to BLOCK assigning VALUE to the element length field of the array
   descriptor DESC.  */

void
gfc_conv_descriptor_elem_len_set (stmtblock_t *block, tree desc, tree value)
{
  location_t loc = input_location;
  tree t = conv_descriptor_elem_len (desc);
  gfc_add_modify_loc (loc, block, t,
		      fold_convert_loc (loc, TREE_TYPE (t), value));
}


/* Return a reference to the type discriminator field of the array descriptor
   DESC.  */

static tree
conv_descriptor_type (tree desc)
{
  tree tmp;
  tree dtype;

  dtype = conv_descriptor_dtype (desc);
  tmp = gfc_advance_chain (TYPE_FIELDS (TREE_TYPE (dtype)), GFC_DTYPE_TYPE);
  gcc_assert (tmp!= NULL_TREE
	      && TREE_TYPE (tmp) == signed_char_type_node);
  return fold_build3_loc (input_location, COMPONENT_REF, TREE_TYPE (tmp),
			  dtype, tmp, NULL_TREE);
}

/* Return the type discriminator value of the array descriptor DESC.  */

tree
gfc_conv_descriptor_type_get (tree desc)
{
  return conv_descriptor_type (desc);
}

/* Add code to BLOCK assigning VALUE to the type discriminator field of the
   array descriptor DESC.  */

void
gfc_conv_descriptor_type_set (stmtblock_t *block, tree desc, tree value)
{
  location_t loc = input_location;
  tree t = conv_descriptor_type (desc);
  gfc_add_modify_loc (loc, block, t,
		      fold_convert_loc (loc, TREE_TYPE (t), value));
}

/* Add code to BLOCK assigning VALUE to the type discriminator field of the
   array descriptor DESC.  */

void
gfc_conv_descriptor_type_set (stmtblock_t *block, tree desc, int value)
{
  tree type = TREE_TYPE (desc);
  gcc_assert (GFC_DESCRIPTOR_TYPE_P (type));

  tree dtype = get_type_field (type, DTYPE_FIELD);
  tree field = get_type_field (TREE_TYPE (dtype), GFC_DTYPE_TYPE);
  tree type_value = build_int_cst (TREE_TYPE (field), value);

  gfc_conv_descriptor_type_set (block, desc, type_value);
}

/* Return a statement assigning VALUE to the type discriminator field of the
   array descriptor DESC.  */

tree
gfc_conv_descriptor_type_set (tree desc, tree value)
{
  stmtblock_t block;

  gfc_init_block (&block);
  gfc_conv_descriptor_type_set (&block, desc, value);
  return gfc_finish_block (&block);
}

/* Return a statement assigning VALUE to the type discriminator field of the
   array descriptor DESC.  */

tree
gfc_conv_descriptor_type_set (tree desc, int value)
{
  stmtblock_t block;

  gfc_init_block (&block);
  gfc_conv_descriptor_type_set (&block, desc, value);
  return gfc_finish_block (&block);
}


/* Return a reference to the array of dimension descriptors of the array
   descriptor DESC.  */

tree
gfc_get_descriptor_dimension (tree desc)
{
  tree field = gfc_get_descriptor_field (desc, DIMENSION_FIELD);
  gcc_assert (TREE_CODE (TREE_TYPE (field)) == ARRAY_TYPE
	      && TREE_CODE (TREE_TYPE (TREE_TYPE (field))) == RECORD_TYPE);
  return field;
}


/* Return a reference to the dimension descriptor for the (zero-based) dimension
   DIM of the array descriptor DESC.  */

static tree
conv_descriptor_dimension (tree desc, tree dim)
{
  tree tmp;

  tmp = gfc_get_descriptor_dimension (desc);

  return gfc_build_array_ref (tmp, dim, NULL_TREE, true);
}


/* Return a reference to the coarray token field of the array descriptor
   DESC.  */

tree
gfc_conv_descriptor_token (tree desc)
{
  gcc_assert (flag_coarray == GFC_FCOARRAY_LIB);
  tree field = gfc_get_descriptor_field (desc, CAF_TOKEN_FIELD);
  /* Should be a restricted pointer - except in the finalization wrapper.  */
  gcc_assert (TREE_TYPE (field) == prvoid_type_node
	      || TREE_TYPE (field) == pvoid_type_node);
  return field;
}

/* Add code to BLOCK assigning VALUE to the coarray token field of the array
   descriptor DESC.  */

void
gfc_conv_descriptor_token_set (stmtblock_t *block, tree desc, tree value)
{
  location_t loc = input_location;
  tree t = gfc_conv_descriptor_token (desc);
  gfc_add_modify_loc (loc, block, t,
		      fold_convert_loc (loc, TREE_TYPE (t), value));
}


/* Return a reference to the FIELD_IDX'th subfield of the dimension descriptor
   of the (zero-based) dimension DIM of the array descriptor DESC.  */

static tree
gfc_conv_descriptor_subfield (tree desc, tree dim, unsigned field_idx)
{
  tree tmp = conv_descriptor_dimension (desc, dim);
  tree field = gfc_advance_chain (TYPE_FIELDS (TREE_TYPE (tmp)), field_idx);
  gcc_assert (field != NULL_TREE);

  return fold_build3_loc (input_location, COMPONENT_REF, TREE_TYPE (field),
			  tmp, field, NULL_TREE);
}


/* Return a reference to the stride field of the (zero-based) dimension DIM of
   the array descriptor DESC.  */

static tree
conv_descriptor_stride (tree desc, tree dim)
{
  tree field = gfc_conv_descriptor_subfield (desc, dim, STRIDE_SUBFIELD);
  gcc_assert (TREE_TYPE (field) == gfc_array_index_type);
  return field;
}

/* Return the stride value for the (zero-based) dimension DIM of the array
   descriptor DESC.  */

tree
gfc_conv_descriptor_stride_get (tree desc, tree dim)
{
  tree type = TREE_TYPE (desc);
  gcc_assert (GFC_DESCRIPTOR_TYPE_P (type));
  if (integer_zerop (dim)
      && (GFC_TYPE_ARRAY_AKIND (type) == GFC_ARRAY_ALLOCATABLE
	  || GFC_TYPE_ARRAY_AKIND (type) == GFC_ARRAY_ASSUMED_SHAPE_CONT
	  || GFC_TYPE_ARRAY_AKIND (type) == GFC_ARRAY_ASSUMED_RANK_CONT
	  || GFC_TYPE_ARRAY_AKIND (type) == GFC_ARRAY_ASSUMED_RANK_ALLOCATABLE
	  || GFC_TYPE_ARRAY_AKIND (type) == GFC_ARRAY_ASSUMED_RANK_POINTER_CONT
	  || GFC_TYPE_ARRAY_AKIND (type) == GFC_ARRAY_POINTER_CONT))
    return gfc_index_one_node;

  return conv_descriptor_stride (desc, dim);
}

/* Add code to BLOCK assigning VALUE to the stride field of the (zero-based)
   dimension DIM of the array descriptor DESC.  */

void
gfc_conv_descriptor_stride_set (stmtblock_t *block, tree desc,
				tree dim, tree value)
{
  tree t = conv_descriptor_stride (desc, dim);
  gfc_add_modify (block, t, fold_convert (TREE_TYPE (t), value));
}


/* Return a reference to the lower bound field of the (zero-based) dimension DIM
   of the array descriptor DESC.  */

static tree
conv_descriptor_lbound (tree desc, tree dim)
{
  tree field = gfc_conv_descriptor_subfield (desc, dim, LBOUND_SUBFIELD);
  gcc_assert (TREE_TYPE (field) == gfc_array_index_type);
  return field;
}

/* Return the lower bound value for the (zero-based) dimension DIM of the array
   descriptor DESC.  */

tree
gfc_conv_descriptor_lbound_get (tree desc, tree dim)
{
  return conv_descriptor_lbound (desc, dim);
}

/* Add code to BLOCK assigning VALUE to the lower bound field of the
   (zero-based) dimension DIM of the array descriptor DESC.  */

void
gfc_conv_descriptor_lbound_set (stmtblock_t *block, tree desc,
				tree dim, tree value)
{
  tree t = conv_descriptor_lbound (desc, dim);
  gfc_add_modify (block, t, fold_convert (TREE_TYPE (t), value));
}


/* Return a reference to the upper bound field of the (zero-based) dimension DIM
   of the array descriptor DESC.  */

static tree
conv_descriptor_ubound (tree desc, tree dim)
{
  tree field = gfc_conv_descriptor_subfield (desc, dim, UBOUND_SUBFIELD);
  gcc_assert (TREE_TYPE (field) == gfc_array_index_type);
  return field;
}

/* Return the upper bound value for the (zero-based) dimension DIM of the array
   descriptor DESC.  */

tree
gfc_conv_descriptor_ubound_get (tree desc, tree dim)
{
  return conv_descriptor_ubound (desc, dim);
}

/* Add code to BLOCK assigning VALUE to the upper bound field of the
   (zero-based) dimension DIM of the array descriptor DESC.  */

void
gfc_conv_descriptor_ubound_set (stmtblock_t *block, tree desc,
				tree dim, tree value)
{
  tree t = conv_descriptor_ubound (desc, dim);
  gfc_add_modify (block, t, fold_convert (TREE_TYPE (t), value));
}


/* Obtain offsets for trans-types.cc(gfc_get_array_descr_info).  */

void
gfc_get_descriptor_offsets_for_info (const_tree desc_type, tree *data_off,
				     tree *rank_off, tree *span_off,
				     tree *dim_off, tree *dim_size,
				     tree *stride_suboff, tree *lower_suboff,
				     tree *upper_suboff)
{
  tree field;
  tree type;

  type = TYPE_MAIN_VARIANT (desc_type);
  tree fields = TYPE_FIELDS (type);
  field = gfc_advance_chain (fields, DATA_FIELD);
  *data_off = byte_position (field);
  field = gfc_advance_chain (fields, DTYPE_FIELD);
  tree dtype_off = byte_position (field);
  type = TREE_TYPE (field);
  field = gfc_advance_chain (TYPE_FIELDS (type), GFC_DTYPE_RANK);
  tree rank_suboff = byte_position (field);
  *rank_off = fold_build2 (PLUS_EXPR, TREE_TYPE (dtype_off), dtype_off,
			   rank_suboff);
  field = gfc_advance_chain (fields, SPAN_FIELD);
  *span_off = byte_position (field);
  field = gfc_advance_chain (fields, DIMENSION_FIELD);
  *dim_off = byte_position (field);
  type = TREE_TYPE (TREE_TYPE (field));
  *dim_size = TYPE_SIZE_UNIT (type);
  field = gfc_advance_chain (TYPE_FIELDS (type), STRIDE_SUBFIELD);
  *stride_suboff = byte_position (field);
  field = gfc_advance_chain (TYPE_FIELDS (type), LBOUND_SUBFIELD);
  *lower_suboff = byte_position (field);
  field = gfc_advance_chain (TYPE_FIELDS (type), UBOUND_SUBFIELD);
  *upper_suboff = byte_position (field);
}


/* Array descriptor higher level routines.
 ******************************************************************************/

/* Return a constructor for a descriptor dtype with the caracteristics given by
   the arguments.  */

tree
gfc_build_dtype_constructor (tree size, int type, int rank)
{
  tree field;
  vec<constructor_elt, va_gc> *v = NULL;

  tree dtype_type_node = get_dtype_type_node ();
  if (size)
    {
      STRIP_NOPS (size);
      size = fold_convert (size_type_node, size);
      field = gfc_advance_chain (TYPE_FIELDS (dtype_type_node),
				 GFC_DTYPE_ELEM_LEN);
      CONSTRUCTOR_APPEND_ELT (v, field,
			      fold_convert (TREE_TYPE (field), size));
    }
  field = gfc_advance_chain (TYPE_FIELDS (dtype_type_node),
			     GFC_DTYPE_VERSION);
  CONSTRUCTOR_APPEND_ELT (v, field,
			  build_zero_cst (TREE_TYPE (field)));

  field = gfc_advance_chain (TYPE_FIELDS (dtype_type_node),
			     GFC_DTYPE_RANK);
  if (rank >= 0)
    CONSTRUCTOR_APPEND_ELT (v, field,
			    build_int_cst (TREE_TYPE (field), rank));

  field = gfc_advance_chain (TYPE_FIELDS (dtype_type_node),
			     GFC_DTYPE_TYPE);
  CONSTRUCTOR_APPEND_ELT (v, field,
			  build_int_cst (TREE_TYPE (field), type));

  return build_constructor (dtype_type_node, v);
}


/* Build a null array descriptor constructor.  */

tree
gfc_build_null_descriptor (tree type)
{
  tree field;
  tree tmp;

  gcc_assert (GFC_DESCRIPTOR_TYPE_P (type));
  gcc_assert (DATA_FIELD == 0);
  field = TYPE_FIELDS (type);

  /* Set a NULL data pointer.  */
  tmp = build_constructor_single (type, field, null_pointer_node);
  TREE_CONSTANT (tmp) = 1;
  /* All other fields are ignored.  */

  return tmp;
}


/* Cleanup those #defines.  */

#undef DATA_FIELD
#undef OFFSET_FIELD
#undef DTYPE_FIELD
#undef SPAN_FIELD
#undef DIMENSION_FIELD
#undef CAF_TOKEN_FIELD

#undef STRIDE_SUBFIELD
#undef LBOUND_SUBFIELD
#undef UBOUND_SUBFIELD

#undef GFC_DTYPE_ELEM_LEN
#undef GFC_DTYPE_VERSION
#undef GFC_DTYPE_RANK
#undef GFC_DTYPE_TYPE
#undef GFC_DTYPE_ATTRIBUTE


/* For an array descriptor, get the total number of elements.  This is just
   the product of the extents along from_dim to to_dim.  */

static tree
gfc_conv_descriptor_size_1 (tree desc, int from_dim, int to_dim)
{
  tree res;
  int dim;

  res = gfc_index_one_node;

  for (dim = from_dim; dim < to_dim; ++dim)
    {
      tree lbound;
      tree ubound;
      tree extent;

      lbound = gfc_conv_descriptor_lbound_get (desc, gfc_rank_cst[dim]);
      ubound = gfc_conv_descriptor_ubound_get (desc, gfc_rank_cst[dim]);

      extent = gfc_conv_array_extent_dim (lbound, ubound, NULL);
      res = fold_build2_loc (input_location, MULT_EXPR, gfc_array_index_type,
			     res, extent);
    }

  return res;
}


/* Full size of an array.  */

tree
gfc_conv_descriptor_size (tree desc, int rank)
{
  return gfc_conv_descriptor_size_1 (desc, 0, rank);
}


/* Size of a coarray for all dimensions but the last.  */

tree
gfc_conv_descriptor_cosize (tree desc, int rank, int corank)
{
  return gfc_conv_descriptor_size_1 (desc, rank, rank + corank - 1);
}


static void
shift_dimension_bounds (stmtblock_t * block, tree descr, tree dim,
			tree new_lbound, tree orig_lbound, tree orig_ubound,
			tree orig_stride, tree *offset_value)
{
  new_lbound = fold_convert (gfc_array_index_type, new_lbound);
  new_lbound = gfc_evaluate_now (new_lbound, block);

  orig_stride = gfc_evaluate_now (orig_stride, block);

  /* Get difference (new - old) by which to shift stuff.  */
  tree diff = fold_build2_loc (input_location, MINUS_EXPR, gfc_array_index_type,
			       new_lbound, orig_lbound);

  /* Shift ubound and offset accordingly.  This has to be done before
     updating the lbound, as they depend on the lbound expression!  */
  tree ubound = fold_build2_loc (input_location, PLUS_EXPR,
				 gfc_array_index_type, orig_ubound, diff);
  gfc_conv_descriptor_ubound_set (block, descr, dim, ubound);
  tree tmp = fold_build2_loc (input_location, MULT_EXPR,
			      gfc_array_index_type, new_lbound, orig_stride);
  *offset_value = fold_build2_loc (input_location, MINUS_EXPR,
				   gfc_array_index_type, *offset_value, tmp);

  /* Finally set lbound to value we want.  */
  gfc_conv_descriptor_lbound_set (block, descr, dim, new_lbound);
}


/* Modify a descriptor such that the lbound of a given dimension is the value
   specified.  This also updates ubound and offset accordingly.  */

static void
conv_shift_descriptor_lbound (stmtblock_t* block, tree desc,
			      int dim, tree new_lbound, tree *offset)
{
  tree ubound, lbound, stride;

  new_lbound = fold_convert (gfc_array_index_type, new_lbound);

  lbound = gfc_conv_descriptor_lbound_get (desc, gfc_rank_cst[dim]);
  ubound = gfc_conv_descriptor_ubound_get (desc, gfc_rank_cst[dim]);
  stride = gfc_conv_descriptor_stride_get (desc, gfc_rank_cst[dim]);

  shift_dimension_bounds (block, desc, gfc_rank_cst[dim], new_lbound, lbound,
			  ubound, stride, offset);
}


void
gfc_copy_descriptor (stmtblock_t *block, tree dst, tree src, int rank)
{
  int n;
  tree dim;
  tree tmp;
  tree tmp2;
  tree size;
  tree offset;

  offset = gfc_index_zero_node;

  /* Use memcpy to copy the descriptor.  The size is the minimum of
     the sizes of 'src' and 'dst'. This avoids a non-trivial conversion.  */
  tmp = TYPE_SIZE_UNIT (TREE_TYPE (src));
  tmp2 = TYPE_SIZE_UNIT (TREE_TYPE (dst));
  size = fold_build2_loc (input_location, MIN_EXPR,
			  TREE_TYPE (tmp), tmp, tmp2);
  tmp = builtin_decl_explicit (BUILT_IN_MEMCPY);
  tmp = build_call_expr_loc (input_location, tmp, 3,
			     gfc_build_addr_expr (NULL_TREE, dst),
			     gfc_build_addr_expr (NULL_TREE, src),
			     fold_convert (size_type_node, size));
  gfc_add_expr_to_block (block, tmp);

  /* Set the offset correctly.  */
  for (n = 0; n < rank; n++)
    {
      dim = gfc_rank_cst[n];
      tmp = gfc_conv_descriptor_lbound_get (src, dim);
      tmp2 = gfc_conv_descriptor_stride_get (src, dim);
      tmp = fold_build2_loc (input_location, MULT_EXPR, TREE_TYPE (tmp),
			     tmp, tmp2);
      offset = fold_build2_loc (input_location, MINUS_EXPR,
			TREE_TYPE (offset), offset, tmp);
      offset = gfc_evaluate_now (offset, block);
    }

  gfc_conv_descriptor_offset_set (block, dst, offset);
}


/* Extend the data in array DESC by EXTRA elements.  */

void
gfc_grow_array (stmtblock_t * pblock, tree desc, tree extra)
{
  tree arg0, arg1;
  tree tmp;
  tree size;
  tree ubound;

  if (integer_zerop (extra))
    return;

  ubound = gfc_conv_descriptor_ubound_get (desc, gfc_rank_cst[0]);

  /* Add EXTRA to the upper bound.  */
  tmp = fold_build2_loc (input_location, PLUS_EXPR, gfc_array_index_type,
			 ubound, extra);
  gfc_conv_descriptor_ubound_set (pblock, desc, gfc_rank_cst[0], tmp);

  /* Get the value of the current data pointer.  */
  arg0 = gfc_conv_descriptor_data_get (desc);

  /* Calculate the new array size.  */
  size = TYPE_SIZE_UNIT (gfc_get_element_type (TREE_TYPE (desc)));
  tmp = fold_build2_loc (input_location, PLUS_EXPR, gfc_array_index_type,
			 ubound, gfc_index_one_node);
  arg1 = fold_build2_loc (input_location, MULT_EXPR, size_type_node,
			  fold_convert (size_type_node, tmp),
			  fold_convert (size_type_node, size));

  /* Call the realloc() function.  */
  tmp = gfc_call_realloc (pblock, arg0, arg1);
  gfc_conv_descriptor_data_set (pblock, desc, tmp);
}


void
gfc_nullify_descriptor (stmtblock_t *block, tree descr)
{
  gfc_conv_descriptor_data_set (block, descr, null_pointer_node);
}


void
gfc_init_descriptor_result (stmtblock_t *block, tree descr)
{
  gfc_conv_descriptor_data_set (block, descr, null_pointer_node);
}


void
gfc_init_absent_descriptor (stmtblock_t *block, tree descr)
{
  gfc_conv_descriptor_data_set (block, descr, null_pointer_node);
}


void
gfc_init_static_descriptor (tree descr)
{
  tree type = TREE_TYPE (descr);
  DECL_INITIAL (descr) = gfc_build_null_descriptor (type);
}


void
gfc_nullify_descriptor (stmtblock_t *block, gfc_symbol *sym, gfc_expr *expr,
			tree descr, tree string_length)
{
  symbol_attribute attr = gfc_symbol_attr (sym);

  /* NULLIFY the data pointer for non-saved allocatables, or for non-saved
     pointers when -fcheck=pointer is specified.  */
  if (attr.allocatable
      || attr.optional
      || (attr.pointer && (gfc_option.rtcheck & GFC_RTCHECK_POINTER)))
    {
      gfc_conv_descriptor_data_set (block, descr, null_pointer_node);
      if (flag_coarray == GFC_FCOARRAY_LIB && attr.codimension)
	gfc_conv_descriptor_token_set (block, descr, null_pointer_node);
    }

  gfc_array_spec *as;
  if (sym->ts.type == BT_CLASS)
    as = CLASS_DATA (sym)->as;
  else
    as = sym->as;

  int rank;
  if (as == nullptr)
    rank = 0;
  else if (as->type != AS_ASSUMED_RANK)
    rank = as->rank;
  else if (expr)
    rank = expr->rank;
  else
    rank = -1;

  tree etype = gfc_get_element_type (TREE_TYPE (descr));
  tree dtype = gfc_get_dtype_rank_type_slen (rank, etype, string_length);
  gfc_conv_descriptor_dtype_set (block, descr, dtype);
}

void
gfc_init_descriptor_variable (stmtblock_t *block, gfc_symbol *sym,
			      gfc_expr *expr, tree descr)
{
  return gfc_nullify_descriptor (block, sym, expr, descr, NULL_TREE);
}


void
gfc_init_descriptor_variable (stmtblock_t *block, gfc_symbol *sym, tree descr)
{
  return gfc_init_descriptor_variable (block, sym, nullptr, descr);
}


tree
gfc_create_unallocated_library_result_descriptor (stmtblock_t *block, tree source_descr, tree dtype)
{
  if (dtype == NULL_TREE)
    dtype = gfc_get_dtype (TREE_TYPE (source_descr));

  gfc_conv_descriptor_dtype_set (block, source_descr, dtype);

  tree res_desc = gfc_evaluate_now (source_descr, block);
  gfc_conv_descriptor_data_set (block, res_desc, null_pointer_node);

  return res_desc;
}


tree
gfc_create_null_actual_descriptor (stmtblock_t *block, gfc_typespec *ts,
				   symbol_attribute attr, int rank)
{
  tree etype = gfc_typenode_for_spec (ts);

  enum gfc_array_kind akind;

  if (attr.pointer)
    akind = GFC_ARRAY_POINTER_CONT;
  else if (attr.allocatable)
    akind = GFC_ARRAY_ALLOCATABLE;
  else
    akind = GFC_ARRAY_ASSUMED_SHAPE_CONT;

  tree lower[GFC_MAX_DIMENSIONS];
  tree upper[GFC_MAX_DIMENSIONS];
  memset (&lower, 0, rank * sizeof (lower[0]));
  memset (&upper, 0, rank * sizeof (upper[0]));

  tree type = gfc_get_array_type_bounds (etype, rank, 0, lower, upper, 1,
					 akind, !(attr.pointer || attr.target));
  tree desc = gfc_create_var (type, "desc");
  DECL_ARTIFICIAL (desc) = 1;

  gfc_conv_descriptor_dtype_set (block, desc,
				 gfc_get_dtype_rank_type (rank, etype));
  gfc_conv_descriptor_data_set (block, desc, null_pointer_node);
  gfc_conv_descriptor_span_set (block, desc,
				gfc_conv_descriptor_elem_len_get (desc));

  return desc;
}


void
gfc_set_descriptor_from_scalar (stmtblock_t *block, tree descr, tree scalar,
				symbol_attribute attr, tree cond_presence,
				tree caf_token)
{
  if (flag_coarray == GFC_FCOARRAY_LIB && caf_token)
    gfc_conv_descriptor_token_set (block, descr, caf_token);

  tree type = gfc_get_scalar_to_descriptor_type (TREE_TYPE (scalar), attr);
  if (!POINTER_TYPE_P (TREE_TYPE (scalar)))
    scalar = gfc_build_addr_expr (NULL_TREE, scalar);
  if (cond_presence)
    scalar = build3_loc (input_location, COND_EXPR,
			 TREE_TYPE (scalar),
			 cond_presence, scalar,
			 fold_convert (TREE_TYPE (scalar),
				       null_pointer_node));

  gfc_conv_descriptor_dtype_set (block, descr, gfc_get_dtype (type));
  gfc_copy_coarray_desc_part (block, descr, scalar);
  gfc_conv_descriptor_data_set (block, descr, scalar);
  gfc_conv_descriptor_span_set (block, descr,
				gfc_conv_descriptor_elem_len_get (descr));
}


void
gfc_set_descriptor_from_scalar_class (stmtblock_t *block, tree descr,
				      tree scalar, gfc_expr *scalar_expr)
{
  gfc_set_descriptor_from_scalar (block, descr, gfc_class_data_get (scalar),
				  gfc_expr_attr (scalar_expr));
}


void
gfc_copy_coarray_desc_part (stmtblock_t *block, tree dest, tree src)
{
  tree src_type = TREE_TYPE (src);
  if (TYPE_LANG_SPECIFIC (src_type) && TYPE_LANG_SPECIFIC (src_type)->corank)
    {
      struct lang_type *lang_specific = TYPE_LANG_SPECIFIC (src_type);
      for (int c = 0; c < lang_specific->corank; ++c)
	{
	  int dim = lang_specific->rank + c;
	  tree codim = gfc_rank_cst[dim];

	  if (lang_specific->lbound[dim])
	    gfc_conv_descriptor_lbound_set (block, dest, codim,
					    lang_specific->lbound[dim]);
	  else
	    gfc_conv_descriptor_lbound_set (
	      block, dest, codim, gfc_conv_descriptor_lbound_get (src, codim));
	  if (dim + 1 < lang_specific->corank)
	    {
	      if (lang_specific->ubound[dim])
		gfc_conv_descriptor_ubound_set (block, dest, codim,
						lang_specific->ubound[dim]);
	      else
		gfc_conv_descriptor_ubound_set (
		  block, dest, codim,
		  gfc_conv_descriptor_ubound_get (src, codim));
	    }
	}
    }
}


void
gfc_set_descriptor_from_scalar (stmtblock_t *block, tree descr,
				tree scalar, gfc_expr *scalar_expr,
				tree cond_presence, tree caf_token)
{
  gfc_set_descriptor_from_scalar (block, descr, scalar,
				  gfc_expr_attr (scalar_expr), cond_presence,
				  caf_token);
}


void
gfc_conv_shift_descriptor (stmtblock_t* block, tree desc, int rank)
{
  /* Apply a shift of the lbound when supplied.  */
  tree offset = gfc_index_zero_node;
  for (int dim = 0; dim < rank; ++dim)
    conv_shift_descriptor_lbound (block, desc, dim, gfc_index_one_node,
				  &offset);
  gfc_conv_descriptor_offset_set (block, desc, offset);
}


static void
conv_shift_descriptor (stmtblock_t *block, tree desc, int rank,
		       gfc_expr * const (lbound[GFC_MAX_DIMENSIONS]))
{
  /* Apply a shift of the lbound when supplied.  */
  tree offset = gfc_index_zero_node;
  for (int dim = 0; dim < rank; ++dim)
    {
      gfc_expr *lb_expr = lbound[dim];

      tree lower_bound;
      if (lb_expr == nullptr)
	lower_bound = gfc_index_one_node;
      else
	{
	  gfc_se lb_se;

	  gfc_init_se (&lb_se, nullptr);
	  gfc_conv_expr (&lb_se, lb_expr);

	  gfc_add_block_to_block (block, &lb_se.pre);
	  tree lb_var = gfc_create_var (TREE_TYPE (lb_se.expr), "lower_bound");
	  gfc_add_modify (block, lb_var, lb_se.expr);
	  gfc_add_block_to_block (block, &lb_se.post);

	  lower_bound = lb_var;
	}

      conv_shift_descriptor_lbound (block, desc, dim, lower_bound, &offset);
    }
  gfc_conv_descriptor_offset_set (block, desc, offset);
}


static void
conv_shift_descriptor (stmtblock_t *block, tree desc,
		       const gfc_array_spec &as)
{
  conv_shift_descriptor (block, desc, as.rank, as.lower);
}


static void
set_type (array_type &type, array_type value)
{
  gcc_assert (type == AS_UNKNOWN || type == value);
  type = value;
}


static void
array_ref_to_array_spec (const gfc_array_ref &ref, gfc_array_spec &spec)
{
  spec.rank = ref.dimen;
  spec.corank = ref.codimen;

  spec.type = AS_UNKNOWN;
  spec.cotype = AS_ASSUMED_SIZE;

  for (int dim = 0; dim < spec.rank + spec.corank; dim++)
    switch (ref.dimen_type[dim])
      {
      case DIMEN_ELEMENT:
	spec.upper[dim] = ref.start[dim];
	set_type (spec.type, AS_EXPLICIT);
	break;

      case DIMEN_RANGE:
	spec.lower[dim] = ref.start[dim];
	spec.upper[dim] = ref.end[dim];
	if (spec.upper[dim] == nullptr)
	  set_type (spec.type, AS_DEFERRED);
	else
	  set_type (spec.type, AS_EXPLICIT);
	break;

      default:
	break;
      }
}


void
gfc_conv_shift_descriptor (stmtblock_t *block, tree desc,
			   const gfc_array_ref &ar, gfc_expr *src_expr,
			   bool unlimited_poly_dest)
{
  /* Always set dtype.  */
  gfc_conv_descriptor_dtype_set (block, desc,
				 gfc_get_dtype (TREE_TYPE (desc)));

  /* For unlimited polymorphic LHS use elem_len from RHS.  */
  if (unlimited_poly_dest && src_expr->ts.type != BT_CLASS)
    {
      tree elem_len;
      tree tmp = TYPE_SIZE_UNIT (gfc_typenode_for_spec (&src_expr->ts));
      elem_len = fold_convert (gfc_array_index_type, tmp);
      elem_len = gfc_evaluate_now (elem_len, block);
      gfc_conv_descriptor_elem_len_set (block, desc, elem_len);
    }

  gfc_array_spec as;
  array_ref_to_array_spec (ar, as);

  conv_shift_descriptor (block, desc, as);
}


void
gfc_conv_shift_descriptor (stmtblock_t *block, tree dest, tree src,
			   int rank, tree zero_cond)
{
  tree tmp = gfc_conv_descriptor_data_get (src);
  gfc_conv_descriptor_data_set (block, dest, tmp);

  tree offset = gfc_index_zero_node;
  for (int n = 0 ; n < rank; n++)
    {
      tree lbound = gfc_conv_descriptor_lbound_get (dest, gfc_rank_cst[n]);
      lbound = fold_build3_loc (input_location, COND_EXPR,
				gfc_array_index_type, zero_cond,
				gfc_index_one_node, lbound);
      lbound = gfc_evaluate_now (lbound, block);

      tree dim = gfc_rank_cst[n];
      tree stride = gfc_conv_descriptor_stride_get (src, dim);
      shift_dimension_bounds (block, dest, gfc_rank_cst[n],
			      lbound, gfc_index_zero_node,
			      gfc_conv_descriptor_ubound_get (src, dim),
			      stride, &offset);

      gfc_conv_descriptor_stride_set (block, dest, dim, stride);
    }

  gfc_conv_descriptor_offset_set (block, dest, offset);
}


void
gfc_set_subarray_descriptor (stmtblock_t *block, tree descr, tree value,
			     gfc_expr *value_expr, gfc_expr *conv_arg)
{
  if (value_expr->expr_type != EXPR_VARIABLE)
    gfc_conv_descriptor_data_set (block, value,
				  null_pointer_node);

  /* Obtain the array spec of full array references.  */
  gfc_array_spec *as;
  if (conv_arg)
    as = gfc_get_full_arrayspec_from_expr (conv_arg);
  else
    as = gfc_get_full_arrayspec_from_expr (value_expr);

  /* Shift the lbound and ubound of temporaries to being unity,
     rather than zero, based. Always calculate the offset.  */
  tree offset = gfc_index_zero_node;

  for (int n = 0; n < value_expr->rank; n++)
    {
      tree lbound;

      /* Obtain the correct lbound - ISO/IEC TR 15581:2001 page 9.
	 TODO It looks as if gfc_conv_expr_descriptor should return
	 the correct bounds and that the following should not be
	 necessary.  This would simplify gfc_conv_intrinsic_bound
	 as well.  */
      if (as && as->lower[n])
	{
	  gfc_se lbse;
	  gfc_init_se (&lbse, NULL);
	  gfc_conv_expr (&lbse, as->lower[n]);
	  gfc_add_block_to_block (block, &lbse.pre);
	  lbound = gfc_evaluate_now (lbse.expr, block);
	}
      else if (as && conv_arg)
	{
	  tree tmp = gfc_get_symbol_decl (conv_arg->symtree->n.sym);
	  lbound = gfc_conv_descriptor_lbound_get (tmp, gfc_rank_cst[n]);
	}
      else if (as)
	lbound = gfc_conv_descriptor_lbound_get (descr, gfc_rank_cst[n]);
      else
	lbound = gfc_index_one_node;

      lbound = fold_convert (gfc_array_index_type, lbound);

      /* Shift the bounds and set the offset accordingly.  */
      tree dim = gfc_rank_cst[n];
      shift_dimension_bounds (block, descr, dim, lbound,
			      gfc_conv_descriptor_lbound_get (descr, dim),
			      gfc_conv_descriptor_ubound_get (descr, dim),
			      gfc_conv_descriptor_stride_get (descr, dim),
			      &offset);
    }
  gfc_conv_descriptor_offset_set (block, descr, offset);
}


void
gfc_shift_descriptor (stmtblock_t *block, tree descr, int rank,
		      tree lbound[GFC_MAX_DIMENSIONS],
		      tree ubound[GFC_MAX_DIMENSIONS])
{
  tree size = gfc_index_one_node;
  tree offset = gfc_index_zero_node;
  for (int n = 0; n < rank; n++)
    {
      tree dim = gfc_rank_cst[n];
      shift_dimension_bounds (block, descr, dim, gfc_index_one_node,
			      lbound[n], ubound[n], size, &offset);

      tree tmp = fold_build2_loc (input_location, MINUS_EXPR,
				  gfc_array_index_type, ubound[n], lbound[n]);
      tmp = fold_build2_loc (input_location, PLUS_EXPR,
			     gfc_array_index_type, tmp, gfc_index_one_node);
      size = fold_build2_loc (input_location, MULT_EXPR,
			      gfc_array_index_type, size, tmp);
    }

  gfc_conv_descriptor_offset_set (block, descr, offset);
}
 

void
gfc_copy_sequence_descriptor (stmtblock_t *block, tree dest, tree src, int rank)
{
  gfc_conv_descriptor_data_set (block, dest,
				gfc_conv_descriptor_data_get (src));
  gfc_conv_descriptor_lbound_set (block, dest, gfc_index_zero_node,
				  gfc_index_zero_node);
  gfc_conv_descriptor_ubound_set (block, dest, gfc_index_zero_node,
				  gfc_conv_descriptor_size (src, rank));
  tree stride = gfc_conv_descriptor_stride_get (src, gfc_index_zero_node);
  gfc_conv_descriptor_stride_set (block, dest, gfc_index_zero_node, stride);
  gfc_conv_descriptor_dtype_set (block, dest,
				 gfc_conv_descriptor_dtype_get (src));
  gfc_conv_descriptor_rank_set (block, dest, 1);
  gfc_conv_descriptor_span_set (block, dest,
				gfc_conv_descriptor_span_get (src));
  gfc_conv_descriptor_offset_set (block, dest, gfc_index_zero_node);
}


void
gfc_copy_descriptor (stmtblock_t *block, tree dest, tree src,
		     gfc_expr *src_expr, bool subref)
{
  struct lang_type *dest_ls = TYPE_LANG_SPECIFIC (TREE_TYPE (dest));
  struct lang_type *src_ls = TYPE_LANG_SPECIFIC (TREE_TYPE (src));

  /* When only the array_kind differs, do a view_convert.  */
  tree tmp1;
  if (dest_ls
      && src_ls
      && dest_ls->rank == src_ls->rank
      && dest_ls->akind != src_ls->akind)
    tmp1 = build1 (VIEW_CONVERT_EXPR, TREE_TYPE (dest), src);
  else
    tmp1 = src;

  /* Copy the descriptor for pointer assignments.  */
  gfc_add_modify (block, dest, tmp1);

  /* Add any offsets from subreferences.  */
  gfc_get_dataptr_offset (block, dest, src, NULL_TREE, subref, src_expr);

  /* ....and set the span field.  */
  tree tmp2;
  if (src_expr->ts.type == BT_CHARACTER)
    tmp2 = gfc_conv_descriptor_span_get (src);
  else
    tmp2 = gfc_get_array_span (src, src_expr);
  gfc_conv_descriptor_span_set (block, dest, tmp2);
}


static void
copy_coarray_desc_part (stmtblock_t *block, tree dest, tree src)
{
  tree src_type = TREE_TYPE (src);
  if (TYPE_LANG_SPECIFIC (src_type) && TYPE_LANG_SPECIFIC (src_type)->corank)
    {
      struct lang_type *lang_specific = TYPE_LANG_SPECIFIC (src_type);
      for (int c = 0; c < lang_specific->corank; ++c)
	{
	  int dim = lang_specific->rank + c;
	  tree codim = gfc_rank_cst[dim];

	  if (lang_specific->lbound[dim])
	    gfc_conv_descriptor_lbound_set (block, dest, codim,
					    lang_specific->lbound[dim]);
	  else
	    gfc_conv_descriptor_lbound_set (
	      block, dest, codim, gfc_conv_descriptor_lbound_get (src, codim));
	  if (dim + 1 < lang_specific->corank)
	    {
	      if (lang_specific->ubound[dim])
		gfc_conv_descriptor_ubound_set (block, dest, codim,
						lang_specific->ubound[dim]);
	      else
		gfc_conv_descriptor_ubound_set (
		  block, dest, codim,
		  gfc_conv_descriptor_ubound_get (src, codim));
	    }
	}
    }
}


void
gfc_copy_descriptor (stmtblock_t *block, tree dest, tree src, bool lhs_type)
{
  gfc_conv_descriptor_data_set (block, dest,
				gfc_conv_descriptor_data_get (src));
  gfc_conv_descriptor_offset_set (block, dest,
				  gfc_conv_descriptor_offset_get (src));

  gfc_conv_descriptor_dtype_set (block, dest,
				 gfc_conv_descriptor_dtype_get (src));

  /* Assign the dimension as range-ref.  */
  tree lhs_dim = gfc_get_descriptor_dimension (dest);
  tree rhs_dim = gfc_get_descriptor_dimension (src);

  tree type = lhs_type ? TREE_TYPE (lhs_dim) : TREE_TYPE (rhs_dim);
  lhs_dim = build4_loc (input_location, ARRAY_RANGE_REF, type, lhs_dim,
		    gfc_index_zero_node, NULL_TREE, NULL_TREE);
  rhs_dim = build4_loc (input_location, ARRAY_RANGE_REF, type, rhs_dim,
		     gfc_index_zero_node, NULL_TREE, NULL_TREE);
  gfc_add_modify (block, lhs_dim, rhs_dim);

  /* The corank dimensions are not copied by the ARRAY_RANGE_REF.  */
  copy_coarray_desc_part (block, dest, src);
}


void
gfc_copy_descriptor (stmtblock_t *block, tree dest, tree src, tree ptr,
		     int rank)
{
  gfc_conv_descriptor_dtype_set (block, dest,
				 gfc_conv_descriptor_dtype_get (src));

  if (rank == -1)
    {
      tree idx = gfc_create_var (gfc_array_dim_rank_type, "idx");
      tree stride = gfc_create_var (gfc_array_index_type, "stride");
      stmtblock_t loop_body;

      gfc_conv_descriptor_offset_set (block, dest, gfc_index_zero_node);
      gfc_conv_descriptor_span_set (block, dest,
				    gfc_conv_descriptor_span_get (src));
      gfc_add_modify (block, stride, gfc_index_one_node);

      gfc_init_block (&loop_body);

      tree old_field = gfc_conv_descriptor_lbound_get (src, idx);
      gfc_conv_descriptor_lbound_set (&loop_body, dest, idx,
				      old_field);

      old_field = gfc_conv_descriptor_ubound_get (src, idx);
      gfc_conv_descriptor_ubound_set (&loop_body, dest, idx,
				      old_field);

      gfc_conv_descriptor_stride_set (&loop_body, dest, idx,
				      stride);

      tree offset = fold_build2_loc (input_location, MULT_EXPR,
				     gfc_array_index_type, stride,
				     gfc_conv_descriptor_lbound_get (dest,
								     idx));
      offset = fold_build2_loc (input_location, MINUS_EXPR,
				gfc_array_index_type,
				gfc_conv_descriptor_offset_get (dest),
				offset);
      gfc_conv_descriptor_offset_set (&loop_body, dest, offset);

      tree extent = gfc_conv_array_extent_dim
		    (gfc_conv_descriptor_lbound_get (dest, idx),
		     gfc_conv_descriptor_ubound_get (dest, idx),
		     NULL);
      extent = fold_build2_loc (input_location, MULT_EXPR,
				gfc_array_index_type, stride,
				extent);
      gfc_add_modify (&loop_body, stride, extent);

      gfc_simple_for_loop (block, idx, build_int_cst (TREE_TYPE (idx), 0),
			   gfc_conv_descriptor_rank_get (src), LT_EXPR,
			   build_int_cst (TREE_TYPE (idx), 1),
			   gfc_finish_block (&loop_body));
    }
  else
    {
      tree offset = gfc_index_zero_node;

      tree stride = gfc_index_one_node;

      for (int i = 0; i < rank; i++)
	{
	  tree dim = gfc_rank_cst[i];

	  tree lbound = gfc_conv_descriptor_lbound_get (src, dim);
	  lbound = gfc_evaluate_now (lbound, block);
	  gfc_conv_descriptor_lbound_set (block, dest, dim, lbound);

	  tree ubound = gfc_conv_descriptor_ubound_get (src, dim);
	  ubound = gfc_evaluate_now (ubound, block);
	  gfc_conv_descriptor_ubound_set (block, dest, dim, ubound);

	  gfc_conv_descriptor_stride_set (block, dest, dim, stride);

	  tree tmp = fold_build2_loc (input_location, MULT_EXPR,
				      gfc_array_index_type, stride, lbound);
	  offset = fold_build2_loc (input_location, MINUS_EXPR,
				    gfc_array_index_type, offset, tmp);
	  offset = gfc_evaluate_now (offset, block);

	  /* Now calculate the stride for next dimension, unless the
	     current dimension is the last one.  */
	  if (i == rank - 1)
	    break;

	  tmp = fold_build2_loc (input_location, MINUS_EXPR,
				 gfc_array_index_type, lbound,
				 gfc_index_one_node);
	  tree extent = fold_build2_loc (input_location, MINUS_EXPR,
					 gfc_array_index_type, ubound, tmp);
	  stride = fold_build2_loc (input_location, MULT_EXPR,
				    gfc_array_index_type, stride, extent);
	  stride = gfc_evaluate_now (stride, block);
	}

      gfc_conv_descriptor_offset_set (block, dest, offset);
    }

  if (flag_coarray == GFC_FCOARRAY_LIB
      && GFC_DESCRIPTOR_TYPE_P (TREE_TYPE (src))
      && GFC_TYPE_ARRAY_AKIND (TREE_TYPE (src)) == GFC_ARRAY_ALLOCATABLE)
    gfc_conv_descriptor_token_set (block, dest,
				   gfc_conv_descriptor_token (src));

  gfc_conv_descriptor_data_set (block, dest, ptr);
}


void
gfc_conv_remap_descriptor (stmtblock_t *block, tree dest, int dest_rank,
			   tree src, gfc_expr *src_expr, gfc_array_ref *ar,
			   bool unlimited_poly_dest)
{
  /* Set dtype.  */
  gfc_conv_descriptor_dtype_set (block, dest,
				 gfc_get_dtype (TREE_TYPE (dest)));

  /* For unlimited polymorphic LHS use elem_len from RHS.  */
  if (unlimited_poly_dest && src_expr->ts.type != BT_CLASS)
    {
      tree elem_len;
      tree tmp = TYPE_SIZE_UNIT (gfc_typenode_for_spec (&src_expr->ts));
      elem_len = fold_convert (gfc_array_index_type, tmp);
      elem_len = gfc_evaluate_now (elem_len, block);
      gfc_conv_descriptor_elem_len_set (block, dest, elem_len);
    }

  /* Copy data pointer.  */
  gfc_conv_descriptor_data_set (block, dest,
				gfc_conv_descriptor_data_get (src));

  /* Copy the span.  */
  tree span;
  if (VAR_P (src)
      && GFC_DECL_PTR_ARRAY_P (src))
    span = gfc_conv_descriptor_span_get (src);
  else
    {
      tree tmp = TREE_TYPE (src);
      tmp = TYPE_SIZE_UNIT (gfc_get_element_type (tmp));
      span = fold_convert (gfc_array_index_type, tmp);
    }
  gfc_conv_descriptor_span_set (block, dest, span);

  /* Copy offset but adjust it such that it would correspond
     to a lbound of zero.  */
  if (src_expr->rank == -1)
    gfc_conv_descriptor_offset_set (block, dest,
				    gfc_index_zero_node);
  else
    {
      tree offs = gfc_conv_descriptor_offset_get (src);
      for (int dim = 0; dim < src_expr->rank; ++dim)
	{
	  tree stride = gfc_conv_descriptor_stride_get (src, gfc_rank_cst[dim]);
	  tree lbound = gfc_conv_descriptor_lbound_get (src, gfc_rank_cst[dim]);
	  tree tmp = fold_build2_loc (input_location, MULT_EXPR,
				      gfc_array_index_type, stride, lbound);
	  offs = fold_build2_loc (input_location, PLUS_EXPR,
				  gfc_array_index_type, offs, tmp);
	}
      gfc_conv_descriptor_offset_set (block, dest, offs);
    }

  /* Set the bounds as declared for the LHS and calculate strides as
     well as another offset update accordingly.  */
  tree stride = gfc_conv_descriptor_stride_get (src, gfc_rank_cst[0]);
  for (int dim = 0; dim < dest_rank; ++dim)
    {
      gfc_se lower_se;
      gfc_se upper_se;

      gcc_assert (ar->start[dim] && ar->end[dim]);

      if (ar->start[dim]->expr_type != EXPR_CONSTANT
	  || ar->start[dim]->expr_type != EXPR_VARIABLE)
	gfc_resolve_expr (ar->start[dim]);
      if (ar->end[dim]->expr_type != EXPR_CONSTANT
	  || ar->end[dim]->expr_type != EXPR_VARIABLE)
	gfc_resolve_expr (ar->end[dim]);

      /* Convert declared bounds.  */
      gfc_init_se (&lower_se, NULL);
      gfc_init_se (&upper_se, NULL);
      gfc_conv_expr (&lower_se, ar->start[dim]);
      gfc_conv_expr (&upper_se, ar->end[dim]);

      gfc_add_block_to_block (block, &lower_se.pre);
      gfc_add_block_to_block (block, &upper_se.pre);

      tree lbound = fold_convert (gfc_array_index_type, lower_se.expr);
      tree ubound = fold_convert (gfc_array_index_type, upper_se.expr);

      lbound = gfc_evaluate_now (lbound, block);
      ubound = gfc_evaluate_now (ubound, block);

      gfc_add_block_to_block (block, &lower_se.post);
      gfc_add_block_to_block (block, &upper_se.post);

      /* Set bounds in descriptor.  */
      gfc_conv_descriptor_lbound_set (block, dest, gfc_rank_cst[dim], lbound);
      gfc_conv_descriptor_ubound_set (block, dest, gfc_rank_cst[dim], ubound);

      /* Set stride.  */
      stride = gfc_evaluate_now (stride, block);
      gfc_conv_descriptor_stride_set (block, dest, gfc_rank_cst[dim], stride);

      /* Update offset.  */
      tree offs = gfc_conv_descriptor_offset_get (dest);
      tree tmp = fold_build2_loc (input_location, MULT_EXPR,
				  gfc_array_index_type, lbound, stride);
      offs = fold_build2_loc (input_location, MINUS_EXPR,
			      gfc_array_index_type, offs, tmp);
      offs = gfc_evaluate_now (offs, block);
      gfc_conv_descriptor_offset_set (block, dest, offs);

      /* Update stride.  */
      tmp = gfc_conv_array_extent_dim (lbound, ubound, NULL);
      stride = fold_build2_loc (input_location, MULT_EXPR,
				gfc_array_index_type, stride, tmp);
    }
}
