# Colors for output 
RED='\033[0;31m' 
GREEN='\033[0;32m' 
YELLOW='\033[1;33m' 
NC='\033[0m' # No Color 
 
# Function to print colored output 
print_status() 
{ 
    echo -e "${GREEN}[BUILD]${NC} $1" 
} 
 
print_warning() 
{ 
    echo -e "${YELLOW}[WARNING]${NC} $1" 
} 
 
print_error() 
{ 
    echo -e "${RED}[ERROR]${NC} $1" 
}

print_test(){
        echo -e "${GREEN}[TEST]${NC} $1" 

}

#Piepline building
./build.sh


print_status "Test #1: Test uppercaser + logger" 

EXPECTED="[logger] HELLO" 
ACTUAL=$(echo -e "hello\n<END>" | ./output/analyzer 10 uppercaser logger | grep "\[logger\]") 
 
if [ "$ACTUAL" == "$EXPECTED" ]; then 
    print_status "Test uppercaser + logger: PASS" 
else 
    print_error "Test uppercaser + logger: FAIL (Expected '$EXPECTED', got 
'$ACTUAL')" 
    exit 1 
fi

print_status "Test #2: Test rotator + logger" 

EXPECTED="[logger] ohell" 
ACTUAL=$(echo -e "hello\n<END>" | ./output/analyzer 10 rotator logger | grep "\[logger\]") 
 
if [ "$ACTUAL" == "$EXPECTED" ]; then 
    print_status "Test rotator + logger: PASS" 
else 
    print_error "Test rotator + logger: FAIL (Expected '$EXPECTED', got 
'$ACTUAL')" 
    exit 1 
fi

print_status "Test #3: Test expander + logger" 

EXPECTED="[logger] h e l l o" 
ACTUAL=$(echo -e "hello\n<END>" | ./output/analyzer 10 expander logger | grep "\[logger\]") 
 
if [ "$ACTUAL" == "$EXPECTED" ]; then 
    print_status "Test expander + logger: PASS" 
else 
    print_error "Test expander + logger: FAIL (Expected '$EXPECTED', got 
'$ACTUAL')" 
    exit 1 
fi

print_status "Test #4: Test flipper + logger" 

EXPECTED="[logger] olleh" 
ACTUAL=$(echo -e "hello\n<END>" | ./output/analyzer 10 flipper logger | grep "\[logger\]") 
 
if [ "$ACTUAL" == "$EXPECTED" ]; then 
    print_status "Test flipper + logger: PASS" 
else 
    print_error "Test flipper + logger: FAIL (Expected '$EXPECTED', got 
'$ACTUAL')" 
    exit 1 
fi

print_status "Test #5: empty string test " 

EXPECTED="[logger] " 
ACTUAL=$(echo -e "\n<END>" | ./output/analyzer 5 expander rotator uppercaser flipper logger | grep "\[logger\]") 
 
if [ "$ACTUAL" == "$EXPECTED" ]; then 
    print_status "empty string test: PASS" 
else 
    print_error "empty string test: FAIL (Expected '$EXPECTED', got 
'$ACTUAL')" 
    exit 1 
fi

print_status "Test #6: space string test " 

EXPECTED="[logger]  " 
ACTUAL=$(echo -e " \n<END>" | ./output/analyzer 5 expander rotator uppercaser flipper logger | grep "\[logger\]") 
 
if [ "$ACTUAL" == "$EXPECTED" ]; then 
    print_status "space string test: PASS" 
else 
    print_error "empty string test: FAIL (Expected '$EXPECTED', got 
'$ACTUAL')" 
    exit 1 
fi


print_status "Test #7: Multiple words, no newline"
INPUT="Hello, World!"
EXPECTED="[logger] HELLO, WORLD!"
ACTUAL=$(printf "%s\n<END>\n" "$INPUT" | ./output/analyzer 10 uppercaser logger | grep "^\[logger\]")
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
    print_status "Multiple words, no newline: Pass"
else
    print_error "Multiple words, no newline: FAIL:  expected '$EXPECTED', got '$ACTUAL'"
fi

print_status "Test #8: Multi Plugin Pipeline Test"
ACTUAL=$(echo -e "test\n<END>" | ./output/analyzer 15 uppercaser rotator flipper logger | grep "\[logger\]")
EXPECTED="[logger] SETT"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Multi Plugin Pipeline Test: PASS"
else
    print_error "Multi Plugin Pipeline Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #9: Multiple Inputs Test"
