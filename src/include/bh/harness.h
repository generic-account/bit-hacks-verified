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

#ifndef BH_RANDOM_INPUT
#define BH_RANDOM_INPUT(state_ptr) bh_default_random_input(state_ptr)
#endif

static void bh_contract(bh_input_t input, bh_output_t output);
static bh_output_t bh_reference(bh_input_t input);
static void bh_tests(void);
static bool bh_decode_input(const uint8_t *data, size_t size, bh_input_t *out);
static bool bh_output_eq(bh_output_t lhs, bh_output_t rhs);

#ifndef BH_IMPLS
static bh_output_t bh_optimized(bh_input_t input);
#define BH_IMPLS(X) X("optimized", bh_optimized)
#endif

typedef bh_output_t (*bh_impl_fn_t)(bh_input_t input);

typedef struct bh_impl_desc {
    const char *name;
    bh_impl_fn_t fn;
} bh_impl_desc_t;

#define BH_IMPL_DESC_ENTRY(name, fn) { name, fn },

static const bh_impl_desc_t bh_impls[] = {
    BH_IMPLS(BH_IMPL_DESC_ENTRY)
};

enum {
    bh_impl_count = (int)(sizeof(bh_impls) / sizeof(bh_impls[0]))
};

#if defined(__clang__) || defined(__GNUC__)
#define BH_UNUSED __attribute__((unused))
#else
#define BH_UNUSED
#endif

static void bh_fail_mismatch(const char *kind, const char *impl_name,
    const char *file, int line)
{
    fprintf(stderr, "%s mismatch for %s at %s:%d\n", kind, impl_name, file, line);
    abort();
}

static void bh_assert_impl_agrees_at(const char *file, int line,
    const bh_impl_desc_t *impl, bh_input_t input)
{
    const bh_output_t ref = bh_reference(input);
    const bh_output_t actual = impl->fn(input);

    bh_contract(input, ref);
    bh_contract(input, actual);
    if (!bh_output_eq(ref, actual)) {
        bh_fail_mismatch("reference/optimized", impl->name, file, line);
    }
}

static void bh_assert_all_agree_at(const char *file, int line, bh_input_t input)
{
    int i;

    for (i = 0; i < bh_impl_count; ++i) {
        bh_assert_impl_agrees_at(file, line, &bh_impls[i], input);
    }
}

static void bh_assert_impl_eq_at(const char *file, int line,
    const bh_impl_desc_t *impl, bh_input_t input, bh_output_t expected)
{
    const bh_output_t actual = impl->fn(input);

    bh_contract(input, actual);
    if (!bh_output_eq(actual, expected)) {
        bh_fail_mismatch("optimized/expected", impl->name, file, line);
    }
}

static void bh_assert_all_eq_at(const char *file, int line,
    bh_input_t input, bh_output_t expected)
{
    int i;

    for (i = 0; i < bh_impl_count; ++i) {
        bh_assert_impl_eq_at(file, line, &bh_impls[i], input, expected);
    }
}

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
        bh_assert_all_agree_at(__FILE__, __LINE__, bh_input__);                       \
    } while (0)

#define BH_ASSERT_OPT_EQ(input_value, expected_value)                                 \
    do {                                                                              \
        const bh_input_t bh_input__ = (input_value);                                  \
        const bh_output_t bh_expected__ = (expected_value);                           \
        bh_assert_all_eq_at(__FILE__, __LINE__, bh_input__, bh_expected__);           \
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

static bh_input_t BH_UNUSED bh_default_random_input(uint64_t *state)
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
        const bh_input_t input = BH_RANDOM_INPUT(&state);
        bh_assert_all_agree_at(__FILE__, __LINE__, input);
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

    if (!bh_decode_input(data, size, &input)) {
        return 0;
    }

    bh_assert_all_agree_at(__FILE__, __LINE__, input);

    return 0;
}
#endif

#endif
