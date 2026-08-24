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

  makeFlags = [
    "LIBTCP=${libtcp}/lib/libtcp.a"
    "LIBTCP_INC=${libtcp}/include"
    "LIBHTTP=${libhttp}/lib/libhttp.a"
    "LIBHTTP_INC=${libhttp}/include"
  ];

  buildFlags = [ "server" ];
  installTargets = [ "install-server" ];
  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta.mainProgram = "http_server";
}
