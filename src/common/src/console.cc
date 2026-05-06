#include <console.hh>

namespace Utils
{
	void Console::Init()
	{
		Console::file_ = std::fopen("console.log", "ab");

		::AllocConsole();
		::SetConsoleTitleA("Developer Console");

		std::freopen("CONOUT$", "w", stdout);
		std::freopen("CONIN$", "r", stdin);
	}

	::_iobuf* Console::File()
	{
		return Console::file_;
	}

	::_iobuf* Console::file_;
}