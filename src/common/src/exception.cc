#include <exception.hh>

#include <ctime>

#include <hook.hh>
#include <format.hh>
#include <utils.hh>

namespace Utils
{
	void Exception::init(const char* prefix)
	{
		Exception::dump_prefix_ = prefix;

		::AddVectoredExceptionHandler(true, Exception::exception_handler);
		::SetUnhandledExceptionFilter(Exception::exception_filter);

		::_set_purecall_handler(Exception::purecall_handler);
	}

	void Exception::purecall_handler()
	{
		Utils::hook::set<std::uint32_t>(0x00000000, 0xDECEA5ED);
	}

	long __stdcall Exception::exception_handler(::EXCEPTION_POINTERS* ex)
	{
		return Utils::any_of(Exception::safe_exceptions_, [ex](const auto value) -> bool
		{
			return value == ex->ExceptionRecord->ExceptionCode;
		}) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
	}

	long __stdcall Exception::exception_filter(::EXCEPTION_POINTERS* ex)
	{
		if (!::IsDebuggerPresent())
		{
			const auto code = ex->ExceptionRecord->ExceptionCode;
			const auto addr = ex->ExceptionRecord->ExceptionAddress;
			const char* exception_name = get_exception_name(code);

#if DEBUG
			MessageBoxA(nullptr, &Format::VA(
				"An exception occurred and the application needs to close.\n\n"
				"Exception: %s (0x%08X)\n"
				"Address: 0x%p",
				exception_name, code, addr)[0], "Critical Error", 0);
#else
			std::string dumpFilename = &Format::VA("%s-%u-%llu.dmp", Exception::dump_prefix_, 51, std::time(nullptr))[0];

			const auto handle = ::CreateFileA(dumpFilename.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			auto info = ::MINIDUMP_EXCEPTION_INFORMATION{ ::GetCurrentThreadId(), ex, false };

			::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), handle, static_cast<::MINIDUMP_TYPE>(Exception::dump_type_), &info, nullptr, nullptr);

			MessageBoxA(nullptr, &Format::VA(
				"An exception occurred and the application needs to close.\n\n"
				"Exception: %s (0x%08X)\n"
				"Address: 0x%p\n\n"
				"A dump file was generated in \"%s\".\n"
				"Please send the file to the developers.",
				exception_name, code, addr, dumpFilename.c_str())[0], "Critical Error", 0);
#endif
			ExitProcess(0);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	std::initializer_list<std::uint32_t> Exception::safe_exceptions_
	{
		STATUS_INTEGER_OVERFLOW,
		STATUS_FLOAT_OVERFLOW,
		DBG_PRINTEXCEPTION_C,
		STATUS_BREAKPOINT,
	};

	std::uint32_t Exception::dump_type_
	{
		::MiniDumpIgnoreInaccessibleMemory |
		::MiniDumpWithProcessThreadData |
		::MiniDumpWithUnloadedModules |
		::MiniDumpWithFullMemoryInfo |
		::MiniDumpWithThreadInfo |
		::MiniDumpWithHandleData |
		::MiniDumpWithDataSegs |
		::MiniDumpWithCodeSegs |
		::MiniDumpScanMemory
	};

	const char* Exception::get_exception_name(DWORD code)
	{
		switch (code) {
		case EXCEPTION_ACCESS_VIOLATION:         return "Access Violation";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "Array Bounds Exceeded";
		case EXCEPTION_BREAKPOINT:               return "Breakpoint";
		case EXCEPTION_DATATYPE_MISALIGNMENT:    return "Datatype Misalignment";
		case EXCEPTION_FLT_DENORMAL_OPERAND:     return "Float Denormal Operand";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "Float Divide by Zero";
		case EXCEPTION_FLT_INEXACT_RESULT:       return "Float Inexact Result";
		case EXCEPTION_FLT_INVALID_OPERATION:    return "Float Invalid Operation";
		case EXCEPTION_FLT_OVERFLOW:             return "Float Overflow";
		case EXCEPTION_FLT_STACK_CHECK:          return "Float Stack Check";
		case EXCEPTION_FLT_UNDERFLOW:            return "Float Underflow";
		case EXCEPTION_ILLEGAL_INSTRUCTION:      return "Illegal Instruction";
		case EXCEPTION_IN_PAGE_ERROR:            return "In Page Error";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "Integer Divide by Zero";
		case EXCEPTION_INT_OVERFLOW:             return "Integer Overflow";
		case EXCEPTION_INVALID_DISPOSITION:      return "Invalid Disposition";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Noncontinuable Exception";
		case EXCEPTION_PRIV_INSTRUCTION:         return "Privileged Instruction";
		case EXCEPTION_SINGLE_STEP:              return "Single Step";
		case EXCEPTION_STACK_OVERFLOW:           return "Stack Overflow";
		case DBG_PRINTEXCEPTION_C:               return "Debug Print Exception";
		default:                                 return "Unknown Exception";
		}
	}

	const char* Exception::dump_prefix_;
}