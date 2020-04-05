## Backward Multiplication

This C/C++ code illustrates how we might do a multiword multiplication backward,
starting from the most significant words.

To do the full computation the usual way, use the following function:

```c++
#include "backwardmultiply.h"
using backwardmultiply;

// Multiply the integer represented by 'w' with the integer represented
// by the multiword integer b[0], b[1],..., b[n-1]
// Result gets written to output, up to n+1 words can be written: caller is
// responsible to ensure that the memory was allocated.
void multiplication(uint64_t w, const uint64_t *b, size_t n, uint64_t *output);
```

If you just want a few of the most significant words, use the following function:

```c++
// Multiply the integer represented by 'w' with the integer represented
// by the multiword integer b[0], b[1],..., b[n-1]
// Result gets written to output, up to n+1 words can be written: caller is
// responsible to ensure that the memory was allocated. The parameter "minexactwords"
// represents the minimal number of exact words (starting from the most
// significant words) that you need. Returns the number of exact words computed
// (starting from the most significant words).
size_t multiplication_backward_limit(uint64_t w, const uint64_t *b, size_t n,
                                     uint64_t *output, size_t minexactwords)
```


## Requirements

- Modern C++ compiler