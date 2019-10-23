#pragma once
#include <map>
#include <vector>

struct editor_popup {
	static editor_popup sum_all(const std::vector<editor_popup>& popups);

	std::string title;
	std::string message;
	std::string details;

	struct button {
		std::string label;
		rgba col = rgba::zero;
		rgba increment = rgba::zero;
	};

	bool details_expanded = false;
	int perform(const std::vector<button>& buttons = {});
};

using editor_warning = editor_popup;
