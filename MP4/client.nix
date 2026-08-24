{
  stdenv,
  libtcp,
  libhttp,
}:
stdenv.mkDerivation {
  pname = "http_client";
  version = "0.1.0";
  src = ./.;
  buildInputs = [
    libtcp
    libhttp
  ];
  buildPhase = ''
    make -C src clean
    make -C src client LIBTCP=${libtcp}/lib/libtcp.a LIBHTTP=${libhttp}/lib/libhttp.a INCLUDE="-I${libhttp}/include -I${libtcp}/include"
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/client $out/bin/http_client
  '';
  meta.mainProgram = "http_client";
}
