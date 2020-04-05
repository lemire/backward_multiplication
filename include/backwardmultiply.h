#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace backwardmultiply {

struct value128 {
  uint64_t low;
  uint64_t high;

  void add(uint64_t lowbits) {
    low = lowbits + low;
    if (low < lowbits) {
      high++;
    }
  }
};

#ifdef _MSC_VER
#include <intrin.h>
#endif

value128 full_multiplication(uint64_t value1, uint64_t value2) {
  value128 answer;
#ifdef _MSC_VER
  // todo: this might fail under visual studio for ARM
  answer.low = _umul128(value1, value2, &answer.high);
#else
  __uint128_t r = ((__uint128_t)value1) * value2;
  answer.low = r;
  answer.high = r >> 64;
#endif
  return answer;
}

// Multiply the integer represented by 'w' with the integer represented
// by the multiword integer b[0], b[1],..., b[n-1]
// Result gets written to output, up to n+1 words can be written: caller is
// responsible to ensure that the memory was allocated.
void multiplication(uint64_t w, const uint64_t *b, size_t n, uint64_t *output) {
  if ((w == 0) || (n == 0)) { // special case
    std::fill(output, output + n + 1, 0);
    return;
  }
  auto p = full_multiplication(w, b[0]);
  output[0] = p.low;
  uint64_t r = p.high;
  for (size_t i = 1; i < n; i++) {
    p = full_multiplication(w, b[i]);
    p.add(r);
    output[i] = p.low;
    r = p.high;
  }
  output[n] = r;
}

// Multiply the integer represented by 'w' with the integer represented
// by the multiword integer b[0], b[1],..., b[n-1]
// Result gets written to output, up to n+1 words can be written: caller is
// responsible to ensure that the memory was allocated.
void multiplication_backward(uint64_t w, const uint64_t *b, size_t n,
                             uint64_t *output) {
  if ((w == 0) || (n == 0)) { // special case
    std::fill(output, output + n + 1, 0);
    return;
  }
  auto p = full_multiplication(w, b[n - 1]);
  output[n - 1] = p.low;
  output[n] = p.high;
  for (size_t i = n - 2; i != SIZE_MAX; i--) {
    p = full_multiplication(w, b[i]);
    output[i] = p.low;
    bool overflow = (output[i + 1] + p.high < output[i + 1]);
    output[i + 1] += p.high;
    for (size_t j = i + 2; overflow; j++) {
      output[j]++;
      overflow = (output[j] == 0);
    }
  }
}


// Multiply the integer represented by 'w' with the integer represented
// by the multiword integer b[0], b[1],..., b[n-1]
// Result gets written to output, up to n+1 words can be written: caller is
// responsible to ensure that the memory was allocated. The parameter "minexactwords"
// represents the minimal number of exact words (starting from the most
// significant words) that you need. Returns the number of exact words computed
// (starting from the most significant words).
size_t multiplication_backward_limit(uint64_t w, const uint64_t *b, size_t n,
                                     uint64_t *output, size_t minexactwords) {
  if ((w == 0) || (n == 0)) { // special case
    std::fill(output, output + n + 1, 0);
    return n + 1;
  }

  auto p = full_multiplication(w, b[n - 1]);
  output[n - 1] = p.low;
  output[n] = p.high;
  if (minexactwords <= 1) {
    // check if addition overflows
    if (p.low + w - 1 >= p.low) {
      // we are good at output[n]
      return 1;
    }
  }
  for (size_t i = n - 2; i != SIZE_MAX; i--) {
    p = full_multiplication(w, b[i]);
    output[i] = p.low;
    bool overflow = (output[i + 1] + p.high < output[i + 1]);
    output[i + 1] += p.high;
    for (size_t j = i + 2; overflow; j++) {
      output[j]++;
      overflow = (output[j] == 0);
    }
    if (minexactwords <= n - i) {
      // check if addition overflows
      if (p.low + w - 1 >= p.low) {
        // we are good at output[n], ..., output[i+1], so we have
        // n - (i+1) + 1 = n - i good words
        return n - i;
      }
      if (minexactwords < n - i) {
        // then the only way we could overflow is if the following is true
        if (output[i + 1] != 0xFFFFFFFFFFFFFFFF) {
          // we are good at output[n], ..., output[i+2], so we have
          // n - (i+2) + 1 = n - i - 1 good words

          return n - i - 1;
        }
      }
    }
  }
  return n + 1;
}

// Multiply the integer represented by 'w' with the integer represented
// by the multiword integer b[0], b[1],..., b[n-1].
// Result gets written to output, up to n+1 words can be written: caller is
// responsible to ensure that the memory was allocated. previousindex should
// start with n + 1 This function can be used to iteratively compute the answer.
// Returns the number of exact words computed (starting from the most
// significant words).
size_t multiplication_backward_limit_resume(uint64_t w, const uint64_t *b,
                                            size_t n, uint64_t *output,
                                            size_t minexactwords,
                                            size_t &previousindex) {
  if (n == 0) { // special case
    std::fill(output, output + n + 1, 0);
    return n + 1; // we are done!!!
  }
  if (previousindex > n) {
    // assume we are done
    return n + 1;
  }
  if (previousindex == n) {
    previousindex = n - 1;
    auto p = full_multiplication(w, b[n - 1]);
    output[n - 1] = p.low;
    output[n] = p.high;
    if (minexactwords <= 1) {
      // check if addition overflows
      if (p.low + w - 1 >= p.low) {
        // we are good
        return 1;
      }
    }
  }

  for (size_t i = previousindex - 1; i != SIZE_MAX; i--) {
    auto p = full_multiplication(w, b[i]);
    output[i] = p.low;
    bool overflow = (output[i + 1] + p.high < output[i + 1]);
    output[i + 1] += p.high;
    for (size_t j = i + 2; overflow; j++) {
      output[j]++;
      overflow = (output[j] == 0);
    }
    if (minexactwords <= n - i) {
      // check if addition overflows
      if (p.low + w - 1 >= p.low) {
        // we are good
        previousindex = i;
        return n - i;
      }
      if (minexactwords < n - i) {
        // then the only way we could overflow is if the following is true
        if (output[i + 1] != 0xFFFFFFFFFFFFFFFF) {
          previousindex = i;
          return n - i - 1;
        }
      }
    }
  }
  previousindex = SIZE_MAX;
  return n + 1;
}

}; // namespace backwardmultiply