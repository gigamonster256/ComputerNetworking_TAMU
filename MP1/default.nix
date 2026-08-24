{ stdenv, libtcp }:
stdenv.mkDerivation {
  pname = "mp1";
  version = "0.1.0";

  src = ./.;

  makeFlags = [ "PREFIX=${placeholder "out"}" ];

  buildInputs = [ libtcp ];

  meta.mainProgram = "echos";
}
