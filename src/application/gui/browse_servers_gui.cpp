#include "application/gui/browse_servers_gui.h"
#include "3rdparty/cpp-httplib/httplib.h"

#if 0
static std::string get_internal_ip() {
	const char* google_dns_server = "8.8.8.8";
	int dns_port = 53;

	struct sockaddr_in serv;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
	{
		return "";
	}

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(google_dns_server);
	serv.sin_port = htons(dns_port);

	int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));

	if (err < 0)
	{
		return "";
	}

	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*)&name, &namelen);

	char buffer[80];
	const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);

	if (p != NULL)
	{
		return buffer;
	}

	close(sock);
	return "";
}
#endif

void browse_servers_gui_state::perform() {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	centered_size_mult = vec2(0.75f, 0.6f);

	auto imgui_window = make_scoped_window();

	if (!imgui_window) {
		return;
	}

	thread_local ImGuiTextFilter filter;
	filter.Draw();
}
