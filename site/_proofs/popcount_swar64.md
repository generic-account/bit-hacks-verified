---
layout: "proof"
title: "64-bit SWAR popcount"
summary: "Counts the set bits in a 64-bit word using a classic SWAR reduction."
hack_id: "popcount_swar64"
tags: ["bitcount", "integer", "popcount", "swar"]
search_keywords: ["popcount", "population count", "hamming weight", "swar", "64-bit"]
source_path: "src/hacks/popcount_swar64.c"
slug: "popcount_swar64"
---

## Summary

Counts the set bits in a 64-bit word using a classic SWAR reduction.

## Contract

Returns the number of one bits in the input as an integer in the inclusive range [0, 64].

## Notes

The optimized implementation performs lane-wise partial sums and finishes with a multiply-and-shift reduction.
The reference implementation is intentionally direct so differential testing has an obvious oracle.

## Verification

- Strict Clang warnings in test and fuzz builds
- Hand-written edge-case assertions in bh_tests
- Deterministic random differential testing in generated test main
- libFuzzer smoke run with address and undefined behavior sanitizers

## Optimized Implementation

```c
bh_output_t bh_optimized(bh_input_t input)
{
    input = input - ((input >> 1) & UINT64_C(0x5555555555555555));
    input = (input & UINT64_C(0x3333333333333333)) +
        ((input >> 2) & UINT64_C(0x3333333333333333));
    input = (input + (input >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f);
    return (bh_output_t)((input * UINT64_C(0x0101010101010101)) >> 56);
}
```

## Reference Implementation

```c
bh_output_t bh_reference(bh_input_t input)
{
    bh_output_t count = 0;

    while (input != 0u) {
        count += (bh_output_t)(input & 1u);
        input >>= 1;
    }

    return count;
}
```

## Source File

`src/hacks/popcount_swar64.c`

## Sources

- https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
- https://en.wikipedia.org/wiki/Hamming_weight
