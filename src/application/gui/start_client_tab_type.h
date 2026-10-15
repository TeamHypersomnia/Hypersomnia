#pragma once

enum class start_client_tab_type {
	// GEN INTROSPECTOR enum class start_client_tab_type
	BEST_SERVER,
#if !PLATFORM_WEB
	CUSTOM_ADDRESS,
#endif
	REPLAY_DEMO,

	COUNT
	// END GEN INTROSPECTOR
};

