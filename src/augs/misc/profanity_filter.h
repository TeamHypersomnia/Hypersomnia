#pragma once
#include <string>

#if WEB_CRAZYGAMES
namespace augs {
	bool has_profanity(const std::string&, bool cache = false);
}
#else
namespace augs {
	inline bool has_profanity(const std::string&, bool = false) {
		return false;
	}
}
#endif

