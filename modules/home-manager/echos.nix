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
    enable = lib.mkEnableOption "the ECEN 602 TCP echo server (RFC 862) as a systemd user service";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../../packages.nix { inherit pkgs; }).mp1;
      defaultText = lib.literalMD "MP1 built from this repository";
      description = "Package providing the `echos` binary.";
    };

    port = lib.mkOption {
      type = lib.types.port;
      default = 9001;
      description = "TCP port to listen on.";
    };
  };

  config = lib.mkIf cfg.enable {
    systemd.user.services.ecen602-echos = {
      Unit = {
        Description = "ECEN 602 TCP Echo Server";
        After = [ "network.target" ];
      };
      Install = {
        WantedBy = [ "default.target" ];
      };
      Service = {
        ExecStart = "${cfg.package}/bin/echos ${toString cfg.port}";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
