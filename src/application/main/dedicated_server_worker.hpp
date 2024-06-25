#pragma once

struct dedicated_server_worker_input {
	std::unique_ptr<server_setup> server_ptr;
	augs::path_type appimage_path;
	self_update_settings self_update;
	std::function<void(server_vars)> write_vars_to_disk;
	std::string instance_label;
	std::string instance_log_label;
};

inline work_result dedicated_server_worker(
	const dedicated_server_worker_input& in,
	std::function<bool()> should_interrupt
) {
	auto& server = *in.server_ptr;

	const auto run_result = [&]() {
		std::future<self_update_result> availability_check;

		server_network_info server_stats;
		network_profiler network_performance;

		while (server.is_running()) {
			const auto zoom = 1.f;

			if (should_interrupt()) {
				LOG("Interrupt was requested.");
				return work_result::SUCCESS;
			}

			server.advance(
				{
					vec2i(),
					input_settings(),
					zoom,
					nat_detection_result(),
					network_performance,
					server_stats
				},
				solver_callbacks()
			);

			if (server.should_write_vars_to_disk_once()) {
				if (in.write_vars_to_disk != nullptr) {
					in.write_vars_to_disk(server.get_current_vars());
				}
			}

			if (server.should_check_for_updates_once()) {
				LOG("Launching an async check for updates.");

				auto settings = in.self_update;

				/* Give it a little longer, it's async anyway. */
				settings.update_connection_timeout_secs = 10;
				auto appimage_path = in.appimage_path;

				availability_check = launch_async(
					[appimage_path, settings]() {
						const bool only_check_update_availability_and_quit = true;

						return check_and_apply_updates(
							appimage_path,
							only_check_update_availability_and_quit,
							settings
						);
					}
				);
			}

			if (valid_and_is_ready(availability_check)) {
				LOG("Finished the async check for updates.");

				using update_result = self_update_result_type;

				const auto result = availability_check.get();

				if (result.type == update_result::UPDATE_AVAILABLE) {
					return work_result::RELAUNCH_AND_UPDATE_DEDICATED_SERVER;
				}
				else {
					LOG("The dedicated server is up to date.");
				}
			}

			server.sleep_until_next_tick();
		}

		if (server.server_restart_requested()) {
			return work_result::RELAUNCH_DEDICATED_SERVER;
		}

		return work_result::SUCCESS;
	}();

	LOG("Quitting %x server instance with: %x", in.instance_label, ::describe_work_result(run_result));

	return run_result;
}

inline auto make_server_worker(
	dedicated_server_worker_input&& in,
	std::function<bool()> should_interrupt,
	std::function<void(work_result)> on_instance_exit
) {
	return [
		in_moved = dedicated_server_worker_input(std::move(in)),
		should_interrupt,
		on_instance_exit
	]() {
		LOG_THREAD_PREFFIX() = in_moved.instance_log_label;

		on_instance_exit(dedicated_server_worker(in_moved, should_interrupt));
	};
}
