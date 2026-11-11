#pragma once
#include <cstdint>

constexpr std::uint32_t tutorial_revision_shown_to_everyone_v = 1;

template <class Config>
bool starts_in_tutorial(const Config& config) {
	return !config.skip_tutorial || config.tutorial_revision_shown < tutorial_revision_shown_to_everyone_v;
}

template <class Config>
void mark_tutorial_revision_shown(Config& config) {
	config.tutorial_revision_shown = tutorial_revision_shown_to_everyone_v;
}
