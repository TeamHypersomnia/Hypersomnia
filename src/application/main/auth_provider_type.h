#pragma once

namespace augs {
	double secs_since_epoch();
}

enum class auth_provider_type {
	// GEN INTROSPECTOR enum class auth_provider_type
	GOOGLE,
	CRAZYGAMES,
	DISCORD,

	STEAM_NATIVE,

	COUNT
	// END GEN INTROSPECTOR
};

inline auto get_auth_provider_type(std::string p) {
	if (p == "google") {
		return auth_provider_type::GOOGLE;
	}

	if (p == "discord") {
		return auth_provider_type::DISCORD;
	}

	if (p == "crazygames") {
		return auth_provider_type::CRAZYGAMES;
	}

	return auth_provider_type::GOOGLE;
}

struct web_auth_data {
	// GEN INTROSPECTOR struct web_auth_data
	auth_provider_type type = auth_provider_type::COUNT;

	std::string profile_name;
	std::string avatar_url;
	std::string auth_token;
	double expire_timestamp = 0.0;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return type != auth_provider_type::COUNT;
	}

	bool expired() const {
		return is_set() && augs::secs_since_epoch() >= expire_timestamp;
	}

	bool is_signed_in() const {
		return is_set() && !expired();
	}

	void log_out();

	bool check_token_still_valid() const;
};
