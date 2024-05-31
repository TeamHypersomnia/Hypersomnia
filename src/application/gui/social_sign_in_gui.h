#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"
#include "application/main/auth_provider_type.h"

struct social_sign_in_input {
	const necessary_images_in_atlas_map& necessary_images;
	const bool prompted_once;
};

struct social_sign_in_state : public standard_window_mixin<social_sign_in_state> {
	using base = standard_window_mixin<social_sign_in_state>;

	auth_data cached_auth;
	std::string guest_nickname;

    social_sign_in_state(const std::string& title);

    bool perform(social_sign_in_input);
};
