#pragma once
#include <cstdlib>
#include <string>
#include "augs/filesystem/path_declaration.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/window_framework/platform_utils.h"
#include "application/main/self_updater.h"

#if FLATPAK_BUILD
constexpr bool is_flatpak_build_v = true;
#else
constexpr bool is_flatpak_build_v = false;
#endif

inline augs::path_type get_xdg_config_home() {
	if (const char* const config_home = std::getenv("XDG_CONFIG_HOME"); config_home != nullptr && config_home[0] != '\0') {
		return config_home;
	}

	if (const char* const home = std::getenv("HOME"); home != nullptr) {
		return augs::path_type(home) / ".config";
	}

	return augs::path_type();
}

inline augs::path_type get_default_appdata_dir() {
	if constexpr(is_flatpak_build_v) {
		return ::get_xdg_config_home() / "Hypersomnia";
	}
	else {
		return augs::get_default_documents_dir();
	}
}

inline std::string get_how_to_update_the_game() {
	if constexpr(is_flatpak_build_v) {
		return "Update the game through Flathub";
	}
	else {
		return "Update the game";
	}
}

inline std::string get_no_file_dialog_notice() {
	if constexpr(is_flatpak_build_v) {
		return "No file dialog on this system.";
	}
	else {
		return "No file dialog on this system (install zenity).";
	}
}

inline std::string get_update_available_status(const self_update_result& last_update_result) {
	if constexpr(is_flatpak_build_v) {
		return typesafe_sprintf("Version %x is out - Flathub gets it within a few hours.", last_update_result.available_version);
	}
	else {
		(void)last_update_result;
		return "Updates are available. Restart to install.";
	}
}
