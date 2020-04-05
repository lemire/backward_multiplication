#include "backwardmultiply.h"

#include <chrono>
#include <vector>
#include <iostream>

static inline uint64_t rng(uint64_t h) {
  h ^= h >> 33;
  h *= UINT64_C(0xff51afd7ed558ccd);
  h ^= h >> 33;
  h *= UINT64_C(0xc4ceb9fe1a85ec53);
  h ^= h >> 33;
  return h;
}


void demo(size_t integer_size = 32) {
  size_t seed = 17;
  std::vector<uint64_t> out1, out2;
  std::chrono::time_point<std::chrono::steady_clock> start_clock, end_clock;
  std::chrono::duration<double> elapsed1 = std::chrono::duration<double>::max();
  std::chrono::duration<double> elapsed2 = std::chrono::duration<double>::max();

  for (size_t t = 0; t < 100; t++) {
    std::vector<uint64_t> v;
    for(size_t i = 0; i < integer_size; i++) v.push_back(rng(seed++));
    uint64_t multiplier = rng(seed++);
    out1.resize(v.size() + 1);
    start_clock = std::chrono::steady_clock::now();
    backwardmultiply::multiplication(multiplier, v.data(), v.size(),
                                     out1.data());

    end_clock = std::chrono::steady_clock::now();
    elapsed1 =
        end_clock - start_clock < elapsed1 ? end_clock - start_clock : elapsed1;
    out2.resize(v.size() + 1);
    start_clock = std::chrono::steady_clock::now();
    backwardmultiply::multiplication_backward(multiplier, v.data(), v.size(),
                                              out2.data());
    end_clock = std::chrono::steady_clock::now();
    elapsed2 =
        end_clock - start_clock < elapsed2 ? end_clock - start_clock : elapsed2;

  }
    std::cout
      << "forward      : "
      << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed1).count() /
             128.0
      << " ns" << std::endl;
    std::cout
      << "backward      : "
      << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed2).count() /
             128.0
      << " ns" << std::endl;
}

int main() {
    demo();
}