{ mps, ... }:
{
  name = "mp1-echo";

  containers = {
    echos = {
      imports = [ ../nix/modules/nixos ];
      virtualisation.vlans = [ 1 ];
      services.ecen602.echos = {
        enable = true;
        package = mps.mp1;
        port = 9001;
        openFirewall = true;
      };
    };

    client = {
      virtualisation.vlans = [ 1 ];
      environment.systemPackages = [ mps.mp1 ];
    };
  };

  testScript = ''
    start_all()

    client.wait_for_open_port(9001, "echos")

    # env bypasses the shell builtin so the installed echo client resolves
    out = client.succeed("printf 'hello nspawn\\n' | env echo echos 9001")
    assert "hello nspawn" in out, f"unexpected echo reply: {out}"
  '';
}
