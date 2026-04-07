/*
title = "Opposite signs"
hack_id = "opposite_signs_int64_pair"
tags = ["integer", "sign", "xor", "branchless"]
summary = "Detects whether two 64-bit integers have opposite signs."
contract = "Returns true when exactly one input is negative, and false otherwise."
notes = """
This trick relies on the sign bit of x ^ y being set exactly when x and y have different sign bits.
It answers a sign-comparison question only. It does not compare magnitudes or arithmetic ordering.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#DetectOppositeSigns",
]
*/

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int64_t x;
    int64_t y;
} bh_input_t;

typedef bool bh_output_t;

static bh_output_t bh_optimized_xor_signbit(bh_input_t input);

#define BH_IMPLS(X) \
    X("xor_signbit", bh_optimized_xor_signbit)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output == false || output == true);
}

static bh_output_t bh_reference(bh_input_t input)
{
    return (input.x < 0) != (input.y < 0);
}

static bh_output_t bh_optimized_xor_signbit(bh_input_t input)
{
    return (input.x ^ input.y) < 0;
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(-1), .y = INT64_C(1) }), true);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(1), .y = INT64_C(-1) }), true);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(0), .y = INT64_C(0) }), false);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(1), .y = INT64_C(2) }), false);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_C(-1), .y = INT64_C(-2) }), false);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_MIN, .y = INT64_C(1) }), true);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .x = INT64_MAX, .y = INT64_C(-1) }), true);

    BH_ASSERT_AGREE(((bh_input_t){ .x = INT64_C(-7), .y = INT64_C(9) }));
    BH_ASSERT_AGREE(((bh_input_t){ .x = INT64_C(7), .y = INT64_C(9) }));
    BH_ASSERT_AGREE(((bh_input_t){ .x = INT64_C(-7), .y = INT64_C(-9) }));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
