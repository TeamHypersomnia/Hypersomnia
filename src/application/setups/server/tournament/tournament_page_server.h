#pragma once
#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/token_bucket.h"
#include "augs/misc/httplib_utils.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "augs/string/parse_url.h"
#include "application/main/server_key.h"
#include "application/network/account_id.h"
#include "all_paths.h"
#include "application/setups/editor/project/editor_project_paths.h"

/*
	What the tournament's live page serves and sends - /tournament.json made at most once a second whoever asks,
	each address asking at most 5 times a second (10 at once), and the results once it's over: POSTed to the webhook
	and saved as tournament_results.json - the very same bytes.

	Signed by the server's key, served and posted alike: X-Hypersomnia-Server-Key - server.key's public half
	(--print-server-key), X-Hypersomnia-Signature - Ed25519 of the body.
*/

class tournament_page_signer {
	std::optional<augs::ed25519_seed_type> seed = server_key::load_or_create(server_key::path());
	std::string key_hex = seed.has_value() ? server_key::public_key_hex(*seed) : std::string();

public:
	/*
		None if server.key can't be read - then nothing's signed.
	*/

	std::optional<httplib::Headers> headers_for(const std::string& json) const {
		if (!seed.has_value() || key_hex.empty()) {
			return std::nullopt;
		}

		if (const auto signature = augs::ed25519_sign(*seed, json)) {
			return httplib::Headers {
				{ "X-Hypersomnia-Server-Key", key_hex },
				{ "X-Hypersomnia-Signature", augs::bytes_to_hex(*signature) }
			};
		}

		return std::nullopt;
	}
};

/*
	The tournament's arenas' miniatures, read once at the start: an official arena's from the game's content,
	a downloaded one's from the user folder. Only these are served - a name from a request never makes a path.
*/

class tournament_arena_miniatures {
	std::unordered_map<std::string, std::string> png_of;

public:
	explicit tournament_arena_miniatures(const std::vector<std::string>& arenas) {
		for (const auto& arena : arenas) {
			/*
				"fy_iceworld bomb_defusal": the arena, then its game mode.
			*/

			const auto name = arena.substr(0, arena.find(' '));

			if (name.empty() || png_of.find(name) != png_of.end()) {
				continue;
			}

			for (const auto& dir : ::server_arena_folders(true)) {
				const auto path = dir / name / "miniature.png";

				if (!augs::exists(path)) {
					continue;
				}

				auto file = std::ifstream(path, std::ios::binary);
				auto bytes = std::ostringstream();
				bytes << file.rdbuf();

				if (file) {
					png_of[name] = bytes.str();
				}
				else {
					LOG("Tournament: could not read %x.", path.string());
				}

				break;
			}
		}
	}

	const std::string* find(const std::string& name) const {
		const auto found = png_of.find(name);
		return found != png_of.end() ? &found->second : nullptr;
	}
};

/*
	The players' avatars, kept for the page in their own folder (user/tournament_avatars) - so they're there after
	a restart too. A file per player, named by a hash of the account (normalized - 0x... is key_...): what a request
	asks for is only ever hashed, never a path.
*/

class tournament_avatars {
	augs::path_type dir;
	std::mutex lk;

	augs::path_type file_of(const std::string& account) const {
		return dir / (augs::bytes_to_hex(augs::secure_hash(::normalize_account_id(account))).substr(0, 32) + ".png");
	}

public:
	explicit tournament_avatars(augs::path_type dir) : dir(std::move(dir)) {}

	void save(const std::string& account, const std::vector<std::byte>& png) {
		std::scoped_lock guard(lk);

		try {
			augs::create_directories(dir);
			augs::save_as_text_atomic(file_of(account), std::string(reinterpret_cast<const char*>(png.data()), png.size()));
		}
		catch (const std::exception& err) {
			LOG("Tournament: could not save an avatar to %x: %x", dir.string(), err.what());
		}
	}

	std::optional<std::string> find(const std::string& account) {
		if (account.empty() || account.size() > 200) {
			return std::nullopt;
		}

		std::scoped_lock guard(lk);

		auto file = std::ifstream(file_of(account), std::ios::binary);

		if (!file) {
			return std::nullopt;
		}

		auto bytes = std::ostringstream();
		bytes << file.rdbuf();
		return bytes.str();
	}
};

/*
	The page's own pictures, next to it in detail/web, read once: the game's logo. Only these names are served.
*/

