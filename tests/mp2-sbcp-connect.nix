{ mps, ... }:
{
  name = "mp2-sbcp-connect";

  containers = {
    server = import ./sbcp-server-container.nix { inherit mps; };

    client = {
      virtualisation.vlans = [ 1 ];
      environment.systemPackages = [ mps.mp2 ];
    };
  };

  testScript = ''
    from datetime import timedelta

    start_all()

    client.wait_for_open_port(9002, "server")

    # Hold the first session open so the username stays registered while
    # the duplicate join is attempted.
    client.execute(
      "tail -f /dev/null | sbcp_client alice server 9002"
      " > /tmp/connect.log 2>&1 &"
    )
    client.wait_until_succeeds(
      "grep -q Connected /tmp/connect.log", timeout=timedelta(seconds=30)
    )
    out = client.succeed("cat /tmp/connect.log")
    assert "No other clients connected" in out, f"unexpected roster: {out}"

    # A second join with the same username is rejected with a NAK
    client.fail("timeout 15 sbcp_client alice server 9002 </dev/null")
  '';
}
