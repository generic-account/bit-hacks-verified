/*
title = "Test whether a 32-bit integer is non-negative (0/1)"
hack_id = "is_nonnegative_int32"
tags = ["integer", "sign", "bit-shift", "branchless"]
summary = "Tests whether a 32-bit integer is non-negative, returning 1 for zero or positives and 0 for negatives."
contract = "Returns 0 when the input is negative and 1 when the input is zero or positive."
notes = """
This is a non-negativity test, not a signum function.
The classic form flips the high-bit test so that non-negative values map to 1 and negative values map to 0.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign",
]
*/

#include <assert.h>
#include <stdint.h>

typedef int32_t bh_input_t;
typedef int32_t bh_output_t;

static bh_output_t bh_optimized_unsigned_shift(bh_input_t input);

#define BH_IMPLS(X) \
    X("unsigned_shift", bh_optimized_unsigned_shift)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output == 0 || output == 1);
}

static bh_output_t bh_reference(bh_input_t input)
{
    return (input < 0) ? 0 : 1;
}

static bh_output_t bh_optimized_unsigned_shift(bh_input_t input)
{
    return 1 ^ (bh_output_t)((uint32_t)input >> 31);
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(INT32_C(-2147483648), 0);
    BH_ASSERT_OPT_EQ(INT32_C(-1), 0);
    BH_ASSERT_OPT_EQ(INT32_C(0), 1);
    BH_ASSERT_OPT_EQ(INT32_C(1), 1);
    BH_ASSERT_OPT_EQ(INT32_C(2147483647), 1);

    BH_ASSERT_AGREE(INT32_C(-2));
    BH_ASSERT_AGREE(INT32_C(2));
    BH_ASSERT_AGREE(INT32_C(-1073741824));
    BH_ASSERT_AGREE(INT32_C(1073741824));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
