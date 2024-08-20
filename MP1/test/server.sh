#!/usr/bin/env bash

port=7
passed=0

# build the project
make -C ../src/ &> /dev/null

# make sure server is not running
killall echos &> /dev/null

# start the server correctly
../src/echos $port &> /dev/null &
server_PID=$!

sleep 1

#make sure pid is running
if ! kill -0 $server_PID &> /dev/null; then
    echo "Server failed to start!"
    passed=1
else
    # make sure server is running on the correct port
    if ! lsof -i :$port &> /dev/null; then
        echo "Server not running on port $port!"
        passed=1
    fi
fi

killall echos &> /dev/null

# start the server incorrectly
../src/echos $port extra_argument &> /dev/null &
server_PID=$!

sleep 1

#make sure pid is not running
if kill -0 $server_PID &> /dev/null; then
    echo "Server started with extra argument!"
    passed=1
fi

killall echos &> /dev/null

# start the server incorrectly
../src/echos &> /dev/null &
server_PID=$!

sleep 1

#make sure pid is not running
if kill -0 $server_PID &> /dev/null; then
    echo "Server started without port!"
    passed=1
fi

killall echos &> /dev/null

if [ $passed -eq 0 ]; then
    echo "Server tests passed!"
else
    echo "Server tests failed!"
fi

exit $passed