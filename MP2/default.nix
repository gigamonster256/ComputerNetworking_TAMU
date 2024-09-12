{pkgs}: {
  sbcp_server = pkgs.stdenv.mkDerivation {
    name = "server";
    src = ../.;
    buildPhase = ''
      make -C MP2/src server
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r MP2/src/server $out/bin
    '';
  };
  sbcp_client = pkgs.stdenv.mkDerivation {
    name = "client";
    src = ../.;
    buildPhase = ''
      make -C MP2/src client
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r MP2/src/client $out/bin
    '';
  };
}
