#include <io.hh>

namespace Utils
{
	bool IO::WriteFile(const std::string& file, const std::string& data, const bool append)
	{
		const auto pos = file.find_last_of("/\\");
		if (pos != std::string::npos)
		{
			IO::CreateDirectoryPath(file.substr(0, pos));
		}

		// This is split because MinGW complains about it.
		// Original line:
		// std::ofstream stream(file, std::ios::binary | std::ofstream::out | (append ? std::ofstream::app : 0));
        std::ios_base::openmode mode = std::ios::binary | std::ios::out;
        if (append) mode |= std::ios::app;

		std::ofstream stream(file, mode);

		if (stream.is_open())
		{
			stream.write(data.data(), data.size());
			stream.close();
			return true;
		}

		return false;
	}

	bool IO::FileExists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	void IO::CreateDirectoryPath(const std::string& path)
	{
		std::filesystem::create_directories(path);
	}

	std::string IO::ReadFile(const std::string& file)
	{
		std::string data;
		ReadFile(file, &data);
		return data;
	}

	bool IO::ReadFile(const std::string& file, std::string* data)
	{
		if (!data) return false;
		data->clear();

		if (IO::FileExists(file))
		{
			std::ifstream stream(file, std::ios::in | std::ios::binary);
			if (!stream.is_open()) return false;

			stream.seekg(0, std::ios::end);
			const std::streamsize size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				data->resize(static_cast<uint32_t>(size));
				stream.read(const_cast<char*>(data->data()), size);
				stream.close();
				return true;
			}
		}

		return false;
	}

	void IO::DeleteFilename(const std::string& file)
	{
		std::filesystem::remove(file);
	}

	void IO::DeletePath(const std::string& path)
	{
		std::filesystem::remove_all(path);
	}

	void IO::RenameFile(const std::string& file, const std::string& new_name)
	{
		std::filesystem::rename(file, new_name);
	}

	size_t IO::GetFileSize(const std::string& file)
	{
		if (IO::FileExists(file))
		{
			std::ifstream stream(file, std::ios::binary);

			if (stream.good())
			{
				stream.seekg(0, std::ios::end);
				return static_cast<size_t>(stream.tellg());
			}
		}

		return 0;
	}
}
