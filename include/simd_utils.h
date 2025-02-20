#ifndef SIMD_UTILS_H
#define SIMD_UTILS_H

#include <cstdint>
#include <array>

#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_CPU_X86_64
        #include <immintrin.h>
        #define HAS_AVX2 1
    #elif TARGET_CPU_ARM64
        #include <arm_neon.h>
        #define HAS_NEON 1
    #endif
#elif defined(_WIN32)
    #include <immintrin.h>
    #define HAS_AVX2 1
    #ifdef _MSC_VER
        #pragma warning(disable: 4324)  // Disable padding warning
    #endif
#endif

namespace simd {

#if defined(HAS_AVX2)
    using vec_type = __m256i;
#elif defined(HAS_NEON)
    using vec_type = int16x8_t;
#else
    using vec_type = std::array<int16_t, 16>;
#endif

class alignas(32) VectorOps {
private:
    // Add dummy member to ensure proper alignment without padding warning
    static std::array<uint8_t, 32> alignment_helper;

public:
    static vec_type load(const int16_t* ptr) {
        #if defined(HAS_AVX2)
            return _mm256_load_si256(reinterpret_cast<const __m256i*>(ptr));
        #elif defined(HAS_NEON)
            return vld1q_s16(ptr);
        #else
            vec_type result;
            std::copy(ptr, ptr + 16, result.begin());
            return result;
        #endif
    }

    static void store(int16_t* ptr, vec_type val) {
        #if defined(HAS_AVX2)
            _mm256_store_si256(reinterpret_cast<__m256i*>(ptr), val);
        #elif defined(HAS_NEON)
            vst1q_s16(ptr, val);
        #else
            std::copy(val.begin(), val.end(), ptr);
        #endif
    }

    static vec_type add(vec_type a, vec_type b) {
        #if defined(HAS_AVX2)
            return _mm256_add_epi16(a, b);
        #elif defined(HAS_NEON)
            return vaddq_s16(a, b);
        #else
            vec_type result;
            for(size_t i = 0; i < 16; ++i) {
                result[i] = a[i] + b[i];
            }
            return result;
        #endif
    }

    static vec_type sub(vec_type a, vec_type b) {
        #if defined(HAS_AVX2)
            return _mm256_sub_epi16(a, b);
        #elif defined(HAS_NEON)
            return vsubq_s16(a, b);
        #else
            vec_type result;
            for(size_t i = 0; i < 16; ++i) {
                result[i] = a[i] - b[i];
            }
            return result;
        #endif
    }

    static vec_type mul(vec_type a, vec_type b) {
        #if defined(HAS_AVX2)
            return _mm256_mulhi_epi16(a, b);
        #elif defined(HAS_NEON)
            return vqrdmulhq_s16(a, b);
        #else
            vec_type result;
            for(size_t i = 0; i < 16; ++i) {
                result[i] = static_cast<int16_t>((static_cast<int32_t>(a[i]) * b[i]) >> 16);
            }
            return result;
        #endif
    }

    static vec_type max(vec_type a, vec_type b) {
        #if defined(HAS_AVX2)
            return _mm256_max_epi16(a, b);
        #elif defined(HAS_NEON)
            return vmaxq_s16(a, b);
        #else
            vec_type result;
            for(size_t i = 0; i < 16; ++i) {
                result[i] = std::max(a[i], b[i]);
            }
            return result;
        #endif
    }

    static vec_type min(vec_type a, vec_type b) {
        #if defined(HAS_AVX2)
            return _mm256_min_epi16(a, b);
        #elif defined(HAS_NEON)
            return vminq_s16(a, b);
        #else
            vec_type result;
            for(size_t i = 0; i < 16; ++i) {
                result[i] = std::min(a[i], b[i]);
            }
            return result;
        #endif
    }

    static vec_type zero() {
        #if defined(HAS_AVX2)
            return _mm256_setzero_si256();
        #elif defined(HAS_NEON)
            return vdupq_n_s16(0);
        #else
            return vec_type{};
        #endif
    }

    static vec_type set1(int16_t val) {
        #if defined(HAS_AVX2)
            return _mm256_set1_epi16(val);
        #elif defined(HAS_NEON)
            return vdupq_n_s16(val);
        #else
            return vec_type{val, val, val, val, val, val, val, val,
                          val, val, val, val, val, val, val, val};
        #endif
    }

    static int32_t horizontal_add(vec_type a) {
        #if defined(HAS_AVX2)
            __m256i sum = _mm256_add_epi32(
                _mm256_unpackhi_epi16(a, a),
                _mm256_unpacklo_epi16(a, a)
            );
            alignas(32) int32_t temp[8];
            _mm256_store_si256(reinterpret_cast<__m256i*>(temp), sum);
            return temp[0] + temp[1] + temp[2] + temp[3] +
                   temp[4] + temp[5] + temp[6] + temp[7];
        #elif defined(HAS_NEON)
            int32x4_t sum = vpaddlq_s16(a);
            int32x2_t fold = vadd_s32(vget_low_s32(sum), vget_high_s32(sum));
            return vget_lane_s32(fold, 0) + vget_lane_s32(fold, 1);
        #else
            int32_t sum = 0;
            for(size_t i = 0; i < 16; ++i) {
                sum += a[i];
            }
            return sum;
        #endif
    }
};

} // namespace simd

#endif 