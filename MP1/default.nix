{pkgs}: {
  echos = pkgs.stdenv.mkDerivation {
    name = "echos";
    src = ./src;
    buildPhase = ''
      make echos
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r echos $out/bin
    '';
  };
  echo = pkgs.stdenv.mkDerivation {
    name = "echo";
    src = ./src;
    buildPhase = ''
      make echo
    '';
    installPhase = ''
      mkdir -p $out/bin
      cp -r echo $out/bin
    '';
  };
}
