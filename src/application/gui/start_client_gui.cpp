#include <thread>
#include <future>
#include "application/gui/start_client_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#include "augs/network/network_types.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/templates/thread_templates.h"
#include "augs/window_framework/window.h"

#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/filesystem/file.h"

#include "augs/misc/imgui/imgui_enum_radio.h"
#include "application/gui/demo_chooser.h"
#include "application/setups/client/demo_file.h"
#include "application/gui/pretty_tabs.h"
#include "augs/readwrite/byte_readwrite.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

address_and_port client_start_input::get_address_and_port() const {
	if (chosen_address_type == connect_address_type::OFFICIAL) {
		return { preferred_official_address, default_port };
	}

	return { custom_address, default_port };
}

void client_start_input::set_custom(const std::string& target) { 
	custom_address = target;
	chosen_address_type = connect_address_type::CUSTOM_ADDRESS;
}

void start_client_gui_state::clear_demo_choice() {
	demo_choice_result = demo_choice_result_type::SHOULD_ANALYZE;
	demo_meta = {};
	demo_size = {};
}

bool start_client_gui_state::perform(
	const augs::frame_num_type current_frame,
	augs::renderer& renderer,
	augs::graphics::texture& avatar_preview_tex,
	augs::window& window,
	client_start_input& into_start,
	client_vars& into_vars,
	const std::vector<std::string>& official_arena_servers
) {
	if (!show) {
		return false;
	}
	
	using namespace augs::imgui;

	centered_size_mult = vec2(0.55f, 0.4f);
	
	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return false;
	}

	bool result = false;

	if (into_start.chosen_address_type == connect_address_type::REPLAY) {
		using D = demo_choice_result_type;

		auto& demo_path = into_start.replay_demo;

		{
			auto child = scoped_child("connect view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
			auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

			do_pretty_tabs(into_start.chosen_address_type);

			thread_local demo_chooser chooser;

			// text_disabled(typesafe_sprintf("The demos are stored inside \"%x\" folder.", DEMOS_DIR));

			text(typesafe_sprintf("Choose demo to replay (from %x):", DEMOS_DIR));

			ImGui::SameLine();
			
			{
				auto scope = maybe_disabled_cols({}, demo_path.empty());

				if (ImGui::Button("Demo in explorer")) {
					window.reveal_in_explorer(demo_path);
				}

				ImGui::SameLine();
			}

			if (ImGui::Button("Folder in explorer")) {
				window.reveal_in_explorer(DEMOS_DIR);
			}

			chooser.perform(
				"replay_demo_chooser",
				demo_path.string(),
				augs::path_type(DEMOS_DIR),
				[&](const auto& new_choice) {
					demo_path = new_choice.path;
					demo_choice_result = D::SHOULD_ANALYZE;
				}
			);

			if (!demo_path.empty() && demo_choice_result == D::SHOULD_ANALYZE) {
				try {
					auto t = augs::with_exceptions<std::ifstream>();
					t.open(demo_path);

					augs::read_bytes(t, demo_meta);
					demo_size = readable_bytesize(augs::get_file_size(demo_path));

					if (demo_meta.version == hypersomnia_version()) {
						demo_choice_result = D::OK;
					}
					else {
						demo_choice_result = D::MIGHT_BE_INCOMPATIBLE;
					}
				}
				catch (const std::ifstream::failure& err) {
					demo_choice_result = D::FILE_OPEN_ERROR;
				}
				catch (const augs::stream_read_error& err) {
					demo_choice_result = D::FILE_OPEN_ERROR;
				}
			}

			switch (demo_choice_result) {
				case D::FILE_OPEN_ERROR:
					text_color(typesafe_sprintf("Could not open:\n%x\n\nWrong path or the file might be corrupt.", demo_path), red);
					break;
				case D::MIGHT_BE_INCOMPATIBLE:
					text_color(typesafe_sprintf("Your client differs from the one used to record this demo.\nYou may try replaying, but all bets are off - the game might even crash."), orange);
					break;
				case D::OK:
					text_color(typesafe_sprintf("OK! The demo's version matches your client."), green);
					break;
				default:
					break;
			}

			if (!demo_path.empty() && demo_choice_result != D::FILE_OPEN_ERROR) {
				text("Server address:");
				ImGui::SameLine();
				text_color(demo_meta.server_address, cyan);
				text("Size:");
				ImGui::SameLine();
				text_color(demo_size, cyan);
				text("\n");

				text("Version information:");

				text(demo_meta.version.get_summary());
			}
		}

		{
			auto scope = scoped_child("replay cancel");

			ImGui::Separator();

			{
				const bool result_good = demo_choice_result == D::OK || demo_choice_result == D::MIGHT_BE_INCOMPATIBLE;

				auto scope = maybe_disabled_cols({}, !result_good || demo_path.empty());

				if (ImGui::Button("Replay!")) {
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

	{
		auto child = scoped_child("connect view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		do_pretty_tabs(into_start.chosen_address_type);

		// auto& scope_cfg = into;

		if (into_start.chosen_address_type == connect_address_type::CUSTOM_ADDRESS) {
			base::acquire_keyboard_once();

			input_text<100>("Address (ipv4, ipv6, ipv4:port, hostname:port, [ipv6]:port)", custom_address);
		}
		else {
			auto& preferred = into_start.preferred_official_address;

			if (preferred.empty()) {
				if (official_arena_servers.size() > 0) {
					preferred = official_arena_servers[0];
				}
			}

			if (preferred.empty()) {
				text("Could not determine the best official server.", red);
			}
			else {
				auto p = preferred;
				input_text<100>("##Official", p, ImGuiInputTextFlags_ReadOnly);
				ImGui::SameLine();

				text_color("Best official server", yellow);
			}

			base::acquire_keyboard_once();
		}

		const auto label = typesafe_sprintf("Chosen nickname (%x-%x characters)", min_nickname_length_v, max_nickname_length_v);

		input_text(label, into_vars.nickname);

		struct loading_result {
			std::optional<std::string> new_path;
			augs::image loaded_image;
			std::string error_message;

			bool was_shrinked = false;
			bool will_be_upscaled = false;

			void load_image(const augs::path_type& from) {
				loaded_image.from_file(from);

				const auto max_s = static_cast<unsigned>(max_avatar_side_v);

				auto sz = loaded_image.get_size();

				if (sz != vec2u::square(max_s)) {
					sz.x = std::min(sz.x, max_s);
					sz.y = std::min(sz.x, max_s);

					if (sz != loaded_image.get_size()) {
						was_shrinked = true;
						loaded_image.scale(sz);
					}
					else {
						will_be_upscaled = true;
					}
				}

				const auto cached_file_path = USER_FILES_DIR "/cached_avatar.png";
				loaded_image.save_as_png(cached_file_path);

				new_path = cached_file_path;
			}
		};

		thread_local std::future<loading_result> avatar_loading_result;

		auto reload_avatar = [&](const std::string& from_path) {
			const bool avatar_upload_completed = augs::has_completed(current_frame, avatar_submitted_when);

			if (avatar_upload_completed && !avatar_loading_result.valid() && from_path.size() > 0) {
				avatar_loading_result = launch_async(
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
		};

		std::string p = into_vars.avatar_image_path.string();

		if (input_text<512>("Avatar image", p, ImGuiInputTextFlags_EnterReturnsTrue)) {
			reload_avatar(p);
		}

		ImGui::SameLine();

		if (ImGui::Button("Browse") && !error_popup && !mouse_has_to_move_off_browse) {
			mouse_has_to_move_off_browse = true;

			avatar_loading_result = launch_async(
				[&window]() {
					const std::vector<augs::window::file_dialog_filter> filters = { {
						"Image file",
						"*.png;*.jpg;*.jpeg;*.bmp;*.tga"
					} };

					loading_result out;
					out.new_path = window.open_file_dialog(filters, "Choose avatar image");

					if (out.new_path.has_value()) {
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

		if (!ImGui::IsItemHovered()) {
			mouse_has_to_move_off_browse = false;
		}

		if (p.size() > 0) {
			ImGui::SameLine();

			if (ImGui::Button("Clear") && !error_popup) {
				p = "";
				was_shrinked = false;
				will_be_upscaled = false;
			}
		}

		const auto first_size = vec2(max_avatar_side_v, max_avatar_side_v);
		const auto half_size = first_size / 2;
		const auto icon_size = vec2::square(22);

		if (p.size() > 0) {
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
			auto result = avatar_loading_result.get();

			const auto& error_msg = result.error_message;

			was_shrinked = false;
			will_be_upscaled = false;

			if (error_msg.size() > 0) {
				error_popup = simple_popup();
				error_popup->title = "Error";
				error_popup->message = error_msg;
			}
			else if (result.new_path.has_value()) {
				p = *result.new_path;

				avatar_preview_tex.texImage2D(renderer, std::move(result.loaded_image));
				avatar_preview_tex.set_filtering(renderer, augs::filtering_type::LINEAR);

				augs::graphics::texture::set_current_to_previous(renderer);
				avatar_submitted_when = current_frame;

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

		checkbox("Record demo", into_vars.demo_recording_path.is_enabled);

		if (into_start.chosen_address_type == connect_address_type::CUSTOM_ADDRESS) {
			text_disabled("It is best to use the server browser to connect to custom servers.\nThe server browser allows you to connect to servers behind routers.");
		}
		//text_disabled("Tip: to quickly connect, you can press Shift+C here or in the main menu,\ninstead of clicking \"Connect!\" with your mouse.");
	}

	{
		auto scope = scoped_child("connect cancel");

		ImGui::Separator();

		{
			auto scope = maybe_disabled_cols({}, !is_nickname_valid_characters(into_vars.nickname) || !::nickname_len_in_range(into_vars.nickname.length()));

			if (ImGui::Button("Connect!")) {
				clear_demo_choice();
				into_start.replay_demo.clear();

				if (into_start.chosen_address_type == connect_address_type::CUSTOM_ADDRESS) {
					into_start.displayed_connecting_server_name = "";
					into_start.custom_address = custom_address;
				}
				else if (into_start.chosen_address_type == connect_address_type::OFFICIAL) {
					into_start.displayed_connecting_server_name = into_start.preferred_official_address;
				}
				else if (into_start.chosen_address_type == connect_address_type::REPLAY) {
					into_start.displayed_connecting_server_name = "demo replay";
				}

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

