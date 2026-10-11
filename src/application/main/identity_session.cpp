#include <cstring>
#include "augs/log.h"
#include "augs/misc/async_get.h"
#include "augs/misc/secure_hash.h"
#include "augs/misc/date_time.h"
#include "augs/string/hex.h"
#include "augs/string/parse_url.h"
#include "application/main/identity_session.h"

/*
	What is left of a cert's lifetime before it is used no more: its clock started when it arrived,
	so the broker's and the player's wall clocks never matter.
*/

static constexpr double cert_expiry_margin_secs = 120.0;
static constexpr double max_retry_delay_secs = 300.0;
static constexpr double refused_retry_delay_secs = 600.0;
static constexpr double steam_ticket_timeout_secs = 10.0;

/*
	During a ranked match the cert is not refreshed while it has this much left: a linked or unlinked key
	would otherwise change the account under the match, and a rejoin after a drop would not be let back.
*/

static constexpr double hold_in_match_until_left_secs = 15 * 60.0;

static double wall_now() {
	return augs::date_time::secs_since_epoch();
}

identity_session::identity_session() {
	if (const auto seed = augs::ed25519_generate_seed()) {
		session_seed = *seed;

		if (const auto public_key = augs::ed25519_public_key(session_seed)) {
			session_public = *public_key;
		}
	}

	std::memcpy(session_secret.data(), session_seed.data(), session_seed.size());
	std::memcpy(session_secret.data() + session_seed.size(), session_public.data(), session_public.size());
}

std::optional<std::string> identity_session::build_location(const input& in) const {
	auto location = std::string("/cert?session=") + augs::bytes_to_hex(session_public);

	switch (in.source) {
		case source_type::STEAM:
			if (!steam_ticket.has_value()) {
				return std::nullopt;
			}

			return location + "&steam=" + augs::bytes_to_hex(*steam_ticket);

		case source_type::CRAZYGAMES:
			if (in.crazygames_token.empty()) {
				return std::nullopt;
			}

			return location + "&crazygames=" + in.crazygames_token;

		case source_type::KEY: {
			if (!in.account_seed.has_value()) {
				return std::nullopt;
			}

			const auto account_public = augs::ed25519_public_key(*in.account_seed);
			const auto signature = augs::ed25519_sign(*in.account_seed, identity_cert::cert_request_message(session_public));

			if (!account_public.has_value() || !signature.has_value()) {
				return std::nullopt;
			}

			return location + "&key=" + augs::bytes_to_hex(*account_public) + "&sig=" + augs::bytes_to_hex(*signature);
		}

		default:
			return std::nullopt;
	}
}

void identity_session::start_request(const input& in, const double now) {
	const auto location = build_location(in);

	if (!location.has_value()) {
		return;
	}

	const auto url = parsed_url(in.broker_url);

	if (!url.valid()) {
		last_error = "Invalid identity broker address: " + in.broker_url;
		schedule_retry(now, true);
		return;
	}

	LOG("Requesting an identity cert from %x.", in.broker_url);

	auto prefix = url.location;

	while (!prefix.empty() && prefix.back() == '/') {
		prefix.pop_back();
	}

	request = augs::async_get(url.get_base_url(), prefix + *location);
	request_started_at = now;
	requested_for = in;
}

void identity_session::schedule_retry(const double now, const bool refused) {
	if (refused) {
		next_attempt_at = now + refused_retry_delay_secs;
		return;
	}

	next_attempt_at = now + retry_delay;
	retry_delay = std::min(retry_delay * 2, max_retry_delay_secs);
}

