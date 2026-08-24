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

          # MP1: Echo server/client
          echos = pkgs.callPackage ./MP1/server.nix { inherit libtcp; };
          echo = pkgs.callPackage ./MP1/client.nix { inherit libtcp; };

          # MP2: Simple Broadcast Chat Protocol
          sbcp_server = pkgs.callPackage ./MP2/server.nix {
            inherit libtcp libsbcp;
          };
          sbcp_client = pkgs.callPackage ./MP2/client.nix {
            inherit libtcp libsbcp;
          };

          # MP3: Trivial File Transfer Protocol
          tftp_server = pkgs.callPackage ./MP3/server.nix {
            inherit libudp libtftp;
          };

          # MP4: Simple HTTP Proxy
          http_client = pkgs.callPackage ./MP4/client.nix {
            inherit libtcp libhttp;
          };
          http_proxy = pkgs.callPackage ./MP4/proxy.nix {
            inherit libtcp libhttp;
          };
          http_server = pkgs.callPackage ./MP4/server.nix {
            inherit libtcp libhttp;
          };
        }
      );
      checks = forEachSystem (pkgs: {
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
      });
      devShells = forEachSystem (pkgs: {
        default = import ./shell.nix { inherit pkgs; };
        clang = import ./shell.nix {
          inherit pkgs;
          stdenv = pkgs.clangStdenv;
        };
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
