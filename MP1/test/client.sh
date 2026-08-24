#!/usr/bin/env bash

port=7777
passed=0

# build the project
make -C ../ &> /dev/null

# start the server
../echos $port &> /dev/null &
server_PID=$!

mkfifo client.in
mkfifo client.out

# start the client
../echo 127.0.0.1 $port < client.in > client.out &
client_PID=$!

sleep 1

# ensure client is running
if ! kill -0 $client_PID &> /dev/null; then
    echo "Client failed to start!"
    passed=1
fi

# send message
msg="Hello, World!"
echo "$msg" > client.in

# read message back
timeout=2
read -t $timeout msg_rcvd < client.out

# check the message
if [[ "$msg" == "$msg_rcvd" ]]; then
    echo "Client test passed!"
    passed=0
else
    echo "Client test failed!"
    echo "Expected: '$msg'"
    echo "Received: '$msg_rcvd'"
    passed=1
fi

# remove the fifos
rm client.in
rm client.out

# close the server
kill $server_PID &> /dev/null
pkill -x echos &> /dev/null

exit $passed




