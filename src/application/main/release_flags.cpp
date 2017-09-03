#include <imgui/imgui.h>

#include "augs/templates/container_templates.h"

#include "application/main/release_flags.h"

void release_flags::set_all() {
	keys = mouse = true;
}

void release_flags::set_due_to_imgui(const ImGuiIO& io) {
	thread_local bool released_due_to_text_input = false;

	if (io.WantTextInput) {
		if (!released_due_to_text_input) {
			keys = mouse = true;
			released_due_to_text_input = true;
		}
	}
	else {
		released_due_to_text_input = false;
	}
}

void release_flags::set_due_to_window_deactivation(const augs::local_entropy& entropy) {
	for (const auto& n : entropy) {
		if (n.msg == augs::event::message::activate) {
			set_all();
		}
	}
}

void release_flags::set_due_to_esc(const augs::local_entropy& entropy) {
	for (const auto& n : entropy) {
		if (
			n.msg == augs::event::message::keydown
			&& n.key.key == augs::event::keys::key::ESC
		) {
			set_all();
		}
	}
}

void release_flags::apply_into(
	augs::local_entropy& into_entropy,
	augs::event::state& into_state
) const {
	auto apply_releases = [&](const auto& released) {
		for (const auto c : released) {
			into_state.apply(c);
		}

		concatenate(into_entropy, released);
	};

	if (keys) {
		apply_releases(into_state.generate_key_releasing_changes());
	}

	if (mouse) {
		apply_releases(into_state.generate_mouse_releasing_changes());
	}
}

bool release_flags::any() const {
	return keys || mouse;
}