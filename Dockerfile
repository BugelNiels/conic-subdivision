# Use an Ubuntu base image
FROM ubuntu:latest

# Install necessary dependencies
RUN apt-get update && apt-get upgrade -y
RUN apt-get install qt6-base-dev -y
RUN apt-get install libeigen3-dev -y
RUN apt-get install -y wget

# Copy the source code to the container
COPY .. /app


# Set the working directory
WORKDIR /app

RUN mkdir -p build
WORKDIR build/
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=/usr/lib \
    make -j$(nproc) \
    make DESTDIR=../appdir -j$(nproc) install ; find ../appdir/

WORKDIR ../

RUN #wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
RUN cp tools/linuxdeployqt.AppImage linuxdeployqt-continuous-x86_64.AppImage
RUN chmod u+x linuxdeployqt-continuous-x86_64.AppImage
RUN ./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract
# Hack to get the deploy script to work properly
RUN touch /usr/share/doc/libc6/copyright
RUN ./squashfs-root/AppRun appdir/usr/share/applications/*.desktop -appimage -unsupported-allow-new-glibc