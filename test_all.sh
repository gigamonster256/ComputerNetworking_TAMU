#!/usr/bin/env bash

all_passed=0

# MP 1
echo "Testing MP1"
mp1_passed=0

echo "Testing MP1: Server"
(cd MP1/test && ./server.sh)
mp1_passed=$((mp1_passed + $?))

echo "Testing MP1: Client"
(cd MP1/test && ./client.sh)
mp1_passed=$((mp1_passed + $?))

echo "Testing MP1: Concurrent"
(cd MP1/test && ./concurrent.sh)
mp1_passed=$((mp1_passed + $?))

if [ $mp1_passed -eq 0 ]; then
    echo "MP1 tests passed!"
else
    echo "MP1 tests failed!"
fi
all_passed=$((all_passed + mp1_passed))




if [ $all_passed -eq 0 ]; then
    echo "All tests passed!"
else
    echo "Some tests failed!"
fi

    