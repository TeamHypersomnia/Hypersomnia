#pragma once

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
}

#undef DLL_EXPORT