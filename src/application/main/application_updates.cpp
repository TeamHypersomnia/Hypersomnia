#if BUILD_OPENSSL
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "3rdparty/cpp-httplib/httplib.h"

#include "augs/log.h"
#include "augs/templates/thread_templates.h"
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
#include "augs/misc/readable_bytesize.h"

#include "view/shader_paths.h"
#include "augs/readwrite/byte_file.h"

#if BUILD_OPENSSL
using client_type = httplib::SSLClient;
#else
using client_type = httplib::Client;
#endif

using response_ptr = std::shared_ptr<httplib::Response>;

template <class F>
decltype(auto) launch_download(client_type& client, const std::string& resource, F&& callback) {
	return client.Get(resource.c_str(), [&callback](uint64_t len, uint64_t total) {
		return callback(len, total);
	});
}

using R = application_update_result_type;

application_update_result check_and_apply_updates(
	const augs::image& imgui_atlas_image,
	const http_client_settings& http_settings,
	augs::window_settings window_settings
) {
	using namespace augs::imgui;

	const auto win_bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	auto fix_background_color = scoped_style_color(ImGuiCol_WindowBg, ImVec4{win_bg.x, win_bg.y, win_bg.z, 1.f});

	const auto window_size = vec2i(500, 220);

	window_settings.size = window_size;
	window_settings.fullscreen = false;
	window_settings.raw_mouse_input = false;
	window_settings.border = false;

	augs::window window(window_settings);
	const auto disp = window.get_display();

	window_settings.position = vec2i { disp.w / 2 - window_size.x / 2, disp.h / 2 - window_size.y / 2 };
	window.apply(window_settings);

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

#if PLATFORM_UNIX
	const auto platform_string = "Linux";
	const auto archive_extension = "tar.gz";
	(void)archive_extension;
#elif PLATFORM_WINDOWS
	const auto platform_string = "Windows";
	const auto archive_extension = "zip";
	(void)archive_extension;
#else
#error "UNSUPPORTED!"
#endif

	const auto& host_url = http_settings.application_update_host;
	const auto& update_path = http_settings.application_update_path;

	const auto hash_path = typesafe_sprintf("%x/commit_hash-%x.txt", update_path, platform_string);
	const auto archive_path = typesafe_sprintf("%x/Hypersomnia-for-%x.%x", update_path, platform_string, archive_extension);

	LOG_NVPS(hash_path, archive_path);

#if BUILD_OPENSSL
	const auto port = 443;
#else
	const auto port = 80;
#endif

	(void)port;
	client_type http_client(host_url.c_str(), port, http_settings.update_connection_timeout_secs);

#if BUILD_OPENSSL
	LOG("SSL BUILT!");
	http_client.set_ca_cert_path("web/ca-bundle.crt");
	http_client.enable_server_certificate_verification(true);
#endif
	http_client.follow_location(true);

	augs::timer frame_timer;

	std::atomic<uint64_t> downloaded_bytes = 1;
	std::atomic<uint64_t> total_bytes = 1;
	std::atomic<bool> exit_requested = false;

	LOG("Launching download.");

	const auto archive_filename = augs::path_type(archive_path).filename();

	auto future_response = std::async(
		std::launch::async,
		[&exit_requested, archive_path, &http_client, &downloaded_bytes, &total_bytes]() {
			return launch_download(http_client, archive_path, [&](const auto len, const auto total) {
				downloaded_bytes = len;
				total_bytes = total;

				if (exit_requested.load()) {
					return false;
				}

				return true;
			});
		}
	);

	LOG("Finished launching download.");

	bool should_quit = false;

	application_update_result result;

	auto interrupt = [&](const R r) {
		result.type = r;
		should_quit = true;
		exit_requested.store(true);

		LOG("Interrupting the updater due to: %x", static_cast<int>(r));
	};

	while (!should_quit) {
		window.collect_entropy(entropy);

		for (const auto& e : entropy) {
			if (e.is_exit_message()) {
				interrupt(R::EXIT);
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

		{
			const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
			auto loading_window = scoped_window("Loading in progress", nullptr, flags);

			{
				auto child = scoped_child("loading view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)), false, flags);

				text_color("New version available!", yellow);
				ImGui::Separator();

				if (valid_and_is_ready(future_response)) {
					const auto response = future_response.get();

					if (response) {
						LOG_NVPS(response->body.length());

						if (response->body.length() < 2000) {
							LOG(response->body);
						}

						augs::save_string_as_bytes(response->body, archive_filename);

						interrupt(R::SUCCESS);
					}
					else {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
						auto result = http_client.get_openssl_verify_result();
						if (result) {
							LOG("verify error: %x", X509_verify_cert_error_string(result));
						}
#endif

						LOG("Response was null!");

						interrupt(R::ERROR);
					}
				}

				const auto len = downloaded_bytes.load();
				const auto total = total_bytes.load();

				const auto downloaded_bytes = readable_bytesize(len, "%2f");
				const auto total_bytes = readable_bytesize(total, "%2f");

				const auto completion_mult = static_cast<double>(len) / total;

				//const auto percent = completion_mult * 100;
				//const auto percent_string = typesafe_sprintf("%x", percent) + " %";

				//const auto downloading_string = typesafe_sprintf("Acquiring %x...\n\n", archive_filename);
				//text(downloading_string);

				const auto upstream_string = typesafe_sprintf("Mirror:     %x%x", host_url, update_path);

				text_disabled(upstream_string);
				text("Acquiring: ");
				ImGui::SameLine();
				text_color(archive_filename, cyan);
				text("\n");

				const auto downloaded_string = typesafe_sprintf("Downloaded: %x", downloaded_bytes);
				const auto total_string =      typesafe_sprintf("Total size: %x\n\n", total_bytes);

				text(downloaded_string);
				text(total_string);

				ImGui::ProgressBar(static_cast<float>(completion_mult), ImVec2(-1.0f,0.0f));

				text("\n");
			}

			{
				auto scope = scoped_child("Cancel");

				ImGui::Separator();

				if (ImGui::Button("Cancel")) {
					interrupt(R::CANCELLED);
				}
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

	return result;
}
