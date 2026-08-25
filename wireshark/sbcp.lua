-- SBCP (Simple Broadcast Chat Protocol) dissector for Wireshark
--
-- Wire format (matches libsbcp/include/sbcp/sbcp.hpp):
--
--   Header (4 bytes):
--     byte 0      : version[8:1]  (version >> 1)
--     byte 1 bit 7: version[0]    (version & 0x1)
--     byte 1 [6:0]: message type
--     bytes 2-3   : payload length in bytes (network byte order)
--
--   Attributes (repeated until payload length consumed):
--     bytes 0-1   : attribute type          (network byte order)
--     bytes 2-3   : attribute value length  (network byte order)
--     bytes 4-    : attribute value
--
-- Usage:
--   wireshark -X lua_script:sbcp.lua
--   tshark -X lua_script:sbcp.lua -r capture.pcap -Y sbcp -V
--
-- If your chat server runs on a non-default port, either change the
-- SBCP TCP port preference (Edit > Preferences > Protocols > SBCP)
-- or right-click a packet and use Decode As... > SBCP.
--
-- Byte-order checking: every multi-byte field is decoded big-endian as
-- libsbcp stores it. Expert-info warnings flag packets whose length
-- fields look byte-swapped (little-endian), truncated messages, bad
-- versions, and attribute values violating the limits libsbcp enforces.

local sbcp = Proto("sbcp", "Simple Broadcast Chat Protocol")

local SBCP_VERSION = 3

local MESSAGE_TYPES = {
    [2] = "JOIN",
    [3] = "FWD",
    [4] = "SEND",
    [5] = "NAK",
    [6] = "OFFLINE",
    [7] = "ACK",
    [8] = "ONLINE",
    [9] = "IDLE"
}

local ATTRIBUTE_TYPES = {
    [1] = "REASON",
    [2] = "USERNAME",
    [3] = "CLIENT_COUNT",
    [4] = "MESSAGE"
}

local LIMITS = {
    username_max = 16,
    message_max = 512,
    reason_max = 32
}

local f = sbcp.fields
f.version = ProtoField.uint16("sbcp.version", "Version", base.DEC, nil, nil,
                              "Protocol version (9 bits spanning bytes 0-1)")
f.type = ProtoField.uint8("sbcp.type", "Message Type", base.HEX_DEC,
                          MESSAGE_TYPES, 0x7F)
f.length = ProtoField.uint16("sbcp.length", "Payload Length", base.DEC)

f.attr_type = ProtoField.uint16("sbcp.attr.type", "Attribute Type", base.HEX_DEC,
                                ATTRIBUTE_TYPES)
f.attr_length = ProtoField.uint16("sbcp.attr.length", "Attribute Length", base.DEC)
f.username = ProtoField.string("sbcp.username", "Username")
f.message = ProtoField.string("sbcp.message", "Message")
f.reason = ProtoField.string("sbcp.reason", "Reason")
f.client_count = ProtoField.uint16("sbcp.client_count", "Client Count", base.DEC)
f.payload = ProtoField.bytes("sbcp.attr.payload", "Payload")

local ef_bad_version = ProtoExpert.new("sbcp.expert.bad_version",
    "Invalid version (expected 3)", expert.group.MALFORMED, expert.severity.WARN)
local ef_bad_attr = ProtoExpert.new("sbcp.expert.bad_attr_type",
    "Unknown attribute type", expert.group.MALFORMED, expert.severity.WARN)
local ef_truncated = ProtoExpert.new("sbcp.expert.truncated",
    "Declared length exceeds available bytes", expert.group.MALFORMED,
    expert.severity.ERROR)
local ef_leftover = ProtoExpert.new("sbcp.expert.leftover",
    "Trailing bytes do not form a complete attribute", expert.group.MALFORMED,
    expert.severity.WARN)
local ef_oversize = ProtoExpert.new("sbcp.expert.oversize",
    "Value violates libsbcp limits", expert.group.MALFORMED, expert.severity.WARN)
local ef_swap = ProtoExpert.new("sbcp.expert.byte_swapped",
    "Length field looks byte-swapped (little-endian)", expert.group.MALFORMED,
    expert.severity.WARN)

