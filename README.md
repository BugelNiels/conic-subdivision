# conic-subdivision

Repository containing the implementation of a conic subdivision scheme for curves.


## Prerequisites

You need the following to be able to compile and run the project:

* [Make](https://www.gnu.org/software/make/)
* [Qt](https://www.qt.io/)
* [Armadillo](http://arma.sourceforge.net/)
    * [Useful installation guide](https://www.uio.no/studier/emner/matnat/fys/FYS4411/v13/guides/installing-armadillo/)


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

- Calculation settings can be found in the menu on the left;
- View settings, such as which items to show/hide can be found in the top menu
- Presets can be found in the top menu
- Light/dark mode can be enabled via the top right

Main view:

- Click and drag to move points/normals
- Right-clicking adds a new point
- Double-clicking on a normal will reset it
- Holding control favours normal selection over vertex selection when they are close together