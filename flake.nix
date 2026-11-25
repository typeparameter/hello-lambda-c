{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      inherit (nixpkgs) lib;
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ];
      forAllSystems =
        f:
        lib.genAttrs systems (
          system:
          f (
            import nixpkgs {
              inherit system;
              overlays = [ self.overlays.default ];
            }
          )
        );
    in
    {
      overlays.default = final: prev: {
        aws-lambda-c = final.callPackage ./package.nix { };
      };

      packages = forAllSystems (
        pkgs:
        let
          target = if pkgs.stdenv.hostPlatform.isLinux then pkgs.pkgsStatic else pkgs;
        in
        {
          default = pkgs.aws-lambda-c;
          example-hello = target.callPackage ./examples/hello/package.nix { };
        }
      );

      checks = forAllSystems (pkgs: self.packages.${pkgs.stdenv.hostPlatform.system});

      formatter = forAllSystems (
        pkgs:
        pkgs.treefmt.withConfig {
          runtimeInputs = [
            pkgs.clang-tools
            pkgs.nixfmt
          ];
          settings = {
            tree-root-file = "flake.nix";
            formatter.clang-format = {
              command = "clang-format";
              options = [ "-i" ];
              includes = [
                "*.c"
                "*.h"
              ];
            };
            formatter.nixfmt = {
              command = "nixfmt";
              includes = [ "*.nix" ];
            };
          };
        }
      );
    };
}
