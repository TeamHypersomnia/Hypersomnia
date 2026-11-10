#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include "augs/math/vec2.h"
#include "augs/image/image.h"
#include "augs/misc/async_response.h"
#include "augs/misc/imgui/standard_window_mixin.h"

struct dev_journal_announcement {
	const char* youtube_url;
	const char* title;
	std::uint64_t premiere_unix_utc;
	std::uint64_t shown_until_unix_utc;
};

constexpr auto latest_dev_journal_v = dev_journal_announcement {
	"https://youtu.be/ftnChkC1-8w",
	"Developer journal #6",
	1794589200,
	1797181200
};

constexpr auto dev_journal_thumbnail_location_v = "/assets/images/devlog.png";
constexpr auto dev_journal_green_blood_thumbnail_location_v = "/assets/images/devlog_cg.png";

bool should_show_dev_journal(std::uint64_t seen_premiere_unix_utc);

struct dev_journal_window_state : public standard_window_mixin<dev_journal_window_state> {
	using base = standard_window_mixin<dev_journal_window_state>;
	using base::base;

	augs::async_response_ptr thumbnail_response;
	std::optional<augs::image> thumbnail_to_upload;
	std::optional<vec2u> thumbnail_size;

	void open_fetching_thumbnail(const std::string& website_url, bool green_blood_thumbnail);
	bool perform(bool changelog_continues_in_the_same_popup);
};
