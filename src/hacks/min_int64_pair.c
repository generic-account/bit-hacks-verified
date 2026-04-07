/*
title = "Compute the minimum of two 64-bit integers"
hack_id = "min_int64_pair"
tags = ["integer", "minimum", "comparison", "branchless"]
summary = "Computes the minimum of two 64-bit integers without branching."
contract = "Returns the smaller of the two input values."
notes = """
The xor/select implementation avoids overflow and does not require any precondition beyond ordinary int64_t arithmetic and comparisons.
The classic quick-and-dirty subtraction form is intentionally not used here because it needs the stronger precondition that x - y is representable as int64_t.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax",
]
*/

#include <assert.h>
#include <stdint.h>

typedef struct {
    int64_t x;
    int64_t y;
} bh_input_t;

typedef int64_t bh_output_t;

static bh_output_t bh_optimized_xor_select(bh_input_t input);

#define BH_IMPLS(X) \
    X("xor_select", bh_optimized_xor_select)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    assert(output == input.x || output == input.y);
    assert(output <= input.x);
    assert(output <= input.y);
}

static bh_output_t bh_reference(bh_input_t input)
{
    return (input.x < input.y) ? input.x : input.y;
}

static bh_output_t bh_optimized_xor_select(bh_input_t input)
{
    return input.y ^ ((input.x ^ input.y) & -(bh_output_t)(input.x < input.y));
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(0), .y = INT64_C(0) }), INT64_C(0));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(1), .y = INT64_C(2) }), INT64_C(1));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(2), .y = INT64_C(1) }), INT64_C(1));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(-1), .y = INT64_C(1) }), INT64_C(-1));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(1), .y = INT64_C(-1) }), INT64_C(-1));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_MIN, .y = INT64_MAX }), INT64_MIN);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_MAX, .y = INT64_MIN }), INT64_MIN);

    BH_ASSERT_AGREE(((bh_input_t){ .x = INT64_C(-7), .y = INT64_C(-9) }));
    BH_ASSERT_AGREE(((bh_input_t){ .x = INT64_C(7), .y = INT64_C(9) }));
    BH_ASSERT_AGREE(((bh_input_t){ .x = INT64_C(-7), .y = INT64_C(9) }));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
