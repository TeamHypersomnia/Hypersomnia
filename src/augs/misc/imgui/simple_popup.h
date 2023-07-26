#pragma once
#include <map>
#include <vector>

struct simple_popup {
	static simple_popup sum_all(const std::vector<simple_popup>& popups);

	std::string title;
	std::string message;
	std::string details;

	std::string warning_notice = "";
	std::string warning_notice_above = "";

	struct button {
		std::string label;
		rgba col = rgba::zero;
		rgba increment = rgba::zero;
	};

	bool details_expanded = false;
	int perform(const std::vector<button>& buttons = {});
	std::string make_log() const;
};
