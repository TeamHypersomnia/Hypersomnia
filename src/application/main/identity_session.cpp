#include <cctype>
#include <cstring>
#include <filesystem>
#include "augs/log.h"
#include "augs/misc/async_get.h"
#include "augs/misc/date_time.h"
#include "augs/string/hex.h"
#include "augs/misc/secure_hash.h"
#include "augs/string/base64.h"
#include "augs/string/parse_url.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/json_readwrite.h"
#include "application/main/identity_session.h"
#include "application/network/client_platform_type.h"

/*
	A cert is not used for its last two minutes: a server's clock may be a little ahead.
*/

static constexpr double cert_expiry_margin_secs = 120.0;
static constexpr double first_retry_delay_secs = 30.0;
static constexpr double max_retry_delay_secs = 300.0;
static constexpr double steam_ticket_timeout_secs = 10.0;
static constexpr double key_recheck_delay_secs = 30.0;
static constexpr double link_page_lifetime_secs = 3 * 60.0;

static constexpr std::string_view cert_file_prefix = "hypersomnia-cert-v1:";

static double wall_now() {
	return augs::date_time::secs_since_epoch();
}

void identity_session::set_key(const augs::ed25519_seed_type& seed, const augs::path_type& keys_dir) {
	const auto new_cert_file = keys_dir / "steam.cert";

	if (seed == key_seed && new_cert_file == cert_file) {
		return;
	}

	key_seed = seed;
	key_loaded = true;

	if (const auto public_key = augs::ed25519_public_key(key_seed)) {
		key_public = *public_key;
	}

	std::memcpy(key_secret.data(), key_seed.data(), key_seed.size());
	std::memcpy(key_secret.data() + key_seed.size(), key_public.data(), key_public.size());

	cert_file = new_cert_file;
	profile_file = keys_dir / "steam_profile.json";
	import_marker_file = keys_dir / "imported_key.json";
	imported_profile.reset();
	unlinked_noticed.reset();

	import_profile_pending = [&]() {
		if (!augs::exists(import_marker_file)) {
			return false;
		}

		try {
			return augs::from_json_file<imported_key_marker>(import_marker_file).account == key_account();
		}
		catch (...) {
			return false;
		}
	}();

	forget_cert();

	/*
		Whatever was on its way was asked for the old key.
	*/

	request.reset();
	check_in_flight = false;
	link_check = link_check_state::NONE;
	clear_steam_ticket();

	attempts_finished = 0;
	last_attempt_failed = false;
	next_attempt_at = 0.0;
	retry_delay = first_retry_delay_secs;
	last_error.clear();

	load_saved_cert();

	key_check_wanted = true;
	key_check_quiet = true;
	key_check_next_at = 0.0;
	key_recheck_until = 0.0;
}

void identity_session::recheck_link_quietly() {
	key_check_wanted = true;
	key_check_quiet = true;
	key_check_next_at = last_now + key_recheck_delay_secs;
	key_recheck_until = last_now + link_page_lifetime_secs;
}

void identity_session::mark_imported() {
	import_profile_pending = true;
	imported_profile.reset();

	try {
		augs::save_as_json_atomic(imported_key_marker { key_account() }, import_marker_file);
	}
	catch (...) {
		LOG("Could not save %x - the restored key's nickname and avatar are asked for only in this run.", import_marker_file);
	}
}

void identity_session::imported_profile_applied() {
	import_profile_pending = false;
	imported_profile.reset();
	augs::remove_file(import_marker_file);
}

void identity_session::on_key_created() {
	key_check_wanted = false;
}

void identity_session::forget_cert() {
	broker_cert.reset();
	cert_received_at = 0.0;
	cert_received_at_wall = 0.0;
	profile = {};
}

/*
	steam.cert: "hypersomnia-cert-v1:<cert in hex> <unix time it arrived>" - the arrival time, so that a computer
	whose clock is off by hours still sees a fresh cert as fresh. A cert for another key (e.g. a replaced account.key)
	is not ours and is ignored.
*/