void identity_session::advance(const input& in, const double now) {
	last_now = now;

	if (in != last_input) {
		/*
			Another account (e.g. an imported key, a CrazyGames sign in): the old cert is no longer it.
			A refreshed CrazyGames token is the same account - its cert stays.
		*/

		const bool had_input = last_input.source != source_type::NONE;
		const bool account_changed = !in.same_account(last_input);
		last_input = in;

		if (had_input && account_changed) {
			reset();
		}
	}

	/*
		Monotonic time stops while the computer sleeps: once a wake is seen (the wall clock ran on further),
		the next attempt is brought forward - once - and a cert that ran out meanwhile is dropped.
	*/

	{
		const auto wall = wall_now();

		if (last_wall > 0.0 && (wall - last_wall) - (now - last_steady) > 30.0) {
			LOG("The computer slept - checking the identity cert.");
			next_attempt_at = std::min(next_attempt_at, now);
		}

		last_wall = wall;
		last_steady = now;

		if (broker_cert.has_value() && cert_secs_left(now) <= 0.0) {
			broker_cert.reset();
		}
	}

	if (request != nullptr) {
		if (!request->is_ready()) {
			return;
		}

		const auto response = augs::get_once(request);

		/*
			Asked for an account that is no longer the player's (a key imported or forgotten, another CrazyGames user
			meanwhile): whatever came back is not theirs - asked again right away.
		*/

		if (!requested_for.same_account(last_input)) {
			LOG("Dropping an identity cert answer for the previous account.");
			next_attempt_at = 0.0;
			steam_ticket.reset();
			steam_ticket_requested = false;
			steam_ticket_wanted = false;
			return;
		}

		if (response.status == 200) {
			const auto bytes = augs::hex_to_bytes(response.body).value_or(std::vector<uint8_t>());
			const auto cert = identity_cert::from_bytes(reinterpret_cast<const std::byte*>(bytes.data()), bytes.size());

			if (cert.has_value() && cert->session_public_key == session_public && cert->not_after > cert->not_before) {
				broker_cert = cert;
				broker_cert_received_at = now;
				broker_cert_received_at_wall = wall_now();
				last_error.clear();
				retry_delay = 30.0;

				const auto lifetime = static_cast<double>(cert->not_after - cert->not_before);
				next_attempt_at = now + lifetime / 2;

				LOG("Received an identity cert for %x, valid for %x minutes.", cert->account, static_cast<int>(lifetime / 60));
			}
			else {
				last_error = "The identity broker sent an invalid cert.";
				LOG(last_error);
				schedule_retry(now, false);
			}
		}
		else if (response.status >= 400 && response.status < 500 && response.status != 429) {
			last_error = response.body.empty() ? typesafe_sprintf("The identity broker refused (HTTP %x).", response.status) : response.body;
			LOG("Identity cert refused (HTTP %x): %x", response.status, last_error);
			schedule_retry(now, true);
		}
		else {
			last_error = typesafe_sprintf("Could not reach hypersomnia.io (HTTP %x).", response.status);
			LOG("Identity cert request failed (HTTP %x). Retrying in %x seconds.", response.status, retry_delay);
			schedule_retry(now, false);
		}

		steam_ticket.reset();
		steam_ticket_requested = false;
		steam_ticket_wanted = false;

		if (retry_after_request) {
			retry_after_request = false;
			next_attempt_at = now;
		}

		return;
	}

	if (in.source == source_type::NONE || in.broker_url.empty()) {
		return;
	}

	/*
		Steam may never answer a ticket request.
	*/

	if (steam_ticket_requested && now - steam_ticket_requested_at > steam_ticket_timeout_secs) {
		on_steam_ticket({}, false);
	}

	if (now < next_attempt_at) {
		return;
	}

	if (in_ranked_match && cert_secs_left(now) > hold_in_match_until_left_secs) {
		return;
	}

	if (in.source == source_type::STEAM && !steam_ticket.has_value()) {
		steam_ticket_wanted = true;
		return;
	}

	start_request(in, now);
}

std::optional<std::string> identity_session::steam_identity_to_request() {
	if (!steam_ticket_wanted || steam_ticket_requested) {
		return std::nullopt;
	}

	steam_ticket_requested = true;
	steam_ticket_requested_at = last_now;
	return identity_cert::steam_ticket_identity(session_public);
}

void identity_session::on_steam_ticket(const std::vector<std::byte>& ticket_bytes, const bool successful) {
	steam_ticket_wanted = false;
	steam_ticket_requested = false;

	if (!successful || ticket_bytes.empty()) {
		last_error = "Steam did not give a ticket. Is Steam online?";
		LOG(last_error);
		schedule_retry(last_now, false);
		return;
	}

	steam_ticket = ticket_bytes;
}

bool identity_session::request_in_flight() const {
	return request != nullptr || steam_ticket_wanted;
}

void identity_session::retry_now() {
	/*
		With a request on its way (e.g. made just before a link), another one right after it - its answer may predate the change.
	*/

	if (request == nullptr) {
		next_attempt_at = 0.0;
	}
	else {
		retry_after_request = true;
	}
}

void identity_session::reset() {
	broker_cert.reset();
	broker_cert_received_at = 0.0;
	broker_cert_received_at_wall = 0.0;
	next_attempt_at = 0.0;
	retry_delay = 30.0;
	last_error.clear();
}

/*
	The lifetime left by both the monotonic clock (immune to clock changes) and the wall clock (which runs on while the computer sleeps),
	whichever says less.
*/

double identity_session::cert_secs_left(const double now) const {
	if (!broker_cert.has_value()) {
		return 0.0;
	}

	const auto lifetime = static_cast<double>(broker_cert->not_after - broker_cert->not_before);
	const auto by_steady = broker_cert_received_at + lifetime - now;
	const auto by_wall = broker_cert_received_at_wall + lifetime - wall_now();

	return std::min(by_steady, by_wall);
}

bool identity_session::has_broker_cert(const double now) const {
	return broker_cert.has_value() && cert_secs_left(now) > cert_expiry_margin_secs;
}

std::optional<identity_cert::cert> identity_session::cert_for_connect(const input& in, const double now) const {
	if (has_broker_cert(now)) {
		return broker_cert;
	}

	if (in.account_seed.has_value()) {
		return identity_cert::make_self_signed(*in.account_seed, session_public);
	}

	return std::nullopt;
}
