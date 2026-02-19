#pragma once
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "augs/misc/httplib_utils.h"

inline std::string fetch_crazygames_public_key() {
    auto http_client = httplib_utils::make_client("sdk.crazygames.com", 4);
    auto res = http_client->Get("/publicKey.json");

    if (!res || res->status != 200) {
        LOG("Failed to fetch CrazyGames public key: %x", res ? res->status : 0);
        throw std::runtime_error("Failed to fetch CrazyGames public key");
    }

    rapidjson::Document doc;
    doc.Parse(res->body.c_str());

    if (doc.HasParseError()) {
        LOG("Failed to parse public key JSON: %x", rapidjson::GetParseError_En(doc.GetParseError()));
        throw std::runtime_error("Failed to parse public key JSON");
    }

    if (!doc.HasMember("publicKey") || !doc["publicKey"].IsString()) {
        LOG("Invalid public key JSON format");
        throw std::runtime_error("Invalid public key JSON format");
    }

    return doc["publicKey"].GetString();
}

inline std::string base64_url_decode(const std::string& in) {
    std::string modified_in = in;
    std::replace(modified_in.begin(), modified_in.end(), '-', '+');
    std::replace(modified_in.begin(), modified_in.end(), '_', '/');
    while (modified_in.size() % 4) {
        modified_in.push_back('=');
    }
    std::string out;
    std::vector<unsigned char> decoded_data;
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new_mem_buf(modified_in.data(), static_cast<int>(modified_in.size()));
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    decoded_data.resize(modified_in.size());
    int len = BIO_read(bio, decoded_data.data(), static_cast<int>(modified_in.size()));
    BIO_free_all(bio);
    out.assign(decoded_data.begin(), decoded_data.begin() + len);
    return out;
}

inline std::vector<std::string> split_jwt(const std::string& token) {
    std::vector<std::string> parts;
    std::istringstream iss(token);
    std::string part;
    while (std::getline(iss, part, '.')) {
        parts.push_back(part);
    }
    return parts;
}

