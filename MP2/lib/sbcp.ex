defmodule SbcpMessage do
  defmodule Header do
    @sbcp_version 3
    @type_map %{
      join: 2,
      send: 4,
      fwd: 6,
      ack: 7,
      nak: 5,
      online: 8,
      offline: 6,
      idle: 9
    }
    @reverse_type_map Map.new(@type_map, fn {k, v} -> {v, k} end)

    defstruct [:type, length: 0, version: @sbcp_version]

    def deserialize(<<@sbcp_version::9, type::7, length::16>>) do
      type = Map.get(@reverse_type_map, type)
      %Header{type: type, length: length}
    end

    def type_map, do: @type_map
  end

  defstruct [:header, attributes: []]

  def serialize(%SbcpMessage{header: header, attributes: attrs}) do
    payload =
      attrs
      |> Enum.map(&SbcpAttribute.serialize/1)
      |> Enum.join()

    version = <<header.version::9>>
    type = <<Map.get(Header.type_map, header.type)::7>>
    length = header.length
    # sanity check
    ^length = byte_size(payload)
    length = <<length::16>>
    <<version::bitstring, type::bitstring>> <> length <> payload
  end

  def deserialize(%Header{} = header, payload) when is_binary(payload) do
    attrs = SbcpAttribute.deserialize(payload)
    %SbcpMessage{header: header, attributes: attrs}
  end

  def deserialize(header, payload) when is_binary(header) and is_binary(payload) do
    header = Header.deserialize(header)
    deserialize(header, payload)
  end

  def deserialize(packet) when is_binary(packet) do
    <<header::binary-size(4), payload::binary>> = packet
    deserialize(header, payload)
  end

  def join_message(username) do
    user_attr =
      username
      |> SbcpAttribute.username()

    add_attribute(%SbcpMessage{header: %Header{type: :join}}, user_attr)
  end

  def message_message(message) do
    message_attr =
      message
      |> SbcpAttribute.message()

    add_attribute(%SbcpMessage{header: %Header{type: :send}}, message_attr)
  end

  def add_attribute(%SbcpMessage{header: header, attributes: attrs} = message, attr) do
    attr_length = attr |> SbcpAttribute.serialize() |> byte_size()
    length = header.length + attr_length
    %SbcpMessage{message | header: %{header | length: length}, attributes: [attr | attrs]}
  end

  def client_count(%SbcpMessage{attributes: attrs}) do
    attrs
    |> Enum.find(&(&1.type == :client_count))
    |> Map.get(:payload)
  end

  def usernames(%SbcpMessage{attributes: attrs}) do
    attrs
    |> Enum.filter(&(&1.type == :username))
    |> Enum.map(& &1.payload)
  end

  def message(%SbcpMessage{attributes: attrs}) do
    attrs
    |> Enum.find(&(&1.type == :message))
    |> Map.get(:payload)
  end

  def reason(%SbcpMessage{attributes: attrs}) do
    attrs
    |> Enum.find(&(&1.type == :reason))
    |> Map.get(:payload)
  end
end

defmodule SbcpAttribute do
  @type_map %{
    username: 2,
    message: 4,
    reason: 1,
    client_count: 3
  }

  @reverse_type_map Map.new(@type_map, fn {k, v} -> {v, k} end)

  defstruct [:type, :payload]

  def serialize(%SbcpAttribute{type: type, payload: payload}) do
    type = <<Map.get(@type_map, type)::little-integer-size(16)>>
    length = <<byte_size(payload)::16>>
    type <> length <> <<payload::binary>>
  end

  def deserialize(<<type::little-integer-size(16), length::16, rest::binary>>) do
    type = Map.get(@reverse_type_map, type)
    {payload, rest} = to_payload(type, length, rest)

    [
      %SbcpAttribute{type: type, payload: payload}
      | deserialize(rest)
    ]
  end

  def deserialize(<<>>), do: []

  # string type payload
  defp to_payload(type, length, rest) when type in [:username, :message, :reason] do
    <<payload::binary-size(length), rest::binary>> = rest
    {payload, rest}
  end

  # uint16 type payload
  defp to_payload(type, length, rest) when type in [:client_count] do
    <<payload::length*8, rest::binary>> = rest
    {payload, rest}
  end

  def username(username) when is_binary(username) do
    %SbcpAttribute{type: :username, payload: username}
  end

  def message(message) when is_binary(message) do
    %SbcpAttribute{type: :message, payload: message}
  end
end
