
#!/bin/bash

set -u

echo "=== Companion Functional Tests ==="

passed=0
failed=0

run_test() {
    name="$1"
    expected="$2"
    shift 2

    "$@" > /dev/null 2>&1
    actual=$?

    if [ "$actual" -eq "$expected" ]; then
        echo "[OK] $name"
        passed=$((passed + 1))
    else
        echo "[FAIL] $name (expected $expected, got $actual)"
        failed=$((failed + 1))
    fi
}

run_test "Help command" 0 ./companion help
run_test "Status command" 0 ./companion status
run_test "Missing argument" 1 ./companion
run_test "Unknown command" 1 ./companion unknown
run_test "Too many arguments" 1 ./companion help extra

echo "Companion test interface is working" > device_test.txt
run_test "Read test file" 0 ./companion check ./device_test.txt
run_test "Missing file" 1 ./companion check ./missing_test_file.txt
if [ ! -e /dev/companion_demo ]; then
    run_test "Device interface unavailable" 1 ./companion check
else
    echo "[SKIP] Device interface unavailable (device exists)"
fi
echo "=== Output Verification Tests ==="
if [ ! -e /dev/rootkit ]; then
    run_test "Rootkit device unavailable" 1 ./companion check /dev/rootkit
else
    echo "[SKIP] Rootkit device unavailable (device exists)"
fi
if ./companion help | grep -Fq "Available commands:"; then
    echo "[OK] Help output"
    passed=$((passed + 1))
else
    echo "[FAIL] Help output"
    failed=$((failed + 1))
fi

if ./companion status | grep -Fq "Companion program is running."; then
    echo "[OK] Status output"
    passed=$((passed + 1))
else
    echo "[FAIL] Status output"
    failed=$((failed + 1))
fi

if ./companion check ./device_test.txt | grep -Fq "Companion test interface is working"; then
    echo "[OK] Device response content"
    passed=$((passed + 1))
else
    echo "[FAIL] Device response content"
    failed=$((failed + 1))
fi
echo "=== Permission Test ==="

permission_file="./permission_test.txt"
printf 'Permission test\n' > "$permission_file"
chmod 000 "$permission_file"

if [ ! -r "$permission_file" ]; then
    run_test "Permission denied" 1 ./companion check "$permission_file"
else
    echo "[SKIP] Permission denied (cannot enforce permissions for this user)"
fi

chmod 644 "$permission_file"
rm -f "$permission_file"
echo
echo "Passed: $passed"
echo "Failed: $failed"

if [ "$failed" -ne 0 ]; then
    exit 1
fi

exit 0
