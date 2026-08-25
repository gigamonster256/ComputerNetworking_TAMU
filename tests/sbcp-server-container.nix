{ mps, ... }:
{
  imports = [ ../modules/nixos ];
  virtualisation.vlans = [ 1 ];
  services.ecen602.sbcp = {
    enable = true;
    package = mps.mp2;
    listenAddress = "::";
    port = 9002;
    maxClients = 10;
    openFirewall = true;
  };
}
