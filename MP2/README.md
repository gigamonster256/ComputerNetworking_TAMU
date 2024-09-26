# Simple Broadcast Chat Server

## Custom libraries

### libtcp
based on TCP server and client implementation from MP1. Extended with a few extra capabilities (like binding to specific IP address)

### libsbcp
- sbcp: holds all base definitions for the SBCP protocol datastructures, helpful functions, and accessor datastructures... useful for making raw SBCP messages
- messages: Helpers for creating SBCP messages that conform to the SPCP server/client interaction

## Main programs
- sbcp_client: sbcp client implementation
- sbcp_server: sbcp server implementation
