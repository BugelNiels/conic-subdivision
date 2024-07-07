#!/bin/bash

# Setup the correct working directory
initial_loc=$(pwd)
cd "$(dirname "${BASH_SOURCE[0]}")"

# Builds the project
mkdir -p build
cd build/ || exit
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
if [[ $? -eq 0 ]]; then
  echo "Build success."
  echo ""
  echo -e "\tYou can find the generated binary in the \"build/\" directory."
  echo -e "\tExecute using:"
  echo -e "\t\t./conicsubdiv"
  echo ""
fi
cd "$initial_loc"