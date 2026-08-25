{
  config,
  lib,
  pkgs,
  ...
}:
let
  cfg = config.services.ecen602.sbcp;
in
{
  options.services.ecen602.sbcp = {
    enable = lib.mkEnableOption "the ECEN 602 Simple Broadcast Chat Protocol server";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../packages.nix { inherit pkgs; }).mp2;
      defaultText = lib.literalMD "MP2 built from this repository";
      description = "Package providing the `sbcp_server` binary.";
    };

    listenAddress = lib.mkOption {
      type = lib.types.str;
      default = "::";
      description = ''
        Address to bind. Both families work: an IPv4 literal like
        `192.168.1.10` is transparently mapped to its IPv6 counterpart,
        while IPv6 literals (`::`, `::1`, ...) are used as-is.
      '';
    };

    port = lib.mkOption {
      type = lib.types.port;
      default = 9000;
      description = "TCP port to listen on.";
    };

    maxClients = lib.mkOption {
      type = lib.types.ints.positive;
      default = 10;
      description = "Maximum number of simultaneously connected clients.";
    };

    openFirewall = lib.mkOption {
      type = lib.types.bool;
      default = false;
      description = "Automatically open {option}`services.ecen602.sbcp.port` in the firewall.";
    };
  };

  config = lib.mkIf cfg.enable {
    networking.firewall.allowedTCPPorts = lib.mkIf cfg.openFirewall [ cfg.port ];

    systemd.services.ecen602-sbcp = {
      description = "ECEN 602 Simple Broadcast Chat Server";
      wantedBy = [ "multi-user.target" ];
      after = [ "network.target" ];
      serviceConfig = {
        ExecStart = "${cfg.package}/bin/sbcp_server ${cfg.listenAddress} ${toString cfg.port} ${toString cfg.maxClients}";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
