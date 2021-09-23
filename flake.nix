{
  description = "C physics project";

  inputs = {
    nixpkgs.url = "github:NixOs/nixpkgs/nixos-21.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, }:
    utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in rec {
        packages = {
          c_physics = pkgs.stdenv.mkDerivation {
            name = "c_physics";
            nativeBuildInputs = with pkgs; [ autoPatchelfHook ];
            buildInputs = with pkgs; [ raylib libGL ];
            src = ./.;
            buildPhase = ''
              export CompilerFlags="-O3 -fno-exceptions -fno-rtti"
              bash ./build.sh
            '';
            installPhase = ''
              mkdir -p $out/bin
              cp build/c_physics $out/bin/c_physics
            '';
          };
        };
        defaultPackage = packages.c_physics;
      });
}
