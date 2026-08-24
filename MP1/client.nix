{
  stdenv,
  libtcp,
}:
stdenv.mkDerivation {
  pname = "echo";
  version = "0.1.0";

  src = ./.;

  buildInputs = [ libtcp ];

  makeFlags = [
    "LIBTCP=${libtcp}/lib/libtcp.a"
    "LIBTCP_INC=${libtcp}/include"
  ];

  buildFlags = [ "echo" ];
  installTargets = [ "install-echo" ];
  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta.mainProgram = "echo";
}
