/*
title = "Compute the sign of a 64-bit integer (-1/+1)"
hack_id = "sign_pm1_int64"
tags = ["integer", "sign", "bit-shift", "branchless"]
summary = "Computes the sign of a 64-bit integer, returning -1 for negative inputs and +1 otherwise."
contract = "Returns -1 when the input is negative and +1 when the input is zero or positive."
notes = """
This variant never returns 0. It collapses zero into the non-negative branch and produces +1.
The arithmetic-shift implementation is shorter, but it depends on right-shifting signed negatives propagating the sign bit on the target architecture.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign",
]
*/

#include <assert.h>
#include <stdint.h>

typedef int64_t bh_input_t;
typedef int64_t bh_output_t;

static bh_output_t bh_optimized_unsigned_shift(bh_input_t input);
static bh_output_t bh_optimized_arithmetic_shift(bh_input_t input);

#define BH_IMPLS(X) \
    X("unsigned_shift", bh_optimized_unsigned_shift) \
    X("arithmetic_shift", bh_optimized_arithmetic_shift)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output == -1 || output == 1);
}

static bh_output_t bh_reference(bh_input_t input)
{
    return (input < 0) ? -1 : 1;
}

static bh_output_t bh_optimized_unsigned_shift(bh_input_t input)
{
    return 1 | -(bh_output_t)((uint64_t)input >> 63);
}

static bh_output_t bh_optimized_arithmetic_shift(bh_input_t input)
{
    return 1 | (input >> 63);
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(INT64_MIN, -1);
    BH_ASSERT_OPT_EQ(INT64_C(-1), -1);
    BH_ASSERT_OPT_EQ(INT64_C(0), 1);
    BH_ASSERT_OPT_EQ(INT64_C(1), 1);
    BH_ASSERT_OPT_EQ(INT64_MAX, 1);

    BH_ASSERT_AGREE(INT64_C(-2));
    BH_ASSERT_AGREE(INT64_C(2));
    BH_ASSERT_AGREE(INT64_C(-4611686018427387904));
    BH_ASSERT_AGREE(INT64_C(4611686018427387904));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
