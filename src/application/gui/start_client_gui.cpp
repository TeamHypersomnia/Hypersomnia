#include <thread>
#include <future>
#include "application/gui/start_client_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#include "augs/network/network_types.h"
#include "application/setups/editor/gui/property_editor/detail/maybe_different_colors.h"
#include "augs/templates/thread_templates.h"
#include "augs/window_framework/window.h"

#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/filesystem/file.h"

#include "augs/misc/imgui/imgui_enum_radio.h"
#include "application/gui/demo_chooser.h"
#include "application/setups/client/demo_file.h"
#include "application/gui/pretty_tabs.h"
#include "augs/readwrite/byte_readwrite.h"
#include "steam_integration.h"
#include "steam_integration_helpers.hpp"
#include "application/gui/server_list_entry.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

void start_client_gui_state::clear_demo_choice() {
	demo_choice_result = demo_choice_result_type::SHOULD_ANALYZE;
	demo_path.clear();
	demo_meta = {};
	demo_size = {};
}

struct avatar_loading_result_info {
	std::optional<augs::path_type> new_path;
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

		loaded_image.save_as_png(CACHED_AVATAR);
		new_path = CACHED_AVATAR;
	}
};

bool start_client_gui_state::perform(
	const std::optional<server_list_entry>& best_server,
	const bool refresh_in_progress,
	const augs::frame_num_type current_frame,
	augs::renderer& renderer,
	augs::graphics::texture& avatar_preview_tex,
	augs::window& window,
	client_connect_string& into_connect_string,
	std::string& into_displayed_connecting_server_name,
	client_vars& into_vars,
	const bool streamer_mode
) {
	if (!show) {
		return false;
	}
	
	using namespace augs::imgui;

#if WEB_LOWEND
	centered_size_mult = vec2(0.55f, 0.7f);
#else
	centered_size_mult = vec2(0.55f, 0.4f);
#endif
	
	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return false;
	}

	bool result = false;

	{
		auto child = scoped_child("connect view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		do_pretty_tabs(current_tab);

		// auto& scope_cfg = into;

		if (current_tab == start_client_tab_type::REPLAY_DEMO) {
			using D = demo_choice_result_type;

			thread_local demo_chooser chooser;

			// text_disabled(typesafe_sprintf("The demos are stored inside \"%x\" folder.", DEMOS_DIR));

			chooser.perform(
				"##replay_demo_chooser",
				demo_path.string(),
				augs::path_type(DEMOS_DIR),
				[&](const auto& new_choice) {
					demo_path = new_choice.path;
					demo_choice_result = D::SHOULD_ANALYZE;
				}
			);

#if !PLATFORM_WEB
			{
				auto scope = maybe_disabled_cols({}, demo_path.empty());

				if (ImGui::Button("Demo in explorer")) {
					window.reveal_in_explorer(demo_path);
				}

				ImGui::SameLine();
			}

			if (ImGui::Button("Demo folder in explorer")) {
				window.reveal_in_explorer(DEMOS_DIR);
			}
#endif

			if (!demo_path.empty() && demo_choice_result == D::SHOULD_ANALYZE) {
				try {
					auto t = augs::open_binary_input_stream(demo_path);

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
			else {
				text(typesafe_sprintf("Choose demo to replay (from %x).", DEMOS_DIR));
			}
		}
		else if (current_tab == start_client_tab_type::CUSTOM_ADDRESS) {
			base::acquire_keyboard_once();

			const auto flags = streamer_mode ? ImGuiInputTextFlags_Password : 0; 
			input_text<100>("Address (ipv4, ipv6, ipv4:port, hostname:port, [ipv6]:port)", custom_address, flags);
		}
		else if (current_tab == start_client_tab_type::BEST_SERVER) {
			if (!best_server.has_value()) {
				auto cs = scoped_style_color(ImGuiCol_FrameBg, rgba(255,255,255,0));

				auto sname = std::string("Finding the best server...");
				input_text<max_server_name_length_v>("##BestServer", sname, ImGuiInputTextFlags_ReadOnly);
			}
			else {
				{
					thread_local bool hovered = false;

					auto cs = std::make_tuple(
						cond_scoped_style_color(hovered, ImGuiCol_FrameBg, rgba(255,255,255,60)),
						cond_scoped_style_color(!hovered, ImGuiCol_FrameBg, rgba(255,255,255,20))
					);

					auto sname = std::string(best_server->heartbeat.server_name);
					//ImGui::Selectable( "huh t", false, 0, { ImGui::CalcItemWidth(), 0.0f });

					input_text<max_server_name_length_v>("##BestServer", sname, ImGuiInputTextFlags_ReadOnly);

					hovered = ImGui::IsItemHovered();

					if (hovered) {
						text_tooltip("Show in Server Browser");
					}

					if (ImGui::IsItemActivated()) {
						ImGui::ClearActiveID();
						request_server_list_open = true;
					}
				}	

			}

			ImGui::SameLine();

			{
				auto curs = scoped_preserve_cursor();

				text_color("Players:", green);
				ImGui::SameLine();
				text(best_server.has_value() ? std::to_string(best_server->heartbeat.num_online) : std::string("?"));

				ImGui::SameLine();

				text_color("Ping:", cyan);
				ImGui::SameLine();

				//const auto col = best_server->progress.get_ping_color();

#if PLATFORM_WEB
				text_disabled("?");
#else
				text(best_server.has_value() ? best_server->progress.get_ping_string() : std::string("?"));
#endif
			}

			text_color("Players: 99  Ping: 999", rgba(0,0,0,0));
			ImGui::SameLine();

			{
				auto scope = maybe_disabled_cols({}, refresh_in_progress);

				if (ImGui::Button("Refresh")) {
					request_refresh_best_server = true;
				}
			}

#if PLATFORM_WEB
			const bool focus_nickname_edit = false;
#else
			const bool focus_nickname_edit = !is_steam_client;
#endif

			if (focus_nickname_edit) {
				base::acquire_keyboard_once();
			}
		}

		if (current_tab != start_client_tab_type::REPLAY_DEMO) {
			bool nickname_choice_active = true;

			auto signed_in_name = into_vars.signed_in_nickname();

			if (!signed_in_name.empty()) {
				nickname_choice_active = false;
			}

			if (is_steam_client && into_vars.use_account_nickname) {
				nickname_choice_active = false;
			}

			auto label = typesafe_sprintf("Chosen nickname (%x-%x characters)", min_nickname_length_v, max_nickname_length_v);

			if (is_steam_client) {
				label = "##NicknameInactive";
			}

			{
				auto scope = maybe_disabled_with_clear_text_cols({}, !nickname_choice_active);

				if (!signed_in_name.empty()) {
					input_text(label, signed_in_name);
				}
				else {
					input_text(label, into_vars.nickname);
				}
			}

			if (is_steam_client) {
				ImGui::SameLine();

				if (checkbox("Use Steam nickname", into_vars.use_account_nickname)) {
					if (into_vars.use_account_nickname) {
						if (const auto steam_username = ::steam_get_username()) {
							into_vars.nickname = std::string(steam_username);
						}
					}
				}
			}

			thread_local augs::future<avatar_loading_result_info> avatar_loading_result;

			auto reload_avatar = [&](const augs::path_type& from_path) {
				const bool avatar_upload_completed = augs::has_completed(current_frame, avatar_submitted_when);

				if (avatar_upload_completed && !avatar_loading_result.valid() && !from_path.empty()) {
					avatar_loading_result = launch_async(
						[from_path]() {
							avatar_loading_result_info out;
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

			augs::path_type p = into_vars.avatar_image_path;
			const auto first_size = vec2(max_avatar_side_v, max_avatar_side_v);
			const auto half_size = first_size / 2;
			const auto icon_size = vec2::square(22);

			if (!p.empty()) {
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

#if PLATFORM_WEB
			bool avatar_choice_active = false;
#else
			bool avatar_choice_active = true;
#endif

			if (is_steam_client) {
				if (checkbox("Use Steam avatar", into_vars.use_account_avatar)) {
					if (into_vars.use_account_avatar) {
						if (const auto avatar = ::steam_get_avatar_image(); avatar.get_size().is_nonzero()) {
							avatar.save_as_png(CACHED_AVATAR);

							p = CACHED_AVATAR;
							reload_avatar(p);
						}
					}
				}

				avatar_choice_active = !into_vars.use_account_avatar;
			}

			if (avatar_choice_active) {
				if (is_steam_client) {
					ImGui::SameLine();
				}

				if (ImGui::Button("Browse Avatar") && !error_popup && !mouse_has_to_move_off_browse) {
					mouse_has_to_move_off_browse = true;

					avatar_loading_result = launch_async(
						[&window]() {
							const std::vector<augs::window::file_dialog_filter> filters = { {
								"Image file",
								"*.png;*.jpg;*.jpeg;*.bmp;*.tga"
							} };

							avatar_loading_result_info out;
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

				if (!p.empty()) {
					ImGui::SameLine();

					if (ImGui::Button("Clear") && !error_popup) {
						p = "";
						was_shrinked = false;
						will_be_upscaled = false;
					}
				}

	#if PLATFORM_LINUX
				/* In case the file browser doesn't open on Linux */

				ImGui::SameLine();

				text("Path");

				ImGui::SameLine();

				if (input_text<512>("(Enter)", p, ImGuiInputTextFlags_EnterReturnsTrue)) {
					reload_avatar(p);
				}
	#endif
			}

			const auto size_str = typesafe_sprintf("%xx%x", first_size.x, first_size.y);

			const bool show_tip = false;

			if (show_tip) {
				if (was_shrinked) {
					text_disabled(typesafe_sprintf("The chosen image was automatically shrinked to %x.\nTo ensure the best quality,\nsupply an image cropped to exactly %x.\n\n", size_str, size_str));
				}
				else if (will_be_upscaled) {
					text_disabled(typesafe_sprintf("The chosen image will be automatically upscaled to %x.\nTo ensure the best quality,\nsupply an image cropped to exactly %x.\n\n", size_str, size_str));
				}
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

			checkbox("Record demo", into_vars.record_demo);
			ImGui::SameLine();
			text_disabled("You can later rewatch your match in Replay tab.");

			if (const bool show_browser_tip = false) {
				if (current_tab == start_client_tab_type::CUSTOM_ADDRESS) {
					text_disabled("It is best to use the server browser to connect to custom servers.\nThe server browser allows you to connect to servers behind routers.");
				}
			}

			//text_disabled("Tip: to quickly connect, you can press Shift+C here or in the main menu,\ninstead of clicking \"Connect!\" with your mouse.");
		}
	}

	{
		auto scope = scoped_child("connect cancel");

		ImGui::Separator();

		if (current_tab == start_client_tab_type::REPLAY_DEMO) {
			using D = demo_choice_result_type;

			const bool result_good = demo_choice_result == D::OK || demo_choice_result == D::MIGHT_BE_INCOMPATIBLE;

			auto scope = maybe_disabled_cols({}, !result_good || demo_path.empty());

			if (ImGui::Button("Replay!")) {
				result = true;
				into_connect_string = typesafe_sprintf("%x%x", demo_address_preffix_v, demo_path);
				into_displayed_connecting_server_name = demo_path.filename().string();
				//show = false;
			}
		}
		else {
			const bool custom_addr_empty = 
				current_tab == start_client_tab_type::CUSTOM_ADDRESS
				&& custom_address.empty()
			;

			const bool best_still_unresolved = 
				current_tab == start_client_tab_type::BEST_SERVER
				&& best_server == std::nullopt
			;

			auto scope = maybe_disabled_cols({}, best_still_unresolved || custom_addr_empty || !is_nickname_valid_characters(into_vars.nickname) || !::nickname_len_in_range(into_vars.nickname.length()));

			if (ImGui::Button("Connect!")) {
				switch (current_tab) {
					case start_client_tab_type::BEST_SERVER:
						if (best_server) {
							into_connect_string = std::string(best_server->get_my_connect_string());
							into_displayed_connecting_server_name = best_server->heartbeat.server_name;
						}

						break;
					case start_client_tab_type::CUSTOM_ADDRESS:
						into_connect_string = custom_address;
						break;
					default:
						break;
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

