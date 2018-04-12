#pragma once
#include <map>
#include "augs/string/typesafe_sprintf.h"

struct editor_popup {
	template <class C>
	static editor_popup sum_all(const C& popups) {
		editor_popup result;

		// for example: 11 Error(s), 23 Warning(s)

		std::map<std::string, std::size_t> counts;

		for (const auto& p : popups) {
			counts[p.title]++;

			if (p.message.size() > 0) {
				result.message += p.message + "\n\n";
			}

			if (p.details.size() > 0) {
				result.details += p.details + "\n";
			}
		}

		for (const auto& e : counts) {
			result.title += typesafe_sprintf("%x %x(s), ", e.second, e.first);
		}

		if (result.title.size() >= 2) {
			result.title.pop_back();
			result.title.pop_back();
		}

		if (result.message.size() >= 1) {
			result.message.pop_back();
		}

		if (result.details.size() >= 1) {
			result.details.pop_back();
		}

		return result;
	}

	std::string title;
	std::string message;
	std::string details;

	bool details_expanded = false;
	bool perform();
};
