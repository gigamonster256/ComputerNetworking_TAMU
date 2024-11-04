# Simple HTTP Proxy

## Custom libraries

### libtcp
TCP server and client implementation.

### libhttp
- header: Helper methods, data structures and enums to help define and identify HTTP headers. 
- client: HTTP client to request a URL using GET.
- request-line: Helper methods and variables.
- status-line: Status code definitions according to HTTP/1.0.
- message: 
- date: 
- uri: 
- error: Error definitions.

## Main programs
- http_proxy: HTTP Proxy implementation
- http_client: HTTP Client implementation

## Usage
To run the project, use the following commands:
1. Compile all the files using the makefile to get the server binaries **proxy**, **client**.
```bash
make
```
2. Open a terminal and run the HTTP Proxy on the desired IP address and port number.
```bash
./proxy <IP address> <port number>
```
3. Open a new terminal and run the HTTP Client on the same IP address, port number, and provide the URL to retrieve.
```bash
./client <proxy IP Address> <proxy port number> <URL to retrieve>
```

## Contribution
- Caleb: Architecture and code for the HTTP Proxy and Client libraries and main files.
- Rishabh: Improvements to the code for HTTP Proxy and Client libraries and test cases.
