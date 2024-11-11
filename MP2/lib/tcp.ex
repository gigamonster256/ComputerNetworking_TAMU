defmodule TCPClient do
  def connect(host, port) do
    # Convert host to charlist if it's a string
    host = if is_binary(host), do: String.to_charlist(host), else: host

    # Connect with TCP options
    case :gen_tcp.connect(host, port, [
           :binary,
           packet: :raw,
           active: false,
           reuseaddr: true
         ]) do
      {:ok, socket} -> {:ok, socket}
      {:error, reason} -> {:error, reason}
    end
  end

  def send_bytes(socket, data) when is_binary(data) do
    case :gen_tcp.send(socket, data) do
      :ok -> :ok
      {:error, reason} -> {:error, reason}
    end
  end

  def receive_data(socket, bytes \\ 0) do
    case :gen_tcp.recv(socket, bytes) do
      {:ok, data} -> {:ok, data}
      {:error, reason} -> {:error, reason}
    end
  end

  def close(socket) do
    :gen_tcp.close(socket)
  end
end

defmodule TCPServer do
  require Logger

  def start(port) do
    # Listen for incoming connections
    case :gen_tcp.listen(port, [
           :binary,
           packet: :raw,
           active: false,
           reuseaddr: true
         ]) do
      {:ok, listen_socket} ->
        Logger.info("Started server on port #{port}")
        {:ok, listen_socket}

      {:error, reason} ->
        Logger.error("Failed to start server: #{inspect(reason)}")
        {:error, reason}
    end
  end

  def accept_loop(listen_socket, handler) do
    case :gen_tcp.accept(listen_socket) do
      {:ok, client_socket} ->
        # Spawn a new process to handle this client
        spawn(fn -> handler.(client_socket) end)
        # Continue accepting new connections
        accept_loop(listen_socket, handler)

      {:error, reason} ->
        Logger.error("Failed to accept connection: #{inspect(reason)}")
        accept_loop(listen_socket, handler)
    end
  end
end
