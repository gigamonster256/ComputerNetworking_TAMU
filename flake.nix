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
      checks = forEachSystem (
        pkgs:
        {
          lint = pkgs.stdenv.mkDerivation {
            name = "computer-networking-lint";

            src = self;

            nativeBuildInputs = [
              pkgs.fd
              pkgs.clang-tools
            ];

            doCheck = true;
            checkPhase = ''
              bash ./lint.sh
            '';

            installPhase = ''
              mkdir -p $out
            '';
          };
        }
      );
      devShells = forEachSystem (pkgs: {
        default = import ./shell.nix { inherit pkgs; };
      });
      formatter = forEachSystem (
        pkgs:
        pkgs.writeShellApplication {
          name = "computer-networking-formatter";

          runtimeInputs = [
            pkgs.fd
            pkgs.nixfmt
            pkgs.clang-tools
          ];

          text = ''
            # Format Nix with Nixfmt
            fd "$@" -t f -e nix -x nixfmt -q '{}'

            # Format C/C++ with ClangFormat
            fd "$@" -t f -e c -e cpp -e h -e hpp -x clang-format -i '{}'
          '';
        }
      );
    };
}
