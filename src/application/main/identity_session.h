#pragma once
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/async_response.h"
#include "application/network/identity_cert.h"

/*
	The player's identity - see identity_cert.h.

	Every game has its "this computer" key (account.key in the user folder, the browser's key on the web).
	It is also the session key: the game signs its connections with it, and a cert names it.
	With nothing more, the game signs its own cert - the account "key_" and the key in hex - and never asks anyone.

	An account the identity broker on hypersomnia.io vouches for (Steam, CrazyGames) is expected:
	- in the Steam build, always - proven with a Steam ticket;
	- with a key linked to a Steam account, while steam.cert is there - proven with the key's signature;
	- in the CrazyGames build, once the player signs in to CrazyGames - proven with its token.

	One loop keeps that cert: it lives 4 hours and is renewed at half of its lifetime, also during a match,
	so a connection never waits for the broker; a failure is retried after 30 s, doubling up to 5 minutes.
	A cert only matters when connecting - a new one counts from the next connection.

	steam.cert keeps the Steam cert across restarts (a valid one is used at once), and its presence tells a key
	that it is linked. It is no secret: useless without account.key, which lies next to it anyway.
	It goes only when the broker answers with the key's own account - the key was unlinked on the site.
*/

class identity_session {
public:
	enum class provider_type {
		NONE,
		STEAM,
		CRAZYGAMES
	};

	struct input {
		std::string broker_url;
		provider_type provider = provider_type::NONE;

		/*
			The Steam build proves its Steam account with a ticket, the other builds with the key's signature.
		*/

		bool steam_ticket_proof = false;
		std::string crazygames_token;

		/*
			Whose token it is: a refreshed token of the same user keeps the cert, another user's does not.
		*/

		std::string crazygames_user;

		bool operator==(const input&) const = default;

		bool same_account(input other) const {
			auto self = *this;
			self.crazygames_token.clear();
			other.crazygames_token.clear();
			return self == other;
		}
	};

	/*
		The account's name and avatar as the broker sent them (Steam), shown instead of the player's own
		when "Use my Steam name and avatar" is on - see the plan's "two profiles".
	*/

	struct account_profile {
		std::string name;
		std::vector<std::byte> avatar;
	};

	/*
		The answer to "is this key linked?" - asked right after importing a key and by "Link to your Steam account".
	*/

	enum class link_check_state {
		NONE,
		PENDING,
		LINKED,
		NOT_LINKED,
		FAILED
	};

private:
	augs::ed25519_seed_type key_seed = {};
	augs::ed25519_public_key_type key_public = {};
	std::array<uint8_t, 64> key_secret = {};

	augs::path_type cert_file;
	augs::path_type profile_file;
	bool cert_saved = false;

	std::optional<identity_cert::cert> broker_cert;
	double cert_received_at = 0.0;
	double cert_received_at_wall = 0.0;
	account_profile profile;

	augs::async_response_ptr request;
	input requested_for;

	double next_attempt_at = 0.0;
	double retry_delay = 30.0;
	double last_now = 0.0;
	int attempts_finished = 0;
	bool last_attempt_failed = false;

	/*
		Steam: the ticket is requested once per attempt, through the game's Steam callbacks.
	*/

	bool steam_ticket_wanted = false;
	bool steam_ticket_requested = false;
	double steam_ticket_requested_at = 0.0;
	std::optional<std::vector<std::byte>> steam_ticket;

	augs::async_response_ptr link_check_request;
	link_check_state link_check = link_check_state::NONE;
	account_profile link_check_profile;

	input last_input;

	void clear_steam_ticket();
	void forget_cert();
	void load_saved_cert();
	void save_cert();
	void delete_saved_cert();

	std::string key_location() const;
	std::optional<std::string> build_location(const input&) const;
	void start_request(const input&, double now);
	void schedule_retry(double now);
	double cert_secs_left(double now) const;

	struct parsed_answer {
		identity_cert::cert cert;
		account_profile profile;
	};

	std::optional<parsed_answer> parse_answer(const std::string& body) const;

public:
	std::string last_error;

	/*
		The key and where the Steam cert is kept (.keys in the user folder). Another key - an import -
		forgets the cert, which names the old key.
	*/

	void set_key(const augs::ed25519_seed_type& seed, const augs::path_type& keys_dir);

	void advance(const input&, double now);

	/*
		Steam: an identity to request a Web API ticket for, once, when one is needed.
	*/

	std::optional<std::string> steam_identity_to_request();
	void on_steam_ticket(const std::vector<std::byte>& ticket_bytes, bool successful);

	/*
		Tries at once and starts the backoff over - the player's [Retry].
	*/

	void retry_now();

	/*
		A key linked to Steam is expected to sign in while steam.cert is there.
	*/

	bool has_saved_cert() const {
		return cert_saved;
	}

	/*
		Unlinked from the game: back to "this computer" right away, steam.cert deleted.
	*/

	void on_unlinked();

	bool has_valid_cert(double now) const;
	const std::optional<identity_cert::cert>& get_cert() const { return broker_cert; }
	const account_profile& get_profile() const { return profile; }

	/*
		The first sign-in of this run is still on its way - the start's "Signing in to your Steam account...".
	*/

	bool signing_in_first_time(const input&, double now) const;

	/*
		An account is expected, but there is no valid cert and the last attempt failed.
	*/

	bool sign_in_failed(const input&, double now) const;

	int get_attempts_finished() const {
		return attempts_finished;
	}

	/*
		What to show a server. forced_account is the account of a ranked match being rejoined (see ranked_match.json):
		a key_ account rejoins with a self-signed cert even when a Steam cert came meanwhile.
		Without a valid broker cert, always a self-signed one.
	*/

	identity_cert::cert cert_for_connect(double now, const std::string& forced_account = "") const;

	const std::array<uint8_t, 64>& session_secret_key() const { return key_secret; }
	const augs::ed25519_public_key_type& public_key() const { return key_public; }
	std::string key_account() const;

	void start_link_check(const std::string& broker_url);
	link_check_state get_link_check() const { return link_check; }
	const account_profile& get_link_check_profile() const { return link_check_profile; }
	void clear_link_check();
};
