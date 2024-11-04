# Simple HTTP Proxy

## Custom libraries

### libtcp
TCP server and client implementation.
Slightly changed for MP4 vs its use in MP2 to use threads instead of processes. This way, we could share the In-Memory cache between TCP clients without having to send messages between processes.

### libhttp
- header: Helper methods, data structures and enums to help define and identify HTTP headers. 
- client: HTTP client to request a URL using GET.
- request-line: Request line for http requests.
- status-line: Status line for http responses.
- message: HTTP request and response message parsing and manipulation.
- date: HTTP-date parsing and manipulation.
- uri: basic URI parsing (based on but not fully implementing RFC1945 section 3.2.1)
- error: Error definitions.

## Main programs
- http_proxy: HTTP Proxy implementation  
Starts up a TCP server and listens for incoming http requests to proxy
- http_client: HTTP Client implementation  
Sends an HTTP request to a server and prints the response to the console.

## Helper programs
- http_server: Simple HTTP Server implementing custom responses for serveral test cases  
This server allows us to directly control the headers and data that are sent in response to an HTTP request. This was helpful for directly setting the Expires headers and sending 304 Not Modified responses. The custom server was also used to demonstrate long running http requests by inserting a delay into the server response when querying /test7.

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

To use the client to directly query www.google.com you can use the following command
```bash
./client www.google.com 80 /
```

and to use the proxy to query www.google.com you can use the following command
```bash
./client <proxy IP Address> <proxy port number> http://www.google.com/
```

## Contribution
- Caleb: Architecture and code for the HTTP Proxy and Client libraries and main files.
- Rishabh: Improvements to the code for HTTP Proxy and Client libraries and test cases.
