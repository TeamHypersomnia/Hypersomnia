#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "augs/log.h"
#include "augs/misc/httplib_utils.h"
#include "augs/string/parse_url.h"
#include "augs/readwrite/json_readwrite.h"
#include "application/network/secure_connect.h"

/*
	A secure connect token for one official server, fetched from the masterserver over HTTPS.
	See secure_connect.h.
*/

static_assert(secure_connect::connect_token_bytes_v == NETCODE_CONNECT_TOKEN_BYTES);

namespace connect_token_fetch {
	inline std::optional<uint8_t> hex_value(const char c) {
		if (c >= '0' && c <= '9') {
			return static_cast<uint8_t>(c - '0');
		}

		if (c >= 'a' && c <= 'f') {
			return static_cast<uint8_t>(c - 'a' + 10);
		}

		if (c >= 'A' && c <= 'F') {
			return static_cast<uint8_t>(c - 'A' + 10);
		}

		return std::nullopt;
	}

	template <std::size_t N>
	bool decode_hex(const std::string& hex, std::array<uint8_t, N>& out) {
		if (hex.size() != N * 2) {
			return false;
		}

		for (std::size_t i = 0; i < N; ++i) {
			const auto high = hex_value(hex[i * 2]);
			const auto low = hex_value(hex[i * 2 + 1]);

			if (!high.has_value() || !low.has_value()) {
				return false;
			}

			out[i] = static_cast<uint8_t>((*high << 4) | *low);
		}

		return true;
	}

	inline std::optional<fetched_connect_token> parse(const std::string& json) {
		try {
			const auto document = augs::json_document_from(json);

			if (!document.IsObject()
				|| !document.HasMember("client_id") || !document["client_id"].IsString()
				|| !document.HasMember("server_id") || !document["server_id"].IsString()
				|| !document.HasMember("token") || !document["token"].IsString()
			) {
				return std::nullopt;
			}

			auto out = fetched_connect_token();

			out.client_id = std::stoull(document["client_id"].GetString());
			out.server_id = document["server_id"].GetString();

			if (out.server_id.empty() || !decode_hex(document["token"].GetString(), out.token)) {
				return std::nullopt;
			}

			return out;
		}
		catch (...) {
			return std::nullopt;
		}
	}

	/*
		Blocking - call from a worker thread.
		target is the server's "ip:port", or its WebRTC id.
	*/

	inline std::optional<fetched_connect_token> fetch(const std::string& provider_url, const std::string& target) {
		const auto parsed = parsed_url(provider_url);

		if (!parsed.valid()) {
			LOG("Invalid connect_token_provider: %x", provider_url);
			return std::nullopt;
		}

#if !BUILD_OPENSSL
		/*
			Without OpenSSL, the HTTP client quietly turns https into http,
			and a connect token carries the session keys in the clear.
		*/

		if (begins_with(provider_url, "https")) {
			LOG("This build has no HTTPS support, not fetching a connect token from %x.", provider_url);
			return std::nullopt;
		}
#endif

		auto client = httplib_utils::make_client(parsed);

#if !PLATFORM_WEB
		/*
			A redirect could send the request on over plain http, and the token carries the session keys.
		*/

		client->set_follow_location(false);
#endif
		const auto location = parsed.location + "?server=" + target;

		LOG("Requesting a secure connect token for %x.", target);

		const auto result = client->Get(location.c_str());

		if (!result || !httplib_utils::successful(result->status)) {
			LOG("Couldn't get a secure connect token for %x. Status: %x", target, result ? result->status : -1);
			return std::nullopt;
		}

		auto token = parse(result->body);

		if (!token.has_value()) {
			LOG("Received an invalid connect token for %x.", target);
		}

		return token;
	}
}
