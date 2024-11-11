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
    message = SbcpMessage.deserialize(header, payload)

    case message.header.type do
      :ack ->
        IO.puts("Join successful as #{username}")
        online_users = message |> SbcpMessage.usernames()
        user_count = message |> SbcpMessage.client_count()

        case user_count do
          1 -> IO.puts("You are the only user online")
          _ -> IO.puts("Online users: #{online_users |> Enum.join(", ")}")
        end

      :nak ->
        reason = message |> SbcpMessage.reason()
        IO.puts("Join failed: #{reason}")
        TCPClient.close(socket)
        exit(:nak)
    end

    message = "Hello from elixir" |> SbcpMessage.message_message() |> SbcpMessage.serialize()
    TCPClient.send_bytes(socket, message)
    TCPClient.close(socket)
  end
end
