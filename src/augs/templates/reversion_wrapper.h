#pragma once
#include <ranges>
#include <utility>

template <std::ranges::bidirectional_range R>
auto reverse(R&& r) {
    return std::forward<R>(r) | std::views::reverse;
}
