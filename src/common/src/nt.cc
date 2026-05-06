#include <nt.hh>

namespace Utils
{
	WinNT WinNT::load(const std::string& name)
	{
		return WinNT(LoadLibraryA(name.data()));
	}

	WinNT WinNT::load(const std::filesystem::path& path)
	{
		return WinNT::load(path.generic_string());
	}

	WinNT WinNT::get_by_address(void* address)
	{
		HMODULE handle = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCSTR>(address), &handle);
		return WinNT(handle);
	}

	WinNT::WinNT()
	{
		this->module_ = GetModuleHandleA(nullptr);
	}

	WinNT::WinNT(const std::string& name)
	{
		this->module_ = GetModuleHandleA(name.data());
	}

	WinNT::WinNT(const HMODULE handle)
	{
		this->module_ = handle;
	}

	bool WinNT::operator==(const WinNT& obj) const
	{
		return this->module_ == obj.module_;
	}

	WinNT::operator bool() const
	{
		return this->is_valid();
	}

	WinNT::operator HMODULE() const
	{
		return this->get_handle();
	}

	PIMAGE_NT_HEADERS WinNT::get_nt_headers() const
	{
		if (!this->is_valid()) return nullptr;
		return reinterpret_cast<PIMAGE_NT_HEADERS>(this->get_ptr() + this->get_dos_header()->e_lfanew);
	}

	PIMAGE_DOS_HEADER WinNT::get_dos_header() const
	{
		return reinterpret_cast<PIMAGE_DOS_HEADER>(this->get_ptr());
	}

	PIMAGE_OPTIONAL_HEADER WinNT::get_optional_header() const
	{
		if (!this->is_valid()) return nullptr;
		return &this->get_nt_headers()->OptionalHeader;
	}

	std::vector<PIMAGE_SECTION_HEADER> WinNT::get_section_headers() const
	{
		std::vector<PIMAGE_SECTION_HEADER> headers;

		auto nt_headers = this->get_nt_headers();
		auto section = IMAGE_FIRST_SECTION(nt_headers);

		for (uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i, ++section)
		{
			if (section) headers.push_back(section);
			else OutputDebugStringA("There was an invalid section :O");
		}

		return headers;
	}

	std::uint8_t* WinNT::get_ptr() const
	{
		return reinterpret_cast<std::uint8_t*>(this->module_);
	}

	void WinNT::unprotect() const
	{
		if (!this->is_valid()) return;

		DWORD protection;
		VirtualProtect(this->get_ptr(), this->get_optional_header()->SizeOfImage, PAGE_EXECUTE_READWRITE, &protection);
	}

	size_t WinNT::get_relative_entry_point() const
	{
		if (!this->is_valid()) return 0;
		return this->get_nt_headers()->OptionalHeader.AddressOfEntryPoint;
	}

	void* WinNT::get_entry_point() const
	{
		if (!this->is_valid()) return nullptr;
		return this->get_ptr() + this->get_relative_entry_point();
	}

	bool WinNT::is_valid() const
	{
		return this->module_ != nullptr && this->get_dos_header()->e_magic == IMAGE_DOS_SIGNATURE;
	}

	std::string WinNT::get_name() const
	{
		if (!this->is_valid()) return "";

		auto path = this->get_path();
		const auto pos = path.find_last_of("/\\");
		if (pos == std::string::npos) return path;

		return path.substr(pos + 1);
	}

	std::string WinNT::get_path() const
	{
		if (!this->is_valid()) return "";

		char name[MAX_PATH] = { 0 };
		GetModuleFileNameA(this->module_, name, sizeof name);

		return name;
	}

	std::string WinNT::get_folder() const
	{
		if (!this->is_valid()) return "";

		const auto path = std::filesystem::path(this->get_path());
		return path.parent_path().generic_string();
	}

	void WinNT::free()
	{
		if (this->is_valid())
		{
			FreeLibrary(this->module_);
			this->module_ = nullptr;
		}
	}

	void* WinNT::InternalGetProcAddress(HMODULE module, const char* proc_name)
	{
		// check input
		if (!module || !proc_name)
			return NULL;

		// check module's header
		PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return NULL;

		// check NT header
		PIMAGE_NT_HEADERS pe_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(module) + dos_header->e_lfanew);
		if (pe_header->Signature != IMAGE_NT_SIGNATURE)
			return NULL;

		// get the export directory
		uint32_t export_adress = pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		if (!export_adress)
			return NULL;

		uint32_t export_size = pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
		uint32_t address;
		uint32_t ordinal_index = -1;
		PIMAGE_EXPORT_DIRECTORY export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<uint8_t*>(module) + export_adress);

		if (proc_name <= reinterpret_cast<const char*>(0xFFFF)) {
			// ordinal
			ordinal_index = static_cast<uint32_t>(INT_PTR(proc_name)) - export_directory->Base; //-V221
			// index is either less than base or bigger than number of functions
			if (ordinal_index >= export_directory->NumberOfFunctions)
				return NULL;
			// get the function offset by the ordinal
			address = (reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(module) + export_directory->AddressOfFunctions))[ordinal_index];
			// check for empty offset
			if (!address)
				return NULL;
		}
		else {
			// name of function
			if (export_directory->NumberOfNames) {
				// start binary search
				int left_index = 0;
				int right_index = export_directory->NumberOfNames - 1;
				uint32_t* names = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(module) + export_directory->AddressOfNames);
				while (left_index <= right_index) {
					uint32_t cur_index = (left_index + right_index) >> 1;
					switch (strcmp((const char*)(reinterpret_cast<uint8_t*>(module) + names[cur_index]), proc_name)) {
					case 0:
						ordinal_index = (reinterpret_cast<WORD*>(reinterpret_cast<uint8_t*>(module) + export_directory->AddressOfNameOrdinals))[cur_index];
						left_index = right_index + 1;
						break;
					case 1:
						right_index = cur_index - 1;
						break;
					case -1:
						left_index = cur_index + 1;
						break;
					}
				}
			}
			// if nothing has been found
			if (ordinal_index >= export_directory->NumberOfFunctions)
				return NULL;
			// get the function offset by the ordinal
			address = (reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(module) + export_directory->AddressOfFunctions))[ordinal_index];
			if (!address)
				return NULL;
		}

		// if it is just a pointer - return it
		if (address < export_adress || address >= export_adress + export_size)
			return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(module) + address);

		// it is a forward
		const char* name = reinterpret_cast<const char*>(reinterpret_cast<uint8_t*>(module) + address); // get a pointer to the module's name
		const char* tmp = name;
		const char* name_dot = NULL;
		// get a pointer to the function's name
		while (*tmp) {
			if (*tmp == '.') {
				name_dot = tmp;
				break;
			}
			tmp++;
		}
		if (!name_dot)
			return NULL;

		size_t name_len = name_dot - name;
		if (name_len >= MAX_PATH)
			return NULL;

		// copy module name
		char file_name[MAX_PATH];
		size_t i;
		for (i = 0; i < name_len && name[i] != 0; i++) {
			file_name[i] = name[i];
		}
		file_name[i] = 0;

		module = GetModuleHandleA(file_name);
		if (!module)
			return NULL;

		// now the function's name
		// if it is not an ordinal, just forward it
		if (name_dot[1] != '#')
			return InternalGetProcAddress(module, name_dot + 1);

		// is is an ordinal
		int ordinal = atoi(name_dot + 2);
		return InternalGetProcAddress(module, LPCSTR(INT_PTR(ordinal)));
	}

	HMODULE WinNT::get_handle() const
	{
		return this->module_;
	}

	void** WinNT::get_iat_entry(const std::string& module_name, const std::string& proc_name) const
	{
		if (!this->is_valid()) return nullptr;

		const WinNT other_module(module_name);
		if (!other_module.is_valid()) return nullptr;

		const auto target_function = other_module.get_proc<void*>(proc_name);
		if (!target_function) return nullptr;

		auto* header = this->get_optional_header();
		if (!header) return nullptr;

		auto* import_descriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(this->get_ptr() + header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		while (import_descriptor->Name)
		{
			if (!_stricmp(reinterpret_cast<char*>(this->get_ptr() + import_descriptor->Name), module_name.data()))
			{
				auto* original_thunk_data = reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->OriginalFirstThunk + this->get_ptr());
				auto* thunk_data = reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->FirstThunk + this->get_ptr());

				while (original_thunk_data->u1.AddressOfData)
				{
					const size_t ordinal_number = original_thunk_data->u1.AddressOfData & 0xFFFFFFF;

					if (ordinal_number > 0xFFFF) continue;

					if (GetProcAddress(other_module.module_, reinterpret_cast<char*>(ordinal_number)) ==
						target_function)
					{
						return reinterpret_cast<void**>(&thunk_data->u1.Function);
					}

					++original_thunk_data;
					++thunk_data;
				}

				//break;
			}

			++import_descriptor;
		}

		return nullptr;
	}
}

