/*
 * Apple Silicon (arm64) 兼容层
 * ------------------------------------------------------------
 * 项目源码直接 #include <immintrin.h> 并使用 SSE/AVX intrinsic，
 * 但 ARM 平台不存在该头文件。此处用 SIMDe 提供的原生别名把
 * __m128 / __m256d / _mm_* / _mm256_* 全部映射到 NEON 实现，
 * 使源码无需任何改动即可在 Apple Silicon 上编译运行。
 *
 * 该目录仅在 CMake 检测到 Apple + arm64 时才会加入 include 路径。
 */
#ifndef RENDERER_COMPAT_ARM_IMMINTRIN_INCLUDE
#define RENDERER_COMPAT_ARM_IMMINTRIN_INCLUDE

#if !defined(SIMDE_ENABLE_NATIVE_ALIASES)
#define SIMDE_ENABLE_NATIVE_ALIASES
#endif

/* avx.h 为 amalgamated 版本，已内含 mmx/sse/sse2/sse3/sse4.1/sse4.2 */
#include <simde/x86/avx.h>
#include <simde/x86/avx2.h>

#endif /* RENDERER_COMPAT_ARM_IMMINTRIN_INCLUDE */
