#pragma once
#include <string>

#if WEB_CRAZYGAMES
namespace augs {
	bool has_profanity(const std::string&);
}
#else
namespace augs {
	inline bool has_profanity(const std::string&) {
		return false;
	}
}
#endif

