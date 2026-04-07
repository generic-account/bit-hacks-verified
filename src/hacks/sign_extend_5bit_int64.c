/*
title = "Sign-extend a 5-bit integer"
hack_id = "sign_extend_5bit_int64"
tags = ["integer", "sign-extension", "bitfield", "constant-width"]
summary = "Sign-extends the low 5 bits of an integer into a full signed result."
contract = "Interprets the low 5 bits of the input as a signed two's-complement integer and returns the corresponding value in the range [-16, 15]."
notes = """
Only the low 5 bits of the input participate in the result.
The verified implementation uses a signed bit-field assignment, matching the classic C trick for constant-width sign extension.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend",
]
*/

#include <assert.h>
#include <stdint.h>

typedef uint64_t bh_input_t;
typedef int64_t bh_output_t;

static bh_output_t bh_optimized_bitfield(bh_input_t input);

#define BH_IMPLS(X) \
    X("bitfield", bh_optimized_bitfield)

#include "bh/harness.h"

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    assert(output >= -16 && output <= 15);
}

static bh_output_t bh_reference(bh_input_t input)
{
    const uint64_t raw = input & UINT64_C(0x1f);

    if ((raw & UINT64_C(0x10)) != 0u) {
        return (bh_output_t)(raw - UINT64_C(0x20));
    }
    return (bh_output_t)raw;
}

static bh_output_t bh_optimized_bitfield(bh_input_t input)
{
    struct {
        signed int x : 5;
    } value;

    value.x = (signed int)input;
    return (bh_output_t)value.x;
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(UINT64_C(0x00), 0);
    BH_ASSERT_OPT_EQ(UINT64_C(0x01), 1);
    BH_ASSERT_OPT_EQ(UINT64_C(0x0f), 15);
    BH_ASSERT_OPT_EQ(UINT64_C(0x10), -16);
    BH_ASSERT_OPT_EQ(UINT64_C(0x1d), -3);
    BH_ASSERT_OPT_EQ(UINT64_C(0x1f), -1);
    BH_ASSERT_OPT_EQ(UINT64_C(0x3f), -1);
    BH_ASSERT_OPT_EQ(UINT64_C(0xffffffffffffffff), -1);

    BH_ASSERT_AGREE(UINT64_C(0x12));
    BH_ASSERT_AGREE(UINT64_C(0x05));
    BH_ASSERT_AGREE(UINT64_C(0x123456789abcdef0));
}

BH_DEFINE_TRIVIAL_INPUT_DECODER()
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
