{ mps, ... }:
{
  name = "mp3-tftp";

  defaults = { pkgs, ... }: {
    environment.systemPackages = [ pkgs.curl ];
  };

  containers = {
    tftp-server = {
      imports = [ ../nix/modules/nixos ];
      virtualisation.vlans = [ 1 ];
      services.ecen602.tftp = {
        enable = true;
        package = mps.mp3;
        openFirewall = true;
      };
      systemd.services.ecen602-tftp.preStart = ''
        printf 'hello tftp\n' > /var/lib/ecen602-tftp/hello.txt
      '';
    };

    client = {
      virtualisation.vlans = [ 1 ];
      # TFTP servers answer from an ephemeral port (RFC 1350 transfer ID),
      # so the swapped-port replies are not conntrack-ESTABLISHED and the
      # stateful firewall would drop them. Trust the test LAN instead.
      networking.firewall.trustedInterfaces = [ "eth1" ];
    };
  };

  testScript = ''
    from datetime import timedelta

    start_all()

    tftp_server.wait_for_unit("ecen602-tftp.service")

    got = client.wait_until_succeeds(
      "curl -4 -sf --max-time 5 tftp://tftp-server:8080/hello.txt",
      timeout=timedelta(seconds=60),
    )
    assert got.strip() == "hello tftp", f"RRQ returned wrong content: {got}"

    client.succeed("printf 'upload payload\\n' > /tmp/upload.txt")
    client.succeed(
      "curl -4 -sfT /tmp/upload.txt --max-time 20 tftp://tftp-server:8080/upload.txt"
    )
    stored = tftp_server.succeed("cat /var/lib/ecen602-tftp/upload.txt")
    assert stored == "upload payload\n", f"WRQ stored wrong content: {stored}"

    client.fail("curl -4 -sf --max-time 20 tftp://tftp-server:8080/no-such-file.bin")
  '';
}
