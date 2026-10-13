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

/*
	A cert is not used for its last two minutes: a server's clock may be a little ahead.
*/

static constexpr double cert_expiry_margin_secs = 120.0;
static constexpr double first_retry_delay_secs = 30.0;
static constexpr double max_retry_delay_secs = 300.0;
static constexpr double steam_ticket_timeout_secs = 10.0;
static constexpr double key_recheck_delay_secs = 30.0;

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
	steam_ticket_needed = false;

	if (const auto public_key = augs::ed25519_public_key(key_seed)) {
		key_public = *public_key;
	}

	std::memcpy(key_secret.data(), key_seed.data(), key_seed.size());
	std::memcpy(key_secret.data() + key_seed.size(), key_public.data(), key_public.size());

	cert_file = new_cert_file;
	profile_file = keys_dir / "steam_profile.json";
	checked_key_file = keys_dir / "account.checked";

	forget_cert();

	/*
		Whatever was on its way was asked for the old key.
	*/

	request.reset();
	link_check_request.reset();
	link_check = link_check_state::NONE;
	clear_steam_ticket();

	attempts_finished = 0;
	last_attempt_failed = false;
	next_attempt_at = 0.0;
	retry_delay = first_retry_delay_secs;
	last_error.clear();

	load_saved_cert();

	key_check_quiet = false;
	key_check_next_at = 0.0;

	/*
		A key whose Steam cert is saved is known; any other is asked about unless it is the one last asked about.
	*/

	if (cert_saved) {
		key_check_wanted = false;
		save_checked_key();
	}
	else {
		const auto checked = [&]() {
			try {
				return augs::file_to_string(checked_key_file);
			}
			catch (...) {
				return std::string();
			}
		}();

		key_check_wanted = checked.find(augs::bytes_to_hex(key_public)) == std::string::npos;
	}
}

void identity_session::save_checked_key() {
	try {
		std::filesystem::create_directories(checked_key_file.parent_path());
		augs::save_as_text_atomic(checked_key_file, augs::bytes_to_hex(key_public) + "\n");
	}
	catch (...) {
		LOG("Could not save %x.", checked_key_file);
	}
}

void identity_session::recheck_link_quietly() {
	key_check_wanted = true;
	key_check_quiet = true;
	key_check_next_at = last_now + key_recheck_delay_secs;
}

void identity_session::on_key_created() {
	key_check_wanted = false;
	save_checked_key();
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
	last_error.clear();
	last_attempt_failed = false;
}

