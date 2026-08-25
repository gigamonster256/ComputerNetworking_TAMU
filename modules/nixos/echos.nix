{
  config,
  lib,
  pkgs,
  ...
}:
let
  cfg = config.services.ecen602.echos;
in
{
  options.services.ecen602.echos = {
    enable = lib.mkEnableOption "the ECEN 602 TCP echo server (RFC 862)";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../packages.nix { inherit pkgs; }).mp1;
      defaultText = lib.literalMD "MP1 built from this repository";
      description = "Package providing the `echos` binary.";
    };

    port = lib.mkOption {
      type = lib.types.port;
      default = 9001;
      description = "TCP port to listen on.";
    };

    openFirewall = lib.mkOption {
      type = lib.types.bool;
      default = false;
      description = "Automatically open {option}`services.ecen602.echos.port` in the firewall.";
    };
  };

  config = lib.mkIf cfg.enable {
    networking.firewall.allowedTCPPorts = lib.mkIf cfg.openFirewall [ cfg.port ];

    systemd.services.ecen602-echos = {
      description = "ECEN 602 TCP Echo Server";
      wantedBy = [ "multi-user.target" ];
      after = [ "network.target" ];
      serviceConfig = {
        ExecStart = "${cfg.package}/bin/echos ${toString cfg.port}";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