void identity_session::load_saved_cert() {
	cert_saved = false;

	if (!augs::exists(cert_file)) {
		return;
	}

	try {
		const auto text = augs::file_to_string(cert_file);

		if (text.rfind(cert_file_prefix, 0) != 0) {
			LOG("steam.cert is not a cert. Ignored.");
			return;
		}

		const auto rest = text.substr(cert_file_prefix.size());
		const auto space = rest.find(' ');

		if (space == std::string::npos) {
			return;
		}

		const auto bytes = augs::hex_to_bytes(rest.substr(0, space));
		const auto received_wall = std::strtod(rest.substr(space + 1).c_str(), nullptr);

		if (!bytes.has_value()) {
			return;
		}

		const auto cert = identity_cert::from_bytes(reinterpret_cast<const std::byte*>(bytes->data()), bytes->size());

		if (!cert.has_value() || cert->session_public_key != key_public || cert->not_after <= cert->not_before) {
			LOG("steam.cert is for another key. Ignored.");
			return;
		}

		broker_cert = cert;
		cert_received_at_wall = received_wall;
		cert_saved = true;

		/*
			The monotonic clock of this run, as if the cert had arrived in it.
		*/

		cert_received_at = augs::steady_secs() - (wall_now() - received_wall);

		/*
			Arrived "in the future": this computer's clock went back since. Its age is unknown - taken as expired,
			so it is renewed at once instead of being shown to servers that refuse it.
		*/

		if (received_wall > wall_now() + 60.0) {
			LOG("steam.cert arrived later than now by this computer's clock - renewing it.");
			cert_received_at = augs::steady_secs() - static_cast<double>(cert->not_after - cert->not_before) - 1.0;
		}

		LOG("Loaded the saved cert for %x.", cert->account);
	}
	catch (...) {
		LOG("Could not read steam.cert.");
	}

	try {
		const auto document = augs::json_document_from(augs::file_to_string(profile_file));

		if (document.IsObject()) {
			if (document.HasMember("name") && document["name"].IsString()) {
				profile.name = document["name"].GetString();
			}

			if (document.HasMember("avatar") && document["avatar"].IsString()) {
				profile.avatar = augs::from_base64(document["avatar"].GetString()).value_or(std::vector<std::byte>());
			}
		}
	}
	catch (...) {

	}
}

void identity_session::save_cert() {
	if (!broker_cert.has_value()) {
		return;
	}

	try {
		std::filesystem::create_directories(cert_file.parent_path());

		const auto hex = augs::bytes_to_hex(identity_cert::to_bytes(*broker_cert));

		augs::save_as_text_atomic(
			cert_file,
			std::string(cert_file_prefix) + hex + " " + std::to_string(static_cast<int64_t>(cert_received_at_wall)) + "\n"
		);

		cert_saved = true;

		const auto avatar = augs::to_base64(profile.avatar);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		writer.StartObject();
		writer.Key("name");
		writer.String(profile.name.c_str(), static_cast<rapidjson::SizeType>(profile.name.size()));
		writer.Key("avatar");
		writer.String(avatar.c_str(), static_cast<rapidjson::SizeType>(avatar.size()));
		writer.EndObject();

		augs::save_as_text_atomic(profile_file, std::string(buffer.GetString()) + "\n");
	}
	catch (...) {
		LOG("Could not save steam.cert.");
	}
}

void identity_session::delete_saved_cert() {
	augs::remove_file(cert_file);
	augs::remove_file(profile_file);
	cert_saved = false;
}

void identity_session::on_unlinked() {
	forget_cert();
	delete_saved_cert();
	unconfirmed_cert.reset();
	unconfirmed_profile = {};
	last_error.clear();
	last_attempt_failed = false;
}

std::string identity_session::key_account() const {
	return identity_cert::key_account(key_public);
}

std::string identity_session::device_query() const {
	const auto device = [&]() {
		auto encoded = std::string();

		/*
			At most 64 bytes, cut before a character, not inside one (UTF-8 continuation bytes are 10xxxxxx).
		*/

		const auto cut = [&]() {
			auto length = std::min<std::size_t>(device_name.size(), 64);

			while (length > 0 && length < device_name.size() && (static_cast<unsigned char>(device_name[length]) & 0xC0) == 0x80) {
				--length;
			}

			return device_name.substr(0, length);
		}();

		for (const auto c : cut) {
			const auto u = static_cast<unsigned char>(c);
			const bool plain = std::isalnum(u) || c == '-' || c == '_' || c == '.';

			if (plain) {
				encoded += c;
			}
			else {
				const auto digits = std::string_view("0123456789ABCDEF");
				encoded += '%';
				encoded += digits[u >> 4];
				encoded += digits[u & 15];
			}
		}

		return encoded.empty() ? std::string() : "&device=" + encoded;
	}();

	/*
		The system, for the icon next to the key on its owner's /profile.
	*/

	const auto os = [&]() -> std::string {
		switch (::get_client_platform_type()) {
			case client_platform_type::WINDOWS: return "&os=windows";
			case client_platform_type::LINUX: return "&os=linux";
			case client_platform_type::MACOS: return "&os=mac";
			case client_platform_type::WEB:
			case client_platform_type::ITCH:
			case client_platform_type::CRAZYGAMES: return "&os=web";
			default: return "";
		}
	}();

	return device + os;
}

