#ifndef BH_HARNESS_H
#define BH_HARNESS_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BH_RANDOM_TRIALS
#define BH_RANDOM_TRIALS 10000u
#endif

static void bh_contract(bh_input_t input, bh_output_t output);
static bh_output_t bh_reference(bh_input_t input);
static bh_output_t bh_optimized(bh_input_t input);
static void bh_tests(void);
static bool bh_decode_input(const uint8_t *data, size_t size, bh_input_t *out);
static bool bh_output_eq(bh_output_t lhs, bh_output_t rhs);

#if defined(__clang__) || defined(__GNUC__)
#define BH_UNUSED __attribute__((unused))
#else
#define BH_UNUSED
#endif

#define BH_DEFINE_TRIVIAL_INPUT_DECODER()                                            \
    static bool BH_UNUSED bh_decode_input(const uint8_t *data, size_t size,          \
        bh_input_t *out)                                                              \
    {                                                                                 \
        if (size != sizeof(*out)) {                                                   \
            return false;                                                             \
        }                                                                             \
        memcpy(out, data, sizeof(*out));                                              \
        return true;                                                                  \
    }

#define BH_DEFINE_TRIVIAL_OUTPUT_EQ()                                                 \
    static bool bh_output_eq(bh_output_t lhs, bh_output_t rhs)                        \
    {                                                                                 \
        return memcmp(&lhs, &rhs, sizeof(lhs)) == 0;                                  \
    }

#define BH_ASSERT_AGREE(input_value)                                                  \
    do {                                                                              \
        const bh_input_t bh_input__ = (input_value);                                  \
        const bh_output_t bh_ref__ = bh_reference(bh_input__);                        \
        const bh_output_t bh_opt__ = bh_optimized(bh_input__);                        \
        bh_contract(bh_input__, bh_ref__);                                            \
        bh_contract(bh_input__, bh_opt__);                                            \
        if (!bh_output_eq(bh_ref__, bh_opt__)) {                                      \
            fprintf(stderr, "reference/optimized mismatch at %s:%d\n",                \
                __FILE__, __LINE__);                                                  \
            abort();                                                                  \
        }                                                                             \
    } while (0)

#define BH_ASSERT_OPT_EQ(input_value, expected_value)                                 \
    do {                                                                              \
        const bh_input_t bh_input__ = (input_value);                                  \
        const bh_output_t bh_expected__ = (expected_value);                           \
        const bh_output_t bh_actual__ = bh_optimized(bh_input__);                     \
        bh_contract(bh_input__, bh_actual__);                                         \
        if (!bh_output_eq(bh_actual__, bh_expected__)) {                              \
            fprintf(stderr, "optimized output mismatch at %s:%d\n",                   \
                __FILE__, __LINE__);                                                  \
            abort();                                                                  \
        }                                                                             \
    } while (0)

#if defined(BH_MODE_TEST)
static uint64_t bh_prng_next(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

static bh_input_t bh_random_input(uint64_t *state)
{
    bh_input_t input;
    unsigned char *bytes = (unsigned char *)&input;
    size_t offset = 0;

    while (offset < sizeof(input)) {
        uint64_t word = bh_prng_next(state);
        size_t chunk = sizeof(word);
        if (chunk > sizeof(input) - offset) {
            chunk = sizeof(input) - offset;
        }
        memcpy(bytes + offset, &word, chunk);
        offset += chunk;
    }

    return input;
}

static void bh_run_random_differential(void)
{
    uint64_t state = UINT64_C(0x6a09e667f3bcc909);
    unsigned i;

    for (i = 0; i < BH_RANDOM_TRIALS; ++i) {
        const bh_input_t input = bh_random_input(&state);
        const bh_output_t ref = bh_reference(input);
        const bh_output_t opt = bh_optimized(input);
        bh_contract(input, ref);
        bh_contract(input, opt);
        if (!bh_output_eq(ref, opt)) {
            fprintf(stderr, "random differential mismatch after %u cases\n", i + 1u);
            abort();
        }
    }
}

int main(void)
{
    bh_tests();
    bh_run_random_differential();
    return 0;
}
#endif

#if defined(BH_MODE_FUZZ)
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
static void (*const BH_UNUSED bh_tests_anchor_)(void) = bh_tests;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    bh_input_t input;
    bh_output_t ref;
    bh_output_t opt;

    if (!bh_decode_input(data, size, &input)) {
        return 0;
    }

    ref = bh_reference(input);
    opt = bh_optimized(input);
    bh_contract(input, ref);
    bh_contract(input, opt);
    if (!bh_output_eq(ref, opt)) {
        abort();
    }

    return 0;
}
#endif

#endif
