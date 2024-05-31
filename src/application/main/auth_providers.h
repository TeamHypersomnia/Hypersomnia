#pragma once
#include "application/main/auth_provider_type.h"

EM_JS(void, call_login_google, (), {
	LOG("Calling loginGoogle");

	Module.loginGoogle();
});

EM_JS(void, call_login_discord, (), {
	LOG("Calling loginDiscord");

	Module.loginDiscord();
});

std::mutex pending_auth_datas_lk;
std::optional<auth_data> new_auth_data;

void sign_in_with_google() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.loginGoogle();
		});
	});
}

void sign_in_with_discord() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.loginDiscord();
		});
	});
}

EM_JS(void, call_revokeDiscord, (const char* access_token), {
	Module.revokeDiscord(access_token);
});

inline void auth_data::log_out() {
	LOG("Logging out at timestamp: %x", augs::secs_since_epoch());

	switch(type) {
		case auth_provider::DISCORD:
			main_thread_queue::execute([&]() {
				call_revokeDiscord(auth_token.c_str());
			});

			break;
	
		default:
			break;
	}

	*this = {};
}

auto get_new_auth_data() {
	std::optional<auth_data> new_auth;

	{
		std::scoped_lock lk(pending_auth_datas_lk);
		new_auth = new_auth_data;
		new_auth_data.reset();
	}

	return new_auth;
}

extern "C" {
	EMSCRIPTEN_KEEPALIVE
	void on_auth_data_received(
		const char* provider,
		const char* profile_name,
		const char* avatar_url,
		const char* auth_token,
		int expires_in
	) {
		LOG("on_auth_data_received");

#if !IS_PRODUCTION_BUILD
		LOG_NVPS(provider, profile_name, avatar_url, auth_token, expires_in);
#endif

		const auto now = augs::secs_since_epoch();

		const double expire_timestamp = 
			now
			/* Let's have a 5 seconds leeway */
			+ std::max(1, expires_in - 5)
		;

		LOG_NVPS(long(now), long(expire_timestamp));

		std::scoped_lock lk(pending_auth_datas_lk);

		new_auth_data.emplace(auth_data {
			::get_auth_provider_type(provider),
			profile_name,
			avatar_url,
			auth_token,
			expire_timestamp
		});
	}
}
