#include <hook.hh>

namespace Utils
{
	void hook::nop(std::uint32_t address, std::uint32_t size)
	{
		for (auto i = 0u; i < size; ++i)
		{
			hook::set<hook::instr>(address + i, instr::nop);
		}
	}

	void hook::write(std::uint32_t address, const std::initializer_list<std::uint8_t>& bytes)
	{
		std::memcpy(reinterpret_cast<std::uint8_t*>(address), bytes.begin(), bytes.size());
	}

	void hook::write_string(std::uint32_t address, const std::string& string)
	{
		std::strcpy(reinterpret_cast<char*>(address), &string[0]);
	}

	void hook::retn(std::uint32_t address)
	{
		Utils::hook::set<hook::instr>(address, instr::retn);
	}
}
