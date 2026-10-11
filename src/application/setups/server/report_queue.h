#pragma once
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "augs/templates/thread_templates.h"
#include "augs/misc/httplib_utils.h"
#include "augs/misc/date_time.h"

/*
	Match reports wait on the disk until hypersomnia.io takes them - a report made while the site is down
	is sent when it is back, not lost. Each server instance has its own folder (by port), since instances share
	the user folder. A report is signed with the server key ("hs-report-v1" and the body): the site knows an official
	server by its public key.

	2xx and 409 (already have it) remove a report, so do the other 4xx (with an error in the log - retrying can not help),
	and so does age past what the site still accepts. A network error or a 5xx is retried with a growing delay.
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
		int status = -1;
	};

	std::future<in_flight_result> in_flight;
	double next_attempt_at = 0.0;
	double retry_delay = 15.0;

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
	void init(const augs::path_type& user_dir, const uint16_t port, const augs::ed25519_seed_type& key) {
		folder = user_dir / "report_queue" / std::to_string(port);
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

	void push(const std::string& url, const std::string& body, const std::string& match_id) {
		auto existing = list_sorted();

		while (existing.size() >= max_reports) {
			LOG("ERROR: the match report queue is full (%x). Dropping the oldest: %x", max_reports, existing.front());
			remove(existing.front());
			existing.erase(existing.begin());
		}

		const auto file = folder / (std::to_string(now_unix()) + "-" + match_id + ".report");

		try {
			augs::save_as_text_atomic(file, url + "\n" + body);
		}
		catch (const std::exception& e) {
			LOG("ERROR: could not queue a match report at %x: %x", file, e.what());
			return;
		}

		/* Right away */
		next_attempt_at = 0.0;
		retry_delay = 15.0;
	}

	void advance(const double now) {
		if (in_flight.valid()) {
			if (!::is_ready(in_flight)) {
				return;
			}

			const auto result = in_flight.get();
			const auto status = result.status;

			if ((status >= 200 && status < 300) || status == 409) {
				LOG("Match report %x delivered (HTTP %x).", result.file.filename(), status);
				remove(result.file);
				retry_delay = 15.0;
				next_attempt_at = now;
				return;
			}

			if (status >= 400 && status < 500) {
				LOG("ERROR: hypersomnia.io refused the match report %x (HTTP %x). Dropping it.", result.file.filename(), status);
				remove(result.file);
				next_attempt_at = now;
				return;
			}

			LOG("Match report %x not delivered (HTTP %x). Retrying in %x seconds.", result.file.filename(), status, retry_delay);
			next_attempt_at = now + retry_delay;
			retry_delay = std::min(retry_delay * 2, 600.0);
			return;
		}

		if (now < next_attempt_at) {
			return;
		}

		const auto files = list_sorted();

		if (files.empty()) {
			next_attempt_at = now + 5.0;
			return;
		}

		const auto& file = files.front();

		if (const auto at = queued_at(file); at != 0 && at + max_age_secs < now_unix()) {
			LOG("ERROR: the match report %x is older than the site accepts. Dropping it.", file.filename());
			remove(file);
			return;
		}

		std::string url;
		std::string body;

		try {
			if (!parse(augs::file_to_string(file), url, body)) {
				LOG("ERROR: unreadable match report %x. Dropping it.", file);
				remove(file);
				return;
			}
		}
		catch (...) {
			remove(file);
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
				auto out = in_flight_result { file, -1 };

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
	}

	void wait_for_in_flight() {
		if (in_flight.valid()) {
			in_flight.wait();
		}
	}
};
