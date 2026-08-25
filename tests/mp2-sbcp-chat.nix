{ mps, ... }:
{
  name = "mp2-sbcp-chat";

  containers = {
    server = import ./sbcp-server-container.nix { inherit mps; };

    alice = {
      virtualisation.vlans = [ 1 ];
      environment.systemPackages = [ mps.mp2 ];
    };

    bob = {
      virtualisation.vlans = [ 1 ];
      environment.systemPackages = [ mps.mp2 ];
    };
  };

  testScript = ''
    from datetime import timedelta

    start_all()

    bob.wait_for_open_port(9002, "server")

    # Alice stays connected for the duration of the chat; tail holds her
    # stdin open and dies with the container.
    alice.execute(
      "tail -f /dev/null | sbcp_client alice server 9002"
      " > /tmp/alice.log 2>&1 &"
    )
    alice.wait_until_succeeds(
      "grep -q Connected /tmp/alice.log", timeout=timedelta(seconds=30)
    )

    out = bob.succeed(
      "printf 'hi from bob\\n\\n' | timeout 15 sbcp_client bob server 9002"
    )
    assert "Connected to server" in out, f"bob failed to join: {out}"
    assert "Client count: 2" in out, f"bob did not see both clients: {out}"
    assert "alice" in out, f"bob did not see alice in the roster: {out}"

    # The client prefixes received messages with its "> " prompt, so no
    # line anchors here.
    alice.wait_until_succeeds(
      "grep -q 'bob: hi from bob' /tmp/alice.log",
      timeout=timedelta(seconds=30),
    )
  '';
}
