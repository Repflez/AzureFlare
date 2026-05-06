#include <format.hh>

namespace Utils
{
	std::string Format::VA(const char* fmt, ...)
	{
		auto va = ::va_list();

		va_start(va, fmt);

		char result[512]{};

		std::vsprintf(result, fmt, va);

		return std::string(result);
	}

	wchar_t* Format::c_to_w(_In_ const char* in)
	{
		const size_t size = strlen(in) + 1;
		wchar_t* out = new wchar_t[size];
		mbstowcs(out, in, size);

		return out;
	}

	std::string Format::Replace(_In_ const std::string& orig, _In_ const std::string& fnd, _In_ const std::string& repl)
	{
		std::string ret = orig;
		size_t pos = 0;
		while (true)
		{
			pos = ret.find(fnd, pos);
			if (pos == std::string::npos) break;

			ret.replace(pos, pos + fnd.size(), repl);
			pos += repl.size();
		}
		return ret;
	}

	auto Format::HashString(_In_ const std::string& string) -> std::uint32_t
	{
		std::uint32_t result = 0xFFFFFFFF;
		for(auto i = 0; i < string.length(); i++)
		{
			result = (result * 0x21);
			result += string[i];
		}
		return result;
	}

	auto Format::Split(_In_ std::string string, _In_ const std::string& delimiter) -> std::vector<std::string>
	{
		auto result = std::vector<std::string>();
		auto position = 0u;

		while ((position = string.find(delimiter)) != std::string::npos)
		{
			result.emplace_back(string.substr(0, position));
			string.erase(0, position + delimiter.size());
		}

		result.emplace_back(string);

		return result;
	}

    // Source:
    // https://github.com/withmorten/hackloader/blob/e5b48814eeffec527c2392c79d923914fb55fd58/src/util.cpp

    char* Format::strscat(char *dest, const char *src, size_t dest_size)
    {
        if (!dest_size) return dest;

        size_t dest_content_len = strlen(dest);

        if ((dest_size - dest_content_len) <= 0) return dest;

        return strscpy(dest + dest_content_len, src, dest_size - dest_content_len) < 0 ? NULL : dest + dest_content_len;
    }

    struct word_at_a_time
    {
        const uintptr_t one_bits, high_bits;
    };

    static inline uintptr_t read_word_at_a_time(const void *addr)
    {
        return *(uintptr_t *)addr;
    }

    static inline uintptr_t has_zero(uintptr_t a, uintptr_t *bits, const word_at_a_time *c)
    {
        uintptr_t mask = ((a - c->one_bits) & ~a) & c->high_bits;
        *bits = mask;
        return mask;
    }

    static inline uintptr_t prep_zero_mask(uintptr_t a, uintptr_t bits, const word_at_a_time *c)
    {
        return bits;
    }

    static inline uintptr_t create_zero_mask(uintptr_t bits)
    {
        bits = (bits - 1) & ~bits;
        return bits >> 7;
    }

    static inline intptr_t count_masked_bytes(intptr_t mask)
    {
        intptr_t a = (0x0ff0001 + mask) >> 23;
        return a & mask;
    }

    static inline uintptr_t find_zero(uintptr_t mask)
    {
        return count_masked_bytes(mask);
    }

    int32_t Format::strscpy(char *dest, const char *src, size_t count)
    {
        int32_t res = 0;

        const word_at_a_time constants = { 0x01010101, 0x80808080 };
        const size_t page_size = 1 << 12;

        size_t max = count;

        if ((intptr_t)src & (sizeof(intptr_t) - 1))
        {
            size_t limit = page_size - ((intptr_t)src & (page_size - 1));

            if (limit < max) max = limit;
        }

        while (max >= sizeof(uintptr_t))
        {
            uintptr_t c, data;

            c = read_word_at_a_time(src + res);

            if (has_zero(c, &data, &constants))
            {
                data = prep_zero_mask(c, data, &constants);
                data = create_zero_mask(data);
                *(uintptr_t *)(dest + res) = c & data;
                return res + find_zero(data);
            }

            *(uintptr_t *)(dest + res) = c;

            res += sizeof(uintptr_t);
            count -= sizeof(uintptr_t);
            max -= sizeof(uintptr_t);
        }

        while (count)
        {
            char c = src[res];
            dest[res] = c;

            if (!c) return res;

            res++;
            count--;
        }

        if (res) dest[res - 1] = '\0';

        return -1;
    }
}
