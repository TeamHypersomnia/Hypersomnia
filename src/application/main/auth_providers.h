#pragma once
#include "application/main/auth_provider_type.h"
#include "augs/misc/mutex.h"

augs::mutex pending_auth_datas_lk;
std::optional<web_auth_data> new_auth_data;

bool request_fresh_auth_token() {
	return main_thread_queue::execute([&]() {
		int result = EM_ASM_INT({
			return Module.request_fresh_auth_token() ? 1 : 0;
		});

		return result != 0;
	});
}

void sign_in_with_crazygames() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.loginCrazyGames();
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

inline void web_auth_data::log_out() {
	if (!is_set()) {
		return;
	}

	LOG("Logging out at timestamp: %x", augs::secs_since_epoch());

	switch(type) {
		case auth_provider_type::DISCORD:
			main_thread_queue::execute([&]() {
				call_revokeDiscord(auth_token.c_str());
			});

			break;
	
		default:
			break;
	}

	*this = {};
}

bool can_self_refresh(const auth_provider_type type) {
	return type == auth_provider_type::CRAZYGAMES;
}

bool web_auth_data::can_self_refresh() const {
	return ::can_self_refresh(type);
}

bool web_auth_data::self_refresh() {
	if (!can_self_refresh()) {
		return false;
	}

	return request_fresh_auth_token();
}

auto has_new_auth_data() {
	auto lock = augs::scoped_lock(pending_auth_datas_lk);
	return new_auth_data.has_value();
}

auto get_new_auth_data() {
	std::optional<web_auth_data> new_auth;

	{
		auto lock = augs::scoped_lock(pending_auth_datas_lk);
		new_auth = new_auth_data;
		new_auth_data.reset();
	}

	return new_auth;
}

extern "C" {
	EMSCRIPTEN_KEEPALIVE
	void on_auth_data_received(
		const char* provider,
		const char* profile_id,
		const char* profile_name,
		const uint8_t* avatar_byte_array,
		int avatar_byte_array_length,
		const char* auth_token,
		int expires_in
	) {
		LOG("on_auth_data_received");

#if !IS_PRODUCTION_BUILD
		LOG_NVPS(provider, profile_id, profile_name, avatar_byte_array_length, auth_token, expires_in);
#endif

		const auto now = augs::secs_since_epoch();

		const double expire_timestamp = 
			now
			/* Let's have a 30 seconds leeway since tokens usually have hour-long lifetimes */
			+ std::max(1, expires_in - 30)
		;

		LOG_NVPS(long(now), long(expire_timestamp));

		auto lock = augs::scoped_lock(pending_auth_datas_lk);

		const auto type = ::get_auth_provider_type(provider);

		const auto ptr = reinterpret_cast<const std::byte*>(avatar_byte_array);

		auto avatar_data = std::vector<std::byte>(ptr, ptr + avatar_byte_array_length);

		new_auth_data.emplace(web_auth_data {
			type,
			profile_name,
			typesafe_sprintf("%x%x", get_provider_preffix(type), profile_id),
			auth_token,
			expire_timestamp,

			std::move(avatar_data)
		});
	}
}