ACTUAL=$(echo -e "line1\nline2\nline3\n<END>" | ./output/analyzer 10 logger | grep "\[logger\]" | tail -1)
EXPECTED="[logger] line3"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Multiple Inputs Test: PASS"
else
    print_error "Multiple Inputs Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #10: Long Input Test"
LONG_INPUT=$(printf 'a%.0s' {1..200})
ACTUAL=$(echo -e "${LONG_INPUT}\n<END>" | ./output/analyzer 10 uppercaser logger | grep "\[logger\]")
EXPECTED="[logger] $(printf 'A%.0s' {1..200})"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Long Input Test: PASS"
else
    print_error "Long Input Test: FAIL (Expected length ${#EXPECTED}, got length ${#ACTUAL})"
    exit 1
fi

print_status "Test #11: Special Characters Test"
ACTUAL=$(echo -e "hello@#\$%^&*()\n<END>" | ./output/analyzer 10 uppercaser logger | grep "\[logger\]")
EXPECTED="[logger] HELLO@#\$%^&*()"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Special Characters Test: PASS"
else
    print_error "Special Characters Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #12: Large Queue Test"
ACTUAL=$(echo -e "test\n<END>" | ./output/analyzer 100 logger | grep "\[logger\]")
EXPECTED="[logger] test"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Large Queue Test: PASS"
else
    print_error "Large Queue Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #13: Small Queue Stress Test"
ACTUAL=$(echo -e "a\nb\nc\n<END>" | ./output/analyzer 2 uppercaser rotator logger | grep "\[logger\]" | tail -1)
EXPECTED="[logger] C"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Small Queue Stress Test: PASS"
else
    print_error "Small Queue Stress Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #14: All Plugins Test"
ACTUAL=$(echo -e "comprehensive\n<END>" | ./output/analyzer 20 uppercaser rotator flipper expander typewriter logger | grep "\[logger\]")
EXPECTED="[logger] V I S N E H E R P M O C E"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "All Plugins Test: PASS"
else
    print_error "All Plugins Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #15: Non-existent Plugin Test"
OUTPUT=$(echo -e "test\n<END>" | ./output/analyzer 10 nonexistent_plugin logger 2>&1)
EXIT_CODE=$?
if [ $EXIT_CODE -eq 1 ] && echo "$OUTPUT" | grep -q "Error.*Failed to load plugin"; then
    print_status "Non-existent Plugin Test: PASS"
else
    print_error "Non-existent Plugin Test: FAIL (Expected exit code 1 with plugin load error, got exit code $EXIT_CODE)"
    print_error "Output: $OUTPUT"
    exit 1
fi

print_status "Test #17: Multiple spaces"
INPUT="    "
EXPECTED="[logger]        "
ACTUAL=$(printf "%s\n<END>\n" "$INPUT" | ./output/analyzer 10 expander logger | grep "^\[logger\]")
if [[ "$ACTUAL" == "$EXPECTED" ]]; then
    print_status "Multiple spaces: Pass"
else
    print_error "Multiple spaces: FAIL:  expected '$EXPECTED', got '$ACTUAL'"
fi

print_status "Test #18: Queue Size 1 Stress Test"
ACTUAL=$(echo -e "item1\nitem2\nitem3\nitem4\nitem5\n<END>" | ./output/analyzer 1 uppercaser rotator typewriter logger | grep "\[logger\]" | tail -1)
EXPECTED="[logger] 5ITEM"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Queue Size 1 Stress Test: PASS"
else
    print_error "Queue Size 1 Stress Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #19: Maximum Length String (1024 chars) Test"
