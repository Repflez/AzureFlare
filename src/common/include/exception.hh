#pragma once

#include <initializer_list>
#include <cstdint>
#include <chrono>

#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <stdlib.h>

namespace Utils
{
	class Exception final
	{
	public:
		static void init(const char* prefix);
		static void purecall_handler();

	private:
		static long __stdcall exception_handler(::EXCEPTION_POINTERS* ex);
		static long __stdcall exception_filter(::EXCEPTION_POINTERS* ex);

		static const char* get_exception_name(DWORD code);

		static std::initializer_list<std::uint32_t> safe_exceptions_;
		static std::uint32_t dump_type_;
		static const char* dump_prefix_;
	};
}