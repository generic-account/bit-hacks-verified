# bit-hacks-verified

Verifies bit hacks and publishes them as a static site.

Each file in `src/hacks/` has:

- TOML metadata in a C comment at the top
- `typedef`s for `bh_input_t` and `bh_output_t`
- `bh_contract`
- `bh_reference`
- one or more optimized implementations declared in `BH_IMPLS(X)`
- `bh_tests`

Shared harness code lives in `src/include/bh/`.

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

The site content under `site/` is generated from the hack files.
