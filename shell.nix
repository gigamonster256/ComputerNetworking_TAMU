{
  pkgs ? import <nixpkgs> { },
  stdenv ? pkgs.stdenv,
}:
pkgs.mkShell.override { inherit stdenv; } {
  packages = [
    pkgs.fd
    pkgs.clang-tools
  ];
}
