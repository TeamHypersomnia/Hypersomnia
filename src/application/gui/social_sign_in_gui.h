#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"
#include "application/main/auth_provider_type.h"

struct social_sign_in_input {
	const necessary_images_in_atlas_map& necessary_images;
	const bool prompted_once;
	const bool is_crazygames;
};

struct social_sign_in_state : public standard_window_mixin<social_sign_in_state> {
	using base = standard_window_mixin<social_sign_in_state>;

	std::string last_reason;
	web_auth_data cached_auth;
	std::string guest_nickname;
	std::string connect_string_post_sign_in;

	void open(std::string reason = "", std::string post_sign_in = "") {
		last_reason = reason;
		connect_string_post_sign_in = post_sign_in;
		base::open();
	}

    social_sign_in_state(const std::string& title);

    bool perform(social_sign_in_input);
};
