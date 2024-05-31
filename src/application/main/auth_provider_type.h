#pragma once

namespace augs {
	double secs_since_epoch();
}

enum class auth_provider {
	// GEN INTROSPECTOR enum class auth_provider
	GOOGLE,
	CRAZYGAMES,
	DISCORD,

	COUNT
	// END GEN INTROSPECTOR
};

inline auto get_auth_provider_type(std::string p) {
	if (p == "google") {
		return auth_provider::GOOGLE;
	}

	if (p == "discord") {
		return auth_provider::DISCORD;
	}

	if (p == "crazygames") {
		return auth_provider::CRAZYGAMES;
	}

	return auth_provider::GOOGLE;
}

struct auth_data {
	// GEN INTROSPECTOR struct auth_data
	auth_provider type = auth_provider::COUNT;

	std::string profile_name;
	std::string avatar_url;
	std::string auth_token;
	double expire_timestamp = 0.0;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return type != auth_provider::COUNT;
	}

	bool expired() const {
		return is_set() && augs::secs_since_epoch() >= expire_timestamp;
	}

	bool is_signed_in() const {
		return is_set() && !expired();
	}

	void log_out();
};
