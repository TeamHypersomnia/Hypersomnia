#include "session_report.h"
#include "3rdparty/http/microhttpd.h"
#include <cstdio>
#include <string>

#include "augs/filesystem/file.h"

static int
ahc_echo(void *cls,
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data, size_t *upload_data_size, void **ptr)
{
	static int aptr;

	session_report& rep = *reinterpret_cast<session_report*>(cls);
	const auto page = rep.prepend_html + rep.fetched_stats + rep.append_html;

	struct MHD_Response *response;
	int ret;

	if (0 != strcmp(method, "GET"))
		return MHD_NO;              /* unexpected method */
	if (&aptr != *ptr)
	{
		/* do never respond on first call */
		*ptr = &aptr;
		return MHD_YES;
	}
	*ptr = NULL;                  /* reset when done */
	response = MHD_create_response_from_buffer(page.length(),
		(void *)page.c_str(),
		MHD_RESPMEM_MUST_COPY);
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	return ret;
}

bool session_report::start_daemon(const std::string session_report_html, const unsigned short port) {
	std::string contents = augs::get_file_contents(session_report_html);
	
	const std::string stats_token = "%stats%";

	auto it = contents.find(stats_token);

	prepend_html.assign(contents.begin(), contents.begin() + it);
	append_html.assign(contents.begin() + it+stats_token.length(), contents.end());

	d = MHD_start_daemon(// MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_POLL,
		MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
		// MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG | MHD_USE_POLL,
		// MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
		port,
		NULL, NULL, &ahc_echo, this,
		MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)120,
		MHD_OPTION_END);

	return d != NULL;
}

void session_report::stop_daemon() {
	MHD_stop_daemon(d);
}