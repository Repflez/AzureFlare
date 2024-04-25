#ifndef __AZ_LOGGING
#define __AZ_LOGGING

#include <stdio.h>

#if _DEBUG
#define DLOG(...) printf_s(__VA_ARGS__)
#else
#define DLOG(...)
#endif

#endif // !__AZ_LOGGING
