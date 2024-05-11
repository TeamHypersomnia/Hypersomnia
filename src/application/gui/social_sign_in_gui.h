#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"

struct social_sign_in_input {
	const necessary_images_in_atlas_map& necessary_images;
};

struct social_sign_in_state : public standard_window_mixin<social_sign_in_state> {
	using base = standard_window_mixin<social_sign_in_state>;

	std::string guest_nickname;
	bool opened_once = false;

    social_sign_in_state(const std::string& title);

    bool perform(social_sign_in_input);
};