LONG_STRING=$(printf 'A%.0s' {1..1024})
ACTUAL=$(echo -e "${LONG_STRING}\n<END>" | ./output/analyzer 10 logger | grep "\[logger\]" | head -1)
EXPECTED="[logger] ${LONG_STRING}"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Maximum Length String Test: PASS"
else
    ACTUAL_LENGTH=${#ACTUAL}
    EXPECTED_LENGTH=$((1024 + 9))  
    if [ "$ACTUAL_LENGTH" -ne "$EXPECTED_LENGTH" ]; then
        print_error "Maximum Length String Test: FAIL (Length mismatch - Expected $EXPECTED_LENGTH chars, got $ACTUAL_LENGTH)"
    else
        print_error "Maximum Length String Test: FAIL (Content mismatch)"
    fi
    exit 1
fi

print_status "Test #20: Single Character Test"
ACTUAL=$(echo -e "a\n<END>" | ./output/analyzer 10 uppercaser logger | grep "\[logger\]")
EXPECTED="[logger] A"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Single Character Test: PASS"
else
    print_error "Single Character Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi


print_status "Test #21: Special Characters and Symbols Test"
ACTUAL=$(echo -e "!@#\$%^&*()\n<END>" | ./output/analyzer 10 logger | grep "\[logger\]")
EXPECTED="[logger] !@#\$%^&*()"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Special Characters and Symbols Test: PASS"
else
    print_error "Special Characters and Symbols Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #22: Numbers Preservation Test"
ACTUAL=$(echo -e "abc123def\n<END>" | ./output/analyzer 10 uppercaser logger | grep "\[logger\]")
EXPECTED="[logger] ABC123DEF"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Numbers Preservation Test: PASS"
else
    print_error "Numbers Preservation Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #23: Very Small Queue Size Test"
ACTUAL=$(echo -e "test\n<END>" | ./output/analyzer 1 uppercaser logger | grep "\[logger\]")
EXPECTED="[logger] TEST"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Very Small Queue Size Test: PASS"
else
    print_error "Very Small Queue Size Test: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #24: Graceful Shutdown Message Test"
OUTPUT=$(echo -e "test\n<END>" | ./output/analyzer 10 logger)
if echo "$OUTPUT" | grep -q "Pipeline shutdown complete"; then
    print_status "Graceful Shutdown Message Test: PASS"
else
    print_error "Graceful Shutdown Message Test: FAIL (Missing graceful shutdown message)"
    exit 1
fi

print_status "Test #25: Immediate END Test"
OUTPUT=$(echo -e "<END>" | ./output/analyzer 10 logger)
if echo "$OUTPUT" | grep -q "Pipeline shutdown complete"; then
    print_status "Immediate END Test: PASS"
else
    print_error "Immediate END Test: FAIL (Failed to handle immediate <END>)"
    exit 1
fi

print_status "Test #26: realistic scenario"
REALISTIC_OUTPUT=$(echo -e "Hello, World!\nThis is a test.\nProcessing multiple lines.\n<END>" | ./output/analyzer 15 uppercaser expander logger 2>&1)
LINE_COUNT=$(echo "$REALISTIC_OUTPUT" | grep "\[logger\]" | wc -l)
if [ "$LINE_COUNT" -eq "3" ] && echo "$REALISTIC_OUTPUT" | grep -q "Pipeline shutdown complete"; then
    print_status "End-to-end realistic scenario: PASS"
else
    print_error "End-to-end realistic scenario: FAIL (Expected 3 logger lines and shutdown message)"
    exit 1
fi

print_status "Test #27: Plugin initialization error handling"
ERROR_OUTPUT=$(./output/analyzer invalid_queue_size logger 2>&1 || true)
if echo "$ERROR_OUTPUT" | grep -q "Usage:"; then
    print_status "Plugin initialization error handling: PASS"
else
    print_error "Plugin initialization error handling: FAIL (Expected usage message on error)"
    exit 1
fi

print_status "Test #28: Missing arguments - no plugins"
ERROR_OUTPUT=$(./output/analyzer 10 2>&1 || true)
if echo "$ERROR_OUTPUT" | grep -q "Error:" && echo "$ERROR_OUTPUT" | grep -q "Usage:"; then
    print_status "Missing arguments - no plugins: PASS"
else
    print_error "Missing arguments - no plugins: FAIL (Expected error and usage)"
    exit 1
fi

print_status "Test #29: No arguments at all"
ERROR_OUTPUT=$(./output/analyzer 2>&1 || true)
if echo "$ERROR_OUTPUT" | grep -q "Error:" && echo "$ERROR_OUTPUT" | grep -q "Usage:"; then
    print_status "No arguments at all: PASS"
else
    print_error "No arguments at all: FAIL (Expected error and usage)"
    exit 1
fi

print_status "Test #30: Negative queue size"
ERROR_OUTPUT=$(./output/analyzer -5 logger 2>&1 || true)
if echo "$ERROR_OUTPUT" | grep -q "Error:" && echo "$ERROR_OUTPUT" | grep -q "must be a positive integer"; then
    print_status "Negative queue size: PASS"
else
    print_error "Negative queue size: FAIL (Expected positive integer error)"
    exit 1
fi

print_status "Test #31: Zero queue size"
ERROR_OUTPUT=$(./output/analyzer 0 logger 2>&1 || true)
if echo "$ERROR_OUTPUT" | grep -q "Error:" && echo "$ERROR_OUTPUT" | grep -q "must be a positive integer"; then
    print_status "Zero queue size: PASS"
else
    print_error "Zero queue size: FAIL (Expected positive integer error)"
    exit 1
fi

print_status "Test #32: Tab character preservation"
ACTUAL=$(echo -e "hello\tworld\n<END>" | ./output/analyzer 10 logger | grep "\[logger\]")
EXPECTED="[logger] hello	world"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Tab character preservation: PASS"
else
    print_error "Tab character preservation: FAIL (Expected tab preserved)"
    exit 1
fi

print_status "Test #33: Mixed case preservation in logger"
ACTUAL=$(echo -e "HeLLo WoRLd\n<END>" | ./output/analyzer 10 logger | grep "\[logger\]")
EXPECTED="[logger] HeLLo WoRLd"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Mixed case preservation in logger: PASS"
else
    print_error "Mixed case preservation in logger: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #34: Rotator with single character"
ACTUAL=$(echo -e "a\n<END>" | ./output/analyzer 10 rotator logger | grep "\[logger\]")
EXPECTED="[logger] a"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Rotator with single character: PASS"
else
    print_error "Rotator with single character: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #35: Flipper with single character"
ACTUAL=$(echo -e "x\n<END>" | ./output/analyzer 10 flipper logger | grep "\[logger\]")
EXPECTED="[logger] x"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Flipper with single character: PASS"
else
    print_error "Flipper with single character: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #36: Expander with empty string"
ACTUAL=$(echo -e "\n<END>" | ./output/analyzer 10 expander logger | grep "\[logger\]")
EXPECTED="[logger] "
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Expander with empty string: PASS"
else
    print_error "Expander with empty string: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #37: Numbers and punctuation only"
ACTUAL=$(echo -e "123!@#456\n<END>" | ./output/analyzer 10 uppercaser logger | grep "\[logger\]")
EXPECTED="[logger] 123!@#456"
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "Numbers and punctuation only: PASS"
else
    print_error "Numbers and punctuation only: FAIL (Expected '$EXPECTED', got '$ACTUAL')"
    exit 1
fi

print_status "Test #38: String with only spaces"
ACTUAL=$(echo -e "     \n<END>" | ./output/analyzer 10 logger | grep "\[logger\]")
EXPECTED="[logger]      "
if [ "$ACTUAL" == "$EXPECTED" ]; then
    print_status "String with only spaces: PASS"
else
    print_error "String with only spaces: FAIL (Expected 5 spaces after logger tag)"
    exit 1
fi

print_status "Test #39: Verify stderr vs stdout separation"
STDOUT_ONLY=$(./output/analyzer invalid 2>/dev/null || true)
STDERR_ONLY=$(./output/analyzer invalid 2>&1 1>/dev/null || true)
if echo "$STDOUT_ONLY" | grep -q "Usage:" && echo "$STDERR_ONLY" | grep -q "Error:"; then
    print_status "stderr vs stdout separation: PASS"
else
    print_error "stderr vs stdout separation: FAIL (Error should go to stderr, Usage to stdout)"
    exit 1
fi

print_status "Test #40: Multiple ENDs handling"
OUTPUT=$(echo -e "test\n<END>\n<END>\n" | ./output/analyzer 10 logger 2>&1)
if echo "$OUTPUT" | grep -q "Pipeline shutdown complete" && [ $(echo "$OUTPUT" | grep -c "Pipeline shutdown complete") -eq 1 ]; then
    print_status "Multiple ENDs handling: PASS"
else
    print_error "Multiple ENDs handling: FAIL (Should shutdown on first <END>)"
    exit 1
fi

print_status "Test #41: 1024 char line followed by another line"
LONG_LINE=$(printf 'B%.0s' {1..1024})
OUTPUT=$(echo -e "${LONG_LINE}\nshort\n<END>" | ./output/analyzer 10 logger | grep "\[logger\]")
LINE_COUNT=$(echo "$OUTPUT" | wc -l)
if [ "$LINE_COUNT" -eq "2" ]; then
    print_status "1024 char line followed by another: PASS"
else
    print_error "1024 char line followed by another: FAIL (Expected 2 lines, got $LINE_COUNT)"
    exit 1
fi