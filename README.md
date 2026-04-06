# bit-hacks-verified

Verifies bit hacks (for converting 32 to 64 bit hacks) and compiles to static website.

Each file has:

- TOML metadata in a C comment at the top
- typedefs for bh_input_t and bh_output_t
- bh_contract
- bh_reference
- bh_optimized
- bh_tests

Shared harness in src/include/bh/

## Build

Use Clang only:

```sh
make warnings
make test
make san-test
make fuzz-smoke
make site
make ci
```

Generated outputs:

- Tests and fuzzers: build/
- Regenerated Jekyll site content: site/

## Sources

https://graphics.stanford.edu/~seander/bithacks.html

https://en.wikipedia.org/wiki/HAKMEM

https://en.wikipedia.org/wiki/Hacker%27s_Delight

https://www.chessprogramming.org/Bit-Twiddling

https://catonmat.net/low-level-bit-hacks

https://leveluppp.ghost.io/advanced-bit-hacks

https://cp-algorithms.com/algebra/bit-manipulation.html

more to come

hi
