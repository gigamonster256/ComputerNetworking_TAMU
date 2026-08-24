{
  stdenv,
  libtcp,
  libhttp,
}:
stdenv.mkDerivation {
  pname = "http_server";
  version = "0.1.0";
  src = ./.;
  buildInputs = [
    libtcp
    libhttp
  ];
  buildPhase = ''
    make -C src clean
    make -C src server LIBTCP=${libtcp}/lib/libtcp.a LIBHTTP=${libhttp}/lib/libhttp.a INCLUDE="-I${libhttp}/include -I${libtcp}/include"
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/server $out/bin/http_server
  '';
  meta.mainProgram = "http_server";
}
