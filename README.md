# ComputerNetworking_TAMU
Files for ECEN 602

Most of the programs have simple makefiles to build, and are wrapped in nix derivations for convenience. If you don't have nix installed, you can cd to a Machine Problem directory, run `make` and then run the applications.

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
TFTP server per [RFC 1350](https://www.rfc-editor.org/rfc/rfc1350) with netascii support  
Server command: `nix run .#tftp_server` (serves on port 8080)

### MP4: Simple HTTP Proxy
HTTP client, proxy, and server built on `libhttp`  
Client command: `nix run .#http_client <proxy> <port> <url>`  
Proxy command: `nix run .#http_proxy <ip> <port>`  
Server command: `nix run .#http_server <port>`

## NixOS Modules
Each server is packaged as a NixOS module under `services.ecen602.*`, available
via the flake's `nixosModules` output:

```nix
{
  imports = [ inputs.ecen602.nixosModules.default ];

  services.ecen602 = {
    echos.enable = true;
    sbcp.enable = true;
    tftp.enable = true;
    http-server.enable = true;
    http-proxy.enable = true;
  };
}
```

Every module supports `package`, port/listen-address options, and
`openFirewall`. The SBCP server and HTTP proxy accept IPv4 or IPv6
`listenAddress` literals (IPv4 is mapped automatically).

