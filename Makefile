

all: basictests benchmark

benchmark: benchmarks/benchmark.cpp include/backwardmultiply.h
	c++ -std=c++17 -o benchmark  benchmarks/benchmark.cpp -O2 -I include

basictests: unit/basictests.cpp include/backwardmultiply.h
	c++ -std=c++17 -o basictests  unit/basictests.cpp -O2 -I include


clean:
	rm -r -f benchmark basictests