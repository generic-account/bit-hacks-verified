CC := clang
PYTHON ?= python3

ROOT := $(CURDIR)
BUILD_DIR := $(ROOT)/build
TEST_DIR := $(BUILD_DIR)/test
SAN_DIR := $(BUILD_DIR)/san-test
FUZZ_DIR := $(BUILD_DIR)/fuzz
FUZZ_ARTIFACTS_DIR := $(BUILD_DIR)/fuzz-artifacts
LOG_DIR := $(BUILD_DIR)/logs

HACKS := $(sort $(wildcard src/hacks/*.c))

CPPFLAGS := -Isrc/include
COMMON_CFLAGS := -std=c11 -O2
WARNING_FLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Werror
TEST_CFLAGS := $(COMMON_CFLAGS) $(WARNING_FLAGS)
SAN_CFLAGS := -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls $(WARNING_FLAGS)
FUZZ_CFLAGS := -std=c11 -O1 -g -fsanitize=fuzzer,address,undefined $(WARNING_FLAGS)

.PHONY: all warnings test san-test fuzz-smoke site site-check ci clean

all: ci

warnings:
	@set -eu; \
	mkdir -p $(BUILD_DIR)/warnings/test $(BUILD_DIR)/warnings/fuzz $(LOG_DIR); \
	for src in $(HACKS); do \
		name="$$(basename "$$src" .c)"; \
		log="$(LOG_DIR)/warnings-$$name.log"; \
		if $(CC) $(CPPFLAGS) $(TEST_CFLAGS) -DBH_MODE_TEST -c "$$src" -o "$(BUILD_DIR)/warnings/test/$$name.o" >"$$log" 2>&1 && \
		   $(CC) $(CPPFLAGS) $(FUZZ_CFLAGS) -DBH_MODE_FUZZ -c "$$src" -o "$(BUILD_DIR)/warnings/fuzz/$$name.o" >>"$$log" 2>&1; then \
			rm -f "$$log"; \
		else \
			cat "$$log"; \
			exit 1; \
		fi; \
	done

test:
	@set -eu; \
	mkdir -p $(TEST_DIR) $(LOG_DIR); \
	for src in $(HACKS); do \
		name="$$(basename "$$src" .c)"; \
		bin="$(TEST_DIR)/$$name"; \
		log="$(LOG_DIR)/test-$$name.log"; \
		if $(CC) $(CPPFLAGS) $(TEST_CFLAGS) -DBH_MODE_TEST "$$src" -o "$$bin" >"$$log" 2>&1 && \
		   "$$bin" >>"$$log" 2>&1; then \
			rm -f "$$log"; \
		else \
			cat "$$log"; \
			exit 1; \
		fi; \
	done

san-test:
	@set -eu; \
	mkdir -p $(SAN_DIR) $(LOG_DIR); \
	for src in $(HACKS); do \
		name="$$(basename "$$src" .c)"; \
		bin="$(SAN_DIR)/$$name"; \
		log="$(LOG_DIR)/san-test-$$name.log"; \
		if $(CC) $(CPPFLAGS) $(SAN_CFLAGS) -DBH_MODE_TEST "$$src" -o "$$bin" >"$$log" 2>&1 && \
		   ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 "$$bin" >>"$$log" 2>&1; then \
			rm -f "$$log"; \
		else \
			cat "$$log"; \
			exit 1; \
		fi; \
	done

fuzz-smoke:
	@set -eu; \
	mkdir -p $(FUZZ_DIR) $(FUZZ_ARTIFACTS_DIR) $(LOG_DIR); \
	for src in $(HACKS); do \
		name="$$(basename "$$src" .c)"; \
		bin="$(FUZZ_DIR)/$$name"; \
		log="$(LOG_DIR)/fuzz-smoke-$$name.log"; \
		if $(CC) $(CPPFLAGS) $(FUZZ_CFLAGS) -DBH_MODE_FUZZ "$$src" -o "$$bin" >"$$log" 2>&1 && \
		   ASAN_OPTIONS=detect_leaks=0 "$$bin" -runs=1000 -artifact_prefix=$(FUZZ_ARTIFACTS_DIR)/$$name- >>"$$log" 2>&1; then \
			rm -f "$$log"; \
		else \
			cat "$$log"; \
			exit 1; \
		fi; \
	done

site:
	@echo "== site =="; \
	if $(PYTHON) src/scripts/gen_site.py; then \
		echo "OK   site      generated"; \
	else \
		echo "BAD  site      generated"; \
		exit 1; \
	fi

site-check: site
	@status="$$(git status --porcelain --untracked-files=all -- site/index.md site/search.json site/_hacks site/tags)"; \
	if [ -n "$$status" ]; then \
		echo "BAD  site-check generated"; \
		printf '%s\n' "$$status"; \
		echo "Generated site files are out of date; run 'make site' and commit the results."; \
		exit 1; \
	fi; \
	echo "OK   site-check generated"

ci:
	@set -eu; \
	mkdir -p $(BUILD_DIR)/warnings/test $(BUILD_DIR)/warnings/fuzz $(TEST_DIR) $(SAN_DIR) $(FUZZ_DIR) $(FUZZ_ARTIFACTS_DIR) $(LOG_DIR); \
	printf '%-28s | %-8s | %-8s | %-8s | %-8s\n' "hack" "warnings" "test" "san" "fuzz"; \
	printf '%-28s-+-%-8s-+-%-8s-+-%-8s-+-%-8s\n' "----------------------------" "--------" "--------" "--------" "--------"; \
	for src in $(HACKS); do \
		name="$$(basename "$$src" .c)"; \
		warnings="OK"; \
		test_status="OK"; \
		san_status="OK"; \
		fuzz_status="OK"; \
		log="$(LOG_DIR)/warnings-$$name.log"; \
		if ! ( $(CC) $(CPPFLAGS) $(TEST_CFLAGS) -DBH_MODE_TEST -c "$$src" -o "$(BUILD_DIR)/warnings/test/$$name.o" >"$$log" 2>&1 && \
		       $(CC) $(CPPFLAGS) $(FUZZ_CFLAGS) -DBH_MODE_FUZZ -c "$$src" -o "$(BUILD_DIR)/warnings/fuzz/$$name.o" >>"$$log" 2>&1 ); then \
			warnings="BAD"; \
		fi; \
		if [ "$$warnings" = "OK" ]; then rm -f "$$log"; fi; \
		log="$(LOG_DIR)/test-$$name.log"; \
		if [ "$$warnings" = "OK" ] && $(CC) $(CPPFLAGS) $(TEST_CFLAGS) -DBH_MODE_TEST "$$src" -o "$(TEST_DIR)/$$name" >"$$log" 2>&1 && \
		   "$(TEST_DIR)/$$name" >>"$$log" 2>&1; then \
			rm -f "$$log"; \
		else \
			test_status="BAD"; \
		fi; \
		log="$(LOG_DIR)/san-test-$$name.log"; \
		if [ "$$warnings" = "OK" ] && $(CC) $(CPPFLAGS) $(SAN_CFLAGS) -DBH_MODE_TEST "$$src" -o "$(SAN_DIR)/$$name" >"$$log" 2>&1 && \
		   ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 "$(SAN_DIR)/$$name" >>"$$log" 2>&1; then \
			rm -f "$$log"; \
		else \
			san_status="BAD"; \
		fi; \
		log="$(LOG_DIR)/fuzz-smoke-$$name.log"; \
		if [ "$$warnings" = "OK" ] && $(CC) $(CPPFLAGS) $(FUZZ_CFLAGS) -DBH_MODE_FUZZ "$$src" -o "$(FUZZ_DIR)/$$name" >"$$log" 2>&1 && \
		   ASAN_OPTIONS=detect_leaks=0 "$(FUZZ_DIR)/$$name" -runs=1000 -artifact_prefix=$(FUZZ_ARTIFACTS_DIR)/$$name- >>"$$log" 2>&1; then \
			rm -f "$$log"; \
		else \
			fuzz_status="BAD"; \
		fi; \
		printf '%-28s | %-8s | %-8s | %-8s | %-8s\n' "$$name" "$$warnings" "$$test_status" "$$san_status" "$$fuzz_status"; \
		for stage in warnings test san-test fuzz-smoke; do \
			case "$$stage" in \
				warnings) status="$$warnings"; log="$(LOG_DIR)/warnings-$$name.log" ;; \
				test) status="$$test_status"; log="$(LOG_DIR)/test-$$name.log" ;; \
				san-test) status="$$san_status"; log="$(LOG_DIR)/san-test-$$name.log" ;; \
				fuzz-smoke) status="$$fuzz_status"; log="$(LOG_DIR)/fuzz-smoke-$$name.log" ;; \
			esac; \
			if [ "$$status" = "BAD" ]; then \
				echo; \
				echo "Failure in $$stage for $$name:"; \
				cat "$$log"; \
				exit 1; \
			fi; \
		done; \
	done; \
	if $(PYTHON) src/scripts/gen_site.py >/dev/null 2>&1; then \
		site_status="OK"; \
	else \
		site_status="BAD"; \
	fi; \
	printf '%-28s | %-8s | %-8s | %-8s | %-8s\n' "site" "$$site_status" "-" "-" "-"; \
	if [ "$$site_status" != "OK" ]; then \
		echo "Failure in site generation."; \
		exit 1; \
	fi; \
	status="$$(git status --porcelain --untracked-files=all -- site/index.md site/search.json site/_hacks site/tags)"; \
	printf '%-28s | %-8s | %-8s | %-8s | %-8s\n' "site-check" "$$( [ -z "$$status" ] && echo OK || echo BAD )" "-" "-" "-"; \
	if [ -n "$$status" ]; then \
		echo; \
		printf '%s\n' "$$status"; \
		echo "Generated site files are out of date; run 'make site' and commit the results."; \
		exit 1; \
	fi

clean:
	rm -rf $(BUILD_DIR)
