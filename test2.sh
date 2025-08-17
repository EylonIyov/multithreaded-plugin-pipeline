#!/bin/bash

cd output

echo "=== Testing Memory Management ==="

# Test 1: Normal termination
echo "Test 1: Normal pipeline termination"
echo -e "test1\ntest2\ntest3\ntest4\ntest5\n<END>" | ./analyzer 1 logger rotator typewriter

cd ..