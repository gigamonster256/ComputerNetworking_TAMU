#!/usr/bin/env python3
"""Generate a pcap of SBCP messages for testing wireshark/sbcp.lua.

Covers: valid JOIN/FWD/ACK/ONLINE, a deliberately byte-swapped length
field, bad version, oversized username, truncated attribute, pipelined
messages in one TCP segment, and one message split across TCP segments.

Usage: python3 gen_test_pcap.py > sbcp_test.pcap
       tshark -r sbcp_test.pcap -X lua_script:../sbcp.lua -Y sbcp
"""

import struct
import sys

# --- SBCP helpers -------------------------------------------------------

JOIN, FWD, SEND = 2, 3, 4
NAK, OFFLINE, ACK, ONLINE, IDLE = 5, 6, 7, 8, 9
REASON, USERNAME, CLIENT_COUNT, MESSAGE = 1, 2, 3, 4


def header(version, msg_type, payload_len):
    # Network byte order, matching libsbcp's htons()/ntohs() usage.
    return struct.pack(">BBH", version >> 1,
                       ((version & 0x1) << 7) | (msg_type & 0x7F),
                       payload_len)


def attr_str(attr_type, value):
    value = value.encode()
    return struct.pack(">HH", attr_type, len(value)) + value


def msg_join(username):
    body = attr_str(USERNAME, username)
    return header(3, JOIN, len(body)) + body


def msg_fwd(username, text):
    body = attr_str(USERNAME, username) + attr_str(MESSAGE, text)
    return header(3, FWD, len(body)) + body


def msg_ack(usernames, count):
    count_attr = struct.pack(">HHH", CLIENT_COUNT, 2, count)
    body = b"".join(attr_str(USERNAME, u) for u in usernames) + count_attr
    return header(3, ACK, len(body)) + body


def msg_online(username):
    body = attr_str(USERNAME, username)
    return header(3, ONLINE, len(body)) + body


# --- pcap / Ethernet / IP / TCP framing ---------------------------------

SRC_IP, DST_IP = bytes([10, 0, 0, 1]), bytes([10, 0, 0, 2])
SRC_MAC, DST_MAC = bytes.fromhex("020000000001"), bytes.fromhex("020000000002")
SPORT, DPORT = 55555, 9999


def checksum(data):
    if len(data) % 2:
        data += b"\x00"
    total = sum(struct.unpack("!%dH" % (len(data) // 2), data))
    total = (total >> 16) + (total & 0xFFFF)
    total += total >> 16
    return ~total & 0xFFFF


def ip_packet(payload, proto):
    ip = struct.pack(">BBHHHBBH", 0x45, 0, 20 + len(payload), 0x1234, 0x4000,
                     40, proto, 0) + SRC_IP + DST_IP
    ip = ip[:10] + struct.pack("!H", checksum(ip)) + ip[12:]
    return DST_MAC + SRC_MAC + b"\x08\x00" + ip + payload


def tcp_packet(payload, seq, sport=SPORT, dport=DPORT):
    tcp = struct.pack(">HHIIBBHHH", sport, dport, seq, seq + 1, (5 << 4),
                      0x18, 8192, 0, 0) + payload
    pseudo = SRC_IP + DST_IP + struct.pack("!BBH", 0, 6, len(tcp))
    cksum = checksum(pseudo + tcp)
    tcp = tcp[:16] + struct.pack("!H", cksum) + tcp[18:]
    return ip_packet(tcp, 6)


def udp_packet(payload, sport=8888, dport=8888):
    udp = struct.pack(">HHHH", sport, dport, 8 + len(payload), 0) + payload
    pseudo = SRC_IP + DST_IP + struct.pack("!BBH", 0, 17, len(udp))
    cksum = checksum(pseudo + udp)
    udp = udp[:6] + struct.pack("!H", cksum or 0xFFFF) + udp[8:]
    return ip_packet(udp, 17)


def write_pcap(packets, out):
    out.write(struct.pack("<IHHiIII", 0xA1B2C3D4, 2, 4, 0, 0, 65535, 1))
    ts = 1700000000
    for i, pkt in enumerate(packets):
        out.write(struct.pack("<IIII", ts, i * 1000, len(pkt), len(pkt)))
        out.write(pkt)


def main():
    m_join = msg_join("alice")
    m_fwd = msg_fwd("bob", "hello world")
    m_ack = msg_ack(["alice", "bob"], 2)

    # Length field deliberately stored little-endian (payload 19 -> 13 00).
    swap_body = attr_str(MESSAGE, "byte order test")
    m_swap = header(3, SEND, 0)[:-2] + struct.pack("<H", len(swap_body)) \
             + swap_body

    # Version 4 instead of 3.
    m_badver = header(4, JOIN, 7) + attr_str(USERNAME, "eve")

    # Username value length 20 > libsbcp max of 16.
    m_oversize = header(3, ACK, 24) + struct.pack(">HH", USERNAME, 20) \
                 + b"a" * 20

    # Header declares 12 payload bytes; REASON declares 8 but only 4 follow.
    m_truncated = header(3, OFFLINE, 12) + struct.pack(">HH", REASON, 8) \
                  + b"bye!"

    m_join2 = msg_join("dave")
    m_send2 = msg_fwd("carol", "second message in one segment")

    online = msg_online("carol")

    packets = []

    # Stream A: well-formed traffic, including pipelining and a split message.
    seq_a = 1
    for data in [m_join, m_fwd, m_ack, m_join2 + m_send2]:
        packets.append(tcp_packet(data, seq_a))
        seq_a += len(data)
    packets.append(tcp_packet(online[:8], seq_a))
    packets.append(tcp_packet(online[8:], seq_a + 8))

    # Stream B: malformed traffic. The byte-swapped message goes last: its
    # bogus length field makes the TCP dissector wait for reassembly bytes
    # that never arrive, which would otherwise hide later frames.
    seq_b = 1
    for data in [m_badver, m_oversize, m_truncated, m_swap]:
        packets.append(tcp_packet(data, seq_b, sport=55556))
        seq_b += len(data)

    # UDP datagrams: same malformed cases without TCP reassembly interference,
    # so the byte-swap expert warning is visible. Decode with:
    #   -d udp.port==8888,sbcp
    packets.append(udp_packet(msg_join("frank")))
    packets.append(udp_packet(m_swap))

    write_pcap(packets, sys.stdout.buffer)


if __name__ == "__main__":
    main()
