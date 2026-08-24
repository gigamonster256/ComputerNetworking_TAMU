{
  stdenv,
  libtcp,
  libhttp,
}:
stdenv.mkDerivation {
  pname = "http_proxy";
  version = "0.1.0";
  src = ./.;
  buildInputs = [
    libtcp
    libhttp
  ];
  buildPhase = ''
    make -C src clean
    make -C src proxy LIBTCP=${libtcp}/lib/libtcp.a LIBHTTP=${libhttp}/lib/libhttp.a INCLUDE="-I${libhttp}/include -I${libtcp}/include"
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/proxy $out/bin/http_proxy
  '';
  meta.mainProgram = "http_proxy";
}
