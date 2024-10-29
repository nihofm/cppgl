all:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -Wno-dev && cmake --build build --parallel

debug:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja -Wno-dev && cmake --build build --parallel

clean:
	rm -rf build
