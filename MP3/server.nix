{
  stdenv,
  libudp,
  libtftp,
}:
stdenv.mkDerivation {
  pname = "tftp_server";
  version = "0.1.0";

  src = ./.;

  buildInputs = [
    libudp
    libtftp
  ];

  makeFlags = [
    "LIBUDP=${libudp}/lib/libudp.a"
    "LIBUDP_INC=${libudp}/include"
    "LIBTFTP=${libtftp}/lib/libtftp.a"
    "LIBTFTP_INC=${libtftp}/include"
  ];

  buildFlags = [ "server" ];
  installFlags = [ "PREFIX=${placeholder "out"}" ];

  meta.mainProgram = "tftp_server";
}
