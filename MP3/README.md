# Trivial File Transfer Protocol (TFTP) Server

## Custom libraries

### libudp
UDP server and client implementation similar to TCP implementation in MP2.

### libtftp
- tftp: Holds all base definitions for the TFTP protocol datastructures, helpful functions, and accessor datastructures... useful for making raw TFTP messages
- packets: Helpers for creating TFTP packets that conform to the TFTP server/client interaction

## Main programs
- tftp_server: TFTP server implementation

## Usage
To run the project, use the following commands:
1. Compile all the files using the makefile to get the server binary **server**.
```bash
make
```
2. Open a terminal and run the TFTP server on default port number 8080.
```bash
./server
```
3. Open a new terminal and run the TFTP client on the same port number. Provide an IPv4 or IPv6 address as well.
```bash
tftp <IP Address> <port number>
```
4. Send or receive files by using 'get' or 'put' commands from TFTP client.
4. Repeat step 3 and 4 in order to create new clients, connect to the server, and transfer files.

## Contribution
- Caleb: Architecture and code for the TFTP server libraries and main files.
- Rishabh: Improvements to the code for TFTP server libraries and test cases.
