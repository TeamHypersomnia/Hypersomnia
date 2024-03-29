#pragma once

enum class ingame_menu_button_type {
	// GEN INTROSPECTOR enum class ingame_menu_button_type
	RESUME,
	SERVER_DETAILS,
	BROWSE_SERVERS,
	SETTINGS,
	QUIT_TO_MENU,
#if !PLATFORM_WEB
	QUIT_GAME,
#endif

	COUNT
	// END GEN INTROSPECTOR
};