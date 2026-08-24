{
  stdenv,
  libtcp,
}:
stdenv.mkDerivation {
  pname = "echos";
  version = "0.1.0";

  src = ./.;

  buildInputs = [ libtcp ];

  makeFlags = [
    "LIBTCP=${libtcp}/lib/libtcp.a"
    "LIBTCP_INC=${libtcp}/include"
  ];

  buildFlags = [ "echos" ];
  installTargets = [ "install-echos" ];
  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta.mainProgram = "echos";
}
