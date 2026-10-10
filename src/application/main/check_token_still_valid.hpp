#pragma once
#include "augs/misc/async_get.h"

/*
	The web build is single-threaded and a synchronous fetch on the main thread always fails,
	so the token is checked with an asynchronous request, polled every frame.

	Returns nullptr when there is nothing to check over the network.
*/

augs::async_response_ptr request_token_still_valid_check(const web_auth_data& auth) {
	if (auth.type == auth_provider_type::DISCORD) {
		LOG("Checking if the Discord token is still valid.");

		return augs::async_get(
			"https://discord.com",
			"/api/v10/users/@me",
			{ { "Authorization", "Bearer " + auth.auth_token } }
		);
	}

	return nullptr;
}

bool token_still_valid(const augs::http_response& response) {
	LOG_NVPS(response.status);

	if (httplib_utils::successful(response.status)) {
		try {
			const auto doc = augs::json_document_from(response.body);

			if (doc.IsObject() && doc.HasMember("id") && doc["id"].IsString()) {
				LOG("The Discord token is confirmed to be still valid.");
				return true;
			}
		}
		catch (...) {
			LOG("Unknown error when verifying Discord auth.");
		}
	}

	LOG("The Discord token is confirmed to be already invalid.");
	return false;
}
