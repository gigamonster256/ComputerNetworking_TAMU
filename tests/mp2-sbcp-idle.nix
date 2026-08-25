{ mps, ... }:
{
  name = "mp2-sbcp-idle";

  containers = {
    server = import ./sbcp-server-container.nix { inherit mps; };

    client = {
      virtualisation.vlans = [ 1 ];
      environment.systemPackages = [ mps.mp2 ];
    };
  };

  testScript = ''
    start_all()

    client.wait_for_open_port(9002, "server")

    # The client sends IDLE after 10 seconds without activity; hold its
    # stdin open for 14s so it lives long enough, then EOF ends it.
    client.succeed(
      "(sleep 14) | timeout 20 sbcp_client alice server 9002"
      " > /tmp/idle.log 2>&1"
    )
    assert "Connected to server" in client.succeed("cat /tmp/idle.log")

    # The server journal shows the IDLE message arriving
    server.wait_until_succeeds(
      "journalctl -u ecen602-sbcp | grep -q 'IDLE'"
    )
  '';
}
