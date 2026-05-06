#pragma once

#include <string>
#include <thread>

using namespace std::literals;

// Windows includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>

typedef unsigned __int32 uAddr;

#define CONFIG_FILENAME "psobb.cfg"

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#define FORCE_INLINE __forceinline
#define NAKED __declspec(naked)
#define RETURN_ADDRESS() _ReturnAddress()
#elif defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#define FORCE_INLINE inline __attribute__((always_inline))
#define NAKED __attribute__((naked))
#define RETURN_ADDRESS() __builtin_return_address(0)
#else
#define NOINLINE
#define FORCE_INLINE
#define NAKED
#define RETURN_ADDRESS()
#endif