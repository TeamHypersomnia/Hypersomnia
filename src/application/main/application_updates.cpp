#if BUILD_OPENSSL
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "augs/log.h"
#include "augs/string/typesafe_sscanf.h"
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

#include "3rdparty/cpp-httplib/httplib.h"

#if BUILD_OPENSSL
using client_type = httplib::SSLClient;
#else
using client_type = httplib::Client;
#endif

#include "hypersomnia_version.h"

using response_ptr = std::shared_ptr<httplib::Response>;

#if PLATFORM_UNIX
#define PLATFORM_STRING "Linux"
#define ARCHIVE_EXTENSION "tar.gz"
#elif PLATFORM_WINDOWS
#define PLATFORM_STRING "Windows"
#define ARCHIVE_EXTENSION "zip"
#else
#error "UNSUPPORTED!"
#endif

template <class... F>
decltype(auto) launch_download(client_type& client, const std::string& resource, F&&... args) {
	return client.Get(resource.c_str(), std::forward<F>(args)...);
}

using R = application_update_result_type;

application_update_result check_and_apply_updates(
	const augs::image& imgui_atlas_image,
	const http_client_settings& http_settings,
	augs::window_settings window_settings
) {
	using namespace augs::imgui;

	application_update_result result;

	const auto& host_url = http_settings.application_update_host;

#if BUILD_OPENSSL
	const auto port = 443;
#else
	const auto port = 80;
#endif

	client_type http_client(host_url.c_str(), port, http_settings.update_connection_timeout_secs);

#if BUILD_OPENSSL
	http_client.set_ca_cert_path("web/ca-bundle.crt");
	http_client.enable_server_certificate_verification(true);
#endif
	http_client.follow_location(true);

	const auto& update_path = http_settings.application_update_path;
	const auto version_path = typesafe_sprintf("%x/version-%x.txt", update_path, PLATFORM_STRING);

	auto log_null_response = [&http_client]() {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
		auto result = http_client.get_openssl_verify_result();

		if (result) {
			LOG("verify error: %x", X509_verify_cert_error_string(result));
		}
#endif

		LOG("Response was null!");
	};

	std::string new_version;

	{
		const auto response = launch_download(http_client, version_path); 

		if (response) {
			const auto& contents = response->body;

			std::string new_hash;
			typesafe_sscanf(contents, "%x\n%x", new_version, new_hash);

			const auto current_hash = hypersomnia_version().commit_hash;

			if (new_hash == current_hash) {
				LOG("The game is up to date. Commit hash: %x", current_hash);

				result.type = R::UP_TO_DATE;
				return result;
			}

			LOG("Commit hash differs. Requesting upgrade. \nOld: %x\nNew: %x", current_hash, new_hash);
		}
		else {
			log_null_response();
			result.type = R::FAILED;
			return result;
		}
	}

	const auto archive_path = typesafe_sprintf("%x/Hypersomnia-for-%x.%x", update_path, PLATFORM_STRING, ARCHIVE_EXTENSION);
	LOG_NVPS(version_path, archive_path);

	const auto win_bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	auto fix_background_color = scoped_style_color(ImGuiCol_WindowBg, ImVec4{win_bg.x, win_bg.y, win_bg.z, 1.f});

	const auto window_size = vec2i(500, 250);

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

	std::atomic<uint64_t> downloaded_bytes = 1;
	std::atomic<uint64_t> total_bytes = 1;
	std::atomic<bool> exit_requested = false;

	LOG("Launching download.");

	const auto archive_filename = augs::path_type(archive_path).filename();

	auto future_response = std::async(
		std::launch::async,
		[&exit_requested, archive_path, &http_client, &downloaded_bytes, &total_bytes]() {
			return launch_download(http_client, archive_path, [&](uint64_t len, uint64_t total) {
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

	auto interrupt = [&](const R r) {
		result.type = r;
		should_quit = true;
		exit_requested.store(true);

		LOG("Interrupting the updater due to: %x", static_cast<int>(r));
	};

	augs::timer frame_timer;

	while (!should_quit) {
		window.collect_entropy(entropy);

		for (const auto& e : entropy) {
			if (e.is_exit_message()) {
				interrupt(R::EXIT_APPLICATION);
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

						interrupt(R::UPGRADED);
					}
					else {
						log_null_response();
						interrupt(R::FAILED);
					}
				}

				const auto len = downloaded_bytes.load();
				const auto total = total_bytes.load();

				const auto downloaded_bytes = readable_bytesize(len, "%2f");
				const auto total_bytes = readable_bytesize(total, "%2f");

				const auto completion_mult = static_cast<double>(len) / total;
				const auto upstream_string = typesafe_sprintf("Mirror:     %x%x", host_url, update_path);


				{
					const auto current_version = hypersomnia_version().get_version_number();

					int major = 0;
					int minor = 0;
					int commit = 0;

					typesafe_sscanf(new_version, "%x.%x.%x", major, minor, commit);

					text(current_version);
					ImGui::SameLine();
					text("->");
					ImGui::SameLine();
					text(new_version);
					ImGui::SameLine();

					const auto num_revs = static_cast<int>(commit) - static_cast<int>(hypersomnia_version().commit_number);
					const auto revision_str = num_revs == 1 ? "revision" : "revisions";

					text_disabled(typesafe_sprintf("(%x new %x)\n\n", num_revs, revision_str));
				}

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
