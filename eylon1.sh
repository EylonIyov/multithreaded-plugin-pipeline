#!/bin/bash
# filepath: /workspaces/OS Final Task/eylon1_fixed.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() {
    echo -e "${GREEN}[TEST]${NC} $1"
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

# IMPORTANT: Change to output directory where analyzer expects .so files
cd output

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
    # Run analyzer from output directory
    actual=$(echo -e "$input" | timeout 10s ./analyzer $args 2>&1 | grep -v "Pipeline shutdown complete" | grep "^\[" || true)
    
    if [ "$actual" = "$expected" ]; then
        print_status "✅ $test_name: PASS"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        print_error "❌ $test_name: FAIL"
        print_error "Expected: '$expected'"
        print_error "Actual:   '$actual'"
        
        # Debug: show the full output for debugging
        print_error "Full output for debugging:"
        echo -e "$input" | timeout 5s ./analyzer $args 2>&1 || true
        echo "---"
        print_error "Available .so files:"
        ls -la *.so
        echo "---"
    fi
}

# Test 1: Basic uppercaser + logger
run_test "Basic uppercaser + logger" "hello\n<END>" "[logger] HELLO" "10 uppercaser logger"

# Test 2: Single logger plugin
run_test "Logger only" "test\n<END>" "[logger] test" "5 logger"

# Test 3: Rotator test
run_test "Rotator test" "abc\n<END>" "[logger] cab" "5 rotator logger"

# Test 4: Flipper test  
run_test "Flipper test" "hello\n<END>" "[logger] olleh" "5 flipper logger"

# Test 5: Expander test
run_test "Expander test" "hi\n<END>" "[logger] h i" "5 expander logger"

# Test 6: Empty string
run_test "Empty string" "\n<END>" "[logger] " "5 logger"

# Test 7: Long string
run_test "Long string" "abcdefghijklmnopqrstuvwxyz\n<END>" "[logger] ABCDEFGHIJKLMNOPQRSTUVWXYZ" "10 uppercaser logger"

print_status "Testing error conditions..."

# Test 8: Invalid arguments - no queue size
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if timeout 5s ./analyzer 2>/dev/null >/dev/null; then
    print_error "❌ Should fail with no arguments"
else
    print_status "✅ Correctly handles missing arguments"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 9: Invalid queue size
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if echo -e "test\n<END>" | timeout 5s ./analyzer 0 logger 2>/dev/null >/dev/null; then
    print_error "❌ Should fail with invalid queue size"
else
    print_status "✅ Correctly handles invalid queue size"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Test 10: Invalid plugin name
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if echo -e "test\n<END>" | timeout 5s ./analyzer 5 nonexistent 2>/dev/null >/dev/null; then
    print_error "❌ Should fail with invalid plugin"
else
    print_status "✅ Correctly handles invalid plugin name"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Go back to original directory
cd ..

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