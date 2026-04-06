/*
title = "64-bit SWAR popcount"
hack_id = "popcount_swar64"
tags = ["bitcount", "integer", "popcount", "swar"]
summary = "Counts the set bits in a 64-bit word using a classic SWAR reduction."
contract = "Returns the number of one bits in the input as an integer in the inclusive range [0, 64]."
notes = """
The optimized implementation performs lane-wise partial sums and finishes with a multiply-and-shift reduction.
The reference implementation is intentionally direct so differential testing has an obvious oracle.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel",
  "https://en.wikipedia.org/wiki/Hamming_weight",
]
*/

#include <assert.h>
#include <stdint.h>

typedef uint64_t bh_input_t;
typedef uint32_t bh_output_t;

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output <= 64u);
}

static bh_output_t bh_reference(bh_input_t input)
{
    bh_output_t count = 0;

    while (input != 0u) {
        count += (bh_output_t)(input & 1u);
        input >>= 1;
    }

    return count;
}

static bh_output_t bh_optimized(bh_input_t input)
{
    input = input - ((input >> 1) & UINT64_C(0x5555555555555555));
    input = (input & UINT64_C(0x3333333333333333)) +
        ((input >> 2) & UINT64_C(0x3333333333333333));
    input = (input + (input >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f);
    return (bh_output_t)((input * UINT64_C(0x0101010101010101)) >> 56);
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(UINT64_C(0), 0u);
    BH_ASSERT_OPT_EQ(UINT64_C(1), 1u);
    BH_ASSERT_OPT_EQ(UINT64_C(0xffffffffffffffff), 64u);
    BH_ASSERT_OPT_EQ(UINT64_C(0x0123456789abcdef), 32u);
    BH_ASSERT_OPT_EQ(UINT64_C(0x8000000000000000), 1u);

    BH_ASSERT_AGREE(UINT64_C(0x1111111111111111));
    BH_ASSERT_AGREE(UINT64_C(0xaaaaaaaaaaaaaaaa));
    BH_ASSERT_AGREE(UINT64_C(0x5555555555555555));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
