#!/bin/bash

# Help message
usage() {
  echo "Builds the conicsubdiv binary."
  echo ""
  echo "Usage: $0 [options]"
  echo ""
  echo "options:"
  echo "  -h, --help:                               Shows help output."
  echo "  -c, --clean:                              Cleans the build directory."
  echo "      --skip-cmake:                         Skips the cmake step of the build_rpm stage during the build process."
  echo "  -b, --cmake-build-type <build-type>:      Specifies the build type for cmake. Must be one of [Release, Debug, RelWithDebInfo, or MinSizeRel]."
  echo "  -r, --run:                                Runs the built binary."
  exit 1
}

build() {
  # defaults
  build_dir="_build"

  # input args
  local build_type="Release"
  local clean=false
  local skip_cmake=false
  local run=false


  # Parse command line arguments
  while [[ "$#" -gt 0 ]]; do
    case $1 in
      -h | --help) usage ;;
      -b|--cmake-build-type)
        if [[ $# -gt 1 ]]; then
          if [ "$2" != "Release" ] && [ "$2" != "Debug" ] && [ "$2" != "RelWithDebInfo" ] && [ "$2" != "MinSizeRel" ]; then
            echo "--cmake-build-type must be one of [Release, Debug, RelWithDebInfo, or MinSizeRel]."
            exit 1
          fi
          build_type="$2"
          shift
        else
          echo "Error: --cmake-build-type requires an argument"
          usage
        fi
        ;;
      -c|--clean) clean=true ;;
      --skip-cmake) skip_cmake=true ;;
      -r|--run) run=true ;;
      *)
        usage
        ;;
    esac
    shift
  done


  # Setup the correct working directory
  initial_loc=$(pwd)
  cd "$(dirname "${BASH_SOURCE[0]}")"

  if [ ${clean} = true ]; then
    rm -rf ${build_dir}
  fi

  # Builds the project
  mkdir -p ${build_dir}
  cd ${build_dir}/ || exit
  if [ ${skip_cmake} = false ]; then
    cmake .. -DCMAKE_BUILD_TYPE=${build_type}
  fi
  make -j$(nproc)
  if [[ $? -eq 0 ]]; then
    echo "Build success."
    echo ""
    echo -e "\tYou can find the generated binary in the \"${build_dir}/\" directory."
    echo -e "\tExecute using:"
    echo -e "\t\t./${build_dir}/conicsubdiv"
    echo ""
  else
    echo "Build failed."
    exit 1
  fi

  if [ ${run} = true ]; then
    ./conicsubdiv
  fi

  cd "$initial_loc"
}

build "$@"
