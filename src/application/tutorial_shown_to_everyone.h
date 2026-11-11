#pragma once
#include <cstdint>

constexpr std::uint32_t tutorial_revision_shown_to_everyone_v = 1;

inline bool tutorial_revision_unseen(const std::uint32_t revision_shown) {
	return revision_shown < tutorial_revision_shown_to_everyone_v;
}
