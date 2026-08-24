{
  stdenv,
  libtcp,
  libsbcp,
}:
stdenv.mkDerivation {
  pname = "mp2";
  version = "0.1.0";

  src = ./.;

  makeFlags = [ "PREFIX=${placeholder "out"}" ];

  buildInputs = [
    libtcp
    libsbcp
  ];

  meta.mainProgram = "sbcp_server";
}