bool verify_jwt_rs256(const std::string& token, const std::string& public_key) {
    auto parts = split_jwt(token);
    if (parts.size() != 3) {
        LOG("Invalid JWT token");
        return false;
    }

    std::string header_and_payload = parts[0] + "." + parts[1];
    std::string signature = base64_url_decode(parts[2]);

	LOG("Token: %x", token);
#if 0
    LOG("Header and Payload: %x", header_and_payload.c_str());
	LOG("Signature: %x", parts[2]);
    LOG("Signature (Base64 decoded): %x", signature.c_str());
#endif

    BIO* keybio = BIO_new_mem_buf(public_key.data(), -1);
    EVP_PKEY* evp_key = PEM_read_bio_PUBKEY(keybio, nullptr, nullptr, nullptr);
    BIO_free(keybio);

    if (!evp_key) {
        LOG("Failed to create EVP_PKEY");
        return false;
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_PKEY_CTX* pctx = nullptr;
    EVP_DigestVerifyInit(ctx, &pctx, EVP_sha256(), nullptr, evp_key);

    bool result = false;
    if (EVP_DigestVerifyUpdate(ctx, header_and_payload.data(), header_and_payload.size()) == 1) {
        if (EVP_DigestVerifyFinal(ctx, reinterpret_cast<const unsigned char*>(signature.data()), signature.size()) == 1) {
            result = true;
        }
		else {
            LOG("Signature verification failed");
        }
    }
	else {
        LOG("Digest update failed");
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(evp_key);

    return result;
}

std::string verify_crazygames_token(const std::string& token) {
    try {
        std::string public_key = fetch_crazygames_public_key();

        if (verify_jwt_rs256(token, public_key)) {
            auto parts = split_jwt(token);
            const auto decoded_payload = base64_url_decode(parts[1]);

            LOG("Decoded payload: %x", decoded_payload);

            rapidjson::Document payload_json;
            payload_json.Parse(decoded_payload.c_str());

            if (payload_json.HasParseError()) {
                LOG("Failed to parse token payload: %x", rapidjson::GetParseError_En(payload_json.GetParseError()));
                return "";
            }

			// Check the token expiration time
			if (!payload_json.HasMember("exp") || !payload_json["exp"].IsInt64()) {
				LOG("Token has no valid expiration claim");
				return "";
			}

			int64_t exp = payload_json["exp"].GetInt64();
			int64_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			if (current_time >= exp) {
				LOG("Token has expired");
				return "";
			}

			if (payload_json.HasMember("userId") && payload_json["userId"].IsString()) {
				return payload_json["userId"].GetString();
			}
			else {
				LOG("userId not found in token payload");
				return "";
			}

            if (payload_json.HasMember("userId") && payload_json["userId"].IsString()) {
                return payload_json["userId"].GetString();
            }
			else {
                LOG("userId not found in token payload");
                return "";
            }
        }
		else {
            LOG("JWT verification failed");
            return "";
        }
    }
	catch (const std::exception& e) {
        LOG("Failed to verify CrazyGames auth token: %x", e.what());
        return "";
    }
}

template <class T=std::string, class F>
std::optional<T> GetIf(F& from, const std::string& label) {
	return augs::json_find<T>(from, label);
}

std::string verify_discord_token(const std::string& token) {
	auto http_client = httplib_utils::make_client("discord.com", 4);

	httplib::Headers headers = {
		{"Authorization", "Bearer " + token }
	};

	const auto result = http_client->Get("/api/v10/users/@me", headers);

	if (result) {
		if (httplib_utils::successful(result->status)) {
			LOG("Discord auth response: %x", result->body);

			try {
				auto doc = augs::json_document_from(result->body);

				if (doc.IsObject()) {
					if (auto id = GetIf(doc, "id")) {
						return *id;
					}
				}

				LOG("Failed to deserialize Discord auth response. Schema is out of date.");
				return std::string("");
			}
			catch (const augs::json_deserialization_error& err) {
				LOG("Failed to deserialize Discord auth response. Reason: %x", err.what());
				return std::string("");
			}
		}
		else {
			LOG("Discord auth failed, http code: %x", result->status);
			return std::string("");
		}
	}
	else {
		LOG("Discord auth failed: no HTTPS response!");
		return std::string("");
	}
}

std::string verify_steam_token(const std::string& api_key, const std::string& ticket_hex) {
	auto http_client = httplib_utils::make_client("api.steampowered.com", 4);

	const auto appid = std::to_string(::steam_get_appid());
	const auto identity = "hypersomnia_gameserver";

	httplib::Params params;
	params.emplace("key", api_key);
	params.emplace("appid", appid);
	params.emplace("ticket", ticket_hex);
	params.emplace("identity", identity);

	const auto result = http_client->Get("/ISteamUserAuth/AuthenticateUserTicket/v1", params, httplib::Headers(), httplib::DownloadProgress());

	if (result) {
		if (httplib_utils::successful(result->status)) {
			LOG("Steam auth response: %x", result->body);

			try {
				auto doc = augs::json_document_from(result->body);

				if (doc.HasMember("response") && doc["response"].HasMember("params")) {
					auto& params = doc["response"]["params"];

					if (GetIf<bool>(params, "publisherbanned") == true) {
						LOG("Banned by the publisher.");
						return std::string("publisherbanned");
					}

					if (GetIf(params, "result") == "OK") {
						if (auto steamid = GetIf(params, "steamid")) {
							LOG("Detected Steam ID: %x", *steamid);
							return *steamid;
						}
					}
				}

				LOG("Failed to deserialize Steam auth response. Schema is out of date.");
				return std::string("");
			}
			catch (const augs::json_deserialization_error& err ) {
				LOG("Failed to deserialize Steam auth response. Reason: %x", err.what());
				return std::string("");
			}
		}
		else {
			LOG("Steam auth failed, http code: %x", result->status);
			return std::string("");
		}
	}
	else {
		LOG("Steam auth failed: no HTTPS response!");
		return std::string("");
	}
}

