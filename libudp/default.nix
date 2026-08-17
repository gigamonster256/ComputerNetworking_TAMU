{ stdenv }:
stdenv.mkDerivation {
  pname = "libudp";
  version = "0.1.0";
  src = ../.;
  buildPhase = ''
    make -C libudp MODE=both
  '';
  installPhase = ''
    make -C libudp install PREFIX=$out
  '';
}
