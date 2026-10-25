#pragma once
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "augs/templates/thread_templates.h"
#include "augs/misc/httplib_utils.h"
#include "augs/misc/date_time.h"
#include "augs/string/hex.h"
#include "augs/templates/container_templates.h"

/*
	Match reports wait on the disk until hypersomnia.io takes them - a report made while the site is down
	is sent when it is back, not lost. Each server instance has its own folder (by its instance label - not by its
	port, which may change), since instances share the user folder. A report is signed with the server key ("hs-report-v1" and the body): the site knows an official
	server by its public key.

	2xx and 409 (already have it) remove a report, so do the other 4xx (with an error in the log - retrying can not help),
	and so does age past what the site still accepts. A network error, a 429 or a 5xx is retried with a growing delay.

	A 401 is a server the site doesn't know as official (e.g. a community tournament): tried once, then kept in
	unsent_reports as it was sent - the body and its signature - for the site's admins to count it later, by hand.
*/

class report_queue {
public:
	static constexpr std::string_view signature_tag = "hs-report-v1";
	static constexpr std::size_t max_reports = 1000;
	static constexpr uint64_t max_age_secs = 3 * 24 * 60 * 60;

private:
	augs::path_type folder;
	augs::ed25519_seed_type server_key = {};
	std::string server_public_key_hex;

	struct in_flight_result {
		augs::path_type file;
		std::string url;
		int status = -1;
	};

	std::future<in_flight_result> in_flight;
	double next_attempt_at = 0.0;

	/*
		Each endpoint backs off on its own: one that is down must not hold back the reports to the others.
	*/

	struct endpoint_backoff {
		double next_attempt_at = 0.0;
		double retry_delay = 15.0;
	};

	std::unordered_map<std::string, endpoint_backoff> backoff_by_url;

	augs::path_type unsent_folder;

	static uint64_t now_unix() {
		return static_cast<uint64_t>(augs::date_time::secs_since_epoch());
	}

	std::vector<augs::path_type> list_sorted() const {
		auto out = std::vector<augs::path_type>();
		std::error_code ec;

		for (const auto& entry : std::filesystem::directory_iterator(folder, ec)) {
			if (entry.path().extension() == ".report") {
				out.push_back(entry.path());
			}
		}

		std::sort(out.begin(), out.end());
		return out;
	}

	/*
		<url>\n<body>
	*/

	/*
		Only the first line - whether its endpoint is backing off is known without reading the report (up to hundreds
		of KB with the avatars).
	*/

	static std::optional<std::string> url_of(const augs::path_type& file) {
		auto in = std::ifstream(file, std::ios::binary);
		std::string url;

		if (!in || !std::getline(in, url)) {
			return std::nullopt;
		}

		return url;
	}

	static bool parse(const std::string& content, std::string& url, std::string& body) {
		const auto newline = content.find('\n');

		if (newline == std::string::npos) {
			return false;
		}

		url = content.substr(0, newline);
		body = content.substr(newline + 1);
		return true;
	}

	static uint64_t queued_at(const augs::path_type& file) {
		try {
			return std::stoull(file.stem().string().substr(0, file.stem().string().find('-')));
		}
		catch (...) {
			return 0;
		}
	}

	static void remove(const augs::path_type& file) {
		std::error_code ec;
		std::filesystem::remove(file, ec);
	}

public:
	/*
		One folder per instance (r1, t1) - the same instance finds its reports after a restart, whatever port the
		"instances" config gives it now.
	*/

	void init(const augs::path_type& user_dir, const std::string& instance_label, const augs::ed25519_seed_type& key) {
		folder = user_dir / "report_queue" / instance_label;
		unsent_folder = user_dir / "unsent_reports";
		server_key = key;

		if (const auto public_key = augs::ed25519_public_key(key)) {
			server_public_key_hex = augs::bytes_to_hex(*public_key);
		}

		std::error_code ec;
		std::filesystem::create_directories(folder, ec);

		if (const auto n = list_sorted().size(); n > 0) {
			LOG("%x match reports wait in %x to be sent.", n, folder);
		}
	}

	/*
		endpoint_index tells apart the files of one match for several endpoints (hypersomnia.io and the aux ones).
	*/

	void push(const std::string& url, const std::string& body, const std::string& match_id, const std::size_t endpoint_index) {
		auto existing = list_sorted();

		while (existing.size() >= max_reports) {
			LOG("ERROR: the match report queue is full (%x). Dropping the oldest: %x", max_reports, existing.front());
			remove(existing.front());
			existing.erase(existing.begin());
		}

		const auto file = folder / (std::to_string(now_unix()) + "-" + match_id + "-" + std::to_string(endpoint_index) + ".report");

		try {
			augs::save_as_text_atomic(file, url + "\n" + body);
		}
		catch (const std::exception& e) {
			LOG("ERROR: could not queue a match report at %x: %x", file, e.what());
			return;
		}

		/* Right away, unless its endpoint is backing off */
		next_attempt_at = 0.0;
	}

