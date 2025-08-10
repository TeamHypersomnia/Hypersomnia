#pragma once

enum class main_menu_button_type {
	// GEN INTROSPECTOR enum class main_menu_button_type
	STEAM,
	DISCORD,
	GITHUB,
	QUICK_PLAY,
	DOWNLOAD_MAPS,
	BROWSE_SERVERS,
	HOST_SERVER,
#if !WEB_LOWEND
	CONNECT_TO_SERVER,
#endif
	SHOOTING_RANGE,
	TUTORIAL,
#if !WEB_CRAZYGAMES
	EDITOR,
#endif
	SETTINGS,
#if !WEB_LOWEND
	CREDITS,
#endif
#if !PLATFORM_WEB
	QUIT,
#endif

	COUNT
	// END GEN INTROSPECTOR
};