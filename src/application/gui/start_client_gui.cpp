#include <thread>
#include <future>
#include "application/gui/start_client_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_status_icons.h"

#include "augs/network/network_types.h"
#include "augs/misc/imgui/maybe_different.h"
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
#include "application/setups/client/connect_target.h"
#include "application/main/favorite_servers.h"
#include "application/gui/browse_servers_gui.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

void start_client_gui_state::clear_demo_choice() {
	demo_choice_result = demo_choice_result_type::SHOULD_ANALYZE;
	demo_path.clear();
	demo_meta = {};
	demo_size = {};
}

bool start_client_gui_state::perform(
	const std::optional<server_list_entry>& best_server,
	const bool refresh_in_progress,
	augs::window& window,
	client_connect_string& into_connect_string,
	std::string& into_displayed_connecting_server_name,
	client_vars& into_vars,
	const bool streamer_mode,
	favorite_servers& favorites
) {
	if (!show) {
		return false;
	}
	
	using namespace augs::imgui;

#if PLATFORM_WEB
	(void)window;
#endif

#if WEB_LOWEND
	centered_size_mult = vec2(0.55f, 0.7f);
#else
	centered_size_mult = vec2(0.55f, 0.4f);
#endif
	
	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return false;
	}

	/*
		An unlisted server's "<address>,0x<key>" (see connect_target::find_server_identifiers) - from the clipboard
		when the window opens (the web build has no clipboard to read), or pasted into the address field.
	*/

	auto take_server_identifiers = [&](const std::string& text) {
		if (const auto found = connect_target::find_server_identifiers(text)) {
			custom_address = found->address;
			custom_key = "0x" + augs::bytes_to_hex(found->key);
			with_key = true;

			/*
				Whatever field was being edited takes the new values at once.
			*/

			augs::imgui::stop_editing_text();
			return true;
		}

		return false;
	};

#if !PLATFORM_WEB
	{
		/*
			A clipboard is taken once - the fields changed since stay until another one is copied.
		*/

		const bool focused_now = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		const bool game_focused_again = window.get_times_activated() != std::exchange(times_window_activated, window.get_times_activated());
		const bool focus_gained = (focused_now && !was_focused) || game_focused_again;
		const bool unlisted_tab_entered = current_tab == start_client_tab_type::CUSTOM_ADDRESS && tab_last_frame != current_tab;

		was_focused = focused_now;
		tab_last_frame = current_tab;

		if (ImGui::IsWindowAppearing() || focus_gained || unlisted_tab_entered) {
			if (const auto clipboard = ImGui::GetClipboardText(); clipboard != nullptr && clipboard != last_autopasted_clipboard) {
				if (take_server_identifiers(clipboard)) {
					last_autopasted_clipboard = clipboard;
					autopasted = std::make_pair(custom_address, custom_key);
					current_tab = start_client_tab_type::CUSTOM_ADDRESS;
				}
			}
		}
	}
#endif

	take_server_identifiers(custom_address);

	bool result = false;

	/*
		Unlisted server: its address and the key it must have (as Server details shows it, with 0x, or bare) -
		or the address only, verifying nothing.
	*/

	const auto typed_key = connect_target::key_of(custom_key);

	const auto custom_target = [&]() {
		auto out = connect_target::parse(custom_address);

		if (with_key) {
			out.key = typed_key;
		}

		return out;
	}();

	const bool custom_key_invalid = with_key && !typed_key.has_value();

	{
		auto child = scoped_child("connect view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		do_pretty_tabs(current_tab, [](const start_client_tab_type t) -> std::optional<std::string> {
			if (t == start_client_tab_type::CUSTOM_ADDRESS) {
				return "Unlisted server";
			}

			return std::nullopt;
		});

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

			/*
				IP with key or only IP: tabs like the window's, each a name and a dimmed note - the buttons draw no text
				of their own, the two colors are drawn over them.
			*/

			{
				static const char* const names[] = { "IP with key", "Only IP address" };
				static const char* const notes[] = { "(recommended)", "(insecure)" };
				static const char* labels[] = { "IP with key (recommended)", "Only IP address (insecure)" };

				const auto name_color = ImGui::GetColorU32(ImGuiCol_Text);
				const auto note_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);

				int index = with_key ? 0 : 1;

				{
					auto padding = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto p = ImGui::GetStyle().FramePadding; p.x *= 4; return p; }());
					auto no_text = scoped_style_color(ImGuiCol_Text, rgba(0, 0, 0, 0));

					ImGui::TabLabels(labels, 2, index, nullptr, [&](const int i) {
						const auto at = ImVec2(ImGui::GetItemRectMin().x + ImGui::GetStyle().FramePadding.x, ImGui::GetItemRectMin().y + ImGui::GetStyle().FramePadding.y);
						const auto note_x = at.x + ImGui::CalcTextSize(names[i]).x + ImGui::CalcTextSize(" ").x;

						ImGui::GetWindowDrawList()->AddText(at, name_color, names[i]);
						ImGui::GetWindowDrawList()->AddText(ImVec2(note_x, at.y), note_color, notes[i]);
					});
				}

				with_key = index == 0;

				auto separator = scoped_style_color(ImGuiCol_Separator, ImGui::GetStyle().Colors[ImGuiCol_Button]);
				ImGui::Separator();
			}

			/*
				A click into a field selects all of it - to paste over at once.
			*/

			const auto flags = ImGuiInputTextFlags_AutoSelectAll | (streamer_mode ? ImGuiInputTextFlags_Password : 0);

