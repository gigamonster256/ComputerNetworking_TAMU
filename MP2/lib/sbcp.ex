defmodule SbcpMessage do
  defmodule Header do
    @sbcp_version 3
    defstruct [:type, :length, version: @sbcp_version]

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

    def serialize(%Header{type: type, length: length, version: version}) do
      <<version::size(9), @type_map[type]::size(7), length::size(16)>>
    end

    def deserialize(<<@sbcp_version::size(9), type::size(7), length::size(16)>>) do
      %Header{type: @reverse_type_map[type], length: length, version: @sbcp_version}
    end
  end

  defstruct [:header, :payload]

  def serialize(%SbcpMessage{header: header, payload: payload}) do
    header = Header.serialize(header)
    <<header::binary, payload::binary>>
  end

  def deserialize(<<payload::binary>>, header) do
    %SbcpMessage{header: header, payload: payload}
  end

  def join_message(username) do
    username = SbcpAttribute.username(username) |> SbcpAttribute.serialize()
    username_attribute_length = byte_size(username)

    %SbcpMessage{
      header: %Header{type: :join, length: username_attribute_length},
      payload: username
    }
  end

  def message_message(message) do
    message = SbcpAttribute.message(message) |> SbcpAttribute.serialize()
    message_attribute_length = byte_size(message)

    %SbcpMessage{
      header: %Header{type: :send, length: message_attribute_length},
      payload: message
    }
  end

  def client_count(%SbcpMessage{payload: payload}) do
    payload
    |> SbcpAttribute.deserialize()
    |> Enum.find(&(&1.type == :client_count))
    |> Map.get(:payload)
  end

  def usernames(%SbcpMessage{payload: payload}) do
    payload
    |> SbcpAttribute.deserialize()
    |> Enum.filter(&(&1.type == :username))
    |> Enum.map(& &1.payload)
  end

  def message(%SbcpMessage{payload: payload}) do
    payload
    |> SbcpAttribute.deserialize()
    |> Enum.find(&(&1.type == :message))
    |> Map.get(:payload)
  end

  def reason(%SbcpMessage{payload: payload}) do
    payload
    |> SbcpAttribute.deserialize()
    |> Enum.find(&(&1.type == :reason))
    |> Map.get(:payload)
    |> to_string()
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

  defstruct [:type, :length, :payload]

  def serialize(%SbcpAttribute{type: type, length: length, payload: payload}) do
    <<@type_map[type]::size(8), <<0::size(8)>>, length::size(16), payload::binary>>
  end

  def deserialize(<<type::size(8), _, length::size(16), rest::binary>>) do
    <<payload::binary-size(length), rest::binary>> = rest

    [
      %SbcpAttribute{type: @reverse_type_map[type], length: length, payload: payload}
      | deserialize(rest)
    ]
  end

  def deserialize(<<>>) do
    []
  end

  def username(username) do
    %SbcpAttribute{type: :username, length: byte_size(username), payload: username}
  end

  def message(message) do
    %SbcpAttribute{type: :message, length: byte_size(message), payload: message}
  end
end
