{
  stdenv,
  libtcp,
  libsbcp,
}:
stdenv.mkDerivation {
  pname = "sbcp_client";
  version = "0.1.0";
  src = ./.;
  buildInputs = [
    libtcp
    libsbcp
  ];
  buildPhase = ''
    make -C src clean
    make -C src client LIBTCP=${libtcp}/lib/libtcp.a LIBSBCP=${libsbcp}/lib/libsbcp.a INCLUDE="-I${libtcp}/include -I${libsbcp}/include"
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/client $out/bin/sbcp_client
  '';
  meta.mainProgram = "sbcp_client";
}
