#include "application/setups/editor/gui/editor_tutorial_gui.h"
#include "augs/templates/enum_introspect.h"
#include "application/setups/editor/editor_setup.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/shell.h"

static auto make_dialog_manual_path(const std::string& stem) {
	const auto manual_dir = augs::path_type("content/manual/editor/dialogs");

	auto p = manual_dir / stem;
	p += ".md";
	return p;
}

static auto make_manual_path(const std::string& stem) {
	const auto manual_dir = augs::path_type("content/manual/editor");

	auto p = manual_dir / stem;
	p += ".md";
	return p;
}

static auto make_manual_path(const editor_tutorial_type t) {
	return make_manual_path(to_lowercase(augs::enum_to_string(t)));
}

editor_tutorial_gui::editor_tutorial_gui(const std::string& name) : standard_window_mixin<editor_tutorial_gui>(name) {
	show = true;

	augs::for_each_enum_except_bounds(
		[&](const editor_tutorial_type t) {
			context_manuals[t] = augs::file_to_string(make_manual_path(t));
		}
	);
}

void editor_tutorial_gui::perform(const editor_setup& setup) {
	using namespace augs::imgui;

	const auto manual_dir = augs::path_type("content/manual/editor");

	if (dialog_manuals.empty()) {
		augs::introspect(
			[&](const auto& label, const auto& gui) {
				using T = remove_cref<decltype(gui)>;

				if constexpr(!std::is_same_v<T, editor_tutorial_gui>) {
					const auto text_path = make_dialog_manual_path(label);
					dialog_manuals[label] = augs::file_to_string(text_path);
				}
			},
			setup
		);
	}

	static const std::vector<std::string> other_dialogs = {
		"current_asset"
	};

	for (const auto& o : other_dialogs) {
		const auto text_path = make_dialog_manual_path(o);
		dialog_manuals[o] = augs::file_to_string(text_path);
	}

	auto tutorial = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!tutorial) {
		return;
	}

	augs::path_type chosen_tutorial_path; 
	
	const auto& chosen_text = [&]() {
		std::string focused_dialog;

		augs::introspect(
			[&](const auto& label, const auto& gui) {
				using T = remove_cref<decltype(gui)>;

				if (!std::is_same_v<T, editor_tutorial_gui>) {
					if constexpr(is_one_of_v<T, editor_images_gui, editor_sounds_gui>) {
						if (gui.is_separate_properties_focused()) {
							focused_dialog = "current_asset";
						}
						else if (gui.is_focused()) {
							focused_dialog = label;
						}
					}
					else {
						if (gui.is_focused()) {
							focused_dialog = label;
						}
					}
				}
			},
			setup
		);

		if (!focused_dialog.empty()) {
			current_dialog = focused_dialog;
		}

		if (!current_dialog.empty()) {
			const auto& text = dialog_manuals.at(current_dialog);

			chosen_tutorial_path = make_dialog_manual_path(current_dialog);
			return text;
		}

		const auto chosen_tutorial_type = [&]() {
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
					if (setup.is_editing_mode()) {
						return T::PLAYTESTING_EDITING;
					}
					else if (setup.is_gameplay_on()) {
						return T::PLAYTESTING_INGAME;
					}
				}
			}

			return T::WELCOME;
		}();

		chosen_tutorial_path = make_manual_path(chosen_tutorial_type);
		return context_manuals.at(chosen_tutorial_type);
	}();

	if (ImGui::Button("Open")) {
		augs::open_text_editor(chosen_tutorial_path);
	}

	ImGui::SameLine();

	text_disabled(augs::filename_first(chosen_tutorial_path));

	if (!current_dialog.empty()) {
		ImGui::SameLine();

		if (ImGui::Button("Return")) {
			current_dialog.clear();
		}
	}
	
	ImGui::Separator();

	auto next_ht = [&](const std::size_t from = 0) {
		return chosen_text.find("##", from);
	};

	const auto it = next_ht();

	const auto col = setup.settings.tutorial_text_color;

	if (it == std::string::npos) {
		text_color(chosen_text, col);
	}
	else {
		text_color(chosen_text.substr(0, it), col);
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

				text_color(contents, col);
			}
			
			prev_it = next_hashtag;
		}
	}
}
