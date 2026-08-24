{
  stdenv,
  libtcp,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "libhttp";
  version = "0.1.0";
  src = {
    name = "source";
    outPath = builtins.path {
      inherit (finalAttrs.src) name;
      path = ../.;
    };
  };
  sourceRoot = "${finalAttrs.src.name}/libhttp";
  buildInputs = [ libtcp ];
  propagatedBuildInputs = [ libtcp ];
  makeFlags = [
    "MODE=both"
    "PREFIX=${placeholder "out"}"
    "LIBTCP_ROOT=${libtcp}"
    "LIBTCP_LIBDIR=${libtcp}/lib"
    "LIBTCP_INC=-I${libtcp}/include"
  ];
  setupHook = ./setup-hook.sh;
})