std::string identity_session::key_location() const {
	const auto signature = augs::ed25519_sign(key_seed, identity_cert::cert_request_message(key_public));

	if (!signature.has_value()) {
		return "";
	}

	return "&key=" + augs::bytes_to_hex(key_public) + "&sig=" + augs::bytes_to_hex(*signature);
}

std::optional<std::string> identity_session::build_location(const input& in) const {
	const auto location = std::string("/cert?session=") + augs::bytes_to_hex(key_public);

	if (in.provider == provider_type::CRAZYGAMES) {
		if (in.crazygames_token.empty()) {
			return std::nullopt;
		}

		/*
			The token itself goes in a header (see start_request) - never in a URL, which logs keep.
		*/

		return location + "&crazygames=header";
	}

	if (in.provider == provider_type::STEAM) {
		if (in.steam_ticket_proof) {
			if (!steam_ticket.has_value()) {
				return std::nullopt;
			}

			return location + "&steam=" + augs::bytes_to_hex(*steam_ticket);
		}

		const auto key = key_location();

		if (key.empty()) {
			return std::nullopt;
		}

		return location + key;
	}

	return std::nullopt;
}

static std::optional<parsed_url> broker_url_of(const std::string& broker_url, std::string& error) {
	const auto url = parsed_url(broker_url);

	if (!url.valid()) {
		error = "Invalid identity broker address: " + broker_url;
		return std::nullopt;
	}

	return url;
}

static std::string location_prefix(const parsed_url& url) {
	auto prefix = url.location;

	while (!prefix.empty() && prefix.back() == '/') {
		prefix.pop_back();
	}

	return prefix;
}

void identity_session::start_request(const input& in, const double now, const std::string& key_query) {
	const auto location = key_query.empty() ? build_location(in) : std::optional<std::string>("/cert?session=" + augs::bytes_to_hex(key_public) + key_query);

	if (!location.has_value()) {
		return;
	}

	const auto url = broker_url_of(in.broker_url, last_error);

	if (!url.has_value()) {
		/*
			A failed attempt like any other - a start never waits forever on a broker it cannot ask.
		*/

		++attempts_finished;
		last_attempt_failed = true;
		schedule_retry(now);
		return;
	}

	LOG("Requesting an identity cert from %x.", in.broker_url);

	auto headers = augs::Headers();

	if (in.provider == provider_type::CRAZYGAMES) {
		headers["X-CrazyGames-Token"] = in.crazygames_token;
	}

	/*
		Never after a redirect: the request carries a Steam ticket, a CrazyGames token or the key's signature.
	*/

	request = augs::async_get(url->get_base_url(), location_prefix(*url) + *location, headers, nullptr, false);
	requested_for = in;
}

void identity_session::schedule_retry(const double now) {
	next_attempt_at = now + retry_delay;
	retry_delay = std::min(retry_delay * 2, max_retry_delay_secs);
}

/*
	{"cert": "<hex>", "name": "...", "avatar": "<base64>"} - the cert must name this very key.
*/

