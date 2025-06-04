{

  description = "";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    devkitNix.url = "github:bandithedoge/devkitNix";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      devkitNix,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { 
          inherit system;
          overlays = [
            devkitNix.overlays.default
          ];
          config = {
            allowUnfree = true;
          };
        };
      in
      {
        nix.settings = {
          substituters = [
            "https://hydra.lordofthelags.net"
            "https://cache.nixos.org"
          ];
          trusted-users = [
            "root"
            "@wheel"
          ];
          trusted-public-keys = [
            "hydra.nixos.org-1:CNHJZBh9K4tP3EKF6FkkgeVYsS3ohTl+oS0Qa8bezVs="
            "hydra.lordofthelags.net:v3OFf3HWmShqFqJIYCBRDVGpFxyq9Pc8QMflK8hcOYE="
          ];
        };
        devShells.default = pkgs.stdenvNoCC.mkDerivation {
          name = "Nintendo DS shell";
          buildInputs = [
            pkgs.devkitNix.devkitARM
            pkgs.gcc13
          ];
          inherit (pkgs.devkitNix.devkitARM) shellHook;
        };

        packages.default = pkgs.stdenv.mkDerivation {
        name = "DS-game";
        src = ./.;

        # `TARGET` determines the name of the executable.
        makeFlags = ["TARGET=game"];
        # The shell hook is used in the build to point your build system to
        # devkitPro.
        preBuild = pkgs.devkitNix.devkitARM.shellHook;
        # This is a simple Switch app example that only builds a single
        # executable. If your project outputs multiple files, make `$out` a
        # directory and copy everything there.
        installPhase = ''
          cp game.nds $out
        '';
      };


      }
    );
}
