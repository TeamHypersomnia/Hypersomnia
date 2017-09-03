#pragma once
#include "augs/misc/machine_entropy.h"

struct ImGuiIO;

struct release_flags {
	bool keys = false;
	bool mouse = false;

	void set_all();

	void set_due_to_imgui(const ImGuiIO& io);
	void set_due_to_window_deactivation(const augs::local_entropy& entropy);
	void set_due_to_esc(const augs::local_entropy& entropy);

	void apply_into(
		augs::local_entropy& entropy,
		augs::event::state& state
	) const;

	bool any() const;
};