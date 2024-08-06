STATIC_ANALYSIS=valgrind --leak-check=full --exit-on-first-error=yes --error-exitcode=1 --quiet
.PHONY: build test benchmark test-dev dev init-dev init
init-dev:
	rm -rf build/ && mkdir build && cd build && cmake -GNinja -DCMAKE_BUILD_TYPE=ReleaseWithDebug ..
init:
	rm -rf build/ && mkdir build && cd build && cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
build:
	cmake --build build/
test: build
	./build/test
benchmark: build
	./build/benchmark
test-dev: build
	${STATIC_ANALYSIS} ./build/test --quiet
dev: init-dev
	watch -n 1 "${MAKE} build test-dev benchmark"
