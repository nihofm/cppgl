FROM ubuntu

# set timezone
ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install deps
RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y build-essential libwayland-dev libxkbcommon-dev xorg-dev libopengl-dev freeglut3-dev cmake ninja-build

# build
WORKDIR /workspace
COPY CMakeLists.txt ./
COPY src/ src/
COPY submodules/ submodules/
COPY examples/ examples/
RUN cmake -S . -B build -DCPPGL_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release -G Ninja -Wno-dev && cmake --build build --parallel
