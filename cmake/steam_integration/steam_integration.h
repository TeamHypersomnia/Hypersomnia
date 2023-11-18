#pragma once
#include <cstdint>

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

enum class steam_init_result {
	SUCCESS,
	FAILURE,
	DISABLED
};

extern "C" {
	DLL_EXPORT int steam_get_appid();

	DLL_EXPORT int steam_init();
	DLL_EXPORT bool steam_restart();
	DLL_EXPORT void steam_deinit();

	DLL_EXPORT const char* steam_get_username();
	DLL_EXPORT uint8_t* steam_get_avatar(uint32_t* width, uint32_t* height);

	DLL_EXPORT bool steam_set_rich_presence(const char*, const char*);
	DLL_EXPORT void steam_clear_rich_presence();
	DLL_EXPORT int steam_get_launch_command_line(char* buf, int bufsize);
}

#undef DLL_EXPORT