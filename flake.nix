{
  description = "Python C++ project development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    openeb = {
      url = "github:prophesee-ai/openeb/5.0.0";
      flake = false;
    };
    libcaer = {
      url = "https://gitlab.com/inivation/dv/libcaer/-/archive/master/libcaer-master.tar.gz";
      type = "tarball";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, flake-utils, openeb, libcaer }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        pythonEnv = pkgs.python311;
        pyPkgs = pkgs.python311Packages;

        openebPackage = pkgs.stdenv.mkDerivation {
          pname = "openeb";
          version = "5.0.0";
          
          src = openeb;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
            pythonEnv
          ];

          buildInputs = with pkgs; [
            # Core dependencies
            boost
            opencv
            libusb1
            glew
            glfw
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DBUILD_TESTING=OFF"
            "-DBUILD_SAMPLES=OFF"
            "-DUSE_PROTOBUF=OFF"
            "-DCOMPILE_PYTHON3_BINDINGS=OFF"
            "-DHDF5_DISABLED=ON"
            "-DUDEV_RULES_SYSTEM_INSTALL=OFF"
          ];

          meta = with pkgs.lib; {
            description = "Open Event-Based Vision SDK";
            homepage = "https://github.com/prophesee-ai/openeb";
            license = licenses.asl20;
            platforms = platforms.unix;
          };
        };

        # Add libcaer package definition
        libcaerPackage = pkgs.stdenv.mkDerivation {
          pname = "libcaer";
          version = "3.3.9";
          
          src = libcaer;

          nativeBuildInputs = with pkgs; [
            cmake
            pkg-config
          ];

          buildInputs = with pkgs; [
            libusb1
            libserialport
          ];

          cmakeFlags = [
            "-DENABLE_OPENCV=0"
            "-DENABLE_SERIALDEV=1"
          ];

        #   # Updated patch to handle prefix
        #   preConfigure = ''
        #     substituteInPlace CMakeLists.txt \
        #       --replace "set(prefix \${CMAKE_INSTALL_PREFIX})" "set(prefix $out)" \
        #       --replace "\${prefix}/\${CMAKE_INSTALL_LIBDIR}" "\${CMAKE_INSTALL_FULL_LIBDIR}" \
        #       --replace "\${prefix}/\${CMAKE_INSTALL_INCLUDEDIR}" "\${CMAKE_INSTALL_FULL_INCLUDEDIR}"
        #   '';
          preBuild = ''
            substituteInPlace libcaer.pc --replace // /
          '';

          meta = with pkgs.lib; {
            description = "Minimal C library to access neuromorphic sensors";
            homepage = "https://gitlab.com/inivation/dv/libcaer";
            license = licenses.bsd2;
            platforms = platforms.unix;
          };
        };
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            cmake
            ninja
            gcc
            git
            pythonEnv
            poetry
            pipx
            pre-commit
            ccls
            openebPackage
            opencv
            boost
            libcaerPackage
            pyPkgs.numpy
            autoPatchelfHook
          ];

          shellHook = ''
            # Create virtual environment if it doesn't exist
            if [ ! -d .venv ]; then
              echo "Creating virtual environment..."
              ${pythonEnv.interpreter} -m venv .venv
            fi

            # Activate virtual environment
            source .venv/bin/activate

            # Install development dependencies
            if [ ! -f .venv/.initialized ]; then
              echo "Installing development dependencies..."
              pip install -U pip
              pip install scikit-build-core pytest build setuptools_scm
              pip install nanobind --no-deps
              autoPatchelf .venv/
              touch .venv/.initialized
            fi

            # Set environment variables
            export PYTHONPATH="$PWD/src:$PYTHONPATH"
            export CPLUS_INCLUDE_PATH="${pkgs.opencv}/include/opencv4:$CPLUS_INCLUDE_PATH"
            
            echo ""
            echo "Development environment ready!"
            echo "To build the package locally:"
            echo "  pip install -e ."
            echo "To run tests:"
            echo "  pytest"
            echo ""
          '';

          # Add any necessary environment variables
          LD_LIBRARY_PATH = (pkgs.lib.makeLibraryPath [
            openebPackage
            "${openebPackage}"
            pkgs.opencv
            libcaerPackage
          ]);

          NIX_CFLAGS_COMPILE = [
            "-I${pyPkgs.numpy}/${pythonEnv.sitePackages}/numpy/_core/include"
          ];
    
        };

        packages = {
          default = openebPackage;
          libcaer = libcaerPackage;
        };
      }
    );
} 