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

  makeFlags = [
    "LIBTCP=${libtcp}/lib/libtcp.a"
    "LIBTCP_INC=${libtcp}/include"
    "LIBSBCP=${libsbcp}/lib/libsbcp.a"
    "LIBSBCP_INC=${libsbcp}/include"
  ];

  buildFlags = [ "server" ];
  installTargets = [ "install-server" ];
  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta.mainProgram = "sbcp_server";
}
