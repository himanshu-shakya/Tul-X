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

run_test() {
    local test_name="$1"
    local input_file="$2"
    local expected_status="$3"
    local extra_flag="$4"

    printf "Running %-45s ... " "$test_name"
    set +e
    if [ -n "$extra_flag" ]; then
        $EXECUTABLE "$extra_flag" "$input_file" > /dev/null 2>&1
    else
        $EXECUTABLE "$input_file" > /dev/null 2>&1
    fi
    local status=$?
    set -e

    if [ "$status" -eq "$expected_status" ]; then
        echo "PASSED [code: $status]"
        PASSED=$((PASSED + 1))
    else
        echo "FAILED (Expected $expected_status, got $status)"
        FAILED=$((FAILED + 1))
    fi
}

# 1. Integration / Example Tests (Scanner test for now)
if [ -f "examples/hello.tul" ]; then
    run_test "examples/hello.tul" "examples/hello.tul" 0 "--scan"
fi

# 2. Scanner Tests (Expected exit code: 0 with --scan)
for test_file in tests/scanner/*.tul; do
    if [ -f "$test_file" ]; then
        run_test "$test_file" "$test_file" 0 "--scan"
    fi
done

# 3. Expression AST Tests (Expected exit code: 0)
for test_file in tests/expressions/*.tul; do
    if [ -f "$test_file" ]; then
        run_test "$test_file" "$test_file" 0
    fi
done

# 4. Error Tests (Expected exit code: 65 EX_DATAERR)
for test_file in tests/errors/*.tul; do
    if [ -f "$test_file" ]; then
        run_test "$test_file" "$test_file" 65
    fi
done

echo "=========================================="
echo " Test Results: $PASSED Passed, $FAILED Failed"
echo "=========================================="

if [ "$FAILED" -ne 0 ]; then
    exit 1
fi
