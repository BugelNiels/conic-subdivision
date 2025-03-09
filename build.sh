#!/bin/bash

set -e

# Help message
usage() {
  echo "Builds the conis software."
  echo ""
  echo "Usage: $0 [options]"
  echo ""
  echo "options:"
  echo "  -h, --help:                           Shows help output."
  echo "  -c, --clean:                          Cleans the build directory."
  echo "      --skip-cmake:                     Skips the cmake step of the build_rpm stage during the build process."
  echo "  -d, --debug:                          Builds the program in Debug mode instead of Release."
  echo "  -t  --test:                           Builds and runs the unit tests."
  echo "  -r, --run:                            Runs the built binary."
  echo "  -l, --library-only:                   Only builds the core library. No Qt needed to run this"
  echo "  --disable-shader-double-precision:    Use floats in the shader instead of double. Use this for GPUs that don't support doubles."
  exit 1
}

build() {
  # defaults
  build_dir="_build"

  # input args
  local build_type="Release"
  local do_tests=false
  local clean=false
  local skip_cmake=false
  local run=false
  local library_only=false
  local shader_double_precision=true


  # Parse command line arguments
  while [[ "$#" -gt 0 ]]; do
    case $1 in
      -h | --help) usage ;;
      -d|--debug) build_type="Debug" ;;
      -c|--clean) clean=true ;;
      --skip-cmake) skip_cmake=true ;;
      -t|--test) do_tests=true ;;
      -r|--run) run=true ;;
      -l|--library-only) library_only=true ;;
      --disable-shader-double-precision) shader_double_precision=false ;;
      *)
        echo "Unrecognised command: $1"
        usage
        ;;
    esac
    shift
  done


  # Setup the correct working directory
  initial_loc=$(pwd)
  cd "$(dirname "${BASH_SOURCE[0]}")"

  if [ ${clean} = true ]; then
    echo "Cleaning build directory"
    rm -rf ${build_dir}
  fi

  # Builds the project
  mkdir -p ${build_dir}
  cd ${build_dir}/ || exit
  if [ ${skip_cmake} = false ]; then
    local cmake_flags=""
    if [ ${do_tests} = true ]; then
      cmake_flags+=" -DBUILD_UNIT_TESTS=ON"
    else
      cmake_flags+=" -DBUILD_UNIT_TESTS=OFF"
    fi
    if [ ${library_only} = true ]; then
      cmake_flags+=" -DLIBRARY_ONLY=ON"
    else
      cmake_flags+=" -DLIBRARY_ONLY=OFF"
    fi
    if [ ${shader_double_precision} = true ]; then
      cmake_flags+=" -DSHADER_DOUBLE_PRECISION=ON"
    else
      cmake_flags+=" -DSHADER_DOUBLE_PRECISION=OFF"
    fi
    cmake .. -DCMAKE_BUILD_TYPE=${build_type} ${cmake_flags} -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g -fno-omit-frame-pointer"
  fi
  make -j$(nproc)
  if [[ $? -eq 0 ]]; then
    echo "Build success."
    echo ""
    echo -e "\tYou can find the generated binary in the \"${build_dir}/\" directory."
    echo -e "\tExecute using:"
    echo -e "\t\t./${build_dir}/conisLauncher"
    if [ ${do_tests} = true ]; then
      echo -e "\tExecute tests using:"
      echo -e "\t\t./${build_dir}/conisTests"
    fi
    echo ""
  else
    echo "Build failed."
    exit 1
  fi

  if [ ${do_tests} = true ]; then
    echo "Running tests"
    ./conisTests
  fi

  if [ ${library_only} = false ] && [ ${run} = true ]; then
    echo "Running CONIS"
    ./conisLauncher
  fi

  cd "$initial_loc"
}

build "$@"
