#include "3rdparty/imgui/imgui.h"
#include "augs/log.h"

#include "augs/templates/container_templates.h"
#include "application/main/release_flags.h"

release_flags& release_flags::set_all() {
	keys = mouse = true;
	return *this;
}

release_flags& release_flags::set_due_to_imgui(const ImGuiIO& io) {
	thread_local bool released_due_to_text_input = false;

	if (io.WantTextInput) {
		if (!released_due_to_text_input) {
			set_all();
			released_due_to_text_input = true;
		}
	}
	else {
		released_due_to_text_input = false;
	}

	return *this;
}

void release_flags::append_releases(
	augs::local_entropy& into_entropy,
	const augs::event::state& from_state
) const {
	if (keys) {
		concatenate(into_entropy, from_state.generate_key_releasing_changes());
		LOG("Released all keys");
	}

	if (mouse) {
		concatenate(into_entropy, from_state.generate_mouse_releasing_changes());
		LOG("Released mouse");
	}
}

#if 0
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

void release_flags::apply_into(
	augs::local_entropy& into_entropy,
	const augs::event::state& into_state
) const {
	if (keys) {
		concatenate(into_entropy, into_state.generate_key_releasing_changes());
	}

	if (mouse) {
		concatenate(into_entropy, into_state.generate_mouse_releasing_changes());
	}
}
#endif

bool release_flags::any() const {
	return keys || mouse;
}