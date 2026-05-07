#pragma once

#include <string>
#include <fstream>

namespace Utils
{
	class IO final
	{
		public:
			static bool WriteFile(const std::string& file, const std::string& data, bool append = false);
			static bool ReadFile(const std::string& file, std::string* data);
			static void DeleteFilename(const std::string& file);
			static bool DeletePath(const std::string& path);
			static void RenameFile(const std::string& file, const std::string& new_name);
			static std::string ReadFile(const std::string& file);
			size_t GetFileSize(const std::string& file);
			static bool FileExists(const std::string& path);
			static bool CreateDirectoryPath(const std::string& path);
	};
}
