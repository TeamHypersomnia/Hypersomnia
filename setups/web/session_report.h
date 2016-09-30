#pragma once
#include <thread>

struct session_report {
	struct MHD_Daemon *d = nullptr;

	std::string prepend_html;
	std::string append_html;

	std::string fetched_stats;

	bool start_daemon(const std::string session_report_html, const unsigned short port);
	void stop_daemon();
};

