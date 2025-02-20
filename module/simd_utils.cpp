#include "../include/simd_utils.h"

namespace simd {
    alignas(32) std::array<uint8_t, 32> VectorOps::alignment_helper{};
} // namespace simd 