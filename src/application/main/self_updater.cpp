#if PLATFORM_UNIX
#include <csignal>
#endif

#include "augs/log.h"
#include "augs/string/typesafe_sscanf.h"
#include "augs/templates/thread_templates.h"
#include "augs/math/matrix.h"
#include "application/main/self_updater.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/renderer_backend.h"
#include "augs/misc/timing/timer.h"
#include "augs/window_framework/window.h"
#include "3rdparty/include_httplib.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "augs/graphics/shader.h"
#include "augs/graphics/texture.h"
#include "augs/misc/readable_bytesize.h"

#include "view/shader_paths.h"
#include "augs/readwrite/byte_file.h"

#include "augs/filesystem/directory.h"
#include "application/detail_file_paths.h"
#include "application/main/new_and_old_hypersomnia_path.h"
#include "application/main/verify_signature.h"
#include "application/main/extract_archive.h"
#include "hypersomnia_version.h"
#include "augs/window_framework/exec.h"

#if PLATFORM_MACOS
#define PLATFORM_STRING "MacOS"
#define ARCHIVE_EXTENSION "app.sfx"
#elif PLATFORM_LINUX
#define PLATFORM_STRING "Linux"
#define ARCHIVE_EXTENSION "sfx"
#elif PLATFORM_WINDOWS
#define PLATFORM_STRING "Windows"
#define ARCHIVE_EXTENSION "exe"
#else
#error "UNSUPPORTED!"
#endif

#if PLATFORM_UNIX
extern std::atomic<int> signal_status;
#endif

#include "augs/misc/httplib_utils.h"

using namespace httplib_utils;

using R = self_update_result_type;
namespace fs = std::filesystem;

void rename_cwd_to_old();

