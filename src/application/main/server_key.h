#pragma once
#include <cstdint>
#include <stdexcept>
#include <sodium.h>
#include <string_view>
#include "augs/log.h"
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "application/main/identity_key.h"

/*
	Every server's Ed25519 key: one secret, server.key in KEYS_DIR (see all_paths.h), created on the first start and never
	overwritten - and from it a key for each instance, by its kind and number (see instance_seed). The clients seal their
	handshakes to that key, and the masterserver and hypersomnia.io know an official server by it.
	The same format as the account keys - see identity_key.h.
*/

namespace server_key {
	inline constexpr std::string_view file_name = "server.key";

	/*
		Hosting from the game with a key file it can not read still starts - with a key for this run only, so it is not known
		as the server it was; the file is left as it is. A dedicated server does not start then (see damaged).
	*/

	inline augs::ed25519_seed_type obtain(const augs::path_type& path) {
		if (const auto seed = identity_key::load_or_create(path)) {
			return *seed;
		}

		LOG("ERROR: could not read or create the server key at %x. Using a key for this run only.", path);

		if (const auto seed = augs::ed25519_generate_seed()) {
			return *seed;
		}

		/*
			Never a made-up key (e.g. all zeros - the same for every such server, its private key known to all).
		*/

		throw std::runtime_error("Could not generate a server key.");
	}

	/*
		There, but not readable (damaged, no permission): a dedicated server does not start then - on a key for this run only
		it would not be the server it was (e.g. no longer official).
	*/

	inline bool damaged(const augs::path_type& path) {
		return augs::exists(path) && !identity_key::load(path).has_value();
	}

	/*
		One key per server, all from one server.key - as many wallets from one seed: a casual or a ranked (any with progress,
		tournaments too) instance number N has its own key, the same on any host and port. Moving instances elsewhere
		(or splitting them between hosts) keeps their keys: copy server.key and set their numbers (instance_number).
	*/

	inline augs::ed25519_seed_type instance_seed(const augs::ed25519_seed_type& server_seed, const bool with_progress, const uint16_t number) {
		const auto tag = std::string_view("hs-instance-v1");

		uint8_t message[32];
		std::size_t n = 0;

		for (const auto c : tag) {
			message[n++] = static_cast<uint8_t>(c);
		}

		message[n++] = with_progress ? 1 : 0;
		message[n++] = static_cast<uint8_t>(number & 0xff);
		message[n++] = static_cast<uint8_t>(number >> 8);

		auto out = augs::ed25519_seed_type();

		crypto_generichash(
			out.data(),
			out.size(),
			message,
			n,
			server_seed.data(),
			server_seed.size()
		);

		return out;
	}

	inline std::string kind_name(const bool with_progress) {
		return with_progress ? "ranked" : "casual";
	}

	inline std::string public_key_hex(const augs::ed25519_seed_type& seed) {
		if (const auto public_key = augs::ed25519_public_key(seed)) {
			return augs::bytes_to_hex(*public_key);
		}

		return "";
	}
}
