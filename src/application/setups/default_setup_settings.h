#pragma once
#include "view/viewables/viewables_loading_type.h"

struct default_setup_settings {
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL_ONLY_ONCE;
	static constexpr bool handles_window_input = false;
	static constexpr bool has_additional_highlights = false;
};