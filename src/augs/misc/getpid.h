#pragma once

#if PLATFORM_UNIX
#include <sys/types.h>
#include <unistd.h>

namespace augs {
	inline auto getpid() {
		return ::getpid();
	}
}

#else

namespace augs {
	inline auto getpid() {
		return 0xdeadbeef;
	}
}

#endif
