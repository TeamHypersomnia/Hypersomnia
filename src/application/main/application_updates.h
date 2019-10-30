#pragma once
#include <string>
#include "augs/window_framework/window_settings.h"
#include "application/http_client/http_client_settings.h"
#include "augs/image/image.h"

enum class application_update_result_type {
	EXIT,
	ERROR,
	CANCELLED,
	SUCCESS
};

struct application_update_result {
	using result_type = application_update_result_type;

	result_type type = result_type::ERROR;
};

application_update_result check_and_apply_updates(
	const augs::image& imgui_atlas_image,
	const http_client_settings& settings,
	augs::window_settings window_settings
);
