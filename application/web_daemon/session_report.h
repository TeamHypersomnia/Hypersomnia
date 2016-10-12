#pragma once
#include <mutex>
#include <functional>

class config_values;

struct session_report {
	struct MHD_Daemon *d = nullptr;

	std::function<void()> last_seen_updater;
	std::string prepend_html;
	std::string append_html;

	std::string fetched_stats;

	std::mutex fetch_stats_mutex;

	void fetch_stats(std::string new_stats);

	bool start_daemon(const config_values& cfg);
	void stop_daemon();
};

