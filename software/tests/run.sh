#!/bin/sh
set -eu
ulimit -c 0

test_root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_build_dir=$(mktemp -d "${TMPDIR:-/tmp}/catgenius-host-checks.XXXXXX")
test_cc=${CC:-gcc}
trap 'rm -f -- "$test_build_dir/16f877a" "$test_build_dir/16f1939" "$test_build_dir/water-quality"; rmdir -- "$test_build_dir"' 0

"$test_cc" -std=c99 -Wall -Wextra -Werror \
	-g -fsanitize=address,undefined -fno-omit-frame-pointer \
	"$test_root/software/tests/water-quality.c" \
	"$test_root/software/common/waterquality.c" \
	-o "$test_build_dir/water-quality"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
	"$test_build_dir/water-quality"

for test_target in 16f877a 16f1939; do
	case "$test_target" in
		16f877a) test_device=_16F877A ;;
		16f1939) test_device=_16F1939 ;;
	esac
	printf '%s\n' "Checking $test_target with $test_cc"
	"$test_cc" -std=c99 -Wall -Wextra -Wno-pointer-sign \
		-g -fsanitize=address,undefined -fno-omit-frame-pointer \
		-DHW_CATGENIE120 -D"$test_device" \
		-I"$test_root/software/tests/include" \
		"$test_root/software/tests/state-machines.c" \
		"$test_root/software/common/waterquality.c" \
		-o "$test_build_dir/$test_target"
	# No heap allocation is used; LeakSanitizer cannot run under some sandboxes.
	ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
		"$test_build_dir/$test_target"
done
