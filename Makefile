CC := clang
PYTHON ?= python3

ROOT := $(CURDIR)
BUILD_DIR := $(ROOT)/build
TEST_DIR := $(BUILD_DIR)/test
SAN_DIR := $(BUILD_DIR)/san-test
FUZZ_DIR := $(BUILD_DIR)/fuzz
FUZZ_ARTIFACTS_DIR := $(BUILD_DIR)/fuzz-artifacts

HACKS := $(sort $(wildcard src/hacks/*.c))
HACK_IDS := $(patsubst src/hacks/%.c,%,$(HACKS))
TEST_BINS := $(addprefix $(TEST_DIR)/,$(HACK_IDS))
SAN_BINS := $(addprefix $(SAN_DIR)/,$(HACK_IDS))
FUZZ_BINS := $(addprefix $(FUZZ_DIR)/,$(HACK_IDS))
WARNING_TEST_OBJS := $(addprefix $(BUILD_DIR)/warnings/test/,$(addsuffix .o,$(HACK_IDS)))
WARNING_FUZZ_OBJS := $(addprefix $(BUILD_DIR)/warnings/fuzz/,$(addsuffix .o,$(HACK_IDS)))

CPPFLAGS := -Isrc/include
COMMON_CFLAGS := -std=c11 -O2
WARNING_FLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Werror
TEST_CFLAGS := $(COMMON_CFLAGS) $(WARNING_FLAGS)
SAN_CFLAGS := -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls $(WARNING_FLAGS)
FUZZ_CFLAGS := -std=c11 -O1 -g -fsanitize=fuzzer,address,undefined $(WARNING_FLAGS)

.PHONY: all warnings test san-test fuzz-smoke site site-check ci clean

all: ci

warnings: $(WARNING_TEST_OBJS) $(WARNING_FUZZ_OBJS)

test: $(TEST_BINS)
	set -e; for bin in $(TEST_BINS); do "$$bin"; done

san-test: $(SAN_BINS)
	set -e; export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1; \
	for bin in $(SAN_BINS); do "$$bin"; done

fuzz-smoke: $(FUZZ_BINS)
	set -e; export ASAN_OPTIONS=detect_leaks=0; mkdir -p $(FUZZ_ARTIFACTS_DIR); \
	for bin in $(FUZZ_BINS); do \
		name="$$(basename "$$bin")"; \
		"$$bin" -runs=1000 -artifact_prefix=$(FUZZ_ARTIFACTS_DIR)/$$name- ; \
	done

site:
	$(PYTHON) src/scripts/gen_site.py

site-check: site
	@status="$$(git status --porcelain --untracked-files=all -- site/index.md site/search.json site/_hacks site/tags)"; \
	if [ -n "$$status" ]; then \
		printf '%s\n' "$$status"; \
		echo "Generated site files are out of date; run 'make site' and commit the results."; \
		exit 1; \
	fi

ci: warnings test san-test fuzz-smoke site-check

clean:
	rm -rf $(BUILD_DIR)

$(TEST_DIR)/%: src/hacks/%.c | $(TEST_DIR)
	$(CC) $(CPPFLAGS) $(TEST_CFLAGS) -DBH_MODE_TEST $< -o $@

$(SAN_DIR)/%: src/hacks/%.c | $(SAN_DIR)
	$(CC) $(CPPFLAGS) $(SAN_CFLAGS) -DBH_MODE_TEST $< -o $@

$(FUZZ_DIR)/%: src/hacks/%.c | $(FUZZ_DIR)
	$(CC) $(CPPFLAGS) $(FUZZ_CFLAGS) -DBH_MODE_FUZZ $< -o $@

$(BUILD_DIR)/warnings/test/%.o: src/hacks/%.c | $(BUILD_DIR)/warnings/test
	$(CC) $(CPPFLAGS) $(TEST_CFLAGS) -DBH_MODE_TEST -c $< -o $@

$(BUILD_DIR)/warnings/fuzz/%.o: src/hacks/%.c | $(BUILD_DIR)/warnings/fuzz
	$(CC) $(CPPFLAGS) $(FUZZ_CFLAGS) -DBH_MODE_FUZZ -c $< -o $@

$(TEST_DIR) $(SAN_DIR) $(FUZZ_DIR) $(BUILD_DIR)/warnings/test $(BUILD_DIR)/warnings/fuzz:
	mkdir -p $@
