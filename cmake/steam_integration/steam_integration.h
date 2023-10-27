#pragma once

enum class steam_init_result {
	SUCCESS,
	FAILURE,
	DISABLED
};

extern "C" {
	int steam_init();
	bool steam_restart();
	void steam_deinit();
}