std::string identity_session::key_account() const {
	return identity_cert::key_account(key_public);
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
		if (in.steam_ticket_proof && steam_ticket_needed) {
			if (!steam_ticket.has_value()) {
				return std::nullopt;
			}

			return location + "&steam=" + augs::bytes_to_hex(*steam_ticket) + "&link=1";
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

void identity_session::start_request(const input& in, const double now) {
	const auto location = build_location(in);

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

	/*
		The device's name, percent-encoded - only letters, digits and a few marks go as they are.
	*/

	const auto device = [&]() {
		auto encoded = std::string();

		for (const auto c : device_name.substr(0, 64)) {
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

	if (in.provider == provider_type::CRAZYGAMES) {
		headers["X-CrazyGames-Token"] = in.crazygames_token;
	}

	request = augs::async_get(url->get_base_url(), location_prefix(*url) + *location + device, headers);
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
		last_input = in;

		if (account_changed && had_provider) {
			forget_cert();
			attempts_finished = 0;
			last_attempt_failed = false;
			next_attempt_at = 0.0;
			retry_delay = first_retry_delay_secs;
		}
	}

	if (key_check_wanted && link_check_request == nullptr && !in.broker_url.empty() && now >= key_check_next_at) {
		LOG("Asking whether the key is linked (%x).", key_check_quiet ? "again, quietly" : "not seen before - as on an import");
		const bool quiet = key_check_quiet;
		start_link_check(in.broker_url);
		key_check_quiet = quiet;
		key_check_started = !quiet && link_check == link_check_state::PENDING;
	}

	if (link_check_request != nullptr && link_check_request->is_ready()) {
		const auto response = augs::get_once(link_check_request);
		const auto answer = response.status == 200 ? parse_answer(response.body) : std::nullopt;

		if (!answer.has_value()) {
			LOG("Checking the key's link failed (HTTP %x).", response.status);
			link_check = link_check_state::FAILED;

			/*
				A refusal (e.g. a ban) tells why - only short plain text, never a page.
			*/

			const bool refused = response.status >= 400 && response.status < 500 && response.status != 429;
			const bool plain_text = response.body.size() <= 300 && response.body.find('<') == std::string::npos;
			link_check_error = refused && plain_text ? response.body : std::string();

			/*
				Asked again only when hypersomnia.io may answer later - not after a refusal (e.g. a ban).
			*/

			const bool may_answer_later = response.status <= 0 || response.status == 429 || response.status >= 500;

			if (key_check_quiet && may_answer_later) {
				key_check_wanted = true;
				key_check_next_at = now + key_recheck_delay_secs;
			}
			else {
				key_check_quiet = false;
			}
		}
		else if (is_key_account(answer->cert.account)) {
			link_check = link_check_state::NOT_LINKED;
			link_check_profile = answer->profile;
			key_check_quiet = false;
			save_checked_key();
		}
		else {
			/*
				Linked: the Steam cert is ours from now on, and steam.cert says so on the next start.
			*/

			link_check = link_check_state::LINKED;
			link_check_profile = answer->profile;

			broker_cert = answer->cert;
			profile = answer->profile;
			cert_received_at = now;
			cert_received_at_wall = wall_now();
			attempts_finished = std::max(attempts_finished, 1);
			last_attempt_failed = false;
			last_error.clear();
			save_cert();
			key_check_quiet = false;
			save_checked_key();
		}
	}

	if (request != nullptr) {
		if (!request->is_ready()) {
			return;
		}

		const auto response = augs::get_once(request);
		clear_steam_ticket();
		++attempts_finished;

		/*
			Asked for an account that is no longer the player's: whatever came back is not theirs - asked again right away.
		*/

		if (!requested_for.same_account(last_input)) {
			LOG("Dropping an identity cert answer for the previous account.");
			next_attempt_at = 0.0;
			return;
		}

		const auto answer = response.status == 200 ? parse_answer(response.body) : std::nullopt;

		if (answer.has_value() && in.steam_ticket_proof && answer->cert.account != in.steam_account) {
			if (!steam_ticket_needed) {
				/*
					The key is not linked to this Steam account (yet, or no longer): once with a ticket, which links it.
					Not an attempt of its own - the first sign-in still waits for that one.
				*/

				LOG("The key is not linked to %x - asking with a Steam ticket.", in.steam_account);
				steam_ticket_needed = true;
				next_attempt_at = 0.0;
				--attempts_finished;
				return;
			}

			LOG("A cert for %x came for a Steam ticket of %x - not used.", answer->cert.account, in.steam_account);
		}

		if (answer.has_value() && in.steam_ticket_proof && answer->cert.account == in.steam_account) {
			steam_ticket_needed = false;
		}

		if (answer.has_value() && (!in.steam_ticket_proof || answer->cert.account == in.steam_account)) {
			last_attempt_failed = false;
			last_error.clear();
			retry_delay = first_retry_delay_secs;
			next_attempt_at = 0.0;

			if (is_key_account(answer->cert.account) && !in.steam_ticket_proof && in.provider == provider_type::STEAM) {
				/*
					The key's own account: it was unlinked on the site. Back to "this computer", no error.
				*/

				LOG("The key is no longer linked to a Steam account.");
				forget_cert();
				delete_saved_cert();
				return;
			}

			broker_cert = answer->cert;
			profile = answer->profile;
			cert_received_at = now;
			cert_received_at_wall = wall_now();

			const auto lifetime = static_cast<double>(broker_cert->not_after - broker_cert->not_before);
			LOG("Received an identity cert for %x, valid for %x minutes.", broker_cert->account, static_cast<int>(lifetime / 60));

			if (in.provider == provider_type::STEAM) {
				save_cert();
			}

			return;
		}

		last_attempt_failed = true;

		if (response.status == 200) {
			last_error = "hypersomnia.io sent an invalid answer.";
		}
		else if (response.status >= 400 && response.status < 500 && response.status != 429) {
			/*
				Refused (e.g. a ban): the reason is the player's to see, and asking again soon changes nothing.
			*/

			last_error = response.body.empty() ? typesafe_sprintf("hypersomnia.io refused (HTTP %x).", response.status) : response.body;
			retry_delay = max_retry_delay_secs;
		}
		else {
			last_error = typesafe_sprintf("Could not reach hypersomnia.io (HTTP %x).", response.status);
		}

		LOG("Identity cert request failed: %x Retrying in %x seconds.", last_error, retry_delay);
		schedule_retry(now);
		return;
	}

	if (in.provider == provider_type::NONE || in.broker_url.empty()) {
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

	if (in.steam_ticket_proof && steam_ticket_needed && !steam_ticket.has_value()) {
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

identity_cert::cert identity_session::cert_for_connect(const double now, const std::string& forced_account) const {
	const bool forced_key = is_key_account(forced_account);
	const bool forced_other = !forced_account.empty() && !forced_key;

	if (!forced_key && has_valid_cert(now)) {
		if (!forced_other || broker_cert->account == forced_account) {
			return *broker_cert;
		}
	}

	return identity_cert::make_self_signed(key_seed, key_public).value_or(identity_cert::cert());
}

void identity_session::start_link_check(const std::string& broker_url) {
	key_check_wanted = false;
	key_check_quiet = false;

	const auto key = key_location();
	auto error = std::string();
	const auto url = broker_url_of(broker_url, error);

	if (key.empty() || !url.has_value()) {
		link_check = link_check_state::FAILED;
		return;
	}

	LOG("Checking whether the key is linked to a Steam account.");

	link_check_request = augs::async_get(
		url->get_base_url(),
		location_prefix(*url) + "/cert?session=" + augs::bytes_to_hex(key_public) + key
	);

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

	link_check_request.reset();
	link_check = link_check_state::NONE;
	link_check_profile = {};
}

void identity_session::continue_without_link_check() {
	/*
		Not asked again at every start - but quietly, until hypersomnia.io answers.
	*/

	save_checked_key();
	recheck_link_quietly();
}
