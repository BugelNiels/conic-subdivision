# Conis: A library for Conic Curve Subdivision

Repository containing the implementation of a conic subdivision scheme for curves. This framework was the basis for the paper: [A point-normal interpolatory subdivision scheme preserving conics](https://doi.org/10.1016/j.cagd.2024.102347).

The program supports the loading of object files (provided that the `.obj` file contains a single 2D curve).

![CONIS GUI](assets/conis_screenshot.png)

## Prerequisites

To compile and run the project, you will need the following:

1. A C++ compiler such as `g++`
    - Should support at least C++ 17
2. [CMake](https://cmake.org/)
3. [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
4. [Qt 6.2+](https://www.qt.io/)
    - Qt is required to run the program with a GUI. It is optional for building the core library.
6. [Google Test (GTest)](https://github.com/google/googletest)
    - Google Test is required to run unit tests. Pass `-DBUILD_UNIT_TESTS=OFF` to the `cmake` command to skip this requirement.

 Basic instructions are provided for Ubuntu-based distros, Fedora-based distros and MacOS. If you plan to run this on Windows, install & setup [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) and follow the instructions for the Ubuntu-based distros.
In addition, the GUI application will also require OpenGL to run, but this should be available by default on most systems.

- **Ubuntu-based distros**:

     ```bash
     sudo apt-get install -y libeigen3-dev qt6-base-dev qt6-tools-dev libgtest-dev
     ```

- **Fedora-based distros**:

     ```bash
     sudo dnf install -y eigen3-devel qt6-qtbase-devel qt6-qttools-devel gtest-devel
     ```

- **Mac (using Homebrew)**:

     ```bash
     brew install eigen qt6 googletest
     ```

## Getting Started

To start, clone the repository and initialise its submodules:

```bash
git clone https://github.com/BugelNiels/conic-subdivision.git
cd conic-subdivision
git submodule update --init
```

### Quick Start

To build and run:

```shell
./build.sh -r
```

> Note that not all GPUs support doubles in shaders. As such, the default shaders will use floats. To get more accurate results at higher subdivision levels, use the `--enable-shader-double-precision` flag.

### Compilation

Compilation can be done easily via the provided build script:

```shell
./build.sh
```

See `./build.sh --help` for more information on how to use the build script.

#### Manual Compilation

Alternatively, you can compile it manually (note that this does not create a _release_ build):

```bash
mkdir build
cd build
cmake .. -DBUILD_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
make -j6
```

The generated binary will be named `build/conisLauncher`. You can run the program by running this binary:

```sh
./conisLauncher
```

### Running Through Docker

It is also possible to run the application through Docker. This might be useful if you don't want to install the above dependencies on your system or if you have trouble doing so.

```bash
# Ensure you are in the root directory of the project
docker build -t conis .
```

Now the "tricky" part will be to run this Docker image with a GUI (and GPU access). How to do this differs per operating system, so you might have to search for how to do this on your operating system. On Ubuntu with an Intel/AMD GPU, the following should work:

```bash
xhost +local:docker
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri conis:latest
```

You might run into other issues depending on your system and GPU. I can't exhaustively provide all options here, so Google around and create an issue in this repository if you get stuck.

## Usage

Settings:

- Calculation settings can be found in the menu on the left.
- View settings, such as which items to show/hide can be found in the top menu.
- Presets can be found in the top menu.
- Light/dark mode can be enabled via the top right.

Controls:

- Click and drag to move points/normals.
- Right-clicking adds a new point.
- Double-clicking on a normal will reset it.
- Selecting an edge will display the conic constructed based on the patch surrounding said edge (not that this does not automatically insert inflection points)
- Up/Down/Left/Right arrow keys can be used to translate the mesh.

> Note that memory alignment has been (temporarily) disabled for the Eigen data structures due to memory corruption issues. This might have a slight impact on the subdivision performance.

## Design

A simplified diagram of the `core` library:

![UML Diagram CONIS](assets/conis-uml.png)

## Questions or Issues?

If you run into any issues with the setup process or with the application in general, please don't hesitate to [create an issue](https://github.com/BugelNiels/conic-subdivision/issues/new/choose).
