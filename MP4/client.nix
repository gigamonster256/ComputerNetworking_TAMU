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

  makeFlags = [
    "LIBTCP=${libtcp}/lib/libtcp.a"
    "LIBTCP_INC=${libtcp}/include"
    "LIBHTTP=${libhttp}/lib/libhttp.a"
    "LIBHTTP_INC=${libhttp}/include"
  ];

  buildFlags = [ "client" ];
  installTargets = [ "install-client" ];
  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta.mainProgram = "http_client";
}
