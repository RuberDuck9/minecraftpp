{

  description = "Minecraft++";

  inputs = {
    nixpkgs.url = "github:Nixos/nixpkgs/nixos-26.05";
  };

  outputs = { self, nixpkgs }: 
    let 
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "minecraftpp";
            version = "0.0.1";
            src = ./.;

            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              pkg-config
            ];

            buildInputs = with pkgs; [
              libGL
              glfw
              glm
              stb
            ];
        };
      });

    devShells = forAllSystems (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];

          nativeBuildInputs = with pkgs; [
              gdb
          ];
        };
      });
    };

}
