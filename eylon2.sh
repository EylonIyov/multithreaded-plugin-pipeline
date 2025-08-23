#!/usr/bin/env bash
set -e

# ─────────────────────────────────────────────────────────────────────────────
#  Colors & helpers
# ─────────────────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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
# 4. Multi-stage pipeline: uppercaser → rotator → logger → flipper
# ─────────────────────────────────────────────────────────────────────────────
# FIXED VERSION: flipper doesn't print, it just transforms and passes to next
# Since flipper is last, it just processes and frees the string
print_status "Test: uppercaser → rotator → logger → flipper"
INPUT="abcd"
# after uppercaser: "ABCD" → after rotator: "DABC" → logger prints "[logger] DABC" → flipper processes "DABC" but doesn't print
EXPECTED_LOGGER="[logger] DABC"
OUT=$(printf "%s\n<END>\n" "$INPUT" | ./output/analyzer 8 uppercaser rotator logger flipper)
LOG_LINE=$(echo "$OUT" | grep "^\[logger\]" | head -1)
SHUTDOWN_LINE=$(echo "$OUT" | grep "Pipeline shutdown complete")

if [[ "$LOG_LINE" == "$EXPECTED_LOGGER" && -n "$SHUTDOWN_LINE" ]]; then
    print_status "PASS"
else
    print_error "FAIL (multi-stage): expected '$EXPECTED_LOGGER' and shutdown message, got logger:'$LOG_LINE' shutdown:'$SHUTDOWN_LINE'"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 5. Expander plugin: inserts a space between every character :contentReference[oaicite:6]{index=6}
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: expander → logger"
INPUT="XYZ"
EXPECTED="[logger] X Y Z"
ACTUAL=$(printf "%s\n<END>\n" "$INPUT" | ./output/analyzer 5 expander logger | grep "^\[logger\]")
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
    print_status "PASS"
else
    print_error "FAIL (expander): expected '$EXPECTED', got '$ACTUAL'"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 6. Typewriter plugin stub: just check prefix exists (delay may vary)
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: typewriter → logger"
INPUT="hi"
# typewriter prints each char prefixed; we only assert it appears
OUT=$(printf "%s\n<END>\n" "$INPUT" | ./output/analyzer 5 typewriter)
if grep -q "^\[typewriter\]" <<< "$OUT"; then
    print_status "PASS"
else
    print_error "FAIL (typewriter): no [typewriter] line in output"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 7. Shutdown message & exit code
#    Program should print "Pipeline shutdown complete" on stdout and exit 0 :contentReference[oaicite:7]{index=7}
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: shutdown message & exit code"
OUT=$(printf "<END>\n" | ./output/analyzer 3 logger 2>&1)
CODE=$?
if [[ $CODE -ne 0 ]]; then
    print_error "FAIL (shutdown code): expected 0, got $CODE"
elif ! grep -q "Pipeline shutdown complete" <<< "$OUT"; then
    print_error "FAIL (shutdown msg): 'Pipeline shutdown complete' not found"
else
    print_status "PASS"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 8. Invalid usage: no arguments, bad queue, unknown plugin
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: missing arguments"
if ./output/analyzer >/dev/null 2>&1; then
    print_error "FAIL (missing args): expected non-zero exit"
else
    print_status "PASS"
fi

print_status "Test: non-numeric queue size"
if ./output/analyzer abc uppercaser >/dev/null 2>&1; then
    print_error "FAIL (bad queue): expected non-zero exit"
else
    print_status "PASS"
fi

print_status "Test: invalid plugin name"
if ./output/analyzer 5 no_such_plugin >/dev/null 2>&1; then
    print_error "FAIL (unknown plugin): expected non-zero exit"
else
    print_status "PASS"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 9. Edge case: empty string input
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: empty string → logger"
EXPECTED_EMPTY="[logger] "
ACTUAL_EMPTY=$(printf "\n<END>\n" | ./output/analyzer 3 logger | grep "^\[logger\]")
if [[ "$ACTUAL_EMPTY" == "$EXPECTED_EMPTY" ]]; then
    print_status "PASS"
else
    print_error "FAIL (empty string): expected '$EXPECTED_EMPTY', got '$ACTUAL_EMPTY'"
fi

# ─────────────────────────────────────────────────────────────────────────────
# 10. Long string (200 chars) – ensure no truncation
# ─────────────────────────────────────────────────────────────────────────────
print_status "Test: long string → logger"
LONG=$(head -c 200 < /dev/zero | tr '\0' 'x')
EXPECTED_LONG="[logger] $LONG"
ACTUAL_LONG=$(printf "%s\n<END>\n" "$LONG" | ./output/analyzer 50 logger | grep "^\[logger\]")
if [[ "$ACTUAL_LONG" == "$EXPECTED_LONG" ]]; then
    print_status "PASS"
else
    print_error "FAIL (long string): output mismatch"
fi

print_status "All tests passed!"
