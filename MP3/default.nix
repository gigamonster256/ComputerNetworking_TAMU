{
  stdenv,
  libudp,
  libtftp,
}:
stdenv.mkDerivation {
  pname = "mp3";
  version = "0.1.0";

  src = ./.;

  makeFlags = [ "PREFIX=${placeholder "out"}" ];

  buildInputs = [
    libudp
    libtftp
  ];

  meta.mainProgram = "tftp_server";
}
