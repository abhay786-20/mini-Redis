build:
	cmake --build build

run: build
	./build/mini-redis

watch:
	find src -name "*.cpp" -o -name "*.hpp" | entr -r sh -c "cmake --build build && ./build/mini-redis"

clean:
	rm -rf build