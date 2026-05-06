#pragma once

#include <fstream>

#define PRINT_FILE_CONSOLE(__FMT__, ...)												\
	if (const auto File = Utils::Console::File())									\
	{																					\
		std::fprintf(File, __FMT__, ##__VA_ARGS__);										\
		std::fflush(File);																\
	}																					\
																						\
	std::printf(__FMT__, ##__VA_ARGS__)


#ifdef DEBUG
	#define PRINT_DEBUG(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ DEBUG ][%s]: " __FMT__ "\n", __FUNCTION__, ##__VA_ARGS__)

	#define PRINT_DEBUG_N(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ DEBUG ]: " __FMT__ "\n", ##__VA_ARGS__)

	#define PRINT_INFO(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ INFO ][%s]: " __FMT__ "\n", __FUNCTION__, ##__VA_ARGS__)

	#define PRINT_WARNING(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ WARNING ][%s]: " __FMT__ "\n", __FUNCTION__, ##__VA_ARGS__)

	#define PRINT_ERROR(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ ERROR ][%s]: " __FMT__ "\n", __FUNCTION__, ##__VA_ARGS__)
#else
	#define PRINT_DEBUG(__FMT__, ...)
	#define PRINT_DEBUG_N(__FMT__, ...)

	#define PRINT_INFO(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ INFO ]: " __FMT__ "\n", ##__VA_ARGS__)

	#define PRINT_WARNING(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ WARNING ]: " __FMT__ "\n", ##__VA_ARGS__)

	#define PRINT_ERROR(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ ERROR ]: " __FMT__ "\n", ##__VA_ARGS__)
#endif

namespace Utils
{
	class Console final
	{
		public:
			static void Init();
			static ::_iobuf* File();

		private:
			static ::_iobuf* file_;
	};
}
