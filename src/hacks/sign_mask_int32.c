/*
title = "Compute the sign mask of a 32-bit integer (-1/0)"
hack_id = "sign_mask_int32"
tags = ["integer", "sign", "bit-shift", "branchless"]
summary = "Computes a sign mask for a 32-bit integer, returning -1 for negative inputs and 0 otherwise."
contract = "Returns -1 when the input is negative and 0 when the input is zero or positive."
notes = """
This is not a full signum function. It computes the negative mask described in classic bit-hack collections.
The unsigned-shift implementation is portable across ordinary 32-bit two's-complement environments because it shifts an unsigned value and then negates the low bit.
The arithmetic-shift implementation is shorter, but it depends on right-shifting signed negatives propagating the sign bit on the target architecture.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign",
]
*/

#include <assert.h>
#include <limits.h>
#include <stdint.h>

typedef int32_t bh_input_t;
typedef int32_t bh_output_t;

static bh_output_t bh_optimized_unsigned_shift(bh_input_t input);
static bh_output_t bh_optimized_arithmetic_shift(bh_input_t input);

#define BH_IMPLS(X) \
    X("unsigned_shift", bh_optimized_unsigned_shift) \
    X("arithmetic_shift", bh_optimized_arithmetic_shift)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output == 0 || output == -1);
}

static bh_output_t bh_reference(bh_input_t input)
{
    return -(bh_output_t)(input < 0);
}

static bh_output_t bh_optimized_unsigned_shift(bh_input_t input)
{
    return -(bh_output_t)((uint32_t)input >> 31);
}

static bh_output_t bh_optimized_arithmetic_shift(bh_input_t input)
{
    return input >> 31;
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(INT32_C(-2147483648), -1);
    BH_ASSERT_OPT_EQ(INT32_C(-123456789), -1);
    BH_ASSERT_OPT_EQ(INT32_C(-1), -1);
    BH_ASSERT_OPT_EQ(INT32_C(0), 0);
    BH_ASSERT_OPT_EQ(INT32_C(1), 0);
    BH_ASSERT_OPT_EQ(INT32_C(123456789), 0);
    BH_ASSERT_OPT_EQ(INT32_C(2147483647), 0);

    BH_ASSERT_AGREE(INT32_C(-2));
    BH_ASSERT_AGREE(INT32_C(2));
    BH_ASSERT_AGREE(INT32_C(-1073741824));
    BH_ASSERT_AGREE(INT32_C(1073741824));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
