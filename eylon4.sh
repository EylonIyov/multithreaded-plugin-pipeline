#!/usr/bin/env bash
set -e

# ─────────────────────────────────────────────────────────────────────────────
#  Colors & helpers
# ─────────────────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[TEST]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# ─────────────────────────────────────────────────────────────────────────────
#  1. Build everything
# ─────────────────────────────────────────────────────────────────────────────
print_status "Building project..."
./build.sh
print_status "Build succeeded."

# ─────────────────────────────────────────────────────────────────────────────
#  2. Base case: single-plugin pipeline (logger only)
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: logger only"
INPUT="hello"
EXPECTED="[logger] hello"
ACTUAL=$(printf "%s\n<END>\n" "$INPUT" | ./output/analyzer 5 logger | grep "^\[logger\]")
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
    print_status "PASS"
else
    print_error "FAIL (logger only): expected '$EXPECTED', got '$ACTUAL'"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 3. Two-stage pipeline: uppercaser → logger
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: uppercaser + logger"
INPUT="Hello, World!"
EXPECTED="[logger] HELLO, WORLD!"
ACTUAL=$(printf "%s\n<END>\n" "$INPUT" | ./output/analyzer 10 uppercaser logger | grep "^\[logger\]")
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
    print_status "PASS"
else
    print_error "FAIL (uppercaser+logger): expected '$EXPECTED', got '$ACTUAL'"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 4. Edge case: empty string
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: empty string → logger"
EXPECTED="[logger] "
ACTUAL=$(printf "\n<END>\n" | ./output/analyzer 3 logger | grep "^\[logger\]")
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
    print_status "PASS"
else
    print_error "FAIL (empty string): expected '$EXPECTED', got '$ACTUAL'"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 5. Invalid usage: no arguments
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: missing arguments"
if ./output/analyzer >/dev/null 2>&1; then
    print_error "FAIL (missing args): expected non-zero exit code"
else
    print_status "PASS"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 6. Invalid queue size (non-integer)
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: non-numeric queue size"
if ./output/analyzer abc uppercaser >/dev/null 2>&1; then
    print_error "FAIL (non-numeric queue): expected non-zero exit code"
else
    print_status "PASS"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 7. Invalid plugin name
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: invalid plugin name"
if ./output/analyzer 5 doesnotexist >/dev/null 2>&1; then
    print_error "FAIL (invalid plugin): expected non-zero exit code"
else
    print_status "PASS"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 8. Longer string (stays within 1024-char limit)
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: long string → logger"
LONG=$(head -c 200 < /dev/zero | tr '\0' 'x')    # 200-char string of 'x'
EXPECTED="[logger] $LONG"
ACTUAL=$(printf "%s\n<END>\n" "$LONG" | ./output/analyzer 50 logger | grep "^\[logger\]")
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
    print_status "PASS"
else
    print_error "FAIL (long string): output did not match expected"
fi

# ─────────────────────────────────────────────────────────────────────────────
# All done
# ─────────────────────────────────────────────────────────────────────────────
print_status "All tests passed!"
