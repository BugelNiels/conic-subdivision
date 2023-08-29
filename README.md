# Conic Subdivision

Repository containing the implementation of a conic subdivision scheme for curves.

The program supports the loading of object files (provided that the `.obj` file contains a single 2D curve).

## Prerequisites

You need the following to be able to compile and run the project:

* [CMake](https://cmake.org/)
* [Qt 6.2+](https://www.qt.io/)
* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) 
  * ```shell
    sudo apt-get install libeigen3-dev
    ```


## Compilation

Compilation can be done easily via the provided build script:

```shell
bash build.sh
```

Alternatively, you can compile it manually (note that this does not create a _release_ build):
```shell
mkdir build
cd build
cmake ..
make -j6
```

## Running

After building the program, you can run it using:

```shell
./conicsubdiv
```

Or run via an IDE of your choice.

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
- Up/Down/Left/Right arrow keys can be used to translate the mesh.