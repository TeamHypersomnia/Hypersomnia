#include "application/setups/editor/gui/editor_tutorial_gui.h"
#include "augs/templates/enum_introspect.h"
#include "application/setups/editor/editor_setup.h"
#include "augs/filesystem/file.h"

static auto make_manual_path(const editor_tutorial_type t) {
	const auto manual_dir = augs::path_type("content/manual/editor");

	auto p = manual_dir / to_lowercase(augs::enum_to_string(t));
	p += ".md";
	return p;
}

editor_tutorial_gui::editor_tutorial_gui(const std::string& name) : standard_window_mixin<editor_tutorial_gui>(name) {
	show = true;

	augs::for_each_enum_except_bounds(
		[&](const editor_tutorial_type t) {
			text_contents[t] = augs::file_to_string(make_manual_path(t));
		}
	);
}

void editor_tutorial_gui::perform(const editor_setup& setup) {
	using namespace augs::imgui;

	const auto manual_dir = augs::path_type("content/manual/editor");

	auto tutorial = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!tutorial) {
		return;
	}

	const auto chosen_tutorial = [&]() {
		using T = editor_tutorial_type;

		if (!setup.anything_opened()) {
			return T::WELCOME;
		}

		const auto& f = setup.folder();

		if (f.empty()) {
			return T::EMPTY_PROJECT;
		}
		else {
			auto& p = f.player;

			if (!p.has_testing_started()) {
				if (f.is_untitled()) {
					return T::FILLED_UNTITLED_PROJECT;
				}

				{
					const auto op = setup.mover.get_current_op(f.history);

					if (op == mover_op_type::TRANSLATING) {
						return T::MOVING_ENTITIES;
					}

					if (op == mover_op_type::ROTATING) {
						return T::ROTATING_ENTITIES;
					}

					if (op == mover_op_type::RESIZING) {
						return T::RESIZING_ENTITIES;
					}
				}

				if (f.allow_close()) {
					if (f.commanded->work.world[setup.selector.get_held()] || setup.get_all_selected_entities().size() > 0) {
						return T::SELECTED_ENTITIES;
					}
					else {
						return T::SAVED_PROJECT;
					}
				}
				else {
					return T::UNSAVED_CHANGES;
				}
			}
			else {
			}
		}

		return T::WELCOME;
	}();

	text_disabled(augs::filename_first(make_manual_path(chosen_tutorial)));

	ImGui::Separator();

	const auto& chosen_text = text_contents.at(chosen_tutorial);

	auto next_ht = [&](const std::size_t from = 0) {
		return chosen_text.find("##", from);
	};

	const auto it = next_ht();

	if (it == std::string::npos) {
		text(chosen_text);
	}
	else {
		text(chosen_text.substr(0, it));
		std::size_t prev_it = it;

		while (prev_it != std::string::npos) {
			const auto begin_title_pos = prev_it + 3;
			const auto newline_pos = chosen_text.find("\n", begin_title_pos);
			const auto title = chosen_text.substr(begin_title_pos, newline_pos - begin_title_pos);
			const auto next_hashtag = next_ht(newline_pos);

			if (ImGui::CollapsingHeader(title.c_str())) {
				auto contents = chosen_text.substr(newline_pos, next_hashtag - newline_pos);

				if (contents.size() > 0) {
					if (contents.size() > 1) {
						if (contents[1] == '\n') {
							contents.erase(contents.begin(), contents.begin() + 2);
						}
					}
					else {
						contents.erase(contents.begin());
					}
				}

				text(contents);
			}
			
			prev_it = next_hashtag;
		}
	}
}
