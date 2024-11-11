defmodule Mix.Tasks.Server do
  use Mix.Task
  require Logger

  def run(_) do
    # start tcp server
    {:ok, server} = TCPServer.start(4040)
    # accept connections
    TCPServer.accept_loop(server, &handle_client/1)
  end

  def handle_client(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, data} ->
        # Process the received data
        Logger.info("Received data: #{inspect(data)}")

        # Echo the data back to client
        :gen_tcp.send(socket, data)

        Logger.info("Echoed data back to client")

        # Handle next message from this client
        handle_client(socket)

      {:error, :closed} ->
        Logger.info("Client disconnected")
        :gen_tcp.close(socket)

      {:error, reason} ->
        Logger.error("Error receiving data: #{inspect(reason)}")
        :gen_tcp.close(socket)
    end
  end
end
