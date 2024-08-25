#!/usr/bin/env bash

port=7

# build the project
make -C ../src/ &> /dev/null

# make sure server is not running
killall echos &> /dev/null

# start the server
../src/echos $port &> /dev/null &
server_PID=$!

# start the clients
mkfifo client1.in
mkfifo client1.out
mkfifo client2.in
mkfifo client2.out

# wait for the server to start
sleep 1

../src/echo 127.0.0.1 $port < client1.in > client1.out &
../src/echo 127.0.0.1 $port < client2.in > client2.out &

# send messages
msg1="Hello, World!"
msg2="Goodbye, World!"
echo "$msg1" > client1.in
echo "$msg2" > client2.in

# read messages
timeout=2
read -t $timeout msg1_rcvd < client1.out
read -t $timeout msg2_rcvd < client2.out

# echo "msg1_rcvd: $msg1_rcvd"
# echo "msg2_rcvd: $msg2_rcvd"

# check the messages
if [[ "$msg1" == "$msg1_rcvd" && "$msg2" == "$msg2_rcvd" ]]; then
    echo "Concurrency test passed!"
    passed=0
else
    echo "Concurrency test failed!"
    echo "Expected: '$msg1', '$msg2'"
    echo "Received: '$msg1_rcvd', '$msg2_rcvd'"
    passed=1
fi

# remove the fifos
rm client1.in
rm client1.out
rm client2.in
rm client2.out

# close the server
killall echos &> /dev/null

exit $passed

