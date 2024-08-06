STATIC_ANALYSIS=valgrind --leak-check=full --exit-on-first-error=yes --error-exitcode=1 --quiet
build:
	cmake --build build/
test: build
	./build/test
benchmark: build
	./build/benchmark
test-dev: build
	${STATIC_ANALYSIS} ./build/test --quiet
dev:
	watch -n 1 "${MAKE} test-dev benchmark"
