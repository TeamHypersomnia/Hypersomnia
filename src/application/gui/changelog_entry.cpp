#include <array>
#include <cctype>
#include <sstream>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/date_time.h"
#include "hypersomnia_version.h"
#include "application/gui/changelog_entry.h"

/*
	Fills the pretty date and the timestamp from "YYYY-MM-DD". Leaves them empty on a malformed date.
*/

static void parse_date(const std::string& iso, changelog_entry& into) {
	const bool well_formed = [&]() {
		if (iso.size() != 10 || iso[4] != '-' || iso[7] != '-') {
			return false;
		}

		for (const auto i : { 0, 1, 2, 3, 5, 6, 8, 9 }) {
			if (!std::isdigit(static_cast<unsigned char>(iso[i]))) {
				return false;
			}
		}

		return true;
	}();

	if (!well_formed) {
		return;
	}

	const auto year = std::stoi(iso.substr(0, 4));
	const auto month = std::stoi(iso.substr(5, 2));
	const auto day = std::stoi(iso.substr(8, 2));

	if (month < 1 || month > 12 || day < 1 || day > 31) {
		return;
	}

	static const auto months = std::array<const char*, 12> {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	};

	into.date = std::string(months[month - 1]) + " " + std::to_string(day) + ", " + std::to_string(year);
	into.timestamp = static_cast<int64_t>(augs::date_time::from_utc_timestamp(iso + " 00:00:00").t);
}

changelog_entries parse_changelog(const std::string& source) {
	auto result = changelog_entries();
	auto bodies = std::vector<std::string>();

	/*
		Only collects text of the versions we keep. Unreleased and the intro are skipped.
	*/

	bool collecting = false;
	auto s = std::istringstream(source);

	for (std::string line; std::getline(s, line); ) {
		if (const bool is_version_line = line.rfind("## ", 0) == 0) {
			auto header = line.substr(3);

			while (!header.empty() && std::isspace(static_cast<unsigned char>(header.back()))) {
				header.pop_back();
			}

			const auto version = [&]() -> std::string {
				if (header.rfind("[", 0) == 0) {
					const auto close = header.find(']');
					return close == std::string::npos ? std::string() : header.substr(1, close - 1);
				}

				return header.substr(0, header.find(' '));
			}();

			collecting = !version.empty() && version != "Unreleased";

			if (collecting) {
				auto entry = changelog_entry();
				entry.version = version;

				if (const auto dash = header.find(" - "); dash != std::string::npos) {
					::parse_date(header.substr(dash + 3, 10), entry);
				}

				result.emplace_back(std::move(entry));
				bodies.emplace_back();
			}

			continue;
		}

		if (collecting) {
			bodies.back() += line;
			bodies.back() += '\n';
		}
	}

	for (std::size_t i = 0; i < result.size(); ++i) {
		result[i].lines = augs::parse_markdown(bodies[i]);
	}

	return result;
}

changelog_entries load_changelog(const augs::path_type& path) {
	try {
		auto entries = ::parse_changelog(augs::file_to_string_crlf_to_lf(path));

		std::erase_if(entries, [](const changelog_entry& entry) {
			return ::is_more_recent(CHANGELOG_SINCE_VERSION, entry.version);
		});

		return entries;
	}
	catch (...) {
		LOG("Couldn't read the changelog from %x.", path);
	}

	return {};
}

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("ParseChangelog", "[changelog]") {
	const auto entries = parse_changelog(
		"# Changelog\n"
		"\n"
		"Intro to skip.\n"
		"\n"
		"## [Unreleased]\n"
		"\n"
		"- Not shown.\n"
		"\n"
		"## [3.1.0] - 2026-10-04\n"
		"\n"
		"### Added\n"
		"\n"
		"- **Changelog.**\n"
		"\n"
		"## [3.0.0] - 1970-01-02\n"
		"- Old.\n"
		"## 1.0.0\n"
	);

	REQUIRE(entries.size() == 3);

	REQUIRE(entries[0].version == "3.1.0");
	REQUIRE(entries[0].date == "October 4, 2026");
	REQUIRE(entries[0].lines.size() == 2);
	REQUIRE(entries[0].lines[0].type == augs::markdown_line_type::HEADING);
	REQUIRE(entries[0].lines[1].get_plain_text() == "Changelog.");

	REQUIRE(entries[1].version == "3.0.0");
	REQUIRE(entries[1].timestamp == 24 * 60 * 60);
	REQUIRE(entries[1].lines.size() == 1);

	REQUIRE(entries[2].version == "1.0.0");
	REQUIRE(entries[2].date.empty());
	REQUIRE(entries[2].lines.empty());
}
#endif
