#!/bin/bash

cd ..
mkdir -p build
cd build/ || exit
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=/usr/lib
make -j$(nproc)
mkdir share/doc/libc6
make DESTDIR=../appdir -j$(nproc) install ; find ../appdir/
if [[ $? -eq 0 ]]; then
  cd ..
  touch appdir/usr/share/doc/libc6/copyright
  ./tools/linuxdeployqt.AppImage appdir/usr/share/applications/*.desktop -unsupported-allow-new-glibc -appimage
  cd scripts || exit
else
  cd .. || exit
fi
