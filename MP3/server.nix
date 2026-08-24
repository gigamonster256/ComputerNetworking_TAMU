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
  buildPhase = ''
    make -C src clean
    make -C src server LIBUDP=${libudp}/lib/libudp.a LIBTFTP=${libtftp}/lib/libtftp.a INCLUDE="-I${libudp}/include -I${libtftp}/include"
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp src/server $out/bin/tftp_server
  '';
  meta.mainProgram = "tftp_server";
}
