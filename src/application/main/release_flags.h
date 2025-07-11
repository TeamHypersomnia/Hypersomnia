#pragma once
#include <cstddef>
#include "augs/misc/machine_entropy.h"

struct ImGuiIO;

struct release_flags {
	bool keys = false;
	bool mouse = false;

	release_flags& set_all();

	release_flags& set_due_to_imgui(const ImGuiIO& io);

#if 0
	void insert_into(
		augs::local_entropy& entropy,
		std::size_t at_index
	);

	void apply_into(
		augs::local_entropy& entropy,
		augs::event::state& state
	) const;

	void simulate_applies_and_append_releases(
		augs::local_entropy& entropy,
		const augs::event::state& state
	) const;
#endif

	void append_releases(
		augs::local_entropy& entropy,
		const augs::event::state& state
	) const;

	bool any() const;
};