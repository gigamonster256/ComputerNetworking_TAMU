{
  config,
  lib,
  pkgs,
  ...
}:
let
  cfg = config.services.ecen602.http-proxy;
in
{
  options.services.ecen602.http-proxy = {
    enable = lib.mkEnableOption "the ECEN 602 caching HTTP proxy";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../packages.nix { inherit pkgs; }).mp4;
      defaultText = lib.literalMD "MP4 built from this repository";
      description = "Package providing the `http_proxy` binary.";
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
      default = 8888;
      description = "TCP port to listen on.";
    };

    openFirewall = lib.mkOption {
      type = lib.types.bool;
      default = false;
      description = "Automatically open {option}`services.ecen602.http-proxy.port` in the firewall.";
    };
  };

  config = lib.mkIf cfg.enable {
    networking.firewall.allowedTCPPorts = lib.mkIf cfg.openFirewall [ cfg.port ];

    systemd.services.ecen602-http-proxy = {
      description = "ECEN 602 Caching HTTP Proxy";
      wantedBy = [ "multi-user.target" ];
      after = [ "network.target" ];
      serviceConfig = {
        ExecStart = "${cfg.package}/bin/http_proxy ${cfg.listenAddress} ${toString cfg.port}";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
