#pragma once
// Minimal host types for the unmodified DOSBox DBOPL synthesizer.
#include <cstdint>
#include <cstdio>
using Bit8u = uint8_t;
using Bit8s = int8_t;
using Bit16u = uint16_t;
using Bit16s = int16_t;
using Bit32u = uint32_t;
using Bit32s = int32_t;
using Bitu = uintptr_t;
using Bits = intptr_t;
#define INLINE inline
#define DB_FASTCALL
#define GCC_UNLIKELY(x) (x)
#define LOG_MSG(...) std::fprintf(stderr, __VA_ARGS__)