void raise_hard_exception()
{
	int data = false;
	const Utils::WinNT ntdll("ntdll.dll");
	ntdll.invoke_pascal<void>("RtlAdjustPrivilege", 19, true, false, &data);
	ntdll.invoke_pascal<void>("NtRaiseHardError", 0xC000007B, 0, nullptr, nullptr, 6, &data);
}

std::string load_resource(const int id)
{
	auto* const res = FindResource(Utils::WinNT(), MAKEINTRESOURCE(id), RT_RCDATA);
	if (!res) return {};

	auto* const handle = LoadResource(nullptr, res);
	if (!handle) return {};

	return std::string(LPSTR(LockResource(handle)), SizeofResource(nullptr, res));
}

void relaunch_self()
{
	const Utils::WinNT self;

	STARTUPINFOA startup_info;
	PROCESS_INFORMATION process_info;

	ZeroMemory(&startup_info, sizeof(startup_info));
	ZeroMemory(&process_info, sizeof(process_info));
	startup_info.cb = sizeof(startup_info);

	char current_dir[MAX_PATH];
	GetCurrentDirectoryA(sizeof(current_dir), current_dir);
	auto* const command_line = GetCommandLineA();

	CreateProcessA(self.get_path().data(), command_line, nullptr, nullptr, false, NULL, nullptr, current_dir, &startup_info, &process_info);

	if (process_info.hThread && process_info.hThread != INVALID_HANDLE_VALUE) CloseHandle(process_info.hThread);
	if (process_info.hProcess && process_info.hProcess != INVALID_HANDLE_VALUE) CloseHandle(process_info.hProcess);
}

void terminate(const uint32_t code)
{
	TerminateProcess(GetCurrentProcess(), code);
}