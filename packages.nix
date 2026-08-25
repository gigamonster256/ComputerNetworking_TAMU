{
  pkgs ? import <nixpkgs> { },
}:
let
  libtcp = pkgs.callPackage ./libtcp { };
  libsbcp = pkgs.callPackage ./libsbcp { };
  libudp = pkgs.callPackage ./libudp { };
  libtftp = pkgs.callPackage ./libtftp { };
  libhttp = pkgs.callPackage ./libhttp { inherit libtcp; };
in
{
  inherit
    libtcp
    libsbcp
    libudp
    libtftp
    libhttp
    ;

  mp1 = pkgs.callPackage ./MP1 { inherit libtcp; };
  mp2 = pkgs.callPackage ./MP2 { inherit libtcp libsbcp; };
  mp3 = pkgs.callPackage ./MP3 { inherit libudp libtftp; };
  mp4 = pkgs.callPackage ./MP4 { inherit libtcp libhttp; };
}
