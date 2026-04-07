/*
title = "Is a power of two (0/1)"
hack_id = "is_power_of_two_uint64"
tags = ["integer", "power-of-two", "bit-test", "branchless"]
summary = "Tests whether a uint64_t value is a power of two."
contract = "Returns true exactly when the input is one of 1, 2, 4, ..., 2^63."
notes = """
The verified implementation includes the required nonzero check.
The shorter form (v & (v - 1)) == 0 is intentionally not used as an implementation because it incorrectly reports that 0 is a power of two.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2",
]
*/

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t bh_input_t;
typedef bool bh_output_t;

static bh_output_t bh_optimized_and_clear(bh_input_t input);

#define BH_IMPLS(X) \
    X("and_clear", bh_optimized_and_clear)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output == false || output == true);
}

static bh_output_t bh_reference(bh_input_t input)
{
    unsigned count = 0;

    while (input != 0u) {
        count += (unsigned)(input & UINT64_C(1));
        input >>= 1;
    }

    return count == 1u;
}

static bh_output_t bh_optimized_and_clear(bh_input_t input)
{
    return input != 0u && (input & (input - 1u)) == 0u;
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(UINT64_C(0), false);
    BH_ASSERT_OPT_EQ(UINT64_C(1), true);
    BH_ASSERT_OPT_EQ(UINT64_C(2), true);
    BH_ASSERT_OPT_EQ(UINT64_C(3), false);
    BH_ASSERT_OPT_EQ(UINT64_C(4), true);
    BH_ASSERT_OPT_EQ(UINT64_C(5), false);
    BH_ASSERT_OPT_EQ(UINT64_C(8), true);
    BH_ASSERT_OPT_EQ(UINT64_C(9), false);
    BH_ASSERT_OPT_EQ(UINT64_C(0x8000000000000000), true);
    BH_ASSERT_OPT_EQ(UINT64_C(0xffffffffffffffff), false);

    BH_ASSERT_AGREE(UINT64_C(0x0001000000000000));
    BH_ASSERT_AGREE(UINT64_C(0x0001000000000001));
    BH_ASSERT_AGREE(UINT64_C(0x4000000000000000));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
