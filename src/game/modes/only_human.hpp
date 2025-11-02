#pragma once
#include <ranges>
#include <utility>

template <typename Range>
auto only_human(Range&& range) {
    return std::forward<Range>(range) | std::views::filter(
		[](const auto& elem) {
			return !elem.second.is_bot;
		}
	);
}

template <typename Range>
auto only_bot(Range&& range) {
    return std::forward<Range>(range) | std::views::filter(
		[](const auto& elem) {
			return elem.second.is_bot;
		}
	);
}

