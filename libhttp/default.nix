{ stdenv, libtcp }:
stdenv.mkDerivation {
  pname = "libhttp";
  version = "0.1.0";
  src = ../.;
  buildInputs = [ libtcp ];
  buildPhase = ''
    make -C libhttp MODE=both LIBTCP_ROOT=${libtcp} LIBTCP_LIBDIR=${libtcp}/lib LIBTCP_INC=-I${libtcp}/include
  '';
  installPhase = ''
    make -C libhttp install PREFIX=$out
  '';
}
