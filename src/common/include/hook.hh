#pragma once

#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include <cstring>
#include <iostream>

namespace Utils
{
	template <typename t> class function final
	{
	public:
		function() : function_(nullptr)
		{

		}

		function(t* ptr) : function_(ptr)
		{

		}

		function(std::uint32_t address) : function_(reinterpret_cast<t*>(address))
		{

		}

		function(const Utils::function<t>& other) : function_(other.function_)
		{

		}

		auto operator=(std::uint32_t address) -> Utils::function<t>&
		{
			this->function_ = reinterpret_cast<t*>(address);

			return *this;
		}

		auto operator=(const Utils::function<t>& function) -> Utils::function<t>&
		{
			this->function_ = function.function_;

			return *this;
		}

		template <typename... v> auto operator()(v&& ...args) const -> decltype(auto)
		{
			return this->function_(args...);
		}

	private:
		t* function_;
	};

    namespace hook
	{
		enum class instr : std::uint8_t
		{
			nop = 0x90,
			mov = 0xB8,
			jmp = 0xE9,
			retn = 0xC3,
			call = 0xE8,
		};

		void nop(std::uint32_t address, std::uint32_t size);

		void write(std::uint32_t address, const std::initializer_list<std::uint8_t>& bytes);
		void write_string(std::uint32_t address, const std::string& string);
		void retn(std::uint32_t address);

		template <typename T> auto get(std::uint32_t address)
		{
			return *reinterpret_cast<T*>(address);
		}

		template <typename T> void set(std::uint32_t address, T value)
		{
			*reinterpret_cast<T*>(address) = value;
		}

		template <typename T> void jump(std::uint32_t address, T function)
		{
			hook::set<hook::instr>(address, instr::jmp);
			hook::set<std::uint32_t>(address + 1, std::uint32_t(function) - address - 5);
		}

		template <typename T> void call(std::uint32_t address, T function)
		{
			hook::set<hook::instr>(address, instr::call);
			hook::set<std::uint32_t>(address + 1, std::uint32_t(function) - address - 5);
		}

		template <typename T> void return_value(std::uint32_t address, T value)
		{
			hook::set<hook::instr>(address, instr::mov);
			hook::set<std::uint32_t>(address + 1, std::uint32_t(value));
			hook::set<hook::instr>(address + 5, instr::retn);
		}

		template <typename T> auto detour(std::uint32_t source, const T& destination, std::uint32_t size) ->Utils::function<T>
		{
			const auto address = ::VirtualAlloc(nullptr, size + 5, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			std::memcpy(address, reinterpret_cast<std::uint8_t*>(source), size + 5);

			hook::jump(source, destination);
			hook::jump(reinterpret_cast<std::uint32_t>(address) + size, source + size);

			return Utils::function<T>(reinterpret_cast<T*>(address));
		}

		std::uint8_t* pattern(void* module, const char* signature);

		// Swap one entry in a COM vtable. On success `*out_old` (if non-null)
		// receives the previous slot value so the caller can chain through to the
		// original implementation. Returns false if VirtualProtect failed, in
		// which case the vtable is unchanged.
		inline bool VtablePatch(void** vtable, int index, void* new_fn, void** out_old)
		{
			DWORD old_protect = 0;
			if (!VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &old_protect))
				return false;
			if (out_old) *out_old = vtable[index];
			vtable[index] = new_fn;
			DWORD tmp = 0;
			VirtualProtect(&vtable[index], sizeof(void*), old_protect, &tmp);
			return true;
		}
	}

	template <typename t> class vtable final
	{
	public:
		vtable() : table_(nullptr)
		{

		}

		vtable(std::uint32_t address) : table_(reinterpret_cast<t*>(address))
		{

		}

		vtable(const Utils::vtable<t>& other) : table_(other.table_)
		{

		}

		auto get() -> t*
		{
			return reinterpret_cast<t*>(*reinterpret_cast<std::uint32_t**>(this->table_));
		}

		auto get(std::uint32_t index) -> std::uint32_t
		{
			return (**reinterpret_cast<std::uint32_t***>(this->table_))[index];
		}

		template <typename f> auto call(std::uint32_t index) -> Utils::function<f>
		{
			return Utils::function<f>((**reinterpret_cast<std::uint32_t***>(this->table_))[index]);
		}

		template <typename f> auto hook(std::uint32_t index, f function) -> std::uint32_t
		{
			const auto result = **reinterpret_cast<std::uint32_t***>(this->table_);
			const auto original = result[index];

			hook::set<std::uint32_t>(std::uint32_t(&result[index]), std::uint32_t(function));

			return original;
		}

		auto operator->() -> t*
		{
			return reinterpret_cast<t*>(*reinterpret_cast<std::uint32_t**>(this->table_));
		}

	private:
		t* table_;
	};
}
