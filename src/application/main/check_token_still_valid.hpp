#pragma once

bool web_auth_data::check_token_still_valid() const {
	if (const bool trivial_case = !is_signed_in() || expired()) {
		return false;
	}

	if (type == auth_provider_type::DISCORD) {
		auto http_client = httplib_utils::make_client("https://discord.com", 4);

		const auto result = http_client->Get("/api/v10/users/@me", nullptr, { { "Authorization", "Bearer " + auth_token } });

		if (result) {
			LOG_NVPS(result->status);

			if (httplib_utils::successful(result->status)) {
				LOG("Discord auth response: %x", result->body);

				try {
					auto doc = augs::json_document_from(result->body);

					if (doc.IsObject()) {
						if (doc.HasMember("id") && doc["id"].IsString()) {
							LOG("The Discord token is confirmed to be still valid.");
							return true;
						}
					}
				}
				catch (...) {
					LOG("Unknown error when verifying Discord auth.");
				}
			}
		}
		else {
			LOG("Result was null");
		}

		LOG("The Discord token is confirmed to be already invalid.");
		return false;
	}

	return true;
}
