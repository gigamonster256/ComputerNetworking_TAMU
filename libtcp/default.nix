{ stdenv }:
stdenv.mkDerivation (finalAttrs: {
  pname = "libtcp";
  version = "0.1.0";
  src = {
    name = "source";
    outPath = builtins.path {
      inherit (finalAttrs.src) name;
      path = ../.;
    };
  };
  sourceRoot = "${finalAttrs.src.name}/libtcp";
  makeFlags = [
    "MODE=both"
    "PREFIX=${placeholder "out"}"
  ];
  setupHook = ./setup-hook.sh;
})
