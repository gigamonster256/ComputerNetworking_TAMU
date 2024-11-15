{
  description = "ECEN 602: Computer Networking";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";

    personal-packages = {
      url = "github:gigamonster256/nur-packages";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = inputs @ {
    self,
    nixpkgs,
    personal-packages,
    ...
  }: let
    inherit (nixpkgs) lib;
    systems = [
      "x86_64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ];
    pkgsFor = lib.genAttrs systems (
      system: import nixpkgs {inherit system;} // personal-packages.packages.${system}
    );
    forEachSystem = f: lib.genAttrs systems (system: f pkgsFor.${system});
    MPs = forEachSystem (pkgs: import ./MPs.nix {inherit pkgs;});
  in {
    formatter = forEachSystem (pkgs: pkgs.alejandra);
    packages = MPs;
    devShells = forEachSystem (pkgs: import ./shell.nix {inherit pkgs;});
  };
}
