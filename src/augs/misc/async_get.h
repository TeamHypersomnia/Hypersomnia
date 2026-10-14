#pragma once
#include <memory>
#include "augs/misc/async_response.h"

#if PLATFORM_WEB
#include <emscripten/fetch.h>
#include "augs/templates/main_thread_queue.h"
#else
#include "augs/misc/httplib_utils.h"
#endif

#undef ADD

namespace augs {
	using Headers = std::unordered_map<std::string, std::string>;

    inline async_response_ptr async_get(
        const std::string& base_url,
        const std::string& location,
        const Headers& headers = {},
        async_get_progress_callback on_progress = nullptr,
        const bool follow_redirects = true
    ) {
        auto response = std::make_shared<async_response>();
        response->set_on_progress(on_progress);

		/*
			Without the query - it can hold a Steam ticket or a signature.
		*/

		LOG("async_get: %x%x", base_url, location.substr(0, location.find('?'))); 

#if PLATFORM_WEB
		(void)follow_redirects;

		auto fetch_lbd = [=]() {
			emscripten_fetch_attr_t attr;
			emscripten_fetch_attr_init(&attr);
			strcpy(attr.requestMethod, "GET");
			attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
			/*
				A download with its progress shown (e.g. a map from a mirror) may take long - only the rest give up after 5 s.
			*/

			attr.timeoutMSecs = on_progress ? 0 : 5000;

			std::vector<const char*> headerData;
			for (const auto& header : headers) {
				headerData.push_back(header.first.c_str());
				headerData.push_back(header.second.c_str());
			}
			headerData.push_back(nullptr); // Terminate the headers array
			attr.requestHeaders = headerData.data();

			/*
				The fetch owns a reference of its own - the caller may drop its pointer (a cancel, a new key)
				while the request is still on its way.
			*/

			using owner_type = std::shared_ptr<async_response>;

			attr.onsuccess = [](emscripten_fetch_t* fetch) {
				ensure(fetch != nullptr);
				LOG("async_get: onsuccess %x", fetch->status);
				const auto owner = std::unique_ptr<owner_type>(static_cast<owner_type*>(fetch->userData));
				(*owner)->set({ fetch->status, std::string(fetch->data, fetch->numBytes) });
				emscripten_fetch_close(fetch);
			};

			attr.onerror = [](emscripten_fetch_t* fetch) {
				ensure(fetch != nullptr);
				LOG("async_get: onerror %x", fetch->status);
				const auto owner = std::unique_ptr<owner_type>(static_cast<owner_type*>(fetch->userData));

				/*
					An HTTP error still has a body - e.g. why a code or a link was refused.
				*/

				const auto body = fetch->status > 0 && fetch->data != nullptr ? std::string(fetch->data, fetch->numBytes) : std::string();
				(*owner)->set({ fetch->status, body });
				emscripten_fetch_close(fetch);
			};

			attr.onprogress = [](emscripten_fetch_t* fetch) {
				ensure(fetch != nullptr);
				const auto& response = *static_cast<owner_type*>(fetch->userData);

				if (fetch->totalBytes > 0 && response->get_on_progress()) {
					response->get_on_progress()(fetch->numBytes, fetch->totalBytes);
				}
			};

			attr.userData = new owner_type(response);
			std::string full_url = base_url + location;
			LOG("async_get: emscripten_fetch %x%x", base_url, location.substr(0, location.find('?')));
			const auto fetch = emscripten_fetch(&attr, full_url.c_str());

			ensure(fetch != nullptr);
		};

		main_thread_queue::execute_async(fetch_lbd);
#else
        std::thread([base_url, location, headers, on_progress, response, follow_redirects]() {
            auto cli = httplib_utils::make_client(base_url);
            cli->set_follow_location(follow_redirects);

            httplib::Headers httplib_headers;

            for (const auto& header : headers) {
                httplib_headers.insert(header);
            }

            const auto res = [&]() {
                if (on_progress) {
                    return cli->Get(location.c_str(), httplib_headers, on_progress);
                }

                return cli->Get(location.c_str(), httplib_headers);
            }();

            if (res) {
                response->set({ res->status, res->body });
            }
			else {
                response->set(http_response());
            }
        }).detach();
#endif

        return response;
    }
}