	void advance(const double now) {
#if PLATFORM_WEB
		/*
			A server in the browser is never official - it has nothing to report.
		*/

		(void)now;
#else
		if (in_flight.valid()) {
			if (!::is_ready(in_flight)) {
				return;
			}

			const auto result = in_flight.get();
			const auto status = result.status;

			if ((status >= 200 && status < 300) || status == 409) {
				LOG("Match report %x delivered (HTTP %x).", result.file.filename(), status);
				remove(result.file);
				backoff_by_url.erase(result.url);
				next_attempt_at = now;
				return;
			}

			/*
				429 is the site's limits: retried, like a network error (and a 503 - e.g. the site not reading its list of
				official servers for a moment), until the report is too old. A 401 (not an official server) is kept aside
				to be counted by hand (keep_unsent), not retried. Other 4xx are final.
			*/

			if (status == 401) {
				keep_unsent(result.file);
				remove(result.file);
				next_attempt_at = now;
				return;
			}

			const bool retryable = status == 429;

			if (status >= 400 && status < 500 && !retryable) {
				LOG("ERROR: hypersomnia.io refused the match report %x (HTTP %x). Dropping it.", result.file.filename(), status);
				remove(result.file);
				next_attempt_at = now;
				return;
			}

			auto& backoff = backoff_by_url[result.url];

			LOG("Match report %x not delivered (HTTP %x). Retrying its endpoint in %x seconds.", result.file.filename(), status, backoff.retry_delay);
			backoff.next_attempt_at = now + backoff.retry_delay;
			backoff.retry_delay = std::min(backoff.retry_delay * 2, 600.0);
			next_attempt_at = now;
			return;
		}

		if (now < next_attempt_at) {
			return;
		}

		/*
			The oldest report whose endpoint is not backing off.
		*/

		augs::path_type file;
		std::string url;
		std::string body;

		for (const auto& candidate : list_sorted()) {
			if (const auto at = queued_at(candidate); at != 0 && at + max_age_secs < now_unix()) {
				LOG("ERROR: the match report %x is older than the site accepts. Dropping it.", candidate.filename());
				remove(candidate);
				continue;
			}

			if (const auto candidate_endpoint = url_of(candidate)) {
				if (const auto backoff = mapped_or_nullptr(backoff_by_url, *candidate_endpoint); backoff != nullptr && now < backoff->next_attempt_at) {
					continue;
				}
			}

			std::string candidate_url;
			std::string candidate_body;

			try {
				if (!parse(augs::file_to_string(candidate), candidate_url, candidate_body)) {
					LOG("ERROR: unreadable match report %x. Dropping it.", candidate);
					remove(candidate);
					continue;
				}
			}
			catch (...) {
				remove(candidate);
				continue;
			}

			file = candidate;
			url = std::move(candidate_url);
			body = std::move(candidate_body);
			break;
		}

		if (file.empty()) {
			next_attempt_at = now + 5.0;
			return;
		}

		const auto signature = augs::ed25519_sign(server_key, std::string(signature_tag) + body);

		if (!signature.has_value()) {
			LOG("ERROR: could not sign the match report %x.", file.filename());
			next_attempt_at = now + 60.0;
			return;
		}

		in_flight = launch_async(
			[file, url, body, signature_hex = augs::bytes_to_hex(*signature), key_hex = server_public_key_hex]() {
				auto out = in_flight_result { file, url, -1 };

				const auto parsed = parsed_url(url);

				if (!parsed.valid()) {
					out.status = 400;
					return out;
				}

				auto http_client = httplib_utils::make_client(parsed, 60);

				httplib::Headers headers;
				headers.emplace("X-Server-Key", key_hex);
				headers.emplace("X-Signature", signature_hex);

				if (const auto result = http_client->Post(parsed.location.c_str(), headers, body, "application/json")) {
					LOG("Report %x: HTTP %x: %x", file.filename(), result->status, result->body);
					out.status = result->status;
				}

				return out;
			}
		);
#endif
	}

	/*
		A report the site refused as not from an official server: its body as it was sent (<name>.json) and its headers
		(<name>.headers) - posted again unchanged, the signature still holds:
			curl -H @<name>.headers -H 'Content-Type: application/json' --data-binary @<name>.json <url>
		Named as in the queue (its time, match and endpoint) - none overwrites another.
	*/

	void keep_unsent(const augs::path_type& file) const {
		std::string url;
		std::string body;

		auto lost = [&](const char* const why) {
			LOG("ERROR: could not keep the unsent match report %x (%x) - it's lost.", file.filename(), why);
		};

		try {
			if (!parse(augs::file_to_string(file), url, body)) {
				lost("unreadable");
				return;
			}
		}
		catch (...) {
			lost("unreadable");
			return;
		}

		const auto signature = augs::ed25519_sign(server_key, std::string(signature_tag) + body);

		if (!signature.has_value()) {
			lost("not signed");
			return;
		}

		const auto stem = unsent_folder / file.stem();
		const auto json_path = augs::path_type(stem.string() + ".json");
		const auto headers_path = augs::path_type(stem.string() + ".headers");

		try {
			std::error_code ec;
			std::filesystem::create_directories(unsent_folder, ec);
			augs::save_as_text_atomic(json_path, body);
			augs::save_as_text_atomic(headers_path, "X-Server-Key: " + server_public_key_hex + "\nX-Signature: " + augs::bytes_to_hex(*signature) + "\n");
		}
		catch (const std::exception& e) {
			LOG("ERROR: could not keep the unsent match report at %x: %x", json_path, e.what());
			return;
		}

		LOG(
			"Not an official server - the match report isn't counted for MMR. Kept in %x (and its .headers): send both to hypersomnia.io's admins to count it, with this server's key %x (--print-server-key).",
			json_path,
			server_public_key_hex
		);
	}

	void wait_for_in_flight() {
		if (in_flight.valid()) {
			in_flight.wait();
		}
	}
};
