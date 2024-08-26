# ComputerNetworking_TAMU
Files for ECEN 602

Most of the programs have simple makefiles to build, and are wrapped in nix derivations for convenience. If you don't have nix installed, you can cd to the Machine Problem `src` directory, run `make` and then run the applications.

## Machine Problems
### MP1: Echo Server/Client
Simple TCP echo server according to [RFC 862](https://www.rfc-editor.org/rfc/rfc862) and a basic client  
Server command: `nix run .#echos <port>`  
Client command: `nix run .#echo <server> <port>`

### MP2: Simple Broadcast Chat Server
Simple Broadcast Chat Server accoring to the [Design doc](./MP2/MP2.pdf) with all extra features implemented  
Server command: `nix run .#sbcp_server <ip> <port> <max_clients>`  
Client command: `nix run .#sbcp_client <username> <server> <port>`

### MP3: Trivial File Transfer Protocol
TBD

### MP4: Simple HTTP Proxy
TBD

### MP5: Network Simulation
TBD
