{
  pkgs,
  mps,
}:
{
  mp1-echo = pkgs.testers.runNixOSTest {
    imports = [ ./mp1-echo.nix ];
    _module.args.mps = mps;
  };

  mp2-sbcp-connect = pkgs.testers.runNixOSTest {
    imports = [ ./mp2-sbcp-connect.nix ];
    _module.args.mps = mps;
  };

  mp2-sbcp-idle = pkgs.testers.runNixOSTest {
    imports = [ ./mp2-sbcp-idle.nix ];
    _module.args.mps = mps;
  };

  mp2-sbcp-chat = pkgs.testers.runNixOSTest {
    imports = [ ./mp2-sbcp-chat.nix ];
    _module.args.mps = mps;
  };

  mp3-tftp = pkgs.testers.runNixOSTest {
    imports = [ ./mp3-tftp.nix ];
    _module.args.mps = mps;
  };

  mp4-http-proxy = pkgs.testers.runNixOSTest {
    imports = [ ./mp4-http-proxy.nix ];
    _module.args.mps = mps;
  };
}
