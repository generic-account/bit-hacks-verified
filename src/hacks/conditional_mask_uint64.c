/*
title = "Conditionally set or clear bits"
hack_id = "conditional_mask_uint64"
tags = ["integer", "bit-mask", "conditional", "branchless"]
summary = "Conditionally sets or clears the masked bits of a uint64_t word without branching."
contract = "Returns a copy of w where the bits selected by m are set when f is true and cleared when f is false. Bits outside m are unchanged."
notes = """
Both verified implementations compute an all-zero or all-one mask from the boolean flag and then update only the selected bits.
The first form is the classic xor-based trick. The second form is the superscalar-friendly blend form.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching",
]
*/

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    bool f;
    uint64_t m;
    uint64_t w;
} bh_input_t;

typedef uint64_t bh_output_t;

static bh_output_t bh_optimized_xor_blend(bh_input_t input);
static bh_output_t bh_optimized_and_or_blend(bh_input_t input);
#if defined(BH_MODE_TEST)
static bh_input_t bh_random_valid_input(uint64_t *state);
static uint64_t bh_random_word(uint64_t *state);
#endif

#define BH_IMPLS(X) \
    X("xor_blend", bh_optimized_xor_blend) \
    X("and_or_blend", bh_optimized_and_or_blend)
#define BH_RANDOM_INPUT(state_ptr) bh_random_valid_input(state_ptr)

#include "bh/harness.h"

static uint64_t bh_flag_mask(bool flag)
{
    return UINT64_C(0) - (uint64_t)(flag ? 1u : 0u);
}

#if defined(BH_MODE_TEST)
static uint64_t bh_random_word(uint64_t *state)
{
    uint64_t x = *state;

    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

static bh_input_t bh_random_valid_input(uint64_t *state)
{
    bh_input_t input;

    input.f = (bh_random_word(state) & UINT64_C(1)) != 0u;
    input.m = bh_random_word(state);
    input.w = bh_random_word(state);
    return input;
}
#endif

static bool BH_UNUSED bh_decode_input(const uint8_t *data, size_t size, bh_input_t *out)
{
    uint64_t m;
    uint64_t w;

    if (size != 17u) {
        return false;
    }
    memcpy(&m, data + 1, sizeof(m));
    memcpy(&w, data + 1 + sizeof(m), sizeof(w));
    out->f = data[0] != 0u;
    out->m = m;
    out->w = w;
    return true;
}

static void bh_contract(bh_input_t input, bh_output_t output)
{
    (void)input;
    (void)output;
}

static bh_output_t bh_reference(bh_input_t input)
{
    if (input.f) {
        return input.w | input.m;
    }
    return input.w & ~input.m;
}

static bh_output_t bh_optimized_xor_blend(bh_input_t input)
{
    const uint64_t flag_mask = bh_flag_mask(input.f);

    return input.w ^ ((flag_mask ^ input.w) & input.m);
}

static bh_output_t bh_optimized_and_or_blend(bh_input_t input)
{
    const uint64_t flag_mask = bh_flag_mask(input.f);

    return (input.w & ~input.m) | (flag_mask & input.m);
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = false, .m = UINT64_C(0), .w = UINT64_C(0) }), UINT64_C(0));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = true, .m = UINT64_C(0), .w = UINT64_C(0x1234) }), UINT64_C(0x1234));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = true, .m = UINT64_C(0x0f), .w = UINT64_C(0x30) }), UINT64_C(0x3f));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = false, .m = UINT64_C(0x0f), .w = UINT64_C(0x3f) }), UINT64_C(0x30));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = true, .m = UINT64_C(0xffff0000), .w = UINT64_C(0x12345678) }), UINT64_C(0xffff5678));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = false, .m = UINT64_C(0xffff0000), .w = UINT64_C(0x12345678) }), UINT64_C(0x00005678));
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = true, .m = UINT64_MAX, .w = UINT64_C(0) }), UINT64_MAX);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .f = false, .m = UINT64_MAX, .w = UINT64_MAX }), UINT64_C(0));

    BH_ASSERT_AGREE(((bh_input_t){ .f = true, .m = UINT64_C(0x00ff00ff00ff00ff), .w = UINT64_C(0x5500550055005500) }));
    BH_ASSERT_AGREE(((bh_input_t){ .f = false, .m = UINT64_C(0x00ff00ff00ff00ff), .w = UINT64_C(0x5500550055005500) }));
    BH_ASSERT_AGREE(((bh_input_t){ .f = true, .m = UINT64_C(0xaaaaaaaaaaaaaaaa), .w = UINT64_C(0x1111111111111111) }));
}

BH_DEFINE_TRIVIAL_OUTPUT_EQ()
