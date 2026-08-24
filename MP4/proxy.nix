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

  makeFlags = [
    "LIBTCP=${libtcp}/lib/libtcp.a"
    "LIBTCP_INC=${libtcp}/include"
    "LIBHTTP=${libhttp}/lib/libhttp.a"
    "LIBHTTP_INC=${libhttp}/include"
  ];

  buildFlags = [ "proxy" ];
  installTargets = [ "install-proxy" ];
  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta.mainProgram = "http_proxy";
}
