{ stdenv }:
stdenv.mkDerivation {
  pname = "libtcp";
  version = "0.1.0";
  src = ../.;
  buildPhase = ''
    make -C libtcp MODE=both
  '';
  installPhase = ''
    make -C libtcp install PREFIX=$out
  '';
}
