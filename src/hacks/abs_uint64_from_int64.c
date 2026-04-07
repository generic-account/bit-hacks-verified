/*
title = "Absolute value as uint64_t"
hack_id = "abs_uint64_from_int64"
tags = ["integer", "absolute-value", "bit-shift", "branchless"]
summary = "Computes the absolute value of a 64-bit signed integer without branching, producing a uint64_t result."
contract = "Returns the absolute value of the input as uint64_t, including 2^63 for INT64_MIN."
notes = """
The output type is uint64_t so the most negative input is handled without overflow.
Both branchless implementations depend on arithmetic right shift of negative signed integers propagating the sign bit on the target architecture.
The reference implementation uses unsigned negation, which is the portable way to express the absolute value when the result type is unsigned.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs",
]
*/

#include <assert.h>
#include <limits.h>
#include <stdint.h>

typedef int64_t bh_input_t;
typedef uint64_t bh_output_t;

static bh_output_t bh_optimized_add_xor(bh_input_t input);
static bh_output_t bh_optimized_xor_sub(bh_input_t input);

#define BH_IMPLS(X) \
    X("add_xor", bh_optimized_add_xor) \
    X("xor_sub", bh_optimized_xor_sub)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output <= UINT64_C(9223372036854775808));
}

static bh_output_t bh_reference(bh_input_t input)
{
    return (input < 0) ? -(bh_output_t)input : (bh_output_t)input;
}

static bh_output_t bh_optimized_add_xor(bh_input_t input)
{
    const bh_input_t mask = input >> 63;
    return (bh_output_t)((input + mask) ^ mask);
}

static bh_output_t bh_optimized_xor_sub(bh_input_t input)
{
    const bh_input_t mask = input >> 63;
    return (bh_output_t)((input ^ mask) - mask);
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(INT64_MIN, UINT64_C(9223372036854775808));
    BH_ASSERT_OPT_EQ(INT64_C(-1234567890123456789), UINT64_C(1234567890123456789));
    BH_ASSERT_OPT_EQ(INT64_C(-1), UINT64_C(1));
    BH_ASSERT_OPT_EQ(INT64_C(0), UINT64_C(0));
    BH_ASSERT_OPT_EQ(INT64_C(1), UINT64_C(1));
    BH_ASSERT_OPT_EQ(INT64_C(1234567890123456789), UINT64_C(1234567890123456789));
    BH_ASSERT_OPT_EQ(INT64_MAX, UINT64_C(9223372036854775807));

    BH_ASSERT_AGREE(INT64_C(-2));
    BH_ASSERT_AGREE(INT64_C(2));
    BH_ASSERT_AGREE(INT64_C(-4611686018427387904));
    BH_ASSERT_AGREE(INT64_C(4611686018427387904));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
