#pragma once
#include <string>
#include "augs/window_framework/window_settings.h"
#include "application/http_client/http_client_settings.h"
#include "augs/image/image.h"

enum class self_update_result_type {
	NONE,

	EXIT_APPLICATION,
	FAILED,
	COULDNT_DOWNLOAD_BINARY,
	COULDNT_DOWNLOAD_VERSION_FILE,
	COULDNT_SAVE_BINARY,
	FAILED_TO_OPEN_SSH_KEYGEN,
	FAILED_TO_VERIFY_BINARY,
	DOWNLOADED_BINARY_WAS_OLDER,
	CANCELLED,
	UPGRADED,
	UP_TO_DATE,

	FIRST_LAUNCH_AFTER_UPGRADE,

	UPDATE_AVAILABLE
};

struct self_update_result {
	using result_type = self_update_result_type;

	result_type type = result_type::NONE;
	bool exit_with_failure_if_not_upgraded = false;
};

self_update_result check_and_apply_updates(
	const augs::path_type& current_appimage_path,
	bool only_check_availability_and_quit,
	const augs::image& imgui_atlas_image,
	const http_client_settings& settings,
	augs::window_settings window_settings,
	bool headless
);
