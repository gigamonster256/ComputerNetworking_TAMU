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
    enable = lib.mkEnableOption "the ECEN 602 course HTTP origin server as a systemd user service";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../../packages.nix { inherit pkgs; }).mp4;
      defaultText = lib.literalMD "MP4 built from this repository";
      description = "Package providing the `http_server` binary.";
    };

    port = lib.mkOption {
      type = lib.types.port;
      default = 8000;
      description = "TCP port to listen on.";
    };
  };

  config = lib.mkIf cfg.enable {
    systemd.user.services.ecen602-http-server = {
      Unit = {
        Description = "ECEN 602 HTTP Origin Server";
        After = [ "network.target" ];
      };
      Install = {
        WantedBy = [ "default.target" ];
      };
      Service = {
        ExecStart = "${cfg.package}/bin/http_server ${toString cfg.port}";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
