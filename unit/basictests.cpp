#include "backwardmultiply.h"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

static inline uint64_t rng(uint64_t h) {
  h ^= h >> 33;
  h *= UINT64_C(0xff51afd7ed558ccd);
  h ^= h >> 33;
  h *= UINT64_C(0xc4ceb9fe1a85ec53);
  h ^= h >> 33;
  return h;
}

void print(const std::vector<uint64_t> &b) {
  if (b.size() == 0)
    return;
  for (size_t i = b.size() - 1; i != SIZE_MAX; i--) {
    std::cout << std::hex << std::setfill('0') << std::setw(8) << b[i];
    std::cout << " ";
  }
}

bool unittest(bool smallmultiplier) {
  uint64_t seed = 1;
  std::vector<uint64_t> out1, out2;
  for (size_t t = 0; t < 2000000; t++) {
    if ((t % 10000) == 0) {
      std::cout << ".";
      std::cout.flush();
    }
    std::vector<uint64_t> v = {rng(seed++), rng(seed++), rng(seed++),
                               rng(seed++), rng(seed++), rng(seed++),
                               rng(seed++), rng(seed++), rng(seed++)};
    uint64_t multiplier = rng(seed++);
    if (smallmultiplier) {
      multiplier &= 7;
    }
    out1.resize(v.size() + 1);
    backwardmultiply::multiplication(multiplier, v.data(), v.size(),
                                     out1.data());
    out2.resize(v.size() + 1);
    backwardmultiply::multiplication_backward(multiplier, v.data(), v.size(),
                                              out2.data());
    if (out1 != out2) {
      printf("out1 and out2 differs\n");
      std::cout << "input : ";
      print(v);
      std::cout << std::endl;
      std::cout << multiplier << std::endl;
      printf("out1:");
      print(out1);
      std::cout << std::endl;
      printf("out2:");
      print(out2);
      std::cout << std::endl;
      std::cerr << "bug\n" << std::endl;
      return false;
    }
    ////////////
    /// multiplication_backward_limit
    //////////////
    for (size_t words = 1; words <= v.size(); words++) {
      out2.resize(v.size() + 1);
      std::fill(out2.begin(), out2.end(), 0); // clearing to be sure
      size_t exactwords = backwardmultiply::multiplication_backward_limit(
          multiplier, v.data(), v.size(), out2.data(), words);
      if (exactwords < words) {
        printf("exactwords = %zu words = %zu\n", exactwords, words);
        printf("bug\n");
        abort();
      }

      size_t n = out2.size();

      // check that we are good...
      for (size_t i = n - exactwords; i < n; i++) {
        if (out2[i] != out1[i]) {
          printf("bug in multiplication_backward_limit\n");
          return false;
        }
      }
    }
    ////////////
    /// multiplication_backward_limit_resume
    //////////////
    for (size_t words = 1; words <= v.size(); words++) {
      std::fill(out2.begin(), out2.end(), 0); // clearing to be sure
      size_t index = v.size();
      size_t exactwords =
          backwardmultiply::multiplication_backward_limit_resume(
              multiplier, v.data(), v.size(), out2.data(), words, index);
      if (exactwords < words) {
        printf("exactwords = %zu words = %zu\n", exactwords, words);
        printf("bug\n");
        abort();
      }

      size_t n = out2.size();
      // check that we are good...
      for (size_t i = n - exactwords; i < n; i++) {
        if (out2[i] != out1[i]) {
          printf("out : ");
          print(out2);
          printf("\n");
          printf("true: ");
          print(out1);
          printf("\n");

          printf("bug in multiplication_backward_limit_resume\n");
          return false;
        }
      }
      while (exactwords != n) {
        exactwords = backwardmultiply::multiplication_backward_limit_resume(
            multiplier, v.data(), v.size(), out2.data(), exactwords + 1, index);

        for (size_t i = n - exactwords; i < n; i++) {
          if (out2[i] != out1[i]) {
            printf("out : ");
            print(out2);
            printf("\n");
            printf("true: ");
            print(out1);
            printf("\n");

            printf("**bug in multiplication_backward_limit_resume\n");
            return false;
          }
        }
      }
    }
  }
  std::cout << std::endl;
  return true;
}

int main() {
  if (unittest(false) && unittest(true)) {
    printf("Code looks good.\n");
    return EXIT_SUCCESS;
  }
  printf("Error detected.\n");
  return EXIT_FAILURE;
}
