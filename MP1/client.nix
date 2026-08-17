{ stdenv }:
stdenv.mkDerivation {
  name = "echo";
  src = ../.;
  buildPhase = ''
    make -C MP1/src echo
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp -r MP1/src/echo $out/bin
  '';
}
