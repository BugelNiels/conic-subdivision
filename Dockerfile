FROM ubuntu:22.04

# Set noninteractive mode to prevent prompts
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
    libeigen3-dev \
    libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev \
    x11-apps x11-utils \
    libx11-6 libxext6 libxrender1 libxrandr2 libxcursor1 \
    libxcomposite1 libxdamage1 libxfixes3 libxi6 libxtst6 \
    && rm -rf /var/lib/apt/lists/*

# Set environment variables for Qt GUI
ENV DISPLAY=:0
ENV QT_X11_NO_MITSHM=1

WORKDIR /app

COPY . /app

RUN mkdir -p /app/build
WORKDIR /app/build

RUN cmake .. -DBUILD_UNIT_TESTS=OFF -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc)

CMD ["./conisLauncher"]
