{
  config,
  lib,
  pkgs,
  ...
}:
let
  cfg = config.services.ecen602.tftp;
in
{
  options.services.ecen602.tftp = {
    enable = lib.mkEnableOption "the ECEN 602 Trivial File Transfer Protocol server (RFC 1350) as a systemd user service";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../../packages.nix { inherit pkgs; }).mp3;
      defaultText = lib.literalMD "MP3 built from this repository";
      description = "Package providing the `tftp_server` binary.";
    };

    serveRoot = lib.mkOption {
      type = lib.types.str;
      default = "%h/.local/state/ecen602-tftp";
      description = ''
        Directory served over TFTP. The server resolves requested file names
        relative to its working directory, so RRQ reads and WRQ writes both
        happen inside this directory.
      '';
    };
  };

  config = lib.mkIf cfg.enable {
    systemd.user.services.ecen602-tftp = {
      Unit = {
        Description = "ECEN 602 TFTP Server";
        After = [ "network.target" ];
      };
      Install = {
        WantedBy = [ "default.target" ];
      };
      Service = {
        ExecStart = "${cfg.package}/bin/tftp_server";
        WorkingDirectory = cfg.serveRoot;
        StateDirectory = "ecen602-tftp";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
