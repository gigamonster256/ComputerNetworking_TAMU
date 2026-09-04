{ mps, ... }:
{
  name = "mp4-http-proxy";

  containers = {
    web = {
      imports = [ ../nix/modules/nixos ];
      virtualisation.vlans = [ 1 ];
      services.ecen602.http-server = {
        enable = true;
        package = mps.mp4;
        port = 8000;
        openFirewall = true;
      };
    };

    proxy = {
      imports = [ ../nix/modules/nixos ];
      virtualisation.vlans = [ 1 ];
      services.ecen602.http-proxy = {
        enable = true;
        package = mps.mp4;
        listenAddress = "::";
        port = 8888;
        openFirewall = true;
      };
    };

    client = {
      virtualisation.vlans = [ 1 ];
      environment.systemPackages = [ mps.mp4 ];
    };
  };

  testScript = ''
    start_all()

    client.wait_for_open_port(8888, "proxy")

    def http_get(host: str, port: int, path: str) -> str:
        """Real GET over bash /dev/tcp; empty probes crash the servers."""
        cmd = (
          "timeout 10 bash -c "
          f"'exec 3<>/dev/tcp/{host}/{port};"
          f' printf "GET {path} HTTP/1.1\\r\\nHost: {host}\\r\\n'
          'Connection: close\\r\\n\\r\\n" >&3;'
          " head -c 400 <&3'"
        )
        return client.succeed(cmd)

    # Readiness: the origin answers a real request through the proxy.
    first = client.succeed("http_client proxy 8888 http://web:8000/test1")
    assert "200" in first and "Test 1: 0" in first, f"bad first response: {first}"

    second = client.succeed("http_client proxy 8888 http://web:8000/test1")
    assert "Test 1: 0" in second, f"cache hit returned wrong body: {second}"

    web.wait_until_succeeds("journalctl -u ecen602-http-server | grep -q 'Request for'")
    hits = int(web.succeed("journalctl -u ecen602-http-server | grep -c 'Request for'").strip())
    assert hits == 1, f"origin saw {hits} requests; the repeat fetch was not a cache hit"

    missing = client.succeed("http_client proxy 8888 http://web:8000/nope")
    assert "404" in missing, f"expected 404 for unknown route: {missing}"

    # Direct origin sanity check via raw socket (counter now at 1)
    direct = http_get("web", 8000, "/test1")
    assert "200" in direct and "Test 1: 1" in direct, f"direct origin fetch failed: {direct}"
  '';
}
