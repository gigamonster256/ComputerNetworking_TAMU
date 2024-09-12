{pkgs}: {
  echos = pkgs.stdenv.mkDerivation {
    name = "echos";
    src = ../.;
    buildPhase = ''
      make -C MP1/src echos
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r MP1/src/echos $out/bin
    '';
  };
  echo = pkgs.stdenv.mkDerivation {
    name = "echo";
    src = ../.;
    buildPhase = ''
      make -C MP1/src echo
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r MP1/src/echo $out/bin
    '';
  };
}
