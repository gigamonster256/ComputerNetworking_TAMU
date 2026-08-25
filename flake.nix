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

      # All libraries and machine problems, built against a given pkgs.
      machineProblems = import ./packages.nix;
    in
    {
      packages = forEachSystem (
        pkgs:
        let
          mps = machineProblems { inherit pkgs; };

          # Cheap view of an MP build exposing one binary as a `nix run` target.
          appOf =
            mp: name:
            pkgs.runCommand name { meta.mainProgram = name; } ''
              mkdir -p $out/bin
              ln -s ${mp}/bin/${name} $out/bin/${name}
            '';
        in
        (
          mps
          // {
            # MP1: Echo server/client
            echos = appOf mps.mp1 "echos";
            echo = appOf mps.mp1 "echo";

            # MP2: Simple Broadcast Chat Protocol
            sbcp_server = appOf mps.mp2 "sbcp_server";
            sbcp_client = appOf mps.mp2 "sbcp_client";

            # MP3: Trivial File Transfer Protocol
            tftp_server = appOf mps.mp3 "tftp_server";

            # MP4: Simple HTTP Proxy
            http_client = appOf mps.mp4 "http_client";
            http_proxy = appOf mps.mp4 "http_proxy";
            http_server = appOf mps.mp4 "http_server";
          }
        )
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
      nixosModules = {
        default = ./modules/nixos;
        echos = ./modules/nixos/echos.nix;
        sbcp = ./modules/nixos/sbcp.nix;
        tftp = ./modules/nixos/tftp.nix;
        http-server = ./modules/nixos/http-server.nix;
        http-proxy = ./modules/nixos/http-proxy.nix;
      };
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
