/* Author: Mikko Finell
 * License: Public Domain */

#ifndef CASE_HELPER
#define CASE_HELPER

namespace CASE {

// n mod MAX with floored division, so as to wrap backwards around negative n
template <int MAX>
inline int wrap(const int n) {
    return ((n % MAX) + MAX) % MAX;
}

inline int wrap(const int n, const int MAX) {
    return ((n % MAX) + MAX) % MAX;
}

template <int A, int B>
inline int clamp(const int n) {
    return n < A ? A : n > B ? B : n;
}

}

#endif // HELPER
