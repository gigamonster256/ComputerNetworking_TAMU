{ stdenv, libtcp }:
stdenv.mkDerivation {
  pname = "echos";
  version = "0.1.0";
  src = ./.;
  buildInputs = [ libtcp ];
  buildPhase = ''
    make -C src echos LIBTCP=${libtcp}/lib/libtcp.a INCLUDE=-I${libtcp}/include
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/echos $out/bin
  '';
  meta.mainProgram = "echos";
}
