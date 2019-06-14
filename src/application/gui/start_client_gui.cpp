#include <thread>
#include <future>
#include "application/gui/start_client_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#include "augs/network/network_types.h"
#include "application/setups/editor/detail/maybe_different_colors.h"
#include "augs/templates/thread_templates.h"
#include "augs/window_framework/window.h"

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

		std::string p = into_vars.avatar_image_path;

		input_text<512>("Avatar image", p);

		ImGui::SameLine();

		thread_local std::future<std::optional<std::string>> new_avatar_path;
		thread_local const std::vector<augs::window::file_dialog_filter> filters = { {
			"PNG file",
			"*.png"
		} };


		if (ImGui::Button("Browse")) {
			new_avatar_path = std::async(
				std::launch::async,
				[&](){
					return window.open_file_dialog(filters, "Choose avatar image");
				}
			);
		}

		if (::valid_and_is_ready(new_avatar_path)) {
			if (const auto new_value = new_avatar_path.get()) {
				p = *new_value;
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

