# conic-subdivision

Repository containing the implementation of a conic subdivision scheme for curves.


## Prerequisites

You need the following to be able to compile and run the project:

* [CMake](https://cmake.org/)
* [Qt 6.2+](https://www.qt.io/)
* [Eigen]() sudo apt-get install libeigen3-dev


## Setup

```sh
mkdir build
cd build
cmake ..
make -j6
```

To run:

```sh
./ConicSubdiv
```

Or run via QtCreator.

## Usage

Settings:

- Calculation settings_ can be found in the menu on the left;
- View settings_, such as which items to show/hide can be found in the top menu
- Presets can be found in the top menu
- Light/dark mode can be enabled via the top right

Main view:

- Click and drag to move points/normals
- Right-clicking adds a new point
- Double-clicking on a normal will reset it
- Holding control favours normal selection over vertex selection when they are close together
- Up/Down/Left/Right arrow keys can be used to translate the mesh.