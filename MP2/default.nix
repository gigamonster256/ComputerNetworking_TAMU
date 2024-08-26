{pkgs}:
{
  sbcp_server = pkgs.stdenv.mkDerivation {
    name = "server";
    src = ./src;
    buildPhase = ''
      make server
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r server $out/bin
    '';
  };
  sbcp_client = pkgs.stdenv.mkDerivation {
    name = "client";
    src = ./src;
    buildPhase = ''
      make client
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r echo $out/bin
    '';
  };
}