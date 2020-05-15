#if PLATFORM_UNIX
#include <csignal>
#endif

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
#include "augs/filesystem/directory.h"
#include "application/main/new_and_old_hypersomnia_path.h"
#include "application/main/extract_archive.h"

#if BUILD_OPENSSL
using client_type = httplib::SSLClient;
#else
using client_type = httplib::Client;
#endif

#include "hypersomnia_version.h"

using response_ptr = std::shared_ptr<httplib::Response>;

#if USE_GLFW
#define PLATFORM_STRING "MacOS"
#define ARCHIVE_EXTENSION "sfx"
#elif PLATFORM_UNIX
#define PLATFORM_STRING "Linux"
#define ARCHIVE_EXTENSION "sfx"
#elif PLATFORM_WINDOWS
#define PLATFORM_STRING "Windows"
#define ARCHIVE_EXTENSION "exe"
#else
#error "UNSUPPORTED!"
#endif

#if PLATFORM_UNIX
static std::atomic<int> signal_status = 0;
#endif

static auto get_first_folder_in(const augs::path_type& where) {
	augs::path_type result;

	augs::for_each_in_directory(
		where,
		[&result](const auto& dir) {
			result = dir;
			return callback_result::ABORT;
		},
		[](const auto&) {
			return callback_result::CONTINUE;
		}
	);

	return result;
}

bool successful(const int http_status_code) {
	return http_status_code >= 200 && http_status_code < 300;
}

template <class... F>
decltype(auto) launch_download(client_type& client, const std::string& resource, F&&... args) {
	return client.Get(resource.c_str(), std::forward<F>(args)...);
}

using R = application_update_result_type;
namespace fs = std::filesystem;

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
#else
		(void)http_client;
#endif

		LOG("Response was null!");
	};

	std::string new_version;

	{
		const auto response = launch_download(http_client, version_path); 

		if (response == nullptr) {
			log_null_response();
			result.type = R::FAILED;
			return result;
		}

		if (response->status == 404 || response->status == 403) {
			result.type = R::VERSION_FILE_NOT_FOUND;
			return result;
		}

		if (!successful(response->status)) {
			LOG("Request failed with status: %x", response->status);
			result.type = R::FAILED;
			return result;
		}

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

	const auto archive_path = typesafe_sprintf("%x/Hypersomnia-for-%x.%x", update_path, PLATFORM_STRING, ARCHIVE_EXTENSION);
	LOG_NVPS(version_path, archive_path);

	const auto win_bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	auto fix_background_color = scoped_style_color(ImGuiCol_WindowBg, ImVec4{win_bg.x, win_bg.y, win_bg.z, 1.f});

	const auto line_height = 24;
	const auto num_lines = 13;

	const auto window_size = vec2i(500, line_height * num_lines);

	window_settings.size = window_size;
	window_settings.fullscreen = false;
	window_settings.raw_mouse_input = false;
	window_settings.border = false;

	std::optional<augs::window> window;
	std::optional<augs::graphics::renderer_backend> renderer_backend;
	std::optional<augs::graphics::texture> imgui_atlas;
	std::optional<augs::graphics::shader_program> standard;

	augs::renderer renderer;

	try {
		window.emplace(window_settings);
		const auto disp = window->get_display();

		window_settings.position = vec2i { disp.w / 2 - window_size.x / 2, disp.h / 2 - window_size.y / 2 };
		window->apply(window_settings);

		renderer_backend.emplace();
		imgui_atlas.emplace(imgui_atlas_image);

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
	}
	catch (const augs::window_error& err) {
		LOG("Failed to open the window:\n%x", err.what());
		LOG("Updating in headless mode.");
	}

	renderer_backend_result rendering_result;

	augs::local_entropy entropy;

	renderer.set_viewport({ vec2i{0, 0}, window_size });

	std::atomic<uint64_t> downloaded_bytes = 1;
	std::atomic<uint64_t> total_bytes = 1;
	std::atomic<bool> exit_requested = false;

	LOG("Launching download.");

	const auto archive_filename = augs::path_type(archive_path).filename();
	const auto target_archive_path = GENERATED_FILES_DIR / archive_filename;

	const auto NEW_path = augs::path_type(NEW_HYPERSOMNIA);
	const auto OLD_path = augs::path_type(OLD_HYPERSOMNIA);

	auto future_response = launch_async(
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

	enum class state {
		DOWNLOADING,
		EXTRACTING,
		SAVING_ARCHIVE_TO_DISK,
		MOVING_FILES_AROUND
	};

	auto current_state = state::DOWNLOADING;
	auto extractor = std::optional<archive_extractor>();
	//auto mover = std::optional<updated_files_mover>();
	auto completed_move = std::future<callback_result>();
	auto completed_save = std::future<callback_result>();

	double total_secs = 0;

	auto mbox_guarded_action = [&window](auto&& action, const auto& action_name, const auto&... path) {
		for (;;) {
			try {
				action();
				return callback_result::CONTINUE;
			}
			catch (const augs::file_open_error& err) {
				const auto format = "Cannot %x:\n%x\n\nPlease close the applications that might be using it, then try again!\n\nDetails:\n\n%x";
				const auto error_text = typesafe_sprintf(format, action_name, path..., err.what());

				if (window == std::nullopt) {
					LOG(error_text);
					return callback_result::ABORT;
				}

				if (window->retry_cancel("Hypersomnia Updater", error_text) == augs::message_box_button::CANCEL) {
					return callback_result::ABORT;
				}
			}
			catch (const fs::filesystem_error& err) {
				const auto format = [&]() {
					if (err.path2().empty()) {
						return "Cannot %x:\n%x\n\nPlease close the applications that might be using it, then try again!\n\nDetails:\n\n%x";
					}
					else {
						return "Cannot %x:\n%x:\nto:\n%x\n\nPlease close the applications that might be using them, then try again!\n\nDetails:\n\n%x";
					}
				}();

				const auto error_text = [&]() {
					if (err.path2().empty()) {
						return typesafe_sprintf(format, action_name, err.path1(), err.what());
					}
					
					return typesafe_sprintf(format, action_name, err.path1(), err.path2(), err.what());
				}();

				if (window == std::nullopt) {
					LOG(error_text);
					return callback_result::ABORT;
				}

				if (window->retry_cancel("Hypersomnia Updater", error_text) == augs::message_box_button::CANCEL) {
					return callback_result::ABORT;
				}
			}
		}
	};

	auto rm_rf = [&mbox_guarded_action](const auto& p) {
		auto do_remove = [&]() {
			LOG("rm -rf %x", p);
			fs::remove_all(p);
		};

		return mbox_guarded_action(do_remove, "remove");
	};
	
	auto mkdir_p = [&mbox_guarded_action](const auto& p) {
		auto do_mkdir = [&]() {
			LOG("mkdir -p %x", p);

			augs::create_directories(p);
		};

		return mbox_guarded_action(do_mkdir, "create directory");
	};

	auto mv = [&mbox_guarded_action](const auto& a, const auto& b) {
		auto do_rename = [&]() {
			LOG("mv %x %x", a, b);

			fs::rename(a, b);
		};

		return mbox_guarded_action(do_rename, "move");
	};

	auto make_executable = [&mbox_guarded_action](const auto& p) {
		auto do_mkexe = [&]() {
			LOG("chmod +x %x", p);
			
			fs::permissions(p, fs::perms::owner_all, fs::perm_options::add);
		};

		return mbox_guarded_action(do_mkexe, "make executable of");
	};

	auto guarded_save_as_bytes = [&](const std::string& contents, const augs::path_type& target) {
		auto saver = [&]() {
			LOG("Saving the executable as bytes.");
			augs::save_string_as_bytes(contents, target);
		};
		
		return mbox_guarded_action(saver, "save", target);
	};

#define TEST 0
#if TEST
	rm_rf(NEW_path);
	mkdir_p(NEW_path);

	make_executable(target_archive_path);
	extractor.emplace(target_archive_path, NEW_path, exit_requested);
	current_state = state::EXTRACTING;
#endif

	auto print_new_version_available = []() {
		text_color("New version available!", yellow);

		ImGui::Separator();
	};
	
	auto print_version_transition_info = [&new_version, total_secs]() {
		const auto version_info_string = [&]() {
			const auto current_version = hypersomnia_version().get_version_number();

			int major = 0;
			int minor = 0;
			int commit = 0;

			typesafe_sscanf(new_version, "%x.%x.%x", major, minor, commit);

			text(current_version);
			ImGui::SameLine();

			const auto ms = total_secs * 1000;
			const auto n = int(ms / 200);

			if (n % 2 == 0) {
				text("->");
			}
			else {
				text(">-");
			}

			ImGui::SameLine();
			text(new_version);
			ImGui::SameLine();

			const auto num_revs = static_cast<int>(commit) - static_cast<int>(hypersomnia_version().commit_number);
			const auto revision_str = num_revs == 1 ? "revision" : "revisions";

			return typesafe_sprintf("(%x new %x)\n\n", num_revs, revision_str);
		}();

		text_disabled(version_info_string);
	};

	auto print_upstream = [&host_url, &update_path]() {
		text_disabled("Mirror:");
		ImGui::NextColumn();
		const auto upstream_string = typesafe_sprintf("%x%x", host_url, update_path);
		text_disabled(upstream_string);
		ImGui::NextColumn();
	};

	auto print_download_progress_bar = [&downloaded_bytes, &total_bytes, &archive_filename]() {
		const auto len = downloaded_bytes.load();
		const auto total = total_bytes.load();

		const auto downloaded_bytes = readable_bytesize(len, "%2f");
		const auto total_bytes = readable_bytesize(total, "%2f");

		const auto completion_mult = static_cast<double>(len) / total;

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, ImGui::CalcTextSize("DDDownloaded:").x);

		text("Acquiring:");
		ImGui::NextColumn();
		text_color(archive_filename.string(), cyan);

		ImGui::NextColumn();
		text("\n");

		const auto downloaded_string = typesafe_sprintf("%x", downloaded_bytes);
		const auto total_string =      typesafe_sprintf("%x", total_bytes);

		text("Downloaded:");
		ImGui::NextColumn();
		text("\n");
		text(downloaded_string);
		ImGui::NextColumn();

		text("Total size:");
		ImGui::NextColumn();
		text(total_string);
		ImGui::NextColumn();
		ImGui::Columns(1);

		text("\n");

		ImGui::ProgressBar(static_cast<float>(completion_mult), ImVec2(-1.0f,0.0f));

		text("\n");
	};

	auto print_saving_progress = [&archive_filename]() {
		text("Saving:    ");
		ImGui::SameLine();
		text_color(archive_filename.string(), cyan);
	};

	auto print_extracting_progress = [&archive_filename, &extractor]() {
		text("Extracting:");

		ImGui::SameLine();
		text_color(archive_filename.string(), cyan);
		text("\n");

		auto info = extractor->get_info();

		if (info.processed.size() > 0) {
			text("Processing file:");
		}
		else {
			text("");
		}

		auto p = augs::path_type(info.processed);
		auto displayed_path = p;

		if (std::distance(p.begin(), p.end()) > 1) {
			displayed_path = augs::path_type();

			for (auto it = std::next(p.begin()); it != p.end(); ++it) {
				displayed_path = displayed_path / *it;
			}
		}

		text(displayed_path.string());

		ImGui::ProgressBar(static_cast<float>(info.percent) / 100, ImVec2(-1.0f,0.0f));
	};

	auto do_cancel_button = []() {
		auto scope = scoped_child("Cancel");

		ImGui::Separator();

		return ImGui::Button("Cancel");
	};

	auto advance_update_logic = [&]() {
		const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		auto loading_window = scoped_window("Loading in progress", nullptr, flags);

		{
			auto child = scoped_child("loading view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)), false, flags);

			print_new_version_available();
			print_version_transition_info();
			ImGui::Columns(2);
			print_upstream();
			ImGui::Columns(1);

			if (current_state == state::DOWNLOADING) {
				if (valid_and_is_ready(future_response)) {
					auto response = future_response.get();

					if (response == nullptr) {
						log_null_response();
						interrupt(R::FAILED);
						return;
					}

					if (!successful(response->status)) {
						LOG("Request failed with status: %x", response->status);

						if (response->status == 404 || response->status == 403) {
							interrupt(R::BINARY_NOT_FOUND);
							return;
						}

						interrupt(R::FAILED);
						return;
					}

					LOG_NVPS(response->body.length());

					if (response->body.length() < 2000) {
						LOG(response->body);
					}

					completed_save = launch_async(
						[guarded_save_as_bytes, make_executable, rm_rf, mkdir_p, resp_body = std::move(response->body), target_archive_path, NEW_path]() {
							auto failed = [](const auto e) {
								return e == callback_result::ABORT;
							};

							if (failed(guarded_save_as_bytes(resp_body, target_archive_path))) {
								return callback_result::ABORT;
							}
							
							if (failed(make_executable(target_archive_path))) {
								return callback_result::ABORT;
							}
							
							if (failed(rm_rf(NEW_path))) {
								return callback_result::ABORT;
							}

							if (failed(mkdir_p(NEW_path))) {
								return callback_result::ABORT;
							}
							
							return callback_result::CONTINUE;
						}
					);

					current_state = state::SAVING_ARCHIVE_TO_DISK;
				}

				print_download_progress_bar();
			}
			else if (current_state == state::SAVING_ARCHIVE_TO_DISK) {
				if (valid_and_is_ready(completed_save)) {
					if (completed_save.get() == callback_result::CONTINUE) {
						extractor.emplace(target_archive_path, NEW_path, exit_requested);
						current_state = state::EXTRACTING;
						LOG("Extracting.");
					}
					else {
						interrupt(R::CANCELLED);
					}
				}

				print_saving_progress();
			}
			else if (current_state == state::EXTRACTING) {
				if (extractor->has_completed()) {
#if 0
					interrupt(R::UPGRADED);
					return;
#endif
					result.exit_with_failure_if_not_upgraded = true;

					/* Serious stuff begins here. */


					LOG("Moving files around.");
					current_state = state::MOVING_FILES_AROUND;

					completed_move = launch_async(
						[target_archive_path, NEW_path, OLD_path, rm_rf, mkdir_p, mv]() {
							const auto resource_folder = get_first_folder_in(NEW_path);
							const auto NEW_root_path = resource_folder;
							LOG_NVPS(NEW_root_path);

							auto move_content_to_current_from = [&](const auto& source_root) {
								LOG("Moving content from %x to current directory.", source_root);

								auto do_move = [&](const auto& fname) {
									return mv(fname, fname.filename());
								};

								return augs::for_each_in_directory(source_root, do_move, do_move);
							};

							auto move_new_content_to_current = [&]() {
								return move_content_to_current_from(NEW_path / "hypersomnia");
							};

							auto restore_content_back_from_old = [&]() {
								move_content_to_current_from(OLD_path);
							};
							
							auto move_old_content_to_OLD = [&]() {
								LOG("Moving old content to OLD directory.");
								
								if (rm_rf(OLD_path) == callback_result::ABORT) {
									return false;
								}

								if (mkdir_p(OLD_path) == callback_result::ABORT) {
									return false;
								}

								auto do_move = [&](const auto& it) {
									const auto fname = it.filename();

									const auto paths_from_old_version_to_keep = std::array<augs::path_type, 4> {
										OLD_path,
										NEW_path,
										LOG_FILES_DIR,
										USER_FILES_DIR
									};

									if (found_in(paths_from_old_version_to_keep, fname)) {
										LOG("Omitting the move of %x", fname);
										return callback_result::CONTINUE;
									}

									return mv(fname, OLD_path / fname);
								};

								return augs::for_each_in_directory(".", do_move, do_move);
							};

							auto remove_now_unneeded_archive = [&]() {
								rm_rf(target_archive_path);
							};

							auto remove_now_unneeded_NEW_folder = [&]() {
								rm_rf(NEW_path);
							};

							remove_now_unneeded_archive();

							if (move_old_content_to_OLD()) {
								if (move_new_content_to_current()) {
									remove_now_unneeded_NEW_folder();

									return callback_result::CONTINUE;
								}
								else {
									/* 
										Very unlikely that this fails, 
										but in this case the user will just be left with partial files in their game folder,
										but with OLD_HYPERSOMNIA untouched.
									*/
								}
							}
							else {
								/* 
									Most likely case of failure:
									Something in the game's files was opened and could not be moved.
								*/
								restore_content_back_from_old();
							}

							return callback_result::ABORT;
						}
					);
				}
				
				print_extracting_progress();
			}
			else if (current_state == state::MOVING_FILES_AROUND) {
				if (valid_and_is_ready(completed_move)) {
					const auto result = completed_move.get();

					if (result == callback_result::CONTINUE) {
						interrupt(R::UPGRADED);
					}
					else if (result == callback_result::ABORT) {
						interrupt(R::EXIT_APPLICATION);
					}
				}

				text("Moving files around...");
			}
			else {
				ensure(false && "Unknown state!");
			}
		}

		if (current_state != state::MOVING_FILES_AROUND) {
			if (do_cancel_button()) {
				interrupt(R::CANCELLED);
			}
		}
	};

	while (!should_quit) {
#if PLATFORM_WINDOWS
		{
			/* Sleep to be easy on the CPU */

			using namespace std::chrono_literals;
			std::this_thread::sleep_for(16ms);
		}
#endif

#if PLATFORM_UNIX
			if (signal_status != 0) {
				const auto sig = signal_status.load();

				LOG("%x received.", strsignal(sig));

				if(
					sig == SIGINT
					|| sig == SIGSTOP
					|| sig == SIGTERM
				) {
					LOG("Gracefully shutting down.");
					interrupt(R::EXIT_APPLICATION);
				}
			}
#endif

		if (window != std::nullopt) {
			window->collect_entropy(entropy);

			for (const auto& e : entropy) {
				if (e.is_exit_message()) {
					interrupt(R::EXIT_APPLICATION);
				}
			}

			const auto frame_delta = frame_timer.extract_delta();
			const auto dt_secs = frame_delta.in_seconds();
			total_secs += dt_secs;

			augs::imgui::setup_io_settings(
				dt_secs,
				window_size
			);

			augs::imgui::pass_inputs(entropy);
		}

		ImGui::NewFrame();
		center_next_window(vec2::square(1.f), ImGuiCond_Always);
		
		advance_update_logic();
		augs::imgui::render();

		if (window != std::nullopt) {
			ensure(imgui_atlas != std::nullopt);

			renderer.clear_current_fbo();
			renderer.draw_call_imgui(
				*imgui_atlas, 
				nullptr, 
				nullptr, 
				nullptr
			);

			rendering_result.clear();

			{
				auto& r = renderer;

				ensure(renderer_backend != std::nullopt);

				renderer_backend->perform(
					rendering_result,
					r.commands.data(),
					r.commands.size(),
					r.dedicated
				);
			}

			for (const auto& f : rendering_result.imgui_lists_to_delete) {
				IM_DELETE(f);
			}

			window->swap_buffers();
		}

		renderer.next_frame();
	}

	return result;
}
