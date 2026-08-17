{ stdenv }:
stdenv.mkDerivation {
  pname = "libsbcp";
  version = "0.1.0";
  src = ../.;
  buildPhase = ''
    make -C libsbcp MODE=both
  '';
  installPhase = ''
    make -C libsbcp install PREFIX=$out
  '';
}