std::optional<identity_session::parsed_answer> identity_session::parse_answer(const std::string& body) const {
	try {
		const auto document = augs::json_document_from(body);

		if (!document.IsObject() || !document.HasMember("cert") || !document["cert"].IsString()) {
			return std::nullopt;
		}

		const auto bytes = augs::hex_to_bytes(std::string(document["cert"].GetString()));

		if (!bytes.has_value()) {
			return std::nullopt;
		}

		const auto cert = identity_cert::from_bytes(reinterpret_cast<const std::byte*>(bytes->data()), bytes->size());

		if (!cert.has_value() || cert->session_public_key != key_public || cert->not_after <= cert->not_before) {
			return std::nullopt;
		}

		auto out = parsed_answer();
		out.cert = *cert;

		if (document.HasMember("name") && document["name"].IsString()) {
			out.profile.name = document["name"].GetString();
		}

		if (document.HasMember("avatar") && document["avatar"].IsString()) {
			out.profile.avatar = augs::from_base64(document["avatar"].GetString()).value_or(std::vector<std::byte>());
		}

		return out;
	}
	catch (...) {
		return std::nullopt;
	}
}

static bool is_key_account(const std::string& account) {
	return account.rfind("key_", 0) == 0;
}

void identity_session::advance(const input& in, const double now) {
	last_now = now;

	if (in != last_input) {
		/*
			Another account (e.g. another CrazyGames user): the old cert is no longer it.
			A refreshed CrazyGames token is the same account - its cert stays.
		*/

		const bool account_changed = !in.same_account(last_input);
		const bool had_provider = last_input.provider != provider_type::NONE;
		const bool new_token_after_failure = !account_changed && in.crazygames_token != last_input.crazygames_token && last_attempt_failed;
		last_input = in;

		if (new_token_after_failure) {
			retry_delay = first_retry_delay_secs;
			next_attempt_at = 0.0;
		}

		if (account_changed && had_provider) {
			forget_cert();
			attempts_finished = 0;
			last_attempt_failed = false;
			next_attempt_at = 0.0;
			retry_delay = first_retry_delay_secs;
		}
	}

	/*
		Whether the key is linked is asked through the same /cert as every cert - by the key's signature:
		once a run, quietly, and when the player links (start_link_check).
		Not in the Steam build (its key is never linked) nor on CrazyGames (its account is the token's).
	*/

	const bool key_can_be_linked = !in.steam_ticket_proof && in.provider != provider_type::CRAZYGAMES;

	if (key_check_wanted && key_can_be_linked && request == nullptr && !in.broker_url.empty() && now >= key_check_next_at) {
		LOG(key_check_quiet ? "Asking quietly whether the key is linked." : "Checking whether the key is linked to a Steam account.");

		key_check_wanted = false;
		check_in_flight = true;
		start_request(in, now, key_location());

		if (request == nullptr) {
			check_in_flight = false;
			link_check = link_check_state::FAILED;
		}
	}

	if (request != nullptr) {
		if (!request->is_ready()) {
			return;
		}

		const auto response = augs::get_once(request);
		const bool was_check = std::exchange(check_in_flight, false);
		clear_steam_ticket();

		/*
			Asked for an account that is no longer the player's: whatever came back is not theirs - asked again right away.
		*/

		if (!requested_for.same_account(last_input)) {
			LOG("Dropping an identity cert answer for the previous account.");
			next_attempt_at = 0.0;
			key_check_wanted = key_check_wanted || was_check;
			return;
		}

		const auto answer = response.status == 200 ? parse_answer(response.body) : std::nullopt;

		/*
			A check is no attempt of the sign-in - an unlinked key has nothing to sign in to.
		*/

		const bool signs_in = in.provider != provider_type::NONE;

		if (signs_in) {
			++attempts_finished;
		}

		if (answer.has_value() && is_key_account(answer->cert.account) && key_can_be_linked) {
			/*
				The key's own account: not linked - or no longer (unlinked on the site, or an unlink whose answer got lost).
				Back to the key account, no error.
			*/

			if (cert_saved || (broker_cert.has_value() && !is_key_account(broker_cert->account))) {
				LOG("The key is no longer linked to a Steam account.");
				unlinked_noticed = profile.name;
				on_unlinked();
			}

			if (import_profile_pending) {
				imported_profile = answer->profile;
			}

			last_attempt_failed = false;
			last_error.clear();
			retry_delay = first_retry_delay_secs;
			next_attempt_at = 0.0;

			if (was_check) {
				link_check = link_check_state::NOT_LINKED;
				link_check_profile = answer->profile;

				/*
					A link page still open in the browser - asked again in a while.
				*/

				if (key_check_quiet && now < key_recheck_until) {
					key_check_wanted = true;
					key_check_next_at = now + key_recheck_delay_secs;
				}
				else {
					key_check_quiet = false;
				}
			}

			return;
		}

		if (answer.has_value()) {
			/*
				A cert for the account: Steam (a linked key, the Steam build), CrazyGames.
				A restored key found linked plays with the Steam profile - nothing of its own to wait for.
			*/

			if (import_profile_pending && key_can_be_linked) {
				imported_profile_applied();
			}

			last_attempt_failed = false;
			last_error.clear();
			retry_delay = first_retry_delay_secs;
			next_attempt_at = 0.0;

			/*
				Linked to an account this device did not know of (no steam.cert, or one of another Steam account): the
				player is asked first, and until they say it is theirs the cert waits aside - no connection goes as that
				account. A quiet check of a key with steam.cert of the same account only renews it.
			*/

			const bool knew_it_was_linked =
				cert_saved
				&& broker_cert.has_value()
				&& broker_cert->account == answer->cert.account
			;

			/*
				A renewal for another Steam account than the one in use (unlinked and linked again elsewhere since) is a
				link this device did not know of too.
			*/

			const bool other_account_renewed =
				!was_check
				&& broker_cert.has_value()
				&& broker_cert->account != answer->cert.account
			;

			if ((was_check || other_account_renewed) && !knew_it_was_linked && key_can_be_linked) {
				if (cert_saved) {
					LOG("The key is linked to another Steam account now.");
					on_unlinked();
				}

				unconfirmed_cert = answer->cert;
				unconfirmed_profile = answer->profile;
				unconfirmed_received_at_wall = wall_now();

				LOG("Received an identity cert for %x - waiting for the player to confirm the account.", answer->cert.account);

				if (key_check_quiet || other_account_renewed) {
					linked_quietly = true;
				}

				if (was_check) {
					key_recheck_until = 0.0;

					link_check = link_check_state::LINKED;
					link_check_profile = answer->profile;
					key_check_quiet = false;
				}

				return;
			}

			broker_cert = answer->cert;
			profile = answer->profile;
			cert_received_at = now;
			cert_received_at_wall = wall_now();
			attempts_finished = std::max(attempts_finished, 1);

			const auto lifetime = static_cast<double>(broker_cert->not_after - broker_cert->not_before);
			LOG("Received an identity cert for %x, valid for %x minutes.", broker_cert->account, static_cast<int>(lifetime / 60));

			/*
				steam.cert: a valid one is used at once after a restart, and it tells a key that it is linked.
			*/

			if (in.provider == provider_type::STEAM || was_check) {
				save_cert();
			}

			if (was_check) {
				key_recheck_until = 0.0;

				link_check = link_check_state::LINKED;
				link_check_profile = answer->profile;
				key_check_quiet = false;
			}

			return;
		}

		const auto broker_host = parsed_url(in.broker_url).host;

		const auto error = [&]() -> std::string {
			if (response.status == 200) {
				return typesafe_sprintf("%x sent an invalid answer.", broker_host);
			}

			if (response.status == 429) {
				return typesafe_sprintf("%x is busy - the game keeps trying.", broker_host);
			}

			if (response.status >= 400 && response.status < 500) {
				return augs::plain_text_reason(response).value_or(typesafe_sprintf("%x refused (HTTP %x).", broker_host, response.status));
			}

			if (response.status <= 0) {
				return typesafe_sprintf("Could not reach %x.", broker_host);
			}

			return typesafe_sprintf("%x is not working right now (HTTP %x).", broker_host, response.status);
		}();

		if (was_check) {
			LOG("Checking the key's link failed (HTTP %x).", response.status);
			link_check = link_check_state::FAILED;

			/*
				A refusal (e.g. a ban) tells why - only short plain text, never a page.
			*/

			const bool refused = response.status >= 400 && response.status < 500 && response.status != 429;
			link_check_error = refused ? augs::plain_text_reason(response).value_or(std::string()) : std::string();

			/*
				A quiet one is asked again only when hypersomnia.io may answer later - not after a refusal (e.g. a ban).
			*/

			const bool may_answer_later = response.status <= 0 || response.status == 429 || response.status >= 500;

			if (key_check_quiet && may_answer_later) {
				key_check_wanted = true;
				key_check_next_at = now + key_recheck_delay_secs;
			}
			else {
				key_check_quiet = false;
			}

			if (!signs_in) {
				return;
			}
		}

		last_attempt_failed = true;
		last_error = error;

		if (response.status >= 400 && response.status < 500 && response.status != 429) {
			/*
				Refused (e.g. a ban): the reason is the player's to see, and asking again soon changes nothing.
				A CrazyGames token refused as expired is renewed by the game - a new one is asked about at once.
			*/

			if (response.status == 401 && in.provider == provider_type::CRAZYGAMES && response.body.find("expired") != std::string::npos) {
				crazygames_token_refused = true;
			}

			retry_delay = max_retry_delay_secs;
		}

		LOG("Identity cert request failed: %x Retrying in %x seconds.", last_error, retry_delay);
		schedule_retry(now);
		return;
	}

	if (in.provider == provider_type::NONE) {
		return;
	}

	if (in.broker_url.empty()) {
		/*
			A failed attempt, or "Signing in..." would never end.
		*/

		if (!last_attempt_failed) {
			last_error = "No identity broker set (Settings - Advanced).";
			attempts_finished = std::max(attempts_finished, 1);
			last_attempt_failed = true;
		}

		return;
	}

	/*
		Steam may never answer a ticket request.
	*/

	if (steam_ticket_requested && now - steam_ticket_requested_at > steam_ticket_timeout_secs) {
		on_steam_ticket({}, false);
	}

	if (broker_cert.has_value()) {
		const auto lifetime = static_cast<double>(broker_cert->not_after - broker_cert->not_before);

		if (cert_secs_left(now) > lifetime / 2) {
			return;
		}
	}

	if (now < next_attempt_at) {
		return;
	}

	if (in.steam_ticket_proof && !steam_ticket.has_value()) {
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
	return identity_cert::steam_ticket_identity(key_public);
}

void identity_session::clear_steam_ticket() {
	steam_ticket.reset();
	steam_ticket_wanted = false;
	steam_ticket_requested = false;
}

void identity_session::on_steam_ticket(const std::vector<std::byte>& ticket_bytes, const bool successful) {
	steam_ticket_wanted = false;
	steam_ticket_requested = false;

	if (!successful || ticket_bytes.empty()) {
		last_error = "Steam did not give a ticket. Is Steam online?";
		LOG(last_error);
		++attempts_finished;
		last_attempt_failed = true;
		schedule_retry(last_now);
		return;
	}

	steam_ticket = ticket_bytes;
}

void identity_session::retry_now() {
	retry_delay = first_retry_delay_secs;
	next_attempt_at = 0.0;

	if (broker_cert.has_value() && cert_secs_left(last_now) <= 0.0) {
		forget_cert();
	}
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
	const auto by_steady = cert_received_at + lifetime - now;
	const auto by_wall = cert_received_at_wall + lifetime - wall_now();

	return std::min(by_steady, by_wall);
}

bool identity_session::has_valid_cert(const double now) const {
	return broker_cert.has_value() && cert_secs_left(now) > cert_expiry_margin_secs;
}

bool identity_session::signing_in_first_time(const input& in, const double now) const {
	return in.provider != provider_type::NONE && !has_valid_cert(now) && attempts_finished == 0;
}

bool identity_session::sign_in_failed(const input& in, const double now) const {
	return in.provider != provider_type::NONE && !has_valid_cert(now) && attempts_finished > 0 && last_attempt_failed;
}

identity_cert::cert identity_session::cert_for_connect(const double now) const {
	if (has_valid_cert(now)) {
		return *broker_cert;
	}

	return identity_cert::make_self_signed(key_seed, key_public).value_or(identity_cert::cert());
}

void identity_session::start_link_check(const std::string& broker_url) {
	/*
		Asked through the cert loop (see advance) - right away, or after the request on its way.
	*/

	if (broker_url.empty() || key_location().empty()) {
		link_check = link_check_state::FAILED;
		return;
	}

	key_check_wanted = true;
	key_check_quiet = false;
	key_check_next_at = 0.0;

	link_check = link_check_state::PENDING;
	link_check_profile = {};
	link_check_error.clear();
}

void identity_session::clear_link_check() {
	/*
		A quiet check dropped by the UI is not given up - it is asked again in the background.
	*/

	if (key_check_quiet && link_check == link_check_state::PENDING) {
		key_check_wanted = true;
		key_check_next_at = last_now;
	}

	link_check = link_check_state::NONE;
	link_check_profile = {};
}

