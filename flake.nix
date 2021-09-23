{
  description = "C physics project";

  inputs = {
    nixpkgs.url = "github:NixOs/nixpkgs/nixos-21.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, }:
    utils.lib.eachSystem [ "x86_64-linux" ] (system:
      let pkgs = import nixpkgs { inherit system; };
      in rec {
        packages = {
          c_physics = pkgs.stdenv.mkDerivation {
            name = "c_physics";
            nativeBuildInputs = with pkgs; [ autoPatchelfHook ];
            buildInputs = with pkgs; [ raylib libGL ];
            src = ./.;
            dontPatch = true;
            dontConfigure = true;
            dontInstall = true;
            buildPhase = ''
              ./build.sh
            '';
          };
        };
        defaultPackage = packages.c_physics;
      });
}
