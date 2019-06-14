#include <thread>
#include <future>
#include "application/gui/start_client_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#include "augs/network/network_types.h"
#include "application/setups/editor/detail/maybe_different_colors.h"
#include "augs/templates/thread_templates.h"
#include "augs/window_framework/window.h"

#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/filesystem/file.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

bool start_client_gui_state::perform(
	augs::window& window,
	client_start_input& into_start,
	client_vars& into_vars
) {
	if (!show) {
		return false;
	}
	
	using namespace augs::imgui;

	centered_size_mult = 0.3f;
	
	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return false;
	}

	bool result = false;

	auto settings = scoped_window("Connect to server", &show);
	
	{
		auto child = scoped_child("connect view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		// auto& scope_cfg = into;

		base::acquire_keyboard_once();
		input_text<100>("Address (ipv4:port or [ipv6]:port)", into_start.ip_port);
		input_text<max_nickname_length_v>("Chosen nickname (3-30 characters)", into_vars.nickname);

		struct loading_result {
			std::optional<std::string> new_path;
			augs::image loaded_image;
			std::string error_message;

			bool was_shrinked = false;
			bool will_be_upscaled = false;

			void load_image(const augs::path_type& from) {
				loaded_image.from_png(from);

				const auto max_s = static_cast<unsigned>(max_avatar_side_v);

				auto sz = loaded_image.get_size();

				if (sz != vec2u::square(max_s)) {
					sz.x = std::min(sz.x, max_s);
					sz.y = std::min(sz.x, max_s);

					if (sz != loaded_image.get_size()) {
						was_shrinked = true;

						loaded_image.scale(sz);

						const auto resized_file_path = LOCAL_FILES_DIR "/avatar_resized.png";
						loaded_image.save_as_png(resized_file_path);

						new_path = resized_file_path;
					}
					else {
						will_be_upscaled = true;
					}
				}
			}
		};

		thread_local std::future<loading_result> avatar_loading_result;

		auto reload_avatar = [&](const std::string& from_path) {
			if (!avatar_loading_result.valid()) {
				if (from_path.size() > 0 && avatar_preview_tex == std::nullopt) {
					avatar_loading_result = std::async(
						std::launch::async,
						[from_path]() {
							loading_result out;
							out.new_path = from_path;

							try {
								out.load_image(from_path);
							}
							catch (const augs::image_loading_error& err) {
								out.error_message = err.what();
							}
							catch (const augs::file_open_error& err) {
								out.error_message = err.what();
							}

							return out;
						}
					);
				}
			}
		};

		std::string p = into_vars.avatar_image_path.string();

		if (input_text<512>("Avatar image", p, ImGuiInputTextFlags_EnterReturnsTrue)) {
			reload_avatar(p);
		}

		ImGui::SameLine();

		if (ImGui::Button("Browse") && !error_popup) {
			avatar_loading_result = std::async(
				std::launch::async,
				[&window]() {
					const std::vector<augs::window::file_dialog_filter> filters = { {
						"PNG file",
						"*.png"
					} };

					loading_result out;
					out.new_path = window.open_file_dialog(filters, "Choose avatar image");

					if (out.new_path != std::nullopt) {
						try {
							out.load_image(*out.new_path);
						}
						catch (const augs::image_loading_error& err) {
							out.error_message = err.what();
						}
						catch (const augs::file_open_error& err) {
							out.error_message = err.what();
						}
					}

					return out;
				}
			);
		}

		if (avatar_preview_tex != std::nullopt) {
			ImGui::SameLine();

			if (ImGui::Button("Clear") && !error_popup) {
				p = "";
				avatar_preview_tex.reset();
				was_shrinked = false;
				will_be_upscaled = false;
			}
		}

		const auto first_size = vec2(max_avatar_side_v, max_avatar_side_v);
		const auto half_size = first_size / 2;
		const auto icon_size = vec2(22, 22);

		if (avatar_preview_tex != std::nullopt) {
			augs::atlas_entry entry;
			entry.atlas_space = xywh(0, 0, 1, 1);

			if (augs::imgui::game_image_button("##AvatarButton", entry, first_size, {}, augs::imgui_atlas_type::AVATAR_PREVIEW)) {

			}

			ImGui::SameLine();
			text_disabled(typesafe_sprintf("%xx%x", first_size.x, first_size.y));

			ImGui::SameLine();

			if (augs::imgui::game_image_button("##AvatarButtonHalf", entry, half_size, {}, augs::imgui_atlas_type::AVATAR_PREVIEW)) {

			}

			ImGui::SameLine();

			text_disabled(typesafe_sprintf("%xx%x", half_size.x, half_size.y));

			ImGui::SameLine();


			if (augs::imgui::game_image_button("##AvatarButtonSmall", entry, icon_size, {}, augs::imgui_atlas_type::AVATAR_PREVIEW)) {

			}

			ImGui::SameLine();
			text_disabled(typesafe_sprintf("%xx%x", icon_size.x, icon_size.y));
		}

		const auto size_str = typesafe_sprintf("%xx%x", first_size.x, first_size.y);

		if (was_shrinked) {
			text_disabled(typesafe_sprintf("The chosen image was automatically shrinked to %x.\nTo ensure the best quality,\nsupply an image cropped to exactly %x.\n\n", size_str, size_str));
		}
		else if (will_be_upscaled) {
			text_disabled(typesafe_sprintf("The chosen image will be automatically upscaled to %x.\nTo ensure the best quality,\nsupply an image cropped to exactly %x.\n\n", size_str, size_str));
		}

		if (::valid_and_is_ready(avatar_loading_result)) {
			const auto result = avatar_loading_result.get();
			const auto& error_msg = result.error_message;

			was_shrinked = false;
			will_be_upscaled = false;

			if (error_msg.size() > 0) {
				error_popup = editor_popup();
				error_popup->title = "Error";
				error_popup->message = error_msg;
			}
			else if (result.new_path != std::nullopt) {
				p = *result.new_path;

				auto previous_texture = augs::graphics::texture::find_current();

				avatar_preview_tex.emplace(result.loaded_image);
				avatar_preview_tex->set_filtering(augs::filtering_type::LINEAR);

				augs::graphics::texture::set_current_to(previous_texture);

				was_shrinked = result.was_shrinked;
				will_be_upscaled  = result.will_be_upscaled;
			}
		}

		if (do_initial_load) {
			reload_avatar(p);
			do_initial_load = false;
		}

		if (error_popup) {
			if (error_popup->perform()) {
				error_popup = std::nullopt;
			}
		}

		into_vars.avatar_image_path = p;

		text_disabled("Tip: to quickly connect, you can press Shift+C here or in the main menu,\ninstead of clicking \"Connect!\" with your mouse.");
	}

	{
		auto scope = scoped_child("connect cancel");

		ImGui::Separator();

		const auto len = into_vars.nickname.length();
		const auto clamped_len = std::clamp(len, min_nickname_length_v, max_nickname_length_v);

		{
			auto scope = maybe_disabled_cols({}, len != clamped_len);

			if (ImGui::Button("Connect!")) {
				result = true;
				//show = false;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			show = false;
		}
	}

	return result;
}