#if !PLATFORM_WEB
			/*
				"<address>,0x<key>" pasted into either field fills both - before the field takes it (the key's would cut
				it short).
			*/

			if (const auto& io = ImGui::GetIO(); (io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_V, false)) {
				if (const auto clipboard = ImGui::GetClipboardText(); clipboard != nullptr) {
					take_server_identifiers(clipboard);
				}
			}
#endif

			input_text<300>("Address (ipv4, ipv6, ipv4:port, hostname:port, [ipv6]:port)", custom_address, flags);

			/*
				One line under the fields for what is wrong - always there, so the layout never jumps.
			*/

			if (with_key) {
				/*
					The key on lines of its own, as many as it takes to show all of it (a key has no spaces to wrap at -
					it is broken into lines as wide as the field; the breaks are never part of the key).
				*/

				text("Public key (64 hex digits)");

				{
					const auto& style = ImGui::GetStyle();
					const auto field_width = ImGui::GetContentRegionAvail().x;
					const auto char_width = ImGui::CalcTextSize("0").x;
					const auto per_line = std::max(std::size_t(8), static_cast<std::size_t>((field_width - 2 * style.FramePadding.x - char_width) / char_width));

					auto shown = std::string();

					for (std::size_t i = 0; i < custom_key.size(); i += per_line) {
						if (i > 0) {
							shown += '\n';
						}

						shown += custom_key.substr(i, per_line);
					}

					const auto lines = std::max(std::size_t(1), (custom_key.size() + per_line - 1) / per_line);
					const auto height = lines * ImGui::GetTextLineHeight() + 2 * style.FramePadding.y;

					std::array<char, 200> buf {};
					std::copy_n(shown.data(), std::min(shown.size(), buf.size() - 1), buf.data());

					if (ImGui::InputTextMultiline("##custom_key", buf.data(), buf.size(), ImVec2(field_width, height), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll)) {
						auto edited = std::string(buf.data());
						std::erase(edited, '\n');
						custom_key = edited.substr(0, 66);
					}
				}

				/*
					The line under the fields: what is wrong, else that they came from the clipboard (until one is
					changed) - kept even when empty, so the layout never jumps.
				*/

				const bool key_error = !custom_key.empty() && custom_key_invalid;
				const bool as_autopasted = autopasted.has_value() && autopasted->first == custom_address && autopasted->second == custom_key;

				if (key_error) {
					text_color("The key should be 64 hex digits.", orange);
				}
				else if (as_autopasted) {
					checkmark_line("Auto-pasted from clipboard", green);
				}
				else {
#if PLATFORM_WEB
					text(" ");
#else
					text_disabled("\"ip,key\" in clipboard auto-fills these.");
#endif
				}

				ImGui::Separator();

				/*
					A favorite added by hand - the address and the key as given (see favorite_servers.h).
				*/

				if (favorites_kept) {
					const auto address = connect_target::trimmed(custom_address);

					if (typed_key.has_value() && favorites.contains_manual(*typed_key, address)) {
						text_color("Already in favorites", green);
					}
					else {
						const bool can_add = !address.empty() && typed_key.has_value();

						{
							auto scope = maybe_disabled_cols({}, !can_add);

							if (ImGui::Button("Add to favorites") && can_add) {
								favorites.add_manual(*typed_key, address, std::string());
							}
						}

						ImGui::SameLine();
						text_disabled("To quickly connect in the future");
					}
				}
			}
			else {
				text_color("Not verified - anyone at this address could answer.", orange);
				ImGui::Separator();
			}
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

		}

		if (current_tab != start_client_tab_type::REPLAY_DEMO) {
			/*
				The nickname and the avatar are in Settings - Account.
			*/

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
				&& (custom_address.empty() || custom_key_invalid)
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
							into_connect_string = best_server->get_join_string();
							into_displayed_connecting_server_name = best_server->heartbeat.server_name;
						}

						break;
					case start_client_tab_type::CUSTOM_ADDRESS:
						into_connect_string = custom_target.to_string();
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

