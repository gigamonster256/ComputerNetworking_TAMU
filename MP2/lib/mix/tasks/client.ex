defmodule Mix.Tasks.Client do
  use Mix.Task

  def run(args) do
    username = hd(args)
    {:ok, socket} = TCPClient.connect("localhost", 4040)
    message = username |> SbcpMessage.join_message() |> SbcpMessage.serialize()
    TCPClient.send_bytes(socket, message)
    {:ok, header} = TCPClient.receive_data(socket, 4)
    header = SbcpMessage.Header.deserialize(header)
    {:ok, payload} = TCPClient.receive_data(socket, header.length)
    message = payload |> SbcpMessage.deserialize(header)

    case message.header.type do
      :ack ->
        IO.puts("Join successful as #{username}")

      :nak ->
        message = message |> SbcpMessage.reason() |> to_string()
        IO.puts("Join failed: #{message}")
    end

    message = "Hello from elixir" |> SbcpMessage.message_message() |> SbcpMessage.serialize()
    TCPClient.send_bytes(socket, message)
    TCPClient.close(socket)
  end
end
