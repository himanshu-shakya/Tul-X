#!/usr/bin/env bash
set -e

echo "=========================================="
echo " Running TUL-X Test Suite"
echo "=========================================="

PASSED=0
FAILED=0
EXECUTABLE="./tulx"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Executable $EXECUTABLE not found. Building first..."
    make
fi

# Function to run a test
run_test() {
    local test_name="$1"
    local input_file="$2"
    local expected_status="$3"

    echo -n "Running $test_name... "
    set +e
    $EXECUTABLE "$input_file" > /dev/null 2>&1
    local status=$?
    set -e

    if [ "$status" -eq "$expected_status" ]; then
        echo "PASSED [exit code: $status]"
        PASSED=$((PASSED + 1))
    else
        echo "FAILED (Expected exit code $expected_status, got $status)"
        FAILED=$((FAILED + 1))
    fi
}

# Run sample tests
if [ -f "examples/hello.tul" ]; then
    run_test "examples/hello.tul" "examples/hello.tul" 0
fi

echo "=========================================="
echo " Test Results: $PASSED Passed, $FAILED Failed"
echo "=========================================="

if [ "$FAILED" -ne 0 ]; then
    exit 1
fi
