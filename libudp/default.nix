{ stdenv }:
stdenv.mkDerivation (finalAttrs: {
  pname = "libudp";
  version = "0.1.0";
  src = {
    name = "source";
    outPath = builtins.path {
      inherit (finalAttrs.src) name;
      path = ../.;
    };
  };
  sourceRoot = "${finalAttrs.src.name}/libudp";
  makeFlags = [
    "MODE=both"
    "PREFIX=${placeholder "out"}"
  ];
  setupHook = ./setup-hook.sh;
})
