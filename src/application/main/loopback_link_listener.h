#pragma once
#include <memory>
#include <optional>
#include <string>

/*
	"Link with Steam" in the native game: a listener on 127.0.0.1 that the browser comes back to after the sign-in
	through Steam (a loopback redirect, RFC 8252 - as desktop apps sign in with OAuth). The site sends the browser to
	http://127.0.0.1:<port>/linked?code=<one-time code>&state=<the game's state>; only a request with this game's
	state is taken. Loopback only: no port is opened to the network, so no firewall asks.
	Not in the web build (it gets the code through its own tab).
*/

class loopback_link_listener {
	struct impl;
	std::unique_ptr<impl> pimpl;

public:
	/*
		Listens on a free port of 127.0.0.1; no value if it could not.
	*/

	std::optional<int> start(const std::string& state);
	void stop();

	/*
		The code the browser brought back, once.
	*/

	std::optional<std::string> take_code();

	bool is_listening() const;

	loopback_link_listener();
	~loopback_link_listener();

	loopback_link_listener(const loopback_link_listener&) = delete;
	loopback_link_listener& operator=(const loopback_link_listener&) = delete;
};
