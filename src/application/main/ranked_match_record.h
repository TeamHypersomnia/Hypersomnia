#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/date_time.h"
#include "augs/string/hex.h"
#include "augs/misc/secure_hash.h"
#include "augs/readwrite/json_readwrite.h"

/*
	user/ranked_match.json - a ranked match the player is in, kept so that:
	- the account stays set in stone: the server takes back a dropped player by account, so a rejoin must be made
	  on the account the match was joined with, even if a Steam cert came in the meantime;
	- the game brings the player back to it on its own after a crash or a restart.

	rejoin_until is rewritten during the match as now + what is left of the player's rejoin budget,
	so after a crash at any moment it is right within the rewrite period. Past it a rejoin is refused anyway -
	a stale file does nothing.
*/

struct ranked_match_record {
	std::string connect;
	augs::ed25519_public_key_type server_key = {};
	std::string account;
	int64_t rejoin_until = 0;

	bool valid_now() const {
		return static_cast<int64_t>(augs::date_time::secs_since_epoch()) < rejoin_until;
	}

	static std::optional<ranked_match_record> load(const augs::path_type& path) {
		if (!augs::exists(path)) {
			return std::nullopt;
		}

		try {
			const auto document = augs::json_document_from(augs::file_to_string(path));

			if (!document.IsObject()) {
				return std::nullopt;
			}

			auto string_of = [&](const char* const key) -> std::string {
				if (document.HasMember(key) && document[key].IsString()) {
					return document[key].GetString();
				}

				return "";
			};

			auto out = ranked_match_record();
			out.connect = string_of("connect");
			out.account = string_of("account");

			if (document.HasMember("rejoin_until") && document["rejoin_until"].IsInt64()) {
				out.rejoin_until = document["rejoin_until"].GetInt64();
			}

			const auto key = augs::hex_to_array<32>(string_of("server_key"));

			if (out.connect.empty() || out.account.empty() || !key.has_value()) {
				return std::nullopt;
			}

			out.server_key = *key;
			return out;
		}
		catch (...) {
			LOG("Could not read %x.", path);
			return std::nullopt;
		}
	}

	void save(const augs::path_type& path) const {
		const auto key = augs::bytes_to_hex(server_key);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		writer.StartObject();
		writer.Key("connect");
		writer.String(connect.c_str(), static_cast<rapidjson::SizeType>(connect.size()));
		writer.Key("server_key");
		writer.String(key.c_str(), static_cast<rapidjson::SizeType>(key.size()));
		writer.Key("account");
		writer.String(account.c_str(), static_cast<rapidjson::SizeType>(account.size()));
		writer.Key("rejoin_until");
		writer.Int64(rejoin_until);
		writer.EndObject();

		try {
			augs::save_as_text_atomic(path, std::string(buffer.GetString()) + "\n");
		}
		catch (...) {
			LOG("Could not save %x.", path);
		}
	}
};
