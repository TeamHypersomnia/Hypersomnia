#pragma once
#include "augs/misc/async_response.h"

#if PLATFORM_WEB
#include <emscripten/fetch.h>
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
        async_get_progress_callback on_progress = nullptr
    ) {
        auto response = std::make_shared<async_response>();
        response->set_on_progress(on_progress);

#if PLATFORM_WEB
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.timeoutMSecs = 5000;

        std::vector<const char*> headerData;
        for (const auto& header : headers) {
            headerData.push_back(header.first.c_str());
            headerData.push_back(header.second.c_str());
        }
        headerData.push_back(nullptr); // Terminate the headers array
        attr.requestHeaders = headerData.data();

        attr.onsuccess = [](emscripten_fetch_t* fetch) {
            auto response = static_cast<async_response*>(fetch->userData);
            response->set({ fetch->status, std::string(fetch->data, fetch->numBytes) });
            emscripten_fetch_close(fetch);
        };

        attr.onerror = [](emscripten_fetch_t* fetch) {
            auto response = static_cast<async_response*>(fetch->userData);
            response->set({ fetch->status, "" });
            emscripten_fetch_close(fetch);
        };

        attr.onprogress = [](emscripten_fetch_t* fetch) {
            auto response = static_cast<async_response*>(fetch->userData);
            if (fetch->totalBytes > 0 && response->get_on_progress()) {
                response->get_on_progress()(fetch->numBytes, fetch->totalBytes);
            }
        };

        attr.userData = response.get();
        std::string full_url = base_url + location;
        emscripten_fetch(&attr, full_url.c_str());
#else
        std::thread([base_url, location, headers, on_progress, response]() {
            auto cli = httplib_utils::make_client(base_url);

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
