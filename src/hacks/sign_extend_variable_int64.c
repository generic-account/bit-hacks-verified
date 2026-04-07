/*
title = "Sign-extend from a variable bit width"
hack_id = "sign_extend_variable_int64"
tags = ["integer", "sign-extension", "variable-width", "branchless"]
summary = "Sign-extends a value represented in a runtime-selected bit width into a full int64_t."
contract = "For bits in the inclusive range [1, 64], interprets the low `bits` bits of `x` as a signed two's-complement integer and returns the corresponding int64_t value."
notes = """
The verified implementations mask away unused high bits before extending the sign.
The arithmetic-shift form ((x << m) >> m) is intentionally not included as a verified implementation because its behavior depends on signed right shift and on the treatment of the source value before shifting.
"""
sources = [
  "https://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend",
]
*/

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t bits;
    uint64_t x;
} bh_input_t;

typedef int64_t bh_output_t;

static bh_output_t bh_optimized_xor_sub(bh_input_t input);
static bh_output_t bh_optimized_negate_or(bh_input_t input);
#if defined(BH_MODE_TEST)
static bh_input_t bh_random_valid_input(uint64_t *state);
static uint64_t bh_random_word(uint64_t *state);
#endif

#define BH_IMPLS(X) \
    X("xor_sub", bh_optimized_xor_sub) \
    X("negate_or", bh_optimized_negate_or)
#define BH_RANDOM_INPUT(state_ptr) bh_random_valid_input(state_ptr)

#include "bh/harness.h"

static uint64_t bh_low_mask(uint32_t bits)
{
    if (bits >= 64u) {
        return UINT64_MAX;
    }
    return (UINT64_C(1) << bits) - UINT64_C(1);
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

    input.bits = (uint32_t)((bh_random_word(state) % UINT64_C(64)) + UINT64_C(1));
    input.x = bh_random_word(state);
    return input;
}
#endif

static bool BH_UNUSED bh_decode_input(const uint8_t *data, size_t size, bh_input_t *out)
{
    if (size != sizeof(*out)) {
        return false;
    }
    memcpy(out, data, sizeof(*out));
    return out->bits >= 1u && out->bits <= 64u;
}

static void bh_contract(bh_input_t input, bh_output_t output)
{
    assert(input.bits >= 1u && input.bits <= 64u);
    (void)output;
}

static bh_output_t bh_reference(bh_input_t input)
{
    const uint64_t masked = input.x & bh_low_mask(input.bits);

    if (input.bits == 64u) {
        return (bh_output_t)masked;
    }
    if ((masked & (UINT64_C(1) << (input.bits - 1u))) != 0u) {
        return (bh_output_t)(masked - (UINT64_C(1) << input.bits));
    }
    return (bh_output_t)masked;
}

static bh_output_t bh_optimized_xor_sub(bh_input_t input)
{
    const uint64_t masked = input.x & bh_low_mask(input.bits);
    const uint64_t sign_bit = UINT64_C(1) << (input.bits - 1u);

    return (bh_output_t)((masked ^ sign_bit) - sign_bit);
}

static bh_output_t bh_optimized_negate_or(bh_input_t input)
{
    const uint64_t masked = input.x & bh_low_mask(input.bits);
    const uint64_t sign_bit = UINT64_C(1) << (input.bits - 1u);

    return (bh_output_t)(-(masked & sign_bit) | masked);
}

static void bh_tests(void)
{
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 1u, .x = UINT64_C(0) }), 0);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 1u, .x = UINT64_C(1) }), -1);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 5u, .x = UINT64_C(0x0f) }), 15);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 5u, .x = UINT64_C(0x10) }), -16);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 5u, .x = UINT64_C(0x1d) }), -3);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 5u, .x = UINT64_C(0x3d) }), -3);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 8u, .x = UINT64_C(0xfd) }), -3);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 8u, .x = UINT64_C(0x7f) }), 127);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 64u, .x = UINT64_C(0xffffffffffffffff) }), -1);
    BH_ASSERT_OPT_EQ(((bh_input_t){ .bits = 64u, .x = UINT64_C(0x8000000000000000) }), INT64_MIN);

    BH_ASSERT_AGREE(((bh_input_t){ .bits = 13u, .x = UINT64_C(0x1abc) }));
    BH_ASSERT_AGREE(((bh_input_t){ .bits = 31u, .x = UINT64_C(0x40000000) }));
    BH_ASSERT_AGREE(((bh_input_t){ .bits = 63u, .x = UINT64_C(0x4000000000000001) }));
}
BH_DEFINE_TRIVIAL_OUTPUT_EQ()
