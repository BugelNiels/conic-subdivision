#!/bin/bash

# Help message
usage() {
  echo "Performs the build of CTA through a dedicated Kubernetes pod."
  echo "The pod persists between runs of this script (unless the --reset flag is specified), which ensures that the build does not need to happen from scratch."
  echo "It is also able to deploy the built rpms via minikube for a basic testing setup."
  echo ""
  echo "Important prerequisite: this script expects a CTA/ directory in /home/cirunner/shared/ on a VM"
  echo ""
  echo "Usage: $0 [options]"
  echo ""
  echo "options:"
  echo "  -h, --help:                               Shows help output."
  echo "  -r, --reset:                              Shut down the build pod and start a new one to ensure a fresh build."
  echo "  -o, --operating-system <os>:              Specifies for which operating system to build the rpms. Supported operating systems: [cc7, alma9]. Defaults to alma9 if not provided."
  echo "      --skip-build:                         Skips the build step."
  echo "      --skip-deploy:                        Skips the redeploy step."
  echo "      --skip-cmake:                         Skips the cmake step of the build_rpm stage during the build process."
  echo "      --skip-unit-tests:                    Skips the unit tests. Speeds up the build time by not running the unit tests."
  echo "      --cmake-build-type <build-type>:      Specifies the build type for cmake. Must be one of [Release, Debug, RelWithDebInfo, or MinSizeRel]."
  echo "      --force-install:                      Adds the --install flag to the build_rpm step, regardless of whether the pod was reset or not."
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
      --clean) clean=true ;;
      --skip-cmake) skip_cmake=true ;;
      --run) run=true ;;
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
  fi

  if [ ${run} = true ]; then
    ./conicsubdiv
  fi

  cd "$initial_loc"
}

build "$@"