sbcp.experts = {ef_bad_version, ef_bad_attr, ef_truncated, ef_leftover,
                ef_oversize, ef_swap}

local prefs = sbcp.prefs
prefs.tcp_port = Pref.uint("TCP port", 9999, "TCP port for SBCP (0 disables)")
prefs.udp_port = Pref.uint("UDP port", 0, "UDP port for SBCP (0 disables)")

local function bswap16(x)
    return math.floor(x / 256) + (x % 256) * 256
end

local function looks_byte_swapped(len_be, available)
    local swapped = bswap16(len_be)
    return len_be > available and swapped <= available and swapped > 0
end

local function add_string_attr(attr_tree, field, value, max_len, label)
    attr_tree:add(field, value)
    if #value > max_len then
        attr_tree:add_proto_expert_info(ef_oversize, string.format(
            "%s length %d exceeds libsbcp max %d", label, #value, max_len))
    end
end

local function dissect_sbcp(tvb, pinfo, tree)
    local len = tvb:len()

    local b0 = tvb(0, 1):uint()
    local b1 = tvb(1, 1):uint()
    local version = b0 * 2 + math.floor(b1 / 128)
    if version ~= SBCP_VERSION then
        -- Not SBCP; let another dissector try.
        return false
    end

    local msg_type = b1 % 128
    local payload_len = tvb(2, 2):uint()
    local remaining = len - 4

    pinfo.cols.protocol = "SBCP"

    local msg_name = MESSAGE_TYPES[msg_type]
                    or string.format("UNKNOWN(%d)", msg_type)

    local header_tree = tree:add(sbcp, tvb(), "SBCP " .. msg_name)

    header_tree:add(f.version, tvb(0, 2), version)
    header_tree:add(f.type, tvb(1, 1))
    local length_item = header_tree:add(f.length, tvb(2, 2))

    if looks_byte_swapped(payload_len, remaining) then
        length_item:add_proto_expert_info(ef_swap, string.format(
            "Payload length %d > %d bytes present; byte-swapped value would be %d",
            payload_len, remaining, bswap16(payload_len)))
        pinfo.cols.info:append(" [length byte-swapped?]")
    elseif payload_len > remaining then
        length_item:add_proto_expert_info(ef_truncated, string.format(
            "Declared payload length %d but only %d bytes present",
            payload_len, remaining))
    end

    pinfo.cols.info:set(msg_name)

    local offset = 4
    local payload_end = math.min(4 + payload_len, len)
    local attr_index = 0
    local truncated = false

    while offset + 4 <= payload_end do
        local attr_type_raw = tvb(offset, 2):uint()
        local attr_len = tvb(offset + 2, 2):uint()
        local attr_name = ATTRIBUTE_TYPES[attr_type_raw]
                          or string.format("UNKNOWN(%d)", attr_type_raw)

        local attr_end = offset + 4 + attr_len
        if attr_end > payload_end then
            header_tree:add_proto_expert_info(ef_truncated, string.format(
                "%s attribute at payload offset %d declares %d value bytes but only %d remain",
                attr_name, offset - 4, attr_len, payload_end - offset - 4))
            truncated = true
            break
        end

        attr_index = attr_index + 1
        local attr_range = tvb(offset, 4 + attr_len)
        local attr_tree = header_tree:add(sbcp, attr_range, string.format(
            "Attribute %d: %s", attr_index, attr_name))

        local type_item = attr_tree:add(f.attr_type, tvb(offset, 2))
        if ATTRIBUTE_TYPES[attr_type_raw] == nil then
            type_item:add_proto_expert_info(ef_bad_attr)
        end
        local attr_len_item = attr_tree:add(f.attr_length, tvb(offset + 2, 2))
        if looks_byte_swapped(attr_len, payload_end - offset - 4) then
            attr_len_item:add_proto_expert_info(ef_swap, string.format(
                "Attribute length %d suspicious; byte-swapped value would be %d",
                attr_len, bswap16(attr_len)))
        end

        local value_tvb = tvb(offset + 4, attr_len)

        if attr_type_raw == 2 then
            add_string_attr(attr_tree, f.username, value_tvb:string(),
                            LIMITS.username_max, "username")
            pinfo.cols.info:append(" user=" .. value_tvb:string())
        elseif attr_type_raw == 4 then
            add_string_attr(attr_tree, f.message, value_tvb:string(),
                            LIMITS.message_max, "message")
            pinfo.cols.info:append(": " .. value_tvb:string())
        elseif attr_type_raw == 1 then
            add_string_attr(attr_tree, f.reason, value_tvb:string(),
                            LIMITS.reason_max, "reason")
            pinfo.cols.info:append(" reason=" .. value_tvb:string())
        elseif attr_type_raw == 3 then
            if attr_len ~= 2 then
                attr_len_item:add_proto_expert_info(ef_oversize,
                    "CLIENT_COUNT attribute must be exactly 2 bytes")
            else
                attr_tree:add(f.client_count, value_tvb)
                pinfo.cols.info:append(" count=" .. tostring(value_tvb:uint()))
            end
        else
            attr_tree:add(f.payload, value_tvb)
        end

        offset = attr_end
    end

    if not truncated and offset ~= payload_end and payload_end <= len then
        header_tree:add_proto_expert_info(ef_leftover, string.format(
            "%d undeclared trailing byte(s) after attributes", payload_end - offset))
    end

    return true
end

-- libsbcp caps payload at SBCP_MAX_PAYLOAD_LENGTH (1024); anything larger
-- is impossible on the wire and almost certainly a byte-order bug.
local MAX_MESSAGE_LEN = 4 + 1024

function sbcp.dissector(tvb, pinfo, tree)
    -- TCP may deliver partial messages or pack several per segment.
    local offset = 0
    local len = tvb:len()
    while offset < len do
        local available = len - offset

        if available < 4 then
            pinfo.desegment_offset = offset
            pinfo.desegment_len = DESEGMENT_ONE_MORE_SEGMENT
            return
        end

        local msg_len = 4 + tvb(offset + 2, 2):uint()

        if available < msg_len then
            if msg_len <= MAX_MESSAGE_LEN then
                -- Plausibly split across segments: wait for the rest.
                pinfo.desegment_offset = offset
                pinfo.desegment_len = msg_len - available
            else
                -- Impossible declared length (byte-swapped?):
                -- dissect what we have so the warning shows up.
                dissect_sbcp(tvb(offset):tvb(), pinfo, tree)
            end
            return
        end

        if not dissect_sbcp(tvb(offset, msg_len):tvb(), pinfo, tree) then
            -- Stay quiet if we already decoded earlier messages in this
            -- packet: the failure is trailing garbage after an incomplete
            -- stream, not a genuine SBCP packet being misidentified.
            if pinfo.cols.protocol ~= "SBCP" then
                tree:add_proto_expert_info(ef_bad_version)
            end
            return
        end

        offset = offset + msg_len
    end
end

DissectorTable.get("tcp.port"):set(prefs.tcp_port, sbcp)
DissectorTable.get("udp.port"):set(prefs.udp_port, sbcp)

-- Conservative heuristic so SBCP on dynamic ports is auto-detected
-- (Enable/Disable it under View > Enabled Protocols > "sbcp (heuristic)").
local function heur_check(tvb, pinfo)
    if tvb:len() < 8 then
        return false
    end

    local b0 = tvb(0, 1):uint()
    local b1 = tvb(1, 1):uint()
    if b0 * 2 + math.floor(b1 / 128) ~= SBCP_VERSION then
        return false
    end
    local declared = tvb(2, 2):uint()
    if declared > 1024 or 4 + declared > tvb:len() then
        return false
    end
    -- First attribute must have a known type and fit in the payload.
    local attr_type = tvb(4, 2):uint()
    if ATTRIBUTE_TYPES[attr_type] == nil then
        return false
    end
    local attr_len = tvb(6, 2):uint()
    if attr_len > declared - 4 then
        return false
    end
    return true
end

sbcp:register_heuristic("tcp", heur_check)
sbcp:register_heuristic("udp", heur_check)
