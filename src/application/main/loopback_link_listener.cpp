#include <mutex>
#include <thread>
#include <utility>
#include "augs/log.h"
#include "application/main/loopback_link_listener.h"

#if !PLATFORM_WEB
#include "3rdparty/include_httplib.h"
#endif

#if !PLATFORM_WEB
struct loopback_link_listener::impl {
	httplib::Server server;
	std::thread thread;
	std::string state;

	std::mutex code_mutex;
	std::optional<std::string> code;
};
#else
struct loopback_link_listener::impl {};
#endif

loopback_link_listener::loopback_link_listener() = default;

loopback_link_listener::~loopback_link_listener() {
	stop();
}

static bool is_link_code(const std::string& code) {
	if (code.size() != 8) {
		return false;
	}

	for (const auto c : code) {
		const bool upper = c >= 'A' && c <= 'Z';
		const bool digit = c >= '0' && c <= '9';

		if (!upper && !digit) {
			return false;
		}
	}

	return true;
}

std::optional<int> loopback_link_listener::start(const std::string& state) {
#if PLATFORM_WEB
	(void)state;
	return std::nullopt;
#else
	stop();

	pimpl = std::make_unique<impl>();
	pimpl->state = state;

	auto* const self = pimpl.get();

	/*
		The page the browser lands on - the game finishes the link by itself.
	*/

	auto page = [](const std::string& title, const std::string& text) {
		return
			"<!doctype html><html><head><meta charset=\"utf-8\"><title>Hypersomnia</title>"
			"<style>body{background:#151514;color:#e8e8e3;font:18px/1.5 system-ui,sans-serif;"
			"display:flex;align-items:center;justify-content:center;height:90vh;text-align:center}"
			"h1{font-size:28px}</style></head><body><div><h1>" + title + "</h1><p>" + text + "</p></div></body></html>"
		;
	};

	pimpl->server.Get("/linked", [self, page](const httplib::Request& req, httplib::Response& res) {
		const auto code = req.get_param_value("code");
		const auto state = req.get_param_value("state");

		res.set_header("Cache-Control", "no-store");
		res.set_header("Connection", "close");

		if (state != self->state || !is_link_code(code)) {
			LOG("The link listener got a request that is not for this game.");
			res.status = 400;
			res.set_content(page("Not for this game", "This link was not started by the game on this computer. Start again in the game: Settings - Account - Link with Steam."), "text/html");
			return;
		}

		{
			std::lock_guard<std::mutex> lock(self->code_mutex);
			self->code = code;
		}

		res.set_content(page("Signed in", "Go back to Hypersomnia - it finishes the link by itself. You can close this tab."), "text/html");
	});

	const auto port = pimpl->server.bind_to_any_port("127.0.0.1");

	if (port <= 0) {
		LOG("The link listener could not bind to 127.0.0.1.");
		pimpl.reset();
		return std::nullopt;
	}

	pimpl->thread = std::thread([self]() {
		self->server.listen_after_bind();
	});

	LOG("The link listener is on 127.0.0.1:%x.", port);
	return port;
#endif
}

void loopback_link_listener::stop() {
#if !PLATFORM_WEB
	if (pimpl == nullptr) {
		return;
	}

	pimpl->server.stop();

	if (pimpl->thread.joinable()) {
		pimpl->thread.join();
	}

	pimpl.reset();
#endif
}

std::optional<std::string> loopback_link_listener::take_code() {
#if PLATFORM_WEB
	return std::nullopt;
#else
	if (pimpl == nullptr) {
		return std::nullopt;
	}

	std::lock_guard<std::mutex> lock(pimpl->code_mutex);
	return std::exchange(pimpl->code, std::nullopt);
#endif
}

bool loopback_link_listener::is_listening() const {
	return pimpl != nullptr;
}
