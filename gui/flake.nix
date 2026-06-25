{
  description = "Zappy GUI Development Environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            cmake
            gnumake
            pkg-config
          ];

          buildInputs = with pkgs; [
            xorg.libX11
            xorg.libXrandr
            xorg.libXinerama
            xorg.libXcursor
            xorg.libXi
            xorg.libXext

            libGL
            libGLU

            wayland
            wayland-protocols
            wayland-scanner
            libxkbcommon

            raylib
            tree
          ];

          shellHook = ''
            export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [ pkgs.libGL pkgs.libGLU ]}:$LD_LIBRARY_PATH"
            echo "⚡ [Zappy GUI Env] Environnement de dev Flake chargé avec succès ! ⚡"
          '';
        };
      }
    );
}