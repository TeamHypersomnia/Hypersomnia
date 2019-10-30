#include "3rdparty/cpp-httplib/httplib.h"

#include "augs/log.h"
#include "augs/math/matrix.h"
#include "application/main/application_updates.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/renderer_backend.h"
#include "augs/misc/timing/timer.h"
#include "augs/window_framework/window.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#include "augs/graphics/shader.h"
#include "augs/graphics/texture.h"

#include "view/shader_paths.h"

application_update_result check_and_apply_updates(
	const augs::image& imgui_atlas_image,
	const std::string& url,
	augs::window_settings settings
) {
	using namespace augs::imgui;
	(void)url;

	const auto win_bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	auto fix_background_color = scoped_style_color(ImGuiCol_WindowBg, ImVec4{win_bg.x, win_bg.y, win_bg.z, 1.f});

	const auto window_size = vec2i(500, 150);

	settings.size = window_size;
	settings.fullscreen = false;
	settings.raw_mouse_input = false;
	settings.border = false;

	augs::window window(settings);
	const auto disp = window.get_display();

	settings.position = vec2i { disp.w / 2 - window_size.x / 2, disp.h / 2 - window_size.y / 2 };
	window.apply(settings);

	augs::graphics::renderer_backend renderer_backend;
	augs::graphics::renderer_backend::result_info renderer_backend_result;

	const auto imgui_atlas = augs::graphics::texture(imgui_atlas_image);

	augs::local_entropy entropy;

	augs::renderer renderer;

	std::optional<augs::graphics::shader_program> standard;

	LOG("Initializing the standard shader.");

	try {
		const auto canon_vsh_path = typesafe_sprintf("%x/%x.vsh", CANON_SHADER_FOLDER, "standard");
		const auto canon_fsh_path = typesafe_sprintf("%x/%x.fsh", CANON_SHADER_FOLDER, "standard");

		standard.emplace(
			canon_vsh_path,
			canon_fsh_path
		);
	}
	catch (const augs::graphics::shader_error& err) {
		(void)err;
	}

	if (standard != std::nullopt) {
		standard->set_as_current(renderer);
		standard->set_projection(renderer, augs::orthographic_projection(vec2(window_size)));
	}

	renderer.set_viewport({ vec2i{0, 0}, window_size });

	augs::timer frame_timer;

	float cntr = 0.f;

	while (cntr < 1.f) {
		window.collect_entropy(entropy);

		for (const auto& e : entropy) {
			if (e.is_exit_message()) {
				application_update_result result;
				result.type = application_update_result_type::EXIT;
				return result;
			}
		}

		const auto frame_delta = frame_timer.extract_delta();
		const auto dt_secs = frame_delta.in_seconds();

		augs::imgui::setup_input(
			entropy,
			dt_secs,
			window_size
		);

		ImGui::NewFrame();
		center_next_window(1.f, ImGuiCond_Always);

		cntr += dt_secs;

		{
			auto loading_window = scoped_window("Loading in progress", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

			text_color("Upgrading Hypersomnia...", yellow);
			ImGui::Separator();

			if (cntr >= 0.f) {
				ImGui::ProgressBar(cntr, ImVec2(-1.0f,0.0f));
			}
		}

		augs::imgui::render();

		renderer.clear_current_fbo();
		renderer.draw_call_imgui(
			imgui_atlas, 
			nullptr, 
			nullptr, 
			nullptr
		);

		renderer_backend_result.clear();

		{
			auto& r = renderer;

			renderer_backend.perform(
				renderer_backend_result,
				r.commands.data(),
				r.commands.size(),
				r.dedicated
			);
		}

		for (const auto& f : renderer_backend_result.imgui_lists_to_delete) {
			IM_DELETE(f);
		}

		window.swap_buffers();

		renderer.next_frame();
	}

	application_update_result result;
	result.type = application_update_result_type::SUCCESS;
	return result;
}
