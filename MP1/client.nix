{ stdenv, libtcp }:
stdenv.mkDerivation {
  pname = "echo";
  version = "0.1.0";
  src = ./.;
  buildInputs = [ libtcp ];
  buildPhase = ''
    make -C src echo LIBTCP=${libtcp}/lib/libtcp.a INCLUDE=-I${libtcp}/include
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/echo $out/bin
  '';
  meta.mainProgram = "echo";
}
