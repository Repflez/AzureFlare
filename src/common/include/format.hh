#pragma once

#include <string>
#include <vector>
#include <cmath>

#include <stdarg.h>

namespace Utils
{
	class Format final
	{
		public:
			static std::string VA(const char* fmt, ...);
			static std::string Replace(const std::string& orig, const std::string& fnd, const std::string& repl);
			static auto HashString(const std::string& string) -> std::uint32_t;
			static wchar_t* c_to_w(const char* in);
            static char* strscat(char *dest, const char *src, size_t dest_size);
            static int32_t strscpy(char *dest, const char *src, size_t count);

			static auto Split(std::string string, const std::string& delimiter) -> std::vector<std::string>;

			template<typename T> static bool CompareFloats(T f1, T f2)
			{
				return (std::fabs(f1 - f2) <= std::numeric_limits<T>::epsilon() * std::fmax(std::fabs(f1), std::fabs(f2)));
				//return (fabs(A - B) < epsilon);
			}

            template<size_t count> static FORCE_INLINE char *strscat(char (&dest)[count], const char *src)
            {
                return strscat(dest, src, count);
            }

			template<typename S> static auto stoi(const char* string, S d) -> S
			{
				char* end;

				const auto result = static_cast<S>(std::strtol(string, &end, 0));

				if (*string == '\0' || *end != '\0')
				{
					return d;
				}

				return result;
			}

	};
}