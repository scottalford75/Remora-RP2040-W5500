{
  description = "Remora RP2040-W5500 firmware build environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        # Fetch Pico SDK with all submodules (Nix package is incomplete)
        pico-sdk = pkgs.fetchFromGitHub {
          owner = "raspberrypi";
          repo = "pico-sdk";
          rev = "2.2.0";
          hash = "sha256-8ubZW6yQnUTYxQqYI6hi7s3kFVQhe5EaxVvHmo93vgk=";
          fetchSubmodules = true;
        };

      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            cmake
            gnumake
            gcc-arm-embedded
            picotool
            python3
            pkg-config
          ];

          shellHook = ''
            export PICO_SDK_PATH="${pico-sdk}"
            export PICO_TOOLCHAIN_PATH="${pkgs.gcc-arm-embedded}"

            echo "Remora RP2040-W5500 build environment"
            echo "======================================"
            echo ""
            echo "To build:"
            echo "  cmake -B build -DCMAKE_BUILD_TYPE=Debug"
            echo "  cmake --build build"
            echo ""
            echo "Output: build/remora.uf2"
          '';
        };
      }
    );
}
