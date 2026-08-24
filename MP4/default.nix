{
  stdenv,
  libtcp,
  libhttp,
}:
stdenv.mkDerivation {
  pname = "mp4";
  version = "0.1.0";

  src = ./.;

  makeFlags = [ "PREFIX=${placeholder "out"}" ];

  buildInputs = [
    libtcp
    libhttp
  ];

  meta.mainProgram = "http_proxy";
}