self_update_result check_and_apply_updates(
	const augs::path_type& current_appimage_path,
	const bool only_check_availability_and_quit,
	const augs::image& imgui_atlas_image,
	const http_client_settings& http_settings,
	augs::window_settings window_settings,
	const bool headless
) {
	using namespace augs::imgui;

	self_update_result result;

	const auto ca_path = CA_CERT_PATH;
	const auto& host_url = http_settings.self_update_host;

	http_client_type http_client(host_url.c_str());

#if BUILD_OPENSSL
	http_client.set_ca_cert_path(ca_path.c_str());
	http_client.enable_server_certificate_verification(true);
#endif
	http_client.set_follow_location(true);
	http_client.set_read_timeout(http_settings.update_connection_timeout_secs);
	http_client.set_write_timeout(http_settings.update_connection_timeout_secs);

	const auto& update_path = http_settings.self_update_path;

#if PLATFORM_LINUX
	const bool is_appimage = !current_appimage_path.empty();
#else
	const bool is_appimage = false;
	(void)current_appimage_path;
#endif

	const auto version_path = 
		is_appimage ? 
		typesafe_sprintf("%x/version-AppImage.txt", update_path) :
		typesafe_sprintf("%x/version-%x.txt", update_path, PLATFORM_STRING)
	;

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
	std::string new_commit_hash; // unused yet
	std::string new_signature;
	const auto current_version = hypersomnia_version().get_version_string();

	{
		const auto response = launch_download(http_client, version_path); 

		if (response == nullptr) {
			log_null_response();
			result.type = R::FAILED;
			return result;
		}

		if (response->status == 404 || response->status == 403) {
			result.type = R::COULDNT_DOWNLOAD_VERSION_FILE;
			return result;
		}

		if (!successful(response->status)) {
			LOG("Request failed with status: %x", response->status);
			result.type = R::FAILED;
			return result;
		}

		const auto& contents = response->body;

		auto s = std::stringstream(contents);

		if (!std::getline(s, new_version)) {
			result.type = R::COULDNT_DOWNLOAD_VERSION_FILE;
			return result;
		}

		if (!std::getline(s, new_commit_hash)) {
			result.type = R::COULDNT_DOWNLOAD_VERSION_FILE;
			return result;
		}

		for (std::string line; std::getline(s, line); ) {
			new_signature += line;

			if (s.good()) {
				new_signature += "\n";
			}
		}

		const bool more_recent = ::is_more_recent(new_version, current_version);

		if (!more_recent) {
			LOG("The game is up to date. Version: %x", current_version);

			result.type = R::UP_TO_DATE;
			return result;
		}

		LOG("Newer version available. Requesting upgrade. \nOld: %x\nNew: %x", current_version, new_version);
	}
	
	if (only_check_availability_and_quit) {
		result.type = R::UPDATE_AVAILABLE;
		return result;
	}

	const auto archive_path = 
		is_appimage ? 
		typesafe_sprintf("%x/Hypersomnia.AppImage", update_path) :
		typesafe_sprintf("%x/Hypersomnia-for-%x.%x", update_path, PLATFORM_STRING, ARCHIVE_EXTENSION)
	;

	LOG_NVPS(version_path, archive_path);

	if (is_appimage) {
		/*
			Changes in flow when we update an AppImage, chronologically:

			1) No managing NEW_HYPERSOMNIA/OLD_HYPERSOMNIA. 
			2) No extracting necessary (yay).
			3) Do not move around any folders in the cwd.
			4) rm/mv just the single downloaded AppImage.
			5) Instead simply rename cwd to cwd.old. The run script will populate the new cwd with the necessary config and content files.
		*/

		LOG("Updating from an AppImage: %x", archive_path);
	}

	const auto win_bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	auto fix_background_color = scoped_style_color(ImGuiCol_WindowBg, ImVec4{win_bg.x, win_bg.y, win_bg.z, 1.f});

	const auto line_height = 28;
	const auto num_lines = 13;

	const auto window_size = vec2i(600, line_height * num_lines);

	window_settings.size = window_size;
	window_settings.fullscreen = false;
	window_settings.border = false;

	std::optional<augs::window> window;
	std::optional<augs::graphics::renderer_backend> renderer_backend;
	std::optional<augs::graphics::texture> imgui_atlas;
	std::optional<augs::graphics::shader_program> standard;

	augs::renderer renderer;

	if (headless) {
		LOG("Forced updating in headless mode (server application).");
	}
	else {
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

			if (standard.has_value()) {
				standard->set_as_current(renderer);
				standard->set_projection(renderer, augs::orthographic_projection(vec2(window_size)));
			}
		}
		catch (const augs::window_error& err) {
			LOG("Failed to open the window:\n%x", err.what());
			LOG("Updating in headless mode.");
		}
	}

	renderer_backend_result rendering_result;

	augs::local_entropy entropy;

	renderer.set_viewport({ vec2i{0, 0}, window_size });

	std::atomic<uint64_t> downloaded_bytes = 1;
	std::atomic<uint64_t> total_bytes = 1;
	std::atomic<bool> exit_requested = false;

	LOG("Launching download.");

	const auto archive_filename = augs::path_type(archive_path).filename();

	auto appimage_new_path = current_appimage_path;
	appimage_new_path += ".new";

	/* 
		The new AppImage has to be on the same device as the current,
		otherwise the final std::filesystem::rename will fail with "Invalid cross-device link" error.
	*/

	const auto target_archive_path = 
		is_appimage ?
		appimage_new_path :
		GENERATED_FILES_DIR / archive_filename
	;

	const auto NEW_path = augs::path_type(NEW_HYPERSOMNIA);
	const auto OLD_path = augs::path_type(OLD_HYPERSOMNIA);

	const auto version_verification_file_path = NEW_path / "hypersomnia" / "release_notes.txt";

	auto future_response = launch_async(
		/* Using optional as the return type only to fix the compilation error on Windows */
		[&exit_requested, archive_path, &http_client, &downloaded_bytes, &total_bytes]() -> std::optional<httplib::Result> {
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
		SAVING_ARCHIVE_TO_DISK,
		EXTRACTING,
		MOVING_FILES_AROUND
	};

	auto current_state = state::DOWNLOADING;
	auto extractor = std::optional<archive_extractor>();
	//auto mover = std::optional<updated_files_mover>();
	auto completed_move = std::future<callback_result>();
	auto completed_save = std::future<self_update_result_type>();

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
		/*
			Note that std::filesystem::remove_all does not need escaping with " like a command argument.
		*/

		auto do_remove = [&]() {
			LOG("rm -rf \"%x\"", p);
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
		/*
			Note that std::filesystem::rename does not need escaping with " like a command argument.
		*/
		auto do_rename = [&]() {
			LOG("mv \"%x\" \"%x\"", a, b);

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
			const auto current_version = hypersomnia_version().get_version_string();

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

			const auto num_revs = std::abs(static_cast<int>(commit) - static_cast<int>(hypersomnia_version().commit_number));
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

	augs::timer download_progress_timer;

	auto print_download_progress_bar = [&download_progress_timer, &downloaded_bytes, &total_bytes, &archive_filename]() {
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

		if (download_progress_timer.get<std::chrono::seconds>() > 1.0/3) {
			LOG("Download progress: %x%", completion_mult * 100);
			download_progress_timer.reset();
		}

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
					auto result = future_response.get();

					if (result == std::nullopt || result.value() == nullptr) {
						log_null_response();
						interrupt(R::FAILED);
						return;
					}

					const auto& response = result.value();

					if (!successful(response->status)) {
						LOG("Request failed with status: %x", response->status);

						if (response->status == 404 || response->status == 403) {
							interrupt(R::COULDNT_DOWNLOAD_BINARY);
							return;
						}

						interrupt(R::FAILED);
						return;
					}

					LOG_NVPS(response->body.length());

					const auto response_length = response->body.length();
					const bool is_an_error_message_instead_of_binary = response_length < 2000;

					if (is_an_error_message_instead_of_binary) {
						LOG(response->body);
						interrupt(R::COULDNT_DOWNLOAD_BINARY);
						return;
					}

					completed_save = launch_async(
						[is_appimage, guarded_save_as_bytes, make_executable, rm_rf, mkdir_p, new_signature, new_binary_bytes = std::move(response->body), target_archive_path, NEW_path]() {
							auto failed = [](const auto e) {
								return e == callback_result::ABORT;
							};

							if (failed(guarded_save_as_bytes(new_binary_bytes, target_archive_path))) {
								return R::COULDNT_SAVE_BINARY;
							}
							
							{
								const auto target_signature_path = augs::path_type(GENERATED_FILES_DIR) / "last_updater_signature.sig";
								augs::save_as_text(target_signature_path, new_signature);

								const auto result = ::verify_ssh_signature(
									target_archive_path,
									target_signature_path
								);

								if (result == ssh_verification_result::OK) {
									LOG("Successfully verified the new binary's signature. Continuing update.");
								}
								else {
									rm_rf(target_archive_path);

									if (result == ssh_verification_result::NO_KEYGEN) {
										return R::FAILED_TO_OPEN_SSH_KEYGEN;
									}
									else {
										return R::FAILED_TO_VERIFY_BINARY;
									}
								}
							}

							if (failed(make_executable(target_archive_path))) {
								return R::COULDNT_SAVE_BINARY;
							}
							
							(void)is_appimage;

							/* AppImage flow 1) No managing NEW_HYPERSOMNIA/OLD_HYPERSOMNIA. */
							if (!is_appimage) {
								/* Create fresh NEW_HYPERSOMNIA folder - remove the old one */

								if (failed(rm_rf(NEW_path))) {
									return R::COULDNT_SAVE_BINARY;
								}

								if (failed(mkdir_p(NEW_path))) {
									return R::COULDNT_SAVE_BINARY;
								}
							}

							return R::NONE;
						}
					);

					current_state = state::SAVING_ARCHIVE_TO_DISK;
				}

				print_download_progress_bar();
			}
			else if (current_state == state::SAVING_ARCHIVE_TO_DISK) {
				if (valid_and_is_ready(completed_save)) {
					const auto code_successful = R::NONE;
					const auto exit_code = completed_save.get();

					if (exit_code == code_successful) {
						/* AppImage flow 2) No extracting necessary (yay). */
						if (is_appimage) {
							try {
								/* 
									Verify that the version is the same as the claimed new one.
									The claimed one is already known to be more recent.

									This time verify this on a signed version file,
									so that we are immune to a rollback attack.
								*/

								const auto signed_downloaded_version = augs::exec(typesafe_sprintf("%x --version-line", target_archive_path.string()));

								if (signed_downloaded_version == new_version) {
									/* Serious stuff begins here. */

									LOG("Downloaded version matches the claimed one. Swapping AppImages.");

									result.exit_with_failure_if_not_upgraded = true;
									current_state = state::MOVING_FILES_AROUND;

									auto appimage_move_files_around_procedure = [target_archive_path, current_appimage_path, rm_rf, mv]() {
										/* AppImage flow 3) Do not move around any folders in the cwd. */
										/* AppImage flow 4) rm/mv just the single downloaded AppImage. */

										if (rm_rf(current_appimage_path) == callback_result::ABORT) {
											return callback_result::ABORT;
										}

										if (mv(target_archive_path, current_appimage_path) == callback_result::ABORT) {
											return callback_result::ABORT;
										}

										try {
											/* 
												AppImage flow 5) Instead simply rename cwd to cwd.old. 
												The run script will populate the cwd with the necessary config and content files.

												It will also bring back the user/log files from the .old folder.
											*/

											rename_cwd_to_old();
											return callback_result::CONTINUE;
										}
										catch (...) {
											return callback_result::ABORT;
										}
									};

									completed_move = launch_async(appimage_move_files_around_procedure);
								}
								else {
									LOG("Downloaded version DOES NOT MATCH the claimed one! (%x != %x). Possible rollback attack!", signed_downloaded_version, new_version);
									interrupt(R::DOWNLOADED_BINARY_WAS_OLDER);
								}
							}
							catch (...) {
								LOG("Couldn't verify the version of the downloaded archive.");

								interrupt(R::DOWNLOADED_BINARY_WAS_OLDER);
							}
						}
						else {
							extractor.emplace(target_archive_path, NEW_path, exit_requested);
							current_state = state::EXTRACTING;
							LOG("Extracting.");
						}
					}
					else {
						interrupt(exit_code);
					}
				}

				print_saving_progress();
			}
			else if (current_state == state::EXTRACTING) {
				ensure(!is_appimage);

				if (extractor->has_completed()) {
					auto move_files_around_procedure = [target_archive_path, NEW_path, OLD_path, rm_rf, mkdir_p, mv]() {
						const auto paths_from_old_version_to_keep = std::array<augs::path_type, 2> {
							LOG_FILES_DIR,
							USER_FILES_DIR
						};

						auto remove_dangling_OLD_path = [&]() {
							return rm_rf(OLD_path) == callback_result::CONTINUE;
						};

#ifdef __APPLE__
						{
							/* 
								For MacOS, simply move around the Contents folders.
								We don't even create any folders out of thin air.
							*/

							(void)mkdir_p;

							const auto BUNDLE_path = get_bundle_directory();
							const auto CURRENT_path = BUNDLE_path / "Contents";
							const auto EXE_dir = augs::path_type(get_executable_path()).replace_filename("");

							auto backup_user_files = [&]() {
								for (const auto& u : paths_from_old_version_to_keep) {
									if (mv(EXE_dir / u, BUNDLE_path / u) == callback_result::ABORT) {
										return false;
									}
								}

								return true;
							};

							auto move_old_content_to_OLD = [&]() {
								return mv(CURRENT_path, OLD_path) == callback_result::CONTINUE;
							};

							auto move_NEW_to_CURRENT = [&]() {
								return mv(NEW_path / "Hypersomnia.app" / "Contents", CURRENT_path) == callback_result::CONTINUE;
							};

							auto restore_user_files = [&]() {
								for (const auto& u : paths_from_old_version_to_keep) {
									if (mv(BUNDLE_path / u, EXE_dir / u) == callback_result::ABORT) {
										return false;
									}
								}

								return true;
							};

							if (!remove_dangling_OLD_path()) {
								return callback_result::ABORT;
							}

							if (!backup_user_files()) {
								return callback_result::ABORT;
							}

							if (!move_old_content_to_OLD()) {
								return callback_result::ABORT;
							}

							if (!move_NEW_to_CURRENT()) {
								return callback_result::ABORT;
							}

							if (!restore_user_files()) {
								return callback_result::ABORT;
							}

							return callback_result::CONTINUE;
						}
#else
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

							if (!remove_dangling_OLD_path()) {
								return false;
							}

							if (mkdir_p(OLD_path) == callback_result::ABORT) {
								return false;
							}

							auto do_move = [&](const auto& it) {
								const auto fname = it.filename();

								const auto intermediate_paths_to_keep = std::array<augs::path_type, 2> {
									OLD_path,
									NEW_path
								};

								if (found_in(paths_from_old_version_to_keep, fname) || found_in(intermediate_paths_to_keep, fname)) {
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
#endif

						return callback_result::ABORT;
					};

					try {
						/* 
							Verify that the version is the same as the claimed new one.
							The claimed one is already known to be more recent.

							This time verify this on a signed version file,
							so that we are immune to a rollback attack.
						*/

						const auto signed_downloaded_version = augs::file_read_first_line(version_verification_file_path);

						if (signed_downloaded_version == new_version) {
							LOG("Downloaded version matches the claimed one (%x).", signed_downloaded_version);

							current_state = state::MOVING_FILES_AROUND;

							result.exit_with_failure_if_not_upgraded = true;

							/* Serious stuff begins here. */

							LOG("Moving files around.");

							current_state = state::MOVING_FILES_AROUND;
							completed_move = launch_async(move_files_around_procedure);
						}
						else {
							LOG("Downloaded version DOES NOT MATCH the claimed one! (%x != %x). Possible rollback attack!", signed_downloaded_version, new_version);
						}
					}
					catch (const augs::file_open_error& err) {
						LOG("Error: couldn't open %x to verify the downloaded game's version (%x).\n%x", version_verification_file_path, new_version, err.what());
					}

					const bool was_successful = current_state == state::MOVING_FILES_AROUND;

					if (!was_successful) {
						interrupt(R::DOWNLOADED_BINARY_WAS_OLDER);
					}
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

		if (window.has_value()) {
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

		if (window.has_value()) {
			renderer.clear_current_fbo();

			ensure(imgui_atlas.has_value());

			if (imgui_atlas.has_value()) {
				renderer.draw_call_imgui(
					*imgui_atlas, 
					nullptr, 
					nullptr, 
					nullptr,
					nullptr
				);
			}

			rendering_result.clear();

			{
				auto& r = renderer;

				ensure(renderer_backend.has_value());

				if (renderer_backend.has_value()) {
					renderer_backend->perform(
						rendering_result,
						r.commands.data(),
						r.commands.size(),
						r.dedicated
					);
				}
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

void rename_cwd_to_old() {
	LOG("rename_cwd_to_old: CWD: %x", fs::current_path());
	fs::path cwd = fs::current_path();

	// make a new name for cwd
	fs::path new_cwd = cwd;
	new_cwd += ".old";

	LOG("rename_cwd_to_old: Remove CWD.old: %x", new_cwd);
	fs::remove_all(new_cwd);

	LOG("rename_cwd_to_old: Change CWD to CWD/..: %x", cwd.parent_path());
	fs::current_path(cwd.parent_path());

	LOG("rename_cwd_to_old: Rename %x to %x", cwd, new_cwd);
	fs::rename(cwd, new_cwd);

	// change current working directory back to the newly renamed directory
	// won't really be necessary as we're quitting but just in case something wants to dump logs
	LOG("rename_cwd_to_old: Set CWD to .old: %x", new_cwd);
	fs::current_path(new_cwd);
}
