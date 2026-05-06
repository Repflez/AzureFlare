#pragma once

#include <string>
#include <thread>

using namespace std::literals;

// Windows includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define NOINLINE
#define FORCE_INLINE
#endif