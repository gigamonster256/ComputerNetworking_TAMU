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
    enable = lib.mkEnableOption "the ECEN 602 Trivial File Transfer Protocol server (RFC 1350)";

    package = lib.mkOption {
      type = lib.types.package;
      default = (import ../packages.nix { inherit pkgs; }).mp3;
      defaultText = lib.literalMD "MP3 built from this repository";
      description = "Package providing the `tftp_server` binary.";
    };

    serveRoot = lib.mkOption {
      type = lib.types.str;
      default = "/var/lib/ecen602-tftp";
      description = ''
        Directory served over TFTP. The server resolves requested file names
        relative to its working directory, so RRQ reads and WRQ writes both
        happen inside this directory.
      '';
    };

    openFirewall = lib.mkOption {
      type = lib.types.bool;
      default = false;
      description = "Automatically open the TFTP port in the firewall.";
    };
  };

  config = lib.mkIf cfg.enable {
    networking.firewall.allowedUDPPorts = lib.mkIf cfg.openFirewall [ 8080 ];

    systemd.services.ecen602-tftp = {
      description = "ECEN 602 TFTP Server";
      wantedBy = [ "multi-user.target" ];
      after = [ "network.target" ];
      serviceConfig = {
        ExecStart = "${cfg.package}/bin/tftp_server";
        WorkingDirectory = cfg.serveRoot;
        StateDirectory = "ecen602-tftp";
        Restart = "on-failure";
        RestartSec = 1;
      };
    };
  };
}
