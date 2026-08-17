{ stdenv }:
stdenv.mkDerivation {
  pname = "libtftp";
  version = "0.1.0";
  src = ../.;
  buildPhase = ''
    make -C libtftp MODE=both
  '';
  installPhase = ''
    make -C libtftp install PREFIX=$out
  '';
}
