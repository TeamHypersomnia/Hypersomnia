#pragma once
#include <vector>
#include <string>
#include <algorithm>

namespace augs {
	auto make_get_line_until(
		const std::vector<std::string>& lines,
		std::size_t& current_line
	) {
		return [&lines, &current_line](const std::string delimiter = std::string()) {
			while (
				current_line < lines.size()
				&& (
					std::all_of(lines[current_line].begin(), lines[current_line].end(), isspace)
					|| lines[current_line][0] == '%'
				)
			) {
				++current_line;
			}

			if (!(current_line < lines.size())) {
				return false;
			}
			
			if (!delimiter.empty() && lines[current_line] == delimiter) {
				++current_line;
				return false;
			}
			
			return true;
		};
	}
}