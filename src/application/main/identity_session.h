#pragma once
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "augs/misc/ed25519.h"
#include "augs/misc/async_response.h"
#include "application/network/identity_cert.h"

/*
	The player's identity for this run of the game - see identity_cert.h.

	The session key is made at start and lives only in memory. Right away the game asks the identity broker
	on hypersomnia.io for a cert binding the player's account to it - with a Steam ticket, a CrazyGames token or
	the signature of the account key - and asks again at half of its lifetime. Joining a server never waits for the broker
	beyond a short moment for a request already on its way. A failed request is retried with a growing delay, one the broker
	refused (e.g. a ban) only after a long one; the last reason is kept for the player to see.

	With an account key the player can always sign a cert himself - good everywhere but in ranked, e.g. for admins
	while the broker is down.
*/

class identity_session {
public:
	enum class source_type {
		NONE,
		STEAM,
		CRAZYGAMES,
		KEY
	};

	struct input {
		std::string broker_url;
		source_type source = source_type::NONE;
		std::string crazygames_token;
		std::optional<augs::ed25519_seed_type> account_seed;

		bool operator==(const input&) const = default;
	};

private:
	augs::ed25519_seed_type session_seed = {};
	augs::ed25519_public_key_type session_public = {};
	std::array<uint8_t, 64> session_secret = {};

	std::optional<identity_cert::cert> broker_cert;
	double broker_cert_received_at = 0.0;
	double broker_cert_received_at_wall = 0.0;

	augs::async_response_ptr request;
	double request_started_at = 0.0;
	input requested_for;

	double next_attempt_at = 0.0;
	double retry_delay = 30.0;

	/*
		Steam: the ticket is requested once per attempt, through the game's Steam callbacks.
	*/

	bool steam_ticket_wanted = false;
	bool steam_ticket_requested = false;
	double steam_ticket_requested_at = 0.0;
	double last_now = 0.0;
	std::optional<std::vector<std::byte>> steam_ticket;

	input last_input;

	void start_request(const input&, double now);
	void schedule_retry(double now, bool refused);
	double cert_secs_left(double now) const;
	std::optional<std::string> build_location(const input&) const;

public:
	std::string last_error;

	/*
		Set by the game while the player is in a ranked match - see hold_in_match_until_left_secs.
	*/

	bool in_ranked_match = false;

	identity_session();

	void advance(const input&, double now);

	/*
		Steam: an identity to request a Web API ticket for, once, when one is needed.
	*/

	std::optional<std::string> steam_identity_to_request();
	void on_steam_ticket(const std::vector<std::byte>& ticket_bytes, bool successful);

	bool request_in_flight() const;

	/*
		Skips the wait before the next attempt - e.g. joining a ranked server without a cert,
		or right after linking or unlinking the key.
	*/

	void retry_now();

	/*
		Forgets the cert, e.g. after the account key was replaced - the next one is asked for right away.
	*/

	void reset();

	bool has_broker_cert(double now) const;
	const std::optional<identity_cert::cert>& get_broker_cert() const { return broker_cert; }

	/*
		What to show a server: the broker's cert while it is valid, else one signed with the account key if there is one.
	*/

	std::optional<identity_cert::cert> cert_for_connect(const input&, double now) const;

	const std::array<uint8_t, 64>& session_secret_key() const { return session_secret; }
	const augs::ed25519_public_key_type& session_public_key() const { return session_public; }
};
