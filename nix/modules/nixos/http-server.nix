{
  config,
  lib,
  pkgs,
  ...
}:
let
  cfg = config.services.ecen602.http-server;
in
{
  options.services.ecen602.http-server = {
    enable = lib.mkEnableOption "the ECEN 602 course HTTP origin server";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../packages.nix { inherit pkgs; }).mp4;
      defaultText = lib.literalMD "MP4 built from this repository";
      description = "Package providing the `http_server` binary.";
    };

    port = lib.mkOption {
      type = lib.types.port;
      default = 8000;
      description = "TCP port to listen on.";
    };

    openFirewall = lib.mkOption {
      type = lib.types.bool;
      default = false;
      description = "Automatically open {option}`services.ecen602.http-server.port` in the firewall.";
    };
  };

  config = lib.mkIf cfg.enable {
    networking.firewall.allowedTCPPorts = lib.mkIf cfg.openFirewall [ cfg.port ];

    systemd.services.ecen602-http-server = {
      description = "ECEN 602 HTTP Origin Server";
      wantedBy = [ "multi-user.target" ];
      after = [ "network.target" ];
      serviceConfig = {
        ExecStart = "${cfg.package}/bin/http_server ${toString cfg.port}";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
