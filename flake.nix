{
  description = "ECEN 602: Computer Networking";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    inputs@{
      self,
      nixpkgs,
      ...
    }:
    let
      inherit (nixpkgs) lib;
      systems = lib.singleton "x86_64-linux";
      forEachSystem = f: lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      formatter = forEachSystem (pkgs: pkgs.nixfmt-tree);
      packages = forEachSystem (
        pkgs:
        let
          libtcp = pkgs.callPackage ./libtcp { };
        in
        {
          inherit libtcp;
          libsbcp = pkgs.callPackage ./libsbcp { };
          libudp = pkgs.callPackage ./libudp { };
          libtftp = pkgs.callPackage ./libtftp { };
          libhttp = pkgs.callPackage ./libhttp { inherit libtcp; };
          echos = pkgs.callPackage ./MP1/server.nix { inherit libtcp; };
          echo = pkgs.callPackage ./MP1/client.nix { inherit libtcp; };
        }
      );
      devShells = forEachSystem (pkgs: {
        default = import ./shell.nix { inherit pkgs; };
      });
    };
}
