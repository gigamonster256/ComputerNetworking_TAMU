#!/usr/bin/env bash

port=7777
passed=0

# build the project
make -C ../ &> /dev/null

# start the server correctly
../echos $port &> /dev/null &
server_PID=$!

sleep 1

# make sure pid is running
if ! kill -0 $server_PID &> /dev/null; then
    echo "Server failed to start!"
    passed=1
else
    # make sure the server is listening on the port
    if ! (exec 3<>/dev/tcp/127.0.0.1/$port) 2> /dev/null; then
        echo "Server not running on port $port!"
        passed=1
    fi
fi

kill $server_PID &> /dev/null
wait $server_PID 2> /dev/null

# start the server incorrectly
../echos $port extra_argument &> /dev/null &
server_PID=$!

sleep 1

# make sure pid is not running
if kill -0 $server_PID &> /dev/null; then
    echo "Server started with extra argument!"
    passed=1
    kill $server_PID &> /dev/null
fi

# start the server incorrectly
../echos &> /dev/null &
server_PID=$!

sleep 1

# make sure pid is not running
if kill -0 $server_PID &> /dev/null; then
    echo "Server started without port!"
    passed=1
    kill $server_PID &> /dev/null
fi

# clean up any remaining servers (echos forks)
pkill -x echos &> /dev/null

if [ $passed -eq 0 ]; then
    echo "Server tests passed!"
else
    echo "Server tests failed!"
fi

exit $passed