class tournament_page_pictures {
	std::unordered_map<std::string, std::string> png_of;

public:
	tournament_page_pictures() {
		for (const auto name : { "logo.png", "logo@2x.png" }) {
			auto file = std::ifstream(augs::path_type(DETAIL_DIR) / "web" / name, std::ios::binary);
			auto bytes = std::ostringstream();
			bytes << file.rdbuf();

			if (file) {
				png_of[name] = bytes.str();
			}
		}
	}

	const std::string* find(const std::string& name) const {
		const auto found = png_of.find(name);
		return found != png_of.end() ? &found->second : nullptr;
	}
};

struct signed_tournament_page {
	std::string json;
	httplib::Headers headers;
};

inline double tournament_page_now_secs() {
	return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

class tournament_page_cache {
	std::mutex lk;
	signed_tournament_page page;
	double built_at = -1.0;

public:
	/*
		make() and signing under this lock - who asks meanwhile waits for it, never makes it a second time.
	*/

	signed_tournament_page get(const std::function<std::string()>& make, const tournament_page_signer& signer) {
		std::scoped_lock guard(lk);

		const auto now = ::tournament_page_now_secs();

		if (built_at < 0.0 || now - built_at >= 1.0) {
			page.json = make();
			page.headers = signer.headers_for(page.json).value_or(httplib::Headers());
			built_at = now;
		}

		return page;
	}
};

class tournament_page_rate_limit {
	static constexpr double per_sec = 5.0;
	static constexpr double burst = 10.0;

	std::mutex lk;
	std::unordered_map<std::string, augs::token_bucket> by_address;
	double when_forgotten = 0.0;

public:
	bool allow(const std::string& address) {
		std::scoped_lock guard(lk);

		const auto now = ::tournament_page_now_secs();

		/*
			Who hasn't asked for a while has a full bucket again - the same as no bucket, so forgotten;
			looked through at most once a second, not on every request.
		*/

		if (by_address.size() > 4096 && now - when_forgotten >= 1.0) {
			when_forgotten = now;

			std::erase_if(by_address, [now](const auto& entry) {
				return now - entry.second.when_last > burst / per_sec;
			});
		}

		return by_address[address].take(now, per_sec, burst);
	}
};

/*
	The finished tournament's JSON next to the tournament's state (the user folder) - the last one finished;
	the archives keep the earlier ones' states.
*/

inline void save_tournament_results(const augs::path_type& path, const std::string& json) {
	try {
		augs::save_as_text_atomic(path, json);
		LOG("Tournament: the results saved to %x.", path.string());
	}
	catch (const std::exception& err) {
		LOG("Tournament: could not save the results to %x: %x", path.string(), err.what());
	}
}

/*
	The finished tournament's JSON to server_private.tournament_results_webhook_url - tried 3 times, 5 seconds apart,
	until it's answered with a 2xx. Never unsigned: the receiver trusts it by the signature.
*/

inline void post_tournament_results(
	const std::string& url,
	const std::string& json,
	const tournament_page_signer& signer,
	const std::function<bool()>& interrupted
) {
	if (url.empty()) {
		return;
	}

#if PLATFORM_WEB
	/*
		A server in the browser has no blocking HTTP client - its tournament's results stay on the live page and in
		tournament_results.json.
	*/

	(void)json;
	(void)signer;
	(void)interrupted;
	LOG("Tournament: a server in the browser sends no results webhook.");
#else
	const auto headers = signer.headers_for(json);

	if (!headers.has_value()) {
		LOG("Tournament: could not sign the results with %x - not sent.", server_key::path().string());
		return;
	}

	const auto parsed = parsed_url(url);

	if (!parsed.valid()) {
		LOG("Tournament: the results webhook is not a URL - not sent.");
		return;
	}

	const auto location = (parsed.location.empty() ? std::string("/") : parsed.location) + (parsed.query.empty() ? std::string() : "?" + parsed.query);

	for (auto attempt = 1; attempt <= 3; ++attempt) {
		auto client = httplib_utils::make_client(parsed, 10);
		const auto response = client->Post(location.c_str(), *headers, json, "application/json");

		if (response && response->status >= 200 && response->status < 300) {
			LOG("Tournament: the results were sent to the webhook (%x).", response->status);
			return;
		}

		if (response) {
			LOG("Tournament: the results webhook answered %x (attempt %x of 3).", response->status, attempt);
		}
		else {
			LOG("Tournament: the results webhook did not answer (attempt %x of 3).", attempt);
		}

		for (auto i = 0; i < 50 && attempt < 3; ++i) {
			if (interrupted && interrupted()) {
				return;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	LOG("Tournament: the results were not sent to the webhook.");
#endif
}
