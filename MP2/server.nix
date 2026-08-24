{
  stdenv,
  libtcp,
  libsbcp,
}:
stdenv.mkDerivation {
  pname = "sbcp_server";
  version = "0.1.0";
  src = ./.;
  buildInputs = [
    libtcp
    libsbcp
  ];
  buildPhase = ''
    make -C src clean
    make -C src server LIBTCP=${libtcp}/lib/libtcp.a LIBSBCP=${libsbcp}/lib/libsbcp.a INCLUDE="-I${libtcp}/include -I${libsbcp}/include"
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/server $out/bin/sbcp_server
  '';
  meta.mainProgram = "sbcp_server";
}
