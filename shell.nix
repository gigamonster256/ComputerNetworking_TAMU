{
  pkgs ? import <nixpkgs> { },
}:
pkgs.mkShell {
  packages = [
    pkgs.fd
    pkgs.clang-tools
  ];
}
