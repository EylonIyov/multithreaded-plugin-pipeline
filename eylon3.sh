#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[TEST]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_status "Starting comprehensive tests..."

# Build the project first
print_status "Building project..."
./build.sh || {
    print_error "Build failed!"
    exit 1
}

# Test counter
TESTS_PASSED=0
TESTS_TOTAL=0

# Function to run a test
run_test() {
    local test_name="$1"
    local input="$2"
    local expected="$3"
    local args="$4"
    
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
    print_status "Running test: $test_name"
    
    local actual
    # Capture both stdout and stderr, then filter out the shutdown message
    actual=$(echo -e "$input" | timeout 10s ./output/analyzer $args 2>&1 | grep -v "Pipeline shutdown complete" | grep "^\[" || true)
    
    if [ "$actual" = "$expected" ]; then
        print_status "✅ $test_name: PASS"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        print_error "❌ $test_name: FAIL"
        print_error "Expected: '$expected'"
        print_error "Actual:   '$actual'"
        
        # Debug: show the full output for debugging
        print_error "Full output for debugging:"
        echo -e "$input" | timeout 5s ./output/analyzer $args 2>&1 || true
        exit 1
    fi
}

# EXISTING TESTS (your current ones)
run_test "Basic uppercaser + logger" "hello\n<END>" "[logger] HELLO" "10 uppercaser logger"
run_test "Full pipeline test" "hello\n<END>" $'[logger] OHELL\n[typewriter] LLEHO' "20 uppercaser rotator logger flipper typewriter"
run_test "Logger only" "test\n<END>" "[logger] test" "5 logger"
run_test "Rotator test" "abc\n<END>" "[logger] cab" "5 rotator logger"
run_test "Flipper test" "hello\n<END>" "[logger] olleh" "5 flipper logger"
run_test "Expander test" "hi\n<END>" "[logger] h i" "5 expander logger"
run_test "Multiple rotators" "abc\n<END>" "[logger] bca" "5 rotator rotator logger"
run_test "Empty string" "\n<END>" "[logger] " "5 logger"
run_test "Long string" "abcdefghijklmnopqrstuvwxyz\n<END>" "[logger] ABCDEFGHIJKLMNOPQRSTUVWXYZ" "10 uppercaser logger"

# ADDITIONAL STRESS TESTS

# Test 10: Multiple lines processing
run_test "Multiple lines" "line1\nline2\nline3\n<END>" $'[logger] LINE1\n[logger] LINE2\n[logger] LINE3' "5 uppercaser logger"

# Test 11: Very small queue size (stress test)
run_test "Small queue stress" "test\n<END>" "[logger] TEST" "1 uppercaser logger"

# Test 12: Large queue size
run_test "Large queue" "test\n<END>" "[logger] TEST" "1000 uppercaser logger"

# Test 13: Complex pipeline with all plugins
run_test "All plugins pipeline" "hello\n<END>" "[logger] L L E H O" "10 uppercaser rotator expander flipper logger"

# Test 14: Special characters
run_test "Special characters" "hello@123!\n<END>" "[logger] HELLO@123!" "5 uppercaser logger"

# Test 15: Mixed case input
run_test "Mixed case" "HeLLo WoRLd\n<END>" "[logger] HELLO WORLD" "5 uppercaser logger"

# Test 16: Numbers and symbols
run_test "Numbers and symbols" "test123!@#\n<END>" "[logger] TEST123!@#" "5 uppercaser logger"

# Test 17: Spaces handling
run_test "Spaces handling" "hello world\n<END>" "[logger] d l r o w   o l l e h" "5 expander flipper logger"

# Test 18: Single character
run_test "Single character" "a\n<END>" "[logger] A" "5 uppercaser logger"

# Test 19: Very long pipeline (same plugin multiple times)
run_test "Long pipeline" "abcd\n<END>" "[logger] abcd" "5 rotator rotator rotator rotator logger"

