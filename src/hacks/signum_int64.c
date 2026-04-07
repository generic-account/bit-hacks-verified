/*
title = "Signum (-1/0/+1)"
hack_id = "signum_int64"
tags = ["integer", "sign", "bit-shift", "branchless"]
summary = "Computes the signum of a 64-bit integer, returning -1 for negatives, 0 for zero, and +1 for positives."
contract = "Returns -1 when the input is negative, 0 when the input is zero, and +1 when the input is positive."
notes = """
The comparison-based implementation is the most portable and explicit form.
The shift-based implementations match the classic bit-hack forms, with the arithmetic-shift variant depending on right-shifting signed negatives propagating the sign bit.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign",
]
*/

#include <assert.h>
#include <stdint.h>

typedef int64_t bh_input_t;
typedef int64_t bh_output_t;

static bh_output_t bh_optimized_comparisons(bh_input_t input);
static bh_output_t bh_optimized_unsigned_shift(bh_input_t input);
static bh_output_t bh_optimized_arithmetic_shift(bh_input_t input);

#define BH_IMPLS(X) \
    X("comparisons", bh_optimized_comparisons) \
    X("unsigned_shift", bh_optimized_unsigned_shift) \
    X("arithmetic_shift", bh_optimized_arithmetic_shift)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output >= -1 && output <= 1);
}

static bh_output_t bh_reference(bh_input_t input)
{
    return (input > 0) - (input < 0);
}

static bh_output_t bh_optimized_comparisons(bh_input_t input)
{
    return (input > 0) - (input < 0);
}

static bh_output_t bh_optimized_unsigned_shift(bh_input_t input)
{
    return (bh_output_t)(input != 0) | -(bh_output_t)((uint64_t)input >> 63);
}

static bh_output_t bh_optimized_arithmetic_shift(bh_input_t input)
{
    return (bh_output_t)(input != 0) | (input >> 63);
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(INT64_MIN, -1);
    BH_ASSERT_OPT_EQ(INT64_C(-1), -1);
    BH_ASSERT_OPT_EQ(INT64_C(0), 0);
    BH_ASSERT_OPT_EQ(INT64_C(1), 1);
    BH_ASSERT_OPT_EQ(INT64_MAX, 1);

    BH_ASSERT_AGREE(INT64_C(-2));
    BH_ASSERT_AGREE(INT64_C(2));
    BH_ASSERT_AGREE(INT64_C(-4611686018427387904));
    BH_ASSERT_AGREE(INT64_C(4611686018427387904));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