# Test 19a: Single rotation for comparison
run_test "Single rotation" "abcd\n<END>" "[logger] dabc" "5 rotator logger"

# Test 20: Typewriter without other plugins (check timing)
print_status "Testing typewriter timing..."
TESTS_TOTAL=$((TESTS_TOTAL + 1))
start_time=$(date +%s%N)
echo -e "hi\n<END>" | timeout 5s ./output/analyzer 5 typewriter >/dev/null 2>&1
end_time=$(date +%s%N)
duration=$((($end_time - $start_time) / 1000000)) # Convert to milliseconds

if [ $duration -ge 200 ] && [ $duration -le 1000 ]; then
    print_status "✅ Typewriter timing: PASS (${duration}ms)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    print_error "❌ Typewriter timing: FAIL (${duration}ms, expected ~200ms)"
    exit 1
fi

print_status "Testing error conditions..."

# Test 21: Invalid arguments - no queue size
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if timeout 5s ./output/analyzer 2>/dev/null; then
    print_error "❌ Should fail with no arguments"
    exit 1
else
    print_status "✅ Correctly handles missing arguments"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 22: Invalid queue size
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if echo -e "test\n<END>" | timeout 5s ./output/analyzer 0 logger 2>/dev/null; then
    print_error "❌ Should fail with invalid queue size"
    exit 1
else
    print_status "✅ Correctly handles invalid queue size"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 23: Negative queue size
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if echo -e "test\n<END>" | timeout 5s ./output/analyzer -5 logger 2>/dev/null; then
    print_error "❌ Should fail with negative queue size"
    exit 1
else
    print_status "✅ Correctly handles negative queue size"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 24: Invalid plugin name
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if echo -e "test\n<END>" | timeout 5s ./output/analyzer 5 nonexistent 2>/dev/null; then
    print_error "❌ Should fail with invalid plugin"
    exit 1
else
    print_status "✅ Correctly handles invalid plugin name"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 25: No plugins specified
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if echo -e "test\n<END>" | timeout 5s ./output/analyzer 5 2>/dev/null; then
    print_error "❌ Should fail with no plugins"
    exit 1
else
    print_status "✅ Correctly handles no plugins specified"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 26: Non-numeric queue size
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if echo -e "test\n<END>" | timeout 5s ./output/analyzer abc logger 2>/dev/null; then
    print_error "❌ Should fail with non-numeric queue size"
    exit 1
else
    print_status "✅ Correctly handles non-numeric queue size"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 27: Test long input handling (verify no crash, not exact output)
print_status "Testing long input handling..."
TESTS_TOTAL=$((TESTS_TOTAL + 1))
LONG_INPUT=$(printf 'a%.0s' {1..500})  # 500 characters
if echo -e "${LONG_INPUT}\n<END>" | timeout 10s ./output/analyzer 5 uppercaser logger >/dev/null 2>&1; then
    print_status "✅ Long input handling: PASS"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    print_error "❌ Long input handling: FAIL"
    exit 1
fi

# Test 28: Test resource cleanup (multiple runs)
print_status "Testing resource cleanup with multiple runs..."
for i in {1..5}; do
    echo -e "test$i\n<END>" | timeout 5s ./output/analyzer 5 uppercaser logger >/dev/null 2>&1 || {
        print_error "❌ Resource cleanup test failed on iteration $i"
        exit 1
    }
done
print_status "✅ Resource cleanup test: PASS"
TESTS_PASSED=$((TESTS_PASSED + 1))
TESTS_TOTAL=$((TESTS_TOTAL + 1))

# Summary
print_status "========================================="
print_status "Tests completed: $TESTS_PASSED/$TESTS_TOTAL passed"

if [ $TESTS_PASSED -eq $TESTS_TOTAL ]; then
    print_status "🎉 All tests passed!"
    exit 0
else
    print_error "❌ Some tests failed!"
    exit 1
fi